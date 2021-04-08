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
#include "BnO_MainApp.h"

static void MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_LedDimTimerHandler(cMainApp * const me, QEvt const * const e);

/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_AseTkBootingTimeoutHandler},       //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},            //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_SwitchChannelTimerHandler},        //TIMER_ID_SW_CH
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},        //TIMER_ID_ASE_TK_FACTORY_REBOOT
};
/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static void MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 0;
    Nvm_StoreExceptionCode(REBOOT_CODE_ASE_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    BSP_SoftReboot(); /* for release build */
    //TODO: let system bootup and do not wait power key
}

static void MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0;
    Nvm_StoreExceptionCode(REBOOT_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    BSP_SoftReboot(); /* for release build */
    //TODO: let system bootup and do not wait power key
}

static void MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_SW_CH].timer = 0;
    /* Unmute system after switch audio channel.*/
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, FALSE);
    me->nextState = (QStateHandler*)(me->sourceHandler[me->audioSource].stateHandler);
}

static void MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = 0;
    BSP_SoftReboot(); 
}

static void MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = 0;
    ASSERT(0); /* for debug build */
    BSP_SoftReboot(); /* for release build */
}


void MainApp_ActiveTickEvtHandler(cMainApp * const me, QEvt const * const e)
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
            }
        }
    }
}

