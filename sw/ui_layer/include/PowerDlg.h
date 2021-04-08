/*****************************************************************************
*  @file      PowerDlg.h
*  @brief     Interface file for Power Delegate class
*  @author    Johnny Fan
*  @date      14-JUL-2014
*  @copyright Tymphany Ltd.

*****************************************************************************/

#ifndef POWER_DLG_H
#define POWER_DLG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qp_port.h"
#include "qf.h"
#include "signals.h"
#include "cplus.h"
#include "commonTypes.h"
#include "product.config"
#include "delegate.h"
#include "MainApp.h"  // for system status

#ifdef HAS_POWER_DELEGATE

typedef enum
{
    DC9V_MODE = 0,
    USB5V_CHARGE_INT_BATT_MODE,
    USB5V_CHARGE_EXT_BATT_MODE,
    NORMAL_BATTERY_MODE,
    LOW_BATTERY_MODE,
    SHUTDOWN_MODE, 
    EMERGENCY_MODE,
    MAX_POWER_MODE,
}ePowerMode; 



/* battery level states*/
typedef enum
{
    BATT_95_100_PERT_STA,       /* 0x0 */
    BATT_76_94_PERT_STA,        /* 0x1 */
    BATT_51_75_PERT_STA,        /* 0x2 */
    BATT_26_50_PERT_STA,        /* 0x3 */
    BATT_16_25_PERT_STA,        /* 0x4 */
    BATT_5_15_PERT_STA,         /* 0x5 */
    BATT_0_5_PERT_STA,          /* 0x6 */
#ifdef EXTERNAL_BATTERY
    EJECT_BATT_STA,             /* 0x7 */
#endif
    BATTERY_STATE_MAX,          /* 0x8 */
} eBatteryState;


typedef enum
{
    ENTER_EMERGENCY_MODE_CMD = 0,
    EXIT_EMERGENCY_MODE_CMD,
    MAX_EMERGENCY_MODE_CMD,
}eEmergencyModeCmd;

/*******************************************************/
/*  Set power delegate event*/
/*******************************************************/
REQ_EVT(PowerDlgSetEvt)
    eEmergencyModeCmd cmd;
END_REQ_EVT(PowerDlgSetEvt)

RESP_EVT(PowerDlgSetRespEvt)
END_RESP_EVT(PowerDlgSetRespEvt)


/*******************************************************/
/*  power delegate class */
/*******************************************************/
SUBCLASS(cPowerDlg, cDelegate)
    bool          isExtBatteryDock;
    bool          isDcPlugIn;
    bool          isEnterEmergencyReq;
    bool          isExitEmergencyReq;  
    ePowerMode    powerMode;
    eSystemStatus systemStatus;
    QTimeEvt timeEvt;
METHODS
/* public functions */
cPowerDlg * PowerDlg_Ctor(cPowerDlg * me, QActive *ownerObj);
void PowerDlg_Xtor(cPowerDlg * me);

END_CLASS

#endif /* HAS_POWER_DELEGATE */

#ifdef __cplusplus
}
#endif

#endif /* POWER_DLG_H */

