/**
 *  @file      UartDrv.c
 *  @brief     WIN32 UART
 *  @author    Wesley, Jake Szot
 *  @date      13-Nov-2013
 *  @copyright Tymphany Ltd.
 */
#include <windows.h>
#include <stdio.h>
#include "qp_port.h"
#include "bsp.h"
#include "trace.h"
#include "UartDrv.h"

static HANDLE hUart[UART_DEV_MAX];
cUartDrv uartDrvList[UART_DEV_MAX];

/* Uart */
#define SFINIT(f, ...) __VA_ARGS__
const tUARTDevice uart1Config = 
{
    SFINIT( .deviceInfo.deviceID, DEBUG ),
    SFINIT( .deviceInfo.deviceType, UART_DEV ),
    SFINIT( uartId, 7 ),
    SFINIT( baudrate, CBR_115200 ),
    SFINIT( byteSize, 8 ),
    SFINIT( parity, NOPARITY ),
    SFINIT( stopBits,   ONESTOPBIT ),
    SFINIT( sysClockHz, 0 ),        // not used
    SFINIT( .interrupt.priority,    0 ),    // not used
    SFINIT( .interrupt.subPriority, 0 ),    // not used
};

void UartDrv_Ctor(cUartDrv* me, const tUARTDevice* pConfig, cRingBuf* pTx, cRingBuf* pRx)

{
    uint8 uartId = pConfig->uartId;
    DCB dcb;
    bool bSuccess;
    char buf[14];

    if(pConfig->uartId > 9) //http://support.microsoft.com/default.aspx?scid=kb;EN-US;q115831#appliesto
        wsprintf( buf,"\\\\.\\COM%u", pConfig->uartId);   //"\\\\.\\COM10",     
    else
        wsprintf( buf,"COM%u", pConfig->uartId);

    ASSERT( me );
    ASSERT( pConfig );

    me->pConfig = pConfig;
    hUart[uartId] = CreateFile( buf,
                        GENERIC_READ | GENERIC_WRITE,
                        0,              // must be opened with exclusive-access
                        NULL,           // no security attributes
                        OPEN_EXISTING,  // must use OPEN_EXISTING
                        0,              // not overlapped I/O
                        NULL            // hTemplate must be NULL for comm devices
                        );

    if (hUart[uartId] == INVALID_HANDLE_VALUE) 
    {
        printf ("CreateFile() failed with error %d.\n", GetLastError());
        return;
    }

    bSuccess = GetCommState(hUart[uartId], &dcb);

    if (!bSuccess) 
    {
        printf ("GetCommState() failed with error %d.\n", GetLastError());
        return;
    }

    dcb.BaudRate = pConfig->baudrate;
    dcb.ByteSize = pConfig->byteSize;
    dcb.Parity   = pConfig->parity;
    dcb.StopBits = pConfig->stopBits;

    bSuccess = SetCommState(hUart[uartId], &dcb);

    if (!bSuccess) 
    {
        printf ("SetCommState() failed with error %d.\n", GetLastError());
        return;
    }

    printf ("Serial port %u successfully configured.\n", pConfig->uartId);

}

void UartDrv_Xtor(cUartDrv* me)
{
    uint8 uartId = me->pConfig->uartId;

    ASSERT(me);
    ASSERT(hUart[uartId]);

    CloseHandle(hUart[uartId]);
}

uint32 UartDrv_Write(cUartDrv* me, uint8 *p, uint32 size)
{
    bool bSuccess = FALSE;
    unsigned wrote = 0;
    uint8 uartId = me->pConfig->uartId;

    ASSERT( me );
    ASSERT( hUart[uartId] );

    bSuccess = WriteFile( hUart[uartId], p, size, &wrote, NULL );

    if( !bSuccess )
        return TP_FAIL;

    return TP_SUCCESS;
}

int UART_Read(cUartDrv* me, char* pBuf, uint32 count)
{
    uint8 uartId = me->pConfig->uartId;
    bool bSuccess = FALSE;
    unsigned read = 0;

    ASSERT( me );
    ASSERT( hUart[uartId] );

    bSuccess = ReadFile( hUart[uartId], pBuf, count, &read, NULL );

    if( !bSuccess )
        return -1;

    return read;
}
