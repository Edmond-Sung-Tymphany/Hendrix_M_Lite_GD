/**
*  @file      delegate.c
*  @brief     Source file delegate class
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*/

#include "delegate.h"
#include "controller.h"


cDelegate * Delegate_Ctor(cDelegate *me, QActive *pDlgOwner, QStateHandler initState)
{
    QActive_ctor((QActive *)me, initState);
    me->delegateOwner = pDlgOwner;
    return me;
}
void Delegate_Start(cDelegate *me,
                   QEvt const *eventQueue[], uint32_t evtQueueLen)
{

    /* active object start */
    QActive_start((QActive*)me, GetNextFreePriority(), eventQueue, evtQueueLen, (void *)0, 0U, (QEvt *)0);
}


void Delegate_Xtor(cDelegate *me)
{
    ReleasePriority(((QActive*)me)->prio);
    QActive_stop((QActive *)me);
}

