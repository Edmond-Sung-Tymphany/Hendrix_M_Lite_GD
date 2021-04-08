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
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "IdleDlg.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

static QState MainApp_BtBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_AmpErrCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DoublePressTimeoutHandler(cMainApp * const me, QEvt const * const e);


#define CRITICAL_TEMP   (90)

/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_BtBootingTimeoutHandler},    //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler},    //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},    //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},    //TIMER_ID_FACTORY_RESET_TIMEOUT
    {0, (TickHandler)MainApp_PowerDownTimeoutHandler},       //TIMER_ID_POWER_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_VolumeChangeTimeoutHandler},    //TIMER_ID_VOLUME_CHANGE_TIMEOUT
    {0, (TickHandler)MainApp_StackCheckTimeoutHandler},      //TIMER_ID_STACK_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_AmpErrCheckTimeoutHandler},     //TIMER_ID_AMP_ERR_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_DoublePressTimeoutHandler},     //TIMER_ID_DOUDBLE_PRESS_TIMEOUT
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static QState MainApp_BtBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_BT_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    BSP_SoftReboot(); /* for release build */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS;
    if (me->isVolChanged == TRUE)
    {
       // To Do: set volume here
        me->isVolChanged = FALSE;
    }
    return Q_UNHANDLED(); //do not transit state
}

//void stack_overflow_test(int level)
//{
//    char ch[100];
//    ch[0]= level;
//    TP_PRINTF("stack_overflow_test(i=%d)\r\n", ch[0]);
//    if(level>0)
//    {
//        stack_overflow_test(level-1);
//    }
//}

static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
    bool stackOverflow= stackOverflowCheck();
    ASSERT(!stackOverflow);
    if( stackOverflow )
    {
        TP_PRINTF("\r\n\r\n\r\n *** ERROR: stack overflow ***\r\n\r\n\r\n\r\n");
        MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    }

    uint32 stack_max_usage= stackMaxUsage();
    const uint32 stack_size= stackSize();
    uint32 stack_usage_percent= stack_max_usage*100/stack_size;
    // To Do: Set MAX STACK USAGE here
    TP_PRINTF("\r\nmax stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);
    if(stack_usage_percent>=70)
    {
        //debug build
        TP_PRINTF("\r\n\r\n\r\n*** WARNING: stack usage trigger 70%, please increase stack size\r\n\r\n\r\n");
        ASSERT(0);
    }

//    //test stack overflow
//    static int level=20;
//    stack_overflow_test(level++);
//    stack_max_usage= stackMaxUsage();
//    stack_usage_percent= stack_max_usage*100/stack_size;
//    TP_PRINTF("new stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);

    /* print QP queue status */
    DynamicAnalysis();
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_AmpErrCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    //Amplifier health checking
    bool ampHealth= TRUE;
    me->tickHandlers[TIMER_ID_AMP_ERR_CHECK_TIMEOUT].timer = MAINAPP_AMP_ERR_CHECK_TIMEOUT_IN_MS;
    // To Do: get amp health status form setting here

    if (me->ampHealth != ampHealth)
    {
        if (!ampHealth)
        {
            TP_PRINTF("\r\n\r\n\r\n*** Amplifier failture shutdown!!! ***\r\n\r\n\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/TRUE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier
        }
        else
        {
            TP_PRINTF("\r\n\r\n\r\n*** Wake up Amplifier ***\r\n\r\n\r\n");
            MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/FALSE, /* NO USED*/0, /* NO USED*/0); //wakeup amplifier
        }
        me->ampHealth = ampHealth;
    }
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DoublePressTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* Note: The function will be performed if double press is NOT being triggered within 0.5second after first press. */
    TP_PRINTF("FepAseCommand_Command_SOUND_SILENCE_TOGGLE\r\n");
	// To Do: operation of double press time out
    return Q_UNHANDLED(); //do not transit state
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
                if(trans_state!=Q_UNHANDLED())
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

