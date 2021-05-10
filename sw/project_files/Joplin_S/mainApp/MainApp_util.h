/**
*  @file      MainApp_util.h
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_UTIL_H
#define MAINAPP_UTIL_H

#ifdef  __cplusplus
extern "C" {
#endif

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID);

void MainApp_SetRotaterLedOn(cMainApp* me, eLed startLed, uint32 level);
void MainApp_SetRotaterLedOff(cMainApp* me, eLed startLed, uint32 level);

void MainApp_DelayedErrorReboot(cMainApp * const me);

void MainApp_SwitchMode(cMainApp* me, uint16 modeId);

void MainApp_ResumeVol(cMainApp * const me);

void MainApp_Mute(cMainApp* me, bool muteEnable2);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);

void MainApp_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel);

void MainApp_InitStatusVariables(cMainApp * const me);

#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_UTIL_H */
