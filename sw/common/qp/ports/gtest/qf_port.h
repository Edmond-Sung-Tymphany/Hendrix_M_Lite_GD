/*****************************************************************************
* Product: QF/C port to Win32
* Last Updated for Version: 5.0.0
* Date of the Last Update:  Sep 12, 2013
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

                                      /* Win32 event queue and thread types */
#define QF_EQUEUE_TYPE              QEQueue
#define QF_OS_OBJECT_TYPE           void*
#define QF_THREAD_TYPE              void*

                 /* The maximum number of active objects in the application */
#define QF_MAX_ACTIVE               63
                                   /* The number of system clock tick rates */
#define QF_MAX_TICK_RATE            2

                     /* various QF object sizes configuration for this port */
#define QF_EVENT_SIZ_SIZE           4
#define QF_EQUEUE_CTR_SIZE          4
#define QF_MPOOL_SIZ_SIZE           4
#define QF_MPOOL_CTR_SIZE           4
#define QF_TIMEEVT_CTR_SIZE         4


#define QF_INT_DISABLE() ((void)0)
#define QF_INT_ENABLE() ((void)0)
                                      /* Win32 critical section, see NOTE01 */
/* QF_CRIT_STAT_TYPE not defined */
#define QF_CRIT_ENTRY(dummy)        ((void)0)
#define QF_CRIT_EXIT(dummy)         ((void)0)

#include "qep_port.h"                                           /* QEP port */
#include "qequeue.h"                             /* Win32 needs event-queue */
#include "qmpool.h"                              /* Win32 needs memory-pool */
#include "qf.h"                 /* QF platform-independent public interface */
#include "qvanilla.h"                       /* "Vanilla" cooperative kernel */
#include "product.config"

/* GTEST SPECIFICS */

/* Declare a largeType in your test source file then it will be extern'd in to the qf_port.c */
typedef struct { uint8_t t[SIZE_OF_LARGE_EVENTS];} tLargeEvt;

/* Macros for easy sending msgs to the Object Under Test */

#define SEND_EVT(obj_, evt_) _SEND_EVT(&((QActive*)(obj_))->super, evt_)
#define _SEND_EVT(obj_, evt_) ((*(obj_)->vptr->dispatch)((obj_), (QEvt*)(evt_)))

#define INIT_OBJ(obj_, evt_) _SEND_EVT(&((QActive*)(obj_))->super, evt_)
#define _INIT_OBJ(obj_, evt_) ((*(obj_)->vptr->init)((obj_), (QEvt*)(evt_)))		


/* NOTES: ********************************************************************
*
* NOTE01:
* QF, like all real-time frameworks, needs to execute certain sections of
* code indivisibly to avoid data corruption. The most straightforward way of
* protecting such critical sections of code is disabling and enabling
* interrupts, which Win32 does not allow.
*
* This QF port uses therefore a single package-scope Win32 critical section
* object QF_win32CritSect_ to protect all critical sections.
*
* Using the single critical section object for all crtical section guarantees
* that only one thread at a time can execute inside a critical section. This
* prevents race conditions and data corruption.
*
* Please note, however, that the Win32 critical section implementation
* behaves differently than interrupt locking. A common Win32 critical section
* ensures that only one thread at a time can execute a critical section, but
* it does not guarantee that a context switch cannot occur within the
* critical section. In fact, such context switches probably will happen, but
* they should not cause concurrency hazards because the critical section
* eliminates all race conditionis.
*
* Unlinke simply disabling and enabling interrupts, the critical section
* approach is also subject to priority inversions. Various versions of
* Windows handle priority inversions differently, but it seems that most of
* them recognize priority inversions and dynamically adjust the priorities of
* threads to prevent it. Please refer to the MSN articles for more
* information.
*/

#endif                                                         /* qf_port_h */
