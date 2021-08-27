/**
 *  @file      UartDrv.c
 *  @brief     This file implements stm32f0xx UART driver
 *  @author    Wesley Lee
 *  @date      22-May-2014
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx_usart.h"
#include "./UartDrv_priv.h"
#include "trace.h"
#include "product.config"


/***********************************************
 * Definition
 ***********************************************/
#if NUM_OF_UART > 0

#define MIN(a,b) (((a)<(b))?(a):(b))

// Non-Blocking implementation is always highly-recommended
// UART_DRV_BLOCKING should not be defined
//#define UART_DRV_BLOCKING

#define UART_INT_MASK (USART_FLAG_RXNE | USART_FLAG_WU | USART_FLAG_TXE | USART_FLAG_ORE | \
                       USART_FLAG_NE   | USART_FLAG_FE | USART_FLAG_PE)
  



/***********************************************
 * Global Variable
 ***********************************************/
#ifndef UART_NON_DIRECT_MAP
static USART_TypeDef *gUartMap[] =
{
    USART1,
#if NUM_OF_UART == 2
    USART2,
#elif NUM_OF_UART > 2
    USART2,
    USART3,
//    #error Too many UART number, please adjust gUartMap[]
#endif
};

static const uint8_t gUartNvicIrq[] =
{
    USART1_IRQn,
    USART2_IRQn,
#ifdef HAS_UART_3
    USART3_8_IRQn,
#endif
};
#else /* #ifndef UART_NON_DIRECT_MAP */
extern USART_TypeDef *gUartMap[NUM_OF_UART];
extern const uint8_t gUartNvicIrq[NUM_OF_UART];
#endif /* #ifndef UART_NON_DIRECT_MAP */

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
#ifndef UART_DRV_BLOCKING
static tUartDrvStatus uartDrvStatus[NUM_OF_UART];
#endif  // UART_DRV_BLOCKING



/***********************************************
 * Function Implemenation
 ***********************************************/
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
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = me->pConfig->interrupt.priority;
    //NVIC_InitStructure.NVIC_IRQChannelSubPriority  = me->pConfig->interrupt.priority;
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
    (void)UART_Handler; // to supress compiler warning
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
        me->cummulativeHwRxBufOverrun = 0;
        me->cummulativeFrameError = 0;
        me->cummulativeParityError = 0;
        me->cummulativeNoiseDetect = 0;

#ifndef UART_DRV_BLOCKING
        uartDrvStatus[me->pConfig->uartId].txDataFlag = TX_DATA_EMPTY;
        uartDrvStatus[me->pConfig->uartId].txStatus   = TP_SUCCESS;
#endif  // UART_DRV_BLOCKING

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
        USART_StopModeWakeUpSourceConfig(uartBaseAddr, USART_WakeUpSource_RXNE);
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

    /* UART deinit should before RingBuf init, otherwise after ring-buf reset, 
     * UART interrupt have low probability to execute and think buffer empty (because Tx=Rx=0),
     * then call ASSERT()
     */
    UartDrv_Deinit(me->pConfig->uartId);
    RingBuf_Reset(me->pTx);
    RingBuf_Reset(me->pRx);
    uartDrvList[me->pConfig->uartId] = NULL;
    me->pConfig = NULL;
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
    ASSERT(me);
    ASSERT(me->pConfig);
    ASSERT(me->pTx);
    ASSERT(p);
    ASSERT(size);
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

eTpRet UartDrv_RegisterTxCallback(cUartDrv* me, uartTxCb fCb)
{
    ASSERT(me && fCb);

    me->pTxCallback = fCb;
    return TP_SUCCESS;
}

eTpRet UartDrv_RegisterRxCallback(cUartDrv* me, uartRxCb fCb)
{
    ASSERT(me && fCb);

    me->pRxCallback = fCb;
    return TP_SUCCESS;
}
#ifdef ENABLE_WAKEUP_BY_UART
eTpRet UartDrv_RegisterWakeUpCallback(cUartDrv* me, uartWakeUpCb fCb)
{
    ASSERT(me && fCb);

    me->pWakeUpCallback = fCb;
    return TP_SUCCESS;
}
#endif

eTpRet UartDrv_PauseTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    // ToDo
    return TP_SUCCESS;
}

eTpRet UartDrv_PauseRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));

    // ToDo
    return TP_SUCCESS;
}

eTpRet UartDrv_ResumeTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    // ToDo
    return TP_SUCCESS;
}

eTpRet UartDrv_ResumeRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));

    // ToDo
    return TP_SUCCESS;
}

inline static void UART_Handler(const eTpUartDevice me)
{
    ASSERT(me<NUM_OF_UART);
    ASSERT(NUM_OF_UART==ArraySize(gUartMap));
    ASSERT(NUM_OF_UART==ArraySize(uartDrvList));
    USART_TypeDef * const uartBaseAddr = gUartMap[me];
    cUartDrv *uartDrv = uartDrvList[me];

    if (uartBaseAddr->ISR & USART_FLAG_RXNE)
    {                                       // read interrupt
        volatile uint8 d;
        uartBaseAddr->ICR &= ~USART_FLAG_RXNE;    // clear interrupt

        d = (uartBaseAddr->RDR & 0x1FF);
        if(uartDrv->pRx)
        {
            RingBuf_PushData(uartDrv->pRx, d);
        }

        // Notify the user that a data is arrived by callback
        if(uartDrv->pRxCallback)
        {
            tUartRxData uartData;
            uartData.data = d;
            uartData.uart_port = me;
            uartDrv->pRxCallback(&uartData); //to let server know from which uart port callback is
        }
    }
#ifdef ENABLE_WAKEUP_BY_UART
    if (uartBaseAddr->ISR & USART_FLAG_WU)
    { /* need to clear the wake up flag from mode STOP mode*/
        USART_ClearFlag(uartBaseAddr, USART_FLAG_WU);
        uartDrv->pWakeUpCallback();
    }
#endif
#ifndef UART_DRV_BLOCKING
    if (uartBaseAddr->ISR & USART_FLAG_TXE)
    {
        tUartDrvStatus * status = &uartDrvStatus[me];
        uartBaseAddr->ICR &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        if(status->txDataFlag == TX_DATA_EMPTY)
        {
            // ignored, this is an Rx interrupt
        }
        else if(RingBuf_IsEmpty(uartDrv->pTx))
        {
            // Notify the user that all signals are sent by callback
            if(uartDrv->pTxCallback)
            {
                status->txStatus = TP_SUCCESS;
                uartDrv->pTxCallback(&status->txStatus);
            }
            status->txDataFlag = TX_DATA_EMPTY;
            USART_ITConfig(uartBaseAddr, USART_IT_TXE, DISABLE);
        }
        else
        {
            uartBaseAddr->TDR = RingBuf_PopData(uartDrv->pTx);
        }
    }
#endif  // UART_DRV_BLOCKING
    if (uartBaseAddr->ISR & USART_FLAG_ORE)
    {
        /* An overrun error occurs when a character is received when RXNE has not been reset. 
         * Data can not be transferred from the shift register to the RDR register until the 
         * RXNE bit is cleared.
         */
        uartBaseAddr->ICR = USART_ICR_ORECF;
#ifndef HAS_IGNORE_UART_OVER_RUN_ERROR
        ASSERT(0);
#endif
    }

    if (uartBaseAddr->ISR & USART_FLAG_NE)
    {
        uartBaseAddr->ICR = USART_ICR_NCF;
        uartDrv->cummulativeNoiseDetect++;
        ASSERT(0);
    }

    if (uartBaseAddr->ISR & USART_FLAG_FE)
    {
        /* Bit 1 FE: Framing error
         * This bit is set by hardware when a de-synchronization, excessive noise or a break character
         * is detected. It is cleared by software, writing 1 to the FECF bit in the USARTx_ICR register.
         * In Smartcard mode, in transmission, this bit is set when the maximum number of transmit
         * attempts is reached without success (the card NACKs the data frame).
         * An interrupt is generated if EIE=1 in the USARTx_CR1 register.
         * 0: No Framing error is detected
         * 1: Framing error or break character is detected
         */
        uartBaseAddr->ICR = USART_ICR_FECF;
        uartDrv->cummulativeFrameError++;
#ifndef HAS_IGNORE_UART_FRAMING_ERROR
        ASSERT(0);
#endif        
    }

    if (uartBaseAddr->ISR & USART_FLAG_PE)
    {
        uartBaseAddr->ICR = USART_ICR_PECF;
        uartDrv->cummulativeParityError++;
        ASSERT(0);
    }
}

/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
void USART1_IRQHandler (void) {
    UART_Handler(TP_UART_DEV_1);
}

/*----------------------------------------------------------------------------
  USART2_IRQHandler
  Handles USART2 global interrupt request.
 *----------------------------------------------------------------------------*/
#if NUM_OF_UART>=2
void USART2_IRQHandler (void) {
    UART_Handler(TP_UART_DEV_2);
}

#ifdef HAS_UART_3
void USART3_8_IRQHandler (void)
{
    cUartDrv *uartDrv = uartDrvList[TP_UART_DEV_3];
    if (USART3->ISR & USART_FLAG_RXNE)
    {                                       // read interrupt
        uint8 d;
        USART3->ICR &= ~USART_FLAG_RXNE;    // clear interrupt

        d = (USART3->RDR & 0x1FF);
        if(uartDrv->pRx)
        {
            RingBuf_PushData(uartDrv->pRx, d);
        }

        // Notify the user that a data is arrived by callback
        if(uartDrv->pRxCallback)
        {
            tUartRxData uartData;
            uartData.data = d;
            uartData.uart_port = TP_UART_DEV_3;
            uartDrv->pRxCallback(&uartData); //to let server know from which uart port callback is
        }
    }
#ifdef ENABLE_WAKEUP_BY_UART
    if (USART3->ISR & USART_FLAG_WU)
    { /* need to clear the wake up flag from mode STOP mode*/
        USART_ClearFlag(USART3, USART_FLAG_WU);
        uartDrv->pWakeUpCallback();
    }
#endif
#ifndef UART_DRV_BLOCKING
    if (USART3->ISR & USART_FLAG_TXE)
    {
        USART3->ICR &= ~USART_FLAG_TXE; // disable TX interrupt if nothing to send
        if(uartDrvStatus[TP_UART_DEV_3].txDataFlag == TX_DATA_EMPTY)
        {
            // ignored, this is an Rx interrupt
        }
        else if(RingBuf_IsEmpty(uartDrv->pTx))
        {
            // Notify the user that all signals are sent by callback
            if(uartDrv->pTxCallback)
            {
                uartDrvStatus[TP_UART_DEV_3].txStatus = TP_SUCCESS;
                uartDrv->pTxCallback(&uartDrvStatus[TP_UART_DEV_3].txStatus);
            }
            uartDrvStatus[TP_UART_DEV_3].txDataFlag = TX_DATA_EMPTY;
            USART_ITConfig(USART3, USART_IT_TXE, DISABLE);
        }
        else
        {
            USART3->TDR = RingBuf_PopData(uartDrv->pTx);
        }
    }
#endif  // UART_DRV_BLOCKING
    if (USART3->ISR & USART_FLAG_ORE)
    {
        /* An overrun error occurs when a character is received when RXNE has not been reset. 
         * Data can not be transferred from the shift register to the RDR register until the 
         * RXNE bit is cleared.
         */
        USART3->ICR = USART_ICR_ORECF;
        uartDrv->cummulativeHwRxBufOverrun++;
        //ASSERT(0);
    }

    if (USART3->ISR & USART_FLAG_NE)
    {
        USART3->ICR = USART_ICR_NCF;
        uartDrv->cummulativeNoiseDetect++;
        //ASSERT(0);
    }

    if (USART3->ISR & USART_FLAG_FE)
    {
        USART3->ICR = USART_ICR_FECF;
        
        /* Bit 1 FE: Framing error
         * This bit is set by hardware when a de-synchronization, excessive noise or a break character
         * is detected. It is cleared by software, writing 1 to the FECF bit in the USARTx_ICR register.
         * In Smartcard mode, in transmission, this bit is set when the maximum number of transmit
         * attempts is reached without success (the card NACKs the data frame).
         * An interrupt is generated if EIE=1 in the USARTx_CR1 register.
         * 0: No Framing error is detected
         * 1: Framing error or break character is detected
         */
        uartDrv->cummulativeFrameError++;
        // ASSERT(0);
    }

    if (USART3->ISR & USART_FLAG_PE)
    {
        USART3->ICR = USART_ICR_PECF;
        uartDrv->cummulativeParityError++;
        ASSERT(0);
    }
}
#endif

#endif  // NUM_OF_UART >= 2


#ifdef UART_NON_DIRECT_MAP
void USART3_8_IRQHandler (void)
{
    if((USART3->CR1 & USART_CR1_UE) && (USART3->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_3);
    }

    if((USART4->CR1 & USART_CR1_UE) && (USART4->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_4);
    }

    if((USART5->CR1 & USART_CR1_UE) && (USART5->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_5);
    }

    if((USART6->CR1 & USART_CR1_UE) && (USART6->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_6);
    }

    if((USART7->CR1 & USART_CR1_UE) && (USART7->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_7);
    }

    if((USART8->CR1 & USART_CR1_UE) && (USART8->ISR & UART_INT_MASK))
    {
        UART_Handler(TP_UART_DEV_8);
    }
}
#endif  // UART_NON_DIRECT_MAP


/* NOTE: remove the following putchar() let printf() output to ST-Link message windows
 * It is useful on the beginning of project when UART is not ready.
 */
#ifndef Q_SPY
//PUTCHAR_PROTOTYPE
int putchar (int c)
{
    uint8 d = c & 0xFF;    // from wide character to ASCII
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
