/*****************************************************************************
* BSP for PICDEM 2 PLUS with PIC18F452
* Last Updated for Version: 4.4.01
* Date of the Last Update:  Mar 23, 2012
*
*                    Q u a n t u m     L e a P s
*                    ---------------------------
*                    innovating embedded systems
*
* Copyright (C) 2002-2012 Quantum Leaps, LLC. All rights reserved.
*
* This software may be distributed and modified under the terms of the GNU
* General Public License version 2 (GPL) as published by the Free Software
* Foundation and appearing in the file GPL.TXT included in the packaging of
* this file. Please note that GPL Section 2[b] requires that all works based
* on this software must also be made publicly available under the terms of
* the GPL ("Copyleft").
*
* Alternatively, this software may be distributed and modified under the
* terms of Quantum Leaps commercial licenses, which expressly supersede
* the GPL and are specifically designed for licensees interested in
* retaining the proprietary status of their code.
*
* Contact information:
* Quantum Leaps Web sites: http://www.quantum-leaps.com
*                          http://www.state-machine.com
* e-mail:                  info@quantum-leaps.com
*****************************************************************************/
#include "bsp.h"
#include "projBsp.h"
#include "qp_port.h"

#include "commonTypes.h"
#include "trace.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>                            /* for memcpy() and memset() */
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>


#define INTERNAL_CLOCK  0
#define EXTERNAL_CLOCK 0x04
#define PLL_CLOCK  0x08

/* clock frequency 8M (internal clock) */
uint32_t SystemCoreClock    = 8000000;

//Q_DEFINE_THIS_FILE
#define Sleep() // TODO

volatile static uint32 currTime = 0;
static struct termios l_tsav;   /* structure with saved terminal attributes */

void SysTick_Handler(void);

/* system_sleeping_status*/
static bool isSystemAwake = TRUE;


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

/*..........................................................................*/
void QF_onClockTick(void) {
    struct timeval timeout = { 0 };                 /* timeout for select() */
    fd_set con;                          /* FD set representing the console */

    ++currTime;
    QF_TICK(&l_clock_tick);         /* perform the QF clock tick processing */

    FD_ZERO(&con);
    FD_SET(0, &con);
    /* check if a console input is available, returns immediately */
    if (0 != select(1, &con, 0, 0, &timeout)) {      /* any descriptor set? */
        char ch;
        read(0, &ch, 1);
        if (ch == '\33') {                                  /* ESC pressed? */
            QF_stop();
        }
    }
}
/*..........................................................................*/
void BSP_init(void)
{

}
/*..........................................................................*/
void QF_onStartup(void)
{
    struct termios tio;                     /* modified terminal attributes */

    tcgetattr(0, &l_tsav);          /* save the current terminal attributes */
    tcgetattr(0, &tio);           /* obtain the current terminal attributes */
    tio.c_lflag &= ~(ICANON | ECHO);   /* disable the canonical mode & echo */
    tcsetattr(0, TCSANOW, &tio);                  /* set the new attributes */

    QF_setTickRate(BSP_TICKS_PER_SEC);         /* set the desired tick rate */
}
/*..........................................................................*/
void QF_onCleanup(void) {
    tcsetattr(0, TCSANOW, &l_tsav);/* restore the saved terminal attributes */
}
/*..........................................................................*/
void QF_onIdle(void) {      /* entered with interrupts disabled, see NOTE01 */
#ifdef Q_SPY
    if (PIR1bits.TXIF != 0U) {                              /* TXREG empty? */
        uint16_t b = QS_getByte();
        if (b != QS_EOD) {                 /* next QS trace byte available? */
            TXREG = (uint8_t)b; /* stick the byte to TXREG for transmission */
        }
    }
#elif defined NDEBUG
    Sleep();                        /* transition to SLEEP mode, see NOTE02 */
#endif
}
/*..........................................................................*/
void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    TP_PRINTF("Assertion failed in %s, line %d", file, line);
    exit(-1);
}

/*..........................................................................*/
/* assert function for stm library,          */
/* add Macro "USE_FULL_ASSERT" to turn it on */
/*..........................................................................*/
void assert_failed(uint8_t* file, uint32_t line)
{
    Q_onAssert((char*)file,(int)line);
}

/**
* @brief Get the tick elapsed
* @return   uint32  the tick elapsed
*/
uint32 getSysTime()
{
    return currTime;
}


/***************************************************************************
*
* watchdog place holder
*
****************************************************************************/

/* Watchdog */
void BSP_EnableWatchdog(void)
{
}

void BSP_DisableWatchdog(void)
{
}

void BSP_FeedWatchdog(void)
{
}



/*--------------------------------------------------------------------------*/
#ifdef Q_SPY
#define QS_BUF_SIZE        200U
#define BAUD_RATE          19200U
QSTimeCtr BSP_tickTime;
uint8_t QS_onStartup(void const *arg) {

}
/*..........................................................................*/
void QS_onCleanup(void) {
}
/*..........................................................................*/
void QS_onFlush(void) {
}
/*..........................................................................*/
/* NOTE: invoked within a critical section (inetrrupts disabled) */
QSTimeCtr QS_onGetTime(void) {
}
#endif                                                             /* Q_SPY */
/*--------------------------------------------------------------------------*/
/*****************************************************************************
* NOTE01:
* The callback function QF_onIdle() is called with interrupts disabled,
* because the idle condition can be invalidated by any enabled interrupt that
* would post events. The QF_onIdle() function *must* enable interrupts
* internally
*
* NOTE02:
* The low-power mode must be entered with interrupts disabled to avoid
* an interrupt to preempt the transition to the low-power mode. According to
* the "PIC18FXX2 Data Sheet", the Sleep mode can be exited even if the global
* inetrrupt enable flag is cleared (INTCON<7> == 0). This allows for
* an atomic transition to the SLEEP mode.
*
* Selected interrupts, such as Timer1 interrupt with external clock (not
* synchronized to the CPU clock) can wake up the CPU from the SLEEP mode.
* after waking up, the CPU does not service the interrupt immediately,
* because interrupts are still disabled. The interrupt gets serviced only
* after the CPU unlocks interrupts.
*/
