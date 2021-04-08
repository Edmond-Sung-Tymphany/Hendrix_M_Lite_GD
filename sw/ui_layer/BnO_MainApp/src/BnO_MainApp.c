/**
*  @file      main_app.c
*  @brief     Main application for polk_allplay
*  @author    Christopher Alexander
*  @date      15-Jan-2014
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
#include "projBsp.h" /* for  ProjBsp_SoftReboot()*/
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "IdleDlg.h"
#include "tym_qp_lib.h"
#include "BnO_MainApp.h"
#include "BnO_MainApp_util.h"
#include "BnO_MainApp_aseTkEvtHandler.h"
#include "BnO_MainApp_keyEvtHandler.h"
#include "BnO_MainApp_tickEvtHandler.h"

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
#ifdef HAS_IDLE_DELEGATE
static cIdleDlg IdleDlg = {{0}};
static cIdleDlg *pIdleDlg = NULL;
#endif //#ifdef HAS_IDLE_DELEGATE

/*****************************************************************
 * Function Implemenation
 *****************************************************************/
void BnO_MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source, bool forceChgSrc)
{
    TP_PRINTF("BnO_MainApp_SwitchAudioSource: source=%d, forceChgSrc=%d\r\n", source, forceChgSrc);
    
    //me->audioSource = BnO_MainApp_GetNextAvialableSource(me);
    if(source >= AUDIO_SOURCE_MAX || source <= AUDIO_SOURCE_MIN)
    {
        source = AUDIO_SOURCE_MIN;
    }
    if(forceChgSrc || me->audioSource!=source)
    {
        me->audioSource= source;
        
        /* Mute system before switch audio channel.*/
        //AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, TRUE);

        ASSERT(me->audioSource<AUDIO_SOURCE_MAX);
        BnO_MainApp_SendLedReq(me, me->sourceHandler[me->audioSource].ledInd);
        eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;

#ifndef DEBUG_QUICK_BOOT_NO_AUDIO
        AudioSrv_SetChannel((QActive *)me, audioChannel);
#endif
        Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    }
}

/* Power Off state*/
QState BnO_MainApp_Off(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            BnO_MainApp_SwitchMode(me, SLEEP_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            return Q_HANDLED();
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

/* Network Standby state
 * Basically it is the same with BnO_MainApp_Active()
 */
QState BnO_MainApp_NetworkStandby(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            BnO_MainApp_SwitchMode(me, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( STANDBY_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
                //BnO_MainApp_ResumeVol(me);
                /* Switch to WIFI by default.*/
                //BnO_MainApp_SwitchAudioSource(me, AUDIO_SOURCE_WIFI, /*forceSrcChg:*/TRUE);
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            BnO_MainApp_BtEvtHandler(me, e);            
            QState trans_state= BnO_MainApp_AseTkEvtHandler(me,e);       
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
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
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            BnO_MainApp_AseTkKeyHandler(me, e);
            BnO_MainApp_KeyHandler(me,e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


/* Powering Down state
 * This state is only for LED indicating, all key events are ignored
 */
QState BnO_MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);            
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            BnO_MainApp_SwitchMode(me, POWERING_DOWN_MODE);
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
                TP_PRINTF("BnO_MainApp_PoweringDown: (1)send LED_IND_ID_POWERING_DOWN\r\n");
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            TP_PRINTF("BnO_MainApp_PoweringDown: (2)get LED_STATE_SIG, goto BnO_MainApp_Off()\r\n");
            return Q_TRAN(&MainApp_Off);
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* system should never come to here in normal cases. */
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            MainApp_ActiveTickEvtHandler(me, e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            //Nvm_StoreExceptionCode(REBOOT_CODE_POWERING_DOWN_TIMEOUT);
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

/* Upgrade Mode */
QState BnO_MainApp_Upgrading(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /*set a 90 min timer for debug */
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* In UPGRADE_MODE, key server will be deactived. */
            BnO_MainApp_SwitchMode(me, UPGRADE_MODE); /* After switched mode, the signal SYSTEM_MODE_RESP_SIG is expected. */
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( UPGRADE_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
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
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Soft Reset state*/
QState BnO_MainApp_SoftReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /* In SOFTRESET_MODE, all the servers will be deactived EXCEPT power server */
            BnO_MainApp_SwitchMode(me, SOFTRESET_MODE); /* After switched mode, the signal SYSTEM_MODE_RESP_SIG is expected. */
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            /* After power server get and handle the signal POWER_MCU_RESET_SIG, *
            * system will soft reset */
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( SOFTRESET_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                /* Because there will be a pop noise when call to ProjBsp_SoftReboot(),
                 * so jump to bootloader to restart system in here
                 */
                ProjBsp_JumpToBootloader();
                //Application_SendSigToServer((cApplication *)me, POWER_SRV_ID, POWER_MCU_RESET_SIG);
            }
            return Q_HANDLED();
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

/* Ase Tk booting up state */
QState BnO_MainApp_AseTkBootingUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /* Led server should be active before entry SAM booting up state. */
            me->combinedKey = COM_KEY_INVALID;
            
            BnO_MainApp_SwitchMode(me, ASE_TK_BOOTING_MODE);
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = MAINAPP_ASE_TK_BOOTING_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            //me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 23*1000;
            //PersistantObj_RefreshTick((cPersistantObj*)me, 23*1000);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( ASE_TK_BOOTING_MODE != evt->modeId ) {
                //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //Boot up when ASE-TK is ready
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            AseTkStateIndEvt* aseTkEvt = (AseTkStateIndEvt*) e;
            if( aseTkEvt->aseFepCmd.which_OneOf==AseFepMessage_aseFepEvent_tag &&
                aseTkEvt->aseFepCmd.OneOf.aseFepEvent.event==AseFepEvent_Event_ASE_BOOTED )
            {
                return Q_TRAN(&MainApp_NetworkStandby);
            }
            return Q_HANDLED();
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
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(POWER_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, POWER_IN_L_HOLD);
                }
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state*/
QState BnO_MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            BnO_MainApp_SwitchMode(me, NORMAL_MODE);
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
                //BnO_MainApp_ResumeVol(me);
#ifdef HAS_IDLE_DELEGATE
                pIdleDlg = &IdleDlg;
                IdleDlg_Ctor(pIdleDlg, (QActive *)me);
#endif //#ifdef HAS_IDLE_DELEGATE
                
                /* switch to WIFI by default.*/
                BnO_MainApp_SwitchAudioSource(me, AUDIO_SOURCE_WIFI, /*forceSrcChg:*/TRUE);
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            BnO_MainApp_BtEvtHandler(me, e);            
            QState trans_state= BnO_MainApp_AseTkEvtHandler(me,e);

            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case KEY_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            BnO_MainApp_AseTkKeyHandler(me, e);
            BnO_MainApp_KeyHandler(me,e);
            return Q_HANDLED();
        }
#ifdef HAS_IDLE_DELEGATE
        case IDLE_TIMEOUT_SIG:
        {
            MAINAPP_DEBUG_MSG(" idleTimer is timeout now. System is going to standby now!!!  \n");
            return Q_TRAN(&MainApp_NetworkStandby);
        }
#endif // #ifdef HAS_IDLE_DELEGATE
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
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
#ifdef HAS_IDLE_DELEGATE
            if(pIdleDlg)
            {
                IdleDlg_Xtor(pIdleDlg);
                pIdleDlg = NULL;
            }
#endif // #ifdef HAS_IDLE_DELEGATE
            me->combinedKey = COM_KEY_INVALID;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

