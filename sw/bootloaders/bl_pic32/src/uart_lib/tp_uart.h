/**
 *  @file      tp_uart.h
 *  @brief     This file defines a platform independent interface to UART
 *  @author    Donald Leung
 *  @date      22-Aug-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef __TP_UART_H__
#define __TP_UART_H__

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <sys/types.h> //ssize_t
#include "Bootloader.h"
#include "uart_basic.h"


/*****************************************************************************
 * Define                                                                    *
 *****************************************************************************/
/* Add "size" and "sw_overflow_count" field into uart ring buffer structure
 */
#ifndef NDEBUG //debug build
#define BL_UART_DEBUG
#endif

/* Set unuse buffer position to zero when pop data
 */
#define BL_UART_FILL_ZERO_WHEN_POP

/* Switch UART_CONSOLE and UART_SAM
 * It let MCU communicate with PC sender
 */
//#define BL_UART_DBG_SWITCH_IF

/* Print in/out data for UART_SAM
 * Enable this feature will casue UART_CONSOLE_TX overflow, any may fail to receive firmware
 */
//#define BL_UART_DBG_DUMP_SAM


/*****************************************************************************
 * Type                                                                      *
 *****************************************************************************/
//The index of uartPool[]
#ifndef BL_UART_DBG_SWITCH_IF
    #define UART_SAM      UART1
    #define UART_CONSOLE  UART2
#else
    #define UART_SAM      UART2
    #define UART_CONSOLE  UART1
#endif


typedef enum
{
    TP_UART_DATA_SIZE_8_BITS   = 0x0000,  // Sets the data transfer size to 8-bits per frame.
    TP_UART_DATA_SIZE_9_BITS   = 0x0006,  // Sets the data transfer size to 9-bits per frame.
    TP_UART_PARITY_ODD         = 0x0004,  // Enables parity bit generation for odd parity.
    TP_UART_PARITY_EVEN        = 0x0002,  //Enables parity bit generation for even parity.
    TP_UART_PARITY_NONE        = 0x0000,  // Disables parity bit generation.
    TP_UART_STOP_BITS_2        = 0x0001,  // Enables generation of 2 stop bits per frame.
    TP_UART_STOP_BITS_1        = 0x0000   // Enables generation of 1 stop bit per frame (default).
}UartLineCtrlMode;


typedef  enum{
    UART_EVENT_NEW_DATA_RECIEVED,                       /* event on newly recieved data */
    UART_EVENT_BUFFER_OVERRUN_SYSTEM_BUSY,              /* event on buffer overrun and data is missed due to
                                                           system is too busy to handle the incoming data */
    UART_EVENT_BUFFER_OVERRUN_USER_CONSUME_TOO_SLOW     /* event on buffer overrun and data is missed due to
                                                           the user is not consuming the uart buffer fast enough */
}Uart_evt;

typedef void* EvtHandler(Uart_evt evt, void* userPtr);   /* function pointer typedef for event handling */
typedef void (*TP_UartRxCompleteFunc)(uint8* buf, uint16 len);
typedef void (*TP_UartTxCompleteFunc)(uint8* buf, uint16 len);

typedef struct{
    UartLineCtrlMode  lineCtrlMode;       /* lineCtrl mode setting to the uarthandle */
    BOOL              bisTxBlocking;      /* a flag to set whether the target uart handle should be blocking or not */
    uint32            baudrate;           /* baudrate setting to the uarthandle */
    char*             pTx_buf;            /* Tx buffer given to the handle by the user, for transmit buffering use */
    uint32            txBufSize;          /* size of the tx buffer from user */
    char*             pRx_buf;            /* Rx buffer given to the handle by the user, for recieve buffering use */
    uint32            rxBufSize;          /* size of the rx buffer from user */
}Uart_config;


#ifdef BL_UART_DEBUG
typedef  enum{
    UART_SRC_NONE= 0,
    UART_SRC_PUSH,
    UART_SRC_POP,
    UART_SRC_CHECK_OVERFLOW,
}UART_CS_SOURCE;
#endif

typedef struct{
    uint8*     pdata; //point to original buffer
    uint8*     pIn;   //point to empty position for next pull
    uint8*     pOut;  //point to data position for next pop
    uint32     size;  //size for original buffer
    int32      consume_size;
    BOOL       overflow_allow; //when overflow, 1 means reset buffer, 0 means assert
#ifdef BL_UART_DEBUG
    char       name[32];         // buffer name. ex. UART_CONSOLE_TX
    uint32     sw_overflow_count;   // overflow occurs count (for sw buffer)
    uint32     interrupt_count;  // interrupt trigger count
    uint32     pushed_bytes;     // accumulate pushed bytes
    uint32     poped_bytes;     // accumulate poped bytes
    UART_CS_SOURCE cs_status;
#endif
}RingBuf;


struct _Uart{
    uint8       uart_id;
    UART_MODULE uart_mod;
    Uart_config config;
    RingBuf     tx;
    RingBuf     rx;
#ifdef BL_UART_DEBUG
    char        name[20];
    uint32      rx_hw_overflow_count;   // overflow occurs count (for hw buffer)
#endif
    BOOL        isCreated;
};

typedef struct _Uart Uart_handle;


/**
* This creates an instance of UART according to the config given
* This powers up the UART and makes read and write available
* @param[in]    uart_id              the target UART port that the user wants to create
* @param[in]    pConfig         Configuration to the UART instance
* @param[in]    name            UART name
* @return       Uart_handle*    handle of the created UART instance
*/
Uart_handle*    UART_Create(UART_MODULE uart_id, UART_MODULE uart_mod, const Uart_config* pConfig, const char *name_uart, BOOL overflow_allow_tx, BOOL overflow_allow_rx);

/**
* This destorys an previously created UART instance
* This powers down the UART and makes read and write unavailable
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @return       int32             0: success, otherwise: fail
*/
int32             UART_Destroy(Uart_handle* phandle);

/**
* This writes to an UART instance
* if the Uart_tx_mode is blocking, it will return after the sending is completed
* if the Uart_tx_mode is non-blocking, it will return immediately and after the sending is completed
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @param[in]    buf*           pointer to the user given buffer
* @param[in]    bufSize         size of the buf from the user
* @return       int32             0: success, otherwise: fail
*/
int32             UART_Write(Uart_handle* phandle, const uint8* buf, uint32 bufSize);

/**
* This reads from the buffer of an UART instance, user could specify the max. number of bytes to read in "count" var.
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @param[in]    buf*           pointer to the user given buffer
* @param[in]    count           the number of the bytes that the user asks for
* @return       int32             the number of bytes acutally being read, -1 for failure
*/
int32             UART_Read(Uart_handle* phandle, uint8* buf, uint32 count);

/**
* This pauses the target uart module rx by stopping all the manipulation to the actual hardware
* so it will not feed hardware uart data into the ring buffer and data sent to the target UART Rx
* might be lost.
*
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @return       int32             negativeInt: fail otherwise: success
*/
int32             UART_pauseRx(Uart_handle* phandle);

/**
* This pauses the target uart module tx by stopping all the manipulaiton to the actual hardware.
* So it will stop feeding data into the hardware uart output
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @return       int32             negativeInt: fail otherwise: success
*/
int32             UART_pauseTx(Uart_handle* phandle);

/**
* This resumes the target uart module rx by resuming all the manipulaiton to the actual hardware.
* data reading from the actual hardware is then resumed
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @return       int32             negativeInt: fail otherwise: success
*/
int32             UART_resumeRx(Uart_handle* phandle);

/**
* This resumes the target uart module tx by resuming all the manipulaiton to the actual hardware.
* data feeding to the actual hardware is then resumed
* @param[in]    Uart_handle*    handle of the previously created UART instance
* @return       int32             negativeInt: fail otherwise: success
*/
int32             UART_resumeTx(Uart_handle* phandle);


BOOL Uart_check_tx_overflow(Uart_handle *phandle, uint32 write_len);
void UART_handle_interrupt(UART_MODULE uart_mod);
int32 UART_wait_tx_flush(Uart_handle* phandle, uint32 timeout_ms);
int32 Uart_buf_getRemainingSize(RingBuf* p_rbuf); //not thread-save

/**
 * Wait for some input data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if data is available
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 UART_waitInput(Uart_handle* phandle, ssize_t ms);

/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if ready
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 UART_waitOutput(Uart_handle* phandle, ssize_t ms);


#endif

