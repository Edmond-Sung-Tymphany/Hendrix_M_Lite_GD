
/*****************************************************************************
* BSP for DPP example, Microstick II board, Vanilla kernel, XC32
* Note hacked for PIC32 USB Starter Kit III
* Last Updated for Version: 4.5.04
* Date of the Last Update:  Jun 22, 2013
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2013 Quantum Leaps, LLC. All rights reserved.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Alternatively, this program may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GNU General Public License and are specifically designed for
* licensees interested in retaining the proprietary status of their code.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#include <p32xxxx.h>
#include "product.config" // All .c/.h files must include product.config
#include "qp_port.h"
#include "bsp.h"
#include "pragma.h"
#include "trace.h"
#include "tp_hwsetup.h"

/*****************************************************************************
 * Definition
 *****************************************************************************/
#define SYS_CFG_PCACHE  0x00000004

/* After upgrade, app. and bootloader may have different WDTPS configuration.
 * App. should disable watchdog if WDIPS (set by bootloader) is too small.
 *
 * Thus when you modify WDTPS on hwsetup.c (for bootloader),
 * remember to modify WDTPS_VALUE to corresponding valule here.
 *
 *   WDTPS_VALUE    WDTPS        Timeout
 *   12(01100)      PS4096       4.096s
 *   13(01101)      PS8192       8.192s
 *   14(01110)      PS16384      16.384s
 *   15(01111)      PS32768      32.768s
 */
#define WDTPS_VALUE  13  //mapping to configuration PS8192 (8.192s)

/*****************************************************************************
 * Global Variable
 *****************************************************************************/
/* system_sleeping_status*/
static bool isSystemAwake = TRUE;
volatile static uint32 currTime = 0;

/*****************************************************************************
 * QP
 *****************************************************************************/
Q_DEFINE_THIS_FILE
#ifdef Q_SPY
static uint8_t const l_tickISR = 0U;
static uint8_t const l_testISR = 0U;

#define QS_BUF_SIZE        4096
#define QS_BAUD_RATE       115200

#include "uartDrv.h"
#include "ringBuf.h"
cUartDrv qsUartDrv;
cRingBuf qsRingBuf;

enum AppRecords {                 /* application-specific trace records */
    PHILO_STAT = QS_USER
};
#endif



/*****************************************************************************
 * Function Implemenation
 *****************************************************************************/
/**
* Public function to check if the system is in deep sleep status
* @return[in]   TRUE/FALSE      Sleep or wakedup
*/
bool BSP_IsSystemAwake()
{
    return (isSystemAwake ? TRUE : FALSE);
}

void BSP_SetSystemAwake(bool awake)
{
    isSystemAwake = awake;
}
/* End of system_sleeping_status */

/* Watchdog */
void BSP_EnableWatchdog(void)
{
    //Disable watchdog if WDTPS is too small
    if( WDTCONbits.WDTPS < WDTPS_VALUE ) {
         TP_PRINTF("\r\n\r\n*** Watchdog mis-match between bootloader(WDTPS:%d) and app.(WDTPS:%d) ***\r\n\r\n\r\n", WDTCONbits.WDTPS, WDTPS_VALUE);
    }
    else {
        EnableWDT();
    }
}

void BSP_DisableWatchdog(void)
{
    DisableWDT();
}

void BSP_FeedWatchdog(void)
{
    ClearWDT();
}


#ifdef HAS_ALLPLAY
void SysTick_Handler(void);
#endif



/* ISRs --------------------------------------------------------------------*/
/* system clock tick */
void __ISR(_TIMER_2_VECTOR, IPL4) tickISR(void) {
    IFS0CLR = _IFS0_T2IF_MASK;              /* clear Timer 2 interrupt flag */
    QF_TICK(&l_tickISR);              /* handle all armed time events in QF */
    ++currTime;
}

/*--------------------------------------------------------------------------*/
void BSP_init(void)
{
    BSP_EnableWatchdog();
#ifndef PIC32_STARTER_KIT
    /*The JTAG is on by default on POR.  A PIC32 Starter Kit uses the JTAG, but
    for other debug tool use, like ICD 3 and Real ICE, the JTAG should be off
    to free up the JTAG I/O */
    DDPCONbits.JTAGEN = 0;
#endif

    // Configure the device for maximum performance, but do not change the PBDIV clock divisor.
    // Given the options, this function will change the program Flash wait states,
    // RAM wait state and enable prefetch cache, but will not change the PBDIV.
    // The PBDIV value is already set via the pragma FPBDIV option above..
    SYSTEMConfig(SYS_FREQ, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    /* Initialize I/O and Peripherals for application */
    INTConfigureSystem(INT_SYSTEM_CONFIG_MULT_VECTOR);

    Hardware_Init();
    Q_ALLEGE(QS_INIT((void *)0) != 0); /*initialize the QS software tracing */
    QS_RESET();
    QS_OBJ_DICTIONARY(&l_tickISR);
    QS_OBJ_DICTIONARY(&l_testISR);
}
/*..........................................................................*/
/**
* @brief Get the tick elapsed
* @return   uint32  the tick elapsed
*/
uint32 getSysTime()
{
    return currTime;
}

void BSP_BlockingDelayUs(uint16 us)
{    
    while(us--)
    {
        asm("nop");
        asm("nop");
    }
}

/* Get RTC time in string format. ex. "[  0. 33.384]"
 * Note core timer will overflow every 214.748 seconds. (under clock frequency 40MHz)
 */
INLINE uint32 getCoreTimeStr(char *buf, uint32 size)
{
    ASSERT(buf);
    uint32 time= GetSystemTimeUs();
    uint32 us= time%1000;
    uint32 ms= (time/1000)%1000;
    uint32 sec= time/1000/1000;
    uint32 len= snprintf(buf, size, "[%3ld.%3ld.%03ld]", sec, ms, us);
    return len;
}

/*..........................................................................*/
/* NOTE: this implementation of the assertion handler is intended only for
 * debugging and MUST be changed for deployment of the application (assuming
 * that you ship your production code with assertions enabled).
*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    char errString[255];

    QF_INT_DISABLE();             /* make sure that interrupts are disabled */
    sprintf(errString, "ASSERT: %s [%d]\r\n", file, line);
    ProjBsp_CyclePrintError(errString);
}


/* Allplay library use many assert() which is defiend in XC32 assert.h and indicate to _fassert().
 * Thus we overwrite _fassert. Then both assert() of Allplay library and ASSERT() of tym_plat call to Q_onAssert()
 */
void __attribute__((noreturn)) _fassert(int line, const char *file, const char *expression, const char *func)
{
    Q_onAssert(file,  line);
}


// This function overrides the normal _weak_ generic handler
void __attribute__((nomips16)) _general_exception_handler(void)
{
    // Declared static in case exception condition would prevent
    // an auto variable from being created
    static enum {
            EXCEP_IRQ = 0,      // interrupt
            EXCEP_AdEL = 4,     // address error exception (load or ifetch)
            EXCEP_AdES,         // address error exception (store)
            EXCEP_IBE,          // bus error (ifetch)
            EXCEP_DBE,          // bus error (load/store)
            EXCEP_Sys,          // syscall
            EXCEP_Bp,           // breakpoint
            EXCEP_RI,           // reserved instruction
            EXCEP_CpU,          // coprocessor unusable
            EXCEP_Overflow,     // arithmetic overflow
            EXCEP_Trap,         // trap (possible divide by zero)
            EXCEP_IS1 = 16,     // implementation specfic 1
            EXCEP_CEU,          // CorExtend Unuseable
            EXCEP_C2E           // coprocessor 2
    } _excep_code;
    static uint32_t _excep_addr;

    asm volatile("mfc0 %0,$13" : "=r" (_excep_code));
    asm volatile("mfc0 %0,$14" : "=r" (_excep_addr));

    _excep_code = (_excep_code & 0x0000007C) >> 2;
    // Examine _excep_code to identify the type of exception
    // Examine _excep_addr to find the address that caused the exception

    QF_INT_DISABLE();             /* make sure that interrupts are disabled */
    char errString[255];
    sprintf(errString, "EXCEPTION: %X [%d]\r\n", _excep_addr, _excep_code);
    ProjBsp_CyclePrintError(errString);
}

/*..........................................................................*/
// Memory Diagnostics
static const char* ramstart = (char*)0x20070000;
static const char* ramend = (char*)0x20088000;
extern char _end;

inline int stack_used() {
    register char* stack_ptr asm ("sp");
    return (ramend - stack_ptr);
}

inline int static_used() {
    return (&_end - ramstart);
}

inline int heap_used() {
//    struct mallinfo mi = mallinfo();
//    return (mi.uordblks);
    return 0;
}

void ram_diag()
{
    TP_PRINTF("SRAM usage (stack, heap, static): %d, %d, %d\n",
           stack_used(),
           heap_used(),
           static_used());
}

/*..........................................................................*/
void QF_onStartup(void) {            /* entered with interrupts disabled */
    INTCONSET = _INTCON_MVEC_MASK;   /* configure multi-vectored interrupts */
    INTCONbits.TPC = 7;              /* enable the Proximity Timer for all IPLs, NOTE02 */

    T2CON = 0x0060;                  /* stop timer, set up for 1:64 prescaler */
    TMR2 = 0;                        /* count from zero up to the period */
    PR2 = SYS_FREQ / (BSP_TICKS_PER_SEC * 64);     /* set the Timer2 period */
    IPC2bits.T2IP = 4;               /* Timer2 interrupt priority, must match tickISR (_TIMER_2_VECTOR, IPL4) tickISR */
    IFS0CLR = _IFS0_T2IF_MASK;       /* clear Timer2 Interrupt Flag */
    IEC0SET = _IEC0_T2IE_MASK;       /* enable Timer2 interrupt */
    T2CONSET = _T2CON_ON_MASK;       /* Start Timer2 */
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
static void __attribute__((nomips16)) pic32_wait()
{
    _wait();
}

void QF_onIdle(void) {          /* entered with interrupts disabled, NOTE01 */
    /* NOTE: not enough LEDs on the Microstick II board for both the app LED
    *  and the idle loop activity indicator ...
    */
#ifdef Q_SPY
    QF_INT_ENABLE();                       /* enable interrupts, see NOTE01 */

    const uint8_t * b;
    uint16_t pNbytes = 16;
    QF_INT_DISABLE();
    b = QS_getBlock(&pNbytes);
    QF_INT_ENABLE();

    if (b != NULL || pNbytes != 0) {
        UartDrv_Write(&qsUartDrv, (uint8 *)b, (uint32)pNbytes);
    }
#elif defined NDEBUG
    IPTMR = 4;       /* set the proximity timer to 4 CPU clocks, see NOTE02 */
    QF_INT_ENABLE();                               /* enable CPU interrupts */
     /* this "wait" code will put MCU to idle mode if OSCCON bit 3 is "0" (default value),
      but it will put MCU into deep sleep (no timer) if the bit is set,
      so make sure clear that bit after changing it*/
    pic32_wait();     /* execute the WAIT instruction to stop the CPU */
#else
    QF_INT_ENABLE();                       /* enable interrupts, see NOTE01 */
#endif
}


/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
uint8_t QS_onStartup(void const *arg) {
    static uint8_t qsBuf[QS_BUF_SIZE];            /* buffer for Quantum Spy */
    static uint8 txBuf[QS_BUF_SIZE];
    QS_initBuf(qsBuf, sizeof(qsBuf));     /* initialize the QS trace buffer */

    RingBuf_Ctor(&qsRingBuf, txBuf, QS_BUF_SIZE);
    UartDrv_Ctor(&qsUartDrv,(tUARTDevice *)getDevicebyId(DEBUG_ID,0), &qsRingBuf, 0);

    /* setup the QS filters... */
    QS_FILTER_ON(QS_ALL_RECORDS);

//    QS_FILTER_OFF(QS_QEP_STATE_EMPTY);
//    QS_FILTER_OFF(QS_QEP_STATE_ENTRY);
//    QS_FILTER_OFF(QS_QEP_STATE_EXIT);
//    QS_FILTER_OFF(QS_QEP_STATE_INIT);
//    QS_FILTER_OFF(QS_QEP_INIT_TRAN);
//    QS_FILTER_OFF(QS_QEP_INTERN_TRAN);
//    QS_FILTER_OFF(QS_QEP_TRAN);
//    QS_FILTER_OFF(QS_QEP_dummyD);

    QS_FILTER_OFF(QS_QF_ACTIVE_ADD);
    QS_FILTER_OFF(QS_QF_ACTIVE_REMOVE);
    QS_FILTER_OFF(QS_QF_ACTIVE_SUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_UNSUBSCRIBE);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET);
    QS_FILTER_OFF(QS_QF_ACTIVE_GET_LAST);
    QS_FILTER_OFF(QS_QF_EQUEUE_INIT);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_FIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_POST_LIFO);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET);
    QS_FILTER_OFF(QS_QF_EQUEUE_GET_LAST);
    QS_FILTER_OFF(QS_QF_MPOOL_INIT);
    QS_FILTER_OFF(QS_QF_MPOOL_GET);
    QS_FILTER_OFF(QS_QF_MPOOL_PUT);
    QS_FILTER_OFF(QS_QF_PUBLISH);
    QS_FILTER_OFF(QS_QF_NEW);
    QS_FILTER_OFF(QS_QF_GC_ATTEMPT);
    QS_FILTER_OFF(QS_QF_GC);
    QS_FILTER_OFF(QS_QF_TICK);
    QS_FILTER_OFF(QS_QF_TIMEEVT_ARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_AUTO_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM_ATTEMPT);
    QS_FILTER_OFF(QS_QF_TIMEEVT_DISARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_REARM);
    QS_FILTER_OFF(QS_QF_TIMEEVT_POST);
    QS_FILTER_OFF(QS_QF_CRIT_ENTRY);
    QS_FILTER_OFF(QS_QF_CRIT_EXIT);
    QS_FILTER_OFF(QS_QF_ISR_ENTRY);
    QS_FILTER_OFF(QS_QF_ISR_EXIT);

    return (uint8_t)1;            /* indicate successfull QS initialization */
}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
void QS_onFlush(void) {
//    QF_INT_ENABLE();                       /* enable interrupts, see NOTE01 */
//
//    const uint8_t * b;
//    uint16_t  pNbytes = QS_BUF_SIZE;
//    QF_INT_DISABLE();
//    b = QS_getBlock(&pNbytes);
//    QF_INT_ENABLE();
//
//    if (b != NULL || pNbytes != 0) {
//        UartDrv_Write(&qsUartDrv, (uint8 *)b, (uint32)pNbytes);
//    }
}
/*..........................................................................*/
/* NOTE: works properly with interrupts enabled or disabled */
QSTimeCtr QS_onGetTime(void) {
    return ReadCoreTimer();
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/

/*****************************************************************************
* NOTE01:
* The callback function QF_onIdle() is called with interrupts disabled,
* because the idle condition can be invalidated by any enabled interrupt
* that would post events. The QF_onIdle() function _must_ enable interrupts
* internally.
*
* NOTE02:
* The Temporal Proximity Timer is used to prevent a race condition of
* servicing an interrupt after re-enabling interrupts and before executing
* the WAIT instruction. The Proximity Timer is enabled for all interrupt
* priority levels (see QF_onStartup()). The Proximity Timer is set to 4
* CPU clocks right before re-enabling interrupts (with the DI instruction)
* The 4 clock ticks should be enough to execute the (DI,WAIT) instruction
* pair _atomically_.
*/

