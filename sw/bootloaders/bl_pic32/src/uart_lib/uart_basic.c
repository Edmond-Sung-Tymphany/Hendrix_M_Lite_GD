/**
 * @file      uart_basic.c
 * @brief     Implement UART operation
 * @author    Gavin Lee
 * @date      21-Jul-2014
 * @copyright Tymphany Ltd.
 */

/*******************************************************************************
  UART Library Interface Example

  Summary:
    This file contains the interface definition for the UART peripheral library.
    
  Description:
    This library provides a low-level abstraction of the UART (Universal 
    Asynchronous Receiver/Transmtter) module on Microchip PIC32MX family 
    microcontrollers with a convenient C language interface.  It can be used to
    simplify low-level access to the module without the necessity of interacting
    directly with the module's registers, thus hiding differences from one 
    microcontroller variant to another.
*******************************************************************************/
//DOM-IGNORE-BEGIN
/*******************************************************************************
FileName:       uart_basic.c
Dependencies:   See includes
Processor:      PIC32MX

Compiler:       Microchip MPLAB XC32 v1.06 or higher
Company:        Microchip Technology Inc.

Copyright � 2008-2009 released Microchip Technology Inc.  All rights
reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED �AS IS� WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Includes
// *****************************************************************************
// *****************************************************************************
#include "Bootloader.h"
#include "HardwareProfile.h"
#include "uart_basic.h"
#include "string.h"
// *****************************************************************************
// *****************************************************************************
// Section: Configuration bits
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: System Macros
// *****************************************************************************
// *****************************************************************************

//#define    GetPeripheralClock(SystemClockHz)        (SystemClockHz/(1 << OSCCONbits.PBDIV))

// *****************************************************************************
// *****************************************************************************
// Section: Function Prototypes
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Constant Data
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// Section: Code
// *****************************************************************************
// *****************************************************************************

unsigned UARTInit(UART_MODULE uart, unsigned baud)
{
    unsigned actual_baud = 0;

    UARTConfigure(uart, UART_ENABLE_HIGH_SPEED | UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(uart, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(uart, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    actual_baud = UARTSetDataRate(uart, PERIPHERAL_CLOCK, baud);
    UARTEnable(uart, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    return actual_baud;
}

void UARTTxSendDataBuffer(UART_MODULE uart, const char *buffer, unsigned size)
{
    while(size)
    {
        while(!UARTTransmitterIsReady(uart))
            ;

        UARTSendDataByte(uart, *buffer);

        buffer++;
        size--;
    }

    while(!UARTTransmissionHasCompleted(uart))
        ;
}

void UARTTxSendString(UART_MODULE uart, const char *string)
{
    UARTTxSendDataBuffer( uart, string, strlen(string) );
}

unsigned UARTRxGetDataBuffer(UART_MODULE uart, char *buffer, unsigned max_size)
{
    unsigned num_char;

    num_char = 0;

    while(num_char < max_size)
    {
        uint8 character;

        while(!UARTReceivedDataIsAvailable(uart))
            ;

        character = UARTGetDataByte(uart);

        if(character == '\r')
            break;

        *buffer = character;

        buffer++;
        num_char++;
    }

    return num_char;
}
