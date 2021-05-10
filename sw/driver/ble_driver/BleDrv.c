/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  Bluetooth Driver
                  -------------------------
                  SW Module Document

@file        BluetoothDrv.c
@brief       It's the bluetooth driver to control the ROM base CSR module by toggling GPIO
@author      Johnny Fan
@date        2014-05-13
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-05-13     Johnny Fan
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "BleDrv_priv.h"
#include "BleDrv.config"

#include "stm32f0xx.h"
#include "trace.h"
#include "bsp.h"
#include "timer.h"
#include "attacheddevices.h"
#include "UartDrv.h"
#include "BleSrv.h"
#include "ringbuf.h"
#include "tp_common.h"
#include "controller.h"
#include "AppPrivSig.Config"

#ifndef BLE_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif

#define MSG_POOL_NUM    3 // small, medium and large

static uint8 rxBuf[BLE_RX_BUFF_SIZE];
static uint8 txBuf[BLE_TX_BUFF_SIZE];
static cBleDrv* pBleDrv;
static cUartDrv uartDrvObj;
static uint16 recvTimerId = 0;

static uint16 msg_size_map[MSG_POOL_NUM] = {SIZE_OF_SMALL_EVENTS, SIZE_OF_MEDIUM_EVENTS, SIZE_OF_LARGE_EVENTS};

/*******************************************************************************

*******************************************************************************/
const static tInitSection InitSection[] =
{
    {BleDrv_InitSection0, 50},
};
    
const static tInitSection DeInitSection[] =
{
    {BleDrv_DeInitSection0, 50},
};


/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BleDrv_Ctor(cBleDrv *me)
{
    tUARTDevice *pUartDevice;

    RingBuf_Ctor(&me->txBufObj, txBuf, BLE_TX_BUFF_SIZE);
    RingBuf_Ctor(&me->rxBufObj, rxBuf, BLE_RX_BUFF_SIZE);

    pUartDevice = (tUARTDevice*)getDevicebyIdAndType(BLE_DEV_ID, UART_DEV_TYPE, NULL);
    ASSERT(pUartDevice);
    UartDrv_Ctor(&uartDrvObj, pUartDevice, &me->txBufObj, &me->rxBufObj);

    UartDrv_RegisterRxCallback(&uartDrvObj, BleDrv_RxCallback);

    BleDrv_ResetRxBuffer(me);
    me->isReady = FALSE;
    me->step = 0;
    pBleDrv = me;
}

void BleDrv_Xtor(cBleDrv *me)
{
    // never mind the XTor, it is useless on tymphany platform
    (void)me;
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/

uint16 BleDrv_Init(cBleDrv *me)
{
    uint16 delayMs = 0;

    if(!me->isReady)
    {
        if(ArraySize(InitSection) == me->step)
        {
            me->step = 0;
            me->isReady = TRUE;
            delayMs = 0;
        }
        else
        {
            InitSection[me->step].initSectionFunc(me);
            delayMs = InitSection[me->step].delaytime;
            me->step++;
        }
    }
    
    return delayMs;
}

uint16 BleDrv_DeInit(cBleDrv *me)
{
    uint16 delayMs = 0;

    if(me->isReady)
    {
        if(ArraySize(DeInitSection) == me->step)
        {
            me->step = 0;
            me->isReady = FALSE;
            delayMs = 0;
        }
        else
        {
            DeInitSection[me->step].initSectionFunc(me);
            delayMs = DeInitSection[me->step].delaytime;
            me->step++;
        }
    }

    return delayMs;
}

/**********************************************************

**********************************************************/
static void BleDrv_InitSection0(void *p)
{
    //cBleDrv *me = (cBleDrv *)p;
}

static void BleDrv_DeInitSection0(void *p)
{
    //cBleDrv *me = (cBleDrv *)p;
}

static void BleDrv_ResetRxBuffer(cBleDrv *me)
{
    me->isHeaderReceived    = FALSE;
    me->recvIndex           = 0;
    me->recvSize            = DMSG_MIN_SIZE;


    Timer_StopTimer(recvTimerId);
    RingBuf_Reset(&me->rxBufObj);
}

/* Note that this callback is called by uart interrupt handler,
*  so we should not put too much thing at here, otherwise the uart interrupt
*  can not receive a full packet, like Timer_StartTimer(...)
*/
static void BleDrv_RxCallback(void *p)
{
    cBleDrv *me = pBleDrv;
    
    if(START_SIGN_IDX == me->recvIndex)
    {
        if(START_SIGN == ((tUartRxData*)p)->data)
        {
            me->recvIndex++;
            me->isHeaderReceived = TRUE;

            /*start a timer */
            BleReqEvt *reqEvt = Q_NEW(BleReqEvt, BLE_REQ_SIG);
            reqEvt->type = BLE_START_TIMER_REQ;
            SendToServer(BLE_CTRL_SRV_ID, (QEvt*)reqEvt);
        }
        else
        {
            /* reset setting */
            BleDrv_ResetRxBuffer(me);
        }
        return;
    }
    else if((SIZEMSB_IDX + 1) == me->recvIndex)
    {
        me->recvSize = (rxBuf[SIZEMSB_IDX] << 8) | (rxBuf[SIZELSB_IDX]);
        if(me->recvSize > BLE_RX_BUFF_SIZE)
        {
            //ASSERT(0);
            BleDrv_ResetRxBuffer(me);
        }
    }

    if((me->recvIndex <= me->recvSize) && me->isHeaderReceived)
    {
        me->recvIndex++;
        if(me->recvIndex == me->recvSize)
        {
            me->isHeaderReceived = FALSE;
            BleReqEvt *reqEvt = Q_NEW(BleReqEvt, BLE_REQ_SIG);
            reqEvt->type = BLE_RECV_FRAME_REQ;
            SendToServer(BLE_CTRL_SRV_ID, (QEvt*)reqEvt);
        }        
    }
    
}

void BleDrv_UpdateStatus(cBleDrv * const me)
{

}

void BleDrv_SendData(uint8* buf, uint16 size)
{
    if(buf && size > 0)
    {
        UartDrv_Write(&uartDrvObj, buf, size);
    }
}

void BleDrv_RequestHandler(cBleDrv * const me, QEvt const * const e)
{
    BleReqEvt *reqEvt = (BleReqEvt *)e;

    //TP_PRINTF("BleDrv_RequestHandler %d\r\n", reqEvt->type);
    switch(reqEvt->type)
    {
        case BLE_START_TIMER_REQ:
        {
            Timer_StartTimer(BLE_RECV_START_TIMEOUT_MS, &recvTimerId, BleDrv_RecvStartTimeoutCallBack, NULL);            
            break;
        }
        case BLE_RECV_TIMEOUT_REQ:
        {
            /*Clean the buffer */
            BleDrv_ResetRxBuffer(me);
            break;
        }
        case BLE_RECV_FRAME_REQ:
        {
            BleDrv_HandleCompleteFrame(me);
            BleDrv_ResetRxBuffer(me);
            break;
        }
        case BLE_CTRL_REQ:
        {
            BleDrv_ExecuteCmd(me, e);
            break;
        }
        default:
        {
            break;
        }

    }
}

static void BleDrv_HandleCompleteFrame(cBleDrv * const me)
{    /* Check CRC */
    if(Tp_CheckCrc(rxBuf, me->recvSize))
    {
        /*Parse frame and diliver msg to servers */
        tpMsgEvt dmsg;
        dmsg.target_srv_id = (ePersistantObjID)rxBuf[SRVID_IDX];
        dmsg.signal = (eSignal)rxBuf[SIG_IDX];
        dmsg.msg_size = me->recvSize;
        eSignalType type = Tp_GetSignalType(dmsg.signal, (tAppSigMapTable*)app_priv_sig_map, sizeof(app_priv_sig_map));
        Tp_DeliverFrameToServer(me->pSender, &dmsg, type, &rxBuf[DATASTART_IDX], me->recvSize - DMSG_MIN_SIZE);
    } 
}

void BleDrv_ExecuteCmd(cBleDrv * const me, QEvt const * const e)
{
    BleReqEvt *reqEvt = (BleReqEvt *)e;
    switch(reqEvt->bleCmd)
    {
        case BLE_FACTORY_RESET_CMD:
        {
            uint8 sendData[16];
            uint8 cmd = BLE_FACTORY_RESET_CMD;
            uint16 size = Tp_PackData(BLE_UART_VERIFY_REQ_SIG, sendData, &cmd, sizeof(cmd));
            BleDrv_SendData(sendData, size);
            break;
        }
        default:
        {
            break;
        }
    }
}

static void BleDrv_RecvStartTimeoutCallBack(void *pCbPara)
{
    BleReqEvt *reqEvt = Q_NEW(BleReqEvt, BLE_REQ_SIG);
    reqEvt->type = BLE_RECV_TIMEOUT_REQ;
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)reqEvt);
}

void BleDrv_HandleUnknown(cBleDrv * const me, QEvt const * const e)
{
    /* handle some unknown message */
    uint16 length;
    eSignalType type;
    uint8 databuf[SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE] = {0};

    type = Tp_GetSignalType(e->sig, (tAppSigMapTable*)app_priv_sig_map, sizeof(app_priv_sig_map));
    length = Tp_HandleResponse(type, databuf, e->sig, msg_size_map[e->poolId_ - 1], (uint8*)e);

    BleDrv_SendData(databuf, length);
}