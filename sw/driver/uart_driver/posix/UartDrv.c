/**
 *  @file      UartDrv.c
 *  @brief     
 *  @author    
 *  @date      
 *  @copyright Tymphany Ltd.
 */

#include "./UartDrv_priv.h"
#include "trace.h"
#include "product.config"

#if NUM_OF_UART > 0

#define MIN(a,b) (((a)<(b))?(a):(b))

// Non-Blocking implementation is always highly-recommended
// UART_DRV_BLOCKING should not be defined
//#define UART_DRV_BLOCKING

cUartDrv *uartDrvList[NUM_OF_UART] = {0};     // UART objects list
static tUartDrvStatus uartDrvStatus[NUM_OF_UART];

void UartDrv_Write_Blocking(eTpUartDevice uart, const uint8 *buffer, uint32 size)
{
    (void) uart;
    (void) buffer;
    (void) size;
}

void UartDrv_Ctor(cUartDrv *me, const tUARTDevice *pConfig , cRingBuf *pTx, cRingBuf *pRx)
{
    ASSERT(me);
    ASSERT(pConfig);
    ASSERT(pConfig->deviceInfo.deviceType == UART_DEV_TYPE);

    me->pConfig = pConfig;
    me->pTx     = pTx;
    me->pRx     = pRx;
}

void UartDrv_Xtor(cUartDrv *me)
{
    ASSERT(me);
}

#ifdef ENABLE_WAKEUP_BY_UART
void UartDrv_EnableWakeUp(cUartDrv* me)
{

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

#ifdef UART_DEBUG
    int i = 0;
    printf("UART sending %d data: \r\n", size);
    for ( ; i < size; ++i)
    {
        printf("0x%02X ", *(p+i));
        if ((i & 0xF) == 0xF)
        {
            printf("\r\n");
        }
    }
    printf("\r\n");
#endif
    
    return ret;
}

uint32 UartDrv_Read(cUartDrv* me, uint8 *p, uint32 size)
{
    uint32 ret = TP_SUCCESS;

    ASSERT(me);
    ASSERT(me->pConfig);
    ASSERT(me->pRx);
    ASSERT(p);
    ASSERT(size);

    return ret;
}

uint32 UartDrv_RegisterTxCallback(cUartDrv* me, uartTxCb fCb)
{
    ASSERT(me);
    ASSERT(fCb);

    me->pTxCallback = fCb;
    return TP_SUCCESS;
}

uint32 UartDrv_RegisterRxCallback(cUartDrv* me, uartRxCb fCb)
{
    ASSERT(me);
    ASSERT(fCb);

    me->pRxCallback = fCb;
    return TP_SUCCESS;
}

#ifdef ENABLE_WAKEUP_BY_UART
uint32 UartDrv_RegisterWakeUpCallback(cUartDrv* me, uartWakeUpCb fCb)
{
    ASSERT(me);
    ASSERT(fCb);

    return TP_SUCCESS;
}
#endif

uint32 UartDrv_PauseTx(cUartDrv* me)
{
    ASSERT(me);
    ASSERT(me->pConfig);

    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_PauseRx(cUartDrv* me)
{
    ASSERT(me);
    ASSERT(me->pConfig);

    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_ResumeTx(cUartDrv* me)
{
    ASSERT(me);
    ASSERT(me->pConfig);

    // ToDo
    return TP_SUCCESS;
}

uint32 UartDrv_ResumeRx(cUartDrv* me)
{
    ASSERT(me);
    ASSERT(me->pConfig);

    // ToDo
    return TP_SUCCESS;
}

/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
void USART1_IRQHandler (void)
{

}

/*----------------------------------------------------------------------------
  USART2_IRQHandler
  Handles USART2 global interrupt request.
 *----------------------------------------------------------------------------*/
#if NUM_OF_UART>=2
void USART2_IRQHandler (void)
{

}
#endif  // NUM_OF_UART >= 2

#endif  // NUM_OF_UART > 0
