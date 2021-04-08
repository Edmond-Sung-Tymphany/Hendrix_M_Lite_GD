/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  Main Application
                  -------------------------

                  SW Module Document

@file        MainApp.c
@brief       Main application for SVS14_Ultra
@author      Christopher Alexander,Bob.Xu
@date        2014-04-24
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2016-06-30     Viking Wang
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "./MainApp_priv.h"
#include "projBsp.h"
#include "bsp.h"
#include "SettingSrv.h"
#include "trace.h"
#include "devicetypes.h"
#include "AudioSrv.h"
#include "keySrv.h"
#include "DebugSSrv.h"
#ifdef HAS_SYSTEM_CONTROL
#include "SystemDrv.h"
#endif
#ifdef HAS_GPIO_LED
#include "GpioLed.h"
#endif
#ifdef HAS_ADAU1761_DSP
#include "Adau1761_Drv.h"
#endif
#ifdef HAS_BLUETOOTH
#include "BluetoothDrv.h"
#endif
#ifdef HAS_BOOST_ENABLE
#include "AudioDrv.h"
#endif

#define MAIN_APP_TIMEOUT_IN_MS          100
#define MAIN_APP_FACTORY_RESET_START_TIMEOUT_IN_MS      (100)
#define MAIN_APP_FACTORY_RESET_TIMEOUT_IN_MS    (2000)
#define MAIN_APP_SLEEP_DELAY_TIMEOUT_IN_MS          (500)
#define FACTORY_RESET_COMB                  COMB_KEY_ID_0


#define TEN_SECONDS                                 (100)
#define MAINAPP_CRITICAL_REPEAT_TIME        TEN_SECONDS
#define THREE_MINUTES                           (1800)

#define THREE_SECONDS                               (30)
#define FIVE_SECONDS                                (50)
#define MAINAPP_BAT_LOW_AUDIO_CUE_DELAY_TIME        FIVE_SECONDS
#define MAINAPP_BAT_LOW_AUDIO_CUE_TIME_REACHED          (1)
#define MAINAPP_BAT_CRITICAL_AUDIO_CUE_DELAY_TIME   FIVE_SECONDS
#define MAINAPP_BAT_CRITICAL_AUDIO_CUE_TIME_REACHED          (1)
#define MAINAPP_BAT_LED_UPDATE_DELAY_TIME       THREE_SECONDS

#define MAINAPP_RECONNECT_TIMEOUT   (TEN_SECONDS)
#define MAINAPP_PAIRING_TIMEOUT         (THREE_MINUTES)


#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#endif

#define POWER_KEY_DEBOUNCE_CNT  5   /* 500ms debouncing time */
static uint8_t power_switch_cnt = 0;
#define BT_STATUS_DEBOUNCE_CNT  10   /* 1000ms debouncing time */
static uint8_t bt_status_timer = 0;
static bool bt_status_updated = FALSE;
static uint32_t reconnect_pairing_timer = 0;            /* reconnect or pairing timeout timer */
#define DELAY_AUDIO_CUE_VOLUME      10               /* N * 100ms */
static uint8_t  delay_audio_cue_volume_time = 0;

#ifdef POWER_OFF_TO_ON_DEBOUNCE
#define POWER_ON_DEBOUNCE_TIME          (500)
static int16_t power_on_debounce_timeout=0;
#endif

typedef struct
{
    cGpioDrv      ledHwObj;
} led_t;


enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    // the following is for mainapp signal only
    MAINAPP_VOLUME_CTL_SIG = 201,
    MAINAPP_TREBLE_CTL_SIG = 202,
    MAINAPP_BASS_CTL_SIG = 203,
    MAINAPP_LED_CTL_SIG = 204,
    MAINAPP_SOURCE_CTL_SIG = 205,
    MAINAPP_BYPASS_CTL_SIG = 206,
    MAINAPP_BAT_CAPACITY_CTL_SIG = 207,
    MAINAPP_MICLOOP_CTL_SIG = 208,
    MAINAPP_NTC_CTL_SIG = 209,
    MAINAPP_AMP_TEMP_CTL_SIG = 210,
    MAINAPP_MAX_CTL_SIG
};


static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[6];
/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[10];
static uint8_t gpio_key_pressed=GPIO_KEY_RELEASED;
static uint8_t gpio_key_ignore_cnt = 0;


#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
static QState MainApp_Initial(cMainApp * const me, QEvt const *const e);
/* Active state  - super state for "normal" behaviour */
static QState MainApp_Active(cMainApp * const me, QEvt const * const e);
static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);
static void MainApp_AllLedOff(void);
static void MainApp_FactoryReset_Led(void);
static void MainApp_BT_LED_Handler(cMainApp * const me, QEvt const * const e);
static void MainApp_BatteryStatusUpdate(cMainApp * const me, QEvt const * const e);
static void MainApp_TemperatureMonitor(cMainApp * const me, QEvt const * const e);
static QState MainApp_DeActive(cMainApp * const me, QEvt const * const e);
static QState MainApp_Standby(cMainApp * const me, QEvt const * const e);
static QState MainApp_Sleep(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e);
static void MainApp_PowerOff(cMainApp * const me, QEvt const * const e);
static void MainApp_SwitchMode(cMainApp* me, uint16 modeId);
static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e);
#ifdef SHORT_PRESS_PAIRING
static void MainApp_ProcBtPairing(cMainApp * const me, QEvt const * const e);
#endif
static void MainApp_ParseBTEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_PostBTEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_ParseAUXEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_BoostUpdate(cMainApp * const me, QEvt const * const e);
static void MainApp_CheckBatteryFullyCharged(cMainApp * const me, QEvt const * const e);
static void MainApp_ExternalChargingUpdate(cMainApp * const me, QEvt const * const e);
static void MainApp_SendBTCmdWithAudioCue(cMainApp * const me, QEvt const * const e, eBtCmd bt_cmd);
static void MainApp_RecoverChargingLED(void);
#ifdef UNIT_TEST_ENABLE
static void MainStatus_UnitTest(uint16 level);
#endif

/* Pairing Data List Status */
typedef enum
{
    EMPTY_PDL,
    NOT_EMPTY_PDL
} PDLStatus_t;

typedef enum
{
    AUX_UNPLUGGED,
    AUX_PLUGGED
} AUXStatus_t;

typedef enum
{
    BAT_NORMAL,
    BAT_LOW,
    BAT_CRITICAL,
    BAT_URGENT,
    BAT_FULL
} BATStatus_t;

typedef enum
{
    AMP_OUTPUT_VERY_LOW,
    AMP_OUTPUT_LOW,
    AMP_OUTPUT_MEDIUM,
    AMP_OUTPUT_HIGH
} AMP_OUTPUT_t;

typedef struct
{
    uint32_t CurrBTStatus;
    uint32_t PrevBTStatus;
    uint32_t CurrSource;
    uint32_t PrevSource;
    uint8 isAuxPlugged;
    uint32_t pdlStatus;
    uint8 isDCPlugged;
    uint8 batteryStatus;
    uint8 prevbatteryStatus;
    uint8 criticalLoop;
    uint8 isPowerSwitchOn;
    bool btLEDtrigger;
    uint8 CurrVolume;
    bool isAudioCuePlaying;
    bool isConnectedAudioCuePlayed;
    bool isBatteryError;
    bool isNTCAlert;
    bool isNTCTurnOffChargingRequest;
    bool isReconnectingOrPairing;
} MainStatus_t;

static MainStatus_t MainStatus;


const static uint32_t source_list[MAINAPP_SOURCE_MAX+1] =
{
    MAINAPP_SOURCE_AUX,
    MAINAPP_SOURCE_BT,
    MAINAPP_SOURCE_AUX  // this is default source
};

static void MainApp_GLED_SetupMode(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt)
{
    if ((FALSE == MainStatus.isBatteryError)&&(FALSE == MainStatus.isNTCAlert))
    {
        GLED_SetupMode(gled_id, mode, repeat_cnt);
    }
}

static void MainApp_ShowVersion(void)
{
    TP_PRINTF("\n\r-------------------------------------------------------\n\r");
    TP_PRINTF("\n\r Fender Newport MCU:V%s, DSP:V%s.\n\r", PRODUCT_VERSION_MCU, PRODUCT_DSP_VERSION);
    TP_PRINTF("-------------------------------------------------------\n\r");
}

static void MainApp_Version2TPMonitor(void)
{
    char ver_msg[24];
#ifdef HAS_HW_VERSION_TAG
    char hw_msg[][8]= {"Unknown", "ES1", "ES2", "ES3", "EVT1", "EVT2", "DVT1", "DVT2", "PVT", "MP1", "MP2"};
    HwVersion_t hw_ver;
    hw_ver = SystemDrv_GetHWversion();
    sprintf(ver_msg, "HW Ver:%s", hw_msg[hw_ver]);
    DebugSSrv_Printf(ver_msg);
#endif
    sprintf(ver_msg, "MCU:V%s.DSP:V%s", PRODUCT_VERSION_MCU, PRODUCT_DSP_VERSION);
    DebugSSrv_Printf(ver_msg);
}

static void MainApp_BatteryCapacityRequest(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char batt_msg[24];
    value = SystemDrv_getBatteryStateOfCharge();
    sprintf(batt_msg, "Capacity=%d", value);
    DebugSSrv_Printf(batt_msg);
}

static void MainApp_NTCRequest(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char NTC_msg[24];
    value = SystemDrv_GetNTCValue();
    sprintf(NTC_msg, "NTC=%d", value);
    DebugSSrv_Printf(NTC_msg);
}

static void MainApp_AmpTemperatureRequest(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char msg[24];
    value = AudioDrv_GetAmpTemperature();
    sprintf(msg, "Amp Temp=%d", value);
    DebugSSrv_Printf(msg);
}

static void MainApp_VolumeCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;
    if( value < MAX_VOLUME_STEPS )
    {
        AudioSrv_SetVolume(value);
        Setting_Set(SETID_VOLUME, &value);
        sprintf(vol_msg, "Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

static void MainApp_TrebleCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;

    if( value < MAX_TREBLE_STEPS )
    {
        AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, value, 0);
        Setting_Set(SETID_TREBLE, &value);
        sprintf(vol_msg, "Treble=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Treble=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

static void MainApp_BassCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;

    if( value < MAX_BASS_STEPS )
    {
        AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, value, 0);
        Setting_Set(SETID_BASS, &value);
        sprintf(vol_msg, "Bass=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Bass=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

/* LED define for TP monitor,
 * the jason file should edit according the following define
 */
/*
#define LED_AUX_ON        1
#define LED_POWER_ON      2
#define LED_RCA_ON        4
#define LED_BT_ON         8
*/
static void MainApp_LedCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint16_t ctl_mode, ctl_value;
    uint32_t value, led_mask;
    GLED_ID_t id;

    value = p_ctl_evt->value;
    ctl_mode = (uint16_t)(value & 0xffff);
    ctl_value = (uint16_t)((value>>16) & 0xffff);

    // if ctl_mode > 255, we use ctr_value to control
    if( ctl_mode & 0xff00 )
    {
        led_mask = (uint32_t)ctl_value;
    }
    else
    {
        led_mask = (uint32_t)ctl_mode;
    }

    for(id=GLED_ID_START; id<GLED_ID_END; id++)
    {
        if( led_mask & 0x01 )
            MainApp_GLED_SetupMode(id, GLED_MODE_ON, 0);
        else
            MainApp_GLED_SetupMode(id, GLED_MODE_OFF, 0);
        led_mask >>= 1;
    }
}

static void MainApp_SourceCtlSig(cMainApp * const me, MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value, i;
    value = p_ctl_evt->value;

    switch( value )
    {
        case MAINAPP_SOURCE_AUX:
        case MAINAPP_SOURCE_BT:
            for(i=0; i<MAINAPP_SOURCE_MAX; i++)
            {
                if( value == source_list[i] )
                {
                    break;
                }
            }
            MainStatus.CurrSource = source_list[i];
            Setting_Set(SETID_AUDIO_SOURCE, (uint32_t *)&MainStatus.CurrSource);
            AudioSrv_SetChannel((QActive *)me, (eAudioChannel)MainStatus.CurrSource);
            MainStatus.btLEDtrigger = FALSE;
            break;
        default:    // error
            DebugSSrv_Printf("ERROR Source.");
            break;
    }
}

static void MainApp_BypassCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    bool enable;

    if( p_ctl_evt->value )
    {
        enable = TRUE;
        DebugSSrv_Printf("Bypass ENABLE");
    }
    else
    {
        enable = FALSE;
        DebugSSrv_Printf("Bypass Disable");
    }

    AudioSrv_SetAudio(DSP_PASSTHROUGH_SETT_ID, enable, 0, 0);
}

static void MainApp_MicLoopBackCtlSig(cMainApp * const me, MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;

    /*
        if( p_ctl_evt->value )
        {
            enable = TRUE;
            DebugSSrv_Printf("TestMode ENABLE");
        }
        else
        {
            enable = FALSE;
            DebugSSrv_Printf("TestMode Disable");
        }
    */
    /*    AudioSrv_SetAudio(DSP_PLAINEQ_SETT_ID, enable, 0, 0);*/

    BluetoothSrv_SendBtCmd((QActive*)me, BT_TEST_MODE_CMD);
    AudioSrv_SetChannel((QActive *)me, /*MAINAPP_SOURCE_AUX*/MAINAPP_SOURCE_BT);
    if( value < MAX_VOLUME_STEPS )
    {
        AudioSrv_SetVolume(value);
        Setting_Set(SETID_VOLUME, &value);
        sprintf(vol_msg, "Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }

}

static uint32_t MainApp_GetSource(void)
{
    uint32_t audio_channel;
    uint32_t i;

    audio_channel = *(uint32_t*)Setting_Get(SETID_AUDIO_SOURCE);

    for(i=0; i<MAINAPP_SOURCE_MAX; i++)
    {
        if( audio_channel == source_list[i] )
            break;
    }

    if( i == MAINAPP_SOURCE_MAX )
    {
        // source error, set to default.
        audio_channel = source_list[i];
        Setting_Set(SETID_AUDIO_SOURCE, (uint32_t *)&audio_channel);
    }

    return audio_channel;
}

static void MainApp_InitStatus(cMainApp * const me)
{
    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
    MainStatus.PrevSource = MAINAPP_SOURCE_BT;
    MainStatus.CurrBTStatus = BT_OFF_STA;
    MainStatus.isAuxPlugged = AUX_UNPLUGGED;
    MainStatus.isDCPlugged = DC_IN_UNKNOWN;
    MainStatus.batteryStatus = BAT_NORMAL;
    MainStatus.prevbatteryStatus = BAT_LOW;
    MainStatus.criticalLoop = 0;
    MainStatus.isPowerSwitchOn = POWER_SWITCH_OFF;
    MainStatus.btLEDtrigger = FALSE;
    MainStatus.CurrVolume = DEFAULT_VOLUME;
    MainStatus.isAudioCuePlaying = FALSE;
    MainStatus.isConnectedAudioCuePlayed = FALSE;
    MainStatus.isBatteryError = FALSE;
    MainStatus.isNTCAlert = FALSE;
    MainStatus.isNTCTurnOffChargingRequest = FALSE;
    MainStatus.isReconnectingOrPairing = FALSE;
}

static void MainApp_AllLedOff(void)
{
    /* immediate turn off these three LED */
    GLED_ALLLEDOff();

    MainApp_GLED_SetupMode(GLED_ID_POWER, GLED_MODE_OFF, 0);
    MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
    MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_OFF, 0);
}

static void MainApp_FactoryReset_Led(void)
{
    MainApp_GLED_SetupMode(GLED_ID_POWER, GLED_MODE_ON, 0);
    MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_ON, 0);
    MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_ON, 0);
}

static void MainApp_BoostUpdate(cMainApp * const me, QEvt const * const e)
{
    if (DC_IN_NOT_DETECTED == MainStatus.isDCPlugged)
    {
        SystemDrv_BoostEnable(TRUE);
    }
    else if (DC_IN_DETECTED == MainStatus.isDCPlugged)
    {
        SystemDrv_BoostEnable(FALSE);
    }
    else
    {
    }
}

static void MainApp_BT_LED_Handler(cMainApp * const me, QEvt const * const e)
{
    if (!MainStatus.btLEDtrigger)
        return;

    MainStatus.btLEDtrigger = FALSE;

    Setting_Set(SETID_AUDIO_SOURCE, &MainStatus.CurrSource);
    AudioSrv_SetChannel((QActive *)me, (eAudioChannel)MainStatus.CurrSource);

    if (MAINAPP_SOURCE_AUX == MainStatus.CurrSource)
    {
        MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
        MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_OFF, 0);
    }
    else
    {
        switch (MainStatus.CurrBTStatus)
        {
            case BT_RECONNECTING_STA:
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                MainStatus.isReconnectingOrPairing = TRUE;
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_DOUBLE_FLASH, GPIO_LED_ALWAYS_FLASH);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
                break;

            case BT_CONNECTABLE_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_DOUBLE_FLASH, GPIO_LED_ALWAYS_FLASH);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
                break;

            case BT_CONNECTED_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
                break;

            case BT_DISCOVERABLE_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
                break;

            case BT_STREAMING_A2DP_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
                break;

            case BT_ACTIVE_CALL_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_ON, 0);
                break;

            case BT_INCOMING_CALL_EST_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
                break;

            case BT_OUTGOING_CALL_EST_STA:
                MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_ON, 0);
                MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
                break;

            default:
                break;
        }
    }
}


static void MainApp_ExternalChargingUpdate(cMainApp * const me, QEvt const * const e)
{
    switch (MainStatus.batteryStatus)
    {
        case BAT_CRITICAL:
        case BAT_LOW:
            SystemDrv_ExternalChargeEnable(FALSE);
            break;

        case BAT_FULL:
        case BAT_NORMAL:
            if (MainStatus.isNTCTurnOffChargingRequest == FALSE)
            {
                SystemDrv_ExternalChargeEnable(TRUE);
            }
            break;

        default:
            break;
    }
}

static void MainApp_SendBTCmdWithAudioCue(cMainApp * const me, QEvt const * const e, eBtCmd bt_cmd)
{
    switch (bt_cmd)
    {
        case BT_BAT_LOW_CMD:
        case BT_BAT_CRITICAL_CMD:
            MainStatus.isAudioCuePlaying = TRUE;
            BluetoothSrv_SendBtCmd((QActive*)me, bt_cmd);
            /* since 0 volume need more from mute to default volume */
            if (MainStatus.CurrVolume == 0)
            {
                MainStatus.PrevSource = MainStatus.CurrSource;
                AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
                AudioSrv_SetChannel((QActive *)me, MAINAPP_SOURCE_BT);
            }
            break;

        case BT_ENTER_PAIRING_CMD:
            MainStatus.isAudioCuePlaying = TRUE;
            BluetoothSrv_SendBtCmd((QActive*)me, bt_cmd);
            MainStatus.PrevSource = MainStatus.CurrSource;
            delay_audio_cue_volume_time = DELAY_AUDIO_CUE_VOLUME;
            /*AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);*/
            AudioSrv_SetChannel((QActive *)me, MAINAPP_SOURCE_BT);
            break;

        case BT_CONNECTED_CMD:
            /* ignore if aux source */
            if (MAINAPP_SOURCE_BT == MainStatus.CurrSource)
            {
                MainStatus.isAudioCuePlaying = TRUE;
                BluetoothSrv_SendBtCmd((QActive*)me, bt_cmd);
                MainStatus.PrevSource = MainStatus.CurrSource;
                AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
                AudioSrv_SetChannel((QActive *)me, MAINAPP_SOURCE_BT);
            }
            break;

        default:
            break;
    }
}

/* Monitor temperature on charging */
static void MainApp_TemperatureMonitor(cMainApp * const me, QEvt const * const e)
{
    if (SystemDrv_isNTCTurnOffCharging())
    {
        if (MainStatus.isNTCTurnOffChargingRequest == FALSE)
        {
            MainStatus.isNTCTurnOffChargingRequest = TRUE;
            SystemDrv_BatteryChargeEnable(FALSE);
            SystemDrv_ExternalChargeEnable(FALSE);
        }
        GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
    }
    else
    {
        if (MainStatus.isNTCTurnOffChargingRequest == TRUE)
        {
            MainStatus.isNTCTurnOffChargingRequest = FALSE;
            SystemDrv_BatteryChargeEnable(TRUE);
            SystemDrv_ExternalChargeEnable(TRUE);
            MainApp_RecoverChargingLED();
        }
    }
}


//#define HAS_CAPACITY_PRINTOUT

static void MainApp_BatteryStatusUpdate(cMainApp * const me, QEvt const * const e)
{
#ifdef HAS_CAPACITY_PRINTOUT
    static uint16 loop_counter=0;
#endif
    uint32 capacity;
    static bool triggerBatteryStatus = FALSE;
    static uint32 delayTime;
    if (((DC_IN_UNKNOWN == MainStatus.isDCPlugged)||(DC_IN_NOT_DETECTED == MainStatus.isDCPlugged))
        &&(DC_IN_DETECTED == SystemDrv_IsDCInDetected()))
    {
        MainStatus.isDCPlugged = DC_IN_DETECTED;
#ifdef HAS_BAT_CHARGE
        SystemDrv_BatteryChargeEnable(TRUE);
#endif
        delayTime = MAINAPP_BAT_LED_UPDATE_DELAY_TIME;
        triggerBatteryStatus = TRUE;
    }
    else if (((DC_IN_UNKNOWN == MainStatus.isDCPlugged)||(DC_IN_DETECTED == MainStatus.isDCPlugged))
             &&(DC_IN_NOT_DETECTED == SystemDrv_IsDCInDetected()))
    {
        MainStatus.isDCPlugged = DC_IN_NOT_DETECTED;
#ifdef HAS_BAT_CHARGE
        SystemDrv_BatteryChargeEnable(FALSE);
#endif
        delayTime = MAINAPP_BAT_LED_UPDATE_DELAY_TIME;
        triggerBatteryStatus = TRUE;
    }

    capacity = SystemDrv_getBatteryStateOfCharge();

#ifdef HAS_CAPACITY_PRINTOUT
    loop_counter++;
    if (loop_counter > 10)
    {
        loop_counter = 0;
        TP_PRINTF("capacity = %d\n", capacity);
    }
#endif

    if (capacity < MAINAPP_BATTERY_URGENT_LEVEL)
    {
        MainStatus.batteryStatus = BAT_URGENT;
    }
    else if (capacity <= MAINAPP_BATTERY_CRITICAL_LEVEL)
    {
        MainStatus.batteryStatus = BAT_CRITICAL;
    }
    else if (capacity <= MAINAPP_BATTERY_LOW_LEVEL)
    {
        MainStatus.batteryStatus = BAT_LOW;
    }
    else if (capacity >= MAINAPP_BATTERY_FULL_LEVEL)
    {
        MainStatus.batteryStatus = BAT_FULL;
    }
    else
    {
        MainStatus.batteryStatus = BAT_NORMAL;
    }

    /* battery status changed? */
    if (MainStatus.batteryStatus != MainStatus.prevbatteryStatus)
    {
        MainStatus.prevbatteryStatus = MainStatus.batteryStatus;
        delayTime = MAINAPP_BAT_LED_UPDATE_DELAY_TIME;
        triggerBatteryStatus = TRUE;
    }

    /* if battery status changed..... "NOTE: plug/unplug DC also triggerBatteryStatus" */
    if (triggerBatteryStatus)
    {
        if (delayTime)
        {
            delayTime--;
            if (delayTime == 0)
            {
                triggerBatteryStatus = FALSE;
            }
            else
            {
                return;
            }
        }

        switch (MainStatus.batteryStatus)
        {
            case BAT_FULL:
                if (DC_IN_DETECTED == MainStatus.isDCPlugged)
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_ON, 0);
                }
                else
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                    /* change Amp output power */
                    AudioDrv_SetAmpOutput(AMP_OUTPUT_HIGH);
                }
                break;

            case BAT_URGENT:
                if (DC_IN_DETECTED == MainStatus.isDCPlugged)
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_SLOW_FLASH, GPIO_LED_ALWAYS_FLASH);
                }
                else
                {
                    /* No matter Switch off or standby (Switch at ON), Urgent go to standby and must be OFF if no charging */
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                    /* change Amp output power */
                    AudioDrv_SetAmpOutput(AMP_OUTPUT_LOW);

                }
                break;

            case BAT_CRITICAL:
                if (DC_IN_DETECTED == MainStatus.isDCPlugged)
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_SLOW_FLASH, GPIO_LED_ALWAYS_FLASH);
                }
                else
                {
                    MainStatus.criticalLoop = MAINAPP_BAT_CRITICAL_AUDIO_CUE_DELAY_TIME;
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_QUICK_FLASH, GPIO_LED_ALWAYS_FLASH);
                    /* change Amp output power */
                    AudioDrv_SetAmpOutput(AMP_OUTPUT_LOW);
                }
                break;

            case BAT_LOW:
                if (DC_IN_DETECTED == MainStatus.isDCPlugged)
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_SLOW_FLASH, GPIO_LED_ALWAYS_FLASH);
                }
                else
                {
                    MainStatus.criticalLoop = MAINAPP_BAT_LOW_AUDIO_CUE_DELAY_TIME;
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
                    /* change Amp output power */
                    AudioDrv_SetAmpOutput(AMP_OUTPUT_MEDIUM);
                }
                break;

            case BAT_NORMAL:
                if (DC_IN_DETECTED == MainStatus.isDCPlugged)
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_SLOW_FLASH, GPIO_LED_ALWAYS_FLASH);
                }
                else
                {
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                    /* change Amp output power */
                    AudioDrv_SetAmpOutput(AMP_OUTPUT_HIGH);

                }
                break;

            default:
                break;
        }
    }

    /* critical battery repeat audio cue */
    if (DC_IN_NOT_DETECTED == MainStatus.isDCPlugged)
    {
        /* delay "Critical" audio cue */
        if (BAT_CRITICAL == MainStatus.batteryStatus)
        {
            if (MainStatus.criticalLoop == 0)
            {
                //ignore
            }
            else
            {
                if (MainStatus.criticalLoop == MAINAPP_BAT_CRITICAL_AUDIO_CUE_TIME_REACHED)
                {
                    MainApp_SendBTCmdWithAudioCue(me, e, BT_BAT_CRITICAL_CMD);
                }
                MainStatus.criticalLoop--;
            }
        }

        /* delay battery low audio cue */
        if (BAT_LOW == MainStatus.batteryStatus)
        {
            if (MainStatus.criticalLoop == 0)
            {
                //ignore
            }
            else
            {
                if (MainStatus.criticalLoop == MAINAPP_BAT_LOW_AUDIO_CUE_TIME_REACHED)
                {
                    MainApp_SendBTCmdWithAudioCue(me, e, BT_BAT_LOW_CMD);
                }
                MainStatus.criticalLoop--;
            }
        }
    }

    if (DC_IN_DETECTED == MainStatus.isDCPlugged)
    {
        /* also check if battery error when charging, e.g. missing battery or battery dead, etc */
        if (SystemDrv_isBatteryError())
        {
            if (MainStatus.isBatteryError == FALSE)
            {
                MainStatus.isBatteryError = TRUE;
                GLED_ErrorMode();
            }
        }
        else
        {
            if (MainStatus.isBatteryError == TRUE)
            {
                GLED_RecoverMode();
                MainApp_RecoverChargingLED();
            }
            MainStatus.isBatteryError = FALSE;
        }
    }

    if (MainStatus.isBatteryError == FALSE)
    {
        if (SystemDrv_isNTCAlert())
        {
            if (MainStatus.isNTCAlert == FALSE)
            {
                MainStatus.isNTCAlert = TRUE;
                GLED_ErrorMode();
            }
        }
        else
        {
            if (MainStatus.isNTCAlert == TRUE)
            {
                GLED_RecoverMode();
                MainApp_RecoverChargingLED();
            }
            MainStatus.isNTCAlert = FALSE;
        }
    }
}


static void MainApp_RecoverChargingLED(void)
{
    if (DC_IN_DETECTED == MainStatus.isDCPlugged)
    {
        if (BAT_FULL == MainStatus.batteryStatus)
        {
            GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_ON, 0);
        }
        else
        {
            GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_SLOW_FLASH, GPIO_LED_ALWAYS_FLASH);
        }
    }
    else
    {
        switch (MainStatus.batteryStatus)
        {
            case BAT_NORMAL:
            case BAT_FULL:
                GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                break;
            case BAT_URGENT:
                /* No matter Switch off or standby (Switch at ON), Urgent go to standby and must be OFF if no charging */
                GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                break;
            case BAT_CRITICAL:
                GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_QUICK_FLASH, GPIO_LED_ALWAYS_FLASH);
                break;
            case BAT_LOW:
                GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
                break;
            default:
                break;
        }
    }
}

static void MainApp_CheckBatteryFullyCharged(cMainApp * const me, QEvt const * const e)
{
    if ((DC_IN_DETECTED == MainStatus.isDCPlugged)&&
        SystemDrv_getBatteryStateOfCharge() >= MAINAPP_BATTERY_FULL_LEVEL)
    {
        MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
    }
}

static void MainApp_PowerupInit(cMainApp * const me)
{
    BRINGUP_printf("\n\r System power up.\n\r");
    BRINGUP_printf("\n\r cur source = %d.\n\r", MainStatus.CurrSource);

    Setting_Set(SETID_BT_STATUS, &MainStatus.CurrBTStatus);
}

#ifdef SHORT_PRESS_PAIRING
static void MainApp_ProcBtPairing(cMainApp * const me, QEvt const * const e)
{
    AudioDrv_ResetStandbyCounter();
    /* filter any long press at call activity status */
    if (!(((BT_OUTGOING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
           (BT_INCOMING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
           (BT_DISCOVERABLE_STA == MainStatus.CurrBTStatus) ||
           (BT_ACTIVE_CALL_STA == MainStatus.CurrBTStatus))))
    {
        MainStatus.CurrSource = MAINAPP_SOURCE_BT;
        MainStatus.btLEDtrigger = TRUE;
        BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
        MainApp_SendBTCmdWithAudioCue(me, e, BT_ENTER_PAIRING_CMD);
        MainStatus.isReconnectingOrPairing = TRUE;
        reconnect_pairing_timer = MAINAPP_PAIRING_TIMEOUT;
    }
}
#endif

static void MainApp_ProcBtKey(cMainApp * const me, QEvt const * const e)
{
    BRINGUP_printf("\n\r BT key pressed. \n\r");
    uint32_t btStatus = MainStatus.CurrBTStatus;
    KeyStateEvt *evt = (KeyStateEvt*)e;

    switch (evt ->keyEvent)
    {
        case KEY_EVT_SHORT_PRESS:

#ifdef UNIT_TEST_ENABLE
            /* Test */
            MainStatus_UnitTest(1);
#endif

            AudioDrv_ResetStandbyCounter();
            if (MAINAPP_SOURCE_AUX == MainStatus.CurrSource)
            {
                /* maybe still in A2DP streaming status because of iOS not so quick to change status, need 7 sec to change from A2DP streaming to connected status */
                if ((BT_CONNECTED_STA == btStatus) || (BT_STREAMING_A2DP_STA == btStatus))
                {
                    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                    /* auto issue avrcp_play */
                    //BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PLAY_CMD);
                    MainStatus.btLEDtrigger = TRUE;
                }
                else if ((BT_CONNECTABLE_STA == btStatus)||(BT_OFF_STA == btStatus)||(BT_RECONNECTING_STA == btStatus))
                {
                    if (NOT_EMPTY_PDL == MainStatus.pdlStatus)
                    {
                        MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                        /* reconnect */
                        MainStatus.CurrBTStatus = BT_RECONNECTING_STA;
                        MainStatus.btLEDtrigger = TRUE;
                    }
                    else
                    {
                        /* if PDL empty, do nothing */
                    }
                }
                else if (BT_DISCOVERABLE_STA == MainStatus.CurrBTStatus)
                {
                    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                    MainStatus.btLEDtrigger = TRUE;
                }
            }
            else        /* MAINAPP_SOURCE_BT */
            {
                if ((BT_OUTGOING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
                    (BT_INCOMING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
                    (BT_ACTIVE_CALL_STA == MainStatus.CurrBTStatus))
                {
                    /* ignore the short press if call state */
                }
                else
                {
                    if (AUX_PLUGGED == MainStatus.isAuxPlugged)
                    {
                        MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
                        MainStatus.btLEDtrigger = TRUE;
                        if (BT_STREAMING_A2DP_STA == btStatus)
                        {
                            /* auto issue avrcp_pause */
                            BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                        }
                    }
                    else
                    {
                        if (BT_CONNECTABLE_STA == btStatus)
                        {
                            if (NOT_EMPTY_PDL == MainStatus.pdlStatus)
                            {
                                MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                                /* reconnect */
                                MainStatus.CurrBTStatus = BT_RECONNECTING_STA;
                                MainStatus.btLEDtrigger = TRUE;
                            }
                            else
                            {
                                /* if PDL empty, do nothing */
                            }
                        }
                    }
                }
            }
            break;

        case KEY_EVT_HOLD:
#ifdef UNIT_TEST_ENABLE
            /* Test */
            MainStatus_UnitTest(2);
#endif

            AudioDrv_ResetStandbyCounter();
            /* filter any long press at call activity status */
            if (!(((BT_OUTGOING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
                   (BT_INCOMING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
                   (BT_ACTIVE_CALL_STA == MainStatus.CurrBTStatus))))
            {
                MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                MainStatus.btLEDtrigger = TRUE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                MainApp_SendBTCmdWithAudioCue(me, e, BT_ENTER_PAIRING_CMD);
            }
            break;

        default:
            break;
    }
}

static void MainApp_ProcTalkKey(cMainApp * const me, QEvt const * const e)
{
    BRINGUP_printf("\n\r Talk key pressed. \n\r");

    uint32_t btStatus = MainStatus.CurrBTStatus;
    KeyStateEvt *evt = (KeyStateEvt*)e;

    switch (evt->keyEvent)
    {
        case KEY_EVT_SHORT_PRESS:
#ifdef UNIT_TEST_ENABLE
            /* Test */
            MainStatus_UnitTest(3);
#endif

            AudioDrv_ResetStandbyCounter();
            if (BT_INCOMING_CALL_EST_STA == btStatus)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ANSWER_CMD);
            }
            else if (BT_ACTIVE_CALL_STA == btStatus)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_CANCEL_END_CMD);
            }
            break;

        case KEY_EVT_HOLD:
#ifdef UNIT_TEST_ENABLE
            /* Test */
            MainStatus_UnitTest(4);
#endif

            AudioDrv_ResetStandbyCounter();
            if (BT_INCOMING_CALL_EST_STA == btStatus)
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_REJECT_CMD);
            }
            break;

        default:
            break;
    }
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(MainApp);

    /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                     MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);

    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
    /* Subscribe */
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
#endif
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*) me, BT_STATE_SIG);
    QActive_subscribe((QActive*) me, BT_RESP_SIG);
#endif
}

void MainApp_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Application_Xtor((cApplication*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState MainApp_Initial(cMainApp * const me, QEvt const *const e)
{
    CAST_ME;
    MainApp_InitStatus(me);
    MainApp_GLED_SetupMode(GLED_ID_POWER, GLED_MODE_OFF, 0);
    MainApp_GLED_SetupMode(GLED_ID_TALK, GLED_MODE_OFF, 0);
    MainApp_GLED_SetupMode(GLED_ID_BT, GLED_MODE_OFF, 0);
    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
    SetSender((QActive*)me); // provide sender pointer for getting feedback by ENERGYSENSE_SIG and POWER_SWITCH_SIG
    SystemDrv_SetSystemStatus(SYSTEM_STATUS_STANDBY);
    MainApp_SwitchMode(MainApp, SLEEP_MODE);
    return Q_TRAN(&MainApp_DeActive);
}

static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            /* reset status  */
            MainStatus.CurrBTStatus = BT_OFF_STA;
            MainStatus.isDCPlugged = DC_IN_UNKNOWN;
            gpio_key_pressed = GPIO_KEY_RELEASED;
            /* ---------- */
#ifdef POWER_OFF_TO_ON_DEBOUNCE
            power_on_debounce_timeout = 0;
#endif
            MainApp_GLED_SetupMode(GLED_ID_POWER, GLED_MODE_ON, 0);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);

            /* add this in case sudden turn off during power up audio cue */
#ifdef HAS_POWER_SWITCH_KEY
            MainStatus.isPowerSwitchOn = SystemDrv_IsPowerSwitchOn();
            if(POWER_SWITCH_OFF == MainStatus.isPowerSwitchOn)
            {
                power_switch_cnt++;
                if (power_switch_cnt > POWER_KEY_DEBOUNCE_CNT)
                {
#ifdef POWER_OFF_TO_ON_DEBOUNCE
                    power_on_debounce_timeout = POWER_ON_DEBOUNCE_TIME;
#endif
                    power_switch_cnt = 0;
                    if ((DC_IN_NOT_DETECTED == MainStatus.isDCPlugged)||
                        (DC_IN_UNKNOWN == MainStatus.isDCPlugged))
                    {
                        SystemDrv_PowerEnable(FALSE);
                        return Q_HANDLED();
                    }
                    else
                    {
                        AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_SOFT_MUTE, TRUE);
                        MainApp_SwitchMode(MainApp, SLEEP_MODE);
                        AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
                        BSP_BlockingDelayMs(100);
#ifdef  HAS_DSP_EN
                        SystemDrv_DSPEnable(FALSE);
#endif
                        return Q_TRAN(&MainApp_DeActive);
                    }
                }
                else
                    power_switch_cnt = 0;
            }
#endif

            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            MainApp_SwitchMode(MainApp, NORMAL_MODE);
            return Q_TRAN(&MainApp_Active);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state  - super state for "normal" behaviour */
static QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
            gpio_key_pressed = GPIO_KEY_RELEASED;
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_WORKING);
            MainApp_PowerupInit(me);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            while (QActive_recall((QActive*)me, &deferredReqQue))
            {} // check if we have events(key_state_sig) in deferred queue
            break;
        case MAINAPP_TIMEOUT_SIG:
            if (bt_status_updated == TRUE)
            {
                bt_status_timer++;
                if (bt_status_timer > BT_STATUS_DEBOUNCE_CNT)
                {
                    bt_status_updated = FALSE;
                    bt_status_timer = 0;
                    MainApp_PostBTEvent(me, e);
                    MainApp_BT_LED_Handler(me, e);
                }
            }

            if (delay_audio_cue_volume_time != 0)
            {
                delay_audio_cue_volume_time--;
                if (delay_audio_cue_volume_time == 0)
                {
                    AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
                }
            }

            MainApp_ParseAUXEvent(me, e);
            MainApp_BT_LED_Handler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
#ifdef SHORT_PRESS_PAIRING
            if (MainStatus.isReconnectingOrPairing == TRUE)
            {
                if (reconnect_pairing_timer != 0)
                {
                    reconnect_pairing_timer--;
                    if (reconnect_pairing_timer == 0)
                    {
                        MainStatus.isReconnectingOrPairing = FALSE;
                        /* check if Aux plugged. If plugged, change to Aux mode */
                        if (AUX_PLUGGED == MainStatus.isAuxPlugged)
                        {
                            MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
                            MainStatus.btLEDtrigger = TRUE;
                        }
                    }
                }
            }
#endif

#ifdef HAS_POWER_SWITCH_KEY
            MainStatus.isPowerSwitchOn = SystemDrv_IsPowerSwitchOn();
            if(POWER_SWITCH_OFF == MainStatus.isPowerSwitchOn)
            {
                power_switch_cnt++;
                if (power_switch_cnt > POWER_KEY_DEBOUNCE_CNT)
                {
#ifdef POWER_OFF_TO_ON_DEBOUNCE
                    power_on_debounce_timeout = POWER_ON_DEBOUNCE_TIME;
#endif
                    MainApp_SwitchMode(MainApp, SLEEP_MODE);
                    AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
                    BSP_BlockingDelayMs(100);
#ifdef  HAS_DSP_EN
                    SystemDrv_DSPEnable(FALSE);
#endif
                    return Q_TRAN(&MainApp_DeActive);
                }
                else
                    power_switch_cnt = 0;
            }
#endif
#ifdef HAS_AUTO_STANDBY
            if( SYSTEM_STATUS_AUTO_STANDBY == SystemDrv_GetSystemStatus() )
            {
                SystemDrv_SetAutoStandby(TRUE);
                MainApp_SwitchMode(MainApp, SLEEP_MODE);
                return Q_TRAN(&MainApp_Standby);
            }
#endif
            MainApp_BoostUpdate(me, e);
            MainApp_ExternalChargingUpdate(me, e);
            MainApp_BatteryStatusUpdate(me, e);
            MainApp_TemperatureMonitor(me, e);

            if ( ((BAT_URGENT == MainStatus.batteryStatus)&&(DC_IN_NOT_DETECTED == MainStatus.isDCPlugged)) ||
                 (SystemDrv_IsNTCTurnOffSystem()))
            {
                MainApp_SwitchMode(MainApp, SLEEP_MODE);
                return Q_TRAN(&MainApp_Standby);
            }
            break;

        case BT_STATE_SIG:
        {
            BtStatusEvt *pEvt;
            pEvt = (BtStatusEvt *)e;
            if( pEvt->isBtStatus )  // this is BT status
            {
                MainApp_ParseBTEvent(me, e);
                MainApp_BT_LED_Handler(me, e);
            }
            else
            {
                // implement the bt event case later.
            }

            return Q_HANDLED();
        }
        /*************** tp monitor control signal start here ******************/
        case DISPLAY_DEBUG_REQ_SIG:
            MainApp_Version2TPMonitor();
            break;
        case MAINAPP_VOLUME_CTL_SIG:
            MainApp_VolumeCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_TREBLE_CTL_SIG:
            MainApp_TrebleCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_BASS_CTL_SIG:
            MainApp_BassCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_LED_CTL_SIG:
            MainApp_LedCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_SOURCE_CTL_SIG:
            MainApp_SourceCtlSig(me, (MainAppCtlEvt *)e);
            break;
        case MAINAPP_BYPASS_CTL_SIG:
            MainApp_BypassCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_BAT_CAPACITY_CTL_SIG:
            MainApp_BatteryCapacityRequest((MainAppCtlEvt *)e);
            break;
        case MAINAPP_NTC_CTL_SIG:
            MainApp_NTCRequest((MainAppCtlEvt *)e);
            break;
        case MAINAPP_MICLOOP_CTL_SIG:
            MainApp_MicLoopBackCtlSig((cMainApp *)me, (MainAppCtlEvt *)e);
            break;
        case MAINAPP_AMP_TEMP_CTL_SIG:
            MainApp_AmpTemperatureRequest((MainAppCtlEvt *)e);
            break;

        /*************** tp monitor control signal end here ******************/
        case SYSTEM_MODE_RESP_SIG:
        {
            MainApp_ShowVersion();
            MainApp_InitStatus((cMainApp *)me);
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_WORKING);

            /* recall the last source */
            MainStatus.isAuxPlugged= *(uint32_t*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
            MainStatus.CurrSource = MainApp_GetSource();
            /* if aux unplugged, switch to BT source when power on */
            if (AUX_UNPLUGGED == MainStatus.isAuxPlugged)
            {
                MainStatus.pdlStatus = *(uint32_t*)Setting_Get(SETID_PDL_STATUS);
                if (EMPTY_PDL == MainStatus.pdlStatus)
                {
                    MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
                }
                else
                {
                    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                }
                Setting_Set(SETID_AUDIO_SOURCE, &MainStatus.CurrSource);
            }
            AudioSrv_SetChannel((QActive *)me, (eAudioChannel)MainStatus.CurrSource);

            /* manual to connect last device */
            MainStatus.pdlStatus = *(uint32_t*)Setting_Get(SETID_PDL_STATUS);
            if ((NOT_EMPTY_PDL == MainStatus.pdlStatus)&&(MAINAPP_SOURCE_BT == MainStatus.CurrSource))
            {
                /* reconnect */
                MainStatus.CurrBTStatus = BT_RECONNECTING_STA;
                MainStatus.btLEDtrigger = TRUE;
                reconnect_pairing_timer = MAINAPP_RECONNECT_TIMEOUT;
            }

            return Q_HANDLED();
        }

        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if ((evt->keyId == FACTORY_RESET_COMB) &&(evt->keyEvent == COMB_KEY_EVT))
            {
                return Q_TRAN(&MainApp_FactoryReset);
            }
            else
            {
                MainApp_ParseKeyEvent(me, e);
                MainApp_BT_LED_Handler(me, e);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState MainApp_DeActive(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            gpio_key_pressed = GPIO_KEY_RELEASED;
            MainApp_AllLedOff();
            MainApp_PowerOff(me, e);
            SystemDrv_ExternalChargeEnable(FALSE);
            SystemDrv_BoostEnable(FALSE);
            BluetoothDrv_PowerEnable(FALSE);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
#ifdef POWER_OFF_TO_ON_DEBOUNCE
            if( power_on_debounce_timeout )
            {
                power_on_debounce_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if( power_on_debounce_timeout < 0 )
                {
                    power_on_debounce_timeout = 0;
                }
                return Q_HANDLED();
            }
#endif

#ifdef HAS_POWER_SWITCH_KEY
            MainStatus.isPowerSwitchOn = SystemDrv_IsPowerSwitchOn();
            if( POWER_SWITCH_ON == MainStatus.isPowerSwitchOn)
            {
                power_switch_cnt++;
                if (power_switch_cnt > POWER_KEY_DEBOUNCE_CNT)
                {
                    return Q_TRAN(&MainApp_PoweringUp);
                }
            }
            else
            {
                MainApp_TemperatureMonitor(me, e);
                power_switch_cnt = 0;
                if (DC_IN_NOT_DETECTED == MainStatus.isDCPlugged)
                {
                    SystemDrv_PowerEnable(FALSE);
                    return Q_HANDLED();
                }
                else
                {
                    MainApp_BatteryStatusUpdate(me, e);
                    MainApp_CheckBatteryFullyCharged(me, e);
                    return Q_HANDLED();
                }
            }
#else
            return Q_TRAN(&MainApp_PoweringUp);
#endif
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_STANDBY);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
            BSP_BlockingDelayMs(100);
            gpio_key_pressed = GPIO_KEY_RELEASED;
            MainApp_AllLedOff();
            SystemDrv_ExternalChargeEnable(FALSE);
            SystemDrv_BoostEnable(FALSE);
#ifdef  HAS_DSP_EN
            SystemDrv_DSPEnable(FALSE);
#endif
            BluetoothDrv_PowerEnable(FALSE);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
#ifdef HAS_POWER_SWITCH_KEY
            MainStatus.isPowerSwitchOn = SystemDrv_IsPowerSwitchOn();
            if( POWER_SWITCH_OFF == MainStatus.isPowerSwitchOn)
            {
                BSP_SoftReboot();
                return Q_HANDLED();
            }
            else
            {
                MainApp_TemperatureMonitor(me, e);
                if ((DC_IN_NOT_DETECTED == SystemDrv_IsDCInDetected())||(SystemDrv_IsNTCTurnOffSystem()))
                {
                    /************************/
                    /* TO do: Go to Deep Sleep */
                    /************************/
                    MainApp_GLED_SetupMode(GLED_ID_CHARGING, GLED_MODE_OFF, 0);
                    return Q_TRAN(&MainApp_Sleep);
                }
                else
                {
                    MainApp_BatteryStatusUpdate(me, e);
                    MainApp_CheckBatteryFullyCharged(me, e);
                }
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                return Q_HANDLED();
            }
#endif
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_STANDBY);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

QState MainApp_Sleep(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            gpio_key_pressed = GPIO_KEY_RELEASED;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_SLEEP_DELAY_TIMEOUT_IN_MS);
            MainStatus.isDCPlugged = DC_IN_UNKNOWN;        /* reset this flag */
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_SwitchMode(MainApp, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            GLED_OnOff(GLED_ID_CHARGING, FALSE);
            GLED_OnOff(GLED_ID_TALK, FALSE);
            GLED_OnOff(GLED_ID_POWER, FALSE);
            GLED_OnOff(GLED_ID_BT, FALSE);        

            SystemDrv_PowerStopMode();
            return Q_HANDLED();
        }
        case POWER_SWITCH_INT_SIG:
        {
            SetDisableWakeupSources();
            SystemDrv_InitNTCStatus();
            MainApp_SwitchMode(MainApp, SLEEP_MODE);
            return Q_TRAN(&MainApp_DeActive);
        }
        case DC_IN_INT_SIG:
        {
            SetDisableWakeupSources();
            MainApp_SwitchMode(MainApp, SLEEP_MODE);
            return Q_TRAN(&MainApp_Standby);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e)
{
    uint32_t value;
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            MainStatus.pdlStatus = EMPTY_PDL;
            value = MainStatus.pdlStatus;
            Setting_Set(SETID_PDL_STATUS, &value);
            MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
            Setting_Set(SETID_AUDIO_SOURCE, &MainStatus.CurrSource);
            SettingSrv_BookkeepingEx();
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_FACTORY_RESET_START_TIMEOUT_IN_MS);
            MainApp_FactoryReset_Led();
            BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {

        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static void MainApp_PowerOff(cMainApp * const me, QEvt const * const e)
{
    if ( (BT_CONNECTED_STA == MainStatus.CurrBTStatus)||
         (BT_STREAMING_A2DP_STA == MainStatus.CurrBTStatus) ||
         (BT_INCOMING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
         (BT_OUTGOING_CALL_EST_STA == MainStatus.CurrBTStatus) ||
         (BT_ACTIVE_CALL_STA == MainStatus.CurrBTStatus))
    {
        AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);
        BluetoothDrv_DisconnectBT();
        BSP_BlockingDelayMs(1500);
    }
}

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    uint32_t value;
    switch(evt->keyId)
    {
        case VOLUME_KNOB_BASE_KEY_ID:
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_VOLUME_STEPS - 1 - evt->index;
#endif
            ALWAYS_printf("\n\r main vol : %d.\n\r", evt->index);
            MainStatus.CurrVolume = evt->index;         /* backup current volume whenever changed volume knob */
            if (FALSE == MainStatus.isAudioCuePlaying)
            {
                value = evt->index;
                Setting_Set(SETID_VOLUME, &value);
                AudioSrv_SetVolume(evt->index);
            }
            break;
        case BASS_KNOB_BASE_KEY_ID:
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_BASS_STEPS - 1 - evt->index;
#endif
            ALWAYS_printf("\n\r bass : %d.\n\r", evt->index);
            value = evt->index;
            Setting_Set(SETID_BASS, &value);
            AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, evt->index, 0);
            break;
        case TREBLE_KNOB_BASE_KEY_ID:
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_TREBLE_STEPS - 1 - evt->index;
#endif
            ALWAYS_printf("\n\r treble : %d.\n\r", evt->index);
            value = evt->index;
            Setting_Set(SETID_TREBLE, &value);
            AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, evt->index, 0);
            break;
        case BT_KEY:
#ifdef SHORT_PRESS_PAIRING
            if (evt->keyEvent == KEY_EVT_DOWN)
            {
                gpio_key_pressed |= GPIO_KEY_BT_PRESSED;
            }
            else if (evt->keyEvent == KEY_EVT_UP)
            {
                if (gpio_key_ignore_cnt)
                    gpio_key_ignore_cnt --;
                gpio_key_pressed &= ~GPIO_KEY_BT_PRESSED;
                MainApp_ProcBtPairing(me, e);
            }
            else
            {
            }
#else
            if ((evt->keyEvent == KEY_EVT_HOLD)||(evt->keyEvent == KEY_EVT_SHORT_PRESS))
            {
                if (!(gpio_key_pressed & GPIO_KEY_TALK_PRESSED))
                {
                    if (FALSE == MainStatus.isAudioCuePlaying)
                    {
                        MainApp_ProcBtKey(me, e);
                    }
                }
            }
            else if (evt->keyEvent == KEY_EVT_DOWN)
            {
                gpio_key_pressed |= GPIO_KEY_BT_PRESSED;
            }
            else if (evt->keyEvent == KEY_EVT_UP)
            {
                if (gpio_key_ignore_cnt)
                    gpio_key_ignore_cnt --;
                gpio_key_pressed &= ~GPIO_KEY_BT_PRESSED;
            }
            else
            {
            }
#endif
            break;
        case TALK_KEY:
            if ((evt->keyEvent == KEY_EVT_HOLD)||(evt->keyEvent == KEY_EVT_SHORT_PRESS))
            {
                if (!(gpio_key_pressed & GPIO_KEY_BT_PRESSED))
                {
                    if (FALSE == MainStatus.isAudioCuePlaying)
                    {
                        MainApp_ProcTalkKey(me, e);
                    }
                }
            }
            else if (evt->keyEvent == KEY_EVT_DOWN)
            {
                gpio_key_pressed |= GPIO_KEY_TALK_PRESSED;
            }
            else if (evt->keyEvent == KEY_EVT_UP)
            {
                if (gpio_key_ignore_cnt)
                    gpio_key_ignore_cnt --;
                gpio_key_pressed &= ~GPIO_KEY_TALK_PRESSED;
            }
            else
            {
            }
            break;
            break;
        default:
            break;
    }
}

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

/* State Machine of BT status. This function also take action on LED if necessary  */
static void MainApp_PostBTEvent(cMainApp * const me, QEvt const * const e)
{
    uint32_t bt_status;
    bt_status = MainStatus.CurrBTStatus;
    switch(bt_status)
    {
        case BT_CONNECTABLE_STA:
            if (BT_OFF_STA == MainStatus.PrevBTStatus)
            {
                MainStatus.pdlStatus = *(uint32_t*)Setting_Get(SETID_PDL_STATUS);
                if (NOT_EMPTY_PDL == MainStatus.pdlStatus)
                {
                    /* reconnect */
                    MainStatus.CurrBTStatus = BT_RECONNECTING_STA;
                    MainStatus.btLEDtrigger = TRUE;
                }
                else
                {
                    MainStatus.CurrBTStatus = bt_status;
                    MainStatus.btLEDtrigger = TRUE;
                }
            }
            else if ((BT_INCOMING_CALL_EST_STA == MainStatus.PrevBTStatus)||
                     (BT_OUTGOING_CALL_EST_STA == MainStatus.PrevBTStatus)||
                     (BT_ACTIVE_CALL_STA == MainStatus.PrevBTStatus))
            {
                MainStatus.CurrSource = MainStatus.PrevSource;
                MainStatus.CurrBTStatus = bt_status;
                MainStatus.btLEDtrigger = TRUE;
            }
            /* disconnected */
            else if ((BT_STREAMING_A2DP_STA == MainStatus.PrevBTStatus)||
                     (BT_CONNECTED_STA == MainStatus.PrevBTStatus))
            {
                if (AUX_PLUGGED == MainStatus.isAuxPlugged)
                {
                    MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
                }
                MainStatus.CurrBTStatus = bt_status;
                MainStatus.btLEDtrigger = TRUE;
                MainStatus.isConnectedAudioCuePlayed = FALSE;
            }
            else if (BT_RECONNECTING_STA != MainStatus.PrevBTStatus)
            {
                MainStatus.CurrBTStatus = bt_status;
                MainStatus.btLEDtrigger = TRUE;
            }
            break;


        case BT_STREAMING_A2DP_STA:
        case BT_CONNECTED_STA:
            MainStatus.pdlStatus = NOT_EMPTY_PDL;
            Setting_Set(SETID_PDL_STATUS, &MainStatus.pdlStatus);
            MainStatus.isReconnectingOrPairing = FALSE;
            /* if previous BT status is call status */
            if ((BT_INCOMING_CALL_EST_STA == MainStatus.PrevBTStatus)||
                (BT_OUTGOING_CALL_EST_STA == MainStatus.PrevBTStatus)||
                (BT_ACTIVE_CALL_STA == MainStatus.PrevBTStatus))
            {
                MainStatus.CurrSource = MainStatus.PrevSource;
            }
            /* else, give "CONNECTED" audio cue */
            else if ((BT_DISCOVERABLE_STA == MainStatus.PrevBTStatus)||
                     (BT_CONNECTABLE_STA == MainStatus.PrevBTStatus)||
                     (BT_OFF_STA == MainStatus.PrevBTStatus)||
                     (BT_RECONNECTING_STA == MainStatus.PrevBTStatus))
            {
                if (FALSE == MainStatus.isConnectedAudioCuePlayed)
                {
                    MainStatus.isConnectedAudioCuePlayed = TRUE;
                    MainApp_SendBTCmdWithAudioCue(me, e, BT_CONNECTED_CMD);
                }
            }

            MainStatus.CurrBTStatus = bt_status;
            MainStatus.btLEDtrigger = TRUE;
            break;


        case BT_DISCOVERABLE_STA:
            MainStatus.isConnectedAudioCuePlayed = FALSE;
            MainStatus.CurrBTStatus = bt_status;
            MainStatus.btLEDtrigger = TRUE;
            break;

        case BT_ACTIVE_CALL_STA:
            MainStatus.CurrBTStatus = bt_status;
            MainStatus.btLEDtrigger = TRUE;
            break;

        default:
            break;
    }
}

/* State Machine of BT status. This function also take action on LED if necessary  */
static void MainApp_ParseBTEvent(cMainApp * const me, QEvt const * const e)
{
    uint32_t bt_status;
    bt_status = *(uint32_t*)Setting_Get(SETID_BT_STATUS);

    if (MainStatus.CurrBTStatus != bt_status)
    {
        switch(bt_status)
        {
            case BT_AUDIO_CUE_START_STA:
                if (TRUE == MainStatus.isAudioCuePlaying)
                {
                    MainStatus.PrevSource = MainStatus.CurrSource;
                    AudioSrv_SetVolume(DEFAULT_AUDIO_CUE_VOLUME);
                    AudioSrv_SetChannel((QActive *)me, MAINAPP_SOURCE_BT);
                }
                break;

            case BT_AUDIO_CUE_STOP_STA:
                if (TRUE == MainStatus.isAudioCuePlaying)
                {
                    MainStatus.isAudioCuePlaying = FALSE;
                    Setting_Set(SETID_BT_STATUS, &MainStatus.CurrBTStatus);
                    MainStatus.CurrSource = MainStatus.PrevSource;
                    AudioSrv_SetChannel((QActive *)me, (eAudioChannel)MainStatus.CurrSource);
                    Setting_Set(SETID_AUDIO_SOURCE, &MainStatus.CurrSource);
                    AudioSrv_SetVolume(MainStatus.CurrVolume);
                }
                break;

            case BT_STREAMING_A2DP_STA:
            case BT_CONNECTED_STA:
                if ((TRUE == bt_status_updated)&&(bt_status_timer <= BT_STATUS_DEBOUNCE_CNT))
                {
                    /* if within debouncing, update currBTStatus to current bt status, for HUIWEI phone bug, IN12273 */
                    MainStatus.CurrBTStatus = bt_status;
                }
                else
                {
                    /* these status need to check if any continue status coming */
                    if ((BT_DISCOVERABLE_STA == MainStatus.CurrBTStatus)||
                        (BT_CONNECTABLE_STA == MainStatus.CurrBTStatus)||
                        (BT_OFF_STA == MainStatus.CurrBTStatus)||
                        (BT_RECONNECTING_STA == MainStatus.CurrBTStatus))
                    {
                        if (FALSE == MainStatus.isConnectedAudioCuePlayed)
                        {
                            MainStatus.isConnectedAudioCuePlayed = TRUE;
                            MainApp_SendBTCmdWithAudioCue(me, e, BT_CONNECTED_CMD);
                        }
                    }
                    bt_status_updated = TRUE;
                    bt_status_timer = 0;
                    MainStatus.PrevBTStatus = MainStatus.CurrBTStatus;
                    MainStatus.CurrBTStatus = bt_status;
                }
                break;

            case BT_ACTIVE_CALL_STA:
                if (BT_CONNECTABLE_STA == MainStatus.CurrBTStatus)
                {
                    if (FALSE == MainStatus.isConnectedAudioCuePlayed)
                    {
                        MainStatus.isConnectedAudioCuePlayed = TRUE;
                        MainApp_SendBTCmdWithAudioCue(me, e, BT_CONNECTED_CMD);
                    }
                }
                bt_status_updated = TRUE;
                bt_status_timer = 0;
                MainStatus.PrevBTStatus = MainStatus.CurrBTStatus;
                MainStatus.CurrBTStatus = bt_status;
                break;

            case BT_CONNECTABLE_STA:
            case BT_DISCOVERABLE_STA:
                /* these status need to check if any continue status coming */
                bt_status_updated = TRUE;
                bt_status_timer = 0;
                MainStatus.PrevBTStatus = MainStatus.CurrBTStatus;
                MainStatus.CurrBTStatus = bt_status;
                break;

            case BT_INCOMING_CALL_EST_STA:
            case BT_OUTGOING_CALL_EST_STA:
                MainStatus.PrevSource = MainStatus.CurrSource;
                MainStatus.CurrSource = MAINAPP_SOURCE_BT;      /* force go to BT */
                MainStatus.CurrBTStatus = bt_status;
                MainStatus.btLEDtrigger = TRUE;
                break;

            default:
                break;
        }
    }
}



static void MainApp_ParseAUXEvent(cMainApp * const me, QEvt const * const e)
{
    uint8 isAuxPlugged;
    isAuxPlugged= *(uint32_t*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);

    if (MainStatus.isAuxPlugged != isAuxPlugged)
    {
        MainStatus.isAuxPlugged = isAuxPlugged;
        if ((MAINAPP_SOURCE_BT == MainStatus.CurrSource) && (AUX_PLUGGED == MainStatus.isAuxPlugged))
        {
            if ((BT_INCOMING_CALL_EST_STA == MainStatus.CurrBTStatus)||
                (BT_OUTGOING_CALL_EST_STA == MainStatus.CurrBTStatus)||
                (BT_ACTIVE_CALL_STA == MainStatus.CurrBTStatus))
            {
                /* ignore if call state */
            }
            else
            {
                MainStatus.CurrSource = MAINAPP_SOURCE_AUX;
                MainStatus.btLEDtrigger = TRUE;
                if (BT_STREAMING_A2DP_STA == MainStatus.CurrBTStatus)
                {
                    /* auto issue avrcp_pause */
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
                }
            }
        }
        if ((MAINAPP_SOURCE_AUX == MainStatus.CurrSource) && (AUX_UNPLUGGED == MainStatus.isAuxPlugged))
        {

            switch (MainStatus.CurrBTStatus)
            {
                case BT_OFF_STA:
                case BT_CONNECTABLE_STA:
                case BT_DISCOVERABLE_STA:
                    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                    MainStatus.btLEDtrigger = TRUE;
                    if (NOT_EMPTY_PDL == MainStatus.pdlStatus)
                    {
                        /* reconnect */
                        MainStatus.CurrBTStatus = BT_RECONNECTING_STA;
                    }
                    break;

                case BT_CONNECTED_STA:
                case BT_STREAMING_A2DP_STA:
                    MainStatus.CurrSource = MAINAPP_SOURCE_BT;
                    MainStatus.btLEDtrigger = TRUE;
                    break;

                default:
                    break;

            }
        }
    }
}

#ifdef UNIT_TEST_ENABLE
static void MainStatus_UnitTest(uint16 level)
{
    if (level == 1)
    {
        SystemDrv_SetUnitTest(1);
    }
    else if (level == 2)
    {
        SystemDrv_SetUnitTest(2);
    }
    else if (level == 3)
    {
        SystemDrv_SetUnitTest(3);
    }
    else if (level == 4)
    {
        SystemDrv_SetUnitTest(4);
    }
}
#endif
