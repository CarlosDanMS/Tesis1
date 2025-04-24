void LEP_SetGainMode(int option);
void LEP_SetShutter(int option);
void LEP_SetROI(LEP_SYS_VIDEO_ROI_T ROI, LEP_RAD_ROI_T ROI_1);
void LEP_ROIStatistics(LEP_SYS_SCENE_STATISTICS_T_PTR ROI_Stats, LEP_RAD_SPOTMETER_OBJ_KELVIN_T_PTR ROI_Stats_1);

void LEP_SetRaw14();

///////////////////////////////////////////////////////////////////////
////////////////////// Advanced Config //////////////////////////////////
////////////////////////////////////////////////////////////////////
void LEP_SetRadParms(LEP_RAD_FLUX_LINEAR_PARAMS_T Radiometry);
void LEP_SetGainConfig(LEP_SYS_GAIN_MODE_OBJ_T gainModeObj);
void LEP_SetAGCConfig(LEP_AGC_ROI_T agcROI, LEP_UINT16 agcHeqClipLimitHigh, LEP_UINT16 agcHeqDampingFactor, LEP_UINT16 agcHeqLinearPercent);

void LEP_Reboot();

#endif
