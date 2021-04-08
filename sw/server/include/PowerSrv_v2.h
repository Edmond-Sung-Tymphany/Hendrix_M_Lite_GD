/*****************************************************************************
*  @file      PowerSrv.h
*  @brief     Header file for base power server class
*  @author    Wesley Lee, Johnny Fan
*  @date      20-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef POWERSRV_SNKY_H
#define POWERSRV_SNKY_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "server.h"
#include "product.config"
#include "PowerDrv.h"

typedef enum _BatteryStatus
{
    BatteryStatus_NO_BATTERY     = 0,
    BatteryStatus_LEVEL_CRITICAL = 1,
    BatteryStatus_LEVEL_LOW      = 2,
    BatteryStatus_LEVEL_MID      = 3,
    BatteryStatus_LEVEL_EXTRA    = 4,
    BatteryStatus_LEVEL_HIGH     = 5, //may be charging complete or under charing
    BatteryStatus_NUM            = 6
} BatteryStatus;

typedef enum _BTBattStatus
{
    BTBatteryStatus_Lv_0     = 0,
    BTBatteryStatus_Lv_1     = 1,
    BTBatteryStatus_Lv_2     = 2,
    BTBatteryStatus_Lv_3     = 3,
    BTBatteryStatus_Lv_4     = 4,
    BTBatteryStatus_Lv_5     = 5,
    BTBatteryStatus_Lv_6     = 6,
} BTBattStatus;


/* the data stored into setting server*/
typedef struct tBatteryInfo
{
    eChargerState chargerState; /* charging state*/
    BatteryStatus battStatus;
} tBatteryInfo;

typedef enum
{
    JACK_DET_IN_OUT,
    JACK_DET_LEVEL,
} eJackDetType;
/*******************************************************/
/*  Set power battery/input source event*/
/*******************************************************/
REQ_EVT(PowerSrvSetEvt)
uint8 setId;
bool enable;
END_REQ_EVT(PowerSrvSetEvt)

REQ_EVT(PowerSrvGetEvt)
uint8 getId;
END_REQ_EVT(PowerSrvGetEvt)


RESP_EVT(PowerSrvSetRespEvt)
END_RESP_EVT(PowerSrvSetRespEvt)
/*************************************************************/

/*******************************************************/
/* Server => UI response/public event*/
/*******************************************************/

/* Power Server => Debug Server resp event*/
RESP_EVT(PowerSrvDebugRespEvt)
eEvtReturn result;
int32 adcRawValue;
END_RESP_EVT(PowerSrvDebugRespEvt)

IND_EVT(PowerSrvInfoEvt)
tBatteryInfo batteryInfo;
eDcInSta dcInStatus; /* DC state */
END_IND_EVT(PowerSrvInfoEvt)

typedef enum
{
    AC_PLUG_IN_WAKE_UP_TYPE,
    AC_PLUG_OUT_WAKE_UP_TYPE,
    KEY_WAKE_UP_TYPE,
    MAX_WAKE_UP_TYPE
} ePowerSrvWakeUpType;

IND_EVT(PowerSrvWakeUpEvent)
ePowerSrvWakeUpType powerSrvWakeUpType;
END_IND_EVT(PowerSrvWakeUpEvent)

IND_EVT(PowerSrvJackStateEvt)
eJackDetType type;
int32 param;
END_IND_EVT(PowerSrvJackStateEvt)

/*******************************************************/
/* power server class declaration */
/*******************************************************/
SUBCLASS(cPowerSrv, cServer)
/* private: */
int32 timer;
METHODS
/* public functions */
END_CLASS

void PowerSrv_StartUp(cPersistantObj *me);
void PowerSrv_ShutDown(cPersistantObj *me);
void PowerSrv_Set(QActive * sender, ePowerSetId id, bool enable);
void PowerSrv_Get(QActive * sender, ePowerGetId id);


#ifdef  __cplusplus
}
#endif

#endif  /* POWERSRV_H */
