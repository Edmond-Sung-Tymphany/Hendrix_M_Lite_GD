/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        MainApp.c
@brief       Main application for MGT
@author      Alexey
@date        2015-06-10
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
    BATT_GOOD     = 0,
    BATT_NORMAL   = 1,
    BATT_LOW      = 2,
    BATT_CRITICAL = 3,
} eBattState;

#define SYS_STATE_NONE                          (0x0)
#define SYS_STATE_NORMAL                        (0x1)
#define SYS_STATE_STANDBY                       (0x2)

#define SYS_STATE_STARTUP_DONE                  (0x4)

#define SYS_STATE_READY_TO_RESET                (0x8)
#define SYS_STATE_READY_TO_RESET_WAIT_CONFIRM   (0x10)
#define SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE (0x20)
#define SYS_STATE_NEED_RESET                    (0x40)
#define SYS_STATE_READY_TO_ENTER_DEMO           (0x80)
#define SYS_STATE_DEMO_SELECT                   (0x100)
#define SYS_STATE_DEMO_MODE_CONSUMER            (0x200)
#define SYS_STATE_DEMO_MODE_DISPLAY             (0x400)
#define SYS_STATE_DEMO_MODE_RETAIL              (0x800)

#define SYS_STATE_AUX_CHANNEL                   (0x1000)
#define SYS_STATE_BT_CONNECTABLE                (0x2000)
#define SYS_STATE_BT_PAIRING                    (0x4000)
#define SYS_STATE_BT_RECONNECT                  (0x8000)
#define SYS_STATE_BT_CONNECTED                  (0x10000)

#define SYS_STATE_BT_POWER_OFF                  (0x20000)

#define SYS_STATE_AFTER_RESET_START_UP          (0x40000)
#define SYS_STATE_TRUE_FACTORY_RESET            (0x80000)

static uint32 sysState = SYS_STATE_DEMO_MODE_CONSUMER;

static bool sysAuditoryFeedbackEnabled = TRUE;

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

#define BATT_STATUS_CHECK_PERIOD_MS         1000
#define LEDS_UPDATE_PERIOD_MS         MAIN_APP_TIMEOUT_IN_MS

static uint16 batt_status_check_timeout = 0;
static int16 leds_update_timeout = 0;

static bool gl_mute = FALSE;

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
#define GAIN_ADJUSTMENT_STATUS_CHECK_PERIOD_MS  1000
static uint16 gain_adjustment_status_check_timeout = 0;
static eAudioPowerGainLevel prevGainLevel;
#endif

#define TIMER_IS_NOT_SETUP              -1
#define POWERING_OFF_DELAY_MS           2000//1200
static int16 powering_off_delay_timeout = TIMER_IS_NOT_SETUP;

#define BT_POWERING_OFF_DELAY_MS           5000//600//delay after send BT off command to make sure we close bt connection
static int16 bt_powering_off_delay_timeout = TIMER_IS_NOT_SETUP;

#define STOP_BTSTREAM_DELAY_MS             600 //1600//     600
static int16 stopping_btstream_delay_timeout = TIMER_IS_NOT_SETUP;
#define STOP_BTSTREAM_DEADLOOP_TIMEOUT_MS       6000 // prevent dead loop after 6 sec force to continue powering off
static int16 stopping_btstream_deadloop_timeout = TIMER_IS_NOT_SETUP;

#define STARTUP_DELAY_MS           0
static int16 startup_delay_timeout = TIMER_IS_NOT_SETUP;

#define PLAY_STARTUP_TONE_DELAY_MS           2500
static int16 play_startup_tone_delay_timeout = TIMER_IS_NOT_SETUP;

#define SWITCH_TO_LAST_SOURCE_DELAY_MS           4500
static int16 switch_to_last_source_delay_ms = TIMER_IS_NOT_SETUP;
#define RESET_SWITCH_TO_LAST_SOURCE_DELAY_MS switch_to_last_source_delay_ms = SWITCH_TO_LAST_SOURCE_DELAY_MS;

#define FR_LED_TIMEOUT_MS 2600
static int32 fr_led_timeout_ms = TIMER_IS_NOT_SETUP;
#define RESET_FR_LED_TIMEOUT_MS  fr_led_timeout_ms = FR_LED_TIMEOUT_MS

#define ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS 15000
static int32 enter_demo_mode_reset_allowed_gap_timeout = TIMER_IS_NOT_SETUP;
#define RESET_ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS enter_demo_mode_reset_allowed_gap_timeout = ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS

#define WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS 500
static int32 wait_maybe_tripple_press_timeout = TIMER_IS_NOT_SETUP;
#define SET_WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS wait_maybe_tripple_press_timeout = WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS
#define RESET_WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS wait_maybe_tripple_press_timeout = TIMER_IS_NOT_SETUP

#define WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS 500
static int32 wait_maybe_double_press_timeout = TIMER_IS_NOT_SETUP;
#define SET_WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS wait_maybe_double_press_timeout = WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS
#define RESET_WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS wait_maybe_double_press_timeout = TIMER_IS_NOT_SETUP

#define CRITICAL_BATTERY_TONE_TIMEOUT_MS 6000
static int32 critical_battery_tone_timeout_ms = TIMER_IS_NOT_SETUP;
#define SET_CRITICAL_BATTERY_TONE_TIMEOUT_MS critical_battery_tone_timeout_ms = CRITICAL_BATTERY_TONE_TIMEOUT_MS
#define RESET_CRITICAL_BATTERY_TONE_TIMEOUT_MS critical_battery_tone_timeout_ms = TIMER_IS_NOT_SETUP

#define STANDBY_MODE_BATTERY_DISPLAY_TIME_MS 3000

#define WAIT_BATTERY_DISPLAY_MS 7100
static int32 wait_battery_display = TIMER_IS_NOT_SETUP;
#define SET_WAIT_BATTERY_DISPLAY_MS wait_battery_display = WAIT_BATTERY_DISPLAY_MS
#define RESET_WAIT_BATTERY_DISPLAY_MS wait_battery_display = TIMER_IS_NOT_SETUP

#define LOWER_VOLUME_TIMER 1000
static int32 lower_volume_timer = TIMER_IS_NOT_SETUP;
#define SET_LOWER_VOLUME_TIMER lower_volume_timer = LOWER_VOLUME_TIMER
#define RESET_LOWER_VOLUME_TIMER lower_volume_timer = TIMER_IS_NOT_SETUP

#define BT_RECONNECT_TIMER_CHECK_POINT_2 40000
#define BT_RECONNECT_TIMER_CHECK_POINT_1 80000
#define BT_RECONNECT_TIMER 120000 // 2 mins
static int32 bt_reconnect_timer = TIMER_IS_NOT_SETUP;
#define SET_BT_RECONNECT_TIMER bt_reconnect_timer = BT_RECONNECT_TIMER
#define RESET_BT_RECONNECT_TIMER bt_reconnect_timer = TIMER_IS_NOT_SETUP

#define BT_POWER_ON_RECCONECT_TIMER 2000
static int32 bt_power_on_reconnect_timer = TIMER_IS_NOT_SETUP;
#define SET_BT_POWER_ON_RECCONECT_TIMER bt_power_on_reconnect_timer = BT_POWER_ON_RECCONECT_TIMER

#define BT_POWER_ON_PAIRING_TIMER 2000
static int32 bt_power_on_pairing_timer = TIMER_IS_NOT_SETUP;
#define SET_BT_POWER_ON_PAIRING_TIMER bt_power_on_pairing_timer = BT_POWER_ON_PAIRING_TIMER

#define BT_PAIRING_TIMER 120000
static int32 bt_pairing_timer = TIMER_IS_NOT_SETUP;
#define SET_BT_PAIRING_TIMER bt_pairing_timer = BT_PAIRING_TIMER
#define RESET_BT_PAIRING_TIMER bt_pairing_timer = TIMER_IS_NOT_SETUP

#define BT_POWER_OFF_TIMER 2000
static int32 bt_power_off_timer = TIMER_IS_NOT_SETUP;
#define SET_BT_POWER_OF_TIMER bt_power_off_timer = BT_POWER_OFF_TIMER

#define SOURCE_SWITCH_RESTRICT_TIMER 6500
static int32 source_switch_restrict_timer = TIMER_IS_NOT_SETUP;
#define SET_SOURCE_SWITCH_RESTRICT_TIMER source_switch_restrict_timer = SOURCE_SWITCH_RESTRICT_TIMER

#define POWER_KEY_REPEAT_COUNT_29_SECONDS 14
#define POWER_KEY_REPEAT_COUNT_9_SECONDS 4
#define POWER_KEY_REPEAT_COUNT_3_SECONDS 1

#define _1_SECOND_MS  1000
#define _2_SECONDS_MS 2000

static int32 white_led_occupied = TIMER_IS_NOT_SETUP;
static int32 blue_led_occupied = TIMER_IS_NOT_SETUP;
static int32 green_led_occupied = TIMER_IS_NOT_SETUP;
static int32 red_led_occupied = TIMER_IS_NOT_SETUP;

static uint8 power_key_repeat_count;
static uint8 previous_volume;

#define INACTIVITY_DELAY_MS         1800000  // ~ 30 min (after 30 min inactivity switch to standby\off mode if AC in)
#define INACTIVITY_CHECK_POINT_1_MS 1500000  // ~ 25 min (after 5 min inactivity switch to standby\off mode if AC out)

static int32 inactivity_timeout = TIMER_IS_NOT_SETUP;
#define RESET_INACTIVITY    inactivity_timeout = INACTIVITY_DELAY_MS

static bool isMusicDetected = FALSE;
static bool isBlinkingMinMaxVolume = FALSE;
/*-----------------------------------------------------------------*/

static int32       power_init_timer;
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/* Private functions / variables. */

/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[5];

#include "GpioDrv.h"
static cGpioDrv ledGpiodrv;

#define LEDR_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_14);
#define LEDR_OFF GpioDrv_SetBit(&ledGpiodrv,GPIO_14);

#define LEDG_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_16);
#define LEDG_OFF GpioDrv_SetBit(&ledGpiodrv,GPIO_16);

#define LEDB_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_17);
#define LEDB_OFF GpioDrv_SetBit(&ledGpiodrv,GPIO_17);

#define LEDW_ON  GpioDrv_ClearBit(&ledGpiodrv,GPIO_18);
#define LEDW_OFF GpioDrv_SetBit(&ledGpiodrv,GPIO_18);

#define POWER_AND_BT        COMB_KEY_ID_0
#define POWER_AND_VOLUME_UP COMB_KEY_ID_1
#define PLAY_PAUSE_DOUBLE   COMB_KEY_ID_2
#define PLAY_PAUSE_TRIPPLE  COMB_KEY_ID_3
#define UP_DOWN_BT_BT       COMB_KEY_ID_4

static eBtStatus     myBtStatus;

#define ON (1)
#define OFF (0)

typedef struct _ledState
{
    eLedPatternType patternId;
    uint32 timer;
    uint8 ledOn;
} ledState;

static ledState leds[LED_COUNT] = {{LED_PATTERN_NONE, 0xff},{LED_PATTERN_NONE, 0xff},{LED_PATTERN_NONE, 0xff},{LED_PATTERN_NONE, 0xff}};

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
    LEDR_OFF;
    LEDG_OFF;
    LEDW_OFF;
    LEDB_OFF;

    /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                     MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);

    /* Subscribe */
#if defined(HAS_KEYS)
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

static void MainApp_SetSubState(uint32 state)
{
    sysState |= state;
}

static void MainApp_RemoveSubState(uint32 state)
{
    sysState &= ~state;
}

static bool MainApp_CheckSubState(uint32 state)
{
    return ((sysState & state) > 0) ? TRUE : FALSE;
}

static void MainApp_SetChannelsState(uint32 state)
{
    MainApp_RemoveSubState(SYS_STATE_AUX_CHANNEL);
    MainApp_RemoveSubState(SYS_STATE_BT_CONNECTABLE);
    MainApp_RemoveSubState(SYS_STATE_BT_PAIRING);
    MainApp_RemoveSubState(SYS_STATE_BT_RECONNECT);
    MainApp_RemoveSubState(SYS_STATE_BT_CONNECTED);

    MainApp_SetSubState(state);

    if (SYS_STATE_BT_PAIRING != state)
    {
        RESET_BT_PAIRING_TIMER;
    }
    if (SYS_STATE_BT_RECONNECT != state)
    {
        RESET_BT_RECONNECT_TIMER;
    }
}

static void MainApp_SetSysOperationState(uint32 state)
{
    MainApp_RemoveSubState(SYS_STATE_NORMAL | SYS_STATE_STANDBY);
    MainApp_SetSubState(state);
}

static void MainApp_SetLedOnOff(eLedColor led, uint8 OnOff)
{
    if (leds[led].ledOn == OnOff)
        return;

    leds[led].ledOn = OnOff;

    switch(led)
    {
        case LED_RED:
            if(OnOff)
                {LEDR_ON;}
            else
                {LEDR_OFF;}
            break;
        case LED_GREEN:
            if(OnOff)
                {LEDG_ON;}
            else
                {LEDG_OFF;}
            break;
        case LED_BLUE:
             if(OnOff)
                {LEDB_ON;}
            else
                {LEDB_OFF;}
            break;
        case LED_WHITE:
            if(OnOff)
                {LEDW_ON;}
            else
                {LEDW_OFF;}
            break;
        default:
            break;
    }
}

static bool MainApp_IfLedOccupied(eLedColor led)
{
    switch(led)
    {
        case LED_WHITE:
            return (white_led_occupied == TIMER_IS_NOT_SETUP) ? FALSE : TRUE;
        case LED_BLUE:
            return (blue_led_occupied == TIMER_IS_NOT_SETUP) ? FALSE : TRUE;
        case LED_GREEN:
            return (green_led_occupied == TIMER_IS_NOT_SETUP) ? FALSE : TRUE;
        case LED_RED:
            return (red_led_occupied == TIMER_IS_NOT_SETUP) ? FALSE : TRUE;
        default:
            return FALSE;
    }
}

static bool MainApp_SetLedOccupiedTimeMs(eLedColor led, uint32 timeMs)
{
    switch(led)
    {
        case LED_WHITE:
            return white_led_occupied = timeMs;
        case LED_BLUE:
            return blue_led_occupied = timeMs;
        case LED_GREEN:
        case LED_RED:
            green_led_occupied = timeMs;
            return red_led_occupied = timeMs;
        default:
            return TRUE;
            break;
    }
}

static bool MainApp_SetLedPattern(eLedColor led, eLedPatternType pattern, uint32 timer)
{
    if (TRUE == MainApp_IfLedOccupied(led) && (0xFFFFFFFF == timer || 0 == timer))
        return FALSE;

    leds[led].patternId = pattern;
    if(0xFFFFFFFF == timer)
    {
        leds[led].timer = (0 == leds[led].timer) ? timer : leds[led].timer;
    }
    else
    {
        leds[led].timer = timer;
    }

    if (0xFFFFFFFF != timer && 0 != timer)
    {
        MainApp_SetLedOccupiedTimeMs(led, timer);
    }

    if (LED_PATTERN_NONE == pattern)
    {
        if(leds[led].ledOn)
        {
            MainApp_SetLedOnOff(led, OFF);
        }
    }

    return TRUE;
}

static void MainApp_AllLEDsOff()
{
    eLedColor ii;

    for(ii = LED_FIRST; ii < LED_COUNT; ii++)
    {
        MainApp_SetLedPattern(ii, LED_PATTERN_NONE, 20);
    }
}

static void MainApp_UpdateLedState()
{
    eLedColor ii;

    for(ii = LED_FIRST; ii < LED_COUNT; ii++)
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
                MainApp_SetLedOnOff(ii, OFF);
                break;
            }
            case LED_PATTERN_SOLID:
            {
                MainApp_SetLedOnOff(ii, ON);
                break;
            }
            case LED_PATTERN_350_4:
            {
                MainApp_SetLedOnOff(ii, !((leds[ii].timer % 4000) < 500));
                break;
            }
            case LED_PATTERN_1_2:
            {
                MainApp_SetLedOnOff(ii, ((leds[ii].timer % 2000) < 1000));
                break;
            }
            case LED_PATTERN_025_050:
            {
                MainApp_SetLedOnOff(ii, ((leds[ii].timer % 500) < 250));
                break;
            }
            case LED_PATTERN_050_1:
            {
                MainApp_SetLedOnOff(ii, ((leds[ii].timer % 1000) < 500));
                break;
            }
            case LED_PATTERN_050_1_R:
            {
                MainApp_SetLedOnOff(ii, !((leds[ii].timer % 1000) < 500));
                break;
            }
            default:
                break;
        }
    }
}

void MainApp_ResetSystemSettings(cMainApp * const me)
{
    BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
    MainApp_RemoveSubState(SYS_STATE_DEMO_MODE_DISPLAY);
    MainApp_RemoveSubState(SYS_STATE_DEMO_MODE_RETAIL);
    MainApp_SetSubState(SYS_STATE_DEMO_MODE_CONSUMER);
    sysAuditoryFeedbackEnabled = TRUE;
}

static void MainApp_PlayTone(QActive* sender, eBtCmd cmd)
{
    if(sysAuditoryFeedbackEnabled)
    {
        AudioSrv_SendAudioSrvToneCmd(sender, cmd);
    }
}

QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_AllLEDsOff();
            MainApp_SetSysOperationState(SYS_STATE_STANDBY);// while system is starting
            power_key_repeat_count = 0;
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

static void MainApp_ShowBatteryStatusInStandByMode()
{
    if (BATT_GOOD == batteryInfo.intBattState)
    {
        MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 0);
        MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_SOLID, STANDBY_MODE_BATTERY_DISPLAY_TIME_MS);
    }
    else if (BATT_NORMAL == batteryInfo.intBattState)
    {
        MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_SOLID, STANDBY_MODE_BATTERY_DISPLAY_TIME_MS);
        MainApp_SetLedPattern(LED_RED, LED_PATTERN_SOLID, STANDBY_MODE_BATTERY_DISPLAY_TIME_MS);
    }
    else
    {
        MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
        MainApp_SetLedPattern(LED_RED, LED_PATTERN_SOLID, STANDBY_MODE_BATTERY_DISPLAY_TIME_MS);
    }
}

static void MainApp_UpdateLEDsPatterns(cMainApp * const me)
{
    if (FALSE == MainApp_CheckSubState(SYS_STATE_NORMAL))
        return;

    if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE)
     ||(TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_SELECT)))
        return;
    // CHARGE LEVEL LED
    if (BATT_GOOD == batteryInfo.intBattState)
    {
        if (FALSE == MainApp_IfLedOccupied(LED_RED)
         && FALSE == MainApp_IfLedOccupied(LED_GREEN))
        {
            if (FALSE == batteryInfo.inputSourceState.isUsbPlugIn)
            {
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 0);
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_SOLID, 0xFFFFFFFF);
            }
            else
            {
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 0);
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_350_4, 0xFFFFFFFF);
            }
        }
    }
    else if (BATT_NORMAL == batteryInfo.intBattState)
    {
        if (FALSE == batteryInfo.inputSourceState.isUsbPlugIn)
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_SOLID, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_SOLID, 0xFFFFFFFF);
        }
        else
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_350_4, 0xFFFFFFFF);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_350_4, 0xFFFFFFFF);
        }
    }
    else if (BATT_LOW == batteryInfo.intBattState)
    {
        if (FALSE == batteryInfo.inputSourceState.isUsbPlugIn)
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_SOLID, 0xFFFFFFFF);
        }
        else
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_350_4, 0xFFFFFFFF);
        }
    }
    else if (BATT_CRITICAL == batteryInfo.intBattState)
    {
        if (FALSE == batteryInfo.inputSourceState.isUsbPlugIn)
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_350_4, 0xFFFFFFFF);
        }
        else
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_350_4, 0xFFFFFFFF);
        }
    }
    // AUX WHITE LED
    if (TRUE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL))
    {
        MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_SOLID, 0xFFFFFFFF);
    }
    else
    {
        MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 0);
    }
    // BT BLUE LED
    if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_PAIRING))
    {
        MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_025_050, 0xFFFFFFFF);
    }
    else if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_RECONNECT))
    {
        MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_1_2, 0xFFFFFFFF);
    }
    else if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED))
    {
        MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_SOLID, 0xFFFFFFFF);
    }
    else
    {
        MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_NONE, 0);
    }
}

bool MainApp_SourceSwitchAllowed(cMainApp * const me)
{
    return ((TIMER_IS_NOT_SETUP == source_switch_restrict_timer)
         && (TRUE == MainApp_CheckSubState(SYS_STATE_STARTUP_DONE)));
}

void MainApp_BTStartReconnect(cMainApp * const me)
{
    MainApp_SetChannelsState(SYS_STATE_BT_RECONNECT);
    BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
    SET_BT_RECONNECT_TIMER;

    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = BT_CHANNEL;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
}
void MainApp_BTgoToPairing(cMainApp * const me)
{
    MainApp_SetChannelsState(SYS_STATE_NONE);
    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = BT_CHANNEL;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
    SET_BT_PAIRING_TIMER;
}

void MainApp_SwitchToAux(cMainApp * const me)
{
    if (FALSE == MainApp_SourceSwitchAllowed(me))
        return;

    MainApp_SetChannelsState(SYS_STATE_AUX_CHANNEL);

    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = AUXIN_CHANNEL;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);

    if (FALSE == MainApp_CheckSubState(SYS_STATE_BT_POWER_OFF))
    {
        BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
        BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);

        MainApp_SetSubState(SYS_STATE_BT_POWER_OFF);
        SET_BT_POWER_OF_TIMER;
    }
}

extern void PowerDrv_BT_OFF();
extern void PowerDrv_BT_ON();

void MainApp_BtPowerOff(cMainApp * const me)
{
    PowerDrv_BT_OFF();
}

void MainApp_BtPowerOn(cMainApp * const me)
{
    MainApp_RemoveSubState(SYS_STATE_BT_POWER_OFF);
    PowerDrv_BT_ON();
}

/* Active state  - super state for "normal" behaviour */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MainApp_AllLEDsOff();

            batt_status_check_timeout =  BATT_STATUS_CHECK_PERIOD_MS;
            leds_update_timeout = LEDS_UPDATE_PERIOD_MS;

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
            gain_adjustment_status_check_timeout = GAIN_ADJUSTMENT_STATUS_CHECK_PERIOD_MS;
#endif
            startup_delay_timeout = STARTUP_DELAY_MS;
            RESET_INACTIVITY;
            MainApp_InitialVariablesForPowerUp(me);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);

            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
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
          return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            RESET_INACTIVITY;
            KeyStateEvt* evt = (KeyStateEvt*)e;
            if((evt->keyId == POWER_KEY) && (evt->keyEvent == KEY_EVT_SHORT_PRESS))
            {
                if (TRUE == MainApp_SourceSwitchAllowed(me))
                {
                    MainApp_AllLEDsOff();
                    return Q_TRAN(MainApp_IsBtConnected(me));
                }
            }
            MainApp_ParseKeyEvent(me, e);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_UpdateLedState();

            if(inactivity_timeout != TIMER_IS_NOT_SETUP)
            {
                if (MainApp_IsSystemIdle(me))
                {
                    inactivity_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                    if((inactivity_timeout <= 0)
                     ||((INACTIVITY_CHECK_POINT_1_MS == inactivity_timeout)&&(FALSE == batteryInfo.inputSourceState.isDcPlugIn)))
                    {
                        if (FALSE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_CONSUMER))
                        {
                            RESET_INACTIVITY;
                        }
                        else
                        {
                            inactivity_timeout = TIMER_IS_NOT_SETUP;
                            MainApp_AllLEDsOff();
                            return Q_TRAN(MainApp_IsBtConnected(me));
                        }
                    }
                }
                else
                {
                    RESET_INACTIVITY;
                }
            }
            if(fr_led_timeout_ms != TIMER_IS_NOT_SETUP)
            {
                fr_led_timeout_ms -= MAIN_APP_TIMEOUT_IN_MS;
                if(fr_led_timeout_ms <= 0)
                {
                    MainApp_SetSubState(SYS_STATE_NEED_RESET);
                    fr_led_timeout_ms = TIMER_IS_NOT_SETUP;
                }
                if(MainApp_CheckSubState(SYS_STATE_NEED_RESET))
                {
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_RESET);
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_RESET_WAIT_CONFIRM);
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE);
                    MainApp_SetSubState(SYS_STATE_AFTER_RESET_START_UP);
                    MainApp_AllLEDsOff();
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            if(enter_demo_mode_reset_allowed_gap_timeout != TIMER_IS_NOT_SETUP)
            {
                enter_demo_mode_reset_allowed_gap_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if(enter_demo_mode_reset_allowed_gap_timeout <= 0)
                {
                    if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_SELECT))
                    {
                        if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_CONSUMER))
                        {
                            MainApp_SetChannelsState(SYS_STATE_BT_RECONNECT);
                            BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);

                            AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
                            audio_switch->channel = BT_CHANNEL;
                            audio_switch->sender = (QActive *) me;
                            SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
                        }
                        else
                        {
                            MainApp_SetChannelsState(SYS_STATE_AUX_CHANNEL);
                            MainApp_Mute(FALSE);

                            AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
                            audio_switch->channel = AUXIN_CHANNEL;
                            audio_switch->sender = (QActive *) me;
                            SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);

                            BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                            BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                        }
                    }
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_ENTER_DEMO);
                    MainApp_RemoveSubState(SYS_STATE_DEMO_SELECT);
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_RESET);
                    MainApp_RemoveSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE);
                    enter_demo_mode_reset_allowed_gap_timeout = TIMER_IS_NOT_SETUP;
                }
            }
            if (white_led_occupied != TIMER_IS_NOT_SETUP)
            {
                if (white_led_occupied > 0)
                {
                    white_led_occupied -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    white_led_occupied = TIMER_IS_NOT_SETUP;
                }
            }
            if (blue_led_occupied != TIMER_IS_NOT_SETUP)
            {
                if (blue_led_occupied > 0)
                {
                    blue_led_occupied -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    blue_led_occupied = TIMER_IS_NOT_SETUP;
                }
            }
            if (green_led_occupied != TIMER_IS_NOT_SETUP)
            {
                if (green_led_occupied > 0)
                {
                    green_led_occupied -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    green_led_occupied = TIMER_IS_NOT_SETUP;
                }
            }
            if (red_led_occupied != TIMER_IS_NOT_SETUP)
            {
                if (red_led_occupied > 0)
                {
                    red_led_occupied -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    red_led_occupied = TIMER_IS_NOT_SETUP;
                }
            }
            if (leds_update_timeout != TIMER_IS_NOT_SETUP)
            {
                if (leds_update_timeout > 0)
                {
                    leds_update_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    MainApp_UpdateLEDsPatterns(me);
                    leds_update_timeout = LEDS_UPDATE_PERIOD_MS;
                }
            }
            if (wait_maybe_double_press_timeout != TIMER_IS_NOT_SETUP)
            {
                if (wait_maybe_double_press_timeout > 0)
                {
                    wait_maybe_double_press_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_PLAY_PAUSE_CMD);
                    wait_maybe_double_press_timeout = TIMER_IS_NOT_SETUP;
                }
            }
            if (wait_maybe_tripple_press_timeout != TIMER_IS_NOT_SETUP)
            {
                if (wait_maybe_tripple_press_timeout > 0)
                {
                    wait_maybe_tripple_press_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_SKIP_FORWARD_CMD);
                    wait_maybe_tripple_press_timeout = TIMER_IS_NOT_SETUP;
                }
            }
            if (lower_volume_timer != TIMER_IS_NOT_SETUP)
            {
                if (lower_volume_timer > 0)
                {
                    lower_volume_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    uint8 maxVol = *(uint8*)Setting_Get(SETID_MAX_VOLUME);
                    me->vol = *(uint8*)Setting_Get(SETID_VOLUME);

                    if(me->vol > maxVol)
                    {
                        me->vol -= VOLUME_STEP;
                        AudioSrv_SetVolume(me->vol);
                        SET_LOWER_VOLUME_TIMER;
                    }
                    else
                    {
                        RESET_LOWER_VOLUME_TIMER;
                    }
                }
            }
            if (critical_battery_tone_timeout_ms != TIMER_IS_NOT_SETUP)
            {
                if (critical_battery_tone_timeout_ms > 0)
                {
                    critical_battery_tone_timeout_ms -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    if ((BATT_CRITICAL != batteryInfo.intBattState)
                    || (TRUE == batteryInfo.inputSourceState.isDcPlugIn))
                    {
                        RESET_CRITICAL_BATTERY_TONE_TIMEOUT_MS;
                    }
                    else
                    {
                        MainApp_PlayTone((QActive*)me, BT_TONE_RESET_PDL_CMD);
                        SET_CRITICAL_BATTERY_TONE_TIMEOUT_MS;
                    }
                }
            }
            if (bt_reconnect_timer != TIMER_IS_NOT_SETUP)
            {
                if (bt_reconnect_timer > 0)
                {
                    bt_reconnect_timer -= MAIN_APP_TIMEOUT_IN_MS;
                    if ((BT_RECONNECT_TIMER_CHECK_POINT_1 == bt_reconnect_timer)
                     || (BT_RECONNECT_TIMER_CHECK_POINT_2 == bt_reconnect_timer))
                    {
                        BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
                    }
                }
                else
                {
                    bt_reconnect_timer = TIMER_IS_NOT_SETUP;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                }
            }
            if (bt_pairing_timer != TIMER_IS_NOT_SETUP)
            {
                if (bt_pairing_timer > 0)
                {
                    bt_pairing_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_pairing_timer = TIMER_IS_NOT_SETUP;
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                }
            }

            if (bt_power_on_reconnect_timer != TIMER_IS_NOT_SETUP)
            {
                if (bt_power_on_reconnect_timer > 0)
                {
                    bt_power_on_reconnect_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_power_on_reconnect_timer = TIMER_IS_NOT_SETUP;
                    MainApp_BTStartReconnect(me);
                }
            }

            if (bt_power_on_pairing_timer != TIMER_IS_NOT_SETUP)
            {
                if (bt_power_on_pairing_timer > 0)
                {
                    bt_power_on_pairing_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_power_on_pairing_timer = TIMER_IS_NOT_SETUP;
                    MainApp_BTgoToPairing(me);
                }
            }

            if (bt_power_off_timer != TIMER_IS_NOT_SETUP)
            {
                if (bt_power_off_timer > 0)
                {
                    bt_power_off_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    bt_power_off_timer = TIMER_IS_NOT_SETUP;
                    MainApp_BtPowerOff(me);
                }
            }

            if (source_switch_restrict_timer != TIMER_IS_NOT_SETUP)
            {
                if (source_switch_restrict_timer > 0)
                {
                    source_switch_restrict_timer -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    source_switch_restrict_timer = TIMER_IS_NOT_SETUP;
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
            if (FALSE == MainApp_CheckSubState(SYS_STATE_STANDBY))
            {
                if (batt_status_check_timeout > 0)
                {
                    batt_status_check_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                   /* If a battery voltage is lower than 6.6V and
                    * no charger is in system will auto shutdown
                    */
                    if ((batteryInfo.voltage.intBatteryVol <= BATT_0_PERCENTAGE_mVOLT)
                    && (FALSE == batteryInfo.inputSourceState.isDcPlugIn))
                    {
                        MainApp_AllLEDsOff();
                        return Q_TRAN(MainApp_IsBtConnected(me));
                    }
                    batt_status_check_timeout = BATT_STATUS_CHECK_PERIOD_MS;
                }
            }

            if (switch_to_last_source_delay_ms != TIMER_IS_NOT_SETUP)
            {
                if (switch_to_last_source_delay_ms > 0)
                {
                    switch_to_last_source_delay_ms -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    switch_to_last_source_delay_ms = TIMER_IS_NOT_SETUP;

                    MainApp_SetSubState(SYS_STATE_STARTUP_DONE);

                    if (TRUE == MainApp_CheckSubState(SYS_STATE_AFTER_RESET_START_UP))
                    {
                        MainApp_RemoveSubState(SYS_STATE_AFTER_RESET_START_UP);
                        MainApp_BTgoToPairing(me);
                    }
                    else
                    {
                        if (TRUE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL))
                        {
                            MainApp_SwitchToAux(me);
                        }
                        else
                        {
                            if (FALSE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED))
                            {
                                MainApp_BTStartReconnect(me);
                            }
                        }
                    }
                }
            }

            if (play_startup_tone_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                 if (play_startup_tone_delay_timeout > 0)
                {
                    play_startup_tone_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    play_startup_tone_delay_timeout = TIMER_IS_NOT_SETUP;
                    if (FALSE == MainApp_CheckSubState(SYS_STATE_AFTER_RESET_START_UP))
                    {
                        MainApp_PlayTone((QActive*)me, BT_TONE_BAT_DOCK_CMD);
                    }
                    else
                    {
                        if(MainApp_CheckSubState(SYS_STATE_TRUE_FACTORY_RESET))
                        {
                            MainApp_RemoveSubState(SYS_STATE_TRUE_FACTORY_RESET);
                            MainApp_PlayTone((QActive*)me, BT_TONE_BAT_DOCK_CMD);
                        }
                    }
                    RESET_SWITCH_TO_LAST_SOURCE_DELAY_MS;
                }
            }

            if (startup_delay_timeout != TIMER_IS_NOT_SETUP)
            {
                if (startup_delay_timeout > 0)
                {
                    startup_delay_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                }
                else
                {
                    play_startup_tone_delay_timeout = PLAY_STARTUP_TONE_DELAY_MS;
                    startup_delay_timeout = TIMER_IS_NOT_SETUP;

                    MainApp_SetSysOperationState(SYS_STATE_NORMAL);
                    MainApp_PublishSystemStatus(MainApp, SYSTEM_ACTIVE_STA);
                    MainApp_SetSubState(SYS_STATE_READY_TO_RESET);
                    RESET_ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS;
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
            MainApp_BtPowerOn(me);
            MainApp_PublishSystemStatus(MainApp, SYSTEM_SLEEP_STA);
            powering_off_delay_timeout = POWERING_OFF_DELAY_MS;
            if (FALSE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL))
            {
                MainApp_SetChannelsState(SYS_STATE_BT_CONNECTABLE);
            }
            MainApp_AllLEDsOff();
            MainApp_RemoveSubState(SYS_STATE_STARTUP_DONE);
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
                    if (FALSE == MainApp_CheckSubState(SYS_STATE_AFTER_RESET_START_UP))
                    {
                        MainApp_PlayTone((QActive*)me, BT_TONE_POWER_OFF_CMD);
                    }
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

                    if(MainApp_CheckSubState(SYS_STATE_NEED_RESET))
                    {
                        if(MainApp_CheckSubState(SYS_STATE_TRUE_FACTORY_RESET))
                        {
                            MainApp_SwitchMode(MainApp, STANDBY_MODE);
                        }
                        else
                        {
                            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
                        }
                    }
                    else
                    {
                        MainApp_SwitchMode(MainApp, STANDBY_MODE);
                    }
                }
            }
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            if(MainApp_CheckSubState(SYS_STATE_NEED_RESET))
            {
                MainApp_RemoveSubState(SYS_STATE_NEED_RESET);

                if(MainApp_CheckSubState(SYS_STATE_TRUE_FACTORY_RESET))
                {
                    return Q_TRAN(&MainApp_Standby);
                }
                else // user reset
                {
                    return Q_TRAN(&MainApp_PoweringUp);
                }
            }
            else
            {
                return Q_TRAN(&MainApp_Standby);
            }
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
            PowerDrv_Ctor(&powerDrv);
            SetDisableWakeupSources();
            SET_WAIT_BATTERY_DISPLAY_MS;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            power_key_repeat_count = 0;
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if ((evt->keyId == POWER_KEY) && (evt->keyEvent == KEY_EVT_REPEAT))
            {
                if (POWER_KEY_REPEAT_COUNT_3_SECONDS == ++power_key_repeat_count)
                {
                    PowerDrv_Ctor(&powerDrv);
                    SET_WAIT_BATTERY_DISPLAY_MS;
                    MainApp_ShowBatteryStatusInStandByMode();
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (wait_battery_display != TIMER_IS_NOT_SETUP)
            {
                if (wait_battery_display > 0)
                {
                    wait_battery_display -= MAIN_APP_TIMEOUT_IN_MS;
                    MainApp_UpdateBatt(me);
                    MainApp_UpdateLedState();
                    PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
                }
                else
                {
                    RESET_WAIT_BATTERY_DISPLAY_MS;
                    MainApp_SwitchMode(MainApp, STANDBY_MODE);
                    return Q_HANDLED();
                }
            }
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
                PowerDrv_Ctor(&powerDrv);
                SetDisableWakeupSources();
                return Q_TRAN(&MainApp_PoweringUp);
            }
            if ((evt->keyId == POWER_KEY) && (evt->keyEvent == KEY_EVT_HOLD))
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

/* initial variable for power up system */
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me)
{
#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
    prevGainLevel = AUDIO_INVALID_BATTERY_GAIN_LEVEL;
#endif
    myBtStatus = BT_MAX_STA;

    /*set it to defaul t level to play start up tone*/
    me->vol = DEFAULT_VOLUME;
    AudioSrv_SetVolume(me->vol);
    Setting_Set(SETID_VOLUME, &me->vol);
}

/* initial variable for factory reset */
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me)
{
    me->vol = DEFAULT_VOLUME;
    Setting_Set(SETID_VOLUME, &me->vol);
    uint8 maxVol = MAX_VOLUME;
    Setting_Set(SETID_MAX_VOLUME, &maxVol);
    gl_mute = FALSE;
}

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus)
{
    SystemStatusEvt *pe = Q_NEW(SystemStatusEvt, SYSTEM_STATE_SIG);
    pe->systemStatus = systemStatus;
    QF_PUBLISH(&pe->super, me);
}

static void MainApp_SetMaxMinVolumeBlink(bool ifOnce)
{
    if (FALSE == MainApp_CheckSubState(SYS_STATE_NORMAL))
        return;

    if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE)
     ||(TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_SELECT)))
        return;

    if (FALSE == isBlinkingMinMaxVolume)
    {
        isBlinkingMinMaxVolume = (ifOnce ? FALSE : TRUE);
        if (BATT_GOOD == batteryInfo.intBattState)
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_025_050, (ifOnce ? 1000 : 0x7FFFFFFF));
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 0);
        }
        else if (BATT_NORMAL == batteryInfo.intBattState)
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_025_050, (ifOnce ? 1000 : 0x7FFFFFFF));
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_025_050, (ifOnce ? 1000 : 0x7FFFFFFF));
        }
        else
        {
            MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 0);
            MainApp_SetLedPattern(LED_RED, LED_PATTERN_025_050, (ifOnce ? 1000 : 0x7FFFFFFF));
        }
    }
}

static void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
    if (FALSE == MainApp_CheckSubState(SYS_STATE_NORMAL))
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
    if (FALSE == MainApp_CheckSubState(SYS_STATE_NORMAL))
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

static void MainApp_Mute(bool ifMute)
{
    if (gl_mute == ifMute)
        return;

    gl_mute = ifMute;

    if(gl_mute)
    {
        previous_volume = *(uint8*)Setting_Get(SETID_VOLUME);
        AudioSrv_SetVolume(0);
        MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_350_4, 0x7FFFFFFF);
    }
    else
    {
        AudioSrv_SetVolume(previous_volume);
        MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 20);
    }
}
static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    if (FALSE == MainApp_CheckSubState(SYS_STATE_NORMAL))
        return;

    if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE)
     ||(TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_SELECT)))
        return;

    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            if (me->vol == MIN_VOLUME)
            {
                isBlinkingMinMaxVolume = FALSE;
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            if (me->vol == MAX_VOLUME)
            {
                isBlinkingMinMaxVolume = FALSE;
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
            }
            break;
        }
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
            MainApp_Mute(FALSE);

            if(me->vol > MIN_VOLUME)
            {
                MainApp_VolumeDown(me, VOLUME_STEP);
            }
            else
            {
                MainApp_SetMaxMinVolumeBlink(TRUE);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            MainApp_Mute(FALSE);

            if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_SELECT))
            {
                if(TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_CONSUMER))
                {
                    MainApp_RemoveSubState(SYS_STATE_DEMO_MODE_CONSUMER);
                    MainApp_SetSubState(SYS_STATE_DEMO_MODE_RETAIL);

                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 20);

                }
                else if(TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_RETAIL))
                {
                    MainApp_RemoveSubState(SYS_STATE_DEMO_MODE_RETAIL);
                    MainApp_SetSubState(SYS_STATE_DEMO_MODE_DISPLAY);

                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                }
                else
                {
                    MainApp_RemoveSubState(SYS_STATE_DEMO_MODE_DISPLAY);
                    MainApp_SetSubState(SYS_STATE_DEMO_MODE_CONSUMER);

                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 20);
                }
                RESET_ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS;
            }

            if(TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET_WAIT_CONFIRM))
            {
                MainApp_ResetSystemSettings(me);

                RESET_FR_LED_TIMEOUT_MS;
                RESET_ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS;
                MainApp_SetSubState(SYS_STATE_READY_TO_RESET_WAIT_LEDS_DONE);
                MainApp_AllLEDsOff();
                MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_025_050, 2500);
            }

            if (me->vol < MAX_VOLUME)
            {
                MainApp_VolumeUp(me, VOLUME_STEP);
            }
            else
            {
                MainApp_SetMaxMinVolumeBlink(TRUE);
            }
            break;
        }
        case BT_KEY:
        {
            MainApp_Mute(FALSE);

            if (TRUE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL)
            || (TRUE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTABLE)))
            {
                MainApp_BtPowerOn(me);
                SET_BT_POWER_ON_RECCONECT_TIMER;
            }
            else
            {
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
                BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
            }
            break;
        }
        case INPUT_KEY:
        {
            MainApp_SwitchToAux(me);
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            if (TRUE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL))
            {
                MainApp_Mute(!gl_mute);
            }
            else
            {
                if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED))
                {
                    if (TIMER_IS_NOT_SETUP == wait_maybe_double_press_timeout
                     && TIMER_IS_NOT_SETUP == wait_maybe_tripple_press_timeout)
                    {
                        SET_WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS;
                    }
                }
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:
            break;
    }
}

static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case INPUT_KEY:
            break;
        case BT_KEY:
        {
            if(FALSE == MainApp_CheckSubState(SYS_STATE_BT_PAIRING))
            {
                MainApp_BtPowerOn(me);
                SET_BT_POWER_ON_PAIRING_TIMER;
            }
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            MainApp_Mute(FALSE);

            if(me->vol > MIN_VOLUME)
            {
                MainApp_VolumeDown(me, VOLUME_STEP);
            }
            else
            {
                MainApp_SetMaxMinVolumeBlink(FALSE);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            MainApp_Mute(FALSE);

            if (me->vol < MAX_VOLUME)
            {
                MainApp_VolumeUp(me, VOLUME_STEP);
            }
            else
            {
                MainApp_SetMaxMinVolumeBlink(FALSE);
            }
            break;
        }
        default:
            break;
    }
}

static void MainApp_KeyVeryLongHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case INPUT_KEY:
        {
            sysAuditoryFeedbackEnabled = !sysAuditoryFeedbackEnabled;
            MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_025_050, _2_SECONDS_MS);
            break;
        }
        default:
            break;
    }
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
                MainApp_SetMaxMinVolumeBlink(FALSE);
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
                MainApp_SetMaxMinVolumeBlink(FALSE);
            }
            break;
        }
        case POWER_KEY:
        {
#ifdef HAS_DSP_RUNTIME_TUNE_SUPPORT
            if (POWER_KEY_REPEAT_COUNT_9_SECONDS == ++power_key_repeat_count)
            {
                AudioSrv_SendShutDspReq((QActive*)me, TRUE);
                MainApp_SetLedPattern(LED_BLUE,  LED_PATTERN_025_050, _1_SECOND_MS);
                MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_025_050, _1_SECOND_MS);
            }
#else
            ++power_key_repeat_count;
#endif
            if (POWER_KEY_REPEAT_COUNT_29_SECONDS == power_key_repeat_count)
            {
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_050_1, SW_MAJOR_VERSION  * _1_SECOND_MS);
                MainApp_SetLedPattern(LED_BLUE,  LED_PATTERN_050_1, SW_MINOR_VERSION1 * _1_SECOND_MS);
                MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_050_1, SW_MINOR_VERSION2 * _1_SECOND_MS);
            }

            break;
        }

        default:
            break;
    }
}

static void MainApp_KeyCombEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_AND_BT:
        {
            if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET))
            {
                MainApp_SetSubState(SYS_STATE_READY_TO_RESET_WAIT_CONFIRM);
                MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_050_1_R, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
            }
            break;
        }
        case UP_DOWN_BT_BT:
        {
            if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET))
            {
                MainApp_SetSubState(SYS_STATE_TRUE_FACTORY_RESET);
                MainApp_SetSubState(SYS_STATE_READY_TO_RESET_WAIT_CONFIRM);
                MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                MainApp_SetLedPattern(LED_RED, LED_PATTERN_050_1_R, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
            }
            break;
        }
        case POWER_AND_VOLUME_UP:
        {
            if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_ENTER_DEMO))
            {
                MainApp_SetSubState(SYS_STATE_DEMO_SELECT);

                if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_CONSUMER))
                {
                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                    MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 20);
                }
                else if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_DISPLAY))
                {
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_NONE, 20);
                }
                else
                {
                    MainApp_SetLedPattern(LED_BLUE, LED_PATTERN_050_1, ENTER_DEMO_MODE_RESET_ALLOWED_GAP_TIMEOUT_MS);
                    MainApp_SetLedPattern(LED_GREEN, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_WHITE, LED_PATTERN_NONE, 20);
                    MainApp_SetLedPattern(LED_RED, LED_PATTERN_NONE, 20);
                }
            }
            break;
        }
        case PLAY_PAUSE_DOUBLE:
            if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED))
            {
                RESET_WAIT_MAYBE_DOUBLE_PRESS_TIMEOUT_MS;
                SET_WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS;
            }
            break;
        case PLAY_PAUSE_TRIPPLE:
            if (TRUE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED))
            {
                RESET_WAIT_MAYBE_TRIPPLE_PRESS_TIMEOUT_MS;
                BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_SKIP_BACKWORD_CMD);
            }
            break;
        default:
            break;
    }
}

static bool MainApp_IfMustIgnoreKey(QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;

    if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_CONSUMER))
    {
        return FALSE;
    }
    else if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_DISPLAY))
    {
        if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_ENTER_DEMO))
        {
            if ((POWER_AND_VOLUME_UP == evt->keyId)
             || (VOLUME_UP_KEY == evt->keyId))
            {
                return FALSE;
            }
        }
    }
    else if (TRUE == MainApp_CheckSubState(SYS_STATE_DEMO_MODE_RETAIL))
    {
        if ((VOLUME_UP_KEY == evt->keyId)
         || (VOLUME_DOWN_KEY == evt->keyId))
        {
            return FALSE;
        }

        if (TRUE == MainApp_CheckSubState(SYS_STATE_READY_TO_ENTER_DEMO))
        {
            if ((POWER_AND_VOLUME_UP == evt->keyId))
            {
                return FALSE;
            }
        }
    }
    return TRUE;
}
static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;

    if(TRUE == MainApp_IfMustIgnoreKey(e))
        return;

    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            MainApp_KeyUpEvtAction(me, evt);
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
        case COMB_KEY_EVT:
            MainApp_KeyCombEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            MainApp_KeyVeryLongHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            MainApp_KeyRepeatEvtAction(me, evt);
            break;
        default:
            break;
    }
}

static void MainApp_SetWakeUp(cMainApp * const me)
{
    PowerDrv_RegisterIntEvent((QActive*)me);
    MainApp_SetSysOperationState(SYS_STATE_STANDBY);
    PowerDrv_Xtor(&powerDrv);
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

    if (INVALID_BATT_STATE == preState)
    {
        preState = LOWEST_STATE;
    }

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
    static uint8 preIntBattState = INVALID_BATT_STATE;
    batteryInfo.intBattState = MainApp_GetBattState(preIntBattState,
                            batteryInfo.voltage.intBatteryVol,
                            intBattAdcHighBound, intBattAdcLowBound);
    if(preIntBattState != batteryInfo.intBattState)
    {
        preIntBattState = batteryInfo.intBattState;
        batteryInfo.isStateChange = TRUE;

        if (BATT_CRITICAL == batteryInfo.intBattState)
        {
            uint8 maxVol = LOW_POWER_MAX_VOLUME;
            Setting_Set(SETID_MAX_VOLUME, &maxVol);
            SET_CRITICAL_BATTERY_TONE_TIMEOUT_MS;
            SET_LOWER_VOLUME_TIMER;
        }
        else
        {
            uint8 maxVol = MAX_VOLUME;
            Setting_Set(SETID_MAX_VOLUME, &maxVol);
        }
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
        if (TRUE  == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET)
        && (FALSE == MainApp_CheckSubState(SYS_STATE_READY_TO_ENTER_DEMO))
        && (TRUE == inputSourceState->isDcPlugIn))
        {
            MainApp_SetSubState(SYS_STATE_READY_TO_ENTER_DEMO);
        }
        if ((FALSE == inputSourceState->isDcPlugIn)
          && (TRUE == batteryInfo.inputSourceState.isDcPlugIn))
        {
            if (BATT_CRITICAL == batteryInfo.intBattState)
            {
                SET_CRITICAL_BATTERY_TONE_TIMEOUT_MS;
            }
            RESET_INACTIVITY;
        }
        else if ((TRUE == inputSourceState->isDcPlugIn)
          && (FALSE == batteryInfo.inputSourceState.isDcPlugIn))
        {
            RESET_INACTIVITY;
        }
        // Used for power cable plug in detect
        batteryInfo.inputSourceState.isDcPlugIn = inputSourceState->isDcPlugIn;
    }
    if(MainApp_IsInputSourceStable(&inputSourceState->isUsbPlugIn, &preIsUsbPlugIn, &usbPlugInOutCount))
    {
        /*
            Used for charging process activity indication.
            TRUE - charging, FALSE - not charging (either cable out, either cable in, but 100% reached)
        */
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
                if(FALSE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL)
                &&(FALSE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED)))
                {
                    MainApp_SetChannelsState(SYS_STATE_BT_CONNECTED);
                    MainApp_PlayTone((QActive*)me, BT_TONE_AC_IN_CMD);
                }
                break;
            }
            case BT_PAIRING_FAIL_EVT:
            {
                break;
            }
            case BT_LINK_LOSS_EVT:
            {
                SET_BT_POWER_ON_RECCONECT_TIMER;
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
            {
                if (FALSE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL)
                 && FALSE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTABLE)
                 && FALSE == MainApp_CheckSubState(SYS_STATE_READY_TO_RESET))
                {
                    MainApp_SetChannelsState(SYS_STATE_BT_CONNECTABLE);
                }
                break;
            }
            case BT_DISCOVERABLE_STA:
            {
                if (FALSE == MainApp_CheckSubState(SYS_STATE_AUX_CHANNEL)
                 && FALSE == MainApp_CheckSubState(SYS_STATE_BT_PAIRING))
                {
                    if (FALSE == MainApp_CheckSubState(SYS_STATE_BT_CONNECTED)
                      &&FALSE == MainApp_CheckSubState(SYS_STATE_BT_RECONNECT))
                    {
                        MainApp_PlayTone((QActive*)me, BT_TONE_LIM_VOL_CMD);
                        MainApp_SetChannelsState(SYS_STATE_BT_PAIRING);
                        SET_SOURCE_SWITCH_RESTRICT_TIMER;
                    }
                }
                break;
            }
            case BT_CONNECTED_STA:
            {
                break;
            }
            case BT_STREAMING_A2DP_STA:
            {
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

static bool MainApp_IsBtIdle()
{
    bool ret = FALSE;
    eBtStatus btStatus = BT_MAX_STA;
    eAudioChannel channel = AUDIO_CHANNEL_INVALID;
    if(Setting_IsReady(SETID_BT_STATUS))
    {
        btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
        channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
        if((btStatus==BT_CONNECTABLE_STA) || (btStatus==BT_CONNECTED_STA) || (btStatus==BT_DISCOVERABLE_STA) ||
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

static void MainApp_SetDefaultVolume(cMainApp * const me)
{
    me->vol = DEFAULT_VOLUME;
    AudioSrv_SetVolume(me->vol);
    Setting_Set(SETID_VOLUME, &me->vol);
}

static QStateHandler* MainApp_IsBtConnected(cMainApp * const me)
{
    eBtStatus btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
    if ((btStatus == BT_STREAMING_A2DP_STA) || (btStatus == BT_CONNECTED_STA))
    {
        return (QStateHandler*)(&MainApp_BtDisconnect);
    }
    else
    {
        return (QStateHandler*)(&MainApp_PoweringDown);
    }
}

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
static eAudioPowerGainLevel MainApp_GetGainLevel(cMainApp * const me)
{
    if (TRUE == batteryInfo.inputSourceState.isDcPlugIn)
    {
        return AUDIO_ADAPTER_GAIN_LEVEL;
    }
    else if (batteryInfo.voltage.intBatteryVol <= BATT_5_PERCENTAGE_mVOLT_LOW_BOUND)
    {
        return  AUDIO_LOW_BATTERY_GAIN_LEVEL;
    }
    else if (batteryInfo.voltage.intBatteryVol <= BATT_50_PERCENTAGE_mVOLT_LOW_BOUND)
    {
        return AUDIO_HIGH_BATTERY_GAIN_LEVEL;
    }
    else
    {
        return AUDIO_FULL_BATTERY_GAIN_LEVEL;
    }
}
#endif

