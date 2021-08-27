/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Server Snky
                  -------------------------

                  SW Module Document




@file        PowerSrv.c
@brief       Provides general support for power request/responses
@author      Wesley Lee, Johnny Fan
@date        2014-02-10
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-02-10     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :

VERSION    : 2    DRAFT      2014-02-25     Johnny Fan
DESCRIPTION: second Draft.Add Battery monitor, and update it according to UI
SCO/ERROR  :

VERSION    : 3    snky        2014-07-16     Johnny Fan
DESCRIPTION: refactor the power server to make it simple and re-usable
SCO/ERROR  :

VERSION    : 4                2014-12-05    Johnny Fan
DESCRIPTION: refactor the power server to make it work for polkAllplay and iBT150
SCO/ERROR  :


-------------------------------------------------------------------------------
*/

/*******************************************************************************
 * State Machine Note
 * PowerSrv_PreActive: initial the hardware
 * PowerSrv_Active: update battery level
 * PowerSrv_DeActive:  Turn off WIFI/BT, CPU will got into idle to save power,
 *                    ErP-compliance power saving,
 *****************************************************************************/

#include "./PowerSrv_priv.h"


/*****************************************************************************
 * QP related Definition
 *****************************************************************************/
//Q_DEFINE_THIS_FILE

#define CAST_ME cPowerSrv * powerSrv = (cPowerSrv *) me;
static QEvt const * PowerSrvQueueSto[POWER_SRV_EVENT_Q_SIZE];
static cPowerDrv powerDrv;

enum
{
    POW_SRV_TIMEOUT_SIG = MAX_SIG ,
}eInternalSig;

/* the time (ms) per timeout signal */
#define POWER_SRV_TIMEOUT_IN_MS  10

/* the signal requestor*/
static QActive* pRequestor;

#ifdef HAS_BATTERY
/******************************************************
*  Local Macros
*****************************************************/
//use locally, return EJECT_BATT_STATE value if battery is ejected
#ifdef EXTERNAL_BATTERY
#define  EJECT_BATT_STATE   NUMBER_OF_STATE
#endif 

#define INVALID_BATT_STATE  0XFF

#ifdef DECREASING_ORDER_STATE
#define HIGHEST_STATE     0
#define LOWEST_STATE      (NUMBER_OF_STATE-1)
#endif

#ifdef INCREASING_ORDER_STATE
#define HIGHEST_STATE     NUMBER_OF_STATE-1)
#define LOWEST_STATE      0
#endif

/******************************************************
*  Local Variables
*****************************************************/
/* Battery variables */
static uint8 delayCheckChargingCount = 0;
static bool bIsChargingError = FALSE;
static tBatteryFilter battFilter = 
{
    .count = 0,
    .isReady = FALSE,
    .intBatt.isRemovable = FALSE,
#ifdef EXTERNAL_BATTERY
    .extBatt.isRemovable = TRUE,
#endif
};

#endif //endof #ifdef HAS_BATTERY


void PowerSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(powerSrv);
    QS_OBJ_DICTIONARY(PowerSrv_PreActive);
    QS_OBJ_DICTIONARY(PowerSrv_Active);
    QS_OBJ_DICTIONARY(PowerSrv_DeActive);

    PowerDrv_Ctor(&powerDrv);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&PowerSrv_Initial), POW_SRV_TIMEOUT_SIG,
                       PowerSrvQueueSto, Q_DIM(PowerSrvQueueSto), POWER_SRV_ID);

}

void PowerSrv_ShutDown(cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    PowerDrv_Xtor(&powerDrv);
    Server_Xtor((cServer*)me);
}

QState PowerSrv_Initial(cPowerSrv * const me, QEvt const * const e)
{
    (void)e; /* suppress the compiler warning about unused parameter */

#ifdef HAS_BATTERY
    PowerSrv_InitialBattVariable(me);
    PowerSrv_Update(me);
#ifdef EXTERNAL_BATTERY
    me->curTimeEvent = MAX_TIME_EVT;
#endif
#endif
    pRequestor = NULL;

#ifdef HAS_INTERRUPT_WAKE_UP_KEY
    QActive_subscribe((QActive*) me, POWER_MCU_SLEEP_SIG);
#endif

    return Q_TRAN(PowerSrv_DeActive);
}


/* Initial the Power Hardware */
QState PowerSrv_PreActive(cPowerSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            //printf("PowerSrv_PreActive()\n")
            me->timer = PowerDrv_InitialPower(&powerDrv);
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case POW_SRV_TIMEOUT_SIG:
        {
            me->timer -= POWER_SRV_TIMEOUT_IN_MS;
            if ( me->timer <= 0 )
            {
                me->timer = PowerDrv_InitialPower(&powerDrv);
                if(me->timer == 0)
                {
                    return Q_TRAN(&PowerSrv_Active);
                }
            }
#ifdef HAS_BATTERY
            PowerSrv_Update(me);
#endif
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Power Server super state */
QState PowerSrv_Active(cPowerSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            //printf("PowerSrv_Active()\n")
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            pRequestor = pReq->sender;
            return Q_TRAN(PowerSrv_DeActive);
        }
        case POW_SRV_TIMEOUT_SIG:
        {
#ifdef HAS_BATTERY
            /*update the battery and charger*/
            PowerSrv_Update(me);
#ifdef EXTERNAL_BATTERY
            if(me->curTimeEvent == EJECT_EXT_BATTERY_EVT)
            {/* call the eject battery function*/
                PowerSrv_EjectExtBattery(me);
            }
#endif
#endif
            /*Refresh the ticks*/
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_SET_SIG:
        {
            PowerSrvSetEvt* req = (PowerSrvSetEvt*)e;
            PowerSrv_Set(req);
            return Q_HANDLED();
        }
#ifdef EXTERNAL_BATTERY
        case EJECT_EXT_BATTERY_REQ_SIG:
        { /* eject external battery request*/
            PowerSrvEjectBatReqEvt* req = (PowerSrvEjectBatReqEvt*)e;
            pRequestor = req-> sender;
            me->timer = PowerDrv_EjectExtBattery(&powerDrv);
            me->curTimeEvent = EJECT_EXT_BATTERY_EVT;
            return Q_HANDLED();
        }
#endif
#endif
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Power DeActive status, ErP-compliance power saving, turn off the power*/
QState PowerSrv_DeActive(cPowerSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            //printf("PowerSrv_DeActive()\n")
            /*active the reset pin, and turn off external power*/
            PowerDrv_DeinitialPower(&powerDrv);
            /* response the sleep request */
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case POW_SRV_TIMEOUT_SIG:
        {
#ifdef HAS_BATTERY
            /*update the battery and charger*/
            PowerSrv_Update(me);
#ifdef EXTERNAL_BATTERY
            if(me->curTimeEvent == EJECT_EXT_BATTERY_EVT)
            {/* call the eject battery function*/
                PowerSrv_EjectExtBattery(me);
            }
#endif
#endif
            /*Refresh the ticks*/
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_MCU_SLEEP_SIG:
        {
            battFilter.count = 0;
            battFilter.isReady = FALSE;
            delayCheckChargingCount = GET_TICKS_IN_MS(CHECK_CHARGER_PIN_DELAY_MS) /
                                        GET_TICKS_IN_MS(POWER_SRV_TIMEOUT_IN_MS);
            PowerDrv_RegisterIntEvent((QActive*)me);
            PowerDrv_PowerSaveSleep();
            return Q_HANDLED();
        }
        case AC_IN_INTERRUPT_SIG:
        {/* receive the AC power in signal*/
            PowerSrv_PublishWakeUpEvent(me,AC_PLUG_IN_WAKE_UP_TYPE);
            TP_PRINTF("get AC interrupt \n\r");
            return Q_HANDLED();
        }
#ifdef EXTERNAL_BATTERY
        case EXT_BATTERY_IN_INTERRUPT_SIG:
        {/* receive the AC power in signal*/
            PowerSrv_PublishWakeUpEvent(me,EXT_BATTERY_IN_WAKE_UP_TYPE);
            TP_PRINTF("get Ext Battery interrupt \n\r");
            return Q_HANDLED();
        }
        case EJECT_EXT_BATTERY_REQ_SIG:
        { /* eject external battery request*/
            PowerSrvEjectBatReqEvt* req = (PowerSrvEjectBatReqEvt*)e;
            pRequestor = req-> sender;
            me->timer = PowerDrv_EjectExtBattery(&powerDrv);
            me->curTimeEvent = EJECT_EXT_BATTERY_EVT;
            return Q_HANDLED();
        }
#endif // endof #ifdef EXTERNAL_BATTERY
#endif // endof #ifdef HAS_BATTERY
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            pRequestor = pReq->sender;
            return Q_TRAN(PowerSrv_PreActive);
        }
        case Q_EXIT_SIG:
        {
#ifdef HAS_BATTERY
            PowerDrv_UnRegisterIntEvent();
#endif
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/***********************************************************************/
/* Local Used Functions */
/***********************************************************************/

static void PowerSrv_RefreshTick(cPowerSrv * const me)
{
    PersistantObj_RefreshTick((cPersistantObj*)me, POWER_SRV_TIMEOUT_IN_MS);
    BSP_FeedWatchdog();
}

#ifdef HAS_BATTERY
static void PowerSrv_Set(PowerSrvSetEvt* req)
{
    if(req->inputSource < IGNORE_INPUT_SET)
    {
        PowerDrv_SetInputSource(&powerDrv, req->inputSource);
    }
#ifdef EXTERNAL_BATTERY
    if(req->batterySource < IGNORE_BATTERY_SET)
    {
        PowerDrv_SetBatterySource(&powerDrv, req->batterySource);
    }
#endif
    if(req->extraCommand < IGNORE_EXTRA_CMD)
    {
        PowerDrv_SetExtraCommand(&powerDrv, req->extraCommand);
    }
    if(req->sender)
    {
        PowerSrvSetRespEvt* resp = Q_NEW(PowerSrvSetRespEvt, POWER_SET_RESP_SIG);
        resp->evtReturn = RET_SUCCESS;
        QACTIVE_POST(req->sender, (QEvt*)resp, 0);
    }
}

static void PowerSrv_PublishBatteryState(cPowerSrv * const me)
{
    PowerSrvInfoEvt *pe = Q_NEW(PowerSrvInfoEvt, POWER_BATT_STATE_SIG);
    memcpy((uint8*)&pe->batteryInfo, (uint8*)&me->batteryInfo, sizeof(me->batteryInfo));
    QF_PUBLISH(&pe->super, me);
}

static void PowerSrv_PublishWakeUpEvent(cPowerSrv * const me, ePowerSrvWakeUpType wakeUpType)
{
    PowerSrvWakeUpEvent *pe = Q_NEW(PowerSrvWakeUpEvent, POWER_WAKE_UP_SIG);
    pe->powerSrvWakeUpType = wakeUpType;
    QF_PUBLISH(&pe->super, me);
}

static void PowerSrv_InitialBattVariable(cPowerSrv * const me)
{
    /*initial ad state*/
    bool isAcPlugIn = TRUE;
    Setting_Set(SETID_AC_STATUS, &isAcPlugIn);
    /* Initial battery variables*/
    me->batteryInfo.isDataValid = FALSE;
    me->batteryInfo.isStateChange = FALSE;
    me->batteryInfo.chargerState = CHARGER_STATUS_MAX;
    me->batteryInfo.intBattState = INVALID_BATT_STATE;
#ifdef EXTERNAL_BATTERY
    me->batteryInfo.extBattState = INVALID_BATT_STATE;
#endif 
    Setting_Set(SETID_BATT_INFO, &me->batteryInfo);
    /* initial for checking charging status pin*/
    delayCheckChargingCount = 0;
    bIsChargingError = FALSE;
}

static void PowerSrv_Update(cPowerSrv * const me)
{
    PowerSrv_UpdateBatt(me);
    PowerSrv_UpdateInputSource(me);
    Setting_Set(SETID_BATT_INFO, &me->batteryInfo);
    if(me->batteryInfo.isStateChange)
    { /*publish the info only when battery and charger state change */
        me->batteryInfo.isStateChange = FALSE;
        PowerSrv_PublishBatteryState(me);
    }    
}

static void PowerSrv_UpdateBatt(cPowerSrv * const me)
{
    bool ret = FALSE;
    tBatteryVol batteryVol;
    ret = PowerDrv_GetBatteryVol(&powerDrv, &batteryVol);
    if(ret)
    {
        ret = PowerSrv_UpdateBattVoltage(batteryVol.intBatteryVol, batteryVol.extBatteryVol);
        if(ret)
        {
            me->batteryInfo.voltage.intBatteryVol = battFilter.intBatt.filterResult;
#ifdef EXTERNAL_BATTERY
            me->batteryInfo.voltage.extBatteryVol = battFilter.extBatt.filterResult;
#endif
            PowerSrv_UpdateBattState(me);
            if( CHARGER_ERROR_STA == me->batteryInfo.chargerState)
            {
                me->batteryInfo.isDataValid = FALSE;
            }
            else
            {
                me->batteryInfo.isDataValid = TRUE;
            }
        }
    }
}

static void PowerSrv_UpdateInputSource(cPowerSrv * const me)
{
    tInputSourceState inputState;
    /* detect the input source state from power driver */
    PowerDrv_GetInputSourceState(&powerDrv, &inputState);
    /* use filter to set the input source state in me->batteryInfo*/
    PowerSrv_SaveInputSource(me, &inputState);
    PowerSrv_UpdateCharger(me);
}

static void PowerSrv_UpdateCharger(cPowerSrv * const me)
{
    /* get last AC plug in state for charger stat detect delay purpose */
    bool isAcPlugIn = *(bool*)Setting_Get(SETID_AC_STATUS);
    /* the charger has delay on output charging status at STAT pin after AC in plugged in, so the functions needs to delay some time to read it */
    if(delayCheckChargingCount !=0)
    { /* if it's delaying, return and come to check next time event*/
        delayCheckChargingCount--;
        return;
    }
    if (me->batteryInfo.inputSourceState.isDcPlugIn && (!isAcPlugIn))
    {/* if AC power is pluged in, need to set up delay parameter to wait for STAT pin output*/
        Setting_Set(SETID_AC_STATUS, &me->batteryInfo.inputSourceState.isDcPlugIn);
        /* change this register to set the delay time*/
        delayCheckChargingCount = GET_TICKS_IN_MS(CHECK_CHARGER_PIN_DELAY_MS)/ 
                                    GET_TICKS_IN_MS(POWER_SRV_TIMEOUT_IN_MS);
        return;
    }
    Setting_Set(SETID_AC_STATUS, &me->batteryInfo.inputSourceState.isDcPlugIn);
    eChargerState chargeState;
    if (!me->batteryInfo.inputSourceState.isDcPlugIn)
    {
        chargeState = CHARGER_BATT_STA;
    }
    else if (bIsChargingError)
    {
        chargeState = CHARGER_ERROR_STA;
    }
    else if (me->batteryInfo.inputSourceState.isChargingDone)
    {
        chargeState = CHARGER_CHARGING_DONE_STA;
    }
    else
    {
        chargeState = CHARGER_CHARGING_STA;
    }
    if (me->batteryInfo.chargerState != chargeState)
    {
        me->batteryInfo.isStateChange = TRUE;
        me->batteryInfo.chargerState = chargeState;
    }
}


/* filter and set the input source state*/
static void PowerSrv_SaveInputSource(cPowerSrv * const me, tInputSourceState* inputSourceState)
{/* after geting the same input source state for INPUT_SOURCE_FILTER_LEN times, it will confirm it's stable and then set it in me->inputSourceState*/
    static uint8 dcPlugInOutCount = 0;
    static uint8 usbPlugInOutCount = 0;
    static uint8 chargingDoneCount = 0;
    static uint8 dcPlugInOutByGpioCount = 0;
    
    static bool preIsDcPlugIn = FALSE;
    static bool preIsUsbPlugIn = FALSE;
    static bool preIsChargingDone = FALSE;
    static bool preIsDcPlugInDetectByGPIO = FALSE;
    
    if(PowerSrv_IsInputSourceStable(&inputSourceState->isDcPlugIn, &preIsDcPlugIn , &dcPlugInOutCount))
    {
        me->batteryInfo.inputSourceState.isDcPlugIn = inputSourceState->isDcPlugIn;
    }
    if(PowerSrv_IsInputSourceStable(&inputSourceState->isUsbPlugIn, &preIsUsbPlugIn, &usbPlugInOutCount))
    {
        me->batteryInfo.inputSourceState.isUsbPlugIn = inputSourceState->isUsbPlugIn;
    }
    if(PowerSrv_IsInputSourceStable(&inputSourceState->isChargingDone, &preIsChargingDone, &chargingDoneCount))
    {
        me->batteryInfo.inputSourceState.isChargingDone = inputSourceState->isChargingDone;
    }
    if(PowerSrv_IsInputSourceStable(&inputSourceState->isDcPlugInDetectByGPIO, &preIsDcPlugInDetectByGPIO, &dcPlugInOutByGpioCount))
    {
        me->batteryInfo.inputSourceState.isDcPlugInDetectByGPIO = inputSourceState->isDcPlugInDetectByGPIO;
    }
#ifdef EXTERNAL_BATTERY
    static uint8 extBattPlugInOutByGPIOCount =0;
    static bool preIsExtBattDetectByGPIO = FALSE;
    if(PowerSrv_IsInputSourceStable(&inputSourceState->isExtBattDetectByGPIO, &preIsExtBattDetectByGPIO, &extBattPlugInOutByGPIOCount))
    {
        me->batteryInfo.inputSourceState.isExtBattDetectByGPIO = inputSourceState->isExtBattDetectByGPIO;
    }
#endif
    // update the ADC voltage for DC input, only for production test
    me->batteryInfo.inputSourceState.dcPlugInVoltage = inputSourceState->dcPlugInVoltage;
}

//if the input source state stable
static bool PowerSrv_IsInputSourceStable(bool* pCurInputState, bool* pPreInputState, uint8* pCount)
{
    bool ret = FALSE;
    if((*pPreInputState)==(*pCurInputState))
    {/* if it's the same as previous one, counter ++*/
        (*pCount)++;
        if((*pCount)> INPUT_SROUCE_FILTER_LEN)
        { /* over INPUT_SROUCE_FILTER_LEN times, set it as stable*/
            *pCount = 0;
            ret = TRUE;
        }
    }
    else
    { /* otherwise clear the counter, restart filter */
        *pCount = 0;
        (*pPreInputState) =(*pCurInputState);
    }
    return ret;
}

#ifdef BATT_HIGHEST_FILTER
static int16 PowerSrv_FindMaxValue(uint16* pArray, int16 size)
{
    int16 maxValue = 0;
    while(size)
    {
        if(maxValue< (*pArray))
        {
            maxValue = (*pArray);
        }
        size--;
        pArray ++;
    }
    return maxValue;
}

/**
* @brief: it's the battery filter which can filter out the voltage jitter whne playing strong music
* @Author: Johnny Fan
* @Note: below is the steps how it works:
*  1. when start the firmware for the first time , get the battery voltage quickly for battery indication
*  2. if the voltage sample is within oldvoltage +/- SAMPLE_RANGE_mV,
        then consider it as a valid sample and put it in the array. Otherwise discard it
*  3. if (OUT_OF_RANGE_ACCEPT_NUMBER) samples are out of range continuously, then consider it as valid
*  4. when we get BATT_FILTER_LEN number valid samples, find the max value and fill in intBatVoltageMaxValue[]
*  5. repeat step1~4, and when we get BATT_FILTER_LEN number max values in intBatVoltageMaxValue[],
        calculate its avarage value and consider it as a valid battery voltage, and publish it out
*  6. For external battery, there's insert/eject action in which the battery voltage will change quickly,
       so if we find it's inserted/ejected for (extBattActionDebounceCount) times, set the voltage as fininal battery voltage
*/
static bool PowerSrv_StartHigestFilter(tBattFilterCount* pBattFilterCount, 
    tBattFilterValue* pBattFilterValue, uint16 sample)
{
    bool ret = FALSE;    
    if (pBattFilterCount->isFirstStart)
    {
        pBattFilterCount->isFirstStart = FALSE;
        pBattFilterValue->filterResult = sample;
        ret = TRUE;
    }
    else
    {
        if(pBattFilterValue->isRemovable)
        {
            if (((sample < BATT_EJECT_mVOLT) && (pBattFilterCount->isInserted)) ||
             ((sample >= BATT_EJECT_mVOLT) && (!pBattFilterCount->isInserted)))
            { /* if we found there's extBattActionDebounceCount number eject/insert action */
                pBattFilterCount->actionDebounceCount++;
                if(pBattFilterCount->actionDebounceCount>OUT_OF_RANGE_ACCEPT_NUMBER)
                { /* directly set it as battery voltage, and reset the filter*/
                    pBattFilterCount->actionDebounceCount = 0;
                    pBattFilterCount->isInserted = !pBattFilterCount->isInserted;
                    pBattFilterCount->sampleIndex = 0;
                    pBattFilterCount->maxValueIndex = 0;
                    pBattFilterCount->maxValueSum = 0;  
                    pBattFilterValue->filterResult = sample;
                    ret  = TRUE;                    
                }
                else
                {
                    ret = FALSE;                    
                }
                return ret;
            }
        }
        pBattFilterCount->actionDebounceCount = 0;
        /* filter the internal battery*/
        if((sample <= (pBattFilterValue->filterResult + SAMPLE_RANGE_mV))&&
            (sample >= (pBattFilterValue->filterResult - SAMPLE_RANGE_mV)))
        { /* only get the sample within SAMPLE_RANGE_mV range*/
            
            pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
            pBattFilterCount->sampleIndex ++;
            pBattFilterCount->exceedRangeCount = 0;
        }
        else
        { /* if get intBattExceedRangeCount number out-of-range samples, then consider it as valid*/
            pBattFilterCount->exceedRangeCount++;
            if(pBattFilterCount->exceedRangeCount>=OUT_OF_RANGE_ACCEPT_NUMBER)
            {
                pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
                pBattFilterCount->sampleIndex ++;
                pBattFilterCount->exceedRangeCount = 0;
            }
        }
        if(pBattFilterCount->sampleIndex == BATT_FILTER_LEN)
        { /* if already get BATT_FILTER_LEN sample, then pick the max value to intBatVoltageMaxValue[]*/
            pBattFilterCount->sampleIndex = 0;            
            pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex] =
                PowerSrv_FindMaxValue(pBattFilterValue->sample, BATT_FILTER_LEN);
            pBattFilterCount->maxValueSum += pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex];            
            pBattFilterCount->maxValueIndex++;
            if(pBattFilterCount->maxValueIndex == BATT_FILTER_LEN)
            { /* get the average of all the max value and consider it as battery voltage*/
                pBattFilterCount->maxValueIndex = 0;
                pBattFilterValue->filterResult = (pBattFilterCount->maxValueSum / BATT_FILTER_LEN);
                pBattFilterCount->maxValueSum = 0;
                ret  = TRUE;
            }
        }
    }
    return ret;
}

static bool PowerSrv_UpdateBattVoltage(uint16 intBatteryVol, uint16 extBatteryVol)
{
    bool ret = FALSE;
    static tBattFilterCount intBattFilterCount = 
    {
        .isFirstStart = TRUE,
        .maxValueIndex = 0,
        .sampleIndex= 0,
        .exceedRangeCount = 0,
        .maxValueSum = 0,
    };
    ret = PowerSrv_StartHigestFilter(&intBattFilterCount, &battFilter.intBatt, intBatteryVol);
#ifdef EXTERNAL_BATTERY
    bool ret2 = FALSE;
    static tBattFilterCount extBattFilterCount=
    {
        .isFirstStart = TRUE,
        .maxValueIndex = 0,
        .sampleIndex= 0,
        .exceedRangeCount = 0,
        .maxValueSum = 0,
        .isInserted = FALSE,
        .actionDebounceCount = 0,
    };   
    ret2 = PowerSrv_StartHigestFilter(&extBattFilterCount, &battFilter.extBatt, extBatteryVol);
    /* set return value to TRUE when either battery got the result */
    if(ret2)
    {
        ret = TRUE;
    }
#endif
    return ret;
}

#else
/*average filter*/
static bool PowerSrv_UpdateBattVoltage(uint16 intBatteryVol, uint16 extBatteryVol)
{
    uint8 i;
    battFilter.intBatt.sample[battFilter.count] = intBatteryVol;
#ifdef EXTERNAL_BATTERY
    battFilter.extBatt.sample[battFilter.count] = extBatteryVol;
#endif
    battFilter.count ++;
    if(battFilter.count >= BATT_FILTER_LEN)
    {
        battFilter.count = 0;
        battFilter.isReady = TRUE;
    }
    if(battFilter.isReady == FALSE)
    {
        return FALSE;
    }
    battFilter.intBatt.filterResult = 0;
#ifdef EXTERNAL_BATTERY
    battFilter.extBatt.filterResult = 0;
#endif
    for(i=0;i<BATT_FILTER_LEN;i++)
    {
//average filter
        battFilter.intBatt.filterResult+= battFilter.intBatt.sample[i];
#ifdef EXTERNAL_BATTERY
        battFilter.extBatt.filterResult+= battFilter.extBatt.sample[i];
#endif
    }
    battFilter.intBatt.filterResult = battFilter.intBatt.filterResult / BATT_FILTER_LEN;
#ifdef EXTERNAL_BATTERY
    battFilter.extBatt.filterResult = battFilter.extBatt.filterResult / BATT_FILTER_LEN;
#endif
    return TRUE;
}
#endif //endof #ifdef BATT_HIGHEST_FILTER


/**
* @brief: use upper and lower threshold for each battery state detection, to make state more stable
* @Author: Johnny Fan
* @Note: below is the steps how it works:
   1. first check battery voltage if it's higher than current state's upper threshold,
      if yes, then continuously check the upper threshold to get the battery state
   2. if not, then check if voltage is lower than the current state's lower threshold,
      if yes, then continuously check the lower threshold to get the battery state
*/
static uint8 PowerSrv_GetBattState(uint8 preState,
            int16 batteryVol, const int16* highBound,  const int16* lowBound)
{
    uint8 battState = preState;
/* if previous battery state is not the highest state, and the current battery voltage is larger than the upper threshold
    * then increase the battery state by checking the upper threshold
    */
    if((preState != HIGHEST_STATE) &&
       (batteryVol > highBound[preState-1]))
    {
        for(;battState>HIGHEST_STATE;battState--)
        {
            if(batteryVol > highBound[battState-1])
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }
    /* else check the lower threshold, and decrease the battery state*/
    else
    {
        for(;battState<LOWEST_STATE;battState++)
        {
            if(batteryVol < lowBound[battState])
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }   
    return battState;
}


static void PowerSrv_UpdateBattState(cPowerSrv * const me)
{
    static uint8 preIntBattState = LOWEST_STATE;    
    me->batteryInfo.intBattState = PowerSrv_GetBattState(preIntBattState,
                            me->batteryInfo.voltage.intBatteryVol, 
                            intBattAdcHighBound, intBattAdcLowBound);
    if(preIntBattState != me->batteryInfo.intBattState)
    {
        preIntBattState = me->batteryInfo.intBattState;
        me->batteryInfo.isStateChange = TRUE;
    }
#ifdef EXTERNAL_BATTERY
    static uint8 preExtBattState = LOWEST_STATE;
    if(me->batteryInfo.voltage.extBatteryVol < BATT_EJECT_mVOLT)
    {
        me->batteryInfo.extBattState = EJECT_BATT_STATE;
    }
    else
    {
        me->batteryInfo.extBattState = PowerSrv_GetBattState(preExtBattState, 
                                        me->batteryInfo.voltage.extBatteryVol,
                                        extBattAdcHighBound, extBattAdcHighBound);
    }
    if(preExtBattState != me->batteryInfo.extBattState)
    {
        preExtBattState = me->batteryInfo.extBattState;
        me->batteryInfo.isStateChange = TRUE;
    }
#endif 
}

#ifdef EXTERNAL_BATTERY
static void PowerSrv_EjectExtBattery(cPowerSrv * const me)
{
    me->timer -= POWER_SRV_TIMEOUT_IN_MS;
    if(me->timer <= 0)
    {
        me->timer = PowerDrv_EjectExtBattery(&powerDrv);
        if(me->timer <= 0)
        {
            me->curTimeEvent = MAX_TIME_EVT;
            if(pRequestor)
            {
                PowerSrvEjectBatRespEvt* resp = Q_NEW(PowerSrvEjectBatRespEvt, EJECT_EXT_BATTERY_RESP_SIG);
                QACTIVE_POST((QActive*)pRequestor,(QEvt*)resp,me);
                pRequestor = NULL;
            }
        }
    }
}
#endif // endOf #ifdef EXTERNAL_BATTERY
#endif  //endOf #ifdef HAS_BATTERY
