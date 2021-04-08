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
    MAINAPP_FAST_BOOT_SIG,
    MAINAPP_SET_AUDIO_MODE_SIG
};

typedef enum {
    TIMER_ID_ASE_TK_BOOTING_TIMEOUT = 0,
    TIMER_ID_DELAYED_ERROR_REBOOT,
    TIMER_ID_UPGRADE_TIMEOUT,
    TIMER_ID_SW_CH,
    TIMER_ID_FACTORY_RESET_TIMEOUT,
    TIMER_ID_POWER_DOWN_TIMEOUT,
    TIMER_ID_ERROR_UI_CHECK_TIMEOUT,
    TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT,
    TIMER_ID_ENTER_STORAGE_MODE_TIMEOUT,
    TIMER_ID_VOLUME_CHANGE_TIMEOUT,  
#ifdef BnO_fs1
    TIMER_ID_POWER_STATUS_UPDATE_TIMEOUT,
#endif    
    TIMER_ID_SYS_HEALTH_CHECK_TIMEOUT,
#ifdef HAS_DYNAMIC_MEMORY_CONTROL
    TIMER_ID_DYNAMIC_ANALYZE_TIMEOUT,
#endif
#ifdef  DEBUG_DEMO_FEATURE 
    TIMER_ID_DOUBLE_PRESS_FACTORY_RESET_TIMEOUT,
#endif      
    TIMER_ID_MAX
}eTimerID;


//type
//MAINAPP_SET_AUDIO_MODE_SIG:
REQ_EVT(MainAppSetAudioEvt)
    uint32 audio_mode;
END_REQ_EVT(MainAppSetAudioEvt)



#ifdef MAINAPP_DEBUG
    const static char *mainApp_debug = "[MainApp_Debug] ";
    #define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
    #define MAINAPP_DEBUG_MSG(...)
    #define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

/* MainApp Timer should be <= all the tick timer */
#define MAINAPP_TIMEOUT_IN_MS                    (100)  /*100 ms */

/* Tick Timer */
#define MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS     (120*1000)   /* Timeout for ASE-TK normal boot up: 2 min, normally need 53 sec to boot-up */
#define MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS (5*1000)   /* wait 5 second(for finished LED indication), then reboot system. */
#define MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS     (200)        /* 200ms */
#define MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS (200)        /* 200ms */
#define MAINAPP_DYNAMIC_ANALYZE_IN_MS            (120*1000)   /* 120sec */
#define MAINAPP_POWER_DOWN_TIMEOUT_IN_MS         (15*1000)    /* 15s */
#define MAINAPP_FACTORY_RESET_TIMEOUT_MS         (5*60*1000)  /* 5min. Timeout for ASE-TK factory reset, normally need 70 sec */
#define MAINAPP_ENTER_STORAGE_MODE_TIMEOUT_MS    (10*1000)    /* 10s */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (15*60*1000) /* Timeout for ASE-TK upgrade:15 min */
#define MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS      (200)        /* 200ms */
#define MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS       (1000)       /* 1s */

#ifdef BnO_fs1
#define MAINAPP_POWER_STATUS_UPDATE_TIMEOUT_IN_MS (1000*60)   /* 1min */
#endif

#define MAINAPP_SYS_HEALTH_CHECK_TIMEOUT_IN_MS    (1000*5)    /* 5s */


/* Ohter Timer */
#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS     (4000) /* 4 sec */
#define MAINAPP_SOURCE_SWITCH_DELAY_IN_MS        (1000) /* 1sec  */
#define MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS (400)  /* 400 ms  it should be larger than AUDIO_SWITCH_CHANNEL_DELAY_TIME (defined in audioSrv.config)*/
#define MAINAPP_WAKE_UP_SOURCES_DEBOUNCE_MS      (3000)
 
/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);

QState MainApp_PowerInit(cMainApp * const me, QEvt const * const e);
QState MainApp_Base(cMainApp * const me, QEvt const * const e);

#ifdef BnO_fs1 
    QState MainApp_Storage(cMainApp * const me, QEvt const * const e);
#endif

    QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
    QState MainApp_Off(cMainApp * const me, QEvt const * const e);
    QState MainApp_OffCharging(cMainApp * const me, QEvt const * const e);
    QState MainApp_LedBootingUp(cMainApp * const me, QEvt const * const e);
    QState MainApp_AseNgBootingUp(cMainApp * const me, QEvt const * const e);
    QState MainApp_Active(cMainApp * const me, QEvt const * const e);
	    QState MainApp_FastBoot(cMainApp * const me, QEvt const * const e);
    QState MainApp_Wait(cMainApp * const me, QEvt const * const e);
        QState MainApp_WaitUpgrade(cMainApp * const me, QEvt const * const e);
        QState MainApp_WaitFactoryReset(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */
