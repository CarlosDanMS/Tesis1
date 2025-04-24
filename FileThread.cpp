#include <iostream>
#include "FileThread.h"

FileThread::FileThread() : QThread(){
}

FileThread::~FileThread() {
}
//at the begining of the main code we set the path because we can't set it
// at the declaration inside the class
void FileThread::setFile(){file.setFileName(Dir);}

/*Declaration of the public slot saveMatrix, this function save the grayscale image
 * as a matrix of 160x120  everytime it produce a new frame
 */
void FileThread::saveMatrix(QVector<int> tempImage){
	if( file.open(QIODevice::WriteOnly))
	{
		QTextStream Stream(&file);
		for( int i=1;i<19201; i++){
			Stream<<tempImage[i-1];
			if(i%160==0) Stream <<"\n";
			else Stream<<",";
		}
	file.close();
	}
}


/*GetImage is the pubic slot that launche python program.
 * we create a new process to handle the python progrman y write the lines to launch
 * the python pogram as if we were in the terminal.
 * we also create handlers to execute the python code and other to delete the file
 * and stop the python program
 */
void FileThread::GetImage()
{	if(pythonAI){
		 // Create a QProcess instance to run the Python script
        QProcess *process = new QProcess(this);

        // Set the command to run the Python interpreter with the script
        QString program = "python";
        QStringList arguments = {"Hand_predict_Grayscale.py"};
        // Hand_predict_Grayscale

        // Start the Python script process
        process->start(program, arguments);      
	}
	else{
		if (!file.exists()) {
			qDebug() << "Error: File does not exist.";
		} else {
			if (!file.remove()) {
				qDebug() << "Error: Unable to remove the file.";
			}
		}
	} 
}
