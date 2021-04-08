/**
*  @file      MainApp_util.h
*  @brief     Key event handler of MOFA
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_UTIL_H
#define	MAINAPP_UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif

void MainApp_SendLedReq(cMainApp* me, ePattern patternId);

void MainApp_SwitchMode(cMainApp* me, uint16 modeId);

void MainApp_SetExternalSource(cMainApp * const me);

void MainApp_ResumeVol(cMainApp * const me);

eAudioSource MainApp_GetNextAvialableSource(cMainApp * const me);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_UTIL_H */

