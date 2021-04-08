/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/

#include "ringbuf.h"
#include "aj_target.h"
#include "aj_status.h"
#include "aj_serial.h"
#include "aj_serial_rx.h"
#include "aj_serial_tx.h"

#include "aj_serio.h"
#include "aj_debug.h"

#include "qp_port.h"
#include "UartDrv.h"
#include "AllPlayDrv.h"
#include "signals.h"
#include "object_ids.h"

#define FRAME_DELIMITER (0xC0)

#define B115200 115200
#define BITRATE B115200
#define AJ_SERIAL_WINDOW_SIZE   4
#define AJ_SERIAL_PACKET_SIZE   (512 + AJ_SERIAL_HDR_LEN)

/**
 * Interrupt handler for data arriving on the USART
 */
extern cUartDrv    aj_uart;

/**
 * global function pointer for serial transmit funciton
 */
AJ_SerialTxFunc g_AJ_TX;

static AJ_SerIOTxCompleteFunc   AjDrv_TxCb = NULL;

void AJ_RX(uint8_t* buf, uint32_t len)
{
    AllPlayDrv_SetRx(buf, len);
}

void AJ_PauseRX()
{
    UartDrv_PauseRx(&aj_uart);
}

void AJ_ResumeRX()
{
    UartDrv_ResumeRx(&aj_uart);
}

void AJ_TX(uint8_t* buf, uint32_t len)
{
    UartDrv_Write(&aj_uart, (uint8* )buf, len);

    // invoke the All-Joyne Tx callback to free the memory back to library
    if (AjDrv_TxCb)
    {
        AjDrv_TxCb(buf, len);
    }
}

void AJ_PauseTX()
{
// No need to implement this
//    UartDrv_PauseTx(&aj_uart);
}

void AJ_ResumeTX()
{
// No need to implement this
//    UartDrv_ResumeTx(&aj_uart);
}

AJ_Status AJ_SerialIOEnable(uint32_t direction, uint8_t enable)
{
    return AJ_OK;
}

/**
 * Low level routine to push data out of the USART
 */
AJ_Status AJ_UART_Tx(uint8_t* buffer, uint16_t len)
{
    return AJ_OK;
}

void AJ_SetRxCB(AJ_SerIORxCompleteFunc rx_cb)
{
    AllPlayDrv_SetRxCB(rx_cb);
}

void AJ_SetTxCB(AJ_SerIOTxCompleteFunc tx_cb)
{
    AjDrv_TxCb = tx_cb;
}

void AJ_SetTxSerialTransmit(AJ_SerialTxFunc tx_func)
{
    g_AJ_TX = tx_func;
}

AJ_Status AJ_SerialIOInit(AJ_SerIOConfig* config)
{
    // init is done by UartDrv_Ctor
    return AJ_OK;
}

AJ_Status AJ_SerialIOShutdown(void)
{
    return AJ_OK;
}

AJ_Status AJ_Serial_Up(void)
{
    return AJ_SerialInit("/dev/ttyUSB0", BITRATE, AJ_SERIAL_WINDOW_SIZE, AJ_SERIAL_PACKET_SIZE);
}

AJ_Status AJ_SerialTargetInit(const char* ttyName, uint32_t bitRate)
{
    (void) ttyName;
    (void) bitRate;

    return AJ_OK;
}
