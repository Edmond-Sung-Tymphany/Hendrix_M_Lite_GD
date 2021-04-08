/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        MainApp.c
@brief       Main application for MGT
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

typedef enum
{
    SYS_STATE_NORMAL,
    SYS_STATE_STANDBY,
    SYS_STATE_SLEEP,
} eMainAppSysState;

eMainAppSysState sysState;


#define CAST_ME cMainApp * MainApp = (cMainApp *) me;

static cPowerDrv powerDrv;
/* Battery variables */
/* the time (ms) per timeout signal */

#define MAINAPP_POWER_TIMEOUT_IN_MS  10

static tBatteryInfo batteryInfo;
static uint8 delayCheckChargingCount = 0;
static bool bIsChargingError = FALSE;
static tBatteryFilter battFilter =
{
    .count = 0,
    .isReady = FALSE,
    .intBatt.isRemovable = FALSE,
};
#define INVALID_BATT_STATE  0XFF

#ifdef DECREASING_ORDER_STATE
#define HIGHEST_STATE     0
#define LOWEST_STATE      (NUMBER_OF_STATE-1)
#endif

#ifdef INCREASING_ORDER_STATE
#define HIGHEST_STATE     NUMBER_OF_STATE-1)
#define LOWEST_STATE      0
#endif

#define BATT_STATUS_CHECK_PERIOD_MS        5000

#define BATT_90PER_VOLT                     12020   // 12.02V
#define BATT_50PER_VOLT                     10990   // 10.99V

#define BATT_05PER_VOLT                     10270   // 10.27 V

#define CRITICAL_LOW_BATT_VOLT               9900 //   9.9V

static uint16 batt_status_check_timeout = 0;
#define AC_IN_OUT_CHECK_PERIOD_MS           500

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
#define GAIN_ADJUSTMENT_STATUS_CHECK_PERIOD_MS  1000
static uint16 gain_adjustment_status_check_timeout = 0;
static eAudioPowerGainLevel prevGainLevel;
#endif

static uint16 ac_in_out_check_timeout = 0;

#define TIMER_IS_NOT_SETUP              -1
#define POWERING_OFF_DELAY_MS           2000//1200
static int16 powering_off_delay_timeout = TIMER_IS_NOT_SETUP;

#define BT_POWERING_OFF_DELAY_MS           1000 //1200 // recommend by mr Wang 800//600//delay after send BT off command to make sure we close bt connection
static int16 bt_powering_off_delay_timeout = TIMER_IS_NOT_SETUP;

#define STOP_BTSTREAM_DELAY_MS            800
static int16 stopping_btstream_delay_timeout = TIMER_IS_NOT_SETUP;
#define STOP_BTSTREAM_DEADLOOP_TIMEOUT_MS       6000 // prevent dead loop after 6 sec force to continue powering off
static int16 stopping_btstream_deadloop_timeout = TIMER_IS_NOT_SETUP;

#define STARTUP_DELAY_MS           2500 // delay after  bt start and we send cmd to play power on audio cue
static int16 startup_delay_timeout = TIMER_IS_NOT_SETUP;

static bool bIsFirstBtMsg = TRUE;


#define RECONNECT_DELAY_MS          5000
static int16 reconnect_delay_timeout = TIMER_IS_NOT_SETUP;

/*we sould wait after bt link is lost to play disconnect tone before we try to reconnect */
#define PRE_RECONNECT_DELAY_MS          1200
#define SHORT_PRE_RECONNECT_DELAY_MS    20
static int16 pre_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;

#define INACTIVITY_DELAY_MS        891000  // ~ 15 min (after 15 min inactivity switch to standby\off mode)
static int32 inactivity_timeout = TIMER_IS_NOT_SETUP;
#define RESET_INACTIVITY    inactivity_timeout = INACTIVITY_DELAY_MS
static bool isMusicDetected = FALSE;
static bool isRePairing = FALSE;
static bool isLinkLossHappened = FALSE;
static bool isPairingStateContinue = FALSE;

static int16 playing_connect_tone_delay_timeout = TIMER_IS_NOT_SETUP;
#define CONNECT_TONE_DELAY_TIME_MS  1000 // 2500  /*Delay to play connect tone after 1sec*/

static int16 playing_disconnect_tone_delay_timeout = TIMER_IS_NOT_SETUP;
#define DISCONNECT_TONE_DELAY_TIME_MS  1400  // 2000 // try 1000
#define DISCONNECT_TONE_LEN_MS         1000


static int16 factory_reset_flashing_delay_timeout = TIMER_IS_NOT_SETUP;
#define FACTORY_RESET_FLASHING_DELAY_TIME_MS  2000

static bool bIsNeedToPlayDisconnectTone;

static uint8 save_volume_level = DEFAULT_VOLUME;
/*-----------------------------------------------------------------*/

static int32       power_init_timer;

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
static eAudioPowerDrcLevel prevPowerDrcLevel = AUDIO_MAX_DRC_LEVEL;
#endif
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* Private functions / variables. */

/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[5];

#define LED_DRV_TEST

#ifdef LED_DRV_TEST
static bool bIsLEDB_ON;
static bool bIsLEDR_ON;

#include "GpioDrv.h"
static cGpioDrv ledGpiodrv;
#define _LEDR_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_21)
#define LEDR_ON   _LEDR_ON; bIsLEDR_ON = TRUE
#define _LEDR_OFF  GpioDrv_SetBit(&ledGpiodrv,GPIO_21)
#define LEDR_OFF   _LEDR_OFF; bIsLEDR_ON = FALSE
#define _LEDB_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_22)
#define LEDB_ON   _LEDB_ON; bIsLEDB_ON = TRUE
#define _LEDB_OFF  GpioDrv_SetBit(&ledGpiodrv,GPIO_22)
#define LEDB_OFF   _LEDB_OFF; bIsLEDB_ON = FALSE

#else
#define LEDR_ON
#define LEDR_OFF
#define LEDB_ON
#define LEDB_OFF
#endif

static eBtStatus     myBtStatus;
static eMainAppLedFlashingState ledFlashingState = LED_FLASHING_NONE;
static int16 led_flashing_timeout = TIMER_IS_NOT_SETUP;
static uint16 led_flashing_periods_ms[LED_FLASHING_MAX-1] = {500, 250, 50, 50, 250};


#define LED_FLASHING_COUNT  5
#define LED_FLASHING_COUNT_IS_NOT_SETUP     -1
#define LED_FLASHING_COUNT_INDEFINITE       0xFF
static int16 led_flashing_idx = LED_FLASHING_COUNT_IS_NOT_SETUP;

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    PowerDrv_Ctor(&powerDrv);
#ifdef LED_DRV_TEST
    tGPIODevice *pLedGPIOConf;
    uint16 attached_device_index = 0;
    pLedGPIOConf = (tGPIODevice*)getDevicebyId(LED_DEV_ID,&attached_device_index);
    GpioDrv_Ctor(&ledGpiodrv,pLedGPIOConf);
    LEDR_ON;
    LEDB_OFF;
#endif
    /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                     MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);


    /* Subscribe */
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
#endif
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*) me, BT_STATE_SIG);
#endif
    QActive_subscribe((QActive*) me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*) me, AUDIO_MUSIC_STREAM_STATE_SIG);
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
    MainApp_InitialBattVariable(me);
    MainApp_Update(me);
    return Q_TRAN(&MainApp_PoweringUp);
}

QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            sysState = SYS_STATE_STANDBY; // while system is starting
            LEDR_ON;
            LEDB_OFF;
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
        case SYSTEM_MODE_RESP_SIG:
        {
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
            LEDB_OFF;
            LEDR_ON;
            batt_status_check_timeout =  BATT_STATUS_CHECK_PERIOD_MS;
#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
            gain_adjustment_status_check_timeout = GAIN_ADJUSTMENT_STATUS_CHECK_PERIOD_MS;
#endif
            RESET_INACTIVITY;
          MainApp_InitialVariablesForPowerUp(me);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);

            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            BtStatusEvt *evt = (BtStatusEvt*)e;
            if ((evt->btStatus == BT_CONNECTABLE_STA) && bIsFirstBtMsg)
            {/* bt is started and in connectable mode */
                bIsFirstBtMsg = FALSE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_TONE_RESET_PDL_CMD); // play power on audio cue
                startup_delay_timeout = STARTUP_DELAY_MS;
            }
            MainApp_ParseBTEvent(me, e);
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            AudioMusicDetectStateEvt* audio_state_evt = (AudioMusicDetectStateEvt*)e;
            isMusicDetected = audio_state_evt->hasMusicStream;
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
          RESET_INACTIVITY;
          if (sysState == SYS_STATE_NORMAL)
          {
            MainApp_SetNoneLedFlashing();
            MainApp_UpdateLedFlashing();

            eAudioChannel channel;
            AudioStateEvt* evt = (AudioStateEvt*)e;
            if (evt->IsJackIn)
            {
                channel = AUXIN_CHANNEL;
            }
            else
            {
                channel = BT_CHANNEL;
                eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                if (btStatus == BT_CONNECTABLE_STA)
                {
                    MainApp_StartReconnectLedFlashing();
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                    reconnect_delay_timeout = RECONNECT_DELAY_MS;
                }
                else if (btStatus == BT_DISCOVERABLE_STA)
                {
                    MainApp_StartPairingLedFlashing();
                }
            }
            MainApp_SwitchSource(me, channel);
          }
            return Q_HANDLED();
        }

        case KEY_STATE_SIG:
        {
            RESET_INACTIVITY;
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == POWER_KEY)&& (evt->keyEvent == KEY_EVT_SHORT_PRESS))
            {
                if (sysState == SYS_STATE_NORMAL)
                {
                    sysState = SYS_STATE_STANDBY;
                return Q_TRAN(MainApp_IsBtConnected(me));
            }
            }
            MainApp_ParseKeyEvent(me, e);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if(inactivity_timeout != TIMER_IS_NOT_SETUP)
            {
                if (MainApp_IsSystemIdle(me))
                {
                    inactivity_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    if(inactivity_timeout <= 0)
                    {
                        tBatteryInfo battInfo = *(tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                        if ((TRUE == battInfo.inputSourceState.isUsbPlugIn) && (battInfo.chargerState == CHARGER_BATT_STA))
                        {
                            RESET_INACTIVITY;
                        }
                        else
                        {
                            inactivity_timeout = TIMER_IS_NOT_SETUP;
                            return Q_TRAN(MainApp_IsBtConnected(me));
                        }
                    }
                }
                else
                {
                    RESET_INACTIVITY;
                }
            }

            if (pre_reconnect_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (pre_reconnect_delay_timeout > 0)
                {
                    pre_reconnect_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    pre_reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                    BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                    if (!auxinStatus)
                    {
                        MainApp_StartReconnectLedFlashing();
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                        reconnect_delay_timeout = RECONNECT_DELAY_MS;
                    }
                }
            }
            if (factory_reset_flashing_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (factory_reset_flashing_delay_timeout > 0)
                {
                    factory_reset_flashing_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    factory_reset_flashing_delay_timeout = TIMER_IS_NOT_SETUP;
                    MainApp_SetNoneLedFlashing();
                    MainApp_UpdateLedFlashing();
                }
            }

            if (reconnect_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (reconnect_delay_timeout > 0)
                {
                    reconnect_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                    MainApp_SetNoneLedFlashing();
                    MainApp_UpdateLedFlashing();
                }
            }
            if (playing_connect_tone_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (playing_connect_tone_delay_timeout > 0)
                {
                    playing_connect_tone_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    playing_connect_tone_delay_timeout = TIMER_IS_NOT_SETUP;
                    BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                    if ((!auxinStatus) && ((btStatus == BT_CONNECTED_STA) || (btStatus == BT_STREAMING_A2DP_STA)))
                    {
                        AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_CONNECTED_CMD);
                    }
                }
            }
            if (playing_disconnect_tone_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (playing_disconnect_tone_delay_timeout > 0)
                {
                    playing_disconnect_tone_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    playing_disconnect_tone_delay_timeout = TIMER_IS_NOT_SETUP;
                    BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                    if (!auxinStatus)
                    {
                        AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                    }
                }
            }

           if (led_flashing_timeout != TIMER_IS_NOT_SETUP)
            {
                if (led_flashing_timeout > 0)
                {
                    led_flashing_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    led_flashing_timeout = led_flashing_periods_ms[ledFlashingState - 1];
                    if (led_flashing_idx > 0)
                    {
                        MainApp_UpdateLedFlashing();
                        if (led_flashing_idx  != LED_FLASHING_COUNT_INDEFINITE)
                        {
                            led_flashing_idx --;
                        }
                    }
                    else
                    {
                        if (led_flashing_idx == 0)
                        {
                            MainApp_SetNoneLedFlashing();
                            MainApp_UpdateLedFlashing();
                        }
                    }
                }
            }
#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
            if (gain_adjustment_status_check_timeout > 0)
            {
                gain_adjustment_status_check_timeout -= MAIN_APP_TIMEOUT_IN_MS;
            }
            else
            {
                eAudioPowerGainLevel gainLevel = MainApp_GetGainLevel(me);
                if (gainLevel != prevGainLevel)
                {
                    AudioSetPowerLevelGainEvt* pReq = Q_NEW(AudioSetPowerLevelGainEvt, AUDIO_SET_POWER_LEVEL_GAIN_SIG);
                    pReq->powerLevelGain = gainLevel;
                    pReq->sender = (QActive*)me;
                    SendToServer(AUDIO_SRV_ID, (QEvt*)pReq);
                    prevGainLevel = gainLevel;
                }
                gain_adjustment_status_check_timeout = GAIN_ADJUSTMENT_STATUS_CHECK_PERIOD_MS;
            }
#endif
            if (batt_status_check_timeout > 0)
            {
                batt_status_check_timeout -= MAIN_APP_TIMEOUT_IN_MS;
            }
            else
            {
                if ((batteryInfo.voltage.intBatteryVol < BATT_05PER_VOLT)
                    && (batteryInfo.chargerState < CHARGER_CHARGING_STA))
                {
                    MainApp_StartCriticalBattLevelLedFlashing();
                }
                if (((batteryInfo.voltage.intBatteryVol >= BATT_05PER_VOLT)
                    || (batteryInfo.chargerState == CHARGER_CHARGING_STA))
                    &&(ledFlashingState == LED_FLASHING_CRITICAL_BATT_LEVEL))
                {
                    ledFlashingState = LED_FLASHING_NONE;
                    led_flashing_timeout = TIMER_IS_NOT_SETUP;
                    led_flashing_idx = LED_FLASHING_COUNT_IS_NOT_SETUP;
                    MainApp_UpdateLedFlashing();
                }

                /* if battery voltage low than 9.9V and battInfo.chargerState is
                    CHARGER_BATT_STA or CHARGER_ERROR_STA
                 * system will auto shutdown  */
                if ((batteryInfo.voltage.intBatteryVol < CRITICAL_LOW_BATT_VOLT)
                    && (batteryInfo.chargerState < CHARGER_CHARGING_STA))
                {
                    return Q_TRAN(MainApp_IsBtConnected(me));
                }
#ifdef AUDIO_LIMITER_FOR_LOW_POWER
                eAudioPowerDrcLevel powerDrcLevel = MainApp_GetDrcLevel(me);
                if (powerDrcLevel != prevPowerDrcLevel)
                {
                    AudioSrc_SetDrcRange((QActive*)me, powerDrcLevel);
                    prevPowerDrcLevel = powerDrcLevel;
                }
#endif
                batt_status_check_timeout = BATT_STATUS_CHECK_PERIOD_MS;
            }

            if (startup_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (startup_delay_timeout > 0)
                {
                    startup_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    startup_delay_timeout = TIMER_IS_NOT_SETUP;
                    MainApp_PublishSystemStatus(MainApp, SYSTEM_ACTIVE_STA);
                    BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                    if (!auxinStatus)
                    {
                        MainApp_StartReconnectLedFlashing();
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD); // connect to last connected
                        reconnect_delay_timeout = RECONNECT_DELAY_MS;
                    }
                    else
                    {
                        MainApp_SwitchSource(me, AUXIN_CHANNEL);
                    }
                    MainApp_RestoreVolume(me, TRUE);
                    sysState = SYS_STATE_NORMAL;
                }
            }
            MainApp_Update(me);
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
                        if (bIsNeedToPlayDisconnectTone)
                        {
                            AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                        }
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

/* powering down state, which is turning off the system power and have some delay*/
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_POWER_OFF_CMD);
            MainApp_SetNoneLedFlashing();
            MainApp_PublishSystemStatus(MainApp, SYSTEM_SLEEP_STA);
            powering_off_delay_timeout = POWERING_OFF_DELAY_MS;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (powering_off_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (powering_off_delay_timeout > 0)
                {
                    powering_off_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    powering_off_delay_timeout = TIMER_IS_NOT_SETUP;
                    bt_powering_off_delay_timeout = BT_POWERING_OFF_DELAY_MS;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PWR_OFF_CMD);
                }
            }
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


QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            /*save current volume level so we can restore it after we play start up tone*/
            save_volume_level = *(uint8*)Setting_Get(SETID_VOLUME);

            LEDR_OFF;
            LEDB_OFF;
            MainApp_SetWakeUp(me);
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case POWER_MCU_SLEEP_SIG:
        {
            MainApp_SetWakeUp(me);
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if ((evt->keyId == POWER_KEY) && (evt->keyEvent == KEY_EVT_SHORT_PRESS))
            {
                return Q_TRAN(&MainApp_PoweringUp);
            }

            return Q_HANDLED();
        }
        case AC_IN_INTERRUPT_SIG://POWER_WAKE_UP_SIG:
        {
            ac_in_out_check_timeout =  AC_IN_OUT_CHECK_PERIOD_MS;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (ac_in_out_check_timeout > 0)
            {
                ac_in_out_check_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            }
            else
            {
                MainApp_SetWakeUp(me);
                PowerDrv_PowerSaveSleep();
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
static void MainApp_UpdateLedFlashing()
{
    switch(ledFlashingState)
    {
    case LED_FLASHING_NONE:
      {
            eBtStatus  btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
        BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
        if ((!auxinStatus) && ((btStatus == BT_CONNECTED_STA) || (btStatus == BT_STREAMING_A2DP_STA)))
        {
               LEDB_ON;
               LEDR_OFF;
        }
        else
        {
               LEDR_ON;
               LEDB_OFF;
        }
          break;
      }
    case LED_FLASHING_BT_VOL_MAXMIN:

    case LED_FLASHING_BT_RECONNECT:
    case LED_FLASHING_BT_PAIRING:
      {
           LEDR_OFF;
           if (bIsLEDB_ON)
           {
              LEDB_OFF;
           }
           else
           {
              LEDB_ON;
           }
           break;
      }
    case LED_FLASHING_CRITICAL_BATT_LEVEL:
    case LED_FLASHING_AUX_VOL_MAXMIN:
      {
           LEDB_OFF;
           if (bIsLEDR_ON)
           {
              LEDR_OFF;
           }
           else
           {
              LEDR_ON;
           }
         break;
      }
    default:
      break;
    }
}

static void MainApp_StartVolMaxMinLedFlashing()
{ // TODO: have to debug more this func

    eBtStatus  btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
    BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
    if ((!auxinStatus) && ((btStatus == BT_CONNECTED_STA) || (btStatus == BT_STREAMING_A2DP_STA)))
    {
        ledFlashingState = LED_FLASHING_BT_VOL_MAXMIN;
        LEDR_OFF;
        LEDB_ON;
    }
    else
    {
        ledFlashingState = LED_FLASHING_AUX_VOL_MAXMIN;
        LEDR_ON;
        LEDB_OFF;
    }
    led_flashing_timeout = led_flashing_periods_ms[ledFlashingState -1];
}

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

/* initial variable for power up system */
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me)
{
#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
    prevGainLevel = AUDIO_INVALID_BATTERY_GAIN_LEVEL;
#endif
    myBtStatus = BT_MAX_STA;
    isRePairing = FALSE;
    isLinkLossHappened = FALSE;
    isPairingStateContinue = FALSE;
#ifdef AUDIO_LIMITER_FOR_LOW_POWER
    prevPowerDrcLevel = AUDIO_MAX_DRC_LEVEL;
#endif
    bIsFirstBtMsg = TRUE;
}

/* initial variable for factory reset */
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me)
{
    save_volume_level = DEFAULT_VOLUME;
    me->vol = DEFAULT_VOLUME;
    Setting_Set(SETID_VOLUME, &me->vol);
    uint8 maxVol = MAX_VOLUME;
    Setting_Set(SETID_MAX_VOLUME, &maxVol);
}

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus)
{
    SystemStatusEvt *pe = Q_NEW(SystemStatusEvt, SYSTEM_STATE_SIG);
    pe->systemStatus = systemStatus;
    QF_PUBLISH(&pe->super, me);
}

static void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
    if(SYS_STATE_NORMAL != sysState)
    {
        /* If power off, don't process volume key */
        return;
    }
    me->vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if(me->vol >= step)
    {
        me->vol-=step;
    }
    else
    {
        me->vol = 0;
    }
    AudioSrv_SetVolume(me->vol);
}

static void MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
    if(SYS_STATE_NORMAL != sysState)
    {
        /* If power off, don't process volume key */
        return;
    }
    me->vol = *(uint8*)Setting_Get(SETID_VOLUME);
    uint8 maxVol = *(uint8*)Setting_Get(SETID_MAX_VOLUME);
    if((step + me->vol)<= maxVol)
    {
        me->vol+=step;
    }
    else
    {
        me->vol = maxVol;
    }
    AudioSrv_SetVolume(me->vol);
}

static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            if(me->vol > MIN_VOLUME)
            {
                MainApp_VolumeDown(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT_INDEFINITE);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if (me->vol < MAX_VOLUME)
            {
                MainApp_VolumeUp(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT_INDEFINITE);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
}

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            if(me->vol > MIN_VOLUME)
            {
                MainApp_VolumeDown(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if (me->vol < MAX_VOLUME)
            {
                MainApp_VolumeUp(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT);
            }
            break;
        }
        case BT_KEY:
        {
            eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
            BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
            if ((!auxinStatus) && ((btStatus == BT_CONNECTED_STA) || (btStatus== BT_STREAMING_A2DP_STA)))
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_PLAY_PAUSE_CMD);
            }
            if ((!auxinStatus) && (btStatus == BT_CONNECTABLE_STA) && !isPairingStateContinue)
            {
                MainApp_StartReconnectLedFlashing();
                BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                reconnect_delay_timeout = RECONNECT_DELAY_MS;
            }

            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
}

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
          {
             if ((evt->keyId == VOLUME_UP_KEY) || (evt->keyId == VOLUME_DOWN_KEY))
             {
               if ((ledFlashingState == LED_FLASHING_AUX_VOL_MAXMIN) || (ledFlashingState == LED_FLASHING_BT_VOL_MAXMIN))
                  {
                    MainApp_SetNoneLedFlashing();
                    MainApp_UpdateLedFlashing();
                  }
             }
            break;
          }
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
            MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
           {
                if (evt->keyId == POWER_KEY)
                { /* do factory reset here*/
                    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                    if (btStatus == BT_CONNECTABLE_STA)
                    {
                    MainApp_SettingReset(me);
                    LEDB_ON;
                    LEDR_ON;
                    factory_reset_flashing_delay_timeout = FACTORY_RESET_FLASHING_DELAY_TIME_MS;
                }
           }
           }
            break;
        default:
            break;
    }
}

static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case BT_KEY:
        {
            BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
            if (!auxinStatus)
            {
                if (reconnect_delay_timeout != TIMER_IS_NOT_SETUP)
                {
                    reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                }
                eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
                if (btStatus == BT_DISCOVERABLE_STA)
                {
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    isRePairing = TRUE;
                }
                else
                {
                MainApp_StartPairingLedFlashing();
                    isPairingStateContinue = TRUE;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
            }
            }
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            if(me->vol > MIN_VOLUME)
            {
                MainApp_VolumeDown(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT_INDEFINITE);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if (me->vol < MAX_VOLUME)
            {
                MainApp_VolumeUp(me, VOLUME_STEP);
            }
            else
            {
                MainApp_KeySPressVolUpDownEvtAction(me, LED_FLASHING_COUNT_INDEFINITE);
            }
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}

static void MainApp_SetWakeUp(cMainApp * const me)
{
    PowerDrv_RegisterIntEvent((QActive*)me);
    sysState = SYS_STATE_STANDBY;
    LEDR_OFF;
    LEDB_OFF;
    PowerDrv_DeinitialPower(&powerDrv);
}

static void MainApp_Update(cMainApp * const me)
{
    MainApp_UpdateBatt(me);
    MainApp_UpdateInputSource(me);
    Setting_Set(SETID_BATT_INFO, &batteryInfo);
}

static void MainApp_UpdateBatt(cMainApp * const me)
{
    bool ret = FALSE;
    tBatteryVol batteryVol;
    ret = PowerDrv_GetBatteryVol(&powerDrv, &batteryVol);
    if(ret)
    {
        ret = MainApp_UpdateBattVoltage(batteryVol.intBatteryVol);
        if(ret)
        {
            batteryInfo.voltage.intBatteryVol = battFilter.intBatt.filterResult;
            MainApp_UpdateBattState(me);
            if( CHARGER_ERROR_STA == batteryInfo.chargerState)
            {
                batteryInfo.isDataValid = FALSE;
            }
            else
            {
                batteryInfo.isDataValid = TRUE;
            }
        }
    }
}

static bool MainApp_UpdateBattVoltage(uint16 intBatteryVol)
{
    bool ret = FALSE;
    static tBattFilterCount intBattFilterCount =
    {
        .isFirstStart = TRUE,
        .maxValueIndex = 0,
        .sampleIndex= 0,
        .exceedRangeCount = 0,
        .maxValueSum = 0,
    };
    ret = MainApp_StartHigestFilter(&intBattFilterCount, &battFilter.intBatt, intBatteryVol);
    return ret;
}


/**
* @brief: use upper and lower threshold for each battery state detection, to make state more stable
* @Author: Johnny Fan
* @Note: below is the steps how it works:
   1. first check battery voltage if it's higher than current state's upper threshold,
      if yes, then continuously check the upper threshold to get the battery state
   2. if not, then check if voltage is lower than the current state's lower threshold,
      if yes, then continuously check the lower threshold to get the battery state
*/
static uint8 MainApp_GetBattState(uint8 preState,
            int16 batteryVol, const int16* highBound,  const int16* lowBound)
{
    uint8 battState = preState;
/* if previous battery state is not the highest state, and the current battery voltage is larger than the upper threshold
    * then increase the battery state by checking the upper threshold
    */
    if((preState != HIGHEST_STATE) &&
       (batteryVol > highBound[preState-1]))
    {
        for(;battState>HIGHEST_STATE;battState--)
        {
            if(batteryVol > highBound[battState-1])
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }
    /* else check the lower threshold, and decrease the battery state*/
    else
    {
        for(;battState<LOWEST_STATE;battState++)
        {
            if(batteryVol < lowBound[battState])
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }
    return battState;
}


static void MainApp_UpdateBattState(cMainApp * const me)
{
    static uint8 preIntBattState = LOWEST_STATE;
    batteryInfo.intBattState = MainApp_GetBattState(preIntBattState,
                            batteryInfo.voltage.intBatteryVol,
                            intBattAdcHighBound, intBattAdcLowBound);
    if(preIntBattState != batteryInfo.intBattState)
    {
        preIntBattState = batteryInfo.intBattState;
        batteryInfo.isStateChange = TRUE;
    }
}

static void MainApp_UpdateInputSource(cMainApp * const me)
{
    tInputSourceState inputState;
    /* detect the input source state from power driver */
    PowerDrv_GetInputSourceState(&powerDrv, &inputState);
    /* use filter to set the input source state in me->batteryInfo*/
    MainApp_SaveInputSource(me, &inputState);
    MainApp_UpdateCharger(me);
}


static void MainApp_SaveInputSource(cMainApp * const me, tInputSourceState* inputSourceState)
{/* after geting the same input source state for INPUT_SOURCE_FILTER_LEN times, it will confirm it's stable and then set it in me->inputSourceState*/
    static uint8 dcPlugInOutCount = 0;
    static uint8 usbPlugInOutCount = 0;
    static uint8 chargingDoneCount = 0;
    static uint8 dcPlugInOutByGpioCount = 0;

    static bool preIsDcPlugIn = FALSE;
    static bool preIsUsbPlugIn = FALSE;
    static bool preIsChargingDone = FALSE;
    static bool preIsDcPlugInDetectByGPIO = FALSE;

    if(MainApp_IsInputSourceStable(&inputSourceState->isDcPlugIn, &preIsDcPlugIn , &dcPlugInOutCount))
    {
        batteryInfo.inputSourceState.isDcPlugIn = inputSourceState->isDcPlugIn;
    }
    if(MainApp_IsInputSourceStable(&inputSourceState->isUsbPlugIn, &preIsUsbPlugIn, &usbPlugInOutCount))
    {
        batteryInfo.inputSourceState.isUsbPlugIn = inputSourceState->isUsbPlugIn;
    }
    if(MainApp_IsInputSourceStable(&inputSourceState->isChargingDone, &preIsChargingDone, &chargingDoneCount))
    {
        batteryInfo.inputSourceState.isChargingDone = inputSourceState->isChargingDone;
    }
    if(MainApp_IsInputSourceStable(&inputSourceState->isDcPlugInDetectByGPIO, &preIsDcPlugInDetectByGPIO, &dcPlugInOutByGpioCount))
    {
        batteryInfo.inputSourceState.isDcPlugInDetectByGPIO = inputSourceState->isDcPlugInDetectByGPIO;
    }
    // update the ADC voltage for DC input, only for production test
    batteryInfo.inputSourceState.dcPlugInVoltage = inputSourceState->dcPlugInVoltage;
}

static void MainApp_UpdateCharger(cMainApp * const me)
{
    /* get last AC plug in state for charger stat detect delay purpose */
    bool isAcPlugIn = *(bool*)Setting_Get(SETID_AC_STATUS);
    /* the charger has delay on output charging status at STAT pin after AC in plugged in, so the functions needs to delay some time to read it */
    if(delayCheckChargingCount !=0)
    { /* if it's delaying, return and come to check next time event*/
        delayCheckChargingCount--;
        return;
    }
    if (batteryInfo.inputSourceState.isDcPlugIn && (!isAcPlugIn))
    {/* if AC power is pluged in, need to set up delay parameter to wait for STAT pin output*/
        Setting_Set(SETID_AC_STATUS, &batteryInfo.inputSourceState.isDcPlugIn);
        /* change this register to set the delay time*/
        delayCheckChargingCount = GET_TICKS_IN_MS(CHECK_CHARGER_PIN_DELAY_MS)/
                                    GET_TICKS_IN_MS(MAINAPP_POWER_TIMEOUT_IN_MS);
        return;
    }
    Setting_Set(SETID_AC_STATUS, &batteryInfo.inputSourceState.isDcPlugIn);
    eChargerState chargeState;
    if (!batteryInfo.inputSourceState.isDcPlugIn)
    {
        chargeState = CHARGER_BATT_STA;
    }
    else if (bIsChargingError)
    {
        chargeState = CHARGER_ERROR_STA;
    }
    else if (batteryInfo.inputSourceState.isChargingDone)
    {
        chargeState = CHARGER_CHARGING_DONE_STA;
    }
    else
    {
        chargeState = CHARGER_CHARGING_STA;
    }
    if (batteryInfo.chargerState != chargeState)
    {
        batteryInfo.isStateChange = TRUE;
        batteryInfo.chargerState = chargeState;
    }
}

//if the input source state stable
static bool MainApp_IsInputSourceStable(bool* pCurInputState, bool* pPreInputState, uint8* pCount)
{
    bool ret = FALSE;
    if((*pPreInputState)==(*pCurInputState))
    {/* if it's the same as previous one, counter ++*/
        (*pCount)++;
        if((*pCount)> INPUT_SROUCE_FILTER_LEN)
        { /* over INPUT_SROUCE_FILTER_LEN times, set it as stable*/
            *pCount = 0;
            ret = TRUE;
        }
    }
    else
    { /* otherwise clear the counter, restart filter */
        *pCount = 0;
        (*pPreInputState) =(*pCurInputState);
    }
    return ret;
}

static void MainApp_InitialBattVariable(cMainApp * const me)
{
    /*initial ad state*/
    bool isAcPlugIn = TRUE;
    Setting_Set(SETID_AC_STATUS, &isAcPlugIn);
    /* Initial battery variables*/
    batteryInfo.isDataValid = FALSE;
    batteryInfo.isStateChange = FALSE;
    batteryInfo.chargerState = CHARGER_STATUS_MAX;
    batteryInfo.intBattState = INVALID_BATT_STATE;
    Setting_Set(SETID_BATT_INFO, &batteryInfo);
    /* initial for checking charging status pin*/
    delayCheckChargingCount = 0;
    bIsChargingError = FALSE;
}


/**
* @brief: it's the battery filter which can filter out the voltage jitter whne playing strong music
* @Author: Johnny Fan
* @Note: below is the steps how it works:
*  1. when start the firmware for the first time , get the battery voltage quickly for battery indication
*  2. if the voltage sample is within oldvoltage +/- SAMPLE_RANGE_mV,
        then consider it as a valid sample and put it in the array. Otherwise discard it
*  3. if (OUT_OF_RANGE_ACCEPT_NUMBER) samples are out of range continuously, then consider it as valid
*  4. when we get BATT_FILTER_LEN number valid samples, find the max value and fill in intBatVoltageMaxValue[]
*  5. repeat step1~4, and when we get BATT_FILTER_LEN number max values in intBatVoltageMaxValue[],
        calculate its avarage value and consider it as a valid battery voltage, and publish it out
*  6. For external battery, there's insert/eject action in which the battery voltage will change quickly,
       so if we find it's inserted/ejected for (extBattActionDebounceCount) times, set the voltage as fininal battery voltage
*/
static bool MainApp_StartHigestFilter(tBattFilterCount* pBattFilterCount,
                                            tBattFilterValue* pBattFilterValue, uint16 sample)
{
    bool ret = FALSE;
    if (pBattFilterCount->isFirstStart)
    {
        pBattFilterCount->isFirstStart = FALSE;
        pBattFilterValue->filterResult = sample;
        ret = TRUE;
    }
    else
    {
        if(pBattFilterValue->isRemovable)
        {
            if (((sample < BATT_EJECT_mVOLT) && (pBattFilterCount->isInserted)) ||
             ((sample >= BATT_EJECT_mVOLT) && (!pBattFilterCount->isInserted)))
            { /* if we found there's extBattActionDebounceCount number eject/insert action */
                pBattFilterCount->actionDebounceCount++;
                if(pBattFilterCount->actionDebounceCount>OUT_OF_RANGE_ACCEPT_NUMBER)
                { /* directly set it as battery voltage, and reset the filter*/
                    pBattFilterCount->actionDebounceCount = 0;
                    pBattFilterCount->isInserted = !pBattFilterCount->isInserted;
                    pBattFilterCount->sampleIndex = 0;
                    pBattFilterCount->maxValueIndex = 0;
                    pBattFilterCount->maxValueSum = 0;
                    pBattFilterValue->filterResult = sample;
                    ret  = TRUE;
                }
                else
                {
                    ret = FALSE;
                }
                return ret;
            }
        }
        pBattFilterCount->actionDebounceCount = 0;
        /* filter the internal battery*/
        if((sample <= (pBattFilterValue->filterResult + SAMPLE_RANGE_mV))&&
            (sample >= (pBattFilterValue->filterResult - SAMPLE_RANGE_mV)))
        { /* only get the sample within SAMPLE_RANGE_mV range*/

            pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
            pBattFilterCount->sampleIndex ++;
            pBattFilterCount->exceedRangeCount = 0;
        }
        else
        { /* if get intBattExceedRangeCount number out-of-range samples, then consider it as valid*/
            pBattFilterCount->exceedRangeCount++;
            if(pBattFilterCount->exceedRangeCount>=OUT_OF_RANGE_ACCEPT_NUMBER)
            {
                pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
                pBattFilterCount->sampleIndex ++;
                pBattFilterCount->exceedRangeCount = 0;
            }
        }
        if(pBattFilterCount->sampleIndex == BATT_FILTER_LEN)
        { /* if already get BATT_FILTER_LEN sample, then pick the max value to intBatVoltageMaxValue[]*/
            pBattFilterCount->sampleIndex = 0;
            pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex] =
                MainApp_FindMaxValue(pBattFilterValue->sample, BATT_FILTER_LEN);
            pBattFilterCount->maxValueSum += pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex];
            pBattFilterCount->maxValueIndex++;
            if(pBattFilterCount->maxValueIndex == BATT_FILTER_LEN)
            { /* get the average of all the max value and consider it as battery voltage*/
                pBattFilterCount->maxValueIndex = 0;
                pBattFilterValue->filterResult = (pBattFilterCount->maxValueSum / BATT_FILTER_LEN);
                pBattFilterCount->maxValueSum = 0;
                ret  = TRUE;
            }
        }
    }
    return ret;
}

static int16 MainApp_FindMaxValue(uint16* pArray, int16 size)
{
    int16 maxValue = 0;
    while(size)
    {
        if(maxValue< (*pArray))
        {
            maxValue = (*pArray);
        }
        size--;
        pArray ++;
    }
    return maxValue;
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
                isPairingStateContinue = FALSE;
                isLinkLossHappened = FALSE;
                if (startup_delay_timeout != TIMER_IS_NOT_SETUP)
                {
                    MainApp_RestoreVolume(me, TRUE);
                    startup_delay_timeout = TIMER_IS_NOT_SETUP;
                }
                BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if (!auxinStatus)
                {
                    LEDB_ON;
                    LEDR_OFF;
                    MainApp_SetNoneLedFlashing();
                    playing_connect_tone_delay_timeout = CONNECT_TONE_DELAY_TIME_MS;
                }else
                {
                    MainApp_SwitchSource(me, AUXIN_CHANNEL);
                }
                break;
            }
            case BT_PAIRING_FAIL_EVT:
            {
                isPairingStateContinue = FALSE;
                if (isRePairing == FALSE)
                {
                    MainApp_SetNoneLedFlashing();
                    MainApp_UpdateLedFlashing();
                }
                break;
            }
            case BT_LINK_LOSS_EVT:
            {
                BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if (!auxinStatus)
                {
                    /* using this tone command for play disconect audio cue*/
                    AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                    MainApp_StartReconnectLedFlashing();
                    isLinkLossHappened = TRUE;
                }
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
                BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if (!auxinStatus)
                {
                    if ((myBtStatus == BT_CONNECTED_STA) || (myBtStatus == BT_STREAMING_A2DP_STA))
                    {
                        if (isLinkLossHappened == FALSE)
                        { /* this is manual disconnect: play disconnect tone and try to reconnect*/
                            pre_reconnect_delay_timeout = DISCONNECT_TONE_DELAY_TIME_MS + DISCONNECT_TONE_LEN_MS; //wait for disconnect delay plus time needs for play disconect tone
                        MainApp_StartReconnectLedFlashing();
                        /* using this tone command for play disconect audio cue*/
                       playing_disconnect_tone_delay_timeout = DISCONNECT_TONE_DELAY_TIME_MS;
                    }
                        else
                        { /* if its linkloss no need to reconnect */
                            isLinkLossHappened = FALSE;
                            MainApp_SetNoneLedFlashing();
                            MainApp_UpdateLedFlashing();
                        }
                    }
                    else if ((myBtStatus == BT_DISCOVERABLE_STA) && (isRePairing == TRUE))
                    {
                        isRePairing = FALSE;
                        MainApp_StartPairingLedFlashing();
                        isPairingStateContinue = TRUE;
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    }
                }
                break;
            }
            case BT_DISCOVERABLE_STA:
            {
                BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if (!auxinStatus)
                {
                    MainApp_StartPairingLedFlashing();
                    if ((myBtStatus == BT_CONNECTED_STA) || (myBtStatus == BT_STREAMING_A2DP_STA))
                    {
                       /* using this tone command for play disconect audio cue*/
                        AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                    }
                }
                break;
            }
            case BT_CONNECTED_STA:
            {
                reconnect_delay_timeout = TIMER_IS_NOT_SETUP;
                sysState = SYS_STATE_NORMAL; /* for case when we connected before startup timer is finished*/
                if (!isLinkLossHappened)
                {
                MainApp_SetNoneLedFlashing();
                MainApp_UpdateLedFlashing();
                }
                break;
            }
            case BT_STREAMING_A2DP_STA:
            {
                BOOL auxinStatus = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if (!auxinStatus)
                {
                    LEDB_ON;
                    LEDR_OFF;
                    MainApp_SetNoneLedFlashing();
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
static void MainApp_StartCriticalBattLevelLedFlashing()
{
    ledFlashingState = LED_FLASHING_CRITICAL_BATT_LEVEL;
    led_flashing_timeout = led_flashing_periods_ms[ledFlashingState - 1];
    led_flashing_idx = LED_FLASHING_COUNT_INDEFINITE;
    LEDB_OFF;
    LEDR_ON;
}

static void MainApp_StartPairingLedFlashing()
{
    if (ledFlashingState != LED_FLASHING_CRITICAL_BATT_LEVEL)
    {
    ledFlashingState = LED_FLASHING_BT_PAIRING;
    led_flashing_timeout = led_flashing_periods_ms[ledFlashingState - 1];
    led_flashing_idx = LED_FLASHING_COUNT_INDEFINITE;
    LEDB_ON;
    LEDR_OFF;
}
}

static void MainApp_StartReconnectLedFlashing()
{
    if (ledFlashingState != LED_FLASHING_CRITICAL_BATT_LEVEL)
    {
    ledFlashingState = LED_FLASHING_BT_RECONNECT;
    led_flashing_timeout = led_flashing_periods_ms[ledFlashingState -1];
    led_flashing_idx = LED_FLASHING_COUNT_INDEFINITE;
    LEDB_ON;
    LEDR_OFF;
}
}
static void MainApp_SetNoneLedFlashing()
{
    if (ledFlashingState != LED_FLASHING_CRITICAL_BATT_LEVEL)
    {
    ledFlashingState = LED_FLASHING_NONE;
    led_flashing_timeout = TIMER_IS_NOT_SETUP;
    led_flashing_idx = LED_FLASHING_COUNT_IS_NOT_SETUP;
}
}
static void MainApp_KeySPressVolUpDownEvtAction(cMainApp * const me, int16 count)
{
  //if ledFlashingState - LED_FLASHING_NONE or LED_FLASHING_BT_PAIRING or  LED_FLASHING_BT_RECONNECT
   if (ledFlashingState < LED_FLASHING_BT_VOL_MAXMIN)
    {
        MainApp_StartVolMaxMinLedFlashing();
        led_flashing_idx = count;
    }
}

static bool MainApp_IsBtIdle()
{
    bool ret = FALSE;
    eBtStatus btStatus = BT_MAX_STA;
    eAudioChannel channel = AUDIO_CHANNEL_INVALID;
    if(Setting_IsReady(SETID_BT_STATUS))
    {
        btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
        channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
        if( (btStatus==BT_CONNECTABLE_STA) || (btStatus==BT_CONNECTED_STA) || (btStatus==BT_DISCOVERABLE_STA) ||
            (btStatus==BT_STREAMING_A2DP_STA && channel==BT_CHANNEL && isMusicDetected==FALSE) )
        {
            ret = TRUE;
        }
    }
    return ret;
}

static bool MainApp_IsAuxInIdle()
{
    bool ret = TRUE;
    eAudioChannel channel = AUDIO_CHANNEL_INVALID;
    if(Setting_IsReady(SETID_CHANNEL))
    {
        channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
        if((channel==AUXIN_CHANNEL) && (isMusicDetected==TRUE))
        { // aux in is playing
            ret = FALSE;
        }
    }
    return ret;
}

static bool MainApp_IsSystemIdle(cMainApp * const me)
{
    bool ret = FALSE;
    if(MainApp_IsBtIdle() && MainApp_IsAuxInIdle())
    {
        ret = TRUE;
    }
    return ret;
}
static void MainApp_RestoreVolume(cMainApp * const me, bool bIsStartupRestore)
{
    if ((save_volume_level > DEFAULT_VOLUME) && bIsStartupRestore)
    {
        me->vol = DEFAULT_VOLUME;
    }
    else
    {
        me->vol = save_volume_level;
    }
    AudioSrv_SetVolume(me->vol);
    Setting_Set(SETID_VOLUME, &me->vol);
}

static QStateHandler* MainApp_IsBtConnected(cMainApp * const me)
{
    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
    if ((btStatus == BT_STREAMING_A2DP_STA) || (btStatus == BT_CONNECTED_STA))
    {
        bIsNeedToPlayDisconnectTone = TRUE;
    }
    else
    {
        bIsNeedToPlayDisconnectTone = FALSE;
    }
   return (QStateHandler*)(&MainApp_BtDisconnect);
}
static void MainApp_SwitchSource(cMainApp * const me, eAudioChannel channel)
{
    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = channel;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
}

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
static eAudioPowerGainLevel MainApp_GetGainLevel(cMainApp * const me)
{
    if (batteryInfo.chargerState == CHARGER_CHARGING_STA)
    {
        return AUDIO_ADAPTER_GAIN_LEVEL;
    }
    else if (batteryInfo.voltage.intBatteryVol <= BATT_50PER_VOLT)
        {
            return  AUDIO_LOW_BATTERY_GAIN_LEVEL;
        }
        else if (batteryInfo.voltage.intBatteryVol <= BATT_90PER_VOLT)
            {
                return AUDIO_HIGH_BATTERY_GAIN_LEVEL;
            }
            else
            {
                return AUDIO_FULL_BATTERY_GAIN_LEVEL;
            }
}
#endif
#ifdef AUDIO_LIMITER_FOR_LOW_POWER
static eAudioPowerDrcLevel MainApp_GetDrcLevel(cMainApp * const me)
{
    if ((batteryInfo.chargerState == CHARGER_CHARGING_STA) || (batteryInfo.chargerState == CHARGER_CHARGING_DONE_STA))
    {
        return AUDIO_ADAPTER_DRC_LEVEL;
    }
    else
    {
        return AUDIO_NORMAL_BATTERY_DRC_LEVEL;
    }
}

#endif

static void MainApp_SettingReset(cMainApp * const me)
{
    BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
    MainApp_InitialVariablesForFactoryReset(me);
    AudioSrv_SetVolume(me->vol);
}

