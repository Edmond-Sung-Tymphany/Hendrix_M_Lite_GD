/**
 * @file      uart_basic.h
 * @brief     Implement UART operation
 * @author    Gavin Lee
 * @date      21-Jul-2014
 * @copyright Tymphany Ltd.
 */

/*******************************************************************************
  UART Library Interface

  Summary:
    Basic (ie busy waiting no interupts) UART - 8N1

*******************************************************************************/
//DOM-IGNORE-BEGIN
/*******************************************************************************
FileName:       uart_basic.h
Dependencies:   See includes
Processor:      PIC32MX

Compiler:       Microchip MPLAB XC32 v1.00 or higher
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
*******************************************************************************

Author      Date          Description
-------------------------------------------------------------------------------
RP          12-Nov-2012   Cleaned up various code examples and comments.
*******************************************************************************/
//DOM-IGNORE-END

#ifndef _UART_BASIC_H_
#define _UART_BASIC_H_

#include <peripheral/uart.h>
#include <GenericTypeDefs.h>
#include <xc.h>

/* Call this once
 *
 * Returns the actual baud achieved */
unsigned UARTInit(UART_MODULE uart, unsigned baud);

void UARTTxSendDataBuffer(UART_MODULE uart, const char *buffer, unsigned size);

void UARTTxSendString(UART_MODULE uart, const char *string );

unsigned UARTRxGetDataBuffer(UART_MODULE uart, char *buffer, unsigned max_size);


#endif // _UART__BASIC_H_

