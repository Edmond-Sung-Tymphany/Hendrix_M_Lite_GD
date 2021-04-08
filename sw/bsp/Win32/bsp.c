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
#include "qp_port.h"
#include "bsp.h"

#ifdef Q_SPY
    static uint8_t const l_tickISR = 0U;
    static uint8_t const l_testISR = 0U;

#define QS_BUF_SIZE        4096
#define QS_BAUD_RATE       115200

#include "uartDrv.h"
#include "ringBuf.h"
cUartDrv qsUartDrv;
cRingBuf qsRingBuf;

#endif

/* ISRs --------------------------------------------------------------------*/
/* system clock tick */
void tickISR(void) {
    //QF_TICK(&l_tickISR);              /* handle all armed time events in QF */
}

/*--------------------------------------------------------------------------*/
void BSP_init(void)
{

    //BSP_watchdog_start();
}
/*..........................................................................*/
// Watchdog
void BSP_watchdog_start(void)
{
    
}

void BSP_watchdog_stop(void)
{
    
}

void BSP_watchdog_reset(void)
{
    
}

void delay_us(uint32_t time_us) //busy waiting
{
}


/*..........................................................................*/
/**
* For Q_onAssert and _general_exception_handler to print the error message
*   The interrupts should be disabled before this function call
*   This function is a blocking print with UART with DEBUG_ID
*   This function will have the delay so the caller can continuously print
* @param[in]    s               Error message String
*/
// BSP_print_error for Q_onAssert and _general_exception_handler
static void BSP_print_error(char* s)
{
    
}
/*..........................................................................*/
/* NOTE: this implementation of the assertion handler is intended only for
 * debugging and MUST be changed for deployment of the application (assuming
 * that you ship your production code with assertions enabled).
*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    
}


/*..........................................................................*/
void QF_onStartup(void) {            /* entered with interrupts disabled */


  
}
/*..........................................................................*/
void QF_onCleanup(void) {
}
/*..........................................................................*/
void QF_onIdle(void) {          /* entered with interrupts disabled, NOTE01 */
    /* NOTE: not enough LEDs on the Microstick II board for both the app LED
    *  and the idle loop activity indicator ...
    */

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

