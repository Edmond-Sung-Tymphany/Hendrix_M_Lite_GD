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

void MainApp_MuteUpdate(cMainApp* const me);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);

void MainApp_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel);

eLedIndID MainApp_GetUpdatedConnLed(cMainApp * const me);

void MainApp_UpdateConnLed(cMainApp * const me);

void MainApp_UpdateProdLed(cMainApp * const me);

void MainApp_TurnOffConnLed(cMainApp * const me);

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID);

void MainApp_SetAudioMode(cMainApp * const me, eAudioMode newAudioMode);

void MainApp_MusicDetectSignalHandler(cMainApp * const me, bool musicOn, eAudioJackId jackId);

void MainApp_DimConnLed(cMainApp * const me);
#ifdef HAS_SPEAKER_ROLE
void MainApp_SetSpeakerRole(cMainApp * const me, eSpeakerRole role);
#endif
#ifdef HAS_SPEAKER_POSITION
void MainApp_SetSpeakerPosition(cMainApp * const me, eSpeakerPosition position);
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_UTIL_H */

