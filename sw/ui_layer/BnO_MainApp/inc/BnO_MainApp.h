/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef BNO_MAINAPP_H
#define	BNO_MAINAPP_H

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
    REBOOT_CODE_UPGRADE_TIMEOUT,
    REBOOT_CODE_PREACTIVE_TIMEOUT,
    REBOOT_CODE_POWERING_UP_TIMEOUT,
    REBOOT_CODE_POWERING_DOWN_TIMEOUT,
} eRebootCode;

typedef enum
{
    COM_KEY_INVALID         = 0,
    FUNC_KEY_IN_HOLD        = 0x01,
    FUNC_KEY_IN_L_HOLD      = 0x02,
    FUNC_KEY_IN_V_L_HOLD    = 0x04,
    POWER_IN_HOLD           = 0X08,
    POWER_IN_L_HOLD         = 0x10,
    POWER_IN_V_L_HOLD       = 0x20,
    POWER_SP_RELEASE        = 0X40,
    COM_KEY_SWITCH_SOURCE   = POWER_IN_HOLD| POWER_SP_RELEASE,
    COM_KEY_STANDBY         = POWER_IN_HOLD| POWER_IN_L_HOLD,
    COM_KEY_POWER_ON        = POWER_IN_HOLD| POWER_IN_L_HOLD,
    COM_KEY_FUNCTION_KEY    = FUNC_KEY_IN_HOLD| FUNC_KEY_IN_L_HOLD,
    COM_KEY_FACTORY_RESET   = FUNC_KEY_IN_HOLD| FUNC_KEY_IN_L_HOLD | FUNC_KEY_IN_V_L_HOLD | POWER_IN_HOLD | POWER_IN_L_HOLD | POWER_IN_V_L_HOLD,
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
};

typedef enum {
    TIMER_ID_ASE_TK_BOOTING_TIMEOUT = 0,
    TIMER_ID_UPGRADE_TIMEOUT,
    TIMER_ID_SW_CH,
    TIMER_ID_FACTORY_RESET_TIMEOUT,
    TIMER_ID_POWER_DOWN_TIMEOUT,
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

#define MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS     (120*1000)  /* Timeout for ASE-TK normal boot up, normally need 53 sec */
#define MAINAPP_POWER_DOWN_TIMEOUT_IN_MS         (10*1000)
#define MAINAPP_FACTORY_RESET_TIMEOUT_MS         (5*60*1000) /* Timeout for ASE-TK factory reset, normally need 70 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (5*60*1000) /* Timeout for ASE-TK upgrade */
#define MAINAPP_TIMEOUT_IN_MS                    (100)  /*100 ms */
#define MAINAPP_LED_DIM_TIMEOUT_IN_MS            (5000) /*5 sec */
#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS     (4000) /*4 sec */
#define MAINAPP_SOURCE_SWITCH_DELAY_IN_MS        (1000) /* 1sec*/
#define MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS (400)  /* 400 ms  it should be larger than AUDIO_SWITCH_CHANNEL_DELAY_TIME (defined in audioSrv.config)*/
#define MAINAPP_POWER_UP_DOWN_TIMEOUT_IN_MS      (10*1000) /* power up/down timeout: 10 sec */

#define ASE_TK_MAX_VOLUME              (90)  /*0-90 the value should be change according to setting in ASE-TK.*/
#define MainApp_mcuVol2AseTkVol(vol)  ((vol)*(ASE_TK_MAX_VOLUME)/MAX_VOLUME)
#define MainApp_AseTkVol2McuVol(vol)  ((vol)*MAX_VOLUME/(ASE_TK_MAX_VOLUME))
#define MainApp_AseTkVol2McuVol_errorMargin(vol)  ((vol)*MAX_VOLUME/(ASE_TK_MAX_VOLUME/2))
#define MAINAPP_UPDATE_ASE_TK_VOL_INTERVAL (2) /* update sam vol every 2 steps duration when P&H vol key */

 
/************************************************************************************/
/* State function defintions */
void BnO_MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source, bool forceChgSrc);

QState BnO_MainApp_Off(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_NetworkStandby(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_Upgrading(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_SoftReset(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_AseTkBootingUp(cMainApp * const me, QEvt const * const e);

QState BnO_MainApp_Active(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* BNO_MAINAPP_H */
