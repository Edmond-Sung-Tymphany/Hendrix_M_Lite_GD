/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  Bluetooth Driver
                  -------------------------
                  SW Module Document

@file        BluetoothDrv.c
@brief       It's the bluetooth driver to control the flash version CSR module by uart
@author      Daniel Qin
@date        2017-11-27
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "stm32f0xx.h"
#include "trace.h"
#include "attacheddevices.h"
#include "bsp.h"
#include "seq.h"
#include "SettingSrv.h"
#include "ringbuf.h"
#include "UartDrv.h"

#include "tym.pb.h"
#include "drv_pb.h"
#include "./BluetoothDrv_priv.h"
#include "BluetoothDrv.config"

#define BT_DEBUG_ENABLE
#ifndef BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif

#define PB_MSG_BUFFER_SIZE      (50)
#define BT_UART_TX_BUF_SIZE     (PB_MSG_BUFFER_SIZE)
#define BT_UART_RX_BUF_SIZE     (PB_MSG_BUFFER_SIZE)


static tSeqSection bt_init_seq[] =
{
    { &BtDrv_PwrOnStage1,    50 },
    { &BtDrv_PwrOnStage2,    50 },   // power on process
    { &BtDrv_PwrOnStage3,    50 },
    { &BtDrv_PwrOnStage4,    50 },
};

static tSeqSection bt_deinit_seq[] =
{
    { &BtDrv_PwrOffStage1,    50 },
    { &BtDrv_PwrOffStage2,    50 },   // power off process
    { &BtDrv_PwrOffStage3,    50 },
};


static cBluetoothDrv* pBluetoothDrv;
static cGpioDrv btGpioDrv;

static tGPIODevice *p_GPIOForBT=NULL;


static uint8        uartTxBuf[BT_UART_TX_BUF_SIZE];
static uint8        uartRxBuf[BT_UART_TX_BUF_SIZE];
static cRingBuf     txBuf;
static cRingBuf     rxBuf;
cUartDrv            btDrv_uart;
drv_pb_inst_t       pbDrv;


/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BluetoothDrv_Ctor(cBluetoothDrv *me)
{
    p_GPIOForBT = (tGPIODevice *) getDevicebyIdAndType(BT_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(p_GPIOForBT);

    /* Fill me in! */
    GpioDrv_Ctor(&btGpioDrv, p_GPIOForBT);

    pBluetoothDrv = me;
    drv_pb_init(&pbDrv, BluetoothDrv_uartSend, Proto_Tym_McuBtMessage_fields, Proto_Tym_BtMcuMessage_fields);
}

void BluetoothDrv_Xtor(cBluetoothDrv *me)
{
    // never mind the XTor, it is useless on tymphany platform
    (void)me;
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/

void BluetoothDrv_ExecuteCmd(cBluetoothDrv *me, eBtCmd cmd)
{
    switch(cmd)
    {
        case BT_ENTER_PAIRING_CMD:
        {
            BtDrv_SendReq(Proto_McuBt_ReqResp_BT_PAIRING_ON);
            break;
        }
        case BT_ENTER_CONNECTABLE_CMD:
        {
            //enter connectable
            break;
        }
        case BT_AVRCP_PLAY_PAUSE_CMD:
        {
            BtDrv_SendReq(Proto_McuBt_ReqResp_PLAY_PAUSE_TOGGLE);
            break;
        }
        case BT_AVRCP_NEXT_CMD:
        {
            BtDrv_SendReq(Proto_McuBt_ReqResp_NEXT);
            break;
        }
        case BT_AVRCP_PREV_CMD:
        {
            BtDrv_SendReq(Proto_McuBt_ReqResp_PREV);
            break;
        }
        default:
            break;
    }
}

void BluetoothDrv_RegisterRxMsgCb(cBluetoothDrv* me, rxCb_f cb)
{
    ASSERT(me);
    /* clent can reset the receiver if it needs */
    me->rxCb = cb;
}

void BtDrv_PowerOnSeqInit(cBluetoothDrv* me)
{
    Seq_Ctor(&me->seq, me, bt_init_seq, ArraySize(bt_init_seq));
}

bool BtDrv_isSeqFinished(cBluetoothDrv* me)
{
    return Seq_isSeqFinished(&me->seq);
}

uint32 BtDrv_SeqRefresh(cBluetoothDrv *me, uint32 time)
{
    return Seq_Refresh(&me->seq, time);
}

void BtDrv_PowerOffSeqInit(cBluetoothDrv* me)
{
    Seq_Ctor(&me->seq, me, bt_deinit_seq, ArraySize(bt_deinit_seq));
}

void BtDrv_UpdateStatus(cBluetoothDrv *btDrv)
{
    
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

static void BluetoothDrv_uartSend(uint8 *data, uint16 size)
{
    UartDrv_Write(&btDrv_uart, data, size);
}

static void BluetoothDrv_sendMessage(void *pMessage)
{
    uint16 dataLen = 0;
    uint16 txBuffer[PB_MSG_BUFFER_SIZE];

    dataLen = drv_pb_packMsg(&pbDrv, pMessage, (char *)txBuffer);
    if(dataLen)
    {
        BluetoothDrv_uartSend((uint8 *)txBuffer, dataLen);
    }
}

/* Call back function receiving message from ase module */
/* After decode the commands, this function will send the message to srv_pb_inst->task to handle the command */
static void BluetoothDrv_receivedMsg(void* p)
{
    static bool critical_section= FALSE;
    tUartRxData *uartData = (tUartRxData *)p;
    int         ret = 0;

    if(critical_section)
    {
        TP_PRINTF(("\r\n\r\r\n*** BluetoothDrv_receivedMsg: re-enter error ***\r\n\r\n\r\n\n"));
        ASSERT(0);
        return;
    }
    critical_section = TRUE;

    ret = drv_pb_unpackMsg(&pbDrv, (const uint8 *)&uartData->data, sizeof(uint8));
    if(ret >= 0)
    {
        RingBuf_Reset(&rxBuf);
        /* Do NOT handle msg in here, so just send message to server in callback function. */
        pBluetoothDrv->rxCb((eBtReceivedMsgType)pbDrv.control.frame, (uint8 *)pbDrv.rMessage);
    }
    else if (ret == -EIO)
    {
        RingBuf_Reset(&rxBuf);
    }

    critical_section= FALSE;
}


static void BluetoothDrv_InitUart(void)
{
    //Init driver
    RingBuf_Ctor(&txBuf, uartTxBuf, ArraySize(uartTxBuf));
    RingBuf_Ctor(&rxBuf, uartRxBuf, ArraySize(uartRxBuf));
    UartDrv_Ctor(&btDrv_uart, (tUARTDevice*) getDevicebyIdAndType(BT_DEV_ID, UART_DEV_TYPE, NULL), &txBuf, &rxBuf);
    UartDrv_RegisterRxCallback(&btDrv_uart, BluetoothDrv_receivedMsg);
    
}

static void BluetoothDrv_DeInitUart()
{
    //Deinit driver
    UartDrv_Xtor(&btDrv_uart);
    RingBuf_Xtor(&txBuf);
    RingBuf_Xtor(&rxBuf);
}

static void BluetoothDrv_ResetEnable(bool enable)
{
    if (enable)
    {
        BT_RESET_ENABLE(btGpioDrv);
    }
    else
    {
        BT_RESET_DISABLE(btGpioDrv);
    }
}

static void BluetoothDrv_BT_PWR_PIO_ON(void)
{
    SET_GPIO(&btGpioDrv, BT_POWER_PIN_ID);   // power pin id define at project config
}
static void BluetoothDrv_BT_PWR_PIO_OFF(void)
{
    CLEAR_GPIO(&btGpioDrv, BT_POWER_PIN_ID); // power pin id define at project config
}

static void BtDrv_PwrOnStage1(void* me)
{
    BT_3V3_ENABLE(btGpioDrv);
    BluetoothDrv_BT_PWR_PIO_OFF();
    BluetoothDrv_ResetEnable(TRUE);
}

static void BtDrv_PwrOnStage2(void* me)
{
    BluetoothDrv_ResetEnable(FALSE);
}

static void BtDrv_PwrOnStage3(void* me)
{
    BluetoothDrv_BT_PWR_PIO_ON();
}

static void BtDrv_PwrOnStage4(void* me)
{
    BluetoothDrv_InitUart();
}

static void BtDrv_PwrOffStage1(void* me)
{
    BluetoothDrv_ResetEnable(TRUE);
}

static void BtDrv_PwrOffStage2(void* me)
{
    BluetoothDrv_BT_PWR_PIO_OFF();
}

static void BtDrv_PwrOffStage3(void* me)
{
    BT_3V3_DISABLE(btGpioDrv);
    BluetoothDrv_DeInitUart();
}

 static void BtDrv_SendReq(Proto_McuBt_ReqResp type)
{
    Proto_Tym_McuBtMessage mcuBtMsg;

    mcuBtMsg.which_OneOf = Proto_Tym_McuBtMessage_mcuBtReq_tag;
    mcuBtMsg.OneOf.mcuBtReq.has_type = true;
    mcuBtMsg.OneOf.mcuBtReq.type = type;
    BluetoothDrv_sendMessage(&mcuBtMsg);
}

static void BtDrv_SendBtMcuEvent(Proto_BtMcu_Event_Type type)
{
    Proto_Tym_BtMcuMessage btMcuMsg;

    pbDrv.tx_pb_field = Proto_Tym_BtMcuMessage_fields;
    btMcuMsg.which_OneOf = Proto_Tym_BtMcuMessage_btMcuEvent_tag;
    btMcuMsg.OneOf.btMcuEvent.has_type = true;
    btMcuMsg.OneOf.btMcuEvent.type = type;
    BluetoothDrv_sendMessage(&btMcuMsg);
}


