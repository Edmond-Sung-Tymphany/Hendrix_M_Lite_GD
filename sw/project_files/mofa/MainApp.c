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
#include "MainApp_util.h"
#include "MainApp_allplayEvtHandler.h"
#include "MainApp_keyEvtHandler.h"
#include "MainApp_tickEvtHandler.h"

/*****************************************************************
 * Definition
 *****************************************************************/

#define AUX_IN_STATUS_BITMASK       AUXIN_JACK
#define RCA_IN_STATUS_BITMASK       RCA_IN_JACK
#define OPT_IN_STATUS_BITMASK       SPDIF_IN_JACK

#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME
#endif

extern tPatternData patternConfig[PAT_MAX_NUMBER];

extern eCombinedKey MainApp_CombinedKey;
/*****************************************************************
 * Global Variable
 *****************************************************************/

/* Internal evt queue */
static QEvt const *eventQue[10];
static cIdleDlg IdleDlg = {{0}};
static cIdleDlg *pIdleDlg = NULL;

static eAudioSource currentSource = MAX_SOURCE; /* the playing source */
static tSourceHandlerList sourceHandlerList[MAX_SOURCE] = { /* the member order of the array should be changed accordign to eAudioSource */
    {(QStateHandler *)&MainApp_AllPlay_APMode,       "Allplay",          AUDIO_CHANNEL_I2S_2,        AUDIO_CHANNEL_I2S_2,               GREEN,  DIM_GREEN,   TRUE},
#ifdef HAS_ALLPLAY_DIRECTMODE
    {(QStateHandler *)&MainApp_AllPlay_DirectMode,   "Allplay",          AUDIO_CHANNEL_I2S_2,        AUDIO_CHANNEL_I2S_2,               PURPLE, DIM_PURPLE,  TRUE},
#endif
    {(QStateHandler *)&MainApp_AudioJackIn,          "allplay:linein",   AUDIO_CHANNEL_AUXIN_DIRECT, AUDIO_CHANNEL_AUXIN_MULTIROOM,     AMBER,  DIM_AMBER,   FALSE},
    {(QStateHandler *)&MainApp_AudioJackIn,          "allplay:linein",   AUDIO_CHANNEL_RCA_IN_DIRECT,AUDIO_CHANNEL_RCA_IN_MULTIROOM,    WHITE,  DIM_WHITE,   FALSE},
    {(QStateHandler *)&MainApp_Bluetooth,            "allplay:bluetooth",AUDIO_CHANNEL_I2S_2,        AUDIO_CHANNEL_I2S_2,               BLUE,   DIM_BLUE,    TRUE},
    {(QStateHandler *)&MainApp_AudioJackIn,          "allplay:linein",   AUDIO_CHANNEL_OPT_IN_DIRECT,AUDIO_CHANNEL_OPT_IN_MULTIROOM,    RED,    DIM_RED,     FALSE}  //for optical in
};
/*****************************************************************
 * Function Implemenation
 *****************************************************************/

QStateHandler* MainApp_SwitchAudioSource(cMainApp * const me, bool bDelay)
{
    QStateHandler *ret = NULL;

    /* According to MOF allplay UI, the invalid source should be skipped when switching source.*/
    me->audioSource = MainApp_GetNextAvialableSource(me);
    /* If a delay is required, a timer will be setup and Led will indicate *
     * the corrosponding source firstly.
     * Then when the timer is timeout, go into the corresponding source state *
     * and ask SAM to switch mode if need.*/
    if(bDelay)
    {
        patternConfig[SOLID_PAT].color = (me->sourceHandler[me->audioSource]).sourceColor;
        MainApp_SendLedReq(me, SOLID_PAT);
        me->tickHandlers[SRC_SW_TIMER].timer = MAINAPP_SOURCE_SWITCH_DELAY_IN_MS;
    }
    else if(me->audioSource != currentSource)
    {
        /* Mute system before switch audio channel.*/
        AudioSrv_SendMuteReq((QActive *)me, TRUE);

        /*Note:
         * If the speaker is part of a grouped we use the multiroom routing: Input -> DSP(bypass) -> SAM -> DSP -> AMP
         * when the speaker is single, we use the direct approach: Input -> DSP  -> AMP
         */
        AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
        eAudioChannel audioChannel = me->sourceHandler[me->audioSource].audioChannel;
        if(TRUE == allPlayInfo->bIsGrouped) /* the speaker is in a allplay group. */
        {
            audioChannel = me->sourceHandler[me->audioSource].multiRoomAudioChannel;
         }
        AudioSrv_SendSwitchChannelReq((QActive *)me, audioChannel);

        /*
         * When AllPlay audio source hijack other audio sources by playing a song in Jukebox,
         * the source setting in SAM will be changed automaticlly. If MCU send the change
         * source request to SAM again, SAM will change source again and stop the curent music.
         * So in this case, MCU should NOT send the change source request again.
         */
        if((ALLPLAY_PLAYER_STATE_PLAYING == allPlayInfo->ePlayerState) &&
           (ALLPLAY_AP == allPlayInfo->contentSource) && (ALLPLAY_AP_MODE == me->audioSource))
        {
            MAINAPP_DEBUG_MSG(" AllPlay audio source hijack other audio sources by playing a song in Jukebox. \n");
        }
        else
        {
            MainApp_SetExternalSource(me);
        }

        Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
        currentSource = me->audioSource;
    }
    else
    {
        ret = ((QStateHandler*)me->sourceHandler[me->audioSource].stateHandler);
    }
    return ret;
}

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
    QActive_subscribe((QActive*)me, POWER_STATE_SIG);
    QActive_subscribe((QActive*)me, ALLPLAY_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_WAKE_UP_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);

    me->sourceHandler = sourceHandlerList;
    me->tickHandlers  = (tTickHandlerList *)MainApp_GetTickHandlerList();
    return Q_TRAN(&MainApp_Sleep);
}

/* Standby state*/
QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_CombinedKey = INVILAD_COM_KEY;
            MainApp_SwitchMode(me, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( STANDBY_MODE!=evt->modeId ) {
                //ASSERT(0);
            }
            else
            {
                MainApp_SendLedReq(me, STANDBY_PAT);
            }
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            AudioStateEvt* pAudioStateEvt = (AudioStateEvt*) e;
            if((pAudioStateEvt->IsJackIn == TRUE) && (AUXIN_JACK == pAudioStateEvt->jackId))
            {
                me->audioSource = AUXIN;
                return Q_TRAN(&MainApp_SourceSwitching);
            }
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;

            /* if SAM is in updating mode, mainApp should switch to upgrading state. */
            if((ALLPLAY_STATE_SYSTEM_MODE_CHANGED == allPlayEvt->allPlayState) &&
              (ALLPLAY_SYSTEM_MODE_UPDATING ==allPlayEvt->payload.currentSystemMode))
            {
                MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_UPDATING);
                return Q_TRAN(&MainApp_Upgrading);
            }

            /* Connect to speaker using paired device will wake up system. */
            if(ALLPLAY_STATE_BT_DEVICE_CONNECTED == allPlayEvt->allPlayState)
            {
                me->audioSource = BLUETOOTH;
                return Q_TRAN(&MainApp_SourceSwitching);
            }

            /* Streaming allplay music will wake up system. */
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if((ALLPLAY_STATE_PLAYER_PLAYING == allPlayEvt->allPlayState) &&
               (ALLPLAY_AP == allPlayInfo->contentSource))
            {
                me->audioSource = ALLPLAY_AP_MODE;
                return Q_TRAN(&MainApp_SourceSwitching);
            }

            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(POWER_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_UP == (evt->keyEvent) || KEY_EVT_HOLD == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
                }
                if(MainApp_CombinedKey == POWER_ON_TRIGGER)
                {
                    /* the color of POWER_UP_DOWN_PAT should be same as the current audio source color. */
                    patternConfig[POWER_UP_DOWN_PAT].color = me->sourceHandler[me->audioSource].sourceColor;
                    MainApp_SendLedReq(me, POWER_UP_DOWN_PAT); //next: LED_STATE_SIG-step2
                    MainApp_CombinedKey = INVILAD_COM_KEY;
                    return Q_TRAN(&MainApp_Active);
                }
            }

            if(COMB_KEY_EVT == (evt->keyEvent))
            {
                if(COMB_KEY_ID_1 == (evt->keyId))
                {
                    patternConfig[DISPLAY_SWV_START_PAT].nextPattern = APP_MAJOR_VER_PAT;
                    MainApp_SendLedReq(me, DISPLAY_SWV_START_PAT);
                }
                else if(COMB_KEY_ID_3 == (evt->keyId))
                {
                    uint8 bootLoader_ver[4];
                    NvmDrv_ReadWord(NVM_STORAGE_ADDR_BOOTLOADER_VER_CHARS, (uint32 *)bootLoader_ver);
                    /* Each digit of version should not larger than 9. */
                    /* //TODO: Wesley_test  temp disable
                    patternConfig[BOOT_LOADER_MAJOR_VER_PAT].repeatNumber =
                        (bootLoader_ver[0]-'0')>9? 0:(bootLoader_ver[0]-'0');
                    patternConfig[BOOT_LOADER_MINOR_VER_PAT1].repeatNumber =
                        (bootLoader_ver[2]-'0')>9? 0:(bootLoader_ver[2]-'0');
                    patternConfig[BOOT_LOADER_MINOR_VER_PAT2].repeatNumber =
                        (bootLoader_ver[3]-'0')>9? 0:(bootLoader_ver[3]-'0');
                    patternConfig[DISPLAY_SWV_START_PAT].nextPattern = BOOT_LOADER_MAJOR_VER_PAT;
                    MainApp_SendLedReq(me, DISPLAY_SWV_START_PAT);
                    */
                }
            }
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

/* Sleep state*/
QState MainApp_Sleep(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            me->allplayMaxVol = ALLPLAY_MAX_VOLUME;
            MainApp_SwitchMode(me, SLEEP_MODE); //next: SYSTEM_MODE_RESP_SIG
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( SLEEP_MODE != evt->modeId ) {
                //ASSERT(0);
            }
            else
            {
                me->audioSource = *(uint8 *)Setting_Get(SETID_AUDIO_SOURCE);
                if((!Setting_IsReady(SETID_AUDIO_SOURCE)) ||
                    (MAX_SOURCE <= me->audioSource))
                {/* If audio source is not set, set it to ALLPLAY_AP_MODE for the first time */
                    me->audioSource = ALLPLAY_AP_MODE;
                    Setting_Set(SETID_AUDIO_SOURCE, &me->audioSource);
                }
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "me->audioSource:%d \n", me->audioSource);

                /* ignore power key detection when after firmware upgrade and factory reset. */
                uint32 nvm_ignore_pwr_key= 0;
                bool nvm_ret= NvmDrv_ReadWord(NVM_STORAGE_ADDR_IGNORE_PWR_KEY, &nvm_ignore_pwr_key);
                if(nvm_ret && nvm_ignore_pwr_key==0x1) //when after firmware upgrade
                {
                    NVM_STORAGE_VALUE_CLEAR(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                    return Q_TRAN(&MainApp_SamBootingUp);
                }
                //Application_SendSigToServer((cApplication *)me, POWER_SRV_ID, POWER_MCU_SLEEP_SIG);
            }
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            AudioStateEvt* pAudioStateEvt = (AudioStateEvt*) e;
            if((pAudioStateEvt->IsJackIn == TRUE) && (AUXIN_JACK == pAudioStateEvt->jackId))
            {
                me->audioSource = AUXIN;
            }
            else if((pAudioStateEvt->IsJackIn == FALSE) && (AUXIN_JACK == pAudioStateEvt->jackId))
            {
                me->audioSource = ALLPLAY_AP_MODE;
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(POWER_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_UP == (evt->keyEvent) || KEY_EVT_HOLD == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
                }
                if(MainApp_CombinedKey == POWER_ON_TRIGGER)
                {
                    MainApp_CombinedKey = INVILAD_COM_KEY;
                    return Q_TRAN(&MainApp_SamBootingUp);
                }
            }

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

/* Powering Down state*/
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            PersistantObj_RefreshTick((cPersistantObj*)me, 4000); /*set a 10 sec timer for debug */
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
                /* the color of POWER_UP_DOWN_PAT should be same as the current audio source color. */
                patternConfig[POWER_UP_DOWN_PAT].color = me->sourceHandler[me->audioSource].sourceColor;
                MainApp_SendLedReq(me, POWER_UP_DOWN_PAT); //next: LED_STATE_SIG-step2
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            /* system will not go to stanby/sleep mode until*
             * completed showing LED indication for powering down */
            if( POWER_UP_DOWN_PAT == pLedEvt->patternId ) // step1
            {
                return Q_TRAN(&MainApp_Standby);
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* system should never come to here in normal cases. */
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            Nvm_StoreExceptionCode(EXCEPTION_CODE_POWERING_DOWN_TIMEOUT);
            ASSERT(0); /* for debug build */
            return Q_TRAN(&MainApp_Standby);/* for release build */
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
QState MainApp_Upgrading(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /*set a 90 min timer for debug */
            me->tickHandlers[UPGRADING_TIMER].timer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            /* In UPGRADE_MODE, key server will be deactived. */
            MainApp_SwitchMode(me, UPGRADE_MODE); /* After switched mode, the signal SYSTEM_MODE_RESP_SIG is expected. */
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
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            /* The result of SAM updating is expected. *
            * if SAM updating is successful, mainApp will switch to soft-reset state. *
            * TODO: if SAM updating is failed, mainApp should switch to active state. */
            if( allPlayEvt->allPlayState == ALLPLAY_STATE_SAM_FIRMWARE_UPDATE_SUCCESSFUL )
            {
                NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                return Q_TRAN(&MainApp_SoftReset);
            }
            else if(allPlayEvt->allPlayState == ALLPLAY_STATE_SAM_FIRMWARE_UPDATE_FAILED)
            {
                return Q_TRAN(&MainApp_Active);
            }
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
            me->tickHandlers[UPGRADING_TIMER].timer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Soft Reset state*/
QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /* delay 4sec before softreset to avoid corrupt sam's flash.*/
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS); /* After setup a timer, the signal MAINAPP_TIMEOUT_SIG is expected. */
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* In SOFTRESET_MODE, all the servers will be deactived EXCEPT power server */
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            MainApp_SwitchMode(me, SOFTRESET_MODE); /* After switched mode, the signal SYSTEM_MODE_RESP_SIG is expected. */
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
                ProjBsp_SoftReboot();
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

/* SAM booting up state */
static QState MainApp_SamBootingUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /* Led server should be active before entry SAM booting up state. */
            MainApp_CombinedKey = INVILAD_COM_KEY;
            MainApp_SendLedReq(me, SAM_BOOTING_UP_PAT);
            MainApp_SwitchMode(me, SAM_BOOTING_MODE);
            me->tickHandlers[SAM_BOOTING_TIMER].timer = MAINAPP_SAM_BOOTING_TIMEOUT_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( SAM_BOOTING_MODE != evt->modeId ) {
                //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            AudioStateEvt* pAudioStateEvt = (AudioStateEvt*) e;
            if((pAudioStateEvt->IsJackIn == TRUE) && (AUXIN_JACK == pAudioStateEvt->jackId))
            {
                me->audioSource = AUXIN;
            }
            else if((pAudioStateEvt->IsJackIn == FALSE) && (AUXIN_JACK == pAudioStateEvt->jackId))
            {
                me->audioSource = ALLPLAY_AP_MODE;
            }
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            if(ALLPLAY_STATE_MCU_SAM_CONNECTED == allPlayEvt->allPlayState)
            {
               /* This timer handler is just a workaround. After Qualcomm fix below issue, remove the timer and
                * just to MainApp_Active directly:
                * Issue: [Line-in] The line-in music will be mute after playing several seconds.
                * https://qualcomm-cdmatech-support.my.salesforce.com/5003000000aSmF7
                */
                me->tickHandlers[WAIT_SAM_STABLE_TIMER].timer = MAINAPP_WAIT_SAM_STABLE_TIMEOUT_IN_MS;
                /* If audio source is not bluetooth, disable bluetooth to avoid bluetooth reconnect.*/
                if(me->audioSource != BLUETOOTH)
                {
                    AllPlaySrv_BluetoothEnable(FALSE);
                }
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
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(POWER_KEY == (evt->keyId) || POWER_IR_KEY == (evt->keyId))
            {
                if(KEY_EVT_DOWN == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
                }
                else if(KEY_EVT_HOLD == (evt->keyEvent))
                {
                    TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
                }
                if(MainApp_CombinedKey == POWER_ON_TRIGGER)
                {
                    MainApp_CombinedKey = INVILAD_COM_KEY;
                    MainApp_SendLedReq(me, OFF_PATT);
                    /* the color of POWER_UP_DOWN_PAT should be same as the current audio source color. */
                    patternConfig[POWER_UP_DOWN_PAT].color = GREEN;
                    MainApp_SendLedReq(me, POWER_UP_DOWN_PAT); //next: LED_STATE_SIG-step2
                    return Q_TRAN(&MainApp_Sleep);
                }
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[SAM_BOOTING_TIMER].timer = 0;
            me->tickHandlers[WAIT_SAM_STABLE_TIMER].timer = 0;
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state*/
QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_CombinedKey = INVILAD_COM_KEY;
            MainApp_SwitchMode(me, NORMAL_MODE);
            me->tickHandlers[LED_DIM_TIMER].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
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
                MainApp_ResumeVol(me);
                pIdleDlg = &IdleDlg;
                IdleDlg_Ctor(pIdleDlg, (QActive *)me);
                /* switch to ALLPLAY_AP_MODE by default.*/
                return Q_TRAN(&MainApp_SourceSwitching);
            }
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            AudioStateEvt* pAudioStateEvt = (AudioStateEvt*) e;
            if((pAudioStateEvt->IsJackIn == TRUE) && (AUXIN_JACK == pAudioStateEvt->jackId)
                    && (me->audioSource != AUXIN))
            {
                me->audioSource = AUXIN;
                return Q_TRAN(&MainApp_SourceSwitching);
            }
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            MainApp_AllPlayStateHandler(me,e);
            MainApp_BTEvtHandler(me, e);

            me->tickHandlers[LED_DIM_TIMER].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            MainApp_AllPlayKeyHandler(me, e);
            MainApp_KeyHandler(me,e);
            return Q_HANDLED();
        }
        case IDLE_TIMEOUT_SIG:
        {
            MAINAPP_DEBUG_MSG(" idleTimer is timeout now. System is going to standby now!!!  \n");
            return Q_TRAN(&MainApp_PoweringDown);
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
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            if(pIdleDlg)
            {
                IdleDlg_Xtor(pIdleDlg);
                pIdleDlg = NULL;
            }
            currentSource = MAX_SOURCE;
            MainApp_CombinedKey = INVILAD_COM_KEY;
            /* If allplay music is playing, stop it before exit. */
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if((ALLPLAY_PLAYER_STATE_STOPPED != allPlayInfo->ePlayerState)||
                (ALLPLAY_PLAYER_STATE_PAUSED != allPlayInfo->ePlayerState))
            {
                AllPlaySrv_SendCmd(ALLPLAY_CMD_PLAY_PAUSE);
            }
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Source Switching state*/
QState MainApp_SourceSwitching(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            /* Because SAM is difficult to switch mode sensitively. *
             * Thus a delay is required to avoid asking SAM to switch mode continually. */
            MainApp_SwitchAudioSource(me, TRUE); /*In here, TRUE means a delay is reqired before switching source. */
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if(KEY_EVT_SHORT_PRESS == (evt->keyEvent) &&
              (SWITCH_SOURCE_KEY == (evt->keyId) || SOURCE_SWITCH_IR_KEY == (evt->keyId)))
            {
                me->audioSource++;
                if(MAX_SOURCE == me->audioSource)
                {
                    me->audioSource = 0;
                }
                /* Because SAM is difficult to switch mode sensitively. *
                * Thus a delay is required to avoid asking SAM to switch mode continually. */
                MainApp_SwitchAudioSource(me, TRUE); /*In here, TRUE means a delay is reqired before switching source. */
                return Q_HANDLED();
            }
            break;
        }
        case AUDIO_SWITCH_CHANNEL_RESP_SIG:
        {
            /* Set a timer after switch channel to unmute system
             * Note: the unmute delay should be long enough to avoid pop noise
             * when switch audio channel.
             */
            me->tickHandlers[SW_CH_TIMER].timer = MAINAPP_DELAY_AFTER_SWITCH_CHANNEL_IN_MS;
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            /* ALLPLAY_STATE_SIG should be ignored when switching source. */
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            if(ALLPLAY_STATE_BT_DISABLE == allPlayEvt->allPlayState)
            {
                break;
            }
            else
            {
                return Q_HANDLED();
            }
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            me->tickHandlers[LED_DIM_TIMER].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}

/* Auxin state*/
static QState MainApp_AudioJackIn(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            patternConfig[SOLID_PAT].color = me->sourceHandler[me->audioSource].sourceColor;
            MainApp_SendLedReq(me, SOLID_PAT);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_SHORT_PRESS == evt->keyEvent) &&
                ((evt->keyId)== NEXT_KEY || (evt->keyId)== PREV_KEY))
            {
                return Q_HANDLED();
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}

/* AllPlay AP mode state*/
static QState MainApp_AllPlay_APMode(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if(TRUE == allPlayInfo->bAllplayConnected) /* MCU is  connected to SAM */
            {
                uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
                uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);
                AllPlaySrv_SetVol(allplayVol);
                MainApp_DisplayAllplaySystemMode(me, allPlayInfo->eSystemMode);
            }
            else
            {
                MainApp_SendLedReq(me, SAM_BOOTING_UP_PAT);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if((ALLPLAY_NETWORK_WIFI != allPlayInfo->networkType) ||
              (ALLPLAY_SYSTEM_MODE_CONFIGURED != allPlayInfo->eSystemMode))
            {
                me->tickHandlers[LED_DIM_TIMER].timer = 0;
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}

#ifdef HAS_ALLPLAY_DIRECTMODE
/* AllPlay direct mode state*/
static QState MainApp_AllPlay_DirectMode(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            MainApp_SendLedReq(me, DIRECT_INIT_PAT);
            AllPlaySrv_enableDirectMode(TRUE);
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            /*
             * System mode should be direct mode but not others when already in
             * direct mode. So ignore the event if the payload of the mode changed
             * event is not direct mode.
             * So actually, if muc received these events, that means there is
             * something wrong with SAM module when swtching modes.
             */
            if((ALLPLAY_STATE_SYSTEM_MODE_CHANGED == allPlayEvt->allPlayState) &&
                (ALLPLAY_SYSTEM_MODE_DIRECT != allPlayEvt->payload.currentSystemMode))
            {
                return Q_HANDLED();
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            AllPlaySrv_enableDirectMode(FALSE);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}
#endif

/* Bluetooth state*/
static QState MainApp_Bluetooth(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if(FALSE == allPlayInfo->bAllplayBtConnected)
            {
                AllPlaySrv_BluetoothEnable(TRUE);
                if(me->tickHandlers[BT_PAIRING_TIMER].timer > 0)
                {
                    MainApp_SendLedReq(me, BT_PAIRING_ENABLE_PAT);
                    AllPlaySrv_BluetoothPairable(TRUE);
                }
                else
                {
                    MainApp_SendLedReq(me, BT_RECONNECTINT_PAT);
                    me->tickHandlers[BT_RECONNECT_TIMER].timer = MAINAPP_BT_RECONNECT_TIMEOUT_IN_MS;
                }
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if(me->tickHandlers[BT_PAIRING_TIMER].timer || me->tickHandlers[BT_RECONNECT_TIMER].timer)
            {
                me->tickHandlers[LED_DIM_TIMER].timer = 0;
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            me->tickHandlers[BT_PAIRING_TIMER].timer = 0;
            AllPlaySrv_BluetoothEnable(FALSE);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}

