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

#include "setting_id.h"
#include "MainApp.h"
#include "PowerSrv.h"

typedef struct
{
    ePageSettingId  settingId;
    int16           minVal;
    int16           maxVal;
    int16           defaultVal;
    int16           valPerStep;
    eAudioSettId    dspSettId;
}tMenuDataAttr;

/* Note: the member order of the array should be changed according to eLedIndID in pattern.h */
#define LED_BRIGHTNESS_0    (0)
#define LED_BRIGHTNESS_25   (3)
#define LED_BRIGHTNESS_50   (5)
#define LED_BRIGHTNESS_75   (10)
#define LED_BRIGHTNESS_100  (15)
#define LED_BRIGHTNESS_TOTAL_STEPS  4

#define LP_STATUS_ON        (10)
#define LP_STATUS_OFF       (0)

void MainApp_SendLedReq(cMainApp* me, ledMask mask, Color c);
void MainApp_UpdateLeds(cMainApp* me);
void MainApp_CleanLeds(cMainApp* me);
void MainApp_DimLeds(cMainApp* me);
void MainApp_UpgradingLeds(cMainApp* me);
void MainApp_StandbyLeds(cMainApp* me);
void MainApp_PoweringUpLeds(cMainApp* me);
void MainApp_InitBrightnessList(void);
void MainApp_ValidateData(cMainApp* me);
void MainApp_CalcAndSet(cMainApp * const me, bool plus_minus);
uint8 MainApp_CalcStep(cMainApp * const me);
void MainApp_ResetSettings(cMainApp * const me);
uint8 MainApp_GetSettIndex(ePageSettingId pageSettId);
void MainApp_LoadPreset(cMainApp * const me);
uint8* MainApp_GetFeatures(uint8 *size);
void MainApp_InitGpioLeds(void);
void MainApp_SetGpioLeds(cMainApp * const me, eGPIOId gpioId, bool on);
void MainApp_SwitchMode(cMainApp* me, uint16 modeId);
void MainApp_SetMenuData(cMainApp * const me, ePageSettingId pageSettId, int16 value);
int16 MainApp_GetMenuData(cMainApp * const me, ePageSettingId pageSettId);

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source);
bool MainApp_ResetPage(cMainApp * const me, ePageSettingId pageSettingId);
int16 MainApp_GetMenuDefault(uint8 index);

ePageSettingId MainApp_GetSettPage(uint8 settIndex);

eAudioSettId MainApp_GetDspSettId(cMainApp * const me, ePageSettingId pageSettingId);
bool MainApp_ValidateBleData(QEvt const * const e);
int32 MainApp_GetIdleTimeout(cMainApp * const me);
int32 MainApp_GetStandbyTimeout(cMainApp * const me);
int32 MainApp_GetJackLowTimeout(cMainApp * const me);
QState MainApp_JackHandler(cMainApp * const me, eJackDetType type, int32 param);
QState MainApp_PowerHandler(cMainApp * const me, eDcInSta status);
void MainApp_SetVolume(cMainApp * const me, int16 value);
void MainApp_GetVolume(cMainApp * const me);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_UTIL_H */

