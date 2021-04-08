
#define xTP_UART_DUAL_ECHO_SERVER_TEST


#ifdef TP_UART_DUAL_ECHO_SERVER_TEST

#define TP_UART_NUMBER_OF_UART_PORTS 2  //a define that the user of the UART lib.
                                        //could specify how many UART modules they want to use

#include "UartDrv.h"
#include "ringbuf.h"
#include "tp_hwsetup_pic32.h"
#define TP_UART_TEST_WITH_AJ_SAM_MODULE

#ifdef TP_UART_TEST_WITH_AJ_SAM_MODULE
#define SET_RESET_PIN { LATBbits.LATB12 = 1 ; TRISBbits.TRISB12 = 0; ANSELBbits.ANSB12 = 0; }
#define SET_3V3_EN_PIN { LATBbits.LATB15 = 1 ; TRISBbits.TRISB15 = 0; ANSELBbits.ANSB15 = 0; }
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

const tUARTDevice uartConfig0 = {
    .deviceInfo.deviceID    = 0,
    .deviceInfo.deviceType  = UART_TYPE,
    .uartId     = TP_UART_DEV_1,
    .baudrate   = 115200,
    .byteSize   = UART_DATA_SIZE_8_BITS,
    .parity     = UART_PARITY_NONE,
    .stopBits   = UART_STOP_BITS_1,
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
    .dmaEnable  = FALSE,
    .interrupt.priority    = INT_PRIORITY_LEVEL_5,
    .interrupt.subPriority = INT_SUB_PRIORITY_LEVEL_0,
};

int UART_unitTest_echo(void)
{
    unsigned char helloText[] = "Hello World, I am UART #";

    cUartDrv* pUart0 = &uart0;
    cUartDrv* pUart1 = &uart1;
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
    SET_RESET_PIN;
    SET_3V3_EN_PIN;
#endif

    UartDrv_Write(pUart0, helloText, sizeof(helloText));
    UartDrv_Write(pUart0, "0\n\r", sizeof("0\n\r"));
    UartDrv_Write(pUart1, helloText, sizeof(helloText));
    UartDrv_Write(pUart1, "1\n\r", sizeof("1\n\r"));

    while(1)
    {
        char tmp[100];
        int cnt;

        cnt = UartDrv_Read(pUart0, tmp, sizeof(tmp));
        UartDrv_Write(pUart0, tmp, cnt);

        cnt = UartDrv_Read(pUart1, tmp, sizeof(tmp));
        UartDrv_Write(pUart1, tmp, cnt);
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

