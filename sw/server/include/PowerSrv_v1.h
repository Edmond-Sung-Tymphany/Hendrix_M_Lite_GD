/*****************************************************************************
*  @file      PowerSrv.h
*  @brief     Header file for base power server class
*  @author    Wesley Lee, Johnny Fan
*  @date      20-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef POWERSRV_SNKY_H
#define	POWERSRV_SNKY_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"
#include "product.config"

/* define it to enable the standby state which turn off some features to save power*/
//#define STANDBY_STATE_ENABLE
/*******************************************************/
/* declaration for struct, enum*/
/*******************************************************/
/* status*/
/* power server get the below status from driver*/
typedef struct
{
    bool    isDcPlugIn;              //it use ADC reading to detect DC, more spefic and good for UI
    bool    isUsbPlugIn;
    bool    isChargingDone;
    bool    isDcPlugInDetectByGPIO;  //it use GPIO reading to detect DC, used for interrupt status
    bool    isExtBattDetectByGPIO;  //it use GPIO reading to detect external battery, used for interrupt status
    int16   dcPlugInVoltage;         //for production test
}tInputSourceState;

typedef struct
{
	int16	intBatteryVol;
	int16   extBatteryVol;
}tBatteryVol;

/* when power server send the interrupt type, attach the input source state tInputSourceState, */
/* so that server can check whether it's rising or falling interrupt, for instance */
/* SendToServer(QActive* Owner, eInterruptType interruptType, tInputSourceState* inputSourceState)*/
typedef enum
{
    DC_INTERRUPT,
    USB_INTERRUPT,
    CHARGER_STAT_INTERRUPT,
}eInterruptType;

/* control path*/
/* server send the below command to driver to set different power path*/
typedef enum
{
    SET_DC_INPUT = 0,
    SET_USB_INPUT,
    SET_USB_NONE_INPUT,
    SET_NONE_INPUT,
    IGNORE_INPUT_SET,
} eInputSource;

typedef enum
{
    SET_INT_BATTERY = 0,
    SET_EXT_BATTERY,
    SET_NO_BATTERY,
    IGNORE_BATTERY_SET,
}eBatterySource;

typedef enum
{
    ENABLE_TPS54328_CMD = 0,
    DISABLE_TPS54328_CMD,
    ENABLE_SOL_CMD,
    DISABLE_SOL_CMD,
    SET_A_5V_CTL_CMD,
    CLR_A_5V_CTL_CMD,
    IGNORE_EXTRA_CMD,
}eExtraCommand;


/* ehe charging status, and AC/BATTERY mode*/
typedef enum
{
/* no ac is plugged, battery is discharging*/
    CHARGER_BATT_STA,
/* charger error stat: broken battery or no battery*/
    CHARGER_ERROR_STA,
/* battery is charging */
    CHARGER_CHARGING_STA,
/* battery charging is done*/
    CHARGER_CHARGING_DONE_STA,
    CHARGER_STATUS_MAX
} eChargerState;


/* the data stored into setting server*/
typedef struct tBatteryInfo
{
    bool isDataValid;     /* true if the battery voltage data is valid */
    bool isStateChange;
    eChargerState chargerState; /* charging state*/
    tBatteryVol voltage;
    uint8 intBattState;
#ifdef EXTERNAL_BATTERY
    uint8 extBattState;
#endif
    tInputSourceState inputSourceState;
}tBatteryInfo;

/*******************************************************/
/*  Old interface to consistant with polkPlay */

/*  need to remove later */
/*******************************************************/

/* this is to consistant with polkallplay*/


/*******************************************************/
/*  Set power battery/input source event*/
/*******************************************************/
REQ_EVT(PowerSrvSetEvt)
    eInputSource inputSource;
    eBatterySource batterySource;
    eExtraCommand extraCommand;
END_REQ_EVT(PowerSrvSetEvt)

RESP_EVT(PowerSrvSetRespEvt)
END_RESP_EVT(PowerSrvSetRespEvt)
/*************************************************************/

/*******************************************************/
/* Server => UI response/public event*/
/*******************************************************/
IND_EVT(PowerSrvInfoEvt)
    eEvtReturn result;
#ifdef HAS_BATTERY
    tBatteryInfo batteryInfo;
#endif
END_IND_EVT(PowerSrvInfoEvt)

/*SERVER => UI  (publish), showing the wake up interrupt, using signal POWER_WAKE_UP_SIG*/
typedef enum
{
    AC_PLUG_IN_WAKE_UP_TYPE,
    AC_PLUG_OUT_WAKE_UP_TYPE,
    EXT_BATTERY_IN_WAKE_UP_TYPE,
    MAX_WAKE_UP_TYPE
}ePowerSrvWakeUpType;

IND_EVT(PowerSrvWakeUpEvent)
    ePowerSrvWakeUpType powerSrvWakeUpType;
END_IND_EVT(PowerSrvWakeUpEvent)

/*******************************************************/
/*  For debug server and production test */
/*******************************************************/

typedef enum
{
    BATTERY_VALUE,
    AC_VALUE
}eRawValueType;
/* Production Test Module => Server request event*/
REQ_EVT(PowerSrvDebugReqEvt)
    eRawValueType valueType;
END_REQ_EVT(PowerSrvDebugReqEvt)
/* Power Server => Debug Server resp event*/
RESP_EVT(PowerSrvDebugRespEvt)
    eEvtReturn result;
    int32 adcRawValue;
END_RESP_EVT(PowerSrvDebugRespEvt)


/*******************************************************/
/* eject external request event */
/*******************************************************/
#ifdef EXTERNAL_BATTERY
typedef enum
{
    EJECT_EXT_BATTERY_EVT,
    MAX_TIME_EVT,
}eTimeEvent;

REQ_EVT(PowerSrvEjectBatReqEvt)
END_REQ_EVT(PowerSrvEjectBatReqEvt)

RESP_EVT(PowerSrvEjectBatRespEvt)
END_RESP_EVT(PowerSrvEjectBatRespEvt)
#endif


/*******************************************************/
/* power server class declaration */
/*******************************************************/
SUBCLASS(cPowerSrv, cServer)
    /* private: */
#ifdef HAS_BATTERY
    tBatteryInfo batteryInfo;
#endif
    int32 timer;
#ifdef EXTERNAL_BATTERY
    eTimeEvent curTimeEvent;
#endif
METHODS
    /* public functions */
END_CLASS

void PowerSrv_StartUp(cPersistantObj *me);
void PowerSrv_ShutDown(cPersistantObj *me);


#ifdef	__cplusplus
}
#endif

#endif	/* POWERSRV_H */
