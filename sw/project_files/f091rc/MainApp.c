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

#ifdef DEBUG_HW_SPI_API
#include "SpiDrv.h"

static cSpiDrv		adiDspSpiDrv;
#endif

#ifdef DEBUG_SW_I2C_API
#include "SWi2c_Drv.h"

static cSWi2cDrv_t  dbgSWi2cDrv;
#endif

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
    
#ifdef DEBUG_HW_SPI_API
	adiDspSpiDrv.pConfig = (tSpiDevice *) getDevicebyIdAndType(SPI2_DEV_ID, SPI_DEV_TYPE, NULL);
	SpiDrv_Ctor(&adiDspSpiDrv, adiDspSpiDrv.pConfig);
#endif

#ifdef DEBUG_SW_I2C_API
    dbgSWi2cDrv.pConfig = (stSWi2cDevice_t *) getDevicebyIdAndType(SW_I2C1_DEV_ID, SWI2C_DEV_TYPE, NULL);
    SWi2cDrv_Ctor(&dbgSWi2cDrv, dbgSWi2cDrv.pConfig);
#endif

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
//        printf("A");
#ifdef DEBUG_HW_SPI_API
        uint8_t spi_byte, spi_bytes[8];
        SpiDrv_WriteByte(&adiDspSpiDrv, 0x5a);
//      SpiDrv_WriteArray(&adiDspSpiDrv, "hello", 5);
//      SpiDrv_ReadByte(&adiDspSpiDrv, &spi_byte);  
//      SpiDrv_ReadArray(&adiDspSpiDrv, spi_bytes, 5);  
#endif
#ifdef DEBUG_SW_I2C_API
        uint8_t i2c_byte;
        uint8_t i2c_array[6]="abcdef";
        dbgSWi2cDrv.devReady = TRUE;
        SWi2cDrv_WriteByte(&dbgSWi2cDrv, 2, 0x66);
        SWi2cDrv_ReadArray(&dbgSWi2cDrv, 2, i2c_array, 3);
//        SWi2cDrv_ReadByte(&dbgSWi2cDrv, 2, &i2c_byte);
//        SWi2cDrv_DeviceAvailable(&dbgSWi2cDrv, 2);
//        SWi2cDrv_WriteByte(&dbgSWi2cDrv, 1, 0x55);
//        SWi2cDrv_WriteByte(&dbgSWi2cDrv, 2, 0x66);
//        SWi2cDrv_WriteArray(&dbgSWi2cDrv, i2c_array, 6);
#endif
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

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    case VOLUME_UP_KEY:
    {
        static int32 bt_state = 0;

        bt_state = !bt_state;
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
    {

    }
        break;
    default:
        break;
    }
}

static void MainApp_KeyVLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
    case VOLUME_UP_KEY:
    {

    }
        break;
    default:
        break;
    }
}

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    printf("MainApp_Active KEY_STATE_SIG keyId: %d  keyEvt: %d \n\r", evt->keyId, evt->keyEvent);
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
        MainApp_KeyVLHoldEvtAction(me, evt);
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
