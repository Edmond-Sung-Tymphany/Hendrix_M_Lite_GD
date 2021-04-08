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
#include "PowerDrv.h"


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

/* the signal requestor*/
static QActive* pRequestor;

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

    pRequestor = NULL;
    powerDrv.period = POWER_DRIVER_UPDATE_INTERVAL_MS;
    powerDrv.timetick = POWER_SRV_TIMEOUT_IN_MS;
#ifdef BnO_fs1
    //if powerDrv.timer = POWER_DRIVER_UPDATE_INTERVAL_MS, 
    //first printf battinfo will send unread value, so we let it send after Initial 2s to make sure read finished 
    powerDrv.timer = POWER_DRIVER_TIMER_INIT_VALUE;
#else
    powerDrv.timer = POWER_DRIVER_UPDATE_INTERVAL_MS;
#endif

    QActive_subscribe((QActive*) me, POWER_MCU_SLEEP_SIG);

    return Q_TRAN(PowerSrv_DeActive);
}


/* Initial the Power Hardware */
QState PowerSrv_PreActive(cPowerSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
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
            /* update Power driver status */
            PowerDrv_Update(&powerDrv);
            /*Refresh the ticks*/
            PowerSrv_RefreshTick(me);

            return Q_HANDLED();
        }
        case POWER_SET_SIG:
        {
            PowerSrvSetEvt* pReq = (PowerSrvSetEvt*)e;
            pRequestor = pReq->sender;
            PowerDrv_Set(&powerDrv, (ePowerSetId)(pReq->setId), pReq->enable);
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, POWER_SET_RESP_SIG);
            return Q_HANDLED();
        }
        case POWER_GET_SIG:
        {
            PowerSrvGetEvt* pReq = (PowerSrvGetEvt*)e;
            pRequestor = pReq->sender;
            PowerDrv_Get(&powerDrv, (ePowerGetId)pReq->getId);
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, POWER_GET_RESP_SIG);
            return Q_HANDLED();
        }
        case POWER_MCU_SLEEP_SIG:
        {
            PowerDrv_EnterSleepMode(me);
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

/* Power DeActive status, ErP-compliance power saving, turn off the power*/
QState PowerSrv_DeActive(cPowerSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            /*active the reset pin, and turn off external power*/
            PowerDrv_DeinitialPower(&powerDrv);
            /* response the sleep request */
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case POW_SRV_TIMEOUT_SIG:
        {
            /* update Power driver status */
            PowerDrv_Update(&powerDrv);
            /*Refresh the ticks*/
            PowerSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case POWER_MCU_SLEEP_SIG:
        {
            PowerDrv_EnterSleepMode(me);
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            pRequestor = pReq->sender;
            return Q_TRAN(PowerSrv_PreActive);
        }
        case POWER_SET_SIG:
        {
            PowerSrvSetEvt* pReq = (PowerSrvSetEvt*)e;
            pRequestor = pReq->sender;
            PowerDrv_Set(&powerDrv, (ePowerSetId)(pReq->setId), pReq->enable);
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, POWER_SET_RESP_SIG);
            return Q_HANDLED();
        }
        case POWER_GET_SIG:
        {
            PowerSrvGetEvt* pReq = (PowerSrvGetEvt*)e;
            pRequestor = pReq->sender;
            PowerDrv_Get(&powerDrv, (ePowerGetId)pReq->getId);
            CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, POWER_GET_RESP_SIG);
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

void PowerSrv_Set(QActive * sender, ePowerSetId id, bool enable)
{
    PowerSrvSetEvt* pReq = Q_NEW(PowerSrvSetEvt, POWER_SET_SIG);
    pReq->setId = id;
    pReq->sender = sender;
    pReq->enable = enable;
    SendToServer(POWER_SRV_ID, (QEvt*)pReq);
}

void PowerSrv_Get(QActive * sender, ePowerGetId id)
{
    
    PowerSrvGetEvt* pReq = Q_NEW(PowerSrvGetEvt, POWER_GET_SIG);
    pReq->getId = id;
    pReq->sender = sender;
    SendToServer(POWER_SRV_ID, (QEvt*)pReq);
}

/***********************************************************************/
/* Local Used Functions */
/***********************************************************************/

static void PowerSrv_RefreshTick(cPowerSrv * const me)
{
    PersistantObj_RefreshTick((cPersistantObj*)me, POWER_SRV_TIMEOUT_IN_MS);
    BSP_FeedWatchdog();
}

