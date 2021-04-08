  /*
-------------------------------------------------------------------------------
TYMPHANY





                  Timer service
                  -------------------------

                  SW Module Document




@file        Timer.h
@brief       This file defines the interface of the time service which gives a "one-shot" callback 
             event after a specific time interval ie a Timer,
             
             Please note:
             to use this timer service one macro has to be defined, NUM_OF_MAX_TIMER,
             the value of this macrois up to your project
@author      Bob.Xu 
@date        2015-10-15
@Copyright (c) <2015> Tymphany HK Ltd. All rights reserved.

SCO/ERROR  : 
-------------------------------------------------------------------------------
*/

#ifndef TIMER_H
#define TIMER_H
#include "commonTypes.h"
#define TIMER_ID_INVALID           0xFFFF 
typedef void (*timerCallbackFunc)(void *pCbPara);

typedef struct tTimerInfo
{
    uint32            expireTime;   //in ms, it tells you how long does it left to involke the callback function      
    timerCallbackFunc pCbFunc;      //the pointer of the callback function
    void              *pCbParam;    //the parameters of the callback function
    bool              isTimerFree;  //the status of this timer
}tTimerInfo;


/**
* Start a Timer
* @param[in]       time       the time interval in ms, the callback function will be invoked after this period is passed by
* @param[in]       pTimerId   the allocated timer ID will be save to the memory location of this poiter
* @param[in]       pFunc      callback function pointer
* @param[in]       pCbPara    the parameters for the callback function, user should cast it to right type in the callback function
* @param[return]   bool       TRUE/FALSE, TRUE indicate that the timer allocate succeed, otherwise there is no timer available
*/
bool Timer_StartTimer(uint32 timeout,uint16 *pTimerId, timerCallbackFunc pFunc, void *pCbPara);

/**
* Stop a timer
* @param[in]      timerId   The timer which will be stoped
* @param[return]  bool      TRUE/FALSE, TRUE means the timer is stoped, otherwise the timer doesn't exist
*/
bool Timer_StopTimer(uint16 timerId);

/**
* Check a specific timer status
* @param[in]        timerId          the timer ID
* @param[in]        pTimerInfo       a pointer to tell the info of a specific timer
* @param[return]    bool             TRUE or FALSE, FALSE means the given timer ID doesn't exist
*/
bool Timer_GetTimerStatus(uint16 timerId, tTimerInfo * pTimerInfo);

#endif  /* TIMER_H */
