/**
 * @file      Uart_usb_kit3.c
 * @brief     Implement UART operation for USB Start Kit III
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <p32xxxx.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <plib.h>
#include "BootLoader.h"
#include "HardwareProfile.h"
#include "Uart.h"



/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
#define DEFAULT_BAUDRATE  115200

//#define Ux(y)         U2##y   //original sample code
#define Ux(y)           U1##y   //UART DEBUG

#define UxBRG           Ux(BRG)
#define UxMODE          Ux(MODE)
#define UxSTA           Ux(STA)
#define UxSTAbits       Ux(STAbits)
#define UxSTACLR        Ux(STACLR)
#define UxSTASET        Ux(STASET)
#define UxTXREG         Ux(TXREG)
#define UxRXREG         Ux(RXREG)



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
//We only config UART1 on USB Starter Kit III
UART_MODULE uart_table[2]= {
    UART1,      /*  UART_SAM */
    UART_NONE,  /*  UART_CONSOLE   <== We did not config UART2 on UST starter kit yet */
};



/*****************************************************************************
 * Function Defination                                                       *
 *****************************************************************************/
static void dbgput(char *msg);



/*****************************************************************************
 * Function Implementation                                                   *
 *****************************************************************************/
/********************************************************************
* Function:     UartInit()
*
* Precondition: 
*
* Input:         PB Clock
*
* Output:        None.
*
* Side Effects:    None.
*
* Overview:     Initializes UART.
*
*
* Note:             None.
********************************************************************/
void UartInit(uint32 pbClk)
{
    mPORTBClearBits( BIT_5 | BIT_3);
    mPORTBSetPinsDigitalIn( BIT_5 );
    mPORTBSetPinsDigitalOut( BIT_3 );

    PPSUnLock;
    /* for groups see pps.h */
    PPSInput(  1, U1RX, RPB5 );
    PPSOutput( 2, RPB3, U1TX );
    PPSLock;

    // Open UART2 with Receive and Transmitter enable.
    UxBRG = (pbClk/16/DEFAULT_BAUDRATE-1); // calculate actual BAUD generate value.
    UxMODE = UART_EN;
    UxSTA = (UART_RX_ENABLE | UART_TX_ENABLE);
}


/********************************************************************
* Function:     UartClose()
*
* Precondition: 
*
* Input:         None
*
* Output:        None.
*
* Side Effects:    None.
*
* Overview:     Closes UART connection.
*
*
* Note:             None.
********************************************************************/
void UartClose(uint8 uart_type)
{
    DBG_PRINT("UartClose");
}


/********************************************************************
* Function:     GetChar()
*
* Precondition:
*
* Input:         None
*
* Output:        True: If there is some data.
*
* Side Effects:    None.
*
* Overview:     Gets the data from UART RX FIFO.
*
* Note:             None.
********************************************************************/
BOOL GetChar(uint8 uart_type, uint8 *byte)
{
    BYTE dummy;
    UART_MODULE uart_id= uart_table[uart_type];

    switch( uart_id )
    {
        case UART1:
        if(U1STA & 0x000E)          // receive errors?
        {
            dummy = U1RXREG;        // dummy read to clear FERR/PERR
            U1STAbits.OERR = 0;     // clear OERR to keep receiving
        }
        if(U1STAbits.URXDA)
        {
            *byte = U1RXREG;        // get data from UART RX FIFO
            //PutChar(uart_type, *byte);
            return TRUE;
        }
        break;

    case UART2:
        if(U2STA & 0x000E)           // receive errors?
        {
            dummy = U2RXREG;         // dummy read to clear FERR/PERR
            U2STAbits.OERR = 0;      // clear OERR to keep receiving
        }
        if(U2STAbits.URXDA)
        {
            *byte = U2RXREG;        // get data from UART RX FIFO
            //PutChar(uart_type, *byte);
            return TRUE;
        }
        break;
    }
    return FALSE;
}

/********************************************************************
* Function:     GetStream()
*
* Precondition:
*
* Input:         None
*
* Output:        True: If there is some data.
*
* Side Effects:    None.
*
* Overview:     Gets the data stream from UART RX FIFO.
*
* Note:             None.
********************************************************************/
ssize_t GetStream(uint8 uart_type, uint8 *buf, ssize_t size)
{
    int32 i=0;
    UART_MODULE uart_id= uart_table[uart_type];
    
    while( GetChar(uart_id, &buf[i]) )
    {
        i++;
        if( i>=size )
            break;
    }
    return i;
}




/********************************************************************
* Function:     PutChar()
*
* Precondition: 
*
* Input:         None
*
* Output:        None
*
* Side Effects:    None.
*
* Overview:     Puts the data into UART tx reg for transmission.
*
*
* Note:             None.
********************************************************************/
void PutChar(uint8 uart_type, uint8 txChar)
{
    UART_MODULE uart_id= uart_table[uart_type];
    switch (uart_id)
    {
        case UART1:
            while(U1STAbits.UTXBF); // wait for TX buffer to be empty
            U1TXREG = txChar;
            break;
        case UART2:
            while(U2STAbits.UTXBF); // wait for TX buffer to be empty
            U2TXREG = txChar;
            break;
    };
}


#ifdef BL_MSG_PRINT
void dbgprint(char *msg)
{   
    static int32 index= 0;
    char buf[11]= {0};
    itoa(buf, index, 10);
    index++;

    dbgput(buf);
    PutChar(UART_CONSOLE, ' ');
    dbgput(msg);
    dbgput("\r\n");
}

static void dbgput(char *msg)
{
    while(*msg!='\0')
    {
        PutChar(UART_CONSOLE, *msg);
        msg++;
    }
}
#else // BL_MSG_PRINT
void dbgprint(char *msg){};
void dbgput(char *msg){};
#endif // BL_MSG_PRINT

/***************************************End Of File*************************************/
