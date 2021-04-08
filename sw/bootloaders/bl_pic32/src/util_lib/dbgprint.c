/**
 * @file      dbgprint.c
 * @brief     Implement the dbgprint function
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "dbgprint.h"
#include "HardwareProfile.h"
#include "assert.h"
#include "tp_Uart.h"



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
#ifdef BL_MSG_PRINT
static char txUartBufConsole[4096] = {0};
static char rxUartBufConsole[512]  = {0};
static Uart_handle* uart_console = NULL;

void uart_console_init()
{
    Uart_config config =
    {
        .baudrate = 115200,
        .lineCtrlMode = TP_UART_DATA_SIZE_8_BITS|TP_UART_PARITY_NONE|TP_UART_STOP_BITS_1,
        .bisTxBlocking = 1,
        .pTx_buf = txUartBufConsole,
        .txBufSize = sizeof(txUartBufConsole),
        .pRx_buf = rxUartBufConsole,
        .rxBufSize = sizeof(rxUartBufConsole),
    };
    uart_console= UART_Create(/*uart_id*/1, /*uart_mod*/UART_CONSOLE, &config, "UART_CONSOLE", /*overflow_allow_tx*/TRUE, /*overflow_allow_rx*/TRUE); // uart_id is index of uartPool[]
    assert(uart_console);
}


void uart_console_destroy()
{
    assert(uart_console);
    UART_Destroy(uart_console);
}


void dbgprint_data(uint8* buf, uint32 len,  char *header,  char *tail)
{
    int32 i=0;
    int32 write_len= 0;
    static char buf_hex[128];
    assert(buf);
    assert(header);
    assert(tail);

    //prepare header
    uint32 time= TimerUtil_getTimeMs();
    write_len= snprintf(buf_hex, sizeof(buf_hex), "[%3d.%03d] %s (%dbytes)", time/1000, time%1000, header, len);
    UART_Write(uart_console, buf_hex, write_len);

    //len= 0; //disable print data contant
    for(i=0 ; i<len ; i++)
    {
        snprintf(buf_hex, sizeof(buf_hex), "%02x ", buf[i]);
        UART_Write(uart_console, buf_hex, 3);
    }
    UART_Write(uart_console, tail, strlen(tail));
}


//extern Uart_handle* uart_sam;
void dbgprint(const char *fmt, ...)
{
    char buf[256];
    int32 write_len= 0;

    //UART was not initialized
    if(!uart_console || !uart_console->isCreated)
        return;

    assert(fmt);
    memset(buf, 0, sizeof(buf));

    //prepare header
    uint32 time= TimerUtil_getTimeMs();
//#ifdef BL_UART_DEBUG
#if 0
    if(uart_sam && uart_sam->isCreated)
    {
        write_len= snprintf(buf, sizeof(buf), "[%3d.%03d][sam:rx_hw_overflow=%d]",
                                    time/1000, time%1000,
                                    uart_sam->rx_hw_overflow_count);
        /*write_len= snprintf(buf, sizeof(buf), "[%3d.%03d][sam:rx_hw_of=%d,int-count=%d/%d,int-ena:%d/%d,rx-process=%d/%d/%d]",
                                    time/1000, time%1000,
                                    uart_sam->rx_hw_overflow_count,
                                    uart_sam->tx.interrupt_count,
                                    uart_sam->rx.interrupt_count,
                                    INTGetEnable( INT_SOURCE_UART_TX(UART_SAM) )>0,
                                    INTGetEnable( INT_SOURCE_UART_RX(UART_SAM) )>0,
                                    uart_sam->rx.pushed_bytes,
                                    uart_sam->rx.poped_bytes,
                                    uart_sam->rx.pushed_bytes-uart_sam->rx.poped_bytes);*/
    }
    else 
#endif
    {
        write_len= snprintf(buf, sizeof(buf), "[%3d.%03d]", time/1000, time%1000);            
    }
    // format variable arguments
    va_list argus;
    va_start( argus, fmt );

    write_len+= vsnprintf (buf+write_len, sizeof(buf)-write_len, fmt, argus );
    va_end( argus );
    UART_Write(uart_console, buf, write_len);
}


size_t console_read_stream(uint8 *buf, size_t size)
{
    return UART_Read(uart_console, buf, size);
}

size_t console_write_stream(uint8 *buf, size_t len)
{
    return UART_Write(uart_console, buf, len);
}

/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if ready
 * @retval 0 if timeout
 */
int32 uart_console_wait_tx_flush(uint32 timeout_ms)
{
    return UART_wait_tx_flush(uart_console, timeout_ms);
}

#endif /* #ifdef BL_MSG_PRINT */

/*********************End of File************************************/



