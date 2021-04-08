/**
*  @file      main_app.c
*  @brief     Main application for BnO A9-MK3
*  @author    Daniel Qin
*  @date      17-Dec-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "bl_common.h" /* for bl_getStblStatus */
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
#include "MainApp_aseTkEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"
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



static QEQueue deferredMainAppReqQue;
static QEvt const *pDeferredMainAppReqQueSto[6];


/*****************************************************************
 * Function Implemenation
 *****************************************************************/


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
    QActive_subscribe((QActive*)me, ASE_TK_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);
#ifdef BnO_fs1
    QActive_subscribe((QActive*)me, POWER_BATT_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_WAKE_UP_SIG);
#endif
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);

    ASSERT(ArraySize(sourceHandlerList) == AUDIO_SOURCE_MAX);
    me->sourceHandler = sourceHandlerList;
    ASSERT(ArraySize(ledIndList) == LED_IND_ID_MAX);
    me->ledInds = ledIndList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    me->combinedKey = COM_KEY_INVALID;
    Setting_Set(SETID_SW_VER, PRODUCT_VERSION_MCU);
    me->audioSource = AUDIO_SOURCE_MAX;
    me->audioMode = AUDIO_MODE_NORMAL;
    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
    me->systemStatus = SYSTEM_STA_STANDBY_LOW;
    me->aseBtEnable = FALSE;
    me->aseBtleEnable = FALSE;
    me->absoluteVol = INVALID_VALUE;
    me->isTonePlaying = FALSE;
    me->musicPlaying = FALSE;
    me->asetkMute = FALSE;
    bool enableDspTuning = FALSE;
    Setting_Set(SETID_IS_DSP_TUNING, &enableDspTuning);
    me->sysTempLevel = TL_NORMAL;
    me->ampHealth = TRUE;

    Proto_Dsp_RequestAudioInput_AudioInput aseAudioInput = Proto_Dsp_RequestAudioInput_AudioInput_ASE;
    Setting_Set(SETID_ASE_AUDIO_INPUT, &aseAudioInput);

    IwdgInit(IWDG_Prescaler_256, IWDG_RLR_RL);
    IwdgReloadActiveMode();

    RTC_Initialize();
    RTC_SetUpWakeUpAlarm(IWDG_FEED_PERIOD_SEC);

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
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, LED_BOOTING_MODE); //next: SYSTEM_MODE_RESP_SIG
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);

            //Write version if stored version is wrong
            bl_writeVersion((void*)FEP_ADDR_FIRMWARE_VER, SW_MAJOR_VERSION, SW_MINOR_VERSION1, SW_MINOR_VERSION2, SW_MINOR_VERSION3);

            //Start stack usage check and never stop
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
            bool noMusicByDefault = FALSE;
            Setting_Set(SETID_MUSIC_SPDIF_DET, &noMusicByDefault);
            Setting_Set(SETID_AUXIN_PLAYING, &noMusicByDefault);

            char * pHwVer = (char *)Setting_Get(SETID_HW_VER);
            TP_PRINTF("\r\n-------------------------------------------------------\r\n");
            TP_PRINTF(TP_PRODUCT " HW ver: %s SW ver: v%s, built on %s %s\r\n", pHwVer, PRODUCT_VERSION_MCU, __DATE__, __TIME__);
            TP_PRINTF("-------------------------------------------------------\r\n");
            if (PowerDrv_IsHwSupported(pHwVer) == FALSE)
            {
                //TP_PRINTF("\r\n\r\n hwVersion(%s) is NOT supported, it should be %s \r\n", pHwVer, HARDWARE_VERSION_STRING);
#ifdef NDEBUG
                TP_PRINTF("Note: as HW version is not expected, so system can not power on in release SW build.  \r\n\r\n");
#else
                TP_PRINTF("Note: as HW version is not expected, so system will power on 5 seconds later. \r\n\r\n");
#endif
                PersistantObj_RefreshTick((cPersistantObj*)me, 5000); // LED indicate HW error for 5 seconds then power on system.
                MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
                return Q_HANDLED();
            }
            else
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                
#ifdef HAS_SHOP_MODE
                uint32 shopMode= *(uint32*)Setting_Get(SETID_SHOP_MODE);
                if( shopMode == SHOP_MODE_ENABLE )
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_POWERING_UP_PROD);
                    MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_POWERING_UP_CONN);
                }
                else
#endif
                {
                    MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
                }
                return Q_TRAN(&MainApp_AseNgBootingUp);
            }
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
            return Q_TRAN(&MainApp_AseNgBootingUp);
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
            me->audioSource = AUDIO_SOURCE_MAX;
            Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
            
            /* Notify PowerDrv to trigge sleep
             */
            IwdgReloadOffMode();
            QEvt* pEvt = Q_NEW(QEvt, POWER_MCU_SLEEP_SIG);
            SendToServer(POWER_SRV_ID, pEvt);
            
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG keyEvent=%d, keyId=%d", e->sig, evt->keyEvent, evt->keyId);

            if(STANDBY_KEY == evt->keyId)
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, STANDBY_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent) || KEY_EVT_SHORT_PRESS == (evt->keyEvent))
                {
                    TYM_SET_BIT(me->combinedKey, STANDBY_IN_L_HOLD);
                }       
                if(me->combinedKey == COM_KEY_POWER_ON)
                {
                    me->combinedKey = COM_KEY_INVALID;
                    return Q_TRAN(&MainApp_PowerInit);
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
            /* Stay in Powering down state 2 seconds to finish LED fading off. */
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = MAINAPP_POWER_DOWN_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, 2000);
            me->sysTempLevel = TL_NORMAL; // reset system temperature level
            MainApp_SwitchMode(me, POWERING_DOWN_MODE);
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
        case MAINAPP_TIMEOUT_SIG:
            {
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                return Q_TRAN(&MainApp_Off);
            }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_POWER_DOWN_TIMEOUT].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


/* Wait Mode
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_Wait(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* In WAIT_MODE, key server will be deactived. */
            MainApp_SwitchMode(me, WAIT_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( WAIT_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SAFETY_ALARM_OVERTEMPERATURE", e->sig);
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
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



/* Wait Upgrade Mode
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_WaitUpgrade(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            break;
        }
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseNgStateIndEvt* aseNgEvt = (AseNgStateIndEvt*) e;
            if( aseNgEvt->aseFepCmd.which_OneOf==Proto_Core_AseFepMessage_aseFepEvent_tag )
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d, type=%d", e->sig, aseNgEvt->aseFepCmd.which_OneOf, aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type);
                //It seems ASE-NG do not send UPDATE_FAILED or UPDATE_FINISHED , we must check BOOTED
                if( aseNgEvt->aseFepCmd.OneOf.aseFepEvent.has_type &&
                   ( aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_SW_UPDATE_FINISHED ||
                    aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_SW_UPDATE_FAILED ||
                    aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_BOOTED ))                  
                {
                  
#ifdef HAS_SHOP_MODE
                    uint32 shopMode= *(uint32*)Setting_Get(SETID_SHOP_MODE);
                    if( shopMode == SHOP_MODE_ENABLE )
                    {
                        return Q_TRAN(&MainApp_ShopMode);
                    }
                    else
#endif
                    {
                       /* ASEII-465: 
                        *   While test GoogleCast CRT test, after software upgrade, ASE will never send 
                        *   NETWORK_INFO again when upgrade finish. It cause LED always flash RED.
                        * Solution: 
                        *   Update to previous network status LED before jump to MainApp_Active
                        *   For normal software upgrade, ASE will send NETWORK_INFO to change LED.
                        */
                        MainApp_UpdateProdLed(me);  
                        MainApp_UpdateConnLed(me);                     
                        return Q_TRAN(&MainApp_Active);
                    }
                }
            }
            else
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d", e->sig, aseNgEvt->aseFepCmd.which_OneOf);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_UPGRADE_TIMEOUT].timer = 0; //disable timer
            break;
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Wait);
}



/* Wait Factory Reset Mode
 * Purpose: do not show key LED, and wait ASE-TK reboot
 */
QState MainApp_WaitFactoryReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = MAINAPP_FACTORY_RESET_TIMEOUT_MS;
            MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
            break;
        }
        case ASE_TK_STATE_SIG:
        {
            AseNgStateIndEvt* aseNgEvt = (AseNgStateIndEvt*) e;
            if( aseNgEvt->aseFepCmd.which_OneOf==Proto_Core_AseFepMessage_aseFepEvent_tag )
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d, type=%d", e->sig, aseNgEvt->aseFepCmd.which_OneOf, aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type);

                if( aseNgEvt->aseFepCmd.OneOf.aseFepEvent.has_type &&
                   ( aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_FACTORY_RESET_DONE ||
                    aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_BOOTED ))
                {
                    return Q_TRAN(&MainApp_Active);
                }
            }
            else if( aseNgEvt->aseFepCmd.which_OneOf==Proto_Core_AseFepMessage_aseFepReq_tag )
            {
                if( aseNgEvt->aseFepCmd.OneOf.aseFepReq.has_type &&
                    aseNgEvt->aseFepCmd.OneOf.aseFepReq.type==Proto_AseFep_ReqResp_POWER_REQUEST &&
                    aseNgEvt->aseFepCmd.OneOf.aseFepReq.data.powerRequest.has_type )
                {
                    if(aseNgEvt->aseFepCmd.OneOf.aseFepReq.data.powerRequest.type==Proto_System_PowerRequest_Type_POWER_OFF)
                    {
                        return Q_TRAN(&MainApp_PoweringDown);
                    }
                    else if(aseNgEvt->aseFepCmd.OneOf.aseFepReq.data.powerRequest.type==Proto_System_PowerRequest_Type_POWER_RESTART)
                    {
                        return  Q_TRAN(&MainApp_AseNgBootingUp);
                    }
                }
            }
            else
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG, which_OneOf=%d", e->sig, aseNgEvt->aseFepCmd.which_OneOf);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[TIMER_ID_FACTORY_RESET_TIMEOUT].timer = 0; //disable timer
            break;
        }
        default:
            break;
    }
    return Q_SUPER(&MainApp_Wait);
}


/* Soft Reset state*/
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, OFF_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RES", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( OFF_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                BSP_SoftReboot();
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
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


/* Ase Tk booting up state
 * Purpose: wait ASE-TK boot up
 */
QState MainApp_AseNgBootingUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;

            MainApp_SwitchMode(me, ASE_TK_BOOTING_MODE);
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
                eFepFirmwareStatus fw_status = bl_getFirmwareStatus();
                if(FEP_FIRMWARE_NEW == fw_status)
                {
                    bl_setFirmwareStatus(FEP_FIRMWARE_NORMAL);  // avoid skipping wait for ASE-TK boot-up for next power cycle.
                    return Q_TRAN(&MainApp_Active); // go to active state directly after upgraded firmware.
                }
                else
                {
                    MainApp_SendLedReq(me, LED_IND_ID_POWERING_UP);
                }
            }
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            //Boot up when ASE-NG is ready
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            AseNgStateIndEvt* aseNgEvt = (AseNgStateIndEvt*) e;
            if( aseNgEvt->aseFepCmd.which_OneOf==Proto_Core_AseFepMessage_aseFepEvent_tag &&
                aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type==Proto_AseFep_Event_Type_BOOTED )
            {
#ifdef HAS_SHOP_MODE
                uint32 shopMode= *(uint32*)Setting_Get(SETID_SHOP_MODE);
                if( shopMode == SHOP_MODE_ENABLE )
                {
                    return Q_TRAN(&MainApp_ShopMode);
                }
                else
#endif
                {   
                    return Q_TRAN(&MainApp_Active);
                }
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }

        }
        case KEY_STATE_SIG:
        {
            /* ASEII-379: 
             *   FEP can not power down ASE by RST/NPB pins, thus disable power down function on boot up.
             */
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            //KeyStateEvt *evt = (KeyStateEvt*)e;
            //if(STANDBY_KEY == (evt->keyId))
            //{
            //    if(KEY_EVT_DOWN == (evt->keyEvent))
            //    {
            //        TYM_SET_BIT(me->combinedKey, STANDBY_IN_HOLD);
            //    }
            //    else if(KEY_EVT_HOLD == (evt->keyEvent))
            //    {
            //        TYM_SET_BIT(me->combinedKey, STANDBY_IN_L_HOLD);
            //    }
            //    if(me->combinedKey == COM_KEY_POWER_ON)
            //    {
            //        me->combinedKey = COM_KEY_INVALID;
            //        return Q_TRAN(&MainApp_PoweringDown);
            //    }
            //}
            break;
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_ASE_TK_BOOTING_TIMEOUT].timer = INVALID_VALUE;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state
 * Purpose: normal state, ASE-TK should work now
 */
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            
            /* if we alreday have AUX-IN music on boot up, forcely trigger music detect update to ASE 
             */
            bool noMusicByDefault = FALSE;
            Setting_Set(SETID_AUXIN_PLAYING, &noMusicByDefault);

            /* TODO: maybe this boot up is after FEP upgrade, means ASE boot up and wait long time.
             *       FEP may miss many events (ex. network-standby or ON), thus we need request SUE
             *       to send state event again
             */
            me->combinedKey = COM_KEY_INVALID;
            me->systemStatus = SYSTEM_STA_STANDBY_LOW;
            me->playSource = Proto_Player_Data_Source_UNKNOWN;
            me->bIsBtConnected = FALSE;
            MainApp_SwitchMode(me, NORMAL_MODE);

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            me->tickHandlers[TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
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
                /* For conn LED, if turn off here, we need to wait 1s for netowrk change event.
                 *  durring waiting period, conn LED keep off (not good)
                 * For prod LED, we should wait system status event to turn it off
                 * => Thus we should not change LED here
                 */
                //MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                //MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
                me->audioMode = AUDIO_MODE_NORMAL;
                
                
                /* Now AudioDrv already update temp and level, then check overheat here
                 */
                int16 tempAmp= 0;
                tempAmp = *(int16*)Setting_GetEx(SETID_AMP_TEMP, &tempAmp); 
                if( tempAmp > MAINAPP_MAX_TEMP_ALLOW_BOOT )
                {
                    TP_PRINTF("\r\n*** Temp=%dC =>  do not allow boot when overheat (boot threshold: %dC) ***\r\n\r\n", tempAmp, MAINAPP_MAX_TEMP_ALLOW_BOOT);
                    MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            }
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            //AseNgSrv_SendSignalStatus(pAudioMusicStateEvt->hasMusicStream);
            //TP_PRINTF("\r\nReport to ASE: Aux-in music states =%d\r\n\r\n", pAudioMusicStateEvt->hasMusicStream);
            MainApp_MusicDetectSignalHandler(me, pAudioMusicStateEvt->hasMusicStream, pAudioMusicStateEvt->jackId);
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
        case ASE_TK_STATE_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ASE_TK_STATE_SIG", e->sig);
            QState trans_state= MainApp_AseNgMsgHandler(me,e);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case SAFETY_ALARM_OVERTEMPERATURE:
        {
            break;
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_DOUBLE_PRESS_CONNECT_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
#ifdef HAS_SHOP_MODE
            me->tickHandlers[TIMER_ID_DELAYED_ENTER_SHOP_MODE].timer = INVALID_VALUE;
#endif
            /* Reset the audio input to ASE if exit Active state */
            Proto_Dsp_RequestAudioInput_AudioInput aseAudioInput = Proto_Dsp_RequestAudioInput_AudioInput_ASE;
            Setting_Set(SETID_ASE_AUDIO_INPUT, &aseAudioInput);
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



#ifdef HAS_SHOP_MODE

QState MainApp_ShopMode(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SwitchMode(me, SHOP_MODE);
                    
            //Aux-in detect
            bool noMusicByDefault = FALSE;
            Setting_Set(SETID_AUXIN_PLAYING, &noMusicByDefault);
            Setting_Set(SETID_MUSIC_SPDIF_DET, &noMusicByDefault);
            
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( SHOP_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else
            {
                TP_PRINTF("\r\n\r\n\r\n[Enter shop mode]\r\n\r\n\r\n");
                PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
                
                /* Now AudioDrv already update temp and level, then check overheat here
                 */
                int16 tempAmp= 0;
                tempAmp = *(int16*)Setting_GetEx(SETID_AMP_TEMP, &tempAmp); 
                if( tempAmp > MAINAPP_MAX_TEMP_ALLOW_BOOT )
                {
                    TP_PRINTF("\r\n*** Temp=%dC =>  do not allow boot when overheat (boot threshold: %dC) ***\r\n\r\n", tempAmp, MAINAPP_MAX_TEMP_ALLOW_BOOT);
                    MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);
                    return Q_TRAN(&MainApp_PoweringDown);
                }
            
                //Key handler of normal mode need these setting to handle key
                me->isTonePlaying= FALSE;
                me->systemStatus= SYSTEM_STA_ON;
                me->combinedKey = COM_KEY_INVALID;
                me->isVolChanged = FALSE;
                me->isTonePlaying = FALSE;
                me->sysTempLevel = TL_NORMAL; // reset system temperature level
                me->asetkMute = FALSE;

                //Shop mode background LED
                MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
                MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_PROD);
                MainApp_SendLedReq(me, LED_IND_ID_SHOP_MODE_CONN);
        
                //Always unmute
                AudioSrv_SetAudio(AMP_SLEEP_MODE_ID, FALSE, /* NO USED*/0, /*No used*/0); //Amplifier always wakeup
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
                bool muteEnable= FALSE;
                Setting_Set(SETID_AMP_MUTE_STATUS, &muteEnable);
            
                //Volume
                me->absoluteVol= SHOP_MODE_DEFAULT_VOL;
                TP_PRINTF("\r\nSet default audio parameters: vol=%d\r\n\r\n", me->absoluteVol);
                AudioSrv_SetVolume(me->absoluteVol);

                //Line-in source
                me->audioSource = AUDIO_SOURCE_AUXIN;
                Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
                AudioSrv_SetChannel((QActive *)me, AUDIO_CHNANNEL_EXT_SOURCE);
                //Tone Touch back to default values
                double toneTouchVal= DSP_TONE_TOUCH_DEFAULT_Gx, toneTouchValK=DSP_TONE_TOUCH_DEFAULT_Kx;
                bool toneTouchEnabled= FALSE;
                Setting_Set(SETID_TONE_TOUCH_ENABLED, &toneTouchEnabled);
                Setting_Set(SETID_TONE_TOUCH_GX1,     &toneTouchVal);
                Setting_Set(SETID_TONE_TOUCH_GX2,     &toneTouchVal);
                Setting_Set(SETID_TONE_TOUCH_GY1,     &toneTouchVal);
                Setting_Set(SETID_TONE_TOUCH_GY2,     &toneTouchVal);
                Setting_Set(SETID_TONE_TOUCH_GZ,      &toneTouchVal);
                Setting_Set(SETID_TONE_TOUCH_K5,      &toneTouchValK);
                Setting_Set(SETID_TONE_TOUCH_K6,      &toneTouchValK);
                AudioSrv_SetAudio(DSP_WRITE_TONE_TOUCH_SETT_ID, TRUE, /*No used*/0, /*No used*/0);                
                AudioSrv_SetAudio(AUDIO_POS_SOUND_MODE_SETT_ID, TRUE, DSP_POSITION_DEFAULT, /*No used*/0);
                
                //Error protection
                me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS;
                me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = MAINAPP_STACK_CHECK_TIMEOUT_IN_MS;
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(RESET_KEY == evt->keyId && KEY_EVT_VERY_LONG_HOLD == evt->keyEvent)
            {
                MainApp_SendLedReq(me, LED_IND_ID_LP_AND_VLP);
                
                uint32 shopMode= FALSE;
                Setting_Set(SETID_SHOP_MODE, &shopMode);
                
                /* Wait LED_IND_ID_LP_AND_VLP finish (160ms), otherwise
                 * booting LED will have unsynchronized PROD and CONN led.
                 */
                me->tickHandlers[TIMER_ID_DELAYED_POWER_INIT].timer = 500; //must larger than LED flashing period (160ms)
                //return Q_TRAN(&MainApp_PowerInit);
            }
            return Q_HANDLED();
        }
        case TIMER_FEED_WD_SIG:
        {
            IwdgReloadActiveMode();
            return Q_HANDLED();
        }
        case ASE_TK_STATE_SIG:
        {
            /* SHop mode must handle [fw upgrade] and [factory reset], otherwise
             * ASE reboot and cause FEP do not output sound
             */
            AseNgStateIndEvt* aseNgEvt = (AseNgStateIndEvt*) e;
            if(aseNgEvt->aseFepCmd.which_OneOf==Proto_Core_AseFepMessage_aseFepEvent_tag &&
               aseNgEvt->aseFepCmd.OneOf.aseFepEvent.has_type)
            {
                if(aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type == Proto_AseFep_Event_Type_SW_UPDATE_STARTED)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_SW_UPDATING);
                    return Q_TRAN(&MainApp_WaitUpgrade);
                }
                else if(aseNgEvt->aseFepCmd.OneOf.aseFepEvent.type == Proto_AseFep_Event_Type_FACTORY_RESET_START)
                {
                    MainApp_SendLedReq(me, LED_IND_ID_FACTORY_RESET_TRIGGERED);
                    return Q_TRAN(&MainApp_WaitFactoryReset);
                }
            }
            break;            
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            //TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            QState trans_state = MainApp_ActiveTickEvtHandler(me,e);
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            if(trans_state!=Q_UNHANDLED()) {
                return trans_state;
            }
            else {
                return Q_HANDLED();
            }
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            me->combinedKey = COM_KEY_INVALID;
            me->tickHandlers[TIMER_ID_SYS_TEMP_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_STACK_CHECK_TIMEOUT].timer = INVALID_VALUE;
            me->tickHandlers[TIMER_ID_DELAYED_POWER_INIT].timer = INVALID_VALUE;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

#endif /* #ifdef HAS_SHOP_MODE */


