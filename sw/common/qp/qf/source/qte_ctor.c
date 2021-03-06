/*****************************************************************************
* Product: QF/C
* Last Updated for Version: 5.0.0
* Date of the Last Update:  Aug 28, 2013
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
#include "qf_pkg.h"
#include "qassert.h"

Q_DEFINE_THIS_MODULE("qte_ctor")

/**
* \file
* \ingroup qf
* \brief QTimeEvt_ctorX() implementation.
*/

/*..........................................................................*/
void QTimeEvt_ctorX(QTimeEvt * const me, QActive * const act,
                    enum_t const sig, uint8_t tickRate)
{
    Q_REQUIRE((sig >= (enum_t)Q_USER_SIG)           /* signal must be valid */
        && (tickRate < (uint8_t)QF_MAX_TICK_RATE));   /* tick rate in range */

    me->next      = (QTimeEvt *)0;
    me->ctr       = (QTimeEvtCtr)0;
    me->interval  = (QTimeEvtCtr)0;
    me->super.sig = (QSignal)sig;
    me->act       = act;                                      /* see NOTE01 */
                                   /* time event must be static, see NOTE02 */
    me->super.poolId_ = (uint8_t)0;              /* not from any event pool */
    me->super.refCtr_ = tickRate;                             /* see NOTE03 */
}

/*****************************************************************************
* NOTE01:
* For backwards compatibility with QTimeEvt_ctor(), the active object pointer
* can be uninitialized (NULL) and is _not_ validated in the precondition.
* The active object pointer is validated in preconditions to QTimeEvt_arm_()
* and QTimeEvt_rearm().
*
* NOTE02:
* Setting the POOL_ID event attribute to zero is correct only for events not
* allocated from event pools, which must be the case for Time Events.
*
* NOTE03:
* The reference counter attribute is not used in static events, so for the
* Time Events it is reused to hold the tickRate in the bits [0..6] and the
* linkedFlag in the MSB (bit [7]). The linkedFlag is 0 for time events
* unlinked from any list and 1 otherwise.
*/
