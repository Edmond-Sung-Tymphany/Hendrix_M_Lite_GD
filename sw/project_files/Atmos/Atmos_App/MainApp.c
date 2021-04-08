/**
*  @file      mainapp.c
*  @brief     Main application for ATMOS
*  @author    Viking Wang
*  @date      30-May-2016
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "bl_common.h"
#include "trace.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
#include "MainApp_commEvtHandler.h"
#include "MainApp.Config"


/*****************************************************************
 * Definition
 *****************************************************************/
#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME
#endif



/*****************************************************************
 * Global Variable
 *****************************************************************/
/* Internal evt queue */
static QEvt const *eventQue[MAINAPP_EVENT_Q_SIZE];



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
static void MainApp_ResetSettingToFactoryDef(cMainApp * const me);

/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/
/* Start function*/
void MainApp_StartUp( cPersistantObj *me)
{
     /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                            eventQue, Q_DIM(eventQue), MAIN_APP_ID);
}
/* Shut down function*/
void MainApp_ShutDown( cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    Application_Xtor((cApplication*)me);
}



/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(MainApp);
    QS_OBJ_DICTIONARY(MainApp_Active);

    (void)e; /* suppress the compiler warning about unused parameter */
    /* Subsrcribe to all the SIGS */
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);
    QActive_subscribe((QActive*)me, COMM_STATE_SIG);

    ASSERT(ArraySize(sourceHandlerList) == AUDIO_SOURCE_MAX);
    me->sourceHandler = sourceHandlerList;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    me->combinedKey = COM_KEY_INVALID;
    
    uint32 magicNum;
    magicNum = *(uint32*)(Setting_Get(SETID_MAGIC_NUMBER));
    if (magicNum != MAGIC_NUM_SETTING_ROM)
    {
        MainApp_ResetSettingToFactoryDef(me);
    }
    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU);
    me->audioSource = *(uint32*)(Setting_Get(SETID_AUDIO_SOURCE));    
    me->systemStatus = SYSTEM_STA_STANDBY_LOW;

#ifdef  HAS_IWDG
    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IwdgReloadActiveMode();

    RTC_Initialize();
    RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);
#endif
    
    return Q_TRAN(&MainApp_PowerInit);
}

/* PowerInit state
 * Purpose: system initialize before power up
 */
QState MainApp_PowerInit(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "\n\r(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, OFF_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TP_PRINTF("\n\r-------------------------------------------------------\n\r");
            TP_PRINTF("\n\r ATMOS SW ver: v%s, built on %s %s\n\r", PRODUCT_VERSION_MCU, __DATE__, __TIME__);
            TP_PRINTF("-------------------------------------------------------\n\r");
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "\n\r(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            me->systemStatus = SYSTEM_STA_ON;
            return Q_TRAN(&MainApp_Active);
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "\n\r(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Standby low (OFF) state
 * Purpose: let user think system power off
 */
QState MainApp_Off(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, OFF_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG keyEvent=%d, keyId=%d", e->sig, evt->keyEvent, evt->keyId);
  
            if(STANDBY_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent) || KEY_EVT_SHORT_PRESS == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_L_HOLD);
                }
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    return Q_TRAN(&MainApp_Active);
                }
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadOffMode();
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Powering Down state
 * Purpose: show LED
 */
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);            
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_TRANS_OFF);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( POWERING_DOWN_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                //MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt *pe = (LedStateEvt *)e;
            if(pe->patternId == TRANS_OFF_PAT_1)
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                return Q_TRAN(&MainApp_Off);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* system should never come to here in normal cases. */
            return Q_TRAN(&MainApp_Off);
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state
 * Purpose: normal state
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            
            /* TODO: maybe this boot up is after FEP upgrade, means ASE-TK boot up and wait long time.
             *       FEP may miss many events (ex. network-standby or ON), thus we need request SUE
             *       to send state event again
             */
            
            me->combinedKey = COM_KEY_INVALID;
            MainApp_SwitchMode(me, NORMAL_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( NORMAL_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
                MainApp_SwitchAudioSource(me, AUDIO_SOURCE_AUXIN);
            }
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            QState trans_state= MainApp_KeyHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            break;
        }
        case COMM_STATE_SIG:
        {
            QState trans_state= MainApp_commHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            break;
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(NULL != me->nextState)
            {
                QStateHandler* targetState = me->nextState;
                me->nextState = NULL;
                return Q_TRAN(targetState);
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    
    if(Q_UNHANDLED() != ret)
    {
        return ret;
    }
    else
    {
        return Q_SUPER(&QHsm_top);
    }
}

/* initial variable for factory reset */
static void MainApp_ResetSettingToFactoryDef(cMainApp * const me)
{
    uint32 getMagic = MAGIC_NUM_SETTING_ROM;
    uint8 vol = DEFAULT_VOLUME;
    uint32 source = AUDIO_SOURCE_DEFAULT;
    
    Setting_Set(SETID_MAGIC_NUMBER, &getMagic);      
    Setting_Set(SETID_VOLUME, &vol);               
    Setting_Set(SETID_AUDIO_SOURCE, &source);       
    SettingSrv_BookkeepingEx();                         
}