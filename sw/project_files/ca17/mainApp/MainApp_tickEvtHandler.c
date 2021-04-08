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
#include "AseNgSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"

static QState MainApp_AseNgBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedEnterShopModeTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedPowerInitTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_StackCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DoublePressResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DoublePressConnectTimeoutHandler(cMainApp * const me, QEvt const * const e);


/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_AseNgBootingTimeoutHandler},         //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler},   //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_DelayedEnterShopModeTimeoutHandler}, //TIMER_ID_DELAYED_ENTER_SHOP_MODE
    {0, (TickHandler)MainApp_DelayedPowerInitTimeoutHandler},     //TIMER_ID_DELAYED_POWER_INIT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},         //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},         //TIMER_ID_FACTORY_RESET_TIMEOUT
    {0, (TickHandler)MainApp_PowerDownTimeoutHandler},            //TIMER_ID_POWER_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_StackCheckTimeoutHandler},           //TIMER_ID_STACK_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_SysTempCheckTimeoutHandler},         //TIMER_ID_SYS_TEMP_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_DoublePressResetTimeoutHandler},     //TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT
    {0, (TickHandler)MainApp_DoublePressConnectTimeoutHandler},   //TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT

};



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static QState MainApp_AseNgBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_ASE_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    BSP_SoftReboot(); /* for release build */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_DelayedEnterShopModeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{            
    uint32 shopMode= TRUE;
    Setting_Set(SETID_SHOP_MODE, &shopMode);
    
    //We should go to Shop Mode Status directly (not via PowerInit), because PowerInit will re-init ASE
    return Q_TRAN(&MainApp_ShopMode);
}

static QState MainApp_DelayedPowerInitTimeoutHandler(cMainApp * const me, QEvt const * const e)
{            
    return Q_TRAN(&MainApp_PowerInit);
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
    Setting_Set(SETID_MAX_STACK_USAGE, &stack_max_usage);
    TP_PRINTF("\r\nmax stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);
    if(stack_usage_percent>=70)
    {   //debug build
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

static QState MainApp_SysTempCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;

    /* DSP is sometimes unstable after software upgrade
     */
#ifdef HAS_DSP_UNSTABLE_REBOOT
    bool isDspErr= FALSE;
    isDspErr= *(bool*)Setting_GetEx(SETID_IS_DSP_ERROR, &isDspErr);
    if( isDspErr )
    {
        ASSERT(0);
        MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    }
#endif

    int16 tempAmp= 0;
    tempAmp = *(int16*)Setting_GetEx(SETID_AMP_TEMP, &tempAmp);
    
    eTempLevel tempLevel= TL_NORMAL;
    tempLevel= *(eTempLevel*)Setting_GetEx(SETID_AMP_TEMP_LEVEL, &tempLevel);
    
    /* If tempLevel <= TL_WARN (temp > 80C), system is not allowed to powering up.
    *   so set tempLevel to TL_CRITICAL and shutdown system.
    */
    if(me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer > 0 && tempLevel <= TL_WARN)
    {
        tempLevel = TL_CRITICAL;
    }
    if (tempLevel != me->sysTempLevel)
    {
        switch (tempLevel)
        {
            case TL_CRITICAL:
            {
                TP_PRINTF("\r\n*** AmpTemp=%dC, temp level %d -> %d (CRITICAL) ==> Over-heat shutdown ***\r\n\r\n", tempAmp, me->sysTempLevel, tempLevel);
                MainApp_TurnOffConnLed(me);
                AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, TRUE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier

                /* Notify ASE to shutdown itself. If ASE hang because of hot temperature,
                 * FEP have no way to shutdown it
                 */
                AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_OFF);
                
                ret= Q_TRAN(&MainApp_PoweringDown);
                break;
            }
            case TL_SERIOUS:
            {
                TP_PRINTF("\r\n*** AmpTemp=%dC, temp level %d -> %d (SERIOUS) ==> shutdown amplifier ***\r\n\r\n", tempAmp, me->sysTempLevel, tempLevel);
                AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/TRUE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier
                break;
            }
            case TL_WARN:
            {
                TP_PRINTF("\r\n*** AmpTemp=%dC, temp level %d -> %d (WARN) ==> Reduce DSP gain and LED show error ***\r\n\r\n", tempAmp, me->sysTempLevel, tempLevel);
                AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/FALSE, /* NO USED*/0, /* NO USED*/0); //wakeup amplifier
                break;
            }
            case TL_NORMAL:
            {
                TP_PRINTF("\r\n*** AmpTemp=%dC, temp level %d -> %d (NORMAL) ==> LED back to normal ***\r\n\r\n", tempAmp, me->sysTempLevel, tempLevel);
                AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/FALSE, /* NO USED*/0, /* NO USED*/0); //wakeup amplifier
            }
            default:
                break;
        }
        
        me->sysTempLevel = tempLevel; 
        MainApp_UpdateProdLed(me);
    }
    
    return ret;
}


static QState MainApp_DoublePressResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* Note: The function will be performed if double press is NOT being triggered within 0.5second after first press. */   
    return Q_UNHANDLED(); //do not transit state
}



static QState MainApp_DoublePressConnectTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* Note: The function will be performed if double press is NOT being triggered within 0.5second after first press. */
    TP_PRINTF("Proto_FepAse_ReqResp_SOUND_SILENCE_TOGGLE\r\n");
    AseNgSrv_SendFepAseCmd(Proto_FepAse_ReqResp_SOUND_SILENCE_TOGGLE);
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

