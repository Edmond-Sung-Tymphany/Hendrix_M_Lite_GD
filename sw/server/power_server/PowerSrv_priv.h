/*****************************************************************************
*  @file      PowerSrv_priv.h
*  @brief     Private header file for base power server class
*  @author    Johnny Fan
*  @date      25-Feb-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef POWERSRV_PRIV_H
#define	POWERSRV_PRIV_H

#include "../server_priv.h"
#include "bsp.h"      // for GET_TICKS_IN_MS()
#include "trace.h"
#include "controller.h"
#include "SettingSrv.h"
#include "PowerDrv.h"
#include "PowerSrv.Config"
#include "PowerSrv.h"
#include "commontypes.h"

#ifdef	__cplusplus
extern "C" {
#endif


#ifdef HAS_BATTERY

#define CHECK_CHARGER_PIN_DELAY_MS   1200

typedef struct 
{
    bool isRemovable;
    uint16 sample[BATT_FILTER_LEN];
#ifdef BATT_HIGHEST_FILTER
    uint16 maxValue[BATT_FILTER_LEN];
#endif
    uint16 filterResult;
}tBattFilterValue;

typedef struct 
{
    bool isReady;
    uint8 count;
    tBattFilterValue intBatt;
#ifdef EXTERNAL_BATTERY
    tBattFilterValue extBatt;
#endif
}tBatteryFilter;


typedef struct 
{
    bool isFirstStart;        
    uint8 sampleIndex;
    uint8 maxValueIndex;
    uint8 exceedRangeCount;
    bool  isInserted;
    uint8 actionDebounceCount;
    uint32 maxValueSum;
}tBattFilterCount;
#endif




/* private: */
QState PowerSrv_Initial(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_PreActive(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_Active(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_DeActive(cPowerSrv * const me, QEvt const * const e);


#ifdef HAS_BATTERY
static void PowerSrv_InitialBattVariable(cPowerSrv * const me);
static void PowerSrv_Update(cPowerSrv * const me);
static void PowerSrv_UpdateBatt(cPowerSrv * const me);
static int16 PowerSrv_FindMaxValue(uint16* pArray, int16 size);
static bool PowerSrv_StartHigestFilter(tBattFilterCount* pBattFilterCount, 
                                            tBattFilterValue* pBattFilterValue, uint16 sample);
static bool PowerSrv_UpdateBattVoltage(uint16 intBatteryVol, uint16 extBatteryVol);
static void PowerSrv_UpdateBattState(cPowerSrv * const me);


static void PowerSrv_UpdateInputSource(cPowerSrv * const me);
static void PowerSrv_UpdateCharger(cPowerSrv * const me);

static void PowerSrv_SaveInputSource(cPowerSrv * const me, tInputSourceState* inputSourceState);
static bool PowerSrv_IsInputSourceStable(bool* pCurInputState, bool* pPreInputState, uint8* pCount);


static void PowerSrv_PublishWakeUpEvent(cPowerSrv * const me, ePowerSrvWakeUpType wakeUpType);
static void PowerSrv_PublishBatteryState(cPowerSrv * const me);
#ifdef EXTERNAL_BATTERY
static void PowerSrv_EjectExtBattery(cPowerSrv * const me);
#endif//end of EXTERNAL_BATTERY
#endif //end of HAS_BATTERY

static void PowerSrv_RefreshTick(cPowerSrv * const me);


#ifdef	__cplusplus
}
#endif

#endif	/* POWERSRV_PRIV_H */

