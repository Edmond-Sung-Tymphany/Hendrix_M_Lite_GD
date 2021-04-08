/**
 *  @file      UartDrv.c
 *  @brief     This file implements a platform independent interface to UART
 *  @author    Donald Leung, Wesley Lee
 *  @date      22-Aug-2013
 *  @copyright Tymphany Ltd.
 */

/*****************************************************************
 * Include
 *****************************************************************/
#include "./UartDrv_priv.h"
#include "trace.h"
#include "tp_hwsetup_pic32.h" //GetPeripheralClock()
#include "product.config"
#include "bsp.h"


/*****************************************************************
 * Definition
 *****************************************************************/
#if NUM_OF_UART > 0
/**
 * Non-Blocking implementation is always highly-recommended
 * UART_DRV_BLOCKING should not be defined
 */
//#define UART_DRV_BLOCKING

/* Record boot message on array, then print them after UART is initialized
 * Note it cost 1KBtyes RAM.
 * ex.
 * ==================================================================================
 *   Controller_Idle] (1)Q_ENTRY_SIG <===== boot message
 *   -----[END-OF-BOOT-MSG]-----
 *
 *   Polk Allplay v4.00 TymPlat v0.2 Polk_BT_MP
 *   [DebugSrv_Ready] (1)Q_ENTRY_SIG sig[0]:
 *   > Enter AudioSrv_PreActive
 *   ...
 * ==================================================================================
 */
//#define DEBUBSRV_DEBUG_RECORD_BOOT_MSG



/*****************************************************************
 * Global Variable
 *****************************************************************/
cUartDrv  * uartDrvList[NUM_OF_UART] = {0};     // UART objects list
static tUartDrvStatus uartDrvStatus[NUM_OF_UART];

#ifdef DEBUBSRV_DEBUG_RECORD_BOOT_MSG
  static char bmsg_str[1024]= {0};
  static int32 bmsg_len= 0;
#endif /* DEBUBSRV_DEBUG_RECORD_BOOT_MSG */



/*****************************************************************
 * Function Prototype
 *****************************************************************/
static void UartDrv_SetLineControlMode(const tUARTDevice* pConfig);
static eTpRet UartDrv_SetBaudRate(const tUARTDevice* pConfig);
static INLINE eTpRet UartDrv_BmsgWrite(cUartDrv* me, const uint8 *p, uint32 size);
static INLINE void UartDrv_BmsgFlush(cUartDrv *uartDrv);




/*****************************************************************
 * Function Implemenation
 *****************************************************************/
#ifdef UART_HAS_DMA_SUPPORT
/* For uart dma support */
#include "DmaDrv.h"
#define UART_DMA_TMP_BUFF_SIZE  4*1024
uint8 uartRXIrq[UART_DEV_MAX] = {_UART1_RX_IRQ,_UART2_RX_IRQ};
uint8 uartTXIrq[UART_DEV_MAX] = {_UART1_TX_IRQ,_UART2_TX_IRQ};
uint8 uartRxByte[UART_DEV_MAX];
uint8 uartTxByte[UART_DEV_MAX];
cDmaDrv *pDmaUartTXObj[UART_DEV_MAX];
cDmaDrv *pDmaUartRXObj[UART_DEV_MAX];
static void UartDrv_DmaTxIntCb(eDmaChannel dmaChannel)
{
    static uint8 txTempBuf[UART_DEV_MAX][UART_DMA_TMP_BUFF_SIZE];
    uint16 buffDataSize = 0;
    uint8 i;
    /* find the right uart obj which mapped to this dma channel */
    for(i = 0; i < UART_DEV_MAX; i++)
    {
        if(NULL != pDmaUartTXObj[i])
        {
            if(pDmaUartTXObj[i]->channel == dmaChannel)
            {
                break;
            }
        }
    }
    if(uartDrvList[i])
    {
        buffDataSize = RingBuf_GetUsedSize(uartDrvList[i]->pTx);
    }
    else
    {
        return;
    }
    volatile uint8 *txreg = (uint8 *)(&U1TXREG);
    txreg += UART_TX_REG_OFFSET * i;
    
    if(0 == buffDataSize)
    {
        if(uartDrvList[i]->pTxCallback)
        {
            uartDrvStatus[i].txStatus = TP_SUCCESS;
            uartDrvList[i]->pTxCallback(&uartDrvStatus[i].txStatus);
        }
        uartDrvStatus[i].txDataFlag = TX_DATA_EMPTY;
        pDmaUartTXObj[i]->channelBusy = FALSE;
    }
    else
    {
        ASSERT((buffDataSize < UART_DMA_TMP_BUFF_SIZE));
        pDmaUartTXObj[i]->channelBusy = TRUE;
        if(uartDrvList[i]->pTx)
        {
            RingBuf_Pop(uartDrvList[i]->pTx,txTempBuf[i],buffDataSize);
            DmaDrv_SetTxBuffer(pDmaUartTXObj[i],(void*)txTempBuf[i],\
                               (void*)txreg,buffDataSize,1,1);
            DmaDrv_TriggerTransfer(pDmaUartTXObj[i]);
        }
    }
    
    if(UART1GetErrors() & _U1STA_OERR_MASK)
    {
        UART1ClearError(_U1STA_OERR_MASK);
    }
    if(UART2GetErrors() & _U2STA_OERR_MASK)
    {
        UART2ClearError(_U2STA_OERR_MASK);
    }
}

static void UartDrv_DmaRxIntCb(eDmaChannel dmaChannel)
{
    uint8 i;
    /* find the right uart obj which mapped to this dma channel */
    for(i = 0; i < UART_DEV_MAX; i++)
    {
        if(NULL != pDmaUartRXObj[i])
        {
            if(pDmaUartRXObj[i]->channel == dmaChannel)
            {
                break;
            }
        }
    }
    if(uartDrvList[i])
    {
        uartDrvStatus[i].rxStatus =\
                RingBuf_PushData(uartDrvList[i]->pRx, uartRxByte[i]);
    }
    else
    {
        return;
    }
    // Notify the user that a data is arrived by callback
    if(uartDrvList[i]->pRxCallback)
    {
        uartDrvList[i]->pRxCallback(&uartDrvStatus[i].rxStatus);
    }
    volatile uint8 *rxreg = (uint8 *)(&U1RXREG);
    rxreg += UART_RX_REG_OFFSET * i;
    /* reset dma to wait for the next coming byte */
    DmaDrv_SetTxBuffer(pDmaUartRXObj[i], (void*)rxreg, (void*)(&uartRxByte[i]), 1, 1, 1);
    DmaDrv_TriggerTransfer(pDmaUartRXObj[i]);
    if(UART1GetErrors() & _U1STA_OERR_MASK)
    {
        UART1ClearError(_U1STA_OERR_MASK);
    }
    if(UART2GetErrors() & _U2STA_OERR_MASK)
    {
        UART2ClearError(_U2STA_OERR_MASK);
    }
}
#endif
/* End of For uart dma support */


/**
 * @brief       Flush all pending messages
 * @param[in]   uartId  UART ID
 * @return      TRUE if UartDriver is initilized before, FALSE otherwise
 */
bool UartDrv_isUartReady(eTpUartDevice uartId)
{
    return (uartDrvList[uartId] != NULL);
}

/**
 * @brief        Write the buffer to the UART device in blocking manner
 * @param[in]    uart            the UART Driver ID
 * @param[in]    buffer          the data to send
 * @param[in]    size            the size of data
 */
void UartDrv_Write_Blocking(eTpUartDevice uartId, const uint8 *buffer, uint32 size)
{
    while(size)
    {
        while(!UARTTransmitterIsReady(uartId))
            ;

        UARTSendDataByte(uartId, *buffer);

        buffer++;
        size--;
    }

    while(!UARTTransmissionHasCompleted(uartId))
        ;
}

/**
 * @brief       Flush all pending messages
 * @param[in]  uartId  UART ID
 */
void UartDrv_Flush(eTpUartDevice uartId)
{
    if (uartDrvList[uartId])
    {
        while (!RingBuf_IsEmpty(uartDrvList[uartId]->pTx))
        {
            char tmp = RingBuf_PopData(uartDrvList[uartId]->pTx);
            UartDrv_Write_Blocking(uartId, (const uint8 *)(&tmp), 1);
        }
    }
}

static void UartDrv_SetLineControlMode(const tUARTDevice* pConfig)
{
    UART_LINE_CONTROL_MODE lineCtrlMode = 0;

    switch (pConfig->byteSize)
    {
    case TP_UART_BYTE_SIZE_8_BITS:
        lineCtrlMode |= UART_DATA_SIZE_8_BITS;
        break;
    case TP_UART_BYTE_SIZE_9_BITS:
        lineCtrlMode |= UART_DATA_SIZE_9_BITS;
        break;
    default:
        ASSERT(0);
        break;
    }

    switch (pConfig->parity)
    {
    case TP_UART_PARITY_NONE:
        lineCtrlMode |= UART_PARITY_NONE;
        break;
    case TP_UART_PARITY_ODD:
        lineCtrlMode |= UART_PARITY_ODD;
        break;
    case TP_UART_PARITY_EVEN:
        lineCtrlMode |= UART_PARITY_EVEN;
        break;
    default:
        ASSERT(0);
        break;
    }

    switch (pConfig->stopBits)
    {
    case TP_UART_STOP_BIT_1:
        lineCtrlMode |= UART_STOP_BITS_1;
        break;
    case TP_UART_STOP_BIT_2:
        lineCtrlMode |= UART_STOP_BITS_2;
        break;
    default:
        ASSERT(0);
        break;
    }

    UARTSetLineControl(pConfig->uartId,  lineCtrlMode);
}

static eTpRet UartDrv_SetBaudRate(const tUARTDevice* pConfig)
{
    /* check whether if the % error in baudrate would be too large */
    /* note that UART devices will not be able to talk to each other if
     * the clk error is larger than 5%
     * ref: http://www.picaxe.com/docs/baudratetolerance.pdf
     */
    const int32 UART_MAX_5_PERCENT_ACCEPTABLE_ERROR = 5000;
    int32 actual_baud   = 0;
    int32 targetBaud    = pConfig->baudrate;
    int32 err;

    actual_baud = UARTSetDataRate(pConfig->uartId, GetPeripheralClock(SYS_FREQ), targetBaud);
    // for targetBuad = 500k, the equation could only work properly upto +/-4.294%
    err = (1000 * 100 * (actual_baud - targetBaud)) / targetBaud; //err here is in 0.001%
    if(err < 0)
        err = -err;
   
    if(err > UART_MAX_5_PERCENT_ACCEPTABLE_ERROR)
    {
        ASSERT(err <= UART_MAX_5_PERCENT_ACCEPTABLE_ERROR);
        return TP_FAIL;
    }
    return TP_SUCCESS;
}

void UartDrv_Ctor(cUartDrv* me, const tUARTDevice* pConfig , cRingBuf* pTx, cRingBuf* pRx)
{
    ASSERT(me);
    ASSERT(pConfig);
    ASSERT(pConfig->deviceInfo.deviceType == UART_DEV_TYPE);
    /* init UART structure */
    if(uartDrvList[pConfig->uartId] == NULL)
    {
        UartDrv_Init(pConfig->uartId);
        uartDrvList[pConfig->uartId] = me;
        me->pConfig = pConfig;
        me->pTx         = pTx;
        me->pRx         = pRx;
        me->pTxCallback = NULL;
        me->pRxCallback = NULL;
        me->cummulativeHwRxBufOverrun = 0;

        uartDrvStatus[me->pConfig->uartId].txDataFlag = TX_DATA_EMPTY;
        uartDrvStatus[me->pConfig->uartId].txStatus   = TP_SUCCESS;
        uartDrvStatus[me->pConfig->uartId].rxStatus   = TP_SUCCESS;

        /* init Microchip hardware */
        UARTConfigure(me->pConfig->uartId,
                        UART_ENABLE_HIGH_SPEED          // BRGH
                        | UART_ENABLE_PINS_TX_RX_ONLY   // UEN
                        );

        // Set UART Tx/Rx interrupt
        UARTSetFifoMode(me->pConfig->uartId,
                        UART_INTERRUPT_ON_RX_NOT_EMPTY    // URXISEL<1:0>
        #ifndef UART_DRV_BLOCKING
                        | UART_INTERRUPT_ON_TX_BUFFER_EMPTY   // UTXISEL<1:0>
        #endif
                        );

        #ifndef UART_DRV_BLOCKING
        INTClearFlag(INT_SOURCE_UART_TX(me->pConfig->uartId));
        INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_DISABLED); // IEC1bits.U1TXIE = 0;
        #endif

        if(FALSE == me->pConfig->dmaEnable)
        {
            INTClearFlag(INT_SOURCE_UART_RX(me->pConfig->uartId));
            INTEnable(INT_SOURCE_UART_RX(me->pConfig->uartId), INT_ENABLED);
        }
#ifdef UART_HAS_DMA_SUPPORT
        else
        {
            INTClearFlag(INT_SOURCE_UART_RX(me->pConfig->uartId));
            INTEnable(INT_SOURCE_UART_RX(me->pConfig->uartId), INT_DISABLED);
        }
#endif
        // Set the Line Control mode
        UartDrv_SetLineControlMode(me->pConfig);

        // Set the Baud Rate
        UartDrv_SetBaudRate(me->pConfig);

        // Set Interrupt Vector Priority
        INTSetVectorPriority(INT_VECTOR_UART(me->pConfig->uartId), me->pConfig->interrupt.priority );
        INTSetVectorSubPriority(INT_VECTOR_UART(me->pConfig->uartId), me->pConfig->interrupt.subPriority );

        // Enable the UART
        UARTEnable(me->pConfig->uartId, UART_ENABLE_FLAGS(UART_PERIPHERAL // ON
                                                    | UART_TX       // UTXEN
                                                    | UART_RX));       // URXEN
#ifdef UART_HAS_DMA_SUPPORT
        if(TRUE == me->pConfig->dmaEnable)
        {
            /* init DMA UART TX channel */
            pDmaUartTXObj[me->pConfig->uartId] = DmaDrv_Ctor();
            DmaDrv_InitDmaObj(pDmaUartTXObj[me->pConfig->uartId],uartTXIrq[me->pConfig->uartId]);
            DmaDrv_RegisterIntHandlerCb(pDmaUartTXObj[me->pConfig->uartId],UartDrv_DmaTxIntCb);
            /* init DMA UART RX channel */
            volatile uint8 *rxreg = (uint8 *)(&U1RXREG);
            rxreg += UART_X_REG_OFFSET * (me->pConfig->uartId);
            pDmaUartRXObj[me->pConfig->uartId] = DmaDrv_Ctor();
            DmaDrv_InitDmaObj(pDmaUartRXObj[me->pConfig->uartId],uartRXIrq[me->pConfig->uartId]);
            DmaDrv_RegisterIntHandlerCb(pDmaUartRXObj[me->pConfig->uartId],UartDrv_DmaRxIntCb);
            DmaDrv_SetTxBuffer(pDmaUartRXObj[me->pConfig->uartId], (void*)rxreg, (void*)(&uartRxByte[me->pConfig->uartId]), 1, 1, 1);
            DmaDrv_TriggerTransfer(pDmaUartRXObj[me->pConfig->uartId]);
        }
#endif
    }
    else
    {
        ASSERT(FALSE);
        /* Are we trying to re-int the same uart driver? */
    }
}

void UartDrv_Xtor(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
#ifdef UART_HAS_DMA_SUPPORT
    if(TRUE == me->pConfig->dmaEnable)
    {
        DmaDrv_Xtor(pDmaUartTXObj[me->pConfig->uartId]);
        DmaDrv_Xtor(pDmaUartRXObj[me->pConfig->uartId]);
    }
#endif
    // PIC32 register
    INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_DISABLED);
    INTEnable(INT_SOURCE_UART_RX(me->pConfig->uartId), INT_DISABLED);
    UARTEnable(me->pConfig->uartId, UART_PERIPHERAL | UART_DISABLE);

    RingBuf_Reset(me->pTx);
    RingBuf_Reset(me->pRx);
    uartDrvList[me->pConfig->uartId] = NULL;
    UartDrv_Deinit(me->pConfig->uartId);
}

uint32 UartDrv_Write(cUartDrv* me, const uint8 *p, uint32 size)
{
    eTpRet ret = TP_SUCCESS;
    int i;
    ASSERT(me);
    ASSERT(me->pConfig && p && me->pTx);
    if(size == 0)
    {
        return 0;
    }
#ifdef UART_DRV_BLOCKING
    UartDrv_Write_Blocking(me->pConfig->uartId, p, size);
#else

    for(i=0 ; i<size ; i++)
    {
        /* pushing the data into the queue byte by byte such that we will not stop 
         * the IRQ from working for a prolonged period of time, even we are pushing huge amount
         * of data into the buffer.
         */
        INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_DISABLED); // IEC1bits.U1TXIE = 0;
        ret = RingBuf_Push(me->pTx, p+i, 1);
        if(ret != TP_SUCCESS)
        {
            ret = TP_FAIL;
            break;
        }
        INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_ENABLED); // IEC1bits.U1TXIE = 0;

    }
#endif

    return ret;
}

uint32 UartDrv_Read(cUartDrv* me, uint8 *p, uint32 size)
{
    uint32 s;
    uint32 i;
    UART_MODULE module;
    ASSERT(me && me->pConfig && p && size && me->pRx);

    if(me->pConfig->uartId == TP_UART_DEV_1)
        module = UART1;
    else if(me->pConfig->uartId == TP_UART_DEV_2)
        module = UART2;

    INTEnable(INT_SOURCE_UART_RX(module), INT_DISABLED);
    s = RingBuf_GetUsedSize(me->pRx);
    INTEnable(INT_SOURCE_UART_RX(module), INT_ENABLED);

    s = MIN(s, size);

    for(i=0 ; i<s ; i++)
    {
        /* popping the data from the queue byte by byte such that we will not stop 
         * the IRQ from working for a prolonged period of time, even we are popping huge amount
         * of data from the buffer.
         */
        INTEnable(INT_SOURCE_UART_RX(module), INT_DISABLED);
        RingBuf_Pop(me->pRx, p+i, 1);
        INTEnable(INT_SOURCE_UART_RX(module), INT_ENABLED);
    }
    return s;
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

eTpRet UartDrv_PauseTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
#ifdef UART_HAS_DMA_SUPPORT
    if(me->pConfig->dmaEnable)
    {
        DmaDrv_DisableInt(pDmaUartTXObj[me->pConfig->uartId]);
    }
#endif
    INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_DISABLED);
    return TP_SUCCESS;
}

eTpRet UartDrv_PauseRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
#ifdef UART_HAS_DMA_SUPPORT
    if(me->pConfig->dmaEnable)
    {
        DmaDrv_DisableInt(pDmaUartRXObj[me->pConfig->uartId]);
    }
#endif
    INTEnable(INT_SOURCE_UART_RX(me->pConfig->uartId), INT_DISABLED);
    return TP_SUCCESS;
}

eTpRet UartDrv_ResumeTx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    if(!(me->pConfig->dmaEnable))
    {
        INTEnable(INT_SOURCE_UART_TX(me->pConfig->uartId), INT_ENABLED);
    }
#ifdef UART_HAS_DMA_SUPPORT
    else
    {
        DmaDrv_EnableInt(pDmaUartTXObj[me->pConfig->uartId]);
    }
#endif
    return TP_SUCCESS;
}

eTpRet UartDrv_ResumeRx(cUartDrv* me)
{
    ASSERT(me && (me->pConfig));
    if(!(me->pConfig->dmaEnable))
    {
        INTEnable(INT_SOURCE_UART_RX(me->pConfig->uartId), INT_ENABLED);
    }
#ifdef UART_HAS_DMA_SUPPORT
    else
    {
        DmaDrv_EnableInt(pDmaUartRXObj[me->pConfig->uartId]);
    }
#endif
    return TP_SUCCESS;
}

#define UART_PROFILING_RECORD_SIZE  10
int entry_n = 0;
typedef struct
{
    UART_MODULE uartModule; /* the module that this record is about, either uart #0 or uart #1 */

    int txBytesSent;        /* number of bytes sent in this IRQ */
    int TxCbSpentTime;      /* total time spent in the tx callback */

    int rxBytesReceived;    /* number of bytes received in this IRQ */
    int RxCbSpentTime;      /* total time spent in the rx callback */
    
    int irqSpentTime;       /* total time spent in the IRQ */
    int entryTime;          /* entry time of the IRQ */
    int exitTime;           /* exit time of the IRQ */
}UartProfilingRecord;

UartProfilingRecord timeRecord[UART_PROFILING_RECORD_SIZE] = {{0}};

inline void UartDrv_interruptHandler(UART_MODULE module)
{
    UartProfilingRecord* pRecord = &timeRecord[entry_n];
    pRecord->uartModule = module;
    
    pRecord->txBytesSent = 0;
    pRecord->TxCbSpentTime = 0;

    pRecord->rxBytesReceived = 0;
    pRecord->RxCbSpentTime = 0;

    pRecord->entryTime = ReadCoreTimer();

    INTDisableInterrupts();
#ifndef UART_DRV_BLOCKING
    if (INTGetFlag(INT_SOURCE_UART_TX(module)))
    {
        {
            char tmp = '\0';
            bool bBatch = uartDrvList[module]->pConfig->bBatchProcessInIRQ;
            /* the following loop will consume all the bytes in the hardware buffer */
            while(  UARTTransmitterIsReady(module) &&
                    !RingBuf_IsEmpty(uartDrvList[module]->pTx))
            {
                tmp = RingBuf_PopData(uartDrvList[module]->pTx);
                UARTSendDataByte(module, tmp);
                pRecord->txBytesSent++;
                
                if(!bBatch)
                {
                    break;
                }
            }
            if(RingBuf_IsEmpty(uartDrvList[module]->pTx))
            {
                // Notify the user that all signals are sent by callback
                if(uartDrvList[module]->pTxCallback)
                {
                    int tmpTime = ReadCoreTimer();
                    uartDrvStatus[module].txStatus = TP_SUCCESS;
                    uartDrvList[module]->pTxCallback(&uartDrvStatus[module].txStatus);
                    tmpTime = ReadCoreTimer() - tmpTime;
                    pRecord->TxCbSpentTime += tmpTime;
                }
                INTEnable(INT_SOURCE_UART_TX(module), INT_DISABLED); // IEC1bits.U2TXIE = 0;
            }
        }
        INTClearFlag(INT_SOURCE_UART_TX(module));
    }
#endif

    if (INTGetFlag(INT_SOURCE_UART_RX(module)))
    {
        INTEnable(INT_SOURCE_UART_RX(module), INT_DISABLED);
        if(uartDrvList[module]->pRx)
        {
            bool bBatch = uartDrvList[module]->pConfig->bBatchProcessInIRQ;
            /* the following loop will consume all the bytes in the hardware buffer */
            while( UARTReceivedDataIsAvailable(module) )
            {
                uint8 rxData = UARTGetDataByte(module);

                uartDrvStatus[module].rxStatus =
                    RingBuf_PushData(uartDrvList[module]->pRx, rxData);
                pRecord->rxBytesReceived++;
                // Notify the user that a data is arrived by callback
                if(uartDrvList[module]->pRxCallback)
                {
                    tUartRxData uartData;
                    uartData.data = rxData;
                    uartData.uart_port = module ;

                    /*_______*/
                    int tmpTime = ReadCoreTimer();
                    uartDrvList[module]->pRxCallback(&uartData);
                    tmpTime = ReadCoreTimer() - tmpTime;
                    pRecord->RxCbSpentTime += tmpTime;
                }
                if(!bBatch)
                {
                    break;
                }
            }
        }
        INTEnable(INT_SOURCE_UART_RX(module), INT_ENABLED);
        INTClearFlag(INT_SOURCE_UART_RX(module));
    }

    {
        /* Check whether we have Hardware Rx buffer overrun error */
        //TODO send out hw Rx buf. overrun event
        //when we got OERR, that means the 8 Bytes hardware Rx has been overunned
        //and we have lost some data. The Rx interrupt would also stop working
        //and we no longer able to recieve further data.
        //So we must clear out the error to keep the UART recieve working
        if(module == UART1) {
            if(UART1GetErrors() & _U1STA_OERR_MASK)
            {
                UART1ClearError(_U1STA_OERR_MASK);
                uartDrvList[module]->cummulativeHwRxBufOverrun++;
            }
        }
        else if(module == UART2) {
            if(UART2GetErrors() & _U2STA_OERR_MASK)
            {
                UART2ClearError(_U2STA_OERR_MASK);
                uartDrvList[module]->cummulativeHwRxBufOverrun++;
            }
        }
        else {
            ASSERT(0);
        }
    }
    
    {
        /* time profiling update */
        pRecord->exitTime = ReadCoreTimer();
        pRecord->irqSpentTime  = pRecord->exitTime - pRecord->entryTime;
        entry_n ++;
        if(entry_n >= UART_PROFILING_RECORD_SIZE)
        {
            entry_n = 0;
        }
    }
    INTEnableInterrupts();
}

void __ISR(_UART_1_VECTOR, IPL5) _UART1_Interrupt(void)
{
    UartDrv_interruptHandler(UART1);
}

#ifdef _UART_DEV_2
void __ISR(_UART_2_VECTOR, IPL5) _UART2_Interrupt(void)
{
    UartDrv_interruptHandler(UART2);
}
#endif  // NUM_OF_UART >= 2


#ifndef Q_SPY
// Override the PIC32 weak implementation
// for making use the debug server facility to printf()
//reference: microchip, appio.c
void _mon_write (const char * s, unsigned int count)
{
    /* Do not try to output an NULL pointer */
    if (!s) {
        return;
    }    
    
    tUARTDevice *pConfig = (tUARTDevice*)getDevicebyId(DEBUG_DEV_ID, NULL);
    cUartDrv    *uartDrv = uartDrvList[pConfig->uartId];    

    /* If UART Driver is not ready, use UartDrv_BmsgWrite() to record boot message
     * After UART Driver ready, use UartDrv_Write() to omit message
     */
    if( uartDrv ) {
        UartDrv_BmsgFlush(uartDrv); /* Flush boot message only one time */
        UartDrv_Write(uartDrv, (uint8*)s, count);
    }
    else {
        UartDrv_BmsgWrite(uartDrv, (uint8*)s, count);
    }
}



/* Write boot message to a buffer (At this moment, UART driver was not ready) */
static INLINE uint32 UartDrv_BmsgWrite(cUartDrv* me, const uint8 *p, uint32 size)
{   
    uint32 iBuf= 0;

#ifdef DEBUBSRV_DEBUG_RECORD_BOOT_MSG
    /* Return directly if bmsg_str[] is full */
    while( bmsg_len<sizeof(bmsg_str) && iBuf<size )
    {
        bmsg_str[bmsg_len]= p[iBuf];
        bmsg_len++;
        iBuf++;
    }
#endif /* DEBUBSRV_DEBUG_RECORD_BOOT_MSG */
    return iBuf;
}


/* After UART driver is ready, flush boot message */
static INLINE void UartDrv_BmsgFlush(cUartDrv *uartDrv)
{
#ifdef DEBUBSRV_DEBUG_RECORD_BOOT_MSG
    if( bmsg_len > 0 )
    {
        ASSERT(uartDrv);
        ASSERT(bmsg_len<=sizeof(bmsg_str));
        const *str= "\r\n-----[END-OF-BOOT-MSG]-----\r\n\r\n\r\n";
        UartDrv_BmsgWrite(NULL, (const uint8 *)str, strlen(str));
        UartDrv_Write(uartDrv, (uint8*)bmsg_str, bmsg_len);
        bmsg_len= 0;
    }
#endif /* DEBUBSRV_DEBUG_RECORD_BOOT_MSG */
}

#endif /* #ifndef Q_SPY */

#endif // NUM_OF_UART > 0
