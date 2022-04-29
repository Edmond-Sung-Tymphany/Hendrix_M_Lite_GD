/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef MAINAPP_PRIV_H
#define MAINAPP_PRIV_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "MainApp.h"
#include "MainApp_util.h"
#include "MainApp_bluetoothKeyEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp_batteryEvtHandler.h"
#include "KeySrv.h"

/*****************************************************************
 * Type Definition
 *****************************************************************/

/* battery capacity value */
#define MAINAPP_BATTERY_URGENT_LEVEL            (5)
#define MAINAPP_BATTERY_CRITICAL_LEVEL          (10)
#define MAINAPP_BATTERY_LOW_LEVEL               (20)
#define MAINAPP_BATTERY_STANDARD_LEVEL          (80)
#define MAINAPP_BATTERY_NORMAL_LEVEL            (90)
#define MAINAPP_BATTERY_FULL_LEVEL              (92)

/* system gains define*/
typedef enum
{
    SYS_GAIN_DEFAULT = 0,
    SYS_GAIN_LEVEL1,
    SYS_GAIN_LEVEL2,
    SYS_GAIN_LEVEL3,
    SYS_GAIN_LEVEL4,
    SYS_GAIN_MAX,
} eSysGain;

#define BATT_LEVEL_OF_SYS_GAIN_1   (95)
#define BATT_LEVEL_OF_SYS_GAIN_2   (90)
#define BATT_LEVEL_OF_SYS_GAIN_3   (30)
#define BATT_LEVEL_OF_SYS_GAIN_4   (0)

//for M key multiple press
#define NOT_PRESSED_COUNT      (0)
#define SINGLE_PRESS_COUNT     (1)
#define DOUBLE_PRESS_COUNT     (2)
#define TRIPLE_PRESS_COUNT     (3)


typedef enum
{
    REBOOT_CODE_INVALID = 0,
    REBOOT_CODE_UPGRADE_TIMEOUT,
    REBOOT_CODE_PREACTIVE_TIMEOUT,
    REBOOT_CODE_POWERING_UP_TIMEOUT,
    REBOOT_CODE_POWERING_DOWN_TIMEOUT,
} eRebootCode;

typedef enum
{
    COM_KEY_INVALID         = 0,
    VOL_UP_KEY_IN_HOLD      = 0x01,
    VOL_UP_KEY_IN_L_HOLD    = 0x02,
    VOL_DOWN_KEY_IN_HOLD    = 0x04,
    VOL_DOWN_KEY_IN_L_HOLD  = 0x08,
    POWER_IN_HOLD           = 0X10,
    POWER_IN_L_HOLD         = 0x20,
    CONNECT_IN_HOLD         = 0x40,
    CONNECT_IN_VL_HOLD      = 0X80,
    CONFIG_IN_HOLD          = 0x100,
    CONFIG_IN_VL_HOLD       = 0X200,
    COM_KEY_STANDBY         = POWER_IN_HOLD | POWER_IN_L_HOLD,
    COM_KEY_POWER_ON        = POWER_IN_HOLD | POWER_IN_L_HOLD,
    COM_KEY_IN_TURNING      = VOL_UP_KEY_IN_HOLD | VOL_DOWN_KEY_IN_HOLD,
    COM_KEY_IN_L_TURNING    = VOL_UP_KEY_IN_HOLD | VOL_UP_KEY_IN_L_HOLD | VOL_DOWN_KEY_IN_HOLD | VOL_DOWN_KEY_IN_HOLD,
    COM_KEY_VOL_UP_HOLD     = VOL_UP_KEY_IN_HOLD | VOL_UP_KEY_IN_L_HOLD,
    COM_KEY_VOL_DOWN_HOLD   = VOL_DOWN_KEY_IN_HOLD | VOL_DOWN_KEY_IN_L_HOLD,
    COM_KEY_DSP_TUNING_MODE = CONNECT_IN_HOLD | CONNECT_IN_VL_HOLD | CONFIG_IN_HOLD | CONFIG_IN_VL_HOLD,
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,  //126
    MAINAPP_SLEEP_SIG,
    MAINAPP_ACTIVE_SIG,
    MAINAPP_STANDBY_SIG,
    MAINAPP_RESTART_SIG,
    MAINAPP_SHOP_SIG,
    MAINAPP_SHUTDOWN_SIG,
    MAINAPP_SHOW_VERSION_SIG,
    MAINAPP_FACTORY_RESET_SIG,
    MAINAPP_SWITCH_CHANNEL_SIG,
    MAINAPP_SET_AUDIO_SIG,
};

enum MainAppKeyStatus
{
    KeyStatus_KeyUp = 0,
    KeyStatus_KeyDown,
    KeyStatus_KeySP,
    KeyStatus_KeyLP,
    KeyStatus_KeyVLP,
};

typedef enum
{
    TIMER_ID_BT_BOOTING_TIMEOUT = 0,
    TIMER_ID_DELAYED_ERROR_REBOOT,
    TIMER_ID_UNMUTE_DELAY_TIMEOUT,
    TIMER_ID_LOW_BATTERY_NOTIFY_TIMEOUT,
    TIMER_ID_WAIT_IN_OFF_TIMEOUT,
    TIMER_ID_SOURCE_CHANGE_TIMEOUT,
    TIMER_ID_STACK_CHECK_TIMEOUT,
    TIMER_ID_SYS_TEMP_CHECK_TIMEOUT,
    TIMER_ID_AMP_ERR_CHECK_TIMEOUT,
#ifdef HAS_BT_PAIRING_FILTER
    TIMER_ID_PAIRING_FILTER_TIMEOUT,
#endif
    TIMER_ID_SHOW_BATTERY_LED_TIMEOUT,
    TIMER_ID_STANDBY_TIMEOUT,
    TIMER_ID_SLEEP_TIMEOUT,
    TIMER_ID_POWERING_OFF_TIMEOUT,
    TIMER_ID_BT_TO_ACTIVE_TIMEOUT,
    TIMER_ID_POWERING_DOWN_TIMEOUT,
    TIMER_ID_POWER_SWITCH_CHK_TIMEROUT,
    TIMER_ID_CUE_CMD_DELAY_TIMEROUT,
    TIMER_ID_BT_BATT_NOTIFY_TIMEOUT,
    TIMER_ID_Connected_cue_TIMEOUT,
    TIMER_ID_BAT_FULL_CUE_TIMEOUT_TIMEOUT,
    TIMER_ID_SYS_GAIN_ADJUST_TIMEOUT,

    TIMER_ID_MAX
} eTimerID;

#ifdef MAINAPP_DEBUG
const static char *mainApp_debug = "[MainApp_Debug] ";
#define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
#define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
#define MAINAPP_DEBUG_MSG(...)
#define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

//#define MAINAPP_BT_BOOTING_TIMEOUT_IN_MS         (10*1000)  /* Timeout for Bluetooth normal boot up: 2 min, normally need 53 sec to boot-up */
#define MAINAPP_BT_BOOTING_TIMEOUT_IN_MS         (2*1000)  /* Nick: Modify to 2sec to speed up the detection BT not powering up */

#define MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS     (5*1000)  /* wait 5 second(for finished LED indication), then reboot system. */

#define MAINAPP_DELAY_BATT_NOTIFY_TIMEOUT_IN_MS  (15*1000)
#define MAINAPP_LOW_BATTERY_NOTIFY_TIMEOUT_IN_MS (30*60*1000)
#define MAINAPP_POWERING_OFF_TIMEOUT_IN_MS       (1000)

#define MAINAPP_BT_TO_ACTIVE_TIMEOUT_IN_MS       (700)
#define MAINAPP_BT_BATT_NOTIFY_TIMEOUT_IN_MS     (7*1000)
#define MAINAPP_FACTORY_POWER_DOWN_TIMEOUT_MS    (5*1000) /* Timeout for BT factory reset, normally need few sec */
//#define MAINAPP_BT_CUE_START_DELAY_TIMEOUT_IN_MS (600)
#define MAINAPP_BT_CUE_START_DELAY_TIMEOUT_IN_MS (200) //Nick modify to resolve the cue corrupt issue for V3
//#define MAINAPP_BTCUE_NOCONN_DELAY_TIMEOUT_IN_MS (200)
#define MAINAPP_BTCUE_NOCONN_DELAY_TIMEOUT_IN_MS (100) //Nick modify to resolve the cue corrupt issue for V3
#define MAINAPP_TIMEOUT_IN_MS                    (100)
#define MAINAPP_TICK_IN_OFF_TIMEOUT_IN_MS        (200)
#define MAINAPP_TICK_IN_OFF_CHG_TIMEOUT_IN_MS    (300)

#define MAINAPP_LED_DIM_TIMEOUT_IN_MS            (30*1000)
#define MAINAPP_SHOW_ERR_SLEEP_TIMEROUT_IN_MS    (5*1000)
#define MAINAPP_WAIT_IN_OFF_REBOOT_TIMEOUT_IN_MS (10*1000)
#define MAINAPP_STACK_CHECK_TIMEOUT_IN_MS        (1000*10) /* 10s */
#define MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS     (2000)
#define MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS      (1000) /* 1s */
#define MAINAPP_BATT_CUE_PLAY_TIMEOUT_IN_MS      (4000)
#define MAINAPP_STAY_IN_OFF_TIMEOUT_IN_MS        (12*1000)

#define MAINAPP_PAIRING_FILTER_TIMEOUT_IN_MS     (3*1000)
#define MAINAPP_WAITING_FOR_LED_OPER_IN_MS       (800)
#define MAINAPP_WAITING_FOR_CUE_STOP_IN_MS       (1500)

#define MAINAPP_CUE_CMD_DELAY_TIMEOUT_IN_MS      (600)

#define MAINAPP_SYS_GAIN_ADJUST_IN_MS            (15*1000) /*15S*/

#define MAINAPP_SOURCE_CHANGE_TIMEOUT_IN_MS      (200)
#define MAINAPP_START_AUIDO_SWTH_TIMEOUT_IN_MS   (3000)
#define MAINAPP_WAIT_FOR_CUE_TIMEOUT_IN_MS       (800)
#define MAINAPP_BATTERY_LED_CHANGE_TIMEOUT_IN_MS (100)
#define MAINAPP_BATTERY_LED_FADE_TIMEOUT_IN_MS   (600)
#define MAINAPP_Connected_cue_IN_MS              (6*1000)

#define MAINAPP_SLEEP_TIMEOUT_IN_MS              (300)

#define MAINAPP_BatFullCue_TIMEOUT_IN_MS         (400)

#define BATTERY_LEVEL_IND_OFFSET                  5
/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
QState MainApp_Off(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e);
QState MainApp_Sleep(cMainApp * const me, QEvt const * const e);
QState MainApp_WaitBTPoweringUp(cMainApp * const me, QEvt const * const e);
QState MainApp_OffCharging(cMainApp * const me, QEvt const * const e);
QState MainApp_Shop(cMainApp * const me, QEvt const * const e);

QState MainApp_LS_Sample(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_PRIV_H */
