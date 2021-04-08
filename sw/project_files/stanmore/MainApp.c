/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        MainApp.c
@brief       Main application for Stanmore
@author      Dmitry Abdulov
@date        2014-12-11
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

/*to do list*/
/* need to remove the extern with getDeviceId function after the GPIO struct is changed*/

#include "./MainApp_priv.h"
#include "PowerDrv.h"
#include "projBsp.h"
#include "trace.h"

#include "DebugSSrv.h"
#include "AudioSrv.Config"

#define VOLUME_POSITIONS_LEVELS 30
#define BASS_POSITIONS_LEVELS   9
#define TREBLE_POSITIONS_LEVELS 9

#define MAIN_ADC_MAX_VALUE_mVOLT (4095)
#define MAIN_ADC_REFERENCE_mVOLT (3300)
#define MAIN_KNOB_R              (10000)
#define MAIN_ADC_HYS             (1)

static tThresholdLevel volume_threshold_data[VOLUME_POSITIONS_LEVELS];
static tThresholdLevel bass_threshold_data[BASS_POSITIONS_LEVELS];
static tThresholdLevel treble_threshold_data[TREBLE_POSITIONS_LEVELS];

#define NOT_DEFINED (-0x7F)
#define TIMER_IS_NOT_SETUP              -1

#define CAST_ME cMainApp * MainApp = (cMainApp *) me;

static cPowerDrv powerDrv;
/* Battery variables */
/* the time (ms) per timeout signal */

#define MAINAPP_POWER_TIMEOUT_IN_MS  10

#define TIMER_IS_NOT_SETUP              -1

#define BT_POWERING_OFF_DELAY_MS           1000//800//600//delay after send BT off command to make sure we close bt connection
static int16 bt_powering_off_delay_timeout = TIMER_IS_NOT_SETUP;

#define BT_POWERING_ON_DELAY_MS         2500//2000// 2500 //3000
static int16 bt_powering_on_delay_timeout = TIMER_IS_NOT_SETUP;

static bool bIsBtPostponedCmd   = FALSE;

#define STOP_BTSTREAM_DELAY_MS             600 //1600//     600
static int16 stopping_btstream_delay_timeout = TIMER_IS_NOT_SETUP;
#define STOP_BTSTREAM_DEADLOOP_TIMEOUT_MS       6000 // prevent dead loop after 6 sec force to continue powering off
static int16 stopping_btstream_deadloop_timeout = TIMER_IS_NOT_SETUP;

#define BT_RECONNECTING_DELAY_MS          180000
static int32 bt_reconnecting_delay_timeout = TIMER_IS_NOT_SETUP;

#define BLUETOOTH_STARTUP_RECONNECT_DELAY_MS    30000
static int32 bluetooth_startup_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;


#define INACTIVITY_DELAY_MS         1155000 //1140000
static int32 inactivity_timeout = TIMER_IS_NOT_SETUP;
#define RESET_INACTIVITY    inactivity_timeout = INACTIVITY_DELAY_MS + 60000

#define BLUETOOTH_STARTUP_DELAY_MS  1000
static int16 bluetooth_startup_delay_timeout = TIMER_IS_NOT_SETUP;

#define BLUETOOTH_AUTOCANCEL_PAIRING_DELAY_MS  1000
static int16 bluetooth_autocancel_pairing_delay_timeout = TIMER_IS_NOT_SETUP;


#define BLUETOOTH_CONNECT_WAKEUP_DELAY_MS   5000
static int16 bluetooth_connect_wakeup_delay_timeout = TIMER_IS_NOT_SETUP;

static int16 setting_energysense_mask_delay_timeout = TIMER_IS_NOT_SETUP;

static eAudioChannel setMaskForChannel = AUDIO_CHANNEL_INVALID;
// 2 - aux
// 3 - bt
static const eAudioChannel defaultChannel@"SETID_CHANNEL_REGION" = 3; // 3  bt first default input source

#define BUTTON_REACTION_DELAY_MS   500
static int16 button_reaction_delay_timeout = TIMER_IS_NOT_SETUP;

static eAudioChannel channelSwitchTo = AUDIO_CHANNEL_INVALID;

static bool bIsInputSwitchingNow = FALSE;
static eKeyID keyPressed = INVALID_KEY;

#define POWER_KEY_SWITCH_CHECK_DELAY_MS  300
static int16 power_key_switch_check_delay_timeout = TIMER_IS_NOT_SETUP;

#define DEFAULT_ADC_MIN     0
#define DEFAULT_ADC_MAX     4095

typedef enum
{
    CALIBRATION_NONE,
    CALIBRATION_MIN,
    CALIBRATION_MAX
}eKnobCalibrationMode;

static eKnobCalibrationMode calibrationMode = CALIBRATION_NONE;

static uint8 nKnobKeyCalibrationCount = 0;
#define KNOB_KEYS_NUM                   3

static bool isMusicDetected = FALSE;
static bool isRePairing = FALSE;
static bool isLinkLossHappened = FALSE;
static bool isPairingStateContinue = FALSE;

static bool bIsFirstBtState = TRUE;
static bool bIsResumeFromSleep = FALSE;
static bool bIsStartAfterFactoryReset = FALSE;

/*-----------------------------------------------------------------*/

static int32       power_init_timer;
static int32       awake_without_bt_init_timer;

#define FACTORY_RESET_LED_DELAY_TIMEOUT_MS  2000
static int16 factory_reset_led_timeout = TIMER_IS_NOT_SETUP;
#define BT_RESET_DELAY_TIMEOUT_MS  400
static int16 bt_reset_delay_timeout = TIMER_IS_NOT_SETUP;

static bool bIsFirstBtMsg = TRUE;

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
static eAudioPowerDrcLevel prevPowerDrcLevel = AUDIO_MAX_DRC_LEVEL;
#endif
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* Private functions / variables. */

/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[12];

static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[6];

static bool bIsLED_AUX_ON;
static bool bIsLED_RCA_ON;
static bool bIsLED_BT_ON;
#include "GpioDrv.h"
static cGpioDrv ledGpiodrv;

#define _LED_AUX_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_22)
#define LED_AUX_ON  _LED_AUX_ON; bIsLED_AUX_ON = TRUE
#define _LED_AUX_OFF  GpioDrv_SetBit(&ledGpiodrv,GPIO_22)
#define LED_AUX_OFF   _LED_AUX_OFF; bIsLED_AUX_ON = FALSE

#define _LED_RCA_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_21)
#define LED_RCA_ON  _LED_RCA_ON; bIsLED_RCA_ON = TRUE
#define _LED_RCA_OFF  GpioDrv_SetBit(&ledGpiodrv,GPIO_21)
#define LED_RCA_OFF   _LED_RCA_OFF; bIsLED_RCA_ON = FALSE

#define _LED_BT_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_23)
#define LED_BT_ON  _LED_BT_ON; bIsLED_BT_ON = TRUE
#define _LED_BT_OFF  GpioDrv_SetBit(&ledGpiodrv,GPIO_23)
#define LED_BT_OFF   _LED_BT_OFF; bIsLED_BT_ON = FALSE

static eBtStatus     myBtStatus;

#define LED_FLASHING_COUNT  5
#define LED_FLASHING_COUNT_IS_NOT_SETUP     -1
#define LED_FLASHING_COUNT_INDEFINITE       0xFF

#define MAIN_AUX_SOURCE AUDIO_CHANNEL_1
#define MAIN_RCA_SOURCE AUDIO_CHANNEL_RCA
#define MAIN_BT_SOURCE  AUDIO_CHANNEL_2

static int16 led_flashing_idx = LED_FLASHING_COUNT_IS_NOT_SETUP;

#define ON (1)
#define OFF (0)

typedef struct _ledState
{
    eLedPatternType patternId;
    uint32 timer;
    uint8 ledOn;
} ledState;
static ledState leds[LED_COUNT] = {{0, 0xff},{0, 0xff},{0, 0xff}};

static void MainApp_UpdateLedState();
static bool MainApp_SetLedPattern(eLedName led, eLedPatternType pattern, uint32 timer);
static bool MainApp_SetDisplaySource(eSourceState sourceState);
static bool MainApp_AllLEDsOff();

static eAudioChannel prevChannel = AUDIO_CHANNEL_INVALID;

static void MainApp_SaveCalibrationValue(eKeyID key_id, int16 rawData);
static void MainApp_ForceSaveDataIntoFlash();
static void MainApp_FinishCalibration();

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    PowerDrv_Ctor(&powerDrv);

    tGPIODevice *pLedGPIOConf;
    uint16 attached_device_index = 0;
    pLedGPIOConf = (tGPIODevice*)getDevicebyId(LED_DEV_ID,&attached_device_index);
    GpioDrv_Ctor(&ledGpiodrv,pLedGPIOConf);
    LED_AUX_OFF;
    LED_BT_OFF;
    LED_RCA_OFF;

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
#endif
#ifdef HAS_INTERRUPT_WAKE_UP_KEY
    QActive_subscribe((QActive*) me, POWER_MCU_SLEEP_SIG);
#endif

}

void MainApp_ShutDown(cPersistantObj *me)
{
    Application_Xtor((cApplication*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
QState MainApp_Initial(cMainApp * const me, QEvt const * const e)
{
    /* initial the default value for first power up or factory reset*/
    MainApp_InitialVariablesForFactoryReset(me);
    if (PowerDrv_IsMainSwitchOn())
    {
        return Q_TRAN(&MainApp_PoweringUp);
    }
    else
    {
        return Q_TRAN(&MainApp_Standby);
    }
}

QState MainApp_Resuming(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            if (prevChannel == MAIN_BT_SOURCE)
            {
                eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                if (btStatus != BT_CONNECTED_STA)
                {
                    if (!bIsStartAfterFactoryReset)
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                        bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS;
                        MainApp_SetDisplaySource(SOURCE_STATE_BT_RECONNECTING);
                    }
                    else
                    {/* we are still after factory reset, so keep connectable led flashing*/
                        MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTABLE);
                    }
                }
                else
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTED);
                }
            }
            MainApp_SwitchMode(MainApp, NORMAL_MODE);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == VOLUME_KNOB_BASE_KEY_ID) || (evt->keyId == BASS_KNOB_BASE_KEY_ID) || (evt->keyId == TREBLE_KNOB_BASE_KEY_ID))
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
            }
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            bIsResumeFromSleep = TRUE;
            return Q_TRAN(&MainApp_Active);
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LED_AUX_OFF;
            LED_RCA_ON;
            LED_BT_ON;
            bIsFirstBtMsg = TRUE;
            factory_reset_led_timeout = FACTORY_RESET_LED_DELAY_TIMEOUT_MS;
            eAudioChannel channel = AUDIO_CHANNEL_INVALID;
            channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
            if (channel !=  MAIN_BT_SOURCE)
            {
                bIsFirstBtState = TRUE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_ON_CMD);
            }
            else
            {
                bIsFirstBtMsg = FALSE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
                bt_reset_delay_timeout = BT_RESET_DELAY_TIMEOUT_MS;
            }
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            if (bIsFirstBtMsg)
            {
                bIsFirstBtMsg = FALSE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
                bt_reset_delay_timeout = BT_RESET_DELAY_TIMEOUT_MS;
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (factory_reset_led_timeout != TIMER_IS_NOT_SETUP)
            {
                if (factory_reset_led_timeout > 0)
                {
                    factory_reset_led_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    factory_reset_led_timeout = TIMER_IS_NOT_SETUP;
                    LED_RCA_OFF;
                    LED_BT_OFF;
                    if (bt_reset_delay_timeout == TIMER_IS_NOT_SETUP)
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                        MainApp_SettingBtOffState(me);
                        bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                        prevChannel = MAIN_BT_SOURCE;
                        Setting_Set(SETID_CHANNEL, &prevChannel);
                        MainApp_ForceSaveDataIntoFlash();
                        bIsStartAfterFactoryReset = TRUE;
                        bIsResumeFromSleep = FALSE;
                        return Q_TRAN(&MainApp_Active);
                    }
                }
            }
            if (bt_reset_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bt_reset_delay_timeout > 0)
                {
                    bt_reset_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_reset_delay_timeout = TIMER_IS_NOT_SETUP;
                    if (factory_reset_led_timeout == TIMER_IS_NOT_SETUP)
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                        MainApp_SettingBtOffState(me);
                        bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                        prevChannel = MAIN_BT_SOURCE;
                        Setting_Set(SETID_CHANNEL, &prevChannel);
                        MainApp_ForceSaveDataIntoFlash();
                        bIsStartAfterFactoryReset = TRUE;
                        bIsResumeFromSleep = FALSE;
                        return Q_TRAN(&MainApp_Active);
                    }
                }
            }

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

QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            power_init_timer = PowerDrv_InitialPower(&powerDrv);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            power_init_timer -= MAIN_APP_TIMEOUT_IN_MS;
            if ( power_init_timer <= 0 )
            {
                power_init_timer = PowerDrv_InitialPower(&powerDrv);
                if(power_init_timer <= 0)
                {

                    MainApp_SwitchMode(MainApp, NORMAL_MODE);
                }
                else
                {
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
            }
            else
            {
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }

            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == VOLUME_KNOB_BASE_KEY_ID) || (evt->keyId == BASS_KNOB_BASE_KEY_ID) || (evt->keyId == TREBLE_KNOB_BASE_KEY_ID))
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
            }
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            bIsResumeFromSleep = FALSE;
            return Q_TRAN(&MainApp_Active);
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state  - super state for "normal" behaviour */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            RESET_INACTIVITY;
            MainApp_InitialVariablesForPowerUp(me);
            if (!bIsResumeFromSleep)
            {
                MainApp_SwitchToPreviousSource(me);
            }
            else
            {/* we just need to set led for aux or rca as bt led will be set by parse function already */
                if (prevChannel == MAIN_AUX_SOURCE)
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_AUX);
                    channelSwitchTo = prevChannel;
                }
                else if (prevChannel == MAIN_RCA_SOURCE)
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_RCA);
                    channelSwitchTo = prevChannel;
                }
                else if (prevChannel == MAIN_BT_SOURCE)
                {
                    /* keep pcm1862 always on in bt mode, but mask energysense interrupt*/
                    SwitchEnergySenseMode(FALSE);
                    SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                }
                QActive_recall((QActive*)me, &deferredReqQue);
                QActive_recall((QActive*)me, &deferredReqQue);
                QActive_recall((QActive*)me, &deferredReqQue);
            }
            power_key_switch_check_delay_timeout = POWER_KEY_SWITCH_CHECK_DELAY_MS;
            SetEnableWakeupSources();
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            MainApp_ParseBTEvent(me, e);
            return Q_HANDLED();
        }
        case ENERGYSENSE_SIG:
        {
           uint8 mode = GetAdcMode();
           if (mode == PCM1862_SLEEP_MODE)
           {/* this is signal detection */
                SwitchEnergySenseMode(FALSE);
                SetEnergySenseMask(channelSwitchTo);
             /* got music detect, reset idle timer */
                RESET_INACTIVITY;
                isMusicDetected  = TRUE;
            }
           else if (mode == PCM1862_ACTIVE_MODE)
           { /* this is signal loss*/
                SwitchEnergySenseMode(TRUE);
                SetEnergySenseMask(channelSwitchTo);
                /* got 1 min of silence,- start idle timer for 19 min */
                    inactivity_timeout = INACTIVITY_DELAY_MS;//100;//60000; ~ 19 minutes
                    isMusicDetected = FALSE;
                }
            return Q_HANDLED();
        }
        case POWER_SWITCH_SIG:
        {
            if (power_key_switch_check_delay_timeout == TIMER_IS_NOT_SETUP)
            {
                power_key_switch_check_delay_timeout = POWER_KEY_SWITCH_CHECK_DELAY_MS;
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            RESET_INACTIVITY;
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if ((evt->keyId == COMB_KEY_ID_0) && (evt->keyEvent == COMB_KEY_EVT))
            {/* todo: factory reset here */
                return Q_TRAN(&MainApp_FactoryReset);
            }
            MainApp_ParseKeyEvent(me, e);
            return Q_HANDLED();
        }
        case KEY_DEBUG_RESP_SIG:
        {
            int8 tempValue;
            KeyDebugRespEvt* evt = (KeyDebugRespEvt*)e;

            if (evt->serviceState == RET_SUCCESS)
            {
                switch(evt->keyId)
                { /* just in case we get wrong message */
                    case VOLUME_KNOB_BASE_KEY_ID:
                    case BASS_KNOB_BASE_KEY_ID:
                    case TREBLE_KNOB_BASE_KEY_ID:
                    {
                        MainApp_SaveCalibrationValue(evt->keyId, evt->rawData);
                        nKnobKeyCalibrationCount++;
                        MainApp_FinishCalibration();
                        break;
                    }
                    default:
                        break;
                }
            }
            return Q_HANDLED();
        }
       case AUDIO_SWITCH_CHANNEL_RESP_SIG:
       {
            AudioChannelSwitchRespEvt* resp = (AudioChannelSwitchRespEvt*)e;
            if (resp->evtReturn == RET_SUCCESS)
            {
                setting_energysense_mask_delay_timeout = AUDIO_SWITCH_CHANNEL_DELAY_TIME;
                if (setMaskForChannel == MAIN_BT_SOURCE)
                {
                    myBtStatus = BT_MAX_STA; // set previous status to invalid so bt led will update properly
                    /*_______________________________*/
                    bluetooth_startup_reconnect_delay_timeout = BLUETOOTH_STARTUP_RECONNECT_DELAY_MS;
                    SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                    SwitchEnergySenseMode(FALSE);
                    setting_energysense_mask_delay_timeout = TIMER_IS_NOT_SETUP;
                }

                bIsInputSwitchingNow = FALSE;
                QActive_recall((QActive*)me, &deferredReqQue);
            }
            else
            { /* TODO: will add error indication here */
                AudioChannelSwitchRespEvt* resp = (AudioChannelSwitchRespEvt*)e;
                SwitchInputReq* switchInput = Q_NEW(SwitchInputReq, SWITCH_INPUT_SIG);
                switchInput->channel = resp->channel;
                QACTIVE_POST((QActive*)me, (QEvt*)switchInput, 0);
            }

            return Q_HANDLED();
       }
        case BT_IS_OFF_SIG:
        {
            if (bIsBtPostponedCmd)
            {
                bIsBtPostponedCmd = FALSE;
                if (channelSwitchTo == MAIN_BT_SOURCE)
                {
                    /* turning on bt */
                    bIsFirstBtState = TRUE;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_ON_CMD);
                    bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS;
                    bt_powering_on_delay_timeout = BT_POWERING_ON_DELAY_MS;
                    bluetooth_startup_reconnect_delay_timeout = BLUETOOTH_STARTUP_RECONNECT_DELAY_MS;
                }
            }
            return Q_HANDLED();
        }

        case BT_IS_ON_SIG:
        {
            if (bIsBtPostponedCmd)
            {
                bIsBtPostponedCmd = FALSE;
                if (channelSwitchTo != MAIN_BT_SOURCE)
                {
                    if (bluetooth_startup_delay_timeout != TIMER_IS_NOT_SETUP)
                    {
                        bluetooth_startup_delay_timeout  = TIMER_IS_NOT_SETUP;
                    }
                    /* turning off bt */
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                    MainApp_SettingBtOffState(me);
                    bIsStartAfterFactoryReset = FALSE;
                    eBtStatus btStatus = BT_CONNECTABLE_STA;
                    Setting_Set(SETID_BT_STATUS, &btStatus);
                    bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                    bt_reconnecting_delay_timeout = TIMER_IS_NOT_SETUP;
                    bluetooth_startup_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                }
            }

            return Q_HANDLED();
        }
        case SWITCH_INPUT_SIG:
        {
            if (!bIsInputSwitchingNow)
            {
                SwitchInputReq* evt = (SwitchInputReq*)e;
                bIsInputSwitchingNow = TRUE;
                MainApp_SwitchInputSource(me, evt->channel);
            }
            else
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
            }

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_UpdateLedState();
            if (button_reaction_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (button_reaction_delay_timeout > 0)
                {
                    button_reaction_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    button_reaction_delay_timeout = TIMER_IS_NOT_SETUP;
                     switch(keyPressed)
                     {
                        case INPUT_KEY:
                            SwitchInputReq* switchInput = Q_NEW(SwitchInputReq, SWITCH_INPUT_SIG);
                            switchInput->channel = channelSwitchTo;
                            QACTIVE_POST((QActive*)me, (QEvt*)switchInput, 0);
                            break;
                        case BT_KEY:
                            eAudioChannel channel = AUDIO_CHANNEL_INVALID;
                            channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
                            if (channel == MAIN_BT_SOURCE)
                            {
                                eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                                if (btStatus == BT_DISCOVERABLE_STA)
                                {
                                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                                }
                                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                                MainApp_SetDisplaySource(SOURCE_STATE_BT_PAIRING);
                            }
                            else
                            {
                                if (bt_powering_off_delay_timeout == TIMER_IS_NOT_SETUP)
                                {
                                    SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                                    setMaskForChannel = MAIN_BT_SOURCE;
                                    MainApp_SwitchSource(me, MAIN_BT_SOURCE);
                                    /* turning on bt */
                                    bIsFirstBtState = TRUE;
                                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_ON_CMD);
                                    bt_powering_on_delay_timeout = BT_POWERING_ON_DELAY_MS;
                                    bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS;
                                    MainApp_SetDisplaySource(SOURCE_STATE_BT_RECONNECTING);
                                }
                            }
                            channelSwitchTo = MAIN_BT_SOURCE;
                            break;
                        default:
                            break;
                     }
                }
            }
            if (setting_energysense_mask_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (setting_energysense_mask_delay_timeout > 0)
                {
                    setting_energysense_mask_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    setting_energysense_mask_delay_timeout = TIMER_IS_NOT_SETUP;
                    SwitchEnergySenseMode(TRUE);
                    SetEnergySenseMask(channelSwitchTo);
                }
            }
            if (bluetooth_autocancel_pairing_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bluetooth_autocancel_pairing_delay_timeout > 0)
                {
                    bluetooth_autocancel_pairing_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bluetooth_autocancel_pairing_delay_timeout = TIMER_IS_NOT_SETUP;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    if (channelSwitchTo == MAIN_BT_SOURCE)
                    {
                        MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTABLE);  //335
                    }
                }
            }
            if (bluetooth_startup_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bluetooth_startup_delay_timeout > 0)
                {
                    bluetooth_startup_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bluetooth_startup_delay_timeout = TIMER_IS_NOT_SETUP;
                    bIsFirstBtState = TRUE;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_ON_CMD);
                    if (bIsStartAfterFactoryReset)
                    {
                        bluetooth_autocancel_pairing_delay_timeout = BLUETOOTH_AUTOCANCEL_PAIRING_DELAY_MS;
                    }
                    else
                    {
                        bluetooth_startup_reconnect_delay_timeout = BLUETOOTH_STARTUP_RECONNECT_DELAY_MS;
                    }
                }
            }
/*______________________________________________*/
            if (bluetooth_startup_reconnect_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bluetooth_startup_reconnect_delay_timeout > 0)
                {
                    bluetooth_startup_reconnect_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                    if (btStatus != BT_CONNECTABLE_STA)
                    {
                        bluetooth_startup_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                    }
                    else
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                        bluetooth_startup_reconnect_delay_timeout = BLUETOOTH_STARTUP_RECONNECT_DELAY_MS;
                    }
                }
            }
/*______________________________________________*/
            if (bt_reconnecting_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bt_reconnecting_delay_timeout > 0)
                {
                    bt_reconnecting_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bluetooth_startup_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                   /*_________________________________________*/
                    bt_reconnecting_delay_timeout = TIMER_IS_NOT_SETUP;
                    if (channelSwitchTo == MAIN_BT_SOURCE)
                    {
                        eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                        if (btStatus == BT_CONNECTABLE_STA)
                        {
                            MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTABLE);
                        }
                    }
                }
            }
            if (power_key_switch_check_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (power_key_switch_check_delay_timeout > 0)
                {
                    power_key_switch_check_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    power_key_switch_check_delay_timeout = TIMER_IS_NOT_SETUP;
                    if (!PowerDrv_IsMainSwitchOn())
                    {
                        MainApp_AllLEDsOff();
                        return Q_TRAN(MainApp_IsBtChannel(me));
                    }
                    else
                    {
                        SetEnableWakeupSources();
                    }
                }
            }

            if (bt_powering_off_delay_timeout != TIMER_IS_NOT_SETUP)
            {/* run timer for giving bt time for shutdown */
                if (bt_powering_off_delay_timeout > 0)
                {
                    bt_powering_off_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_powering_off_delay_timeout = TIMER_IS_NOT_SETUP;
                    QEvt* btOffEvt = Q_NEW(QEvt, BT_IS_OFF_SIG);
                    QACTIVE_POST((QActive*)me, btOffEvt, 0);
                }
            }
            if (bt_powering_on_delay_timeout != TIMER_IS_NOT_SETUP)
            {/* run timer for giving bt time for power on */
                if (bt_powering_on_delay_timeout > 0)
                {
                    bt_powering_on_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_powering_on_delay_timeout = TIMER_IS_NOT_SETUP;
                    QEvt* btOnEvt = Q_NEW(QEvt, BT_IS_ON_SIG);
                    QACTIVE_POST((QActive*)me, btOnEvt, 0);
                }
            }
            if(inactivity_timeout != TIMER_IS_NOT_SETUP)
            {
                if (!isMusicDetected)
                {
                    inactivity_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    if(inactivity_timeout <= 0)
                    {
                        inactivity_timeout = TIMER_IS_NOT_SETUP;
                         return Q_TRAN(&MainApp_Lulled); // preparing for sleep
                    }
                }
                else
                {
                    RESET_INACTIVITY;
                }
            }
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

QState MainApp_BtDisconnect(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            /*switching to pairing and then cancel the pairing mode will disconnect bt
                and switch the module to connectable mode */
            BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
            BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);

            stopping_btstream_delay_timeout = STOP_BTSTREAM_DELAY_MS;
            stopping_btstream_deadloop_timeout = STOP_BTSTREAM_DEADLOOP_TIMEOUT_MS;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (stopping_btstream_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (stopping_btstream_delay_timeout > 0)
                {
                    stopping_btstream_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                    if (btStatus == BT_CONNECTABLE_STA )
                    {
                        stopping_btstream_delay_timeout = TIMER_IS_NOT_SETUP;
                        /* using this tone command for play disconect audio cue*/
                        AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                        return Q_TRAN(&MainApp_PoweringDown);
                    }
                    else
                    {
                        stopping_btstream_delay_timeout = STOP_BTSTREAM_DELAY_MS;
                    }
                }
            }
            if (stopping_btstream_deadloop_timeout != TIMER_IS_NOT_SETUP)
            {
                if (stopping_btstream_deadloop_timeout > 0)
                {
                    stopping_btstream_deadloop_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    /* using this tone command for play disconect audio cue*/
                    AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                    stopping_btstream_deadloop_timeout = TIMER_IS_NOT_SETUP;
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
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

QState MainApp_Lulled(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_PublishSystemStatus(MainApp, SYSTEM_SLEEP_STA);
            /* check if there is bt connection - disconnect before goes to stanby */

            eAudioChannel channel = AUDIO_CHANNEL_INVALID;
            channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
             if (channel == MAIN_BT_SOURCE)
             {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                SwitchEnergySenseMode(TRUE);

             }
             else
             {
                MainApp_SwitchMode(MainApp, SLEEP_MODE);
             }
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            BtStatusEvt *evt = (BtStatusEvt*)e;
            if (evt->btStatus == BT_CONNECTABLE_STA)
            {
                MainApp_SwitchMode(MainApp, SLEEP_MODE);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            return Q_TRAN(&MainApp_Sleep);
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

/* powering down state, which is turning off the system power and have some delay*/
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_ForceSaveDataIntoFlash();
            eAudioChannel channel = AUDIO_CHANNEL_INVALID;
            channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
            if (channel == MAIN_BT_SOURCE)
            {
                bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                MainApp_SettingBtOffState(me);
                bIsStartAfterFactoryReset = FALSE;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }
            else
            {
                MainApp_SwitchMode(MainApp, STANDBY_MODE);
            }
            MainApp_PublishSystemStatus(MainApp, SYSTEM_SLEEP_STA);

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (bt_powering_off_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bt_powering_off_delay_timeout > 0)
                {
                    bt_powering_off_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_powering_off_delay_timeout = TIMER_IS_NOT_SETUP;
                    MainApp_SwitchMode(MainApp, STANDBY_MODE);
                }
            }
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
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
/* repeat powerinit sequence without touching bt reset pin */
QState MainApp_AwakeningWithoutBT(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            awake_without_bt_init_timer = PowerDrv_AwakePower(&powerDrv);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            awake_without_bt_init_timer -= MAIN_APP_TIMEOUT_IN_MS;
            if ( awake_without_bt_init_timer <= 0 )
            {
                awake_without_bt_init_timer = PowerDrv_AwakePower(&powerDrv);
                if(awake_without_bt_init_timer <= 0)
                {

                    return Q_TRAN(&MainApp_Resuming);
                }
                else
                {
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
            }
            else
            {
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }

            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == VOLUME_KNOB_BASE_KEY_ID) || (evt->keyId == BASS_KNOB_BASE_KEY_ID) || (evt->keyId == TREBLE_KNOB_BASE_KEY_ID))
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
            }
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
/* pre-state before powering up, we need to shutdown audio srv first*/
QState MainApp_Awakening(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            return Q_TRAN(&MainApp_PoweringUp);
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == VOLUME_KNOB_BASE_KEY_ID) || (evt->keyId == BASS_KNOB_BASE_KEY_ID) || (evt->keyId == TREBLE_KNOB_BASE_KEY_ID))
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
            }
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
            MainApp_SetLedPattern(LED_BT, LED_PATTERN_NONE, 0x0);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE, 0x0);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE, 0x0);

            prevChannel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
            PowerDrv_SetPowerForStandby(TRUE);
            SetEnableWakeupSourcesFromStandby();
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case POWER_MCU_SLEEP_SIG:
        {
            SetEnableWakeupSourcesFromStandby();
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            BtStatusEvt *evt = (BtStatusEvt*)e;
            if (evt->btStatus == BT_CONNECTED_STA)
            {
                bluetooth_connect_wakeup_delay_timeout = TIMER_IS_NOT_SETUP;
                SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                PowerDrv_SetPowerForStandby(FALSE);
                return Q_TRAN(&MainApp_AwakeningWithoutBT);
            }
            else
            {
                bluetooth_connect_wakeup_delay_timeout = BLUETOOTH_CONNECT_WAKEUP_DELAY_MS;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }
            return Q_HANDLED();
        }
        case POWER_SWITCH_SIG:
        {
            if (power_key_switch_check_delay_timeout == TIMER_IS_NOT_SETUP)
            {
                power_key_switch_check_delay_timeout = POWER_KEY_SWITCH_CHECK_DELAY_MS;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }

            return Q_HANDLED();
        }
        case ENERGYSENSE_SIG:
        {
            uint8 mode = GetAdcMode();
            if (mode == PCM1862_SLEEP_MODE)
            {   /* this is signal detection */
                /* got music detect, reset idle timer */
                RESET_INACTIVITY;
                isMusicDetected  = TRUE;
                PowerDrv_SetPowerForStandby(FALSE);
                /* this can be only in aux or rca mode*/
                return Q_TRAN(&MainApp_Awakening);
            }
            SetEnableWakeupSourcesFromStandby();
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == VOLUME_KNOB_BASE_KEY_ID) || (evt->keyId == BASS_KNOB_BASE_KEY_ID) || (evt->keyId == TREBLE_KNOB_BASE_KEY_ID))
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
                return Q_HANDLED();
            }
            if((evt->keyId == POWER_KEY)&& (evt->keyEvent == KEY_EVT_UP))
            { /* TODO: check can we switch to Q_TRAN(&MainApp_PoweringDown) instead*/
                return Q_TRAN(MainApp_IsBtChannel(me));
            }
            if (((evt->keyId == INPUT_KEY) || (evt->keyId == BT_KEY)) && (evt->keyEvent == KEY_EVT_SHORT_PRESS) )
            {
                SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                PowerDrv_SetPowerForStandby(FALSE);
                if (channelSwitchTo == MAIN_BT_SOURCE)
                {
                    return Q_TRAN(&MainApp_AwakeningWithoutBT);
                }
                else
                {
                    return Q_TRAN(&MainApp_Awakening);
                }
            }

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (power_key_switch_check_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (power_key_switch_check_delay_timeout > 0)
                {
                    power_key_switch_check_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
                else
                {
                    power_key_switch_check_delay_timeout = TIMER_IS_NOT_SETUP;
                    /* in case power switch key will be turn to off and on very quickly */
                    if (PowerDrv_IsMainSwitchOn())
                    {
                        SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
                        PowerDrv_SetPowerForStandby(FALSE);
                        if (channelSwitchTo == MAIN_BT_SOURCE)
                        {
                            return Q_TRAN(&MainApp_AwakeningWithoutBT);
                        }
                        else
                        {
                            return Q_TRAN(&MainApp_Awakening);
                        }

                    }
                    else
                    {/* switching to standby mode*/
                        QTimeEvt_disarm(TIME_EVT_OF(me));
                         return Q_TRAN(&MainApp_PoweringDown);
                    }
                }
            }
            if (bluetooth_connect_wakeup_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (bluetooth_connect_wakeup_delay_timeout > 0)
                {
                    bluetooth_connect_wakeup_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
                else
                {
                    bluetooth_connect_wakeup_delay_timeout = TIMER_IS_NOT_SETUP;
                    SetEnableWakeupSourcesFromStandby();
                    PowerDrv_PowerSaveSleep();
                }
            }

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


QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            SetDisableSignalLossDetectInt();
            MainApp_SetLedPattern(LED_BT, LED_PATTERN_NONE, 0x0);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE, 0x0);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE, 0x0);

            prevChannel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
            MainApp_SetWakeUp(me);
            if (PowerDrv_IsMainSwitchOn())
            {/* in case power switch was switch back, while we doing shutdown */
                power_key_switch_check_delay_timeout = 0;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }
            else
            {
                PowerDrv_PowerSaveSleep();
            }
            return Q_HANDLED();
        }
        case POWER_MCU_SLEEP_SIG:
        {
            MainApp_SetWakeUp(me);
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case POWER_SWITCH_SIG:
        {
            if (power_key_switch_check_delay_timeout == TIMER_IS_NOT_SETUP)
            {
                power_key_switch_check_delay_timeout = POWER_KEY_SWITCH_CHECK_DELAY_MS;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }

            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (power_key_switch_check_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (power_key_switch_check_delay_timeout > 0)
                {
                    power_key_switch_check_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
                else
                {
                    power_key_switch_check_delay_timeout = TIMER_IS_NOT_SETUP;
                    if (PowerDrv_IsMainSwitchOn())
                    {
                        PowerDrv_Ctor(&powerDrv);
                        return Q_TRAN(&MainApp_PoweringUp);
                    }
                    else
                    {
                        QTimeEvt_disarm(TIME_EVT_OF(me));
                        MainApp_SetWakeUp(me);
                        PowerDrv_PowerSaveSleep();
                    }
                }
            }

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

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}
static void MainApp_SwitchToPreviousSource(cMainApp * const me)
{
    if ((Setting_IsReady(SETID_CHANNEL)) && (Setting_IsIdValid(SETID_CHANNEL)))
    {
        prevChannel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
    }
    else
    {
        eAudioChannel channel = MAIN_BT_SOURCE;
        Setting_Set(SETID_CHANNEL, &channel);
        prevChannel = MAIN_BT_SOURCE; // default source after hard reset
    }
    channelSwitchTo = prevChannel;
    switch (prevChannel)
    {
        case AUDIO_CHANNEL_1:
            MainApp_SetDisplaySource(SOURCE_STATE_AUX);
            MainApp_SwitchSource(me, MAIN_AUX_SOURCE);
            setMaskForChannel = MAIN_AUX_SOURCE;
            break;
        case AUDIO_CHANNEL_RCA:
            MainApp_SetDisplaySource(SOURCE_STATE_RCA);
            MainApp_SwitchSource(me, MAIN_RCA_SOURCE);
            setMaskForChannel = MAIN_RCA_SOURCE;
            break;
        case AUDIO_CHANNEL_2:
        default:
        {
            prevChannel = MAIN_BT_SOURCE;
            channelSwitchTo = prevChannel;
            eBtStatus btStatus = BT_MAX_STA;
            if (Setting_IsReady(SETID_BT_STATUS))
            {
                btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
            }
            switch(btStatus)
            {
                case BT_STREAMING_A2DP_STA:
                case BT_CONNECTED_STA:
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTED);
                    break;
                case BT_DISCOVERABLE_STA:
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_PAIRING);
                    break;
                case BT_CONNECTABLE_STA:
                default: /* as by default after powering on bt start to reconnect to pdl */
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_RECONNECTING);
                    bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS + BLUETOOTH_STARTUP_DELAY_MS;
                    break;
            }
            MainApp_SwitchSource(me, MAIN_BT_SOURCE);
            setMaskForChannel = MAIN_BT_SOURCE;
            bluetooth_startup_delay_timeout = BLUETOOTH_STARTUP_DELAY_MS;
            break;
        }
    }
}
/* initial variable for power up system */
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me)
{
    myBtStatus = BT_MAX_STA;
    isRePairing = FALSE;
    isLinkLossHappened = FALSE;
    isPairingStateContinue = FALSE;
    isMusicDetected = FALSE;
    bIsBtPostponedCmd = FALSE;
    bIsInputSwitchingNow = FALSE;
    keyPressed = INVALID_KEY;

    SetSender((QActive*)me); // provide sender pointer for getting feedback by ENERGYSENSE_SIG
    SetSignalLossDetectInt();
}

static void MainApp_ForceSaveDataIntoFlash()
{
    SettingFlashReqEvt* pReq = Q_NEW(SettingFlashReqEvt, SETTING_FLASH_REQ_SIG);
    pReq->bIsSave = TRUE;
    pReq->sender = NULL;
    SendToServer(SETTING_SRV_ID, (QEvt*)pReq);
}
static void MainApp_FinishCalibration()
{
    if (nKnobKeyCalibrationCount >= KNOB_KEYS_NUM)
    {
        if (calibrationMode == CALIBRATION_MIN)
        {
            DebugSSrv_PrintStr("ADC MIN DATA SAVING TO FLASH.....");
        }
        else
        {
            if (calibrationMode == CALIBRATION_MAX)
            {
                DebugSSrv_PrintStr("ADC MAX DATA SAVING TO FLASH.....");
            }
            else
            {
                ASSERT(0);
            }
            calibrationMode = CALIBRATION_NONE;
            nKnobKeyCalibrationCount = 0;
            MainApp_ForceSaveDataIntoFlash();
        }
    }
}

/* initial variable for factory reset */
static void MainApp_StartCalibration(cMainApp * const me)
{
    KeyDebugReqEvt* kDReq = Q_NEW(KeyDebugReqEvt, KEY_DEBUG_REQ_SIG);
    kDReq->req = DEBUG_RAW_DATA_REQ;
    kDReq->keyId = VOLUME_KNOB_BASE_KEY_ID;
    kDReq->sender = (QActive*)me;
    SendToServer(KEY_SRV_ID, (QEvt*)kDReq);

    KeyDebugReqEvt* kDReq0 = Q_NEW(KeyDebugReqEvt, KEY_DEBUG_REQ_SIG);
    kDReq0->req = DEBUG_RAW_DATA_REQ;
    kDReq0->keyId = BASS_KNOB_BASE_KEY_ID;
    kDReq0->sender = (QActive*)me;
    SendToServer(KEY_SRV_ID, (QEvt*)kDReq0);

    KeyDebugReqEvt* kDReq1 = Q_NEW(KeyDebugReqEvt, KEY_DEBUG_REQ_SIG);
    kDReq1->req = DEBUG_RAW_DATA_REQ;
    kDReq1->keyId = TREBLE_KNOB_BASE_KEY_ID;
    kDReq1->sender = (QActive*)me;
    SendToServer(KEY_SRV_ID, (QEvt*)kDReq1);
}

static void MainApp_SaveCalibrationValue(eKeyID key_id, int16 rawData)
{
    switch(key_id)
    {
        case VOLUME_KNOB_BASE_KEY_ID:
        {
            if (calibrationMode == CALIBRATION_MIN)
            {
                Setting_Set(SETID_VOLUME_ADC_MIN, &rawData);
            }
            else
            {
                if (calibrationMode == CALIBRATION_MAX)
                {
                    Setting_Set(SETID_VOLUME_ADC_MAX, &rawData);
                }
            }
            break;
        }
        case BASS_KNOB_BASE_KEY_ID:
        {
            if (calibrationMode == CALIBRATION_MIN)
            {
                Setting_Set(SETID_BASS_ADC_MIN, &rawData);
            }
            else
            {
                if (calibrationMode == CALIBRATION_MAX)
                {
                    Setting_Set(SETID_BASS_ADC_MAX, &rawData);
                }
            }
            break;
        }
        case TREBLE_KNOB_BASE_KEY_ID:
        {
            if (calibrationMode == CALIBRATION_MIN)
            {
                Setting_Set(SETID_TREBLE_ADC_MIN, &rawData);
            }
            else
            {
                if (calibrationMode == CALIBRATION_MAX)
                {
                    Setting_Set(SETID_TREBLE_ADC_MAX, &rawData);
                }
            }
            break;
        }
        default:
            break;
    }
}

/* initial variable for factory reset */
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me)
{
    eBtStatus btStatus = BT_CONNECTABLE_STA;
    Setting_Set(SETID_BT_STATUS, &btStatus);
    SetSender((QActive*)me); // provide sender pointer for getting feedback by ENERGYSENSE_SIG and POWER_SWITCH_SIG
    bIsResumeFromSleep = FALSE;
    bIsStartAfterFactoryReset = FALSE;

    int16   defaultAdc;
    defaultAdc = DEFAULT_ADC_MAX;
    Setting_Set(SETID_VOLUME_ADC_MAX, &defaultAdc);
    Setting_Set(SETID_BASS_ADC_MAX, &defaultAdc);
    Setting_Set(SETID_TREBLE_ADC_MAX, &defaultAdc);
    defaultAdc = DEFAULT_ADC_MIN;
    Setting_Set(SETID_VOLUME_ADC_MIN, &defaultAdc);
    Setting_Set(SETID_BASS_ADC_MIN, &defaultAdc);
    Setting_Set(SETID_TREBLE_ADC_MIN, &defaultAdc);
}

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus)
{
    SystemStatusEvt *pe = Q_NEW(SystemStatusEvt, SYSTEM_STATE_SIG);
    pe->systemStatus = systemStatus;
    QF_PUBLISH(&pe->super, me);
}

static bool MainApp_SetDisplaySource(eSourceState sourceState)
{
    switch(sourceState)
    {
        case SOURCE_STATE_BT_CONNECTABLE:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_025_050_X2_2000_OFF, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE,  0xFFFFFFFF);
            break;
        }
        case SOURCE_STATE_BT_PAIRING:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_050_1, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE,  0xFFFFFFFF);
            break;
        }
        case SOURCE_STATE_BT_RECONNECTING:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_025_050,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE, 0xFFFFFFFF);
            break;
        }
        case SOURCE_STATE_BT_CONNECTED:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_SOLID, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE,  0xFFFFFFFF);
            break;
        }
        case SOURCE_STATE_AUX:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_SOLID, 0xFFFFFFFF);
            break;
        }
        case SOURCE_STATE_RCA:
        {
            MainApp_SetLedPattern(LED_BT,  LED_PATTERN_NONE,  0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RCA, LED_PATTERN_SOLID, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_AUX, LED_PATTERN_NONE,  0xFFFFFFFF);
            break;
        }
        default:
            break;
    }
}

static bool MainApp_SetLedOnOff(eLedName led, uint8 OnOff)
{
    leds[led].ledOn = OnOff;

    switch(led)
    {
        case LED_AUX:
            if(OnOff)
                {LED_AUX_ON;}
            else
                {LED_AUX_OFF;}
            break;
        case LED_RCA:
            if(OnOff)
                {LED_RCA_ON;}
            else
                {LED_RCA_OFF;}
            break;
        case LED_BT:
            if(OnOff)
                {LED_BT_ON;}
            else
                {LED_BT_OFF;}
            break;
        default:
            break;
    }
}

static bool MainApp_SetLedPattern(eLedName led, eLedPatternType pattern, uint32 timer)
{
    leds[led].patternId = pattern;
    leds[led].timer = timer;

    if (LED_PATTERN_NONE == pattern)
    {
        MainApp_SetLedOnOff(led, OFF);
    }

    return TRUE;
}

static bool MainApp_AllLEDsOff()
{
    uint8 ii;

    for(ii = 0; ii < LED_COUNT; ii++)
    {
        MainApp_SetLedPattern(ii, LED_PATTERN_NONE, 0);
    }
}

static void MainApp_UpdateLedState()
{
    uint8 ii;

    for(ii = 0; ii < LED_COUNT; ii++)
    {
        leds[ii].timer -= MAIN_APP_TIMEOUT_IN_MS;

        if(0 == leds[ii].timer)
        {
            leds[ii].patternId = LED_PATTERN_NONE;
        }

        switch(leds[ii].patternId)
        {
            case LED_PATTERN_NONE:
            {
                if(leds[ii].ledOn)
                {
                    MainApp_SetLedOnOff(ii, OFF);
                }
                break;
            }
            case LED_PATTERN_SOLID:
            {
                if(!leds[ii].ledOn)
                {
                    MainApp_SetLedOnOff(ii, ON);
                }
                break;
            }
            case LED_PATTERN_350_4:
            {
                if((leds[ii].timer % 4000) < 500)
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                else
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                break;
            }
            case LED_PATTERN_350_4_R:
            {
                if((leds[ii].timer % 4000) < 3500)
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                else
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                break;
            }
            case LED_PATTERN_025_050_X2_2000_OFF:
            {
                if((leds[ii].timer % 3000) < 250
                 ||((leds[ii].timer % 3000) < 750 && (leds[ii].timer % 3000) >= 500))
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                else
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                break;
            }
            case LED_PATTERN_1_2:
            {
                if((leds[ii].timer % 2000) < 1000)
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                else
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                break;
            }
            case LED_PATTERN_025_050:
            {
                if((leds[ii].timer % 500) < 250)
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                else
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                break;
            }
            case LED_PATTERN_050_1:
            {
                if((leds[ii].timer % 1000) < 500)
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                else
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                break;
            }
            case LED_PATTERN_050_1_R:
            {
                if((leds[ii].timer % 1000) < 500)
                {
                    if(!leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, ON);
                    }
                }
                else
                {
                    if(leds[ii].ledOn)
                    {
                        MainApp_SetLedOnOff(ii, OFF);
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case BT_KEY:
        {
            channelSwitchTo = MAIN_BT_SOURCE;
            break;
        }
        case INPUT_KEY:
        {
            switch(channelSwitchTo)
            {
                case MAIN_AUX_SOURCE:
                    MainApp_SetDisplaySource(SOURCE_STATE_RCA);
                    channelSwitchTo = MAIN_RCA_SOURCE;
                    break;
                case MAIN_BT_SOURCE:
                    MainApp_SetDisplaySource(SOURCE_STATE_AUX);
                    channelSwitchTo = MAIN_AUX_SOURCE;
                    break;
                case MAIN_RCA_SOURCE:
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_RECONNECTING);
                    channelSwitchTo = MAIN_BT_SOURCE;
                    break;
                default:
                    break;
            }
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            calibrationMode = CALIBRATION_MIN;
            nKnobKeyCalibrationCount = 0;
            MainApp_StartCalibration(me);
            break;
        }
        case VOLUME_UP_KEY:
        {
            calibrationMode = CALIBRATION_MAX;
            nKnobKeyCalibrationCount = 0;
            MainApp_StartCalibration(me);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
    keyPressed = e->keyId;
    button_reaction_delay_timeout = BUTTON_REACTION_DELAY_MS;
    SetEnergySenseMask(AUDIO_CHANNEL_INVALID);
}

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyId)
    {
        case VOLUME_KNOB_BASE_KEY_ID:
        {
            int8 new_volume_level = evt->index;
            AudioSrv_SetVolume(new_volume_level);
            char buffer[MAX_PRINTSTR_SIZE];
#ifdef HAS_DEBUG
            sprintf(buffer, "\nvolume : %d (%d)>>>", new_volume_level, evt->adcRawValue);
#else
            sprintf(buffer, "\nvolume : %d >>>", new_volume_level);
#endif
            DebugSSrv_PrintStr(buffer);
            return;
        }
        case BASS_KNOB_BASE_KEY_ID:
        {
            int8 new_bass_level = MIN_BASS + evt->index;
            AudioSrv_SetBass(new_bass_level);
            char buffer[MAX_PRINTSTR_SIZE];
#ifdef HAS_DEBUG
            sprintf(buffer, "\nbass : %d (%d)>>>", new_bass_level, evt->adcRawValue);
#else
            sprintf(buffer, "\nbass : %d >>>", new_bass_level);
#endif
            DebugSSrv_PrintStr(buffer);
            return;
        }
        case TREBLE_KNOB_BASE_KEY_ID:
        {
            int8 new_treble_level = MIN_TREBLE + evt->index;
            AudioSrv_SetTreble(new_treble_level);
            char buffer[MAX_PRINTSTR_SIZE];
#ifdef HAS_DEBUG
            sprintf(buffer, "\ntreble : %d (%d)>>>", new_treble_level, evt->adcRawValue);
#else
            sprintf(buffer, "\ntreble : %d >>>", new_treble_level);
#endif
            DebugSSrv_PrintStr(buffer);
            return;
        }
        default:
        break;
    }
    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            break;
        case KEY_EVT_DOWN:
            break;
        case KEY_EVT_SHORT_PRESS:
            MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            break;
        case KEY_EVT_HOLD:
            break;
        case KEY_EVT_REPEAT:
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            break;
        default:
            break;
    }
}

static void MainApp_SetWakeUp(cMainApp * const me)
{
    PowerDrv_RegisterIntEvent((QActive*)me);
    PowerDrv_Xtor(&powerDrv);
}

static void MainApp_ParseBTEvent(cMainApp * const me, QEvt const * const e)
{
    BtStatusEvt *evt = (BtStatusEvt*)e;
    if(!evt->isBtStatus)
    { /* leave if it's not BT status*/
        switch(evt->btIndEvt)
        {   /*PAIRING FAIL is same as PAIRING TIMEOUT, but different from LINK LOST*/
            case BT_SCL_CONNECTED_EVT:
            {
                isLinkLossHappened = FALSE;
                if (channelSwitchTo == MAIN_BT_SOURCE)
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTED);
                }
                break;
            }
            case BT_PAIRING_FAIL_EVT:
            {
                MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTABLE);
                myBtStatus = BT_CONNECTABLE_STA;
                break;
            }
            case BT_LINK_LOSS_EVT:
            {
                isLinkLossHappened = TRUE;
                bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS + 5000;
                MainApp_SetDisplaySource(SOURCE_STATE_BT_RECONNECTING);
                MainApp_SetBtNoStream(me);

                break;
            }
            default:
                break;
        }
        return;
    }
    if ((myBtStatus != evt->btStatus))
    {
        switch(evt->btStatus)
        {
            case BT_CONNECTABLE_STA:
            { // TODO: check if here we can play disconnect tone if previus state was connected
                if (((myBtStatus == BT_CONNECTED_STA) || (myBtStatus == BT_STREAMING_A2DP_STA))&& !bIsFirstBtState && !isLinkLossHappened)
                {
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                }
                bIsFirstBtState = FALSE;
                isLinkLossHappened = FALSE;
                MainApp_SetBtNoStream(me);

                break;
            }
            case BT_DISCOVERABLE_STA:
            {
                bIsFirstBtState = FALSE;
                if (channelSwitchTo == MAIN_BT_SOURCE)
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_PAIRING);
                }
                MainApp_SetBtNoStream(me);

                break;
            }
            case BT_CONNECTED_STA:
            {
                MainApp_SetBtNoStream(me);
                break;
            }
            case BT_STREAMING_A2DP_STA:
            {
                if (channelSwitchTo == MAIN_BT_SOURCE)
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTED);
                    RESET_INACTIVITY;
                    isMusicDetected  = TRUE;
                }

                break;
            }
            default:
            {
                break;
            }
        }
        myBtStatus = evt->btStatus;
    }
}
/***********************************************/
static QStateHandler* MainApp_IsBtChannel(cMainApp * const me)
{
    eAudioChannel channel = AUDIO_CHANNEL_INVALID;
    channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);

    if (channel == MAIN_BT_SOURCE)
    {
        return (QStateHandler*)(&MainApp_BtDisconnect);
    }
    else
    {
        return (QStateHandler*)(&MainApp_PoweringDown);
    }
}
static void MainApp_SwitchSource(cMainApp * const me, eAudioChannel channel)
{
    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = channel;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
}

static void MainApp_SwitchInputSource(cMainApp * const me, eAudioChannel channel)
{
    switch(channel)
    {
        case MAIN_RCA_SOURCE:
        case MAIN_AUX_SOURCE:
        {
            if (bt_powering_on_delay_timeout == TIMER_IS_NOT_SETUP)
            {
                /* when bt already started, otherwise ignore it */
                if (bluetooth_startup_delay_timeout != TIMER_IS_NOT_SETUP)
                {
                    bluetooth_startup_delay_timeout  = TIMER_IS_NOT_SETUP;
                }
                /* turning off bt */
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                MainApp_SettingBtOffState(me);
                bIsStartAfterFactoryReset = FALSE;
                eBtStatus btStatus = BT_CONNECTABLE_STA;
                Setting_Set(SETID_BT_STATUS, &btStatus);
                bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                bt_reconnecting_delay_timeout = TIMER_IS_NOT_SETUP;
                /*_________________________*/
                bluetooth_startup_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
            }
            else
            {
                bIsBtPostponedCmd = TRUE;
            }
            break;
        }
        case MAIN_BT_SOURCE:
        {
            if (bt_powering_off_delay_timeout == TIMER_IS_NOT_SETUP)
            {
                eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                if ((btStatus == BT_CONNECTED_STA) || (btStatus == BT_STREAMING_A2DP_STA))
                {
                    MainApp_SetDisplaySource(SOURCE_STATE_BT_CONNECTED);
                }
                else
                {
                /* turning on bt */
                    bIsFirstBtState = TRUE;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_ON_CMD);
                    bt_reconnecting_delay_timeout = BT_RECONNECTING_DELAY_MS;
                    bt_powering_on_delay_timeout = BT_POWERING_ON_DELAY_MS;
                    bluetooth_startup_reconnect_delay_timeout = BLUETOOTH_STARTUP_RECONNECT_DELAY_MS;
                }
            }
            else
            {
                bIsBtPostponedCmd = TRUE;
            }
            break;
        }
        default:
            break;
    }
    MuteAdc(TRUE);
    setMaskForChannel = channel;
    MainApp_SwitchSource(me, channel);
    isMusicDetected = FALSE;
}

static void MainApp_SettingBtOffState(cMainApp * const me)
{
    eBtStatus btStatus = BT_MAX_STA;
    Setting_Set(SETID_BT_STATUS, &btStatus);
}

/* when receive one these bt states:
    BT_LINK_LOSS_EVT
    BT_CONNECTABLE_STA
    BT_DISCOVERABLE_STA
    BT_CONNECTED_STA
  that indicates stop bt streaming and if
    BT_STREAMING_A2DP_STA,- indicates start bt streaming
*/
static void MainApp_SetBtNoStream(cMainApp * const me)
{
    if (channelSwitchTo == MAIN_BT_SOURCE)
    {
        if (isMusicDetected)
        {
            RESET_INACTIVITY;
            isMusicDetected  = FALSE;
        }
    }
}
