/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Ones Yang
*  @date      12-April-2016
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "stm32f0xx.h"
#include "bsp.h"
#include "uartdrv.h"
#include "NvmDrv.h"
#include "trace.h"
#include "junit_report.h"

/*************************************************************
****************Flash Read/Write Test*************************
*************************************************************/
#define TEST_TIMES  1000

cStorageDrv me;

static bool CircleBuffer_RWTest(uint32 RW_Times)
{
    uint32 WriteValue  = 0;
    uint32 ReadValue   = 0;
    uint32 i = 0;
    bool ret = false;
    StorageDrv_Ctor(&me, (tStorageDevice*)getDevicebyId(INT_FLASH_DEV_ID, NULL));
    for(i = 0;i < RW_Times;i++)
    {
        WriteValue = ReadValue;
        me.SetValueCBuffer(&me, 0, (uint8 *)&WriteValue);
        me.GetValueCBuffer(&me,0,(uint8 *)&ReadValue);
        if(ReadValue == WriteValue)
        {
            ReadValue++;
        }
        else
        {
            ret = false;
            return ret;
        }
    }
    ret = true;
    return ret;
}

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

void uartPrint(char * stream, uint16 size)
{
    UartDrv_Write(&debug_dev, (uint8 const *) stream, size);
}

void makeReport()
{
    char * report;

    report = makeJunitXmlReport("CBuffer Component Test");

    uartPrint(startCommand, strlen(startCommand));
    uartPrint(report, strlen(report));
    uartPrint(stopCommand, strlen(stopCommand));
}

void uartRxCallback (void *p)
{
    if (FALSE == CaVaPcReady)
    {
        CaVaPcReady = TRUE;
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

void testMe(void)
{
    if (CircleBuffer_RWTest(TEST_TIMES))
    {
        testCasePassReport PassTestCaseReport;

        InittestCasePass(&PassTestCaseReport);
        sprintf(PassTestCaseReport.testCaseName,"Write-Read Match Test");
        addPassTestCase(&PassTestCaseReport);

        makeReport();
    }
    else
    {
        testCaseFailReport FailTestCaseReport;

        InittestCaseFail(&FailTestCaseReport);
        sprintf(FailTestCaseReport.testCaseName,"Write-Read Match Test");
        sprintf(FailTestCaseReport.failMessage,"Test Fail.ReadValue don't match the WriteValue");
        addFailTestCase(&FailTestCaseReport);

        makeReport();
    }
 }


int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;
    
    BSP_init();

    InitUart();

    while (!CaVaPcReady);
    
    testMe();
    
    while (1);
    
}
