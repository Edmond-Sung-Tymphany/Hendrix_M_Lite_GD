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

bool IsBluetoothPowerOn(uint8_t status);

void MainApp_DelayedErrorReboot(cMainApp * const me);

void MainApp_SwitchMode(cMainApp* me, uint16 modeId);

void MainApp_ResumeVol(cMainApp * const me);

void MainApp_Mute(cMainApp* me, bool muteEnable2);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);

void MainApp_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel);

void MainApp_InitStatusVariables(cMainApp * const me);

void MainApp_ShowVersion(void);

QState MainApp_UpdateBattLedStatus(cMainApp * const me, QEvt const * const e);

void MainApp_SendBattLedReq(cMainApp* me,ledMask baseled,uint8 count, ePattern patternId);

void MainApp_SetVolume(cMainApp * const me, uint8 vol);

void MainApp_SendRespOfMainAppSig(cMainApp * const me, eEvtReturn e );


void MainApp_SendToMainApp(cMainApp * const me, eSignal e);

void MainApp_SetChannel(cMainApp * const me,eAudioChannel channel);


void MainApp_UpdateProdLedStatus(cMainApp * const me, QEvt const * const e);

void MainApp_UpdateAudioChannel(cMainApp * const me,uint8 audiostatus);

void MainApp_SendBTCueCmd(cMainApp * const me,  eBtCmd cmd);


#ifdef HAS_BATTERY
void MainApp_UpdateBTBattIndicator(cMainApp * const me);

uint8 MainaApp_BatteryCapcityToLedLv(uint8 batteryCa);

bool MainApp_IsBatteryAllowCharge(cMainApp * const me);
void MainApp_ChargingEnable(cMainApp * const me);
void MainApp_ChargingDisable(cMainApp * const me);
bool MainApp_IsBatteryRemoved(cMainApp * const me);


#ifdef HAS_SYSTEM_GAIN_CONTROL
void MainApp_AdjustSysGainForDcIn(cMainApp * const me);
void MainApp_AdjustSysGainForDcOut(cMainApp * const me);
#endif


#endif


#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_UTIL_H */

