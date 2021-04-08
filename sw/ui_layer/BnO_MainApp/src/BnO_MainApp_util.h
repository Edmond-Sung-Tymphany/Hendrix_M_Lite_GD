/**
*  @file      BnO_MainApp_util.h
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef BNO_MAINAPP_UTIL_H
#define	BNO_MAINAPP_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

void BnO_MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID);

void BnO_MainApp_SwitchMode(cMainApp* me, uint16 modeId);

void BnO_MainApp_ResumeVol(cMainApp * const me);

eAudioSource BnO_MainApp_GetNextAvialableSource(cMainApp * const me);

void MainApp_Mute(cMainApp* me, bool muteEnable2);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source2);
  
#ifdef	__cplusplus
}
#endif

#endif	/* BNO_MAINAPP_UTIL_H */

