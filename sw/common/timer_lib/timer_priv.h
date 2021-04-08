/**
 * @file        TimerSrv_priv.h
 * @brief       This file implements the timerSrv provider
 * @author      Bob.Xu
 * @date        2015-10-15
 * @copyright   Tymphany Ltd.
 */
#ifndef TIMERSRV_PRIV_H
#define TIMERSRV_PRIV_H
#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "deviceTypes.h"
#include "server.h"
#include "Timer.Config"


#ifdef __cplusplus
extern "C" {
#endif
#define TIMER_SRV_EVENT_Q_SIZE      2

typedef struct tTimer
{
    uint16            timerId;          //timer id
    int32             timeoutTicks;     //how many tick for a specific timer interval
    int32             absRegTime;       //absolute regsiter time
    timerCallbackFunc pCbFunc;          //the pointer of the callback function
    void              *pCbParam;        //the parameters of the callback function
    bool              isTimerFree;      //the status of this timer
}tTimer;

SUBCLASS(cTimeService, cServer)
  BOOL isCreated;
METHODS
/* public functions */
static void Time_StartUp(cPersistantObj *me);
static QState Time_Active(cTimeService * const me, QEvt const * const e);
static QState Time_Initial(cTimeService *const me, QEvt const * const e);
static void Time_UpdateTimer();
static void Time_InitTimers();
static void Time_SortTime(tTimer *timerArr, int len);
static void Time_FreeTimer(tTimer *timer);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* AUDIOSRV_PRIV_H */

