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
#include "AseTkSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "IdleDlg.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

static QState MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e);

/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_AseTkBootingTimeoutHandler},   //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler},    //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},   //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_SwitchChannelTimerHandler},    //TIMER_ID_SW_CH
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},   //TIMER_ID_FACTORY_RESET_TIMEOUT
    {0, (TickHandler)MainApp_PowerDownTimeoutHandler},   //TIMER_ID_POWER_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_VolumeChangeTimeoutHandler},   //TIMER_ID_VOLUME_CHANGE_TIMEOUT
};



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static QState MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
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

static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* Unmute system after switch audio channel.*/
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, FALSE);
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
        //AseTkSrv_SendFepAseCmdAbsoluteVol(me->absoluteVol);
        me->isVolChanged = FALSE;
    }
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
                me->tickHandlers[i].timerHandler(me, e);
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

