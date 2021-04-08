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

#include "stm32f0xx.h"
#include "application.h"
#include "LedSrv.h"
#include "product.config"
#include "keySrv.h"
#include "controller.h"
#include "modes.h"
#include "audioSrv.h"
#include "SettingSrv.h"
#include "gpioDrv.h"
#include "IrLearningDlg.h"
#include "Ringbuf.h"
#include "UartDrv.h"
#include "AudioDrv.h"
#include "SettingSrv_light.h"
#include "irLearningDrv.h"
#include "AudioSrv.Config"

#include "bsp.h"
#include "MainApp.h"

#define NFC_POWER_ON_PAIR_TIME_MS    2500
   
#define POWERUP_DELAY_MS_FOR_BT_Q    5000

#define HIJACK_SOURCE_SCAN_TIME      1000
#define SETTING_SAVE_MS             (5000)          // 5s
#define CHECK_MUSIC_VALID_COUNT     (3)
#define CHECK_SILENCE_VALID_COUNT   (8)

//#define SOURCE_LED_DEBUG


//#define POWERDOWN_DELAY_MS_FOR_BT_Q  1000

typedef enum
{
    SYS_MODE_NORMAL,
    SYS_MODE_IR_LEARNING_MODE,
    SYS_MODE_STANDBY,
}eMainAppSysMode;

typedef enum
{
    INPUT_SOURCE_ANALOG,        // Combo jack Line-in
    INPUT_SOURCE_SPDIF0,        // Toslink optical
    INPUT_SOURCE_SPDIF1,        // Combo jack Optical
    INPUT_SOURCE_RJ45,
    NUM_OF_INPUT_SOURCE
}eSourceName;

typedef struct
{
    bool isHasMusic:1;
    bool isVolLocked:1;
    bool isJackIn:1;
    bool isPhysicalConnected:1;
    bool isMuted:1;
    bool WasSilent:1;
    uint8 currentVol;
    uint8 backupVol;
    uint8 HasMusicCnt;
    uint8 isSilenceCnt;
    uint8 saved_vol;

    const eSourceName inputSource:8;
    const eAudioChannel physicalChannel:8;
}tSourceObj;

typedef struct
{
    eMainAppSysMode     SystemMode:8;
    uint16              SwitchSourceDelay;
    uint16              SettingTimeout;
    uint16              doublePressTimeout;
    uint16              hijackSourceScanTimeout;
    uint16              checkDigitalCodeTimeout;
    uint32              autoStandbyTimer;
    uint16              SystemResetTimeout;
    tSourceObj*         pSourceObj;               // pointer to the current source object
    uint8*              pVol;                     // pointer to the master volume
    bool                IsMute;
    bool                PowerOffFlag;
}tMainAppState;


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

static void MainApp_AudioJackDetect(cMainApp * const me);
static void MainApp_AudioChannelAutoSwitch(cMainApp * const me);

static void MainApp_VolumeDown(cMainApp * const me, uint8 step);
static void MainApp_VolumeUp(cMainApp * const me, uint8 step);
static void MainApp_RequestMuteAudio(cMainApp * const me, bool isMuteOn);

static bool MainApp_IsReadyForSleep();

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus);


static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyLPressHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);

static void MainApp_SwitchAudioChannel(cMainApp * const me, eAudioChannel channel);
//static void MainApp_DebugSrvInputSourceCtor();
static void MainApp_RestoreVariablesForPowerUp(cMainApp * const me);

static void MainApp_SwitchSourceKeyEvt(cMainApp * const me,tMainAppState* appState);

static void MainApp_MusicDetectSignalHandler(cMainApp * const me, QEvt const * const e,tMainAppState* appState);
static void MainApp_AudioStateSignalHandler(cMainApp * const me, QEvt const * const e,tMainAppState* appState);
static void MainApp_StandbyPeriodicTask(cMainApp * const me,tMainAppState* appState, uint16 TickTimeInterval);
static void MainApp_TurnOnLED(QActive * const me, tMainAppState* appState);
static void MainApp_HijackSignalInput(cMainApp * const me, eAudioJackId channel);
static bool MainApp_signalValid(cMainApp * const me, eAudioJackId* channel);
static void MainApp_switchToSource(cMainApp * const me,tMainAppState* appState, eAudioJackId AudioChannel);

static bool MainApp_signalSilent(cMainApp * const me, eAudioJackId* channel);
static void MainApp_VolumeLockProcess(cMainApp * const me,tMainAppState* appState);

static bool MainApp_IsCurrentSourceIdle();

#define     PWR_EN_ON(x)           GpioDrv_SetBit(&(x), GPIO_7)
#define     PWR_EN_OFF(x)          GpioDrv_ClearBit(&(x), GPIO_7)

#define     CODEC_RST_HIGH(x)       GpioDrv_SetBit(&(x), GPIO_5)
#define     CODEC_RST_LOW(x)        GpioDrv_ClearBit(&(x), GPIO_5)

#define     RELAY_SW_ON(x)          GpioDrv_SetBit(&(x), GPIO_8)
#define     RELAY_SW_OFF(x)         GpioDrv_ClearBit(&(x), GPIO_8)

#define     OPAMP_MUTE(sw)           sw?GpioDrv_ClearBit(&gpioDrv, GPIO_9):GpioDrv_SetBit(&gpioDrv, GPIO_9)

#define     RJ45_SENSE_OUT(sw)       sw?GpioDrv_SetBit(&gpioDrv, GPIO_10):GpioDrv_ClearBit(&gpioDrv, GPIO_10)


#ifdef __cplusplus
}
#endif

#endif /* MAIN_APP_PRIVATE_H */
