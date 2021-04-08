/**
*  @file      MainApp_util.h
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_UTIL_H
#define	MAINAPP_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "MainApp.h"


typedef enum
{
    BOOT_REQ_NORMAL = 0,
    BOOT_REQ_POWER_UP,    //must power up, do not consider DC
    BOOT_REQ_POWER_DOWN,  //must power down, do not consider Dc
}eBootReq;


  
  
void MainApp_SendLedReq(cMainApp* const me, eLedIndID ledIndID, bool force);
void MainApp_WakeupFromIdle(cMainApp * const me);
void MainApp_UpdateProdLed(cMainApp * const me);
void MainApp_UpdateConnLed(cMainApp * const me);
void MainApp_SwitchMode(cMainApp* const me, uint16 modeId);
void MainApp_ResumeVol(cMainApp* const me);
void MainApp_MuteUpdate(cMainApp* const me);
void MainApp_Set_OverheatError(cMainApp* const me, bool isOverheat);
void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);
void MainApp_VolumeChangeHandler(cMainApp * const me, int8 relativeVol);
void MainApp_SetAbsoluteVolume(cMainApp* const me, int8 absoluteVol);
void MainApp_AudioShutdown(cMainApp* const me, bool enable);
void MainApp_VolumeButtonHandler(cMainApp * const me, int8 relativeVol);
void MainApp_MusicDetectSignalHandler(cMainApp * const me, bool musicOn);
void MainApp_InitStatusVariables(cMainApp * const me);
void MainApp_DelayedErrorReboot(cMainApp * const me);
void MainApp_PrintVersion(cMainApp * const me);


#ifdef DEBUG_DEMO_FEATURE            
void MainApp_SetAudioMode(cMainApp * const me, eAudioMode newAudioMode);
void MainApp_DspBypassEnable(cMainApp * const me, bool enable);
#endif

#ifdef BnO_fs1
void MainApp_SetDcStatus(cMainApp * const me, bool bDcIn);
#endif

void MainApp_WriteBootRequest(cMainApp * const me, eBootReq boot_req);
ePattern MainApp_GetReplyLedPattern(cMainApp * const me, eLedIndID ledIndId);




#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_UTIL_H */

