/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef MAINAPP_PRIV_H
#define	MAINAPP_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "MainApp.h"
#include "KeySrv.h"

/*****************************************************************
 * Type Definition
 *****************************************************************/

typedef enum
{
    REBOOT_CODE_INVALID = 0,
    REBOOT_CODE_ASE_BOOTING_TIMEOUT,
    REBOOT_CODE_ASE_COM_WDG_TIMEOUT,
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
    POWER_IN_V_L_HOLD       = 0x40,
    POWER_SP_RELEASE        = 0X80,
    COM_KEY_SWITCH_SOURCE   = POWER_IN_HOLD | POWER_SP_RELEASE,
    COM_KEY_STANDBY         = POWER_IN_HOLD | POWER_IN_L_HOLD,
    COM_KEY_POWER_ON        = POWER_IN_HOLD | POWER_IN_L_HOLD,
    COM_KEY_IN_TURNING      = VOL_UP_KEY_IN_HOLD | VOL_DOWN_KEY_IN_HOLD,
    COM_KEY_IN_L_TURNING    = VOL_UP_KEY_IN_HOLD | VOL_UP_KEY_IN_L_HOLD | VOL_DOWN_KEY_IN_HOLD | VOL_DOWN_KEY_IN_HOLD,
    COM_KEY_VOL_UP_HOLD     = VOL_UP_KEY_IN_HOLD | VOL_UP_KEY_IN_L_HOLD,
    COM_KEY_VOL_DOWN_HOLD   = VOL_DOWN_KEY_IN_HOLD | VOL_DOWN_KEY_IN_L_HOLD,
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    MAINAPP_FAST_BOOT_SIG,
};

typedef enum {
    TIMER_ID_ASE_TK_BOOTING_TIMEOUT = 0,
    TIMER_ID_DELAYED_ERROR_REBOOT,
    TIMER_ID_UPGRADE_TIMEOUT,
    TIMER_ID_SW_CH,
    TIMER_ID_FACTORY_RESET_TIMEOUT,
    TIMER_ID_POWER_DOWN_TIMEOUT,
    TIMER_ID_VOLUME_CHANGE_TIMEOUT,
    TIMER_ID_MAX
}eTimerID;

#ifdef MAINAPP_DEBUG
    const static char *mainApp_debug = "[MainApp_Debug] ";
    #define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
    #define MAINAPP_DEBUG_MSG(...)
    #define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

#define MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS     (120*1000)  /* Timeout for ASE-TK normal boot up: 2 min, normally need 53 sec to boot-up */
#define MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS     (5*1000)  /* wait 5 second(for finished LED indication), then reboot system. */
#define MAINAPP_POWER_DOWN_TIMEOUT_IN_MS         (10*1000)
#define MAINAPP_FACTORY_RESET_TIMEOUT_MS         (5*60*1000) /* Timeout for ASE-TK factory reset, normally need 70 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (15*60*1000) /* Timeout for ASE-TK upgrade:15 min */
#define MAINAPP_TIMEOUT_IN_MS                    (100)  /*100 ms */
#define MAINAPP_LED_DIM_TIMEOUT_IN_MS            (5000) /*5 sec */
#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS     (4000) /*4 sec */
#define MAINAPP_SOURCE_SWITCH_DELAY_IN_MS        (1000) /* 1sec*/
#define MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS (400)  /* 400 ms  it should be larger than AUDIO_SWITCH_CHANNEL_DELAY_TIME (defined in audioSrv.config)*/
#define MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS      (200) /* 200ms */

#define ASE_TK_MAX_VOLUME              (90)  /*0-90 the value should be change according to setting in ASE-TK.*/
#define MAINAPP_VOL_STEP1  (MAX_VOLUME/CUST_MAX_VOL)
#define MAINAPP_VOL_STEP2  (MAX_VOLUME/CUST_MAX_VOL +1)
#define MAINAPP_SWITCH_STEP_VOL ((MAX_VOLUME - MAINAPP_VOL_STEP1 * CUST_MAX_VOL) * MAINAPP_VOL_STEP2)
#define MAINAPP_UPDATE_ASE_TK_VOL_INTERVAL (2) /* update sam vol every 2 steps duration when P&H vol key */

 
/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
QState MainApp_PowerInit(cMainApp * const me, QEvt const * const e);
QState MainApp_Off(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
QState MainApp_Wait(cMainApp * const me, QEvt const * const e);
QState MainApp_WaitUpgrade(cMainApp * const me, QEvt const * const e);
QState MainApp_WaitFactoryReset(cMainApp * const me, QEvt const * const e);
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e);
QState MainApp_AseTkBootingUp(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_FastBoot(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */
