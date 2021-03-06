/*****************************************************************************
* Product:  QF/C, port to 80x86, uC/OS-II v2.86, Open Watcom compiler
* Last Updated for Version: 5.0.0
* Date of the Last Update:  Sep 13, 2013
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

#include <dos.h>                       /* for _dos_setvect()/_dos_getvect() */

Q_DEFINE_THIS_MODULE("qf_port")

/* Local objects -----------------------------------------------------------*/
static void interrupt (*l_dosSpareISR)(void);

/*..........................................................................*/
void QF_init(void) {
    OSInit();                                        /* initialize uC/OS-II */
}
/*..........................................................................*/
int16_t QF_run(void) {
                                  /* install uC/OS-II context switch vector */
    l_dosSpareISR = _dos_getvect(uCOS);
    _dos_setvect(uCOS, (void interrupt (*)(void))&OSCtxSw);

    /* NOTE the QF_onStartup() callback must be invoked from the task level */
    OSStart();                               /* start uC/OS-II multitasking */
    Q_ERROR();                             /* OSStart() should never return */
    return (int16_t)0;  /* this unreachable return keeps the compiler happy */
}
/*..........................................................................*/
void QF_stop(void) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();
    _dos_setvect(uCOS, l_dosSpareISR);   /* restore the original DOS vector */
    QF_CRIT_EXIT_();
    QF_onCleanup();                                     /* cleanup callback */
}
/*..........................................................................*/
static void task_function(void *pdata) {         /* uC/OS-II task signature */
    while (((QActive *)pdata)->thread != (uint8_t)0) {
        QEvt const *e = QActive_get_((QActive *)pdata);
        QMSM_DISPATCH((QMsm *)pdata, e);         /* dispatch to the AO's SM */
        QF_gc(e);    /* check if the event is garbage, and collect it if so */
    }

    QF_remove_((QActive *)pdata);  /* remove this object from the framework */
    OSTaskDel(OS_PRIO_SELF);        /* make uC/OS-II forget about this task */
}
/*..........................................................................*/
void QActive_start(QActive *me, uint8_t prio,
                   QEvt const *qSto[], uint32_t qLen,
                   void *stkSto, uint32_t stkSize,
                   QEvt const *ie)
{
    INT8U err;
    me->eQueue = OSQCreate((void **)qSto, qLen);
    Q_ASSERT(me->eQueue != (OS_EVENT *)0);        /* uC/OS-II queue created */
    me->prio = prio;                                /* save the QF priority */
    QF_add_(me);                     /* make QF aware of this active object */
    QMSM_INIT(&me->super, ie);                /* execute initial transition */

    QS_FLUSH();                       /* flush the trace buffer to the host */

                        /* uC/OS task is represented by its unique priority */
    me->thread = (uint8_t)(QF_MAX_ACTIVE - me->prio); /* map to uC/OS prio. */
    err = OSTaskCreateExt(&task_function,              /* the task function */
             me,                                   /* the 'pdata' parameter */
             &(((OS_STK *)stkSto)[(stkSize / sizeof(OS_STK)) - 1]), /* ptos */
             me->thread,                          /* uC/OS-II task priority */
             me->thread,                                         /* task id */
             (OS_STK *)stkSto,                                      /* pbos */
             stkSize/sizeof(OS_STK),   /* size of the stack in OS_STK units */
             (void *)0,                                             /* pext */
             (INT16U)OS_TASK_OPT_STK_CLR);                           /* opt */
    Q_ASSERT(err == OS_NO_ERR);                    /* uC/OS-II task created */
}
/*..........................................................................*/
void QActive_stop(QActive *me) {
    INT8U err;
    me->thread = (uint8_t)0;                        /* stop the thread loop */
    OSQDel(me->eQueue, OS_DEL_ALWAYS, &err);           /* cleanup the queue */
    Q_ASSERT(err == OS_NO_ERR);
}
/*..........................................................................*/
#ifndef Q_SPY
uint8_t QActive_post(QActive * const me, QEvt const * const e,
                     uint16_t const margin)
#else
uint8_t QActive_post(QActive * const me, QEvt const * const e,
                     uint16_t const margin, void const * const sender)
#endif
{
    uint8_t status;
    uint16_t nFree;
    QF_CRIT_STAT_

    QF_CRIT_ENTRY_();

    nFree = (uint16_t)(((OS_Q_DATA *)me->eQueue)->OSQSize
                       - ((OS_Q_DATA *)me->eQueue)->OSNMsgs);
    if (nFree > margin) {
        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_FIFO, QS_priv_.aoObjFilter, me)
            QS_TIME_();                                        /* timestamp */
            QS_OBJ_(sender);                           /* the sender object */
            QS_SIG_(e->sig);                     /* the signal of the event */
            QS_OBJ_(me);                  /* this active object (recipient) */
            QS_U8_(e->poolId_);                 /* the pool Id of the event */
            QS_U8_(e->refCtr_);               /* the ref count of the event */
            QS_EQC_(nFree);                               /* # free entries */
            QS_EQC_(0);                     /* min # free entries (unknown) */
        QS_END_NOCRIT_()

        if (e->poolId_ != (uint8_t)0) {              /* is it a pool event? */
            QF_EVT_REF_CTR_INC_(e);      /* increment the reference counter */
        }
        /* posting the event to uC/OS message queue must succeed, NOTE01 */
        Q_ALLEGE(OSQPost((OS_EVENT *)me->eQueue, (void *)e) == OS_NO_ERR);
        status = (uint8_t)1;                              /* return success */
    }
    else {
        Q_ASSERT(margin != (uint16_t)0);    /* can tollerate dropping evts? */

        QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_ATTEMPT, QS_priv_.aoObjFilter, me)
            QS_TIME_();                                        /* timestamp */
            QS_OBJ_(sender);                           /* the sender object */
            QS_SIG_(e->sig);                     /* the signal of the event */
            QS_OBJ_(me);                  /* this active object (recipient) */
            QS_U8_(e->poolId_);                 /* the pool Id of the event */
            QS_U8_(e->refCtr_);               /* the ref count of the event */
            QS_EQC_(nFree);                       /* number of free entries */
            QS_EQC_((QEQueueCtr)margin);                /* margin requested */
        QS_END_NOCRIT_()

        status = (uint8_t)0;
    }

    QF_CRIT_EXIT_();

    return status;
}
/*..........................................................................*/
void QActive_postLIFO(QActive *me, QEvt const *e) {
    QF_CRIT_STAT_
    QF_CRIT_ENTRY_();

    QS_BEGIN_NOCRIT_(QS_QF_ACTIVE_POST_LIFO, QS_priv_.aoObjFilter, me)
        QS_TIME_();                                            /* timestamp */
        QS_SIG_(e->sig);                        /* the signal of this event */
        QS_OBJ_(me);                                  /* this active object */
        QS_U8_(e->poolId_);                     /* the pool Id of the event */
        QS_U8_(e->refCtr_);                   /* the ref count of the event */
            QS_EQC_(((OS_Q *)me->eQueue)->OSQSize
                  - ((OS_Q *)me->eQueue)->OSQEntries);    /* # free entries */
        QS_EQC_(0);                         /* min # free entries (unknown) */
    QS_END_NOCRIT_()

    if (e->poolId_ != (uint8_t)0) {                  /* is it a pool event? */
        QF_EVT_REF_CTR_INC_(e);          /* increment the reference counter */
    }

           /* posting the event to uC/OS message queue must succeed, NOTE01 */
    Q_ALLEGE(OSQPostFront((OS_EVENT *)me->eQueue, (void *)e) == OS_NO_ERR);

    QF_CRIT_EXIT_();
}
/*..........................................................................*/
QEvt const *QActive_get_(QActive *me) {
    INT8U err;
    QEvt const *e = (QEvt *)OSQPend((OS_EVENT *)me->eQueue, 0, &err);
    QS_CRIT_STAT_

    Q_ASSERT(err == OS_NO_ERR);

    QS_BEGIN_(QS_QF_ACTIVE_GET, QS_priv_.aoObjFilter, me)
        QS_TIME_();                                            /* timestamp */
        QS_SIG_(e->sig);                        /* the signal of this event */
        QS_OBJ_(me);                                  /* this active object */
        QS_U8_(e->poolId_);                     /* the pool Id of the event */
        QS_U8_(e->refCtr_);                   /* the ref count of the event */
        QS_EQC_(0);                     /* number of free entries (unknown) */
    QS_END_()

    return e;
}

/*****************************************************************************
* NOTE01:
* The event posting to the uC/OS message queue occurs inside a critical
* section. This assumes that the uC/OS critical section must be able to nest.
*/
