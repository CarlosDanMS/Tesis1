#include <iostream>


#include "LeptonThread.h"
#include "Palettes.h"
#include "SPI.h"
#include "LEPTON_CONFIG.h"




// Global Variables for Decoding Video
#define PACKET_SIZE 164
#define PACKET_SIZE_UINT16 (PACKET_SIZE/2)
#define PACKETS_PER_FRAME 60
#define FRAME_SIZE_UINT16 (PACKET_SIZE_UINT16*PACKETS_PER_FRAME)
#define FPS 27;
int x=100;

//Global vairables for naming images
QDateTime now;
QString timestamp;
QString fileName;

LeptonThread::LeptonThread() : QThread()
{
	//
	loglevel = 0;

	//
	typeColormap = 3; // 1:colormap_rainbow  /  2:colormap_grayscale  /  3:colormap_ironblack(default)
	selectedColormap = colormap_ironblack;
	selectedColormapSize = get_size_colormap_ironblack();

	//
	typeLepton = 2; // 2:Lepton 2.x  / 3:Lepton 3.x
	myImageWidth = 80;
	myImageHeight = 60;

	//
	spiSpeed = 20 * 1000 * 1000; // SPI bus speed 20MHz

	// min/max value for scaling
	autoRangeMin = true;
	autoRangeMax = true;
	rangeMin = 30000;
	rangeMax = 32000;

	video=false;
	AI=false;
}

LeptonThread::~LeptonThread() {
}

void LeptonThread::setLogLevel(uint16_t newLoglevel)
{
	loglevel = newLoglevel;
}

void LeptonThread::useColormap(int newTypeColormap)
{
	switch (newTypeColormap) {
	case 1:
		typeColormap = 1;
		selectedColormap = colormap_rainbow;
		selectedColormapSize = get_size_colormap_rainbow();
		break;
	case 2:
		typeColormap = 2;
		selectedColormap = colormap_grayscale;
		selectedColormapSize = get_size_colormap_grayscale();
		break;
	default:
		typeColormap = 3;
		selectedColormap = colormap_ironblack;
		selectedColormapSize = get_size_colormap_ironblack();
		break;
	}
}

void LeptonThread::useLepton(int newTypeLepton)
{
	switch (newTypeLepton) {
	case 3:
		typeLepton = 3;
		myImageWidth = 160;
		myImageHeight = 120;
		break;
	default:
		typeLepton = 2;
		myImageWidth = 80;
		myImageHeight = 60;
	}
}

void LeptonThread::useSpiSpeedMhz(unsigned int newSpiSpeed)
{
	spiSpeed = newSpiSpeed * 1000 * 1000;
}

void LeptonThread::setAutomaticScalingRange()
{
	autoRangeMin = true;
	autoRangeMax = true;
}

void LeptonThread::useOutputFormat(uint8_t format)
{
	switch(format){
		case 0: // set Raw 14
			Raw14 = true;
			Raw16 = false;
			AGC = false;
			break;
		case 1:
			Raw14 = false;
			Raw16 = true;
			AGC = false;
			break;
		case 2:
			Raw14 = false;
			Raw16 = false;
			AGC = true;
			break;
		}
}


void LeptonThread::useRangeMinValue(uint16_t newMinValue)
{	autoRangeMin = false;
	if(Raw14) rangeMin = (newMinValue)*10;
	else if(Raw16) rangeMin = (newMinValue/10+273.15)*100;
	else rangeMin = newMinValue/10*255/140;
}

void LeptonThread::useRangeMaxValue(uint16_t newMaxValue)
{	autoRangeMax = false;
	if(Raw14) rangeMax = (newMaxValue)*10;
	else if(Raw16) rangeMax = (newMaxValue/10+273.15)*100;
	else rangeMax = newMaxValue/10*255/140;
}
bool LeptonThread::Datos(QString newName, uint16_t newFrame, uint16_t newTime)
{
	if(newName.isEmpty()) return false;
	else{
		Name = newName;
		Frames = newFrame;
		Time = newTime;
		QString DirName = QString("/home/Thermal_Camera/patients/%1").arg(Name);
		QDir dir(DirName);
		if (!dir.exists())
		{	dir.mkpath(".");
			dir.mkpath("Photos");
			dir.mkpath("Video");
			dir.mkpath("Timed");
		}
		
		return true;}
}

void LeptonThread::Set_NROI(uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2){
		ROI.startCol=x1;
		ROI.endCol=x2;
		ROI.startRow=y1;
		ROI.endRow=y2;
		
		ROI_1.startCol=x1;
		ROI_1.endCol=x2;
		ROI_1.startRow=y1;
		ROI_1.endRow=y2;
		
		LEP_SetROI(ROI,ROI_1);
}

void LeptonThread::startAI(){
	AI=!AI;
	if (AI) QMessageBox::information(nullptr,"Information","Tracking Started");
	else QMessageBox::information(nullptr,"Information","Tracking Finished");
}

void LeptonThread::run()
{
	//create the initial image
	myImage = QImage(myImageWidth, myImageHeight, QImage::Format_RGB888);
	
	uint16_t minValue = rangeMin;
	uint16_t maxValue = rangeMax;
	float diff = maxValue - minValue;
	float scale = 255/diff;
	uint16_t n_wrong_segment = 0;
	uint16_t n_zero_value_drop_frame = 0;
	
	
	
	//open spi port
	SpiOpenPort(0, spiSpeed);

	/*Infinte loop to get and update the image receive by the sensor
	 * we need to update all of this variables at start because this varibales can change oer time
	 * Then, we read the info recieved by SPI in the file system  and store it in result variable,
	 * after some checks of the image we copy the image into segmentNumber array variable for beter control.
	 * And finally we swap the segments to get the 16-bit image. 
	 * Since qt wouldn't allow 16bit image at the time the image has to be reescaled to 8 bit, and send it to
	 * the main to show it.
	 */
	while(true) {
			 const int *colormap = selectedColormap;
			 const int colormapSize = selectedColormapSize;
			 minValue = rangeMin;
			 maxValue = rangeMax;
			 diff = maxValue - minValue;
			 scale = 255/diff;
			 
			 QVector<int> ImageBuffer;
			 
		//read data packets from lepton over SPI
		int resets = 0;
		int segmentNumber = -1;
		for(int j=0;j<PACKETS_PER_FRAME;j++) {
			//if it's a drop packet, reset j to 0, set to -1 so he'll be at 0 again loop
			read(spi_cs0_fd, result+sizeof(uint8_t)*PACKET_SIZE*j, sizeof(uint8_t)*PACKET_SIZE);
			int packetNumber = result[j*PACKET_SIZE+1];
			if(packetNumber != j) {
				j = -1;
				resets += 1;
				usleep(1000);
				//Note: we've selected 750 resets as an arbitrary limit, since there should never be 750 "null" packets between two valid transmissions at the current poll rate
				//By polling faster, developers may easily exceed this count, and the down period between frames may then be flagged as a loss of sync
				if(resets == 750) {
					SpiClosePort(0);
					LEP_Reboot();
					n_wrong_segment = 0;
					n_zero_value_drop_frame = 0;
					usleep(750000);
					SpiOpenPort(0, spiSpeed);
				}
				continue;
			}
			if ((typeLepton == 3) && (packetNumber == 20)) {
				segmentNumber = (result[j*PACKET_SIZE] >> 4) & 0x0f;
				if ((segmentNumber < 1) || (4 < segmentNumber)) {
					log_message(10, "[ERROR] Wrong segment number " + std::to_string(segmentNumber));
					break;
				}
			}
		}
		if(resets >= 30) {
			log_message(3, "done reading, resets: " + std::to_string(resets));
		}


		//
		int iSegmentStart = 1;
		int iSegmentStop;
		if (typeLepton == 3) {
			if ((segmentNumber < 1) || (4 < segmentNumber)) {
				n_wrong_segment++;
				if ((n_wrong_segment % 12) == 0) {
					log_message(5, "[WARNING] Got wrong segment number continuously " + std::to_string(n_wrong_segment) + " times");
				}
				continue;
			}
			if (n_wrong_segment != 0) {
				log_message(8, "[WARNING] Got wrong segment number continuously " + std::to_string(n_wrong_segment) + " times [RECOVERED] : " + std::to_string(segmentNumber));
				n_wrong_segment = 0;
			}

			//
			memcpy(shelf[segmentNumber - 1], result, sizeof(uint8_t) * PACKET_SIZE*PACKETS_PER_FRAME);
			if (segmentNumber != 4) {
				continue;
			}
			iSegmentStop = 4;
		}
		else {
			memcpy(shelf[0], result, sizeof(uint8_t) * PACKET_SIZE*PACKETS_PER_FRAME);
			iSegmentStop = 1;
		}

		if ((autoRangeMin == true) || (autoRangeMax == true)) {
			if (autoRangeMin == true) {
				maxValue = 65535;
			}
			if (autoRangeMax == true) {
				maxValue = 0;
			}
			for(int iSegment = iSegmentStart; iSegment <= iSegmentStop; iSegment++) {
				for(int i=0;i<FRAME_SIZE_UINT16;i++) {
					//skip the first 2 uint16_t's of every packet, they're 4 header bytes
					if(i % PACKET_SIZE_UINT16 < 2) {
						continue;
					}
					uint16_t value;
					//flip the MSB and LSB at the last second
					value = (shelf[iSegment - 1][i*2] << 8) + shelf[iSegment - 1][i*2+1];

					if (value == 0) {
						// Why this value is 0?
						continue;
					}
					if ((autoRangeMax == true) && (value > maxValue)) {
						maxValue = value;
					}
					if ((autoRangeMin == true) && (value < minValue)) {
						minValue = value;
					}
				}
			}
			
			diff = maxValue - minValue;
			scale = 255/diff;
		}

		int row, column;
		uint16_t value;
		uint16_t valueFrameBuffer;
		QRgb color;
		for(int iSegment = iSegmentStart; iSegment <= iSegmentStop; iSegment++) {
			int ofsRow = 30 * (iSegment - 1);
			for(int i=0;i<FRAME_SIZE_UINT16;i++) {
				//skip the first 2 uint16_t's of every packet, they're 4 header bytes
				if(i % PACKET_SIZE_UINT16 < 2) {
					continue;
				}

				//flip the MSB and LSB at the last second
				 valueFrameBuffer = (shelf[iSegment - 1][i*2] << 8) + shelf[iSegment - 1][i*2+1];
				if (valueFrameBuffer == 0) {
					// Why this value is 0?
					n_zero_value_drop_frame++;
					if ((n_zero_value_drop_frame % 12) == 0) {
						log_message(5, "[WARNING] Found zero-value. Drop the frame continuously " + std::to_string(n_zero_value_drop_frame) + " times");
					}
					break;
				}
					value = (valueFrameBuffer - minValue) * scale;
				
					int ofs_r = 3 * value + 0; if (colormapSize <= ofs_r) ofs_r = colormapSize - 1;
					int ofs_g = 3 * value + 1; if (colormapSize <= ofs_g) ofs_g = colormapSize - 1;
					int ofs_b = 3 * value + 2; if (colormapSize <= ofs_b) ofs_b = colormapSize - 1;
					color = qRgb(colormap[ofs_r], colormap[ofs_g], colormap[ofs_b]);
					if (typeLepton == 3) {
						column = (i % PACKET_SIZE_UINT16) - 2 + (myImageWidth / 2) * ((i % (PACKET_SIZE_UINT16 * 2)) / PACKET_SIZE_UINT16);
						row = i / PACKET_SIZE_UINT16 / 2 + ofsRow;
					}
					else {
						column = (i % PACKET_SIZE_UINT16) - 2;
						row = i / PACKET_SIZE_UINT16;
					}
					myImage.setPixel(column, row, color);
					ImageBuffer.append(value);
				}
			}

		if (n_zero_value_drop_frame != 0) {
			log_message(8, "[WARNING] Found zero-value. Drop the frame continuously " + std::to_string(n_zero_value_drop_frame) + " times [RECOVERED]");
			n_zero_value_drop_frame = 0;
		}
		
		// save video
		if(video) saveImage("Video");

		//lets emit the signal for update
		emit updateImage(myImage);
		if (AI){
			emit infereImage(ImageBuffer);
		}
		
		//Get Info of ROI
		//LEP_ROIStatistics(&ROI_Stats,&ROI_Stats_1);
		//qDebug()<<ROI_Stats.meanIntensity<<"  ";
		
	}
	
	//finally, close SPI port just bcuz
	SpiClosePort(0);
}

void LeptonThread::performFFC() {
	//perform FFC
	LEP_PerformFFC();
}


void LeptonThread::log_message(uint16_t level, std::string msg)
{
	if (level <= loglevel) {
		std::cerr << msg << std::endl;
	}
}

void LeptonThread::Photo() {
	saveImage("Photos");
	//x=0;  
}

void LeptonThread::Video() {
	//change video status 
	video = !video;
	
	//give the user some feed if video is started or finished
	if (video) QMessageBox::information(nullptr,"Information","Video Start");
		else QMessageBox::information(nullptr,"Information","VIdeo Stop");
}

void LeptonThread::Timed() {
	//	save images while there strill images(Frames) to take			 
		if(Frames>0){
			 saveImage("Timed");
			 Frames-=1;
			 }
}

void LeptonThread::setBBox(int Temp_BBox[8]){
	// save Bounding Box of the AI into a class variable
	for(int i=0; i<8; i++){
		array[i]=Temp_BBox[i];}
}
/*Inside the saveImage function  we need to always update the date and time
 * with miliseconds to avoid image rewriting
 * If AI is activated we draw a rectangle in the image.
 */ 
void LeptonThread::saveImage(QString Dir){
		now = QDateTime::currentDateTime();
		timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss-zzz"));
		fileName = QString::fromLatin1("/home/Thermal_Camera/patients/%1/%3/%1_%2.png").arg(Name).arg(timestamp).arg(Dir);
		if(AI){
			QPainter painter(&myImage);
			painter.setPen(QPen(Qt::white,1));
			// Draw the line
			for( int i=0;i<8;i+=4)
			{painter.drawRect(array[i],array[i+1],(array[i+2]-array[i]),(array[i+3]-array[i+1]));}
		
			// End painting
			painter.end();
		}
		myImage.save(fileName);	
}

/* this function is exchangeable with the function before and only add the ROI values to the name.
 * this function is apart because getting the values of the ROI add too
 * much delay and provoke lag in the real time video.
 * To use this function is needed to replace only the name of the function
 * in the correspondin callings and uncomment the roi section at the end
 * of the run function.
 */
void LeptonThread::saveImageROI(QString Dir){
		LEP_ROIStatistics(&ROI_Stats,&ROI_Stats_1);
		now = QDateTime::currentDateTime();
		timestamp = now.toString(QLatin1String("yyyyMMdd-hhmmss-zzz"));
		fileName = QString::fromLatin1("/home/Thermal_Camera/patients/%1/%3/%1(%4-%5)_%2.png").arg(Name).arg(timestamp).arg(Dir).
		arg(ROI_Stats.meanIntensity).arg(ROI_Stats_1.radSpotmeterValue);
		if(AI){
			QPainter painter(&myImage);
			painter.setPen(QPen(Qt::white,1));
			// Draw the line
			for( int i=0;i<8;i+=4)
			{painter.drawRect(array[i],array[i+1],(array[i+2]-array[i]),(array[i+3]-array[i+1]));}
		
			// End painting
			painter.end();
		}
		myImage.save(fileName);	
}
