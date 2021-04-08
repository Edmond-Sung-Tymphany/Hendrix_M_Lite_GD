
#include "persistantObj.h"
#include "projBsp.h"



void PersistantObj_Ctor(cPersistantObj *me, QStateHandler initState, eSignal timeOutSignal,
                             QEvt const *evtQue[], uint32 evtQueLen, uint8 objectId)
{
    QTimeEvt_ctorX(TIME_EVT_OF(me), (QActive*)me, timeOutSignal, 0U);
    QActive_ctor((QActive*)me, initState);
    /* Active object start */
    QActive_start((QActive*)me, objectId, evtQue, evtQueLen, (void *)0, 0U, (QEvt *)0);
}



void PersistantObj_Xtor( cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    QActive_stop((QActive *)me);
}



void PersistantObj_RefreshTick(cPersistantObj* me, const uint32 tickTime)
{
    QTimeEvt_disarm(TIME_EVT_OF(me));
    QTimeEvt_armX(TIME_EVT_OF(me), GET_TICKS_IN_MS(tickTime), 0);
}


