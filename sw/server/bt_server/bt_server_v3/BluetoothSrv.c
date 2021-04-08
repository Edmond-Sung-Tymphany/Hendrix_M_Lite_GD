/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Bluetooth Server
                  -------------------------

                  SW Module Document




@file        BluetoothSrv.c
@brief       it's the server to control the BT module based on Google protocol buffer and Uart.
@author      Daniel Qin
@date        2017-11-27
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#include "./BluetoothSrv_priv.h"
#include "settingsrv.h"

#define BT_DEBUG_ENABLEx
#ifndef BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif


/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *BluetoothEvtQue[BT_SRV_EVENT_Q_SIZE];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[3];

enum InternalSignals
{
    BT_TIMEOUT_SIG = MAX_SIG,
};

static uint8 resendNum = 0;
static cBluetoothDrv btDrv;
static cBluetoothSrv *pBtSrv;

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BluetoothSrv_StartUp(cPersistantObj *me)
{
    pBtSrv = (cBluetoothSrv *)me;

    BluetoothDrv_Ctor(&btDrv);
    BluetoothDrv_RegisterRxMsgCb(&btDrv, BluetoothSrv_ReveiveMsgCb);
    /* start up the object and let it run. Called by the controller */
    Server_Ctor((cServer*)me, Q_STATE_CAST(&BluetoothSrv_Initial), BT_TIMEOUT_SIG, BluetoothEvtQue, Q_DIM(BluetoothEvtQue), BT_SRV_ID);

    /* active object start */
    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
}

void BluetoothSrv_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
}

void BluetoothSrv_SendBtCmd(QActive* me, eBtCmd cmd)
{
    BtCmdEvt* req = Q_NEW(BtCmdEvt,BT_REQ_SIG);
    req->btCmd = cmd;
    req->sender = (QActive*)me;
    SendToServer(BT_SRV_ID, (QEvt*)req);
}

void BluetoothSrv_SendBtCmdWithParam(QActive* me, eBtCmd cmd, uint8 length ,uint8* param)
{
    BtCmdEvt* req = Q_NEW(BtCmdEvt,BT_REQ_SIG);
    req->btCmd = cmd;
    req->sender = (QActive*)me;
    req->length = length;
    ASSERT(length <= BT_MAX_EVT_PARAM_LENGTH);
    memcpy(req->param,param,length);
    SendToServer(BT_SRV_ID, (QEvt*)req);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState BluetoothSrv_Initial(cBluetoothSrv * const me, QEvt const * const e)
{
    me->timeCount = 0;
    me->pSender = 0;
    me->executingCmd = BT_MAX_CMD;
    return Q_TRAN(&BluetoothSrv_DeActive);
}

/* This state is used to as wait state. Wait until power  */
static QState BluetoothSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            BtDrv_PowerOnSeqInit(&btDrv);
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            if (!BtDrv_isSeqFinished(&btDrv))
            {
                BtDrv_SeqRefresh(&btDrv, BT_SRV_PER_TICK_TIME_MS);
            }
            else
            {
                CommonEvtResp((QActive*)me, me->pSender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&BluetoothSrv_Active);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
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
static QState BluetoothSrv_Active(cBluetoothSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            me->pSender = pReq->sender;
            return Q_TRAN(BluetoothSrv_PreDeActive);
        }
        case BT_TIMEOUT_SIG:
        {
            /* Note:
               * All the project spicific status checking and updating code should be implemented
               * in API: BtDrv_UpdateStatus in file
               * sw\project_files\xxx\portingLayer\btdrv\BluetoothDrv.c
               */
            BtDrv_UpdateStatus(&btDrv);

            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BT_REQ_SIG:
        {
            BtCmdEvt* evt = (BtCmdEvt*) e;
            me->executingCmd = evt->btCmd;
            me->executingCmdLength = evt->length;
            memcpy(me->executingCmdParam,evt->param,evt->length);
            BluetoothDrv_ExecuteCmd(&btDrv, evt->btCmd, evt->length,evt->param);
            return Q_TRAN(BluetoothSrv_ExecutingCmd);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&pBtSrv->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*   Active state - first state where "normal" service begins  */
static QState BluetoothSrv_ExecutingCmd(cBluetoothSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            me->timeCount = BT_CMD_ACK_TIMEOUT_MS;
            resendNum = 0;
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            me->timeCount -= BT_SRV_PER_TICK_TIME_MS;
            if (me->timeCount <= 0)
            {
                /* If timeout, then resend cmd. */
                TP_PRINTF("cmd reponse time out \r\n");
                BluetoothSrv_ResendCmd(me);
            }
            break;
        }
        case BT_STATUS_SIG:
        {
            BtRxEvt *rxEvt = (BtRxEvt*)e;
            if (rxEvt->type == BT_MSG_TYPE_NACK)
            {
                /* If NACK, then resend cmd. */
                TP_PRINTF("cmd reponse NACK \r\n");
                BluetoothSrv_ResendCmd(me);
            }
            else
            {
                   BtCmdEvt* req = Q_NEW(BtCmdEvt,BT_CMD_DONE_SIG);
                   SendToServer(BT_SRV_ID, (QEvt*)req);
            }
            break;
        }
        case BT_CMD_DONE_SIG:
        {
            TP_PRINTF("cmd done\r\n");
            QActive_recall((QActive*)me, &deferredReqQue);
            return Q_TRAN(&BluetoothSrv_Active);
        }
        case BT_REQ_SIG:
        {
            BtCmdEvt* evt = (BtCmdEvt*) e;
            QActive_defer((QActive*)me, &deferredReqQue, (QEvt *)evt);
            TP_PRINTF("Executing previous cmds now, so deferred the request %d\r\n", evt->btCmd);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            me->timeCount = 0;
            QTimeEvt_disarm(&pBtSrv->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&BluetoothSrv_Active);
}

/* This state is used to as wait state. Wait until power  */
static QState BluetoothSrv_PreDeActive(cBluetoothSrv * const me, QEvt const * const e)
{
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            BtDrv_PowerOffSeqInit(&btDrv);
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, BT_SRV_PER_TICK_TIME_MS);
            if (!BtDrv_isSeqFinished(&btDrv))
            {
                BtDrv_SeqRefresh(&btDrv, BT_SRV_PER_TICK_TIME_MS);
            }
            else
            {
                CommonEvtResp((QActive*)me, me->pSender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&BluetoothSrv_DeActive);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
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
static QState BluetoothSrv_DeActive(cBluetoothSrv * const me, QEvt const * const e)
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
            me->pSender = pReq->sender;
            return Q_TRAN(&BluetoothSrv_PreActive);
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
 * private functions
 *
 *****************************************************************************************************************/
/* The callback function will be performed when reveived message from bluetooth module.
* But please do NOT handle message in this function as it may be called in interrupt handler in
* driver layer.
*/
static void BluetoothSrv_ReveiveMsgCb(eBtReceivedMsgType type, uint8 *pData)
{
    if (BT_MSG_TYPE_DATA == type)
    {
        BtRxEvt* pEvt = Q_NEW(BtRxEvt, BT_STATE_SIG);
        pEvt->type = type;
        /* If it is data type msg, publish to system. */
        memcpy(&pEvt->data, pData, BT_RX_EVT_DATA_LEN);
        QF_PUBLISH(&pEvt->super, (QActive*)pBtSrv);
    }
    else
    {
        BtRxEvt* pEvt = Q_NEW(BtRxEvt, BT_STATUS_SIG);
        pEvt->type = type;
        /* If it is ACK/NACK msg, just send to bt server for handling. */
        QACTIVE_POST((QActive*)pBtSrv,(QEvt*)pEvt,0);
    }
}

static void BluetoothSrv_ResendCmd(cBluetoothSrv * const me)
{
    if (resendNum >= BT_CMD_MAX_RESEND_NUM)
    {
        //ASSERT(0);
        BtCmdEvt* req = Q_NEW(BtCmdEvt,BT_CMD_DONE_SIG);
        SendToServer(BT_SRV_ID, (QEvt*)req);
    }
    me->timeCount = BT_CMD_ACK_TIMEOUT_MS;
    BluetoothDrv_ExecuteCmd(&btDrv, me->executingCmd, me->executingCmdLength,me->executingCmdParam);
    resendNum++;

}

