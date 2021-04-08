/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "stm32f0xx.h"
#include "controller.h"
#include "bsp.h"
#include "uartdrv.h"
#include "attachedDevices.h"
#include "junit_report.h"

#define DMSG_MIN_SIZE   0x07

#define DBG_UART_TX_BUF_SIZE (((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)* DBG_SRV_EVENT_Q_SIZE) + 1)
#define DBG_UART_RX_BUF_SIZE ((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE) + 1)

cUartDrv debug_dev;
cRingBuf txBuf;
cRingBuf rxBuf;

static bool CaVaPcReady = FALSE;

tUARTDevice UartDevice =
{
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,
};

static data uartTxBuf[DBG_UART_TX_BUF_SIZE] = {0};
static data uartRxBuf[DBG_UART_RX_BUF_SIZE] = {0};

char startCommand[] = "COMMJUSTA";
char stopCommand[]  = "COMMJUSTO";
char echoCommand[]  = "COMMECHO_";

static char testEchoSequence[] = "here we test echo functioning";
static uint16 testEchoSequenceIndex = 0;

void uartPrint(char * stream, uint16 size)
{
    UartDrv_Write(&debug_dev, (uint8 const *) stream, size);
}

void testMe()
{
    uartPrint(echoCommand, strlen(echoCommand));
    uartPrint(testEchoSequence, strlen(testEchoSequence));
}

void makeReport()
{
    char * report;

    report = makeJunitXmlReport("Uart component test");

    uartPrint(startCommand, strlen(startCommand));
    uartPrint(report, strlen(report));
    uartPrint(stopCommand, strlen(stopCommand));
}

void uartRxCallback (void *p)
{
    uint8 tt;

    if (FALSE == CaVaPcReady)
    {
        CaVaPcReady = TRUE;
        RingBuf_Reset(&rxBuf);
        testMe();
        return;
    }

    data come = uartRxBuf[RingBuf_GetUsedSize(&rxBuf)-1];

    if (come == testEchoSequence[testEchoSequenceIndex++])
    {
        if (testEchoSequenceIndex == sizeof(testEchoSequence - 1))
        {
            testCasePassReport pp1;

            InittestCasePass(&pp1);
            sprintf(pp1.testCaseName,"Echo test");
            addPassTestCase(&pp1);

            makeReport();
        }
    }
    else
    {
        testCaseFailReport ff1;

        InittestCaseFail(&ff1);
        sprintf(ff1.testCaseName,"Echo test");
        sprintf(ff1.failMessage,"Corrupted echo");
        addFailTestCase(&ff1);

        makeReport();
    }
}
static void InitUart()
{
    InitJunitXmlReport();

    RingBuf_Ctor(&txBuf, uartTxBuf, DBG_UART_TX_BUF_SIZE);
    RingBuf_Ctor(&rxBuf, uartRxBuf, DBG_UART_RX_BUF_SIZE);

    UartDrv_Ctor(&debug_dev, &UartDevice, &txBuf, &rxBuf);

    UartDrv_RegisterRxCallback(&debug_dev, uartRxCallback);
}

static void delay_ms(uint32 delayMs)
{
    uint32 time0 = getSysTime();
    while ((getSysTime() - time0) < delayMs);
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    BSP_init();

    InitUart();

    /* let 'er rip */

    while (1);
    return NULL;
}
