/*****************************************************************************
* Product: QK/C port, PIC32, preemptive QK kernel, MPLABX-XC32 compiler
* Last Updated for Version: 4.5.04
* Date of the Last Update:  Jun 24, 2013
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
#ifndef qk_port_h
#define qk_port_h

                                             /* QK interrupt entry and exit */
#define QK_ISR_ENTRY() do { \
    QF_INT_DISABLE(); \
    ++QK_intNest_; \
    QF_QS_ISR_ENTRY(QK_intNest_, QK_currPrio_); \
    QF_INT_ENABLE(); \
} while (0)

#define QK_ISR_EXIT() do { \
    QF_INT_DISABLE(); \
    QF_QS_ISR_EXIT(QK_intNest_, QK_currPrio_); \
    --QK_intNest_; \
    if (QK_schedPrio_() != 0) { \
        IFS0SET = _IFS0_CS0IF_MASK; \
    } \
    QF_INT_ENABLE(); \
} while (0)

#include "qk.h"                 /* QK platform-independent public interface */

/*****************************************************************************
* NOTE01:
* Any interrupt service routine that interacts with QP must begin with the
* QK_ISR_ENTRY() macro and must end with the QK_ISR_EXIT() macro. The source
* file containing the interrupt service routine must #include <p32xxxx.h> or
* <plib.h>. Core software interrupt 0 and Interrupt Priority Level 1 are
* reserved for use by QK.
*/

#endif                                                         /* qk_port_h */

