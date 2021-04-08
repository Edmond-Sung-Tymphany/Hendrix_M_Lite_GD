
#define xTP_UART_CB_DUAL_ECHO_SERVER_TEST


#ifdef TP_UART_CB_DUAL_ECHO_SERVER_TEST

#define TP_UART_NUMBER_OF_UART_PORTS 2  //a define that the user of the UART lib.
                                        //could specify how many UART modules they want to use

#include "UartDrv.h"
#include "ringbuf.h"
#include "tp_hwsetup_pic32.h"

#ifdef TP_UART_TEST_WITH_AJ_SAM_MODULE
#define SET_PIN(pinAssingment) PORTSetBits( pinAssingment.port, pinAssingment.bitflag )
#endif

static data txBuffer0[1024] = {0};
static data rxBuffer0[1024] = {0};
static data txBuffer1[1024] = {0};
static data rxBuffer1[1024] = {0};
    
cUartDrv uart0;
cUartDrv uart1;

static cRingBuf     txRingBuf0;
static cRingBuf     rxRingBuf0;
static cRingBuf     txRingBuf1;
static cRingBuf     rxRingBuf1;

cUartDrv* pUart0 = NULL;
cUartDrv* pUart1 = NULL;

const tUARTDevice uartConfig0 = {
    .deviceInfo.deviceID    = 0,
    .deviceInfo.deviceType  = UART_TYPE,
    .uartId     = TP_UART_DEV_1,
    .baudrate   = 115200,
    .byteSize   = UART_DATA_SIZE_8_BITS,
    .parity     = UART_PARITY_NONE,
    .stopBits   = UART_STOP_BITS_1,
    //.sysClockHz = SYS_FREQ,
    .dmaEnable  = FALSE,
    .interrupt.priority    = INT_PRIORITY_LEVEL_5,
    .interrupt.subPriority = INT_SUB_PRIORITY_LEVEL_1,
};

const tUARTDevice uartConfig1 = {
    .deviceInfo.deviceID    = 1,
    .deviceInfo.deviceType  = UART_TYPE,
    .uartId     = TP_UART_DEV_2,
    .baudrate   = 115200,
    .byteSize   = UART_DATA_SIZE_8_BITS,
    .parity     = UART_PARITY_NONE,
    .stopBits   = UART_STOP_BITS_1,
    //.sysClockHz = SYS_FREQ,
    .dmaEnable  = FALSE,
    .interrupt.priority    = INT_PRIORITY_LEVEL_5,
    .interrupt.subPriority = INT_SUB_PRIORITY_LEVEL_0,
};

void uart1_callback(void* p)
{
    (void*)p;
    {
        uint32 bytesRead;
        char rxBuf;
        //bytesRead = RingBuf_GetUsedSize(pUart1->pRx);
        {
            //uint8* rxBuf = malloc(bytesRead);
            RingBuf_Pop(pUart1->pRx, &rxBuf, 1);
            UartDrv_Write(pUart1, &rxBuf, 1);
            //free(rxBuf);
        }
    }

}

void uart0_callback(void* p)
{
    (void*)p;
    {
        uint32 bytesRead;
        char rxBuf;
        //bytesRead = RingBuf_GetUsedSize(pUart0->pRx);
        {
            //uint8* rxBuf = malloc(bytesRead);
            RingBuf_Pop(pUart0->pRx, &rxBuf, 1);
            UartDrv_Write(pUart0, &rxBuf, 1);
            //free(rxBuf);
        }
    }

}


int UART_unitTest_echo(void)
{
    unsigned char helloText[] = "Hello World, I am UART #";

    pUart0 = &uart0;
    pUart1 = &uart1;
    {
        /* create the uart 0 */
        RingBuf_Ctor(&txRingBuf0, txBuffer0, ArraySize(txBuffer0));
        RingBuf_Ctor(&rxRingBuf0, rxBuffer0, ArraySize(rxBuffer0));
        UartDrv_Ctor(pUart0, &uartConfig0, &txRingBuf0, &rxRingBuf0);
    }

    {
        /* create the uart 1 */
        RingBuf_Ctor(&txRingBuf1, txBuffer1, ArraySize(txBuffer1));
        RingBuf_Ctor(&rxRingBuf1, rxBuffer1, ArraySize(rxBuffer1));
        UartDrv_Ctor(pUart1, &uartConfig1, &txRingBuf1, &rxRingBuf1);
    }

#ifdef TP_UART_TEST_WITH_AJ_SAM_MODULE
    SET_PIN(RESET_PIN);
#endif

    UartDrv_Write(pUart0, helloText, sizeof(helloText));
    UartDrv_Write(pUart0, "0\n\r", sizeof("0\n\r"));
    UartDrv_Write(pUart1, helloText, sizeof(helloText));
    UartDrv_Write(pUart1, "1\n\r", sizeof("1\n\r"));
    UartDrv_RegisterRxCallback(pUart0, uart0_callback);
    UartDrv_RegisterRxCallback(pUart1, uart1_callback);

    while(1)
    {
        //nth to do, the echo is done by the callback
        ;
    }
    return 0;
}

int main(void)
{
    SYSTEMConfig(CPU_OSC_FREQ_HZ, SYS_CFG_ALL);
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);
    INTEnableInterrupts();
    UART_unitTest_echo();

    return 0;
}

#endif

