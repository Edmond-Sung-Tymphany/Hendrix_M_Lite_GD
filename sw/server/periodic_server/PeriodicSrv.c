/*
-------------------------------------------------------------------------------
TYMPHANY LTD

                  Audio Server
                  -------------------------

                  SW Module Document

@file        PeriodicSrv.c
@brief       This file implements a periodic task.
@author      Viking Wang
@date        2016-07-01
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "controller.h"
#include "trace.h"
#include "signals.h"
#include "server.h"
#include "PeriodicSrv.h"
#include "PeriodicTask.h"

/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *PeriodicEvtQue[2];

/* internal signals */
enum InternalSignals
{
    TIMEOUT_SIG = MAX_SIG,
};

/* State function definitions */
static QState PeriodicSrv_Initial(cPeriodicSrv * const me);
static QState PeriodicSrv_Active(cPeriodicSrv * const me, QEvt const * const e);
#ifdef PERIODIC_SRV_HAS_DEACTIVE
static QState PeriodicSrv_DeActive(cPeriodicSrv * const me, QEvt const * const e);
#endif

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void PeriodicSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(PeriodicSrv_Active);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&PeriodicSrv_Initial), TIMEOUT_SIG,
                                PeriodicEvtQue, Q_DIM(PeriodicEvtQue), PERIODIC_SRV_ID);

    /* Subscribe */
}

void PeriodicSrv_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState PeriodicSrv_Initial(cPeriodicSrv * const me)
{
#ifdef PERIODIC_SRV_HAS_DEACTIVE
    return Q_TRAN(&PeriodicSrv_DeActive);
#else
    return Q_TRAN(&PeriodicSrv_Active);
#endif
}

#ifdef PERIODIC_SRV_HAS_DEACTIVE
static QState PeriodicSrv_DeActive(cPeriodicSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
    case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_TRAN(&PeriodicSrv_Active);
        }
    case SYSTEM_SLEEP_REQ_SIG:
        {
            ASSERT(0);  // something wrong...
            return Q_HANDLED();
        }
    case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}
#endif

static QState PeriodicSrv_Active(cPeriodicSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        {
            PeriodicTask_Init();
            PersistantObj_RefreshTick((cPersistantObj*)me, PERIODIC_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
    case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_HANDLED();
        }
     case TIMEOUT_SIG:
        {
            PeriodicTask_Execute();
            PersistantObj_RefreshTick((cPersistantObj*)me, PERIODIC_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
    case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            return Q_HANDLED();
        }
    case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}


