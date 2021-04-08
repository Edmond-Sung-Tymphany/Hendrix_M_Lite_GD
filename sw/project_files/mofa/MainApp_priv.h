/* 
 * File:   MainApp_priv.h
 * Author: Johnny.Fan
 *
 * Created on March 13, 2014, 2:11 PM
 */

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
    EXCEPTION_CODE_INVALID = 0,
    EXCEPTION_CODE_SAM_BOOTING_TIMEOUT,
    EXCEPTION_CODE_UPGRADE_TIMEOUT,
    EXCEPTION_CODE_PREACTIVE_TIMEOUT,
    EXCEPTION_CODE_POWERING_UP_TIMEOUT,
    EXCEPTION_CODE_POWERING_DOWN_TIMEOUT,
} eRebootCode;

typedef enum
{
    INVILAD_COM_KEY         = 0,
    NET_RESET_IN_HOLD       = 0x01,
    NET_RESET_IN_L_HOLD     = 0x02,
    NET_RESET_IN_V_L_HOLD   = 0x04,
    POWER_IN_HOLD           = 0X08,
    POWER_IN_L_HOLD         = 0x10,
    POWER_IN_V_L_HOLD       = 0x20,
    POWER_SP_RELEASE        = 0X40,
    SWITCH_SOURCE_TRIGGER   = POWER_IN_HOLD| POWER_SP_RELEASE,
    STANDBY_TRIGGER         = POWER_IN_HOLD| POWER_IN_L_HOLD,
    POWER_ON_TRIGGER        = POWER_IN_HOLD| POWER_IN_L_HOLD,
    BT_PAIRING_TRIGGER      = NET_RESET_IN_HOLD| NET_RESET_IN_L_HOLD,
    NETWORK_RESET_TRIGGER   = NET_RESET_IN_HOLD| NET_RESET_IN_L_HOLD | NET_RESET_IN_V_L_HOLD,
    FACTORY_RESET_TRIGGER   = NET_RESET_IN_HOLD| NET_RESET_IN_L_HOLD | NET_RESET_IN_V_L_HOLD | POWER_IN_HOLD | POWER_IN_L_HOLD | POWER_IN_V_L_HOLD,
} eCombinedKey;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
};

/* battery level states*/
typedef enum
{
    FULL_BATT_STA,
    NORM_BATT_STA,
    LOW_BATT_STA,
    CRITICAL_BATT_STA,
    SHUTDOWN_BATT_STA,
    BATTERY_STATE_MAX
} eBatteryStateSave;

typedef enum {
    SAM_BOOTING_TIMER = 0,
    /* This timer is just a workaround. It should be removed after Qualcomm fix below issue:
     * Issue: [Line-in] The line-in music will be mute after playing several seconds.
     * https://qualcomm-cdmatech-support.my.salesforce.com/5003000000aSmF7
     */
    WAIT_SAM_STABLE_TIMER,
    UPGRADING_TIMER,
    SW_CH_TIMER,
    SRC_SW_TIMER,
    LED_DIM_TIMER,
    BT_PAIRING_TIMER,
    BT_RECONNECT_TIMER,
    MAX_TIMER
}eTimerID;

#ifdef MAINAPP_DEBUG
    const static char *mainApp_debug = "[MainApp_Debug] ";
    #define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
    #define MAINAPP_DEBUG_MSG(...)
    #define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

#define MAINAPP_SAM_BOOTING_TIMEOUT_IN_MS       (2*60*1000) /*SAM booting timeout: 2 min */
#define MAINAPP_WAIT_SAM_STABLE_TIMEOUT_IN_MS   (25000) /*Wait SAM stable timeout: 15 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS           (90*60*1000) /*upgrade timeout: 90 min */
#define MAINAPP_TIMEOUT_IN_MS                   (100) /*100 ms */
#define MAINAPP_LED_DIM_TIMEOUT_IN_MS           (5000) /*5 sec */
#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS    (4000) /*4 sec */
#define MAINAPP_SOURCE_SWITCH_DELAY_IN_MS       (800) /* 800 ms*/
#define MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS (300) /* 300 ms */

#define ALLPLAY_MAX_VOLUME              (100)  /*the value should be change according to setting in SAM.*/
#define MainApp_mcuVol2AllplayVol(vol)  ((vol)*(me->allplayMaxVol)/MAX_VOLUME)
#define MainApp_allplayVol2McuVol(vol)  ((vol)*MAX_VOLUME/(me->allplayMaxVol))
#define MainApp_allplayVol2McuVol_errorMargin(vol)  ((vol)*MAX_VOLUME/(me->allplayMaxVol/2))
#define MAINAPP_UPDATE_SAM_VOL_INTERVAL (2) /* update sam vol every 2 steps duration when P&H vol key */

#define MAINAPP_BT_RECONNECT_TIMEOUT_IN_MS      (20000) /*20 sec */
#define MAINAPP_BT_PAIRING_TIMEOUT_IN_MS        (5*60*1000) /* 5 mins */

#define SWITCH_SOURCE_KEY           POWER_KEY /* power key is used as switch source key after powering on. */
/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
QState MainApp_Standby(cMainApp * const me, QEvt const * const e);
QState MainApp_Sleep(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e);
QState MainApp_Upgrading(cMainApp * const me, QEvt const * const e);
static QState MainApp_SamBootingUp(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
    QState MainApp_SourceSwitching(cMainApp * const me, QEvt const * const e);
    static QState MainApp_AudioJackIn(cMainApp * const me, QEvt const * const e);
    static QState MainApp_AllPlay_APMode(cMainApp * const me, QEvt const * const e);
    static QState MainApp_AllPlay_DirectMode(cMainApp * const me, QEvt const * const e);
    static QState MainApp_Bluetooth(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Other Functions */
QStateHandler* MainApp_SwitchAudioSource(cMainApp * const me, bool bDelay);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */

