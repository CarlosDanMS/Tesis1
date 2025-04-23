#include "LEPTON_CONFIG.h"


bool _connected;
LEP_CAMERA_PORT_DESC_T _port;

void LEP_Connect() {
	LEP_OpenPort(1, LEP_CCI_TWI, 400, &_port);
	_connected = true;
}

void LEP_PerformFFC() {
	if(!_connected) {
		LEP_Connect();
	}
	while(LEP_RunSysFFCNormalization(&_port));
	qDebug()<<"FCC Done";

}

///////////////////////////////////////////////////////////////////////
////////////////////// Get Config //////////////////////////////////
////////////////////////////////////////////////////////////////////

void LEP_GetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T_PTR Radiometry){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetRadFluxLinearParams(&_port,Radiometry);	
}


void LEP_GetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T_PTR gainModeObj){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetSysGainModeObj(&_port,gainModeObj);
}

void LEP_GetAGCConfig(LEP_AGC_ROI_T_PTR agcROI, LEP_UINT16 *agcHeqClipLimitHigh, LEP_UINT16 *agcHeqDampingFactor, LEP_UINT16 *agcHeqLinearPercent){		
	if(!_connected) {
		LEP_Connect();
	}
	
	LEP_GetAgcROI( &_port, agcROI);
	LEP_GetAgcHeqDampingFactor( &_port, agcHeqDampingFactor);
	LEP_GetAgcHeqClipLimitHigh( &_port, agcHeqClipLimitHigh);
	LEP_GetAgcHeqLinearPercent( &_port, agcHeqLinearPercent);
}


///////////////////////////////////////////////////////////////////////
////////////////////// Basic Config //////////////////////////////////
////////////////////////////////////////////////////////////////////
void LEP_SetEmissivity(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry, float emissivity){
	if(!_connected) {
		LEP_Connect();
	}
	
	Radiometry.sceneEmissivity=round(8192*emissivity);
	//qDebug()<<Radiometry.sceneEmissivity;
	LEP_SetRadFluxLinearParams(&_port,Radiometry);
	LEP_GetRadFluxLinearParams(&_port,&Radiometry);	
	//qDebug()<<Radiometry.sceneEmissivity;
}

void LEP_SetOutputFormat(int option){
	if(!_connected) {
		LEP_Connect();
	}
	
	// State Variables
	LEP_RAD_ENABLE_E EnableState_Rad;
	LEP_RAD_ENABLE_E EnableState_TLinear;
	LEP_AGC_ENABLE_E agcEnableState;
		
	EnableState_Rad = LEP_RAD_ENABLE; //enable radiometry
	LEP_SetRadEnableState(&_port,EnableState_Rad);
	switch(option){
		case 0: // set Raw 14
			EnableState_TLinear = LEP_RAD_DISABLE; // disable TLineas
			agcEnableState=LEP_AGC_DISABLE; // DIable AGC
			//qDebug()<<"Raw 14 is set";
			break;
		case 1: // set Raw 16 ( Kelvin*100)
			EnableState_TLinear = LEP_RAD_ENABLE; // Enable Tlinear
			agcEnableState=LEP_AGC_DISABLE; // Disable AGC
			//qDebug()<<"Raw 16 is set";
			break;
		case 2: // AGC
			//fist we set raw14
			EnableState_TLinear = LEP_RAD_DISABLE;
			
			//turn off focus metric
			LEP_VID_FOCUS_CALC_ENABLE_E vidFocusCalcEnableState =
											LEP_VID_FOCUS_CALC_DISABLE;
			LEP_SetVidFocusCalcEnableState( &_port,
                                            vidFocusCalcEnableState);
            
			// Enable AGC
			agcEnableState=LEP_AGC_ENABLE;

            //qDebug()<<"AGC is set";
            break;
		}
		LEP_SetRadTLinearEnableState(&_port,EnableState_TLinear);
		LEP_SetAgcEnableState( &_port, agcEnableState);
		
		//LEP_GetRadTLinearEnableState(&_port,&EnableState_TLinear);
		//LEP_GetAgcEnableState( &_port, &agcEnableState);
		//qDebug()<<"TLinear is set: "<<EnableState_TLinear;
		//qDebug()<<"AGC is set to: "<<agcEnableState;
}

void LEP_SetAGCEqu(int option){
	if(!_connected) {
		LEP_Connect();
	}
	
	LEP_AGC_POLICY_E agcPolicy = LEP_AGC_HEQ;
	
	switch (option) {
		case 0:
			agcPolicy = LEP_AGC_LINEAR;
			//qDebug()<<"AGC Linear selected";
			break;
		case 1:
			agcPolicy = LEP_AGC_HEQ;
			//qDebug()<<"AGC HEQ selected";
			break;
		default:
			agcPolicy = LEP_AGC_HEQ;
			//qDebug()<<"AGC HEQ selected";
			break;
	}
	LEP_SetAgcPolicy( &_port, agcPolicy );
	
	//LEP_GetAgcPolicy( &_port, &agcPolicy );
	//qDebug()<<"AGC policy: "<< agcPolicy;
	
}

void LEP_SetGainMode(int option){
	if(!_connected) {
		LEP_Connect();
	}
	
	LEP_SYS_GAIN_MODE_E gainMode = LEP_SYS_GAIN_MODE_HIGH;

    switch (option) {
		case 0:
			gainMode=LEP_SYS_GAIN_MODE_HIGH;
			//qDebug()<<"High Gain mode selected" << gainMode;
			break;
		case 1:
			gainMode=LEP_SYS_GAIN_MODE_LOW;
			//qDebug()<<"Low Gain mode selected" << gainMode;
			break;
		case 2:
			gainMode=LEP_SYS_GAIN_MODE_AUTO;
			//qDebug()<<"Auto Gain mode selected"<< gainMode;
			break;
	 }
	 LEP_SetSysGainMode( &_port, gainMode);
	 
	 //LEP_GetSysGainMode( &_port, &gainMode);
	 //qDebug()<<"Gain mode selected: "<< gainMode;
}

void LEP_SetShutter(int option){
	if(!_connected) {
		LEP_Connect();
	}
	//Get the info the info of the configuration word to only modify 
	//what we need
	LEP_SYS_FFC_SHUTTER_MODE_OBJ_T gainModeObj;
	LEP_GetSysFfcShutterModeObj( &_port,&gainModeObj );
	
	switch (option) {
		case 0:
			gainModeObj.shutterMode=LEP_SYS_FFC_SHUTTER_MODE_MANUAL;
			//qDebug()<<"Manual Shutter"<<gainModeObj.shutterMode;
			break;
		case 1:
			gainModeObj.shutterMode=LEP_SYS_FFC_SHUTTER_MODE_AUTO;
			//qDebug()<<"Auto Shutter"<<gainModeObj.shutterMode;
			break;	
	}
	//gainModeObj.shutterMode=shutter;
	LEP_SetSysFfcShutterModeObj( &_port, gainModeObj);
	
	//LEP_GetSysFfcShutterModeObj( &_port, &gainModeObj);
	//qDebug()<<"shutter mode selected: "<< gainModeObj.shutterMode;
}

///////////////////////////////////////////////////////////////////////
////////////////////// ROI Config //////////////////////////////////
////////////////////////////////////////////////////////////////////
void LEP_SetROI(LEP_SYS_VIDEO_ROI_T ROI, LEP_RAD_ROI_T ROI_1){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_SetSysSceneRoi(&_port,ROI);
	LEP_SetRadSpotmeterRoi(&_port,ROI_1);
}

void LEP_ROIStatistics(LEP_SYS_SCENE_STATISTICS_T_PTR ROI_Stats,LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR ROI_Stats_1){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_GetSysSceneStatistics(&_port,ROI_Stats);
	LEP_GetRadSpotmeterObjInKelvinX100(&_port, ROI_Stats_1);
}

///////////////////////////////////////////////////////////////////////
////////////////////// Advanced Config //////////////////////////////////
////////////////////////////////////////////////////////////////////

void LEP_SetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry){
	if(!_connected) {
		LEP_Connect();
	}
		

	LEP_SetRadFluxLinearParams(&_port,Radiometry);	
}


void LEP_SetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T gainModeObj){
	if(!_connected) {
		LEP_Connect();
	}
	LEP_SetSysGainModeObj(&_port,gainModeObj);
}

void LEP_SetAGCConfig(LEP_AGC_ROI_T agcROI, LEP_UINT16 agcHeqClipLimitHigh, LEP_UINT16 agcHeqDampingFactor, LEP_UINT16 agcHeqLinearPercent){		
	if(!_connected) {
		LEP_Connect();
	}

	LEP_SetAgcROI( &_port, agcROI);
	LEP_SetAgcHeqDampingFactor( &_port, agcHeqDampingFactor);
	LEP_SetAgcHeqClipLimitHigh( &_port, agcHeqClipLimitHigh);
	LEP_SetAgcHeqLinearPercent( &_port, agcHeqLinearPercent);
}





void LEP_Reboot() {
	if(!_connected) {
		LEP_Connect();
	}
	LEP_RunOemReboot(&_port);
	qDebug()<< "camera restart";
}
