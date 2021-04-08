/**
 *  @file      UartDrv.h
 *  @brief     This file defines a platform independent interface to UART
 *  @author    Donald Leung, Jake Szot, Wesley Lee
 *  @date      22-Aug-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef UARTDRV_H
#define UARTDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "attachedDevices.h"
#include "ringbuf.h"


/*
 * Function pointer type for Tx buffer callback
 * This function is called within the ISR,
 * client need to make sure they are not implementing the complex logic that potentially block anything
 * Suggested to post event to the corresponding owner for complete transmission
 * uartTxCb:       UART Tx Callback function, invoke when there is a byte received from UART Rx
*/
typedef void(*uartTxCb)(void* p);

/*
 * Function pointer type for Rx buffer callback
 * This function is called within the ISR,
 * client need to make sure they are not implementing the complex logic that potentially block anything
 * Suggested to post event to the corresponding owner for parsing
 * uartRxCb:       UART Rx Callback function, invoke when there is a byte received from UART Rx
*/
typedef void(*uartRxCb)(void* p);

/*
 * Function pointer type for wakeup callback
 * This function is called within the ISR,
 * client need to make sure they are not implementing the complex logic that potentially block anything
 * Suggested to post event to the corresponding owner for parsing
 * uartWakeUpCb:       UART wakeup callback function
*/

typedef struct
{
  uint8 uart_port;
  uint8 data;
}tUartRxData;


#ifdef ENABLE_WAKEUP_BY_UART
typedef void(*uartWakeUpCb)(void);
#endif

CLASS(cUartDrv)
    const tUARTDevice*    pConfig;
    cRingBuf*       pTx;
    cRingBuf*       pRx;
    uartTxCb        pTxCallback;
    uartRxCb        pRxCallback;
#ifdef ENABLE_WAKEUP_BY_UART
    uartWakeUpCb    pWakeUpCallback; //pointer to function need to call, when system wake up by uart
#endif
    uint32          cummulativeHwRxBufOverrun;  /*  a cummulative counter that counts how many time
                                                    the hw buffer overrun.
                                                    ie. a measure on how many bytes we lost due because
                                                    our UART driver is not able to serve the IRQ timely
                                                    enough.
                                                */

    uint32           cummulativeFrameError;     /*  The stop bit is not recognized on reception at the expected time, 
                                                    following either a de-synchronization or excessive noise. 
                                                 */

    uint32           cummulativeParityError;    /* Parity Error */

    uint32           cummulativeNoiseDetect;    /* Noise Detect Error */
METHODS


/**
* UART Driver object constructor
* @param[in]    me              UART Driver object
* @param[in]    pConfig         Configuration to the UART instance
* @param[in]    pTx             Ring buffer for transmission buffer
* @param[in]    pRx             Ring buffer for reception buffer
*/
void            UartDrv_Ctor(cUartDrv* me, const tUARTDevice* pConfig, cRingBuf* pTx, cRingBuf* pRx);

/**
* UART Driver object destructor
* @param[in]    me              the UART Driver object
*/
void            UartDrv_Xtor(cUartDrv* me);

/**
* Write the content in me->pTx to the UART device
* @param[in]    me              the UART Driver object
* @param[in]    p               the data to send
* @param[in]    size            the size of data
*/
uint32          UartDrv_Write(cUartDrv* me, const uint8 *p, uint32 size);

void UartDrv_Write_Blocking(eTpUartDevice uart, const uint8 *buffer, uint32 size);

/**
* Register the function for callback after all data are transmitted
* @param[in]    me              the UART Driver object
* @param[in]    uartTxCb        function pointer to the callback
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_RegisterTxCallback(cUartDrv* me, uartTxCb fCb);

/**
* Register the function for callback when receiving a byte from UART Rx
* @param[in]    me              the UART Driver object
* @param[in]    uartRxCb        function pointer to the callback
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_RegisterRxCallback(cUartDrv* me, uartRxCb fCb);

/**
* Register the function for callback when wakeup by UART
* @param[in]    me              the UART Driver object
* @param[in]    uartWakeUpCb    function pointer to the callback
* @return       eTpRet          0: success, otherwise: fail
*/
#ifdef ENABLE_WAKEUP_BY_UART
eTpRet          UartDrv_RegisterWakeUpCallback(cUartDrv* me, uartWakeUpCb fCb);
#endif
/**
* Pause the UART transmission
* @param[in]    me              the UART Driver object
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_PauseTx(cUartDrv* me);

/**
* Pause the UART reception
* @param[in]    me              the UART Driver object
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_PauseRx(cUartDrv* me);

/**
* Resume the UART paused transmission
* @param[in]    me              the UART Driver object
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_ResumeTx(cUartDrv* me);

/**
* Resume the UART paused reception
* @param[in]    me              the UART Driver object
* @return       eTpRet          0: success, otherwise: fail
*/
eTpRet          UartDrv_ResumeRx(cUartDrv* me);

/*Enable USART STOP mode by setting the UESM bit in the CR1 register.
  setup wake up by uart mode */
#ifdef ENABLE_WAKEUP_BY_UART
void            UartDrv_EnableWakeUp(cUartDrv* me);
#endif
/**
* Resume the UART paused reception
* @param[in]    uartId          the uart id
* @return       uint32          TRUE/FALSE
*/
bool UartDrv_isUartReady(eTpUartDevice uartId);

void UartDrv_Flush(eTpUartDevice uartId);

END_CLASS


#ifdef __cplusplus
}
#endif

#endif /* UARTDRV_H */

