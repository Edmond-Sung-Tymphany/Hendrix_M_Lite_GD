/**
 * @file        MainApp_priv.h
 * @brief       Main application for iBT150
 * @author      Christopher
 * @date        2014-04-24
 * @copyright   Tymphany Ltd.
 */


#ifndef MAIN_APP_PRIVATE_H
#define MAIN_APP_PRIVATE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "application.h"
#include "LedSrv.h"
#include "product.config"
#include "keySrv.h"
#include "controller.h"
#include "modes.h"
#include "audioSrv.h"
#include "BluetoothSrv.h"
#include "DebugSettSrv.h"

#include "bsp.h"
#include "BluetoothDlg.h"
#include "PowerDlg.h"
#include "MainApp.h"

#define NFC_POWER_ON_PAIR_TIME_MS    2500
#define POWERUP_DELAY_MS_FOR_BT_Q    5000

//#define POWERDOWN_DELAY_MS_FOR_BT_Q  1000

#define EXT_BATT_LED_1 (1<<LED_EXT_BAT1)
#define EXT_BATT_LED_2 (1<<LED_EXT_BAT2)
#define EXT_BATT_LED_3 (1<<LED_EXT_BAT3)
#define EXT_BATT_LED_4 (1<<LED_EXT_BAT4)

#define ALL_INT_BATT_LEDS    ((1<<LED_INT_BAT1) | (1<<LED_INT_BAT2) | (1<<LED_INT_BAT3) | (1<<LED_INT_BAT4))
#define THREE_INT_BATT_LEDS  ((1<<LED_INT_BAT1) | (1<<LED_INT_BAT2) | (1<<LED_INT_BAT3))
#define TWO_INT_BATT_LEDS    ((1<<LED_INT_BAT1) | (1<<LED_INT_BAT2))
#define ONE_INT_BATT_LEDS    (1<<LED_INT_BAT1)

#define ALL_EXT_BATT_LEDS    ((1<<LED_EXT_BAT1) | (1<<LED_EXT_BAT2) | (1<<LED_EXT_BAT3) | (1<<LED_EXT_BAT4))
#define THREE_EXT_BATT_LEDS  ((1<<LED_EXT_BAT1) | (1<<LED_EXT_BAT2) | (1<<LED_EXT_BAT3))
#define TWO_EXT_BATT_LEDS    ((1<<LED_EXT_BAT1) | (1<<LED_EXT_BAT2))
#define ONE_EXT_BATT_LEDS    (1<<LED_EXT_BAT1)

#define ZERO_INT_BATT_LEDS    0
#define FOURTH_INT_BATT_LEDS    (1<<LED_INT_BAT4)
#define THIRD_INT_BATT_LEDS    (1<<LED_INT_BAT3)
#define SECOND_INT_BATT_LEDS    (1<<LED_INT_BAT2)
#define FIRST_INT_BATT_LEDS    (1<<LED_INT_BAT1)

#define ZERO_EXT_BATT_LEDS    0
#define FOURTH_EXT_BATT_LEDS    (1<<LED_EXT_BAT4)
#define THIRD_EXT_BATT_LEDS    (1<<LED_EXT_BAT3)
#define SECOND_EXT_BATT_LEDS    (1<<LED_EXT_BAT2)
#define FIRST_EXT_BATT_LEDS    (1<<LED_EXT_BAT1)

#define POWER_ON_LEDS    ((1<<LED_POWER_WHITE) | (1<<LED_BT) | (1<<LED_EJECT) | (1<<LED_VOL_DOWN) | (1<<LED_VOL_UP) | (1<<LED_PLAY_WHITE))
#define POWER_OFF_LEDS   ((1<<LED_POWER_WHITE) | (1<<LED_EJECT) | (1<<LED_VOL_DOWN) \
                          | (1<<LED_VOL_UP) | (1<<LED_PLAY_WHITE) | (1<<LED_BT))

#define EMERGENCY_MODE_ON_LEDS    ((1<<LED_POWER_WHITE) | (1<<LED_EJECT) | (1<<LED_VOL_DOWN) | (1<<LED_VOL_UP) | (1<<LED_PLAY_WHITE))
#define EMERGENCY_MODE_OFF_LEDS    ((1<<LED_POWER_WHITE) | (1<<LED_BT) | (1<<LED_EJECT) | (1<<LED_VOL_DOWN) | (1<<LED_VOL_UP) | (1<<LED_PLAY_WHITE) | ALL_EXT_BATT_LEDS)
#define EMERGENCY_MODE_FLASH_LEDS    ((1<<LED_POWER_WHITE) | (1<<LED_BT) | (1<<LED_EJECT) | (1<<LED_VOL_DOWN) | (1<<LED_VOL_UP) | (1<<LED_PLAY_WHITE))

#define ENABLE      1
#define DISABLE     0

typedef enum
{
    INT_BATTERY_TYPE,
    EXT_BATTERY_TYPE,
    MAX_BATTERY_TYPE,
}eBatteryType;


/* private state functions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
/* super state for power*/
static QState MainApp_PoweringUp(QActive * const me, QEvt const * const e);
static QState MainApp_PreActive(QActive * const me, QEvt const * const e);
static QState MainApp_Active(QActive * const me, QEvt const * const e);
static QState MainApp_DeActive(QActive * const me, QEvt const * const e);

/* level 2 state for DeActive*/
static QState MainApp_PoweringDown(QActive * const me, QEvt const * const e);
static QState MainApp_Standby(QActive * const me, QEvt const * const e);
static QState MainApp_Sleep(QActive * const me, QEvt const * const e);


/* private  functions */
static void MainApp_SwitchMode(cMainApp* me, uint16 modeId);
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me);
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me);
static void MainApp_EjectExtBattery(cMainApp * const me);

static void MainApp_AudioJackDetect(cMainApp * const me);
static void MainApp_AudioChannelAutoSwitch(cMainApp * const me);

static void MainApp_VolumeDown(cMainApp * const me, uint8 step);
static void MainApp_VolumeUp(cMainApp * const me, uint8 step);
static void MainApp_RequestMuteAudio(cMainApp * const me, bool isMuteOn);

static void MainApp_SendPowerDlgCmd(cMainApp * const me, eEmergencyModeCmd cmd);
static bool MainApp_IsPowerInBatteryMode();
static bool MainApp_IsExtBatteryExisted();
static bool MainApp_IsReadyForSleep();

static void MainApp_UpdateBatteryIndication(cMainApp * const me);
static void MainApp_ShowBatteryIndication(cMainApp * const me, eBatteryType battType,
                                                    uint8 batteryState, bool isCharging);
static bool MainApp_IsExtBattDockEject(cMainApp * const me, eBatteryType battType,
                                                    uint8 localBatteryState, bool isCharging);
static int8 MainApp_GetBatteryLedCount(uint8 batteryState);
static void MainApp_ResetExtBattDockEject(tBatteryInfo battInfo);

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus);

static void MainApp_PlayPauseReset(void);
static void MainApp_PlayPause(cMainApp * const me);

static bool MainApp_IsSystemIdle(cMainApp * const me);
static bool MainApp_IsBtIdle();
static bool MainApp_IsAuxInIdle();
static bool MainApp_IsBatteryOnly();

static void MainApp_StartBtDlg();
static void MainApp_StopBtDlg();

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_EmergencyModeParse(cMainApp * const me, bool isEnable);
static bool MainApp_EmergencyModeQuit(cMainApp * const me);
static void MainApp_EmergencyModeExit(cMainApp * const me);
static void MainApp_EmergencyModeEnter(cMainApp * const me);
static bool MainApp_IsBatteryLowPowerOff(cMainApp * const me);
static void MainApp_UpdateAudioPowerDrcRange(cMainApp * const me);





#ifdef __cplusplus
}
#endif

#endif /* MAIN_APP_PRIVATE_H */
