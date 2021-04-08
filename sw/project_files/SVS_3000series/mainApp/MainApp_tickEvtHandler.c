/**
*  @file      MainApp_tickEvtHandler.c
*  @brief     tick event handler of mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "KeySrv.h"
#include "tym_qp_lib.h"
#include "MainApp.h"
#include "MainApp_priv.h"
#include "SettingSrv.h"

static QState MainApp_EnterIdleTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_EnterStandbyTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_JackLowTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StartupAmpTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_ShutDownAmpTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DspMuteTimeoutHandler(cMainApp * const me, QEvt const * const e);


/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_EnterIdleTimeoutHandler},            //TIMER_ID_ENTER_IDLE_TIMEOUT
    {0, (TickHandler)MainApp_EnterStandbyTimeoutHandler},         //TIMER_ID_ENTER_STANDBY_TIMEOUT
    {0, (TickHandler)MainApp_SysTempCheckTimeoutHandler},         //TIMER_ID_SYS_TEMP_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_StackCheckTimeoutHandler},           //TIMER_ID_STACK_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_JackLowTimeoutHandler},              //TIMER_ID_JACK_LOW_TIMEOUT
    {0, (TickHandler)MainApp_StartupAmpTimeoutHandler},          //TIMER_ID_STARTUP_AMP_TIMEOUT
    {0, (TickHandler)MainApp_ShutDownAmpTimeoutHandler},          //TIMER_ID_SHUTDOWN_AMP_TIMEOUT
    {0, (TickHandler)MainApp_DspMuteTimeoutHandler},              //TIMER_ID_DSPMUTE_TIMEOUT
};



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

void MainApp_DeleteAllTickHandlers(void)
{
    for(uint32 i = 0; i < TIMER_ID_MAX; i++)
    {
        tickHandlerlist[i].timer = INVALID_VALUE;
    }
}

static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
//    me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
//    bool stackOverflow= stackOverflowCheck();
//    ASSERT(!stackOverflow);
//    if( stackOverflow )
//    {
//        TP_PRINTF("\r\n\r\n\r\n *** ERROR: stack overflow ***\r\n\r\n\r\n\r\n");
//    }

//    uint32 stack_max_usage= stackMaxUsage();
//    const uint32 stack_size= stackSize();
//    uint32 stack_usage_percent= stack_max_usage*100/stack_size;

//    TP_PRINTF("\r\nmax stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);
//    if(stack_usage_percent>=70)
//    {   //debug build
//        TP_PRINTF("\r\n\r\n\r\n*** WARNING: stack usage trigger 70%, please increase stack size\r\n\r\n\r\n");
//        ASSERT(0);
//    }
//    
//    //test stack overflow
//    static int level=20;
//    stack_overflow_test(level++);
//    stack_max_usage= stackMaxUsage();
//    stack_usage_percent= stack_max_usage*100/stack_size;
//    TP_PRINTF("new stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);

//    /* print QP queue status */
//    DynamicAnalysis();
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
    return ret;
}

static QState MainApp_EnterIdleTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* now enter idle mode. */
    return Q_TRAN(&MainApp_Idle);
}

static QState MainApp_EnterStandbyTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_HANDLED();
    bool musicStatus = *(bool*)Setting_Get(SETID_MUSIC_DET);

    TP_PRINTF("mode=%d,musicStatus=%d\r\n", me->standbyMode, musicStatus);
    if(STANDBY_MODE_AUTO == me->standbyMode && musicStatus)
    {
        me->tickHandlers[TIMER_ID_ENTER_STANDBY_TIMEOUT].timer = TIMER_ID_ENTER_STANDBY_TIMEOUT_IN_MS;
    }
    else
    {
        ret = Q_TRAN(&MainApp_Standby);
    }

    return ret;
}

static QState MainApp_JackLowTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    return Q_TRAN(&MainApp_Standby);
}

static QState MainApp_StartupAmpTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_STARTUP_AMP_TIMEOUT].timer = INVALID_VALUE;
    AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_MUTE, FALSE);

    return Q_HANDLED();
}

static QState MainApp_ShutDownAmpTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_SHUTDOWN_AMP_TIMEOUT].timer = INVALID_VALUE;
    AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, TRUE, 0, 0);
    return Q_HANDLED();
}

static QState MainApp_DspMuteTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_DSPMUTE_TIMEOUT].timer = INVALID_VALUE;
    if(SYSTEM_STA_STANDBY != me->systemStatus)
    {
        AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, FALSE);
    }
    return Q_HANDLED();
}


QState MainApp_ActiveTickEvtHandler(cMainApp * const me, QEvt const * const e)
{
    uint8 i = 0;
    int32 *pTimer = NULL;
    for(i = 0; i < TIMER_ID_MAX; i++)
    {
        pTimer = &(me->tickHandlers[i].timer);
        if(*pTimer <= 0)
        {
            continue;
        }
        *pTimer -= MAINAPP_TIMEOUT_IN_MS;
        if(0 >= (*pTimer))
        {
            if(me->tickHandlers[i].timerHandler)
            {
                /* If any tick want to transit state, we handle state transit first,
                 * and pospone next tick handling
                 */
                QState trans_state= me->tickHandlers[i].timerHandler(me, e);
                if(trans_state != Q_UNHANDLED())
                {
                    return trans_state;
                }
            }
            else
            {
                ASSERT(0);
            }
        }
    }

    return Q_UNHANDLED();
}

