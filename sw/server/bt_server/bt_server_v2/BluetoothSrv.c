/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Bluetooth Server
                  -------------------------

                  SW Module Document




@file        BluetoothSrv.c
@brief       it's the server to control the BT module
@author      Johnny Fan
@date        2014-05-11
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-05-11     Johnny Fan
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "./BluetoothSrv_priv.h"
#include "settingsrv.h"

#ifdef Q_SPY
#define CAST_ME cBluetoothSrv * BluetoothSrv = (cBluetoothSrv *) me;
#else
#define CAST_ME cBluetoothSrv * BluetoothSrv = (cBluetoothSrv *) me;
#endif


#ifndef BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif


#ifdef BT_DEBUG_ENABLE
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
    #define AUDIOSRV_DEBUG_MSG TP_PRINTF
#else
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...)
    #define AUDIOSRV_DEBUG_MSG(...)
    #undef   TP_PRINTF
    #define  TP_PRINTF(...)
#endif


/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *BluetoothEvtQue[BT_SRV_EVENT_Q_SIZE];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[3];
static QActive* pRequestor;

enum InternalSignals
{
    BT_TIMEOUT_SIG = MAX_SIG,
};

/* the number of ticks for QP to trigger timer out signal*/
const uint16 BT_SRV_TICK_TIME = GET_TICKS_IN_MS(BT_SRV_PER_TICK_TIME_MS);

static cBluetoothDrv bluetoothDrv;

static tBtTimeStruct requestTime;

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BluetoothSrv_StartUp(cPersistantObj *me)
{
    CAST_ME;
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(BluetoothSrv);
    QS_OBJ_DICTIONARY(BluetoothSrv_PreActive);
    QS_OBJ_DICTIONARY(BluetoothSrv_Active);
    QS_OBJ_DICTIONARY(BluetoothSrv_DeActive);

    BluetoothDrv_Ctor(&bluetoothDrv);
    BluetoothDrv_RegisterTimeReqCb(&bluetoothDrv, &BluetoothSrv_SetTimeReq);
    BluetoothDrv_RegisterDriverSig(&bluetoothDrv, (QActive*)me);
    QTimeEvt_ctorX(&BluetoothSrv->timeEvt, (QActive*)BluetoothSrv, BT_TIMEOUT_SIG, 0U);
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

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState BluetoothSrv_Initial(cBluetoothSrv * const me, QEvt const * const e)
{
    me->isCmdExcuting = FALSE;
    me->cmdQueueDelayMs = 0;
    me->isQueueWaiting = FALSE;
    return Q_TRAN(&BluetoothSrv_DeActive);
}

/* This state is used to as wait state. Wait until power  */
static QState BluetoothSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e)
{
    CAST_ME;
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
            BluetoothDrv_RegisterDriverSig(&bluetoothDrv, (QActive*)me);
#ifdef HAS_BT_SEQ_CONTROL
            me->timeCount = BluetoothDrv_StartupInit(&bluetoothDrv);
#else
            me->timeCount = BT_START_UP_DELAY_MS;
#endif
            BluetoothSrv_RefreshTick(BluetoothSrv);
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            me->timeCount -= BT_SRV_PER_TICK_TIME_MS;
#ifdef HAS_BT_SEQ_CONTROL
            if (me->timeCount <= 0)
            {
                me->timeCount = BluetoothDrv_StartupInit(&bluetoothDrv);
                if( me->timeCount <= 0 )
                {
                    CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                    return Q_TRAN(&BluetoothSrv_Active);
                }
            }
#else
            if (me->timeCount <= 0)
            {
                CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                return Q_TRAN(&BluetoothSrv_Active);
            }
            else
#endif
            {
                BluetoothSrv_RefreshTick(BluetoothSrv);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&BluetoothSrv->timeEvt);
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
    CAST_ME;
    uint32_t bt_status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
#ifndef HAS_BT_SEQ_CONTROL
            BluetoothDrv_TurnOnBT(&bluetoothDrv);
#endif
            BluetoothSrv_RefreshTick(BluetoothSrv);
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            /* Update the timer calculation*/
            BluetoothSrv_UpdateTime(me);
#ifdef HAS_FIX_VOL_AUDIO_CUE
            BluetoothSrv_DelayStartAudioCue(me);
            /* Check Audio Cue Playing time */
            BluetoothSrv_CheckAudioCuePlaying(me);
#endif
            /* check event queue and recall the command event*/
            BluetoothSrv_CheckQueueAndRecallCmdEvt(me);
            BluetoothSrv_RefreshTick(BluetoothSrv);
            return Q_HANDLED();
        }
        case BT_STATUS_SIG:
        {
            BtDrvStatusEvt* pEvt = (BtDrvStatusEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "BluetoothSrv_Active:BT_STATUS_SIG:pEvt->btStatus = %d", pEvt->btStatus);
            /*Don't add filter to ignore same status here. It's better to keep common code for all projects.*/
            if(pEvt->btStatus> BT_MAX_STA)
            {
                /* it's a indication*/
                me->btIndEvt = (eBtIndEvt)pEvt->btStatus;
                TP_PRINTF("it's the event indication\r\n");
                BluetoothSrv_PublishStatus(me, FALSE);
            }
            else
            {
                /* it's a BT state*/
                me->btStatus = (eBtStatus) pEvt-> btStatus;
                bt_status = me->btStatus;
                Setting_Set(SETID_BT_STATUS, &bt_status);
                BluetoothSrv_PublishStatus(me, TRUE);
            }
            return Q_HANDLED();
        }
        case BT_CMD_DONE_SIG:
        {
            TP_PRINTF("cmd done\r\n");
            me->isQueueWaiting = TRUE;
            me->cmdQueueDelayMs = BT_CMD_MIN_GAP_MS;
#ifdef HAS_FIX_VOL_AUDIO_CUE
            BluetoothDrv_SetDelayStartAudioCue(AUDIO_CUE_START_DELAY_TIME);
#endif
            return Q_HANDLED();
        }
        case BT_REQ_SIG:
        {
            BtCmdEvt* evt = (BtCmdEvt*) e;
            if(evt->btCmd>BT_COMMON_MAX_CMD)
            {
                /* for production test only*/
                if(evt->btCmd == BT_PWR_ON_CMD)
                {
                    BluetoothDrv_TurnOnBT(&bluetoothDrv);
                }
                else if(evt->btCmd == BT_PWR_OFF_CMD)
                {
                    BluetoothDrv_TurnOffBT(&bluetoothDrv);
                }
                return Q_HANDLED();
            }

            if(me->isCmdExcuting)
            {
                QActive_defer((QActive*)me, &deferredReqQue, e);
                TP_PRINTF("save the request %d\r\n", evt->btCmd);
            }
            else
            {
                me->isCmdExcuting = TRUE;
                BluetoothSrv_ExecuteCmd((BtCmdEvt*) e);
                TP_PRINTF("excute BT request %d\r\n", evt->btCmd);
            }
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            pRequestor=((CommonReqEvt*)e)->sender;
            return Q_TRAN(BluetoothSrv_PreDeActive);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&BluetoothSrv->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* This state is used to as wait state. Wait until power  */
static QState BluetoothSrv_PreDeActive(cBluetoothSrv * const me, QEvt const * const e)
{
    CAST_ME;
    switch(e->sig)
    {
        case Q_ENTRY_SIG:
        {
#ifdef HAS_BT_SEQ_CONTROL
            me->timeCount = BluetoothDrv_PowerOffInit(&bluetoothDrv);
#else
            me->timeCount = BT_SRV_PER_TICK_TIME_MS;
#endif
            BluetoothSrv_RefreshTick(BluetoothSrv);
            return Q_HANDLED();
        }
        case BT_TIMEOUT_SIG:
        {
            me->timeCount -= BT_SRV_PER_TICK_TIME_MS;
#ifdef HAS_BT_SEQ_CONTROL
            if (me->timeCount <= 0)
            {
                me->timeCount = BluetoothDrv_PowerOffInit(&bluetoothDrv);
                if( me->timeCount <= 0 )
                {
                    CommonReqEvt* pReq = (CommonReqEvt*) e;
                    CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
                    return Q_TRAN(&BluetoothSrv_DeActive);
                }
            }
#else
            if (me->timeCount <= 0)
            {
                CommonReqEvt* pReq = (CommonReqEvt*) e;
                CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
                return Q_TRAN(&BluetoothSrv_DeActive);
            }
            else
#endif
            {
                BluetoothSrv_RefreshTick(BluetoothSrv);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&BluetoothSrv->timeEvt);
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
    uint32_t bt_status;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->btStatus = BT_MAX_STA;
            /*un-regester to get more BT status before power down, avoid noise*/
            BluetoothDrv_UnRegisterDriverSig(&bluetoothDrv);
            BluetoothDrv_TurnOffBT(&bluetoothDrv);
            bt_status = me->btStatus;
            Setting_Set(SETID_BT_STATUS, &bt_status);
            return Q_HANDLED();
        }
        /* fill me in */
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            pRequestor=((CommonReqEvt*)e)->sender;
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
static uint32 BluetoothSrv_SetTimeReq(eTimeType reqTimeType, int32 timeInMs)
{
    uint32 rTime = 0;
    switch(reqTimeType)
    {
        case BT_START_TIME:
        {
            ASSERT(!requestTime.countEnable);
            requestTime.countEnable = TRUE;
            requestTime.countInMs = 0;
            break;
        }
        case BT_END_TIME:
        {
            requestTime.countEnable = FALSE;
            rTime = requestTime.countInMs;
            requestTime.countInMs = 0;
            break;
        }
        case BT_SET_TIME:
        {
            ASSERT(!requestTime.delayEnable);
            requestTime.delayEnable= TRUE;
            requestTime.delayInMs= timeInMs;
            break;
        }
        default:
            break;
    }
    return rTime;
}

static void BluetoothSrv_UpdateTime(cBluetoothSrv * const me)
{
    if(requestTime.countEnable)
    {
        requestTime.countInMs += BT_SRV_PER_TICK_TIME_MS;
    }
    if(requestTime.delayEnable)
    {
        requestTime.delayInMs -=BT_SRV_PER_TICK_TIME_MS;
        if(requestTime.delayInMs<=0)
        {
            requestTime.delayEnable = FALSE;
            BluetoothDrv_TimeIsUp(&bluetoothDrv);
        }
    }
}

#ifdef HAS_FIX_VOL_AUDIO_CUE
static void BluetoothSrv_CheckAudioCuePlaying(cBluetoothSrv * const me)
{
    uint32_t bt_status;
    if (BluetoothDrv_CheckAudioCuePlaying(BT_SRV_PER_TICK_TIME_MS))
    {
        bt_status = BT_AUDIO_CUE_STOP_STA;
        Setting_Set(SETID_BT_STATUS, &bt_status);
        BluetoothSrv_PublishStatus(me, TRUE);
    }
}


static void BluetoothSrv_DelayStartAudioCue(cBluetoothSrv * const me)
{
    uint32_t bt_status;
    if (BluetoothDrv_DelayStartAudioCue(BT_SRV_PER_TICK_TIME_MS))
    {
        bt_status = BT_AUDIO_CUE_START_STA;
        Setting_Set(SETID_BT_STATUS, &bt_status);
        BluetoothSrv_PublishStatus(me, TRUE);
    }
}
#endif

static void BluetoothSrv_PublishStatus(cBluetoothSrv * const me, bool isBtStatus)
{
    BtStatusEvt *pe = Q_NEW(BtStatusEvt, BT_STATE_SIG);
    if(isBtStatus)
    {
        pe->btStatus = me->btStatus;
        pe->btIndEvt = BT_MAX_EVT;
    }
    else
    {
        pe->btIndEvt = me->btIndEvt;
        pe->btStatus = BT_MAX_STA;
    }
    pe->isBtStatus = isBtStatus;
    QF_PUBLISH(&pe->super, me);
}

static void BluetoothSrv_ExecuteCmd(BtCmdEvt* pEvt)
{
    if(pEvt->btCmd <BT_COMMON_MAX_CMD)
    {
        BluetoothDrv_ExecuteCmd(&bluetoothDrv, pEvt->btCmd);
    }
}

static void BluetoothSrv_CheckQueueAndRecallCmdEvt(cBluetoothSrv * const me)
{
    if(me->isQueueWaiting)
    {
        me->cmdQueueDelayMs-= BT_SRV_PER_TICK_TIME_MS;
        if(me->cmdQueueDelayMs<=0)
        {
            me->isQueueWaiting = FALSE;
            me->isCmdExcuting = FALSE;
            if(QActive_recall((QActive*)me, &deferredReqQue))
            {
                TP_PRINTF("recall a request\r\n");
            }
        }
    }
}

static void BluetoothSrv_RefreshTick(cBluetoothSrv * const me)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, BT_SRV_TICK_TIME, 0);
}

