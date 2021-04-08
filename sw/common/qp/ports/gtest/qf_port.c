/*****************************************************************************
* Product: QF/C port for gtest - based off the win32 port
* Last Updated for Version: 5.0.0
* Date of the Last Update:  July 31, 2014
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
//#include "qassert.h"

#include "controller.h"
#include "deviceTypes.h"
#include "qf_port.h"
#include <string.h>

#include "fff.h"

tLargeEvt largeEvt;

DEFINE_FFF_GLOBALS;

#ifndef GTEST_CONTROLLER
FAKE_VOID_FUNC(CommonEvtResp, QActive* , QActive* , eEvtReturn ,  eSignal);
FAKE_VOID_FUNC(SendToServer, uint16 , const QEvt* );

#else

FAKE_VALUE_FUNC(int16, QF_run);
FAKE_VOID_FUNC(QF_poolInit,void *, uint32_t, uint32_t);
FAKE_VOID_FUNC(QF_init);
FAKE_VOID_FUNC(QF_psInit, QSubscrList *, uint32_t);

#endif

void QEQueue_init(QEQueue * const me, QEvt const *qSto[], QEQueueCtr const qLen)
{
    (void)me; (void)qSto;
     (void) qLen;
}

void QActive_start(QActive * const me, uint8_t prio,
                   QEvt const *qSto[], uint32_t qLen,
                   void *stkSto, uint32_t stkSize,
                   QEvt const *ie)
{

    (void)me; (void)prio; (void)qSto; (void)qLen;
                   (void)stkSto; (void)ie;
    (void)stkSize;         /* avoid the "unused parameter" compiler warning */

     me->prio = prio;           /* set the QF priority of this active object */
}

void QActive_ctor(QActive * const me, QStateHandler initial) {

    (void)me; (void) initial;
    static QActiveVtbl const vtbl = {              /* QActive virtual table */
        { 0,
          0 },
        &QActive_start,
        &QActive_post,
        &QActive_postLIFO
    };
    me->super.vptr = &vtbl.super; /* hook the vptr to QActive virtual table */
}


FAKE_VALUE_FUNC(const tDevice*, getDevicebyId, eDeviceID, uint16*);

FAKE_VOID_FUNC(QActive_subscribe, QActive const *, enum_t);
FAKE_VOID_FUNC(QActive_unsubscribe, QActive const *, enum_t);
FAKE_VOID_FUNC(QF_publish, QEvt const *);

FAKE_VALUE_FUNC(uint8_t, QActive_defer, QActive *, QEQueue *, QEvt const *);

FAKE_VALUE_FUNC(uint8_t, QActive_recall, QActive *, QEQueue *);

FAKE_VOID_FUNC(Q_onAssert, char const Q_ROM *, int);

FAKE_VOID_FUNC(QTimeEvt_ctorX, QTimeEvt *, QActive *, enum_t, uint8_t );

FAKE_VOID_FUNC(QTimeEvt_armX, QTimeEvt *, QTimeEvtCtr, QTimeEvtCtr);

FAKE_VALUE_FUNC(uint8_t, QTimeEvt_disarm, QTimeEvt *);

FAKE_VALUE_FUNC(uint8_t, QActive_post, QActive *, QEvt const *, uint16_t);
FAKE_VOID_FUNC(QActive_postLIFO, QActive *, QEvt const *);
FAKE_VOID_FUNC(QActive_stop, QActive *);


QState QHsm_top(void const * const me, QEvt const * const e) {
    (void)me;            /* suppress the "unused argument" compiler warning */
    (void)e;             /* suppress the "unused argument" compiler warning */
    return Q_IGNORED();                 /* the top state ignores all events */
}


QEvt * QF_newX_(QEvtSize const evtSize,
               uint16_t const margin, enum_t const sig)
{
    (void) evtSize;
    (void) margin;
	((QEvt*)&largeEvt)->sig = (QSignal)sig;
	return (QEvt*)&largeEvt;
}

