#ifndef UARTDRV_PRIV_H
#define UARTDRV_PRIV_H

#include "UartDrv.h"

// a status flag for the Tx data
// UXTXIF could not be relied on
//      as it will be set whenever the Tx is empty, and
//      will be handled if Rx interrupt is being triggered
// Ringbuf_IsEmpty() could not be relied on
//      as it is used to check if the last byte is sent
typedef enum
{
    TX_DATA_EMPTY = 0,  // no data is pending to send in UART Tx
    TX_DATA_BUSY,       // data is transmitting
    TX_DATA_MAX
}eTxDataStatus;

typedef struct tUartDrvStatus
{
    eTxDataStatus   txDataFlag;
    uint8           txStatus;
    uint8           rxStatus;
}tUartDrvStatus;

#define UART_TX_REG_OFFSET    0x200
#define UART_RX_REG_OFFSET    UART_TX_REG_OFFSET

#endif /* UARTDRV_PRIV_H */
