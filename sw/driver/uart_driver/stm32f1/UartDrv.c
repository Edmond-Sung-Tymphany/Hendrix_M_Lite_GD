/**
 *  @file      UartDrv.c
 *  @brief     This file implements stm32f0xx UART driver
 *  @author    Wesley Lee
 *  @date      22-May-2014
 *  @copyright Tymphany Ltd.
 */

#include "stm32f10x.h"
#include "./UartDrv_priv.h"
#include "trace.h"
#include "product.config"

#if NUM_OF_UART > 0

#define MIN(a,b) (((a)<(b))?(a):(b))

// Non-Blocking implementation is always highly-recommended
// UART_DRV_BLOCKING should not be defined
//#define UART_DRV_BLOCKING

static USART_TypeDef *gUartMap[] =
{
    USART1,
#if NUM_OF_UART > 1
    USART2,
#endif
};

static const uint8_t gUartNvicIrq[] =
{
    USART1_IRQn,
    USART2_IRQn,
};

static const uint32_t gUartWordLength[] =
{
    USART_WordLength_8b,
    USART_WordLength_9b,
};

static const uint32_t gUartParity[] =
{
    USART_Parity_No,
    USART_Parity_Odd,
    USART_Parity_Even,
};

static const uint32_t gUartStopBits[] =
{
    USART_StopBits_1,
    USART_StopBits_1_5,
    USART_StopBits_2,
};

cUartDrv *uartDrvList[NUM_OF_UART] = {0};     // UART objects list
static tUartDrvStatus uartDrvStatus[NUM_OF_UART];

static USART_TypeDef* UartDrv_getUartBaseAddr(cUartDrv *me)
{
    ASSERT(NUM_OF_UART > me->pConfig->uartId);
    return gUartMap[me->pConfig->uartId];
}

#ifndef UART_DRV_BLOCKING
static void UartDrv_EnableIRQ(cUartDrv *me)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    ASSERT(NUM_OF_UART > me->pConfig->uartId);
    NVIC_InitStructure.NVIC_IRQChannel          = gUartNvicIrq[me->pConfig->uartId];
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority  = me->pConfig->interrupt.priority;
    NVIC_InitStructure.NVIC_IRQChannelCmd       = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(UartDrv_getUartBaseAddr(me), USART_IT_RXNE, ENABLE);
}
#endif  // UART_DRV_BLOCKING

void UartDrv_Write_Blocking(eTpUartDevice uart, const uint8 *buffer, uint32 size)
{
    ASSERT(NUM_OF_UART > uart);
    USART_TypeDef *u = gUartMap[uart];

    while(size)
    {
        while(USART_GetFlagStatus(u, USART_FLAG_TXE) == RESET)
            ;

        USART_SendData(u, *buffer);

        buffer++;
        size--;
    }

    while(USART_GetFlagStatus(u, USART_FLAG_TXE) == RESET)
        ;
}


void UartDrv_Ctor(cUartDrv *me, const tUARTDevice *pConfig , cRingBuf *pTx, cRingBuf *pRx)
{
    ASSERT(me && pConfig && (pConfig->deviceInfo.deviceType == UART_DEV_TYPE));
    /* init UART structure */
    if(uartDrvList[pConfig->uartId] == NULL)
    {
        USART_TypeDef* uartBaseAddr;
        UartDrv_Init(pConfig->uartId);
        uartDrvList[pConfig->uartId] = me;
        me->pConfig     = pConfig;
        me->pTx         = pTx;
        me->pRx         = pRx;
        me->pTxCallback = NULL;
        me->pRxCallback = NULL;

        uartDrvStatus[me->pConfig->uartId].txDataFlag = TX_DATA_EMPTY;
        uartDrvStatus[me->pConfig->uartId].txStatus   = TP_SUCCESS;

        /* init STM32F0 hardware */
        USART_InitTypeDef init;
        ASSERT(TP_UART_BYTE_SIZE_MAX > pConfig->byteSize);
        ASSERT(TP_UART_PARITY_MAX > pConfig->parity);
        ASSERT(TP_UART_STOP_BIT_MAX > pConfig->stopBits);

        init.USART_BaudRate             = pConfig->baudrate;
        init.USART_WordLength           = gUartWordLength[pConfig->byteSize];
        init.USART_Parity               = gUartParity[pConfig->parity];
        init.USART_StopBits             = gUartStopBits[pConfig->stopBits];
        init.USART_HardwareFlowControl  = USART_HardwareFlowControl_None;
        init.USART_Mode                 = USART_Mode_Rx | USART_Mode_Tx;

        uartBaseAddr = UartDrv_getUartBaseAddr(me);
        USART_Init(uartBaseAddr, &init);
        /* Configure the wake up Method = Start bit */
#ifdef PORT_TO_STM32F1
        //USART_StopModeWakeUpSourceConfig(uartBaseAddr, USART_WakeUpSource_RXNE);
#endif
        USART_Cmd(uartBaseAddr,ENABLE);

#ifndef UART_DRV_BLOCKING
        UartDrv_EnableIRQ(me);
#endif  // UART_DRV_BLOCKING
    }
    else
    {
        /* Are we trying to re-int the same uart driver? */
        ASSERT(0);
    }
}

void UartDrv_Xtor(cUartDrv *me)
{
    ASSERT(me &&(me->pConfig));
    USART_TypeDef *uartBaseAddr = UartDrv_getUartBaseAddr(me);

    // STM32F0 register
    USART_Cmd(uartBaseAddr, DISABLE);

    RingBuf_Reset(me->pTx);
    RingBuf_Reset(me->pRx);
    uartDrvList[me->pConfig->uartId] = NULL;
    UartDrv_Deinit(me->pConfig->uartId);
}
#ifdef ENABLE_WAKEUP_BY_UART
void UartDrv_EnableWakeUp(cUartDrv* me)
{
    USART_TypeDef *uartBaseAddr = UartDrv_getUartBaseAddr(me);
    /* Enable USART STOP mode by setting the UESM bit in the CR1 register.*/
    USART_STOPModeCmd(uartBaseAddr, ENABLE);
    /* Enable the wake up from stop Interrupt */
    USART_ITConfig(uartBaseAddr, USART_IT_WU, ENABLE);
    /* need to turn off the wake up interrupt for UART after exting stob mode*/
}
#endif
uint32 UartDrv_Write(cUartDrv *me, const uint8 *p, uint32 size)
{
    uint32 ret = TP_SUCCESS;
    ASSERT(me && (me->pConfig) && (me->pRx) && p && size);
    USART_TypeDef *uartBaseAddr = UartDrv_getUartBaseAddr(me);
#ifdef UART_DRV_BLOCKING
    UartDrv_Write_Blocking(me->pConfig->uartId, p, size);
#else   // !UART_DRV_BLOCKING
    ret = RingBuf_Push(me->pTx, p, size);
    if (uartDrvStatus[me->pConfig->uartId].txDataFlag == TX_DATA_EMPTY)
    {
        uartDrvStatus[me->pConfig->uartId].txDataFlag = TX_DATA_BUSY;
        USART_ITConfig(uartBaseAddr, USART_IT_TXE, ENABLE);
    }
#endif  // UART_DRV_BLOCKING

    return ret;
}

uint32 UartDrv_Read(cUartDrv* me, uint8 *p, uint32 size)
{
    uint32 ret = TP_SUCCESS;
    uint32 s;

    ASSERT(me && (me->pConfig) && (me->pRx) && p && size);
    s = RingBuf_GetUsedSize(me->pRx);
    s = MIN(s, size);
    RingBuf_Pop(me->pRx, p, s);

    return ret;
}

uint32 UartDrv_RegisterTxCallback(cUartDrv* me, uartTxCb fCb)
{
    ASSERT(me && fCb);

    me->pTxCallback = fCb;
    return TP_SUCCESS;
}

uint32 UartDrv_RegisterRxCallback(cUartDrv* me, uartRxCb fCb)
{
    ASSERT(me && fCb);

    me->pRxCallback = fCb;
    return TP_SUCCESS;
}
#ifdef ENABLE_WAKEUP_BY_UART
uint32 UartDrv_RegisterWakeUpCallback(cUartDrv* me, uartWakeUpCb fCb)
{
    ASSERT(me && fCb);

    me->pWakeUpCallback = fCb;
    return TP_SUCCESS;
}
#endif

uint32 UartDrv_PauseTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_PauseRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));

    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_ResumeTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_ResumeRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));

    // ToDo
    return TP_SUCCESS;
}

/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
void USART1_IRQHandler (void)
{
    cUartDrv *uartDrv = uartDrvList[TP_UART_DEV_1];
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        //TODO: need to clear the interrupt flag??
        // read interrupt
        uint8 d;
        //USART1->ICR &= ~USART_FLAG_RXNE;    // clear interrupt

        d = USART_ReceiveData(USART1);;
        if(uartDrv->pRx)
        {
            RingBuf_PushData(uartDrv->pRx, d);
        }

        // Notify the user that a data is arrived by callback
        if(uartDrv->pRxCallback)
        {
            tUartRxData uartData;
            uartData.data = d;
            uartData.uart_port = TP_UART_DEV_1;
            uartDrv->pRxCallback(&uartData); //to let server know from which uart port callback is
        }
    }
#ifdef ENABLE_WAKEUP_BY_UART
    if (USART1->ISR & USART_FLAG_WU)
    {
        /* need to clear the wake up flag from mode STOP mode*/
        USART_ClearFlag(USART1, USART_FLAG_WU);
        uartDrv->pWakeUpCallback();
    }
#endif
#ifndef UART_DRV_BLOCKING
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        //TODO: need to clear the interrupt flag??
        //USART1->ICR &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        if(uartDrvStatus[TP_UART_DEV_1].txDataFlag == TX_DATA_EMPTY)
        {
            // ignored, this is an Rx interrupt
        }
        else if(RingBuf_IsEmpty(uartDrv->pTx))
        {
            // Notify the user that all signals are sent by callback
            if(uartDrv->pTxCallback)
            {
                uartDrvStatus[TP_UART_DEV_1].txStatus = TP_SUCCESS;
                uartDrv->pTxCallback(&uartDrvStatus[TP_UART_DEV_1].txStatus);
            }
            uartDrvStatus[TP_UART_DEV_1].txDataFlag = TX_DATA_EMPTY;
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
        }
        else
        {
            USART_SendData(USART1, RingBuf_PopData(uartDrv->pTx));
        }
    }
#endif  // UART_DRV_BLOCKING
//TODO: how to handle it here
    //if (USART1->ISR & USART_FLAG_ORE)
    {
        //    ASSERT(0);
    }
}

/*----------------------------------------------------------------------------
  USART2_IRQHandler
  Handles USART2 global interrupt request.
 *----------------------------------------------------------------------------*/
#if NUM_OF_UART>=2
void USART2_IRQHandler (void)
{
    cUartDrv *uartDrv = uartDrvList[TP_UART_DEV_2];
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        // read interrupt
        uint8 d;
        d = USART_ReceiveData(USART2);;
        if(uartDrv->pRx)
        {
            RingBuf_PushData(uartDrv->pRx, d);
        }

        // Notify the user that a data is arrived by callback
        if(uartDrv->pRxCallback)
        {
            tUartRxData uartData;
            uartData.data = d;
            uartData.uart_port = TP_UART_DEV_2;
            uartDrv->pRxCallback(&uartData); //to let server know from which uart port callback is
        }
    }
#ifdef ENABLE_WAKEUP_BY_UART
    if (USART2->ISR & USART_FLAG_WU)
    {
        /* need to clear the wake up flag from mode STOP mode*/
        USART_ClearFlag(USART2, USART_FLAG_WU);
        uartDrv->pWakeUpCallback();
    }
#endif
#ifndef UART_DRV_BLOCKING
    if (USART_GetITStatus(USART2, USART_IT_TXE) != RESET)
    {
        if(uartDrvStatus[TP_UART_DEV_2].txDataFlag == TX_DATA_EMPTY)
        {
            // ignored, this is an Rx interrupt
        }
        else if(RingBuf_IsEmpty(uartDrv->pTx))
        {
            // Notify the user that all signals are sent by callback
            if(uartDrv->pTxCallback)
            {
                uartDrvStatus[TP_UART_DEV_2].txStatus = TP_SUCCESS;
                uartDrv->pTxCallback(&uartDrvStatus[TP_UART_DEV_2].txStatus);
            }
            uartDrvStatus[TP_UART_DEV_2].txDataFlag = TX_DATA_EMPTY;
            USART_ITConfig(USART2, USART_IT_TXE, DISABLE);
        }
        else
        {
            USART_SendData(USART2, RingBuf_PopData(uartDrv->pTx));
        }
    }
#endif  // UART_DRV_BLOCKING
    //if (USART2->ISR & USART_FLAG_ORE)
    {
        //     ASSERT(0);
    }
}
#endif  // NUM_OF_UART >= 2

#ifndef Q_SPY
//PUTCHAR_PROTOTYPE
int putchar (int c)
{
    uint8       d        = c & 0xFF;    // from wide character to ASCII
    tUARTDevice *pConfig = (tUARTDevice *)getDevicebyIdAndType(DEBUG_DEV_ID, UART_DEV_TYPE, NULL);

    /* pConfig==NULL means not define DEBUG_DEV_ID/UART_DEV_TYPE device, may be release build
     */
    if(pConfig)
    {
        ASSERT(pConfig->uartId<NUM_OF_UART);
        cUartDrv  *uartDrv = uartDrvList[pConfig->uartId];
        if (uartDrv)
        {
            UartDrv_Write(uartDrv, &d, 1);
        }
    }
    return (d);
}
#endif

#endif  // NUM_OF_UART > 0
