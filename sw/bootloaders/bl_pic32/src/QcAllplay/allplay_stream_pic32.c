/**
 * @file      allplay_stream_pic32.c
 * @brief     Implement stream part for Qualcomm Allplay Library
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <p32xxxx.h>
#include <stdlib.h>
#include <plib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <time.h>
#include "BootLoader.h"
#include "commonTypes.h"
#include "allplay_receiver.h"
#include "allplay_protocol.h"
#include "HardwareProfile.h"
#include "allplay_stream.h"
#include "tp_uart.h"
#include "assert.h"
#include "util.h"
#include "TimerUtil.h"
#include "dbgprint.h"


/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
/* Simulate the transmisstion error */
//#define FAKE_FAILURE


/*****************************************************************************
 * Macro                                                                     *
 *****************************************************************************/
#define get_rand(min, max) ( rand()%(max-min+1) + min )



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static char txUartBufSam[1024]  = {0};
//static char rxUartBufSam[10240] = {0}; //10KB
static char rxUartBufSam[(MAX_DATA_SIZE+10)*5] = {0};
static Uart_handle* uart_sam = NULL;



/*****************************************************************************
 * Function Implementation                                                   *
 *****************************************************************************/
 
 /**
 * Return current time in milliseconds. #getTime is intended only to compute
 * time period/duration. As such, the origin is undefined and implementation
 * should avoid significant jumps forward/backward in the time value.
 *
 * @return current time in milliseconds
 */
uint32 getTime(void) 
{
    /* Some operation set "time= getTime()-TIMEOUT_ACCK'. It may get a cause underflow.
     * Thus we let getTime() always get a value largere then 2000ms
     */
    return TimerUtil_getTimeMs()+2000;
}

/**
 * Open serial device.
 *
 * @retval 0 on success
 * @retval -1 on failure
 */
int32 openDevice(void) 
{
    srand((int32)__TIME__ * TimerUtil_getTimeMs() * GetSystemTimeUs()); // seed the pseudo random generator
    Uart_config config =
    {
        .baudrate = 115200,
        .lineCtrlMode = TP_UART_DATA_SIZE_8_BITS|TP_UART_PARITY_NONE|TP_UART_STOP_BITS_1,
        .bisTxBlocking = 1,
        .pTx_buf = txUartBufSam,
        .txBufSize = sizeof(txUartBufSam),
        .pRx_buf = rxUartBufSam,
        .rxBufSize = sizeof(rxUartBufSam),
    };
    
    uart_sam = UART_Create(/*uart_id*/0, /*uart_mod*/UART_SAM, &config, "UART_SAM", /*overflow_allow_tx*/FALSE, /*overflow_allow_rx*/FALSE); // uart_id is index of uartPool[]
    assert(uart_sam);
    return 0;
}

/**
 * Close serial device.
 */
void closeDevice(void) 
{
    UART_Destroy(uart_sam);
}


/**
 * Wait for some input data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if data is available
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 waitInput(ssize_t ms) 
{
    return UART_waitInput(uart_sam, ms);
}

/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if ready
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 waitOutput(ssize_t ms)
{
    return UART_waitOutput(uart_sam, ms);
}

/**
 * Read any available data from serial device.
 * #readStream does not block and will return once @c buf is full or all
 * currently available data has been read.
 *
 * @param buf buffer where to put the data
 * @param len size of the buffer
 * @return the amount of data put in the buffer, -1 if error
 */
ssize_t readStream(uint8 *buf, size_t len) 
{
    return UART_Read(uart_sam, buf, len);
}



#if defined(FAKE_FAILURE)
ssize_t fakePutStream(Uart_handle* phandle, uint8 *buf, size_t len)
{
    ssize_t ret;
    
    if (len <= 1) {
        ret= UART_Write(phandle, buf, len);
    }
    else {
        int32 rand_num= rand();
        switch (rand_num % 30) {
            case 0:
            {    // 3.3%: simulate byte error
                int32 off = get_rand(0, len-1);
                uint8 ch = buf[off];
                buf[off] = get_rand(0, 0xFF);
                DBG_PRINT("[%s] simulate error: change byte (off=0x%02X)\r\n", __func__, off);
                ret = UART_Write(phandle, buf, len);
                buf[off] = ch;
                break;
            }
            case 1:
            {    // 3.3%: simulate byte loss
                int32 len2 = get_rand(1, len-1);
                DBG_PRINT("[%s] simulate error: byte loss (len2=0x%02X)\r\n", __func__, len2);
                ret = UART_Write(phandle, buf, len2);
                break;
            }
            case 2:
            {    // 3.3%: simulate packet loss
                DBG_PRINT("[%s] simulate error: packet loss\r\n", __func__);
                ret= len;
                break;
            }
            default:
            {   // 90.1%: no error
                ret = UART_Write(phandle, buf, len);
                break;
            }
        }
    }
    return ret;        
}
#else
#define fakePutStream UART_Write
#endif

/**
 * Write some data to serial device.
 * #writeStream does not block and will return once the serial device's buffer
 * is full or #c buf is empty.
 *
 * @param buf buffer data to write
 * @param len amount of data to write
 * @return the amount of data actually sent, -1 if error
 */
ssize_t writeStream(uint8 *buf, size_t len) 
{    
    return fakePutStream(uart_sam, buf, len);
}

