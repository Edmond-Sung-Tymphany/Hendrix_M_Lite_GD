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

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID);

void MainApp_DelayedErrorReboot(cMainApp * const me);

void MainApp_SwitchMode(cMainApp* me, uint16 modeId);

void MainApp_ResumeVol(cMainApp * const me);

void MainApp_Mute(cMainApp* me, bool muteEnable2);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_UTIL_H */

