/**
 * @file      tp_uart.c
 * @brief     Implement UART operation
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdio.h>
#include <xc.h>
#include <peripheral/pps.h>
#include <peripheral/uart.h>
#include <peripheral/int.h>
#include "Bootloader.h"
#include "HardwareProfile.h"
#include "tp_uart.h"
#include "dbgprint.h"
#include "assert.h"
#include "hwSetup.h" //DISABLE_INTERRUPT
#include "assert.h"
#include "util.h"




/*****************************************************************************
 * Define                                                                    *
 *****************************************************************************/
#define UART_HARDWARE_BUFFER_SIZE 8
#define UART_PORT_NUM 2

#ifdef BL_UART_DEBUG
   #define UART_ENTER_CRITICAL_SECTION(p_rbuf, new_status) \
        assert(p_rbuf->cs_status==UART_SRC_NONE); \
        p_rbuf->cs_status= new_status;
   #define UART_LEAVE_CRITICAL_SECTION(p_rbuf, prev_status) \
        assert(p_rbuf->cs_status==prev_status); \
        p_rbuf->cs_status= UART_SRC_NONE;
#else
   #define UART_ENTER_CRITICAL_SECTION(cs_val, new_status)
   #define UART_LEAVE_CRITICAL_SECTION(cs_val, prev_status)
#endif



/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
/* helper functions */
uint8 UART_get_HandleID(UART_MODULE uart_mod);

/* buffer related private functions */
int32 Uart_buf_init(RingBuf* p_rbuf, uint8* pbuffer, int32 size, const char *name_uart, const char *name_buf, BOOL overflow_allow);
int32 Uart_buf_reset(RingBuf* p_rbuf);
void Uart_buf_check(RingBuf* p_rbuf);
void Uart_handle_check(Uart_handle* phandle);
int32 Uart_buf_getConsumedSize(RingBuf* p_rbuf);
int32 Uart_buf_push(RingBuf* p_rbuf, const uint8* data, uint32 length);
void Uart_buf_pop(RingBuf* p_rbuf, uint8* data, uint32 size);
int32 Uart_buf_dump(RingBuf* p_rbuf, int32 len);



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
/* preallocated UART objects */
static Uart_handle uartPool[UART_PORT_NUM];


/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
int32 Uart_buf_init(RingBuf* p_rbuf, uint8* pbuffer, int32 size, const char *name_uart, const char *name_buf, BOOL overflow_allow)
{
    assert(p_rbuf);
    assert(pbuffer);
    p_rbuf->pdata = pbuffer;
    p_rbuf->size = size;
    p_rbuf->pIn = p_rbuf->pdata;
    p_rbuf->pOut = p_rbuf->pdata;
    p_rbuf->consume_size= 0; //not thread-save
    p_rbuf->overflow_allow= overflow_allow;
#ifdef BL_UART_DEBUG
    p_rbuf->sw_overflow_count= 0;
    p_rbuf->interrupt_count= 0;
    p_rbuf->cs_status= UART_SRC_NONE;
    p_rbuf->pushed_bytes= 0;
    p_rbuf->poped_bytes= 0;
    snprintf(p_rbuf->name, sizeof(p_rbuf->name), "%s_%s", name_uart, name_buf);
#endif
    return 0;
}

int32 Uart_buf_reset(RingBuf* p_rbuf)
{
    Uart_buf_check(p_rbuf);
    p_rbuf->pIn = p_rbuf->pdata;
    p_rbuf->pOut = p_rbuf->pdata;
    p_rbuf->consume_size= 0; //not thread-save
    assert(p_rbuf->consume_size==Uart_buf_getConsumedSize(p_rbuf));
    return 0;
}

inline void Uart_buf_check(RingBuf* p_rbuf)
{
    assert(p_rbuf);
    /* out of bound checking */
    assert(p_rbuf->pdata);
    assert(p_rbuf->pIn < (p_rbuf->pdata + p_rbuf->size));
    assert(p_rbuf->pIn >= (p_rbuf->pdata));
    assert(p_rbuf->pOut < (p_rbuf->pdata + p_rbuf->size));
    assert(p_rbuf->pOut >= (p_rbuf->pdata));

    //buffer always have one empty item, so that consume_size should not be equal to size
    assert(p_rbuf->consume_size < p_rbuf->size); 
}

void Uart_handle_check(Uart_handle* phandle)
{
    assert(phandle);
    assert(phandle->isCreated);
    assert(phandle->uart_id < UART_PORT_NUM);
    assert( &uartPool[phandle->uart_id]==phandle );
    assert( UART_get_HandleID(phandle->uart_mod) == phandle->uart_id );
}

//not thread-save
inline int32 Uart_buf_getRemainingSize(RingBuf* p_rbuf)
{
    Uart_buf_check(p_rbuf);
    return p_rbuf->size - p_rbuf->consume_size - 1;//well, this circular buf. index design has one slot remains when full
}


//Note thread-save, need to disable interrupt before call Uart_buf_getConsumedSize()
inline int32 Uart_buf_getConsumedSize(RingBuf* p_rbuf)
{
    uint32 consumedSize = 0;
    Uart_buf_check(p_rbuf);
    /* checking for enough space in the buffer */

    if(p_rbuf->pIn >= p_rbuf->pOut) //the case where the circular buf. is not wrapped around.
    {
        consumedSize = p_rbuf->pIn - p_rbuf->pOut;
    }
    else
    {
        consumedSize = p_rbuf->size - (p_rbuf->pOut - p_rbuf->pIn);
    }
    assert(consumedSize < p_rbuf->size);
    return consumedSize;
}

 //not thread-save
//clear all buffer when buffer overflow
inline int32 Uart_buf_push(RingBuf* p_rbuf, const uint8* data, uint32 length)
{
    Uart_buf_check(p_rbuf);
    int32 remain_size= Uart_buf_getRemainingSize(p_rbuf);  //not thread-save
    if( remain_size < length ) {
        if(p_rbuf->overflow_allow) {
            Uart_buf_reset(p_rbuf); //clear console tx buffer
#ifdef BL_UART_DEBUG
            p_rbuf->sw_overflow_count++;
            DBG_PRINT("\r\n**** %s buffer overflow ==> reset buffer (size=%dbytes, sw_overflow_count= %d) ****\r\n\r\n", p_rbuf->name, p_rbuf->size, p_rbuf->sw_overflow_count);
#endif
        }
        else {
            assert(0); //buffer overflow
        }
    }

    UART_ENTER_CRITICAL_SECTION(p_rbuf, UART_SRC_PUSH);
    remain_size= Uart_buf_getRemainingSize(p_rbuf);  //not thread-save
    assert(remain_size >= length);  //make sure we have enough space in the buf.
    /* push data */
    {
        uint32 n = 0;
        for(n=0 ; n<length ; n++)
        {
            *(p_rbuf->pIn) = data[n];
            if(p_rbuf->pIn+1 >= p_rbuf->pdata + p_rbuf->size)
            {
                p_rbuf->pIn = p_rbuf->pdata;
            }
            else
            {
                (p_rbuf->pIn)++;
            }
            p_rbuf->consume_size= Uart_buf_getConsumedSize(p_rbuf);
            //p_rbuf->consume_size++;
            //assert(p_rbuf->consume_size==Uart_buf_getConsumedSize(p_rbuf));
        }
    }
#ifdef BL_UART_DEBUG
    p_rbuf->pushed_bytes+= length;
#endif
    UART_LEAVE_CRITICAL_SECTION(p_rbuf, UART_SRC_PUSH);
    Uart_buf_check(p_rbuf);
    return 0;
}



inline void Uart_buf_pop(RingBuf* p_rbuf, uint8* data, uint32 size)
{
    Uart_buf_check(p_rbuf);
    UART_ENTER_CRITICAL_SECTION(p_rbuf, UART_SRC_POP);

    assert(p_rbuf->consume_size >= size);  //make sure we have enough space in the buf.
    /* pop data */
    {
        uint32 n = 0;
        for(n=0 ; n<size && p_rbuf->consume_size>=0 ; n++)
        {
            data[n] = *(p_rbuf->pOut);
#ifdef BL_UART_FILL_ZERO_WHEN_POP
            *(p_rbuf->pOut)= 0;
#endif
            if(p_rbuf->pOut+1 >= p_rbuf->pdata + p_rbuf->size) {
                p_rbuf->pOut = p_rbuf->pdata;
            }
            else {
                (p_rbuf->pOut)++;
            }
            p_rbuf->consume_size= Uart_buf_getConsumedSize(p_rbuf);
            //p_rbuf->consume_size--;
            //assert(p_rbuf->consume_size==Uart_buf_getConsumedSize(p_rbuf));
        }
    }
#ifdef BL_UART_DEBUG
    p_rbuf->poped_bytes+= size;
#endif
    UART_LEAVE_CRITICAL_SECTION(p_rbuf, UART_SRC_POP);
    Uart_buf_check(p_rbuf);
}



int32 Uart_buf_dump(RingBuf* p_rbuf, int32 len)
{
    int32 i=0;
    Uart_buf_check(p_rbuf);
    assert(len > p_rbuf->size);
    UARTTxSendDataBuffer(UART1, "buf dump: ", 10);
    for(i=0 ; i<len ; i++)
    {
        char tmp[5] = {0};
        sprintf(tmp, "0x%2x ", p_rbuf->pdata[i]);
        UARTTxSendDataBuffer(UART1, tmp, 5);
        //delay(1000);
    }
    UARTTxSendDataBuffer(UART1, "\r\n", 1);
    return 0;
}


Uart_handle* UART_Create(UART_MODULE uart_id, UART_MODULE uart_mod, const Uart_config* pConfig, const char *name_uart, BOOL overflow_allow_tx, BOOL overflow_allow_rx)
{
    int32 actual_baud = 0;
    
    /* check user's config */
    assert(name_uart);
    assert(pConfig);
    assert(pConfig->pTx_buf);
    assert(pConfig->txBufSize);
    assert(pConfig->pRx_buf);
    assert(pConfig->rxBufSize);
    assert(pConfig->pTx_buf);
    assert(uart_id<=UART_PORT_NUM);
    assert(uartPool[uart_id].isCreated==FALSE);
    
    uartPool[uart_id].uart_mod= uart_mod;
    uartPool[uart_id].uart_id = uart_id;
    
    /* Microchip pin settings */
    switch(uart_mod){
        case UART1:
            /* this is hardware UART1 (MCU-SAM UART),
             * TX:  pin32, RF5
             * RX:  pin22, RB9
             */
            mPORTBClearBits( BIT_9 );
            mPORTBSetPinsDigitalIn( BIT_9 );                
            mPORTFClearBits( BIT_5 );
            mPORTFSetPinsDigitalOut( BIT_5 );
                
            /* select pin as non-analog */
            ANSELBbits.ANSB9=0;
            //ANSELFbits.ANSF5=0; //no ANSEL reg. for port F.

            PPSUnLock;
            /* for groups see pps.h */
            PPSInput( 1, U1RX, RPB9 );
            PPSOutput( 2, RPF5, U1TX );
            PPSLock;
            break;
        case UART2:
            /* this is hardware UART2 (MCU Debug UART),
             * TX:  pin45, RD11
             * RX:  pin23, RB10
             */
            mPORTBClearBits( BIT_10 );
            mPORTBSetPinsDigitalIn( BIT_10 );            
            mPORTDClearBits( BIT_11 );
            mPORTDSetPinsDigitalOut( BIT_11 );
           
            /* select pin as non-analog */
            ANSELBbits.ANSB10=0;
            //ANSELDbits.ANSD11=0; // no analog function for D11
             PPSUnLock;
            /* for groups see pps.h */
            U2RXR = 6;//PPSInput( 3, U2RX, RPB10 ); TODO, dont know why PPSInput does not quite work
            PPSOutput( 2, RPD11, U2TX );
            PPSLock;
            break;
        default:
            assert(0);
            break;
    }

    /* clone configuration given by user
     * init structure content
     */
    memcpy(&(uartPool[uart_id].config), (uint8*)pConfig, sizeof(pConfig));

#ifdef BL_UART_DEBUG
    uartPool[uart_id].rx_hw_overflow_count= 0;
    strncpy(uartPool[uart_id].name, name_uart, sizeof(uartPool[uart_id].name) );
#endif
    Uart_buf_init(&(uartPool[uart_id].tx), (uint8*)pConfig->pTx_buf, pConfig->txBufSize, name_uart, "tx", overflow_allow_tx);
    Uart_buf_init(&(uartPool[uart_id].rx), (uint8*)pConfig->pRx_buf, pConfig->rxBufSize, name_uart, "rx", overflow_allow_rx);
    uartPool[uart_id].isCreated = 1;

    /* init Microchip hardware */
    UARTConfigure(uart_mod, UART_ENABLE_HIGH_SPEED | UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(uart_mod, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    
    UARTSetLineControl(uart_mod, pConfig->lineCtrlMode);
    actual_baud = UARTSetDataRate(uart_mod, PERIPHERAL_CLOCK, pConfig->baudrate);

    /* check whether if the % error in baudrate would be too large */
    /* note that UART devices will not be able to talk to each other if
     * the clk error is larger than 5%
     * ref: http://www.picaxe.com/docs/baudratetolerance.pdf
     */
    #define UART_MAX_5_PERCENT_ACCEPTABLE_ERROR 5000
    int32 targetBaud = (int32)pConfig->baudrate;           
    int32 err = (1000 * (100 * (actual_baud - targetBaud))) / targetBaud; //err here is in 0.001%
    if(err<0)
        err = -err;
    assert(err < UART_MAX_5_PERCENT_ACCEPTABLE_ERROR );
    UARTEnable(uart_mod, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
    
    if(uart_mod==UART_SAM) { //high priority for SAM
        INTSetVectorSubPriority(INT_VECTOR_UART(uart_mod), INT_SUB_PRIORITY_LEVEL_1);
    }
#ifndef BL_UART_DEBUG_DISABLE_CONSOLE
    else if(uart_mod==UART_CONSOLE) { //low priority for console
        INTSetVectorSubPriority(INT_VECTOR_UART(uart_mod), INT_SUB_PRIORITY_LEVEL_0);
    }
#endif
    INTSetVectorPriority(INT_VECTOR_UART(uart_mod), INT_PRIORITY_LEVEL_3);  //IPL3
    
    INTClearFlag(INT_SOURCE_UART_RX(uart_mod));
    INTClearFlag(INT_SOURCE_UART_TX(uart_mod));

    /* Configure RX Interrupt */  
    INTEnable(INT_SOURCE_UART_RX(uart_mod), INT_ENABLED);
    return &(uartPool[uart_id]);
}


int32 UART_Destroy(Uart_handle* phandle)
{
    Uart_handle_check(phandle);
    uartPool[phandle->uart_id].isCreated = 0;

    switch(phandle->uart_mod)
    {
        case UART1:
            UART1_TX_ENABLE_BIT=0;
            UART1_RX_ENABLE_BIT=0;
            UART1_ENABLE_BIT = 0;
            //TODO use PPS API
            RPB3R = 0x00;   // RPB3 No connected
            U1RXR = 0x08;   // Reserved
            break;
        case UART2:
            UART2_TX_ENABLE_BIT=0;
            UART2_RX_ENABLE_BIT=0;
            UART2_ENABLE_BIT = 0;
            //RPB3R = 0x00;   // RPB3 No connected
            //U1RXR = 0x08;   // Reserved
            break;
        default:
            assert(0);
            break;
    }
    return 0;
}


int32 UART_Write(Uart_handle* phandle, const uint8* buf, uint32 bufSize)
{
    uint32 i;
    Uart_handle_check(phandle);
    assert(buf);

    for(i=0 ; i<bufSize ; i++)
    {
        INTEnable(INT_SOURCE_UART_TX(phandle->uart_mod), INT_DISABLED);
        Uart_buf_push(&(uartPool[phandle->uart_id].tx), buf+i, 1);
        //Uart_buf_push(&(uartPool[phandle->uart_id].tx), (uint8*)buf, bufSize);
        INTEnable(INT_SOURCE_UART_TX(phandle->uart_mod), INT_ENABLED);
    }

#ifdef BL_UART_DBG_DUMP_SAM
    if(phandle->uart_mod==UART_SAM) {
        DBG_PRINT_DATA(buf, bufSize, "write:", "\r\n");
    }
#endif    
    return bufSize;
}


int32 UART_Read(Uart_handle* phandle, uint8* buf, uint32 size)
{
    uint32 i;
    uint32 read_len = 0;
    int32 consumedSize;
    Uart_handle_check(phandle);
    assert(buf);

    read_len= MIN(phandle->rx.consume_size, size);

    for(i=0 ; i<read_len ; i++)
    {
        INTEnable(INT_SOURCE_UART_RX(phandle->uart_mod), INT_DISABLED);
        /* take the less of size and consumedSize */
        Uart_buf_pop(&(phandle->rx), buf+i, 1);
        //Uart_buf_pop(&(phandle->rx), (uint8*)buf, read_len);
        INTEnable(INT_SOURCE_UART_RX(phandle->uart_mod), INT_ENABLED);
    }

#ifdef BL_UART_DBG_DUMP_SAM
    if(phandle->uart_mod==UART_SAM) {
        DBG_PRINT_DATA(buf, read_len, "read:", "\r\n");
    }
#endif
    return read_len;
}


//UART_get_HandleID() must be called after all the uart handlers are initialized
__inline uint8 UART_get_HandleID(UART_MODULE uart_mod)
{   
    uint8 i=0;
    for(i=0 ; i<UART_PORT_NUM ; i++)
    {
        if( uartPool[i].isCreated && uartPool[i].uart_mod==uart_mod )
            break;
    }
    assert(i<UART_PORT_NUM);
    return i;
}


inline void UART_handle_interrupt(UART_MODULE uart_mod)
{
    uint8 uart_id = UART_get_HandleID(uart_mod);
    //printf("uart[%d] irq entry\r\n", uart_id);

    // Is this an RX interrupt?
    if (INTGetFlag(INT_SOURCE_UART_RX(uart_mod)))
    {
#ifdef BL_UART_DEBUG
        uartPool[uart_id].rx.interrupt_count++;
#endif
        
        /* push the data into the Rx buffer */
        INTEnable(INT_SOURCE_UART_RX(uart_mod), INT_DISABLED);

        //for(n=0 ; UARTReceivedDataIsAvailable(uart_mod) && n<UART_HARDWARE_BUFFER_SIZE ; n++)
        while( UARTReceivedDataIsAvailable(uart_mod) )
        {
            uint8 rxData = UARTGetDataByte(uart_mod);
            //tmp[n] = rxData;

            Uart_buf_push(&(uartPool[uart_id].rx), &rxData, 1);
            int32 size = Uart_buf_getConsumedSize(&uartPool[uart_id].rx);      
        }

        INTEnable(INT_SOURCE_UART_RX(uart_mod), INT_ENABLED);
        INTClearFlag(INT_SOURCE_UART_RX(uart_mod));
    }
    else if ( INTGetFlag(INT_SOURCE_UART_TX(uart_mod)) )
    {
#ifdef BL_UART_DEBUG
        uartPool[uart_id].tx.interrupt_count++;
#endif
        //printf("in uart[%d] tx irq\r\n", uart_id);
        /* send if there is something in the send buffer */
        if( uartPool[uart_id].tx.consume_size > 0)
        {
            char tmp = '\0';
            BOOL bTxFull = FALSE;

            do
            {
                /* update the flag bTxFull */
                if(!UARTTransmitterIsReady(uart_mod))
                {
                    bTxFull = TRUE;
                }
                else
                {
                    bTxFull = FALSE;
                    Uart_buf_pop(&(uartPool[uart_id].tx), (uint8*)(&tmp), 1);
                    UARTSendDataByte(uart_mod, tmp);
                }
            }
            while(!bTxFull && uartPool[uart_id].tx.consume_size>0);
        }
        else
        {
            /* disable tx interrupt if the tx buf is now empty */
            INTEnable(INT_SOURCE_UART_TX(uart_mod), INT_DISABLED);
        }
        INTClearFlag(INT_SOURCE_UART_TX(uart_mod));    
    }
    else
    {
        assert(0); //unknown interrupt source
    }

    /* Check whether we have Hardware Rx buffer overrun error */
    //when we got OERR, that means the 8 Bytes hardware Rx has been overunned
    //and we have lost some data. The Rx interrupt would also stop working
    //and we no longer able to recieve further data.
    //So we must clear out the error to keep the UART recieve working
    if(uart_mod == UART1) {
        if(UART1GetErrors() & _U1STA_OERR_MASK)
        {
            UART1ClearError(_U1STA_OERR_MASK);
            //assert(0);
#ifdef BL_UART_DEBUG
            uartPool[uart_id].rx_hw_overflow_count++;
#endif
        }
    }
    else if(uart_mod == UART2) {
        if(UART2GetErrors() & _U2STA_OERR_MASK)
        {
            UART2ClearError(_U2STA_OERR_MASK);
            //assert(0);
#ifdef BL_UART_DEBUG
            uartPool[uart_id].rx_hw_overflow_count++;
#endif
        }
    }
    else {
        assert(0);
    }
    //printf("uart[%d] irq exit\r\n", uart_id);
}


int32 UART_pauseRx(Uart_handle* phandle)
{
    Uart_handle_check(phandle);
    INTEnable(INT_SOURCE_UART_RX(phandle->uart_mod), INT_DISABLED);
    return 0;
}

int32 UART_pauseTx(Uart_handle* phandle)
{
    Uart_handle_check(phandle);
    INTEnable(INT_SOURCE_UART_TX(phandle->uart_mod), INT_DISABLED);
    return 0;
}

int32 UART_resumeRx(Uart_handle* phandle)
{
    Uart_handle_check(phandle);
    INTEnable(INT_SOURCE_UART_RX(phandle->uart_mod), INT_ENABLED);
    return 0;
}

int32 UART_resumeTx(Uart_handle* phandle)
{
    Uart_handle_check(phandle);
    INTEnable(INT_SOURCE_UART_TX(phandle->uart_mod), INT_ENABLED);
    return 0;
}

/**
 * Wait for some input data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if data is available
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 UART_waitInput(Uart_handle* phandle, ssize_t ms)
{
    int32 ret= 0; //default value is timeout
    uint32 target_ms;
    Uart_handle_check(phandle);

    target_ms= TimerUtil_getTimeMs() + ms;
    while( target_ms>=TimerUtil_getTimeMs() || ms<0 ) {
        if( phandle->rx.consume_size > 0 ) {
            ret= 1; //data available
            break;
        }
    }
    return ret;
}


/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if ready
 * @retval 0 if timeout
 * @retval -1 if error
 */
int32 UART_waitOutput(Uart_handle* phandle, ssize_t ms)
{
    int32 ret= 0; //default value is timeout
    uint32 target_ms;
    Uart_handle_check(phandle);

    target_ms= TimerUtil_getTimeMs() + ms;
    while( target_ms>=TimerUtil_getTimeMs() || ms<0 ) {
        if( Uart_buf_getRemainingSize(&(phandle->tx)) > 0 )  {
            ret= 1; //have free buffer
            break;
        }
    }
    return ret;
}


/**
 * Wait for the output to be ready to send more data for up to @c ms milliseconds.
 *
 * @param ms maximum time to wait in milliseconds, -1 for infinity
 * @retval 1 if flush all data
 * @retval 0 if timeout
 */
int32 UART_wait_tx_flush(Uart_handle* phandle, uint32 timeout_ms)
{
    Uart_handle_check(phandle);

    int32 ret= 0; //default value is timeout
    uint32 target_ms= TimerUtil_getTimeMs() + timeout_ms;
    while( target_ms>=TimerUtil_getTimeMs() || timeout_ms<0 )
    {
        if( phandle->tx.consume_size==0 && UARTTransmitterIsReady(phandle->uart_mod) && UARTTransmissionHasCompleted(phandle->uart_mod) )
        {
            ret= 1; //flush all data
            break;
        }
    }
    return ret;
}

