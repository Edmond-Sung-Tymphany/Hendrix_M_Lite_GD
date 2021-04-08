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
#include "MainApp_bluetoothEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp_pteCmdHandler.h"
#include "KeySrv.h"

/*****************************************************************
 * Type Definition
 *****************************************************************/

typedef enum
{
    REBOOT_CODE_INVALID = 0,
    REBOOT_CODE_BT_BOOTING_TIMEOUT,
    REBOOT_CODE_BT_COM_WDG_TIMEOUT,
    REBOOT_CODE_UPGRADE_TIMEOUT,
    REBOOT_CODE_PREACTIVE_TIMEOUT,
    REBOOT_CODE_POWERING_UP_TIMEOUT,
    REBOOT_CODE_POWERING_DOWN_TIMEOUT,
    REBOOT_CODE_FACTORY_RESET,
} eRebootCode;

typedef enum
{
    COM_KEY_INVALID = 0,
    PLAY_PAUSE_COMB, /* single press Play pause key */
    NEXT_TRACK_COMB, /* double press Play pause key */
    PREV_TRACK_COMB, /* triple press Play pause key */

    COM_KEY_ON_HOLD = 0x8000,
    SKIP_FAST_FW_COMB = COM_KEY_ON_HOLD + 100, /* single press then hold  Play pause key */
    SKIP_REWIND_COMB, /* double press then hold  Play pause key */

    COM_KEY_MAX
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    MAINAPP_FAST_BOOT_SIG,
    MAINAPP_PTE_TEST_CMD_SIG,
};



typedef enum
{
    TIMER_ID_BT_BOOTING_TIMEOUT = 0,
    TIMER_ID_DELAYED_ERROR_REBOOT,
    TIMER_ID_UPGRADE_TIMEOUT,
    TIMER_ID_FACTORY_RESET_TIMEOUT,
    TIMER_ID_POWER_DOWN_TIMEOUT,
    TIMER_ID_VOLUME_CHANGE_TIMEOUT,
    TIMER_ID_STACK_CHECK_TIMEOUT,
    TIMER_ID_AMP_ERR_CHECK_TIMEOUT,
    TIMER_ID_DOUBLE_PRESS_TIMEOUT,
    TIMER_ID_SEMI_ACTIVE_TIMEOUT,
    TIMER_ID_SOURCE_SWITCH_TIMEOUT,
    TIMER_ID_UNMUTE_SYS_TIMEOUT,
    TIMER_ID_BT_COMB_KEY_TIMEOUT,
    TIMER_ID_COMM_PING_INTERVAL,
    TIMER_ID_AUDIO_CUE_TIMEOUT,
    TIMER_ID_COMM_WDG_TIMEOUT,
    TIMER_ID_VOLUME_SYNC_TIMEOUT,
    TIMER_ID_BT_SYNC_TIMEOUT,
    TIMER_ID_MAX
} eTimerID;

typedef enum
{
    POWER_KEY_RELEASE   = 0x01,
    POWER_OFF_TIMEOUT   = 0x02,
    POWER_OFF_CMD_SENT  = 0x04,
    POWER_DOWN_DONE   = POWER_KEY_RELEASE | POWER_OFF_TIMEOUT | POWER_OFF_CMD_SENT,
} ePowerDownSteps;

#ifdef MAINAPP_DEBUG
const static char *mainApp_debug = "[MainApp_Debug] ";
#define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
#define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
#define MAINAPP_DEBUG_MSG(...)
#define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif

#define MAINAPP_BT_BOOTING_TIMEOUT_IN_MS     (120*1000)  /* Timeout for Bluetooth normal boot up: 2 min, normally need 53 sec to boot-up */
#define MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS     (5*1000)  /* wait 5 second(for finished LED indication), then reboot system. */
#define MAINAPP_POWER_DOWN_TIMEOUT_IN_MS         (2000)
#define MAINAPP_FACTORY_RESET_TIMEOUT_MS         (5*60*1000) /* Timeout for ASE-TK factory reset, normally need 70 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (15*60*1000) /* Timeout for ASE-TK upgrade:15 min */
#define MAINAPP_TIMEOUT_IN_MS                    (50)  /*50 ms */
#define MAINAPP_STACK_CHECK_TIMEOUT_IN_MS        (1000*10) /* 10s */
#define MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS      (2000) /* 2s */
#define MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS       (500) /* 0.5s */
#define MAINAPP_SEMI_ACTIVE_TIMEOUT_IN_MS        (30*1000) /* 30s */
#define MAINAPP_SEMI_ACTIVE_TIMEOUT_IN_SHOPMODE_IN_MS (45*1000) /* 45s */
#define MAINAPP_SOURCE_SWITCH_TIMEOUT            (150) /* 150ms */
#define MAINAPP_UNMUTE_SYS_TIMEOUT               (150) /* 150ms */
#define MAINAPP_BT_COMB_KEY_TIMEOUT              (800) /* 800ms */
#define MAINAPP_POWER_UP_CUE_TIMEOUT             (3000) /* 3s */
#define MAINAPP_POWER_UP_IN_SHOPMODE_TIMEOUT     (1500) /* 1.5s */
#define MAINAPP_AUDIO_CUE_TIMEOUT_IN_MS          (4000) /* 4s */
#define MAINAPP_COMM_PING_INTERVAL_IN_MS         (5000) /* 5s */
#define MAINAPP_COMM_WGD_TIMEOUT_IN_MS           (16000) /* 16s */

#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS     (1000) /*1 sec */
#define MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS      (200) /* 200ms */
#define MAINAPP_DELAY_BEFORE_JUMP_TO_STBL_IN_MS  (500) /*500 ms */
#define MAINAPP_VOLUME_SYNC_TIMEOUT_IN_MS        (100)
#define MAINAPP_BT_SYNC_TIMEOUT_IN_MS            (500) /*500ms*/

#define AUDIO_SOURCE_DEFAULT    AUDIO_SOURCE_BT

extern const uint8 vol_led_mapping[MAX_VOLUME_STEPS];

#define MAX_ABSOLUTE_VOLUME                      (127)
#define MIN_ABSOLUTE_VOLUME                      (0)
#define VOLUME_ABSOLUTE_TO_STEP(absolute)   (((absolute) >= MAX_ABSOLUTE_VOLUME) ? (MAX_VOLUME_STEPS-1) : \
                                                (((absolute) < 0) ? 0 : (absolute)/4))

#define VOLUME_STEP_TO_ABSOLUTE(step)       (((step) >= (MAX_VOLUME_STEPS-1)) ? MAX_ABSOLUTE_VOLUME : \
                                                (((step) < 0) ? MIN_ABSOLUTE_VOLUME : (4*(step))))

/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
QState MainApp_Off(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);
QState MainApp_BtModuleBooting(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_Standby(cMainApp * const me, QEvt const * const e);
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e);
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e);
QState MainApp_FirmwareUpdate(cMainApp * const me, QEvt const * const e);


/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_PRIV_H */
