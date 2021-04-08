/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        MainApp.c
@brief       Main application for MGT
@author      Dmitry Abdulov
@date        2014-12-11
@copyright (c) Tymphany Ltd. All rights reserved.

-------------------------------------------------------------------------------
*/

/*to do list*/
/* need to remove the extern with getDeviceId function after the GPIO struct is changed*/

#include "./MainApp_priv.h"
#include "projBsp.h"
#include "trace.h"
#include "pattern.h"

#include "ledsrv.h"

#define CAST_ME cMainApp * MainApp = (cMainApp *) me;

#ifndef MAINAPP_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif


/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[5];

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                     MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);
    /* Subscribe */
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
    QActive_subscribe((QActive*) me, AUDIO_STATE_SIG);
#ifdef HAS_INTERRUPT_WAKE_UP_KEY
    QActive_subscribe((QActive*) me, POWER_MCU_SLEEP_SIG);
#endif

}

void MainApp_ShutDown(cPersistantObj *me)
{
    Application_Xtor((cApplication*)me);
}


/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
QState MainApp_Initial(cMainApp * const me, QEvt const * const e)
{
    /* initial the default value for first power up or factory reset*/
    MainApp_InitialVariablesForFactoryReset(me);
    return Q_TRAN(&MainApp_Active);
}

QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    case MAINAPP_TIMEOUT_SIG:
    {
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state  - super state for "normal" behaviour */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        MainApp_SwitchMode(me, NORMAL_MODE);
        MainApp_InitialVariablesForPowerUp(me);            
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    case SYSTEM_MODE_RESP_SIG:
    {
 //       LedSrv_SetPatt((QActive*)me, RGB_LED, FADE_IN_0_5s_PATT);
        return Q_HANDLED();
    }
    case KEY_STATE_SIG:
    {
        MainApp_ParseKeyEvent(me, e);
        return Q_HANDLED();
    }
    case MAINAPP_TIMEOUT_SIG:
    {
        
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    case Q_EXIT_SIG:
    {
        QTimeEvt_disarm(TIME_EVT_OF(me));
        return Q_HANDLED();
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

/* powering down state, which is turning off the system power and have some delay*/
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    case MAINAPP_TIMEOUT_SIG:
    {
        PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
        return Q_HANDLED();
    }
    case SYSTEM_MODE_RESP_SIG:
    {
        return Q_TRAN(&MainApp_Standby);
    }
    case Q_EXIT_SIG:
    {
        QTimeEvt_disarm(TIME_EVT_OF(me));
        return Q_HANDLED();
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}


QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        return Q_HANDLED();
    }
    case POWER_MCU_SLEEP_SIG:
    {

        return Q_HANDLED();
    }
    case KEY_STATE_SIG:
    {
        KeyStateEvt *evt = (KeyStateEvt*)e;
        if ((evt->keyId == POWER_KEY) && (evt->keyEvent == KEY_EVT_SHORT_PRESS))
        {
            return Q_TRAN(&MainApp_PoweringUp);
        }

        return Q_HANDLED();
    }

    case Q_EXIT_SIG:
    {
        QTimeEvt_disarm(TIME_EVT_OF(me));
        return Q_HANDLED();
    }
    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}





/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

/* initial variable for power up system */
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me)
{

}

/* initial variable for factory reset */
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me)
{
    me->vol = DEFAULT_VOLUME;
    Setting_Set(SETID_VOLUME, &me->vol);
    uint8 maxVol = MAX_VOLUME;
    Setting_Set(SETID_MAX_VOLUME, &maxVol);
}

static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus)
{
    SystemStatusEvt *pe = Q_NEW(SystemStatusEvt, SYSTEM_STATE_SIG);
    pe->systemStatus = systemStatus;
    QF_PUBLISH(&pe->super, me);
}

static void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
    me->vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if(me->vol >= step)
    {
        me->vol-=step;
    }
    else
    {
        me->vol = 0;
    }
    AudioSrv_SetVolume(me->vol);
}

static void MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
    me->vol = *(uint8*)Setting_Get(SETID_VOLUME);
    uint8 maxVol = *(uint8*)Setting_Get(SETID_MAX_VOLUME);
    if((step + me->vol)<= maxVol)
    {
        me->vol+=step;
    }
    else
    {
        me->vol = maxVol;
    }
    AudioSrv_SetVolume(me->vol);
}

static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    default:
        break;
    }
}


#define MAX_VOL  (20)

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    case VOLUME_UP_KEY:
    {
#ifdef ASETK_ENABLE
        static uint8 vol = 10;
        AseTkCmdEvt* evt = Q_NEW(AseTkCmdEvt, ASE_TK_REQ_SIG);
        evt->aseTkCmd = ASE_TK_VOL_UP_CMD;
        evt->data = vol;
        SendToServer(ASETK_SRV_ID,(QEvt*)evt);
        if(++vol > MAX_VOL)
        {
            vol = 0;
        }
#else
        volatile static uint32 i = 0;
        ++i;
#endif
    }
        break;
    default:
        break;
    }
}

static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    case VOLUME_UP_KEY:
#ifdef ASETK_ENABLE
    {
        AseTkCmdEvt* evt = Q_NEW(AseTkCmdEvt, ASE_TK_REQ_SIG);
        evt->aseTkCmd = ASE_TK_PLAY_PAUSE_CMD;
        evt->data = 0;
        SendToServer(ASETK_SRV_ID,(QEvt*)evt);
    }
#endif
        break;
    default:
        break;
    }
}

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    TP_PRINTF("MainApp_Active KEY_STATE_SIG keyId: %d  keyEvt: %d \n\r", evt->keyId, evt->keyEvent);
    switch(evt->keyEvent)
    {
    case KEY_EVT_UP:
        break;
    case KEY_EVT_DOWN:
        break;
    case KEY_EVT_SHORT_PRESS:
        MainApp_KeySPressEvtAction(me, evt);
        break;
    case KEY_EVT_LONG_PRESS:
        MainApp_KeyLPressEvtAction(me, evt);
        break;
    case KEY_EVT_VERY_LONG_PRESS:
        break;
    case KEY_EVT_HOLD:
        MainApp_KeyHoldEvtAction(me, evt);
        break;
    case KEY_EVT_REPEAT:
        MainApp_KeyRepeatEvtAction(me, evt);
        break;
    case KEY_EVT_VERY_LONG_HOLD:
        break;
    default:
        break;
    }
}

static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    /*if necessary, add the handler for keys here */
    default:
        break;
    }
}
