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
#ifdef HAS_SHOP_MODE
typedef enum
{
    SHOP_MODE_DISABLE= 0, //normal mode
    SHOP_MODE_ENABLE,
    SHOP_MODE_MAX
} eShopMode;
#endif


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
	
    //XXX_IN_DOWN means single key down
    STANDBY_IN_DOWN         = 0X0001,
    STANDBY_IN_HOLD         = 0X0002,
    STANDBY_IN_L_HOLD       = 0x0004,
    VOL_UP_IN_DOWN          = 0x0008,
    VOL_DOWN_IN_DOWN        = 0x0010,
    CONNECT_IN_DOWN         = 0x0020,
    RESET_IN_DOWN           = 0x0040,
	
    //XXX_COMB_DOWN means combination key down
    VOL_UP_COMB_DOWN        = 0x0080,
    VOL_DOWN_COMB_DOWN      = 0x0100,
    CONNECT_COMB_DOWN       = 0x0200,
    STANDBY_COMB_DOWN       = 0x0400,
    RESET_COMB_DOWN         = 0X0800,
	
    //Combination key
    //This definition must the same as KeySrv.config
    COM_KEY_POWER_ON        = STANDBY_IN_HOLD | STANDBY_IN_L_HOLD,
    COM_KEY_NEXT            = CONNECT_COMB_DOWN | VOL_UP_COMB_DOWN,
    COM_KEY_PREV            = CONNECT_COMB_DOWN | VOL_DOWN_COMB_DOWN,
    COM_KEY_CONFIG          = RESET_COMB_DOWN | STANDBY_COMB_DOWN,
    COM_SHOP_MODE_CONFIG    = CONNECT_COMB_DOWN | RESET_COMB_DOWN,
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    MAINAPP_FAST_BOOT_SIG,
};

typedef enum {
    TIMER_ID_ASE_TK_BOOTING_TIMEOUT = 0,
    TIMER_ID_DELAYED_ERROR_REBOOT,
    TIMER_ID_DELAYED_ENTER_SHOP_MODE,
    TIMER_ID_DELAYED_POWER_INIT,
    TIMER_ID_UPGRADE_TIMEOUT,
    TIMER_ID_FACTORY_RESET_TIMEOUT,
    TIMER_ID_POWER_DOWN_TIMEOUT,
    TIMER_ID_STACK_CHECK_TIMEOUT,
    TIMER_ID_SYS_TEMP_CHECK_TIMEOUT,
    TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT,
    TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT,
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
#define MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS   (5*1000)  /* wait 5 second(for finished LED indication), then reboot system. */
#define MAINAPP_DELAYED_ENTER_SHOP_MODE_TIMEOUT_IN_MS   (2*1000)  /* wait 2 second to flash LED, then transit mode */
#define MAINAPP_POWER_DOWN_TIMEOUT_IN_MS         (3*1000)
#define MAINAPP_FACTORY_RESET_TIMEOUT_MS         (5*60*1000) /* Timeout for ASE-TK factory reset, normally need 70 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (15*60*1000) /* Timeout for ASE-TK upgrade:15 min */
#define MAINAPP_TIMEOUT_IN_MS                    (100)  /*100 ms */
#define MAINAPP_LED_DIM_TIMEOUT_IN_MS            (5000) /*5 sec */
#define MAINAPP_STACK_CHECK_TIMEOUT_IN_MS        (1000*120) /* 120s */
#define MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS     (1000) /* 1s */
#define MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS      (1000) /* 1s */
#define MAINAPP_DOUBLE_PRESS_TIMEOUT_IN_MS       (1000) /* 1s */   //(500) /* 0.5s */
#define MAINAPP_CLICK_SOUND_DELAY_IN_MS          (100) /* 100ms */
#define MAINAPP_ANTI_COMBO_DELAY_IN_MS           (1000)  /* 1s */

#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS     (4000) /*4 sec */
#define MAINAPP_SOURCE_SWITCH_DELAY_IN_MS        (1000) /* 1sec*/
#define MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS (400)  /* 400 ms  it should be larger than AUDIO_SWITCH_CHANNEL_DELAY_TIME (defined in audioSrv.config)*/

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
QState MainApp_AseNgBootingUp(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_ShopMode(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */
