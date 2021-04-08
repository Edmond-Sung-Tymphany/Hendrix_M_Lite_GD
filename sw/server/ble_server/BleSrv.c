/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Bluetooth Low Energy Server
                  -------------------------

                  SW Module Document




@file        BlleSrv.c
@brief       it's the server to control the BT module based onUart.
@author      Neo Gan
@date        2018-03-21
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#include "BleSrv_priv.h"

#include "settingsrv.h"
#include "trace.h"
#include "tym_qp_lib.h"

#ifdef BLE_DEBUG_ENABLE
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
    #define BLESRV_DEBUG_MSG TP_PRINTF
#else
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...)
    #define BLESRV_DEBUG_MSG(...)
#endif


/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *BleEvtQue[BLE_SRV_EVENT_Q_SIZE];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[3];
static QActive* pRequestor;

static cBleDrv bleDrv;
static cBleSrv *pBleSrv;

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BleSrv_StartUp(cPersistantObj *me)
{
    pBleSrv = (cBleSrv *)me;

    BleDrv_Ctor(&bleDrv);
    bleDrv.pSender = (QActive*)me;

    /* start up the object and let it run. Called by the controller */
    Server_Ctor((cServer*)me, Q_STATE_CAST(&BleSrv_Initial), BLE_TIMEOUT_SIG, BleEvtQue, Q_DIM(BleEvtQue), BLE_CTRL_SRV_ID);

    /* active object start */
    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
}

void BleSrv_ShutDown(cPersistantObj *me)
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
static QState BleSrv_Initial(cBleSrv * const me, QEvt const * const e)
{
    me->timeCount = 0;
    me->pSender = 0;
    me->excutingCmd = BLE_MAX_CMD;
    return Q_TRAN(&BleSrv_DeActive);
}

/* This state is used to as wait state. Wait until power  */
static QState BleSrv_PreActive(cBleSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            PersistantObj_RefreshTick((cPersistantObj*)me, BLE_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BLE_TIMEOUT_SIG:
        {
            uint16 nextDelayTime = BleDrv_Init(&bleDrv);
            if(nextDelayTime)
            {
                PersistantObj_RefreshTick((cPersistantObj*)me, nextDelayTime);
            }
            else
            {
                CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&BleSrv_Active);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED();
        }
        default:
            /* Not list all the QP signals here, so doesn't put a assert in the default*/
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*   Active state - first state where "normal" service begins  */
static QState BleSrv_Active(cBleSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            QActive_recall((QActive*)me, &deferredReqQue);
            PersistantObj_RefreshTick((cPersistantObj*)me, BLE_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            me->pSender = pReq->sender;
            return Q_TRAN(BleSrv_PreDeActive);
        }
        case BLE_TIMEOUT_SIG:
        {
            BleDrv_UpdateStatus(&bleDrv);
            PersistantObj_RefreshTick((cPersistantObj*)me, BLE_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BLE_REQ_SIG:
        {
            BleDrv_RequestHandler(&bleDrv, e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(&pBleSrv->timeEvt);
            return Q_HANDLED();
        }
        default:
        {
            if (e->sig > Q_USER_SIG)
            {
                BleDrv_HandleUnknown(&bleDrv, e);
                /*if (e->sig < Q_USER_SIG), dont return Q_HANDLED() */
                return Q_HANDLED();
            }
        }
    }
    return Q_SUPER(&QHsm_top);
}


/* This state is used to as wait state. Wait until power  */
static QState BleSrv_PreDeActive(cBleSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            PersistantObj_RefreshTick((cPersistantObj*)me, BLE_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BLE_TIMEOUT_SIG:
        {
            uint16 nextDelayTime = BleDrv_DeInit(&bleDrv);
            if(nextDelayTime)
            {
                PersistantObj_RefreshTick((cPersistantObj*)me, nextDelayTime);
            }
            else
            {
                //CommonEvtResp((QActive*)me, me->pSender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&BleSrv_DeActive);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED();
        }
        default:
            /* Not list all the QP signals here, so doesn't put a assert in the default*/
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*   DeActive state - Use this state to ramp down the server  */
static QState BleSrv_DeActive(cBleSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            QActive_recall((QActive*)me, &deferredReqQue);
            return Q_HANDLED();
        }
        /* fill me in */
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            pRequestor = pReq->sender;
            return Q_TRAN(&BleSrv_PreActive);
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        /* fill me in */
        default:
            break;
    }

    return Q_SUPER(&QHsm_top);
}


/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/

void BleSrv_SendBleCmd(eBleCmd cmd)
{
    BleReqEvt* req = Q_NEW(BleReqEvt, BLE_REQ_SIG);
    req->type = BLE_CTRL_REQ;
    req->bleCmd = cmd;
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)req);
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
 /*   Active state - first state where "normal" service begins  */

