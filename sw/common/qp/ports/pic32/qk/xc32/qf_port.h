/*****************************************************************************
* Product:  QF/C, PIC32, preemptive QK kernel, XC32 compiler
* Last Updated for Version: 4.5.04
* Date of the Last Update:  Jun 19, 2013
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
#ifndef qf_port_h
#define qf_port_h

     /* The maximum number of active objects in the application, see NOTE01 */
#define QF_MAX_ACTIVE           32

                                 /* QF interrupt disable/enable, see NOTE02 */
#define QF_INT_DISABLE()        __builtin_disable_interrupts()
#define QF_INT_ENABLE()         __builtin_enable_interrupts()

                              /* QF critical section entry/exit, see NOTE02 */
/* QF_CRIT_STAT_TYPE not defined: unconditional interrupt disabling policy  */
#define QF_CRIT_ENTRY(dummy)    QF_INT_DISABLE()
#define QF_CRIT_EXIT(dummy)     QF_INT_ENABLE()

                                    /* fast log-base-2 with CLZ instruction */
#define QF_LOG2(n_) ((uint8_t)(32 - _clz(n_)))

#include <xc.h>                                               /* for _clz() */

#include "qep_port.h"                                           /* QEP port */
#include "qk_port.h"                           /* QK preemptive kernel port */
#include "qf.h"                 /* QF platform-independent public interface */


/*****************************************************************************
* NOTE01:
* The maximum number of active objects QF_MAX_ACTIVE can be increased
* up to 63, if necessary. Here it is set to a lower level to save some RAM.
*
* NOTE02:
* The DI (disable interrupts) instruction is used for fast, unconditional
* disabling and enabling of interrupts. The DI instruction only disables
* interrupts with priority levels 1-6. Priority level 7 interrupts and all
* trap events still have the ability to interrupt the CPU when the DI
* instruction is active. This means that from the perspective of QP, the
* level 7 interrupts are treated as non-maskable interrupts (NMIs). Such
* non-maskable interrupts _cannot_ call any QP services. In particular,
* they cannot post events.
*
* CAUTION: This QP port assumes that interrupt nesting is _enabled_,
* which is the default in the PIC32 processors. Interrupt nesting should
* never be disabled by setting the NSTDIS control bit (INTCON1<15>). If you
* don't want interrupts to nest, you can always prioritize them at the same
* level. For example, the default priority level for all interrupts is 4 out
* of reset. If you don't change this level for any interrupt the nesting of
* interrupt will not occur.
*/

#endif                                                         /* qf_port_h */
