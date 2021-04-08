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
#include "AllPlaySrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "IdleDlg.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

static void MainApp_SamBootingTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_WaitSamStableTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_UpgradingTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_SourceSwitchTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_LedDimTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_BlueToothPairingTimerHandler(cMainApp * const me, QEvt const * const e);
static void MainApp_BlueToothReconnectTimerHandler(cMainApp * const me, QEvt const * const e);
/*****************************************************************
 * Global Variable
 *****************************************************************/
extern tPatternData patternConfig[PAT_MAX_NUMBER];

/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[MAX_TIMER] =
{
    {0, (TickHandler)MainApp_SamBootingTimerHandler},           //SAM_BOOTING_TIMER
    {0, (TickHandler)MainApp_WaitSamStableTimerHandler},        //WAIT_SAM_STABLE_TIMER
    {0, (TickHandler)MainApp_UpgradingTimerHandler},            //UPGRADING_TIMER
    {0, (TickHandler)MainApp_SwitchChannelTimerHandler},        //SW_CH_TIMER
    {0, (TickHandler)MainApp_SourceSwitchTimerHandler},         //SRC_SW_TIMER
    {0, (TickHandler)MainApp_LedDimTimerHandler},               //LED_DIM_TIMER
    {0, (TickHandler)MainApp_BlueToothPairingTimerHandler},     //BT_PAIRING_TIMER
    {0, (TickHandler)MainApp_BlueToothReconnectTimerHandler},   //BT_RECONNECT_TIMER
};
/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}

static void MainApp_SamBootingTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    me->tickHandlers[SAM_BOOTING_TIMER].timer = 0;
    Nvm_StoreExceptionCode(EXCEPTION_CODE_SAM_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    ProjBsp_SoftReboot(); /* for release build */
}

/* This timer handler is just a workaround. It should be removed after Qualcomm fix below issue:
* Issue: [Line-in] The line-in music will be mute after playing several seconds.
* https://qualcomm-cdmatech-support.my.salesforce.com/5003000000aSmF7
*/
static void MainApp_WaitSamStableTimerHandler(cMainApp * const me, QEvt const * const e)
{
    me->nextState = (QStateHandler*)&MainApp_Active;
}

static void MainApp_UpgradingTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    me->tickHandlers[UPGRADING_TIMER].timer = 0;
    Nvm_StoreExceptionCode(EXCEPTION_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    ProjBsp_SoftReboot(); /* for release build */
}

static void MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* Unmute system after switch audio channel.*/
    AudioSrv_SendMuteReq((QActive *)me, FALSE);
    me->nextState = (QStateHandler*)(me->sourceHandler[me->audioSource].stateHandler);
}

static void MainApp_SourceSwitchTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* When switch source delay timeout, then go into the corresponding source state *
     * and ask SAM to switch mode if need.
     * In here, FALSE means a delay is NOT reqired before switching source.
     */
    me->nextState = MainApp_SwitchAudioSource(me, FALSE);
}

static void MainApp_LedDimTimerHandler(cMainApp * const me, QEvt const * const e)
{
    patternConfig[SOLID_PAT].color = me->sourceHandler[me->audioSource].dimColor;
    MainApp_SendLedReq(me, SOLID_PAT);
}

static void MainApp_BlueToothPairingTimerHandler(cMainApp * const me, QEvt const * const e)
{
    MAINAPP_DEBUG_MSG(" BT pairing is timeout now. System is going to network standby now!!!  \n");
    me->nextState = (QStateHandler*)&MainApp_PoweringDown;
}

static void MainApp_BlueToothReconnectTimerHandler(cMainApp * const me, QEvt const * const e)
{
    if(me->audioSource == BLUETOOTH)
    {
        MainApp_SendLedReq(me, BT_PAIRING_ENABLE_PAT);
        AllPlaySrv_BluetoothPairable(TRUE);
        me->tickHandlers[BT_PAIRING_TIMER].timer = MAINAPP_BT_PAIRING_TIMEOUT_IN_MS;
    }
}

void MainApp_ActiveTickEvtHandler(cMainApp * const me, QEvt const * const e)
{
    uint8 i = 0;
    int32 *pTimer = NULL;
    for(i = 0; i < MAX_TIMER; i++)
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
