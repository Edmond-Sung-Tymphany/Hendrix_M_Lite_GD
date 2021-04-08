/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Main Application
                  -------------------------

                  SW Module Document




@file        MainApp.c
@brief       Main application for B&O S8 MKII
@author      Edmond Sung
@date        2015-01-12
@copyright (c) Tymphany Ltd. All rights reserved.


-------------------------------------------------------------------------------
*/

/*to do list*/
/* need to remove the extern with getDeviceId function after the GPIO struct is changed*/

#include "./MainApp_priv.h"
#include "trace.h"

#ifndef MAINAPP_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif
#ifdef HAS_IR_LEARNING_DELEGATE
static cIrLearningDlg IrLearningDlg;
#endif



#define CAST_ME cMainApp * MainApp = (cMainApp *) me;


enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    MAINAPP_WAKEUP_SIG,
    MAINAPP_HIJACK_SIG,
};



//eMainAppSysMode sysState;

/* Private functions / variables. */

/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[5];

static cGpioDrv gpioDrv;

static tMainAppState MainAppState;

const tSourceObj DeafultSourceObj[4]=
{
        {
            .inputSource = INPUT_SOURCE_ANALOG,
            .isHasMusic = 0,
            .HasMusicCnt = 0,
            .isVolLocked = 0,
            .isJackIn = 0,
            .isMuted =0,
            .currentVol = DEFAULT_VOLUME,
            .isSilenceCnt = 0,
            .physicalChannel = AUDIO_CHANNEL_1,
        },
        {
            .inputSource = INPUT_SOURCE_SPDIF0,
            .isHasMusic = 0,
            .HasMusicCnt = 0,
            .isVolLocked = 0,
            .isJackIn = 0,
            .isMuted =0,
            .currentVol = DEFAULT_VOLUME,
            .isSilenceCnt = 0,
            .physicalChannel = AUDIO_CHANNEL_SPDIF_0,
        },
        {
            .inputSource = INPUT_SOURCE_SPDIF1,
            .isHasMusic = 0,
            .HasMusicCnt = 0,
            .isVolLocked = 0,
            .isJackIn = 0,
            .isMuted =0,
            .currentVol = DEFAULT_VOLUME,
            .isSilenceCnt = 0,
            .physicalChannel = AUDIO_CHANNEL_SPDIF_1,
        },
        {
            .inputSource = INPUT_SOURCE_RJ45,
            .isHasMusic = 0,
            .HasMusicCnt = 0,
            .isVolLocked = 0,
            .isJackIn = 0,
            .isMuted =0,
            .currentVol = DEFAULT_VOLUME,
            .isSilenceCnt = 0,
            .physicalChannel = AUDIO_CHANNEL_1,
        }
};

/* the time (ms) per timeout signal */
#define MAIN_APP_TIMEOUT_IN_MS                  (10)

#define MAIN_APP_STANDBY_MODE_TIMEOUT_IN_MS     (10)       // in standby 

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(MainApp);
    QS_OBJ_DICTIONARY(MainApp_PreActive);

    /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                            MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);
    
    /* Subscribe */
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
#endif
//    QActive_subscribe((QActive*) me, POWER_WAKE_UP_SIG);
    QActive_subscribe((QActive*) me, AUDIO_MUSIC_STREAM_STATE_SIG);
    QActive_subscribe((QActive*) me, AUDIO_STATE_SIG);
    
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
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e)
{
    tGPIODevice *pPowerConf;
    pPowerConf = (tGPIODevice*)getDevicebyId(POWER_DEV_ID,NULL);
    GpioDrv_Ctor(&gpioDrv,pPowerConf);
    /* initial the default value for first power up or factory reset*/
    uint32 factoryRstCode;
    factoryRstCode = *(uint32*)(Setting_Get(SETID_MAGIC_NUMBER));
    if (factoryRstCode!=FACTORY_RESET_CODE)
    {
        MainApp_InitialVariablesForFactoryReset(me);
    }
    MainApp_RestoreVariablesForPowerUp(me);
    IrLearningDlg_Ctor(&IrLearningDlg, (QActive*)me);
    return Q_TRAN(&MainApp_PoweringUp);
}

/* power up the power before preActive*/
static QState MainApp_PoweringUp(QActive * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
            PWR_EN_OFF(gpioDrv);            
            //TP_PRINTF("PWR_EN_OFF\r\n");
            CODEC_RST_LOW(gpioDrv);
            //TP_PRINTF("CODEC_RST_LOW\r\n");
            return Q_HANDLED();
        case SYSTEM_MODE_RESP_SIG:
            return Q_TRAN(MainApp_PreActive);
        default: 
            break;
    }
    return Q_SUPER(&QHsm_top);
}

void MainApp_StartDlg(QActive* Delegate)
{
    DlgStartEvt* evt = Q_NEW(DlgStartEvt, MAINAPP_START_DLG_SIG);
    QACTIVE_POST(Delegate, (QEvt*)evt, 0);
}



/* PreActive state */
static QState MainApp_PreActive(QActive * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            MainApp_SwitchMode(MainApp, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            /* add delay for power up, can directly change the timer armx below after LED is move out*/
            OPAMP_MUTE(1);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            LedSrv_SetPatt((QActive*)me, RGB_LED, FADE_IN_0_5s_PATT);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            /*Sometimes will get the timeout signal before enable timer here, which
              is coming from the timer in DeActive state
            TODO: find a way to flush the time out signal when leaving the state*/
            //PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_TRAN(MainApp_Active);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}


static void MainApp_SwitchRJ45(bool sw)
{
    sw?RELAY_SW_OFF(gpioDrv):RELAY_SW_ON(gpioDrv);
    RJ45_SENSE_OUT(sw);
}


void MainApp_MuteDAC(bool mute, tMainAppState* appState)
{
    TP_PRINTF("Mute(%d)\r\n", mute);
    AudioSrv_muteDAC(mute);

    appState->IsMute = mute;
}

static bool MainApp_IsOpticalIn(tMainAppState* appState)
{
    return !!(appState->pSourceObj->inputSource==INPUT_SOURCE_SPDIF0||appState->pSourceObj->inputSource==INPUT_SOURCE_SPDIF1);
}

static void MainApp_clearIsMusicFlag()  
{
    tSourceObj* tmpSourceObjPtr=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);      // Get the Source object pointer in RAM
    for (eAudioJackId i = AUXIN_JACK; i<ArraySize(AudioJackIn); i++)
    {
        tmpSourceObjPtr[i].isHasMusic=0;
        tmpSourceObjPtr[i].HasMusicCnt =0;
    }

}

/* Active state  - super state for "normal" behaviour */
static QState MainApp_Active(QActive * const me, QEvt const * const e)
{
    CAST_ME;
    tMainAppState* appState = &MainAppState ;
    static bool rstDSP=1;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            //TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            TP_PRINTF("Software Version: %s\r\n", PRODUCT_VERSION_MCU);

            PWR_EN_ON(gpioDrv);            
            TP_PRINTF("PWR_EN_ON\r\n");
            appState->autoStandbyTimer = IDLE_AUTO_POWER_OFF_TIMEOUT_MS;
            MainApp_SwitchMode(MainApp, NORMAL_MODE);
            MainApp_PublishSystemStatus(MainApp, SYSTEM_ACTIVE_STA);
            appState->hijackSourceScanTimeout = HIJACK_SOURCE_SCAN_TIME;
            appState->checkDigitalCodeTimeout = 500;
            appState->SystemMode = SYS_MODE_NORMAL;
            /* Skip the channel has no jack in */
            /*
            tSourceObj* tmpSourceObjPtr=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
            if (AudioDrv_IsLineInJackPluggedIn()==0 && appState->pSourceObj->inputSource==INPUT_SOURCE_ANALOG)
            {
                if (AudioDrv_IsSpdif1InJackPluggedIn())
                    appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF1];
                else if (AudioDrv_IsRJ45InJackPluggedIn())
                    appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_RJ45];
                else 
                    appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF0];

                MainApp_switchToSource(MainApp, appState, (eAudioJackId)appState->pSourceObj->inputSource);
                
            }
            /*--------------------------------------*/

            MainApp_TurnOnLED(me, appState);
            MainApp_SwitchRJ45(appState->pSourceObj->inputSource==INPUT_SOURCE_RJ45);
            rstDSP = 1;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, 8*MAIN_APP_TIMEOUT_IN_MS);
            if (appState->pSourceObj->inputSource!=INPUT_SOURCE_RJ45)
            OPAMP_MUTE(0);
            appState->SwitchSourceDelay = SWITCH_SOURCE_DELAY_MS;
            
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if (appState->SystemMode == SYS_MODE_NORMAL)
            {
                MainApp_ParseKeyEvent(MainApp,e);
            }
            // Becasue UI require Factory Reset during Learning Mode
            if (evt->keyEvent==KEY_EVT_VERY_LONG_HOLD && evt->keyId==POWER_KEY)
            {
                uint32 getMagic=0x00000000;
                LedSrv_SetPatt((QActive*)me, RGB_LED, FACTORY_RESET_PATT_1);
                Setting_Set(SETID_MAGIC_NUMBER, &getMagic);                 // Save Magic Number to mark the default value.
                //Setting_Bookkeeping();                                      // Save everything in RAM to Flash
                appState->SystemResetTimeout = 6000;
            }
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
            return Q_HANDLED();
        }
        case AUDIO_SWITCH_CHANNEL_RESP_SIG:
        {
           if (MainApp_IsOpticalIn(appState))
            {
                if (AudioSrv_IsDigitalPCM())
                {
                    MainApp_MuteDAC(0 , appState);
                }
            }
            return Q_HANDLED();
        }

        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            MainApp_MusicDetectSignalHandler(MainApp, e, appState);
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            MainApp_AudioStateSignalHandler(MainApp, e, appState);
            
            return Q_HANDLED();
        }
        case MAINAPP_HIJACK_SIG:
        {
            if (appState->SystemMode == SYS_MODE_IR_LEARNING_MODE)
            {
                return Q_HANDLED();
            }
            MainAppHijackEvt *evt = (MainAppHijackEvt*)e;
            if (appState->pSourceObj->inputSource != (eSourceName)evt->HijackSrc)                
            {
                TP_PRINTF("MAINAPP_HIJACK_SIG\r\n");
                TP_PRINTF("MainApp_switchToSource(%d)\r\n", evt->HijackSrc);
                MainApp_switchToSource(MainApp, appState, evt->HijackSrc);
                appState->SettingTimeout=SETTING_SAVE_MS;
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            if (rstDSP)
            {
                rstDSP=0;
                CODEC_RST_HIGH(gpioDrv);
                //TP_PRINTF("CODEC_RST_HIGH\r\n");
            }
            if (appState->checkDigitalCodeTimeout)
            {
                appState->checkDigitalCodeTimeout-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->checkDigitalCodeTimeout==0)
                {
                    appState->checkDigitalCodeTimeout=10;       // repeat this checking every 10ms
                    if (MainApp_IsOpticalIn(appState) && (appState->SwitchSourceDelay==0) && (appState->SystemMode != SYS_MODE_IR_LEARNING_MODE))
                    {
                        bool MuteDac=!AudioSrv_IsDigitalPCM();
                        if (appState->IsMute!=MuteDac)
                        {
                            TP_PRINTF("MainApp_MuteDAC(%d) \r\n",MuteDac);
                            MainApp_MuteDAC(MuteDac, appState);
                        }
                    }
                }
            }
            
            if (appState->doublePressTimeout)
            {
                appState->doublePressTimeout-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->doublePressTimeout==0)
                {
                     //MainApp_SwitchSourceKeyEvt(MainApp, appState);
                     KeySrv_SendKeyEvt(KEY_EVT_SHORT_PRESS,SOURCE_SWITCH_IR_KEY);
                }
            }
            if (appState->SwitchSourceDelay)
            {
                appState->SwitchSourceDelay-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->SwitchSourceDelay==350)
                {
                    OPAMP_MUTE(1);
                    MainApp_SwitchAudioChannel(MainApp, appState->pSourceObj->physicalChannel);
                }
                if (appState->SwitchSourceDelay==250)
                {
                    if (appState->pSourceObj->inputSource!=INPUT_SOURCE_RJ45)
                        MainApp_SwitchRJ45(FALSE);

                }
                if (appState->SwitchSourceDelay<=0)
                {
                    if (appState->pSourceObj->inputSource==INPUT_SOURCE_RJ45)
                    {
                        MainApp_SwitchRJ45(TRUE);
                    }
                    else
                    {
                        MainApp_MuteDAC(0 , appState);
                    }
                    OPAMP_MUTE(0);
                   MainApp_TurnOnLED((QActive*)me, appState);
                }
            }
            if (appState->SettingTimeout)
            {
                appState->SettingTimeout-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->SettingTimeout<=0)
                {
                    Setting_Bookkeeping();
                }

            }
            if (appState->hijackSourceScanTimeout)          // was set to 500ms
            {
                appState->hijackSourceScanTimeout-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->hijackSourceScanTimeout<=0)
                {
                    eAudioJackId channel;
                    tSourceObj* pSrcObj = (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
                    MainApp_signalSilent(MainApp, &channel);    
                    if (MainApp_signalValid(MainApp, &channel))
                    {
                            if (pSrcObj[channel].WasSilent)
                            {
                                pSrcObj[channel].WasSilent =0;
                               MainApp_HijackSignalInput(MainApp, channel);
                            }
                    }
                    appState->hijackSourceScanTimeout=HIJACK_SOURCE_SCAN_TIME;
                }
            }
            if(MainApp_IsCurrentSourceIdle())
            {
                appState->autoStandbyTimer -= MAIN_APP_TIMEOUT_IN_MS;
                if (appState->autoStandbyTimer%5000 == 0)
                {
                    TP_PRINTF("autoStandbyTimer = %d\r\n", appState->autoStandbyTimer/1000);
                }
                if(appState->autoStandbyTimer <=0)
                {
                    return Q_TRAN(MainApp_PoweringDown);
                }
            }
            else
            {
                appState->autoStandbyTimer = IDLE_AUTO_POWER_OFF_TIMEOUT_MS;
            }
            if (appState->SystemResetTimeout)
            {
                appState->SystemResetTimeout-=MAIN_APP_TIMEOUT_IN_MS;
                if (appState->SystemResetTimeout <=0)
                {
                    NVIC_SystemReset();
                }
            }
            /* When it is not in Learning mode, process the key */
            if (appState->SystemMode != SYS_MODE_IR_LEARNING_MODE)
            {
                CheckIRPress();
            }
            if (appState->PowerOffFlag)
            {
                appState->PowerOffFlag=FALSE;
                return Q_TRAN(MainApp_PoweringDown);
            }
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);

            return Q_HANDLED();
        }
        case MAINAPP_STOP_DLG_SIG:
        {    
            appState->SystemMode = SYS_MODE_NORMAL;
            MainApp_MuteDAC(0, appState);

            MainApp_TurnOnLED(me, appState);

            return Q_HANDLED();
            
        }
        case Q_EXIT_SIG:
        {
            TP_PRINTF("Exit %s\r\n", __FUNCTION__);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/* DeActive state  -- power off */
static QState MainApp_DeActive(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TP_PRINTF("Software Version: %s\r\n", PRODUCT_VERSION_MCU);
            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/* powering down state, which is turning off the system power and have some delay*/
static QState MainApp_PoweringDown(QActive * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            LedSrv_SetPatt((QActive*)me, RGB_LED, OFF_PATT);
            LedSrv_SetPatt((QActive*)me, RGB_LED, FADE_OUT_0_5s_PATT);
            MainApp_PublishSystemStatus(MainApp, SYSTEM_SLEEP_STA);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            MainApp_SwitchMode(MainApp, POWERING_DOWN_MODE);
            OPAMP_MUTE(1);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            return Q_TRAN(MainApp_Standby);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&MainApp_DeActive);
}


/* standby state, which does NOT turn off system power*/
static QState MainApp_Standby(QActive * const me, QEvt const * const e)
{
    CAST_ME;
    tMainAppState* appState = &MainAppState ;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            //MainApp_clearIsMusicFlag();
            MainApp_SwitchRJ45(FALSE);

            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            CODEC_RST_LOW(gpioDrv);             // Pull low the Reset Pin of DAC
            TP_PRINTF("CODEC_RST_LOW\r\n");

            PWR_EN_OFF(gpioDrv);                // Turn the Main Power OFF
            TP_PRINTF("PWR_EN_OFF\r\n");

            appState->SystemMode = SYS_MODE_STANDBY;
            MainApp_SwitchMode(MainApp, STANDBY_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, 500);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((evt->keyEvent == KEY_EVT_SHORT_PRESS)&&(evt->keyId == POWER_KEY))
            {
                //MainApp_ParseKeyEvent(MainApp,e);
                return Q_TRAN(MainApp_PreActive);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            MainApp_StandbyPeriodicTask(MainApp, appState, MAIN_APP_STANDBY_MODE_TIMEOUT_IN_MS);
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_STANDBY_MODE_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            MainApp_MusicDetectSignalHandler(MainApp, e, appState);
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            MainApp_AudioStateSignalHandler(MainApp, e, appState);
            return Q_HANDLED();
        }
        case MAINAPP_WAKEUP_SIG:
        {
            MainAppWakeupEvt *evt = (MainAppWakeupEvt*)e;
            MainApp_switchToSource(MainApp, appState, evt->WakeupSrc);
            TP_PRINTF("MAINAPP_WAKEUP_SIGs\r\n");
            TP_PRINTF("MainApp_switchToSource(%d)\r\n", evt->WakeupSrc);

            
            return Q_TRAN(MainApp_PreActive);
        }
        case Q_EXIT_SIG:
        {
            TP_PRINTF("Exit %s\r\n", __FUNCTION__);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: 
            break;
    }
    return Q_SUPER(&MainApp_DeActive);
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

static void MainApp_TurnOnLED(QActive * const me, tMainAppState* appState)
{
    if (appState->pSourceObj->isVolLocked)
    {
            LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_RED);
    }
    else
    {
#ifdef SOURCE_LED_DEBUG
        switch (appState->pSourceObj->inputSource)
        {
            case INPUT_SOURCE_ANALOG:
                LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_GREEN);
                break;
            case INPUT_SOURCE_SPDIF0:
                LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_BLUE);
                break;
            case INPUT_SOURCE_SPDIF1:
                LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_YELLOW);
                break;
            case INPUT_SOURCE_RJ45:
                LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_PURPLE);
                break;
            default:
                break;
        }
#else
        LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_WHITE);
#endif
    }
}


static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

/* initial variable for factory reset */
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me)
{
    uint32 getMagic=FACTORY_RESET_CODE;
    eSourceName curSourceObj=INPUT_SOURCE_ANALOG;
    uint8 maxVol = MAX_VOLUME;
    Setting_Set(SETID_MAGIC_NUMBER, &getMagic);                 // Save Magic Number to mark the default value.
    Setting_Set(SETID_MAX_VOLUME, &maxVol);                     // Save MAX volume to RAM
    Setting_Set(SETID_SOURCE_OBJECT, &DeafultSourceObj);        // Save All default Source Object
    Setting_Set(SETID_CURRENT_SOURCE, &curSourceObj);           // Save the default Source
    IrLearningDlg_EraseLearntCode();
    Setting_Bookkeeping();                                      // Save everything in RAM to Flash
}

static void MainApp_RestoreVariablesForPowerUp(cMainApp * const me)
{
    tMainAppState* appState = &MainAppState ;
    tSourceObj* tmpSourceObjPtr;
    eSourceName curSourceObj;
    eAudioChannel audioChannel;
    tmpSourceObjPtr=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);      // Restore all the Source object pointer in Flash

    Setting_Set(SETID_SOURCE_OBJECT, tmpSourceObjPtr);                  // Save the Source to RAM
    tmpSourceObjPtr=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);      // Restore all the Source object pointer in RAM

    tmpSourceObjPtr[0].isJackIn=0;
    tmpSourceObjPtr[1].isJackIn=0;
    tmpSourceObjPtr[2].isJackIn=0;
    tmpSourceObjPtr[3].isJackIn=0;

    curSourceObj = *((eSourceName*)Setting_Get(SETID_CURRENT_SOURCE));  // Restore current source.
    appState->pSourceObj=&tmpSourceObjPtr[curSourceObj];             // Restore the current Source Object pointer in RAM 
    Setting_Set(SETID_VOLUME, &(appState->pSourceObj->currentVol));                      // Save Default volume to RAM
    uint8* tempPtr = (uint8*)Setting_Get(SETID_IR_LEARN_CODE);          // Restore the learnt IR code.
    Setting_Set(SETID_IR_LEARN_CODE, tempPtr);                          // Save the learnt IR code.
    audioChannel=appState->pSourceObj->physicalChannel;
    Setting_Set(SETID_CHANNEL, &audioChannel);                          // Save the learnt IR code.
    
    appState->SwitchSourceDelay = 0;
    appState->doublePressTimeout = 0;
    
    if (appState->pSourceObj->currentVol>MAX_VOLUME)
        appState->pSourceObj->currentVol = DEFAULT_VOLUME;
    Setting_Set(SETID_VOLUME, &(appState->pSourceObj->currentVol));
    uint8 maxVol = MAX_VOLUME;
    Setting_Set(SETID_MAX_VOLUME, &maxVol);
    uint32 getMagic=FACTORY_RESET_CODE;
    Setting_Set(SETID_MAGIC_NUMBER, &getMagic);                 // Save Magic Number to mark the default value.
}

static void MainApp_VolumeDown(cMainApp * const me, uint8 step)
{
    tMainAppState* appState = &MainAppState ;
    uint8 vol;
    if (appState->pSourceObj->isVolLocked==0)
    {
        if (appState->pSourceObj->isMuted)
        {
            appState->pSourceObj->isMuted=0;
            vol = appState->pSourceObj->saved_vol;
        }
        else
        {
            
            vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(vol >= step)
            {
                vol-=step;
            }
            else
            {
                vol = 0;
            }
        }
        appState->pSourceObj->currentVol = vol;
        Setting_Set(SETID_VOLUME, &vol);
        AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
    }
}

static void MainApp_VolumeUp(cMainApp * const me, uint8 step)
{
    tMainAppState* appState = &MainAppState ;
    uint8 vol;
    if (appState->pSourceObj->isVolLocked==0)
    {
        if (appState->pSourceObj->isMuted)
        {
            appState->pSourceObj->isMuted=0;
            vol = appState->pSourceObj->saved_vol;
        }
        else
        {
            vol = *(uint8*)Setting_Get(SETID_VOLUME);
            uint8 maxVol = *(uint8*)Setting_Get(SETID_MAX_VOLUME);
            if((step + vol)<= maxVol)
            {
                vol+=step;
            }
            else
            {
                vol = maxVol;
            }
        }
        appState->pSourceObj->currentVol = vol;
        Setting_Set(SETID_VOLUME, &vol);
        AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
    }
}

static void MainApp_MuteButtonEvt(cMainApp * const me)
{
    tMainAppState* appState = &MainAppState;
    uint8 vol;
    appState->pSourceObj->isMuted=!appState->pSourceObj->isMuted;       // Toggle the current mute status
    vol = *(uint8*)Setting_Get(SETID_VOLUME);
    if (vol && appState->pSourceObj->isMuted)
    {
        appState->pSourceObj->saved_vol=vol;
        vol = 0;
    }
    else if (appState->pSourceObj->isMuted==0)
    {
        vol = appState->pSourceObj->saved_vol;
    }
    appState->pSourceObj->currentVol = vol;
    Setting_Set(SETID_VOLUME, &vol);
    AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
    
    
}

/*
static void MainApp_RequestMuteAudio(cMainApp * const me, bool isMuteOn)
{
    AudioMuteReqEvt* muteReqEvt = Q_NEW(AudioMuteReqEvt, AUDIO_MUTE_SIG);
    muteReqEvt->mute = isMuteOn;
    muteReqEvt->sender = (QActive*)me;
    SendToServer(AUDIO_SRV_ID,(QEvt*)muteReqEvt);
}

static bool MainApp_IsReadyForSleep()
{// this function check if the DC (detect by GPIO) and Micro USB is out, to make sure they are ready for interrupt 
    bool ret = FALSE;

    return ret;
}
*/
static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus)
{
    SystemStatusEvt *pe = Q_NEW(SystemStatusEvt, SYSTEM_STATE_SIG);
    pe->systemStatus = systemStatus;
    QF_PUBLISH(&pe->super, me);
}

static bool MainApp_IsCurrentSourceIdle()
{
    tMainAppState* appState = &MainAppState;
    return (appState->pSourceObj->isHasMusic==0);
}


static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    tMainAppState* appState = &MainAppState ;
    KeyStateEvt *evt = (KeyStateEvt*)e;
    //LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
    switch(evt->keyEvent)
    {
        case KEY_EVT_DOWN:
            MainApp_KeyDownEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            //MainApp_KeyLPressEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            //MainApp_KeyLPressHoldEvtAction(me, evt);
            break;
        default:
            break;
    }
    appState->SettingTimeout=SETTING_SAVE_MS; 
    appState->autoStandbyTimer = IDLE_AUTO_POWER_OFF_TIMEOUT_MS;
    //LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
}
/*--------------------- Key Function implementation ---------------------------*/
static void MainApp_MultiFunctionKeyEvt(cMainApp * const me,tMainAppState* appState)
{
    if (appState->doublePressTimeout==0)
    {
        appState->doublePressTimeout=500;
    }
    else if (appState->doublePressTimeout)
    {
        appState->doublePressTimeout=0;
        MainApp_VolumeLockProcess(me, appState);
        LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
    }
}

static void MainApp_switchToSource(cMainApp * const me,tMainAppState* appState, eAudioJackId AudioChannel)
{
    tSourceObj* SourceObj=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);        // Get the source object address in RAM
    appState->pSourceObj= &SourceObj[AudioChannel];                           // current source object point to the wakeup one in RAM

    MainApp_MuteDAC(1, appState);
    eSourceName curSourceObj=appState->pSourceObj->inputSource;                 // 
    Setting_Set(SETID_CURRENT_SOURCE, &curSourceObj);
    eAudioChannel phyChannel=appState->pSourceObj->physicalChannel;
    Setting_Set(SETID_CHANNEL, &phyChannel);   
    Setting_Set(SETID_VOLUME, &appState->pSourceObj->currentVol);
    AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);

    appState->SwitchSourceDelay = SWITCH_SOURCE_DELAY_MS;
}


static void MainApp_SwitchSourceKeyEvt(cMainApp * const me,tMainAppState* appState)
{
    tSourceObj* tmpSourceObjPtr=(tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
    switch (appState->pSourceObj->inputSource)
    {
        case INPUT_SOURCE_RJ45:
            if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF0].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF0];
            else if (tmpSourceObjPtr[INPUT_SOURCE_ANALOG].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_ANALOG];
            else if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF1].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF1];
            else
                return;
            break;
        case INPUT_SOURCE_ANALOG:
            if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF1].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF1];
            else if (tmpSourceObjPtr[INPUT_SOURCE_RJ45].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_RJ45];
            else if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF0].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF0];
            else
                return;
            break;
        case INPUT_SOURCE_SPDIF0:
            if (tmpSourceObjPtr[INPUT_SOURCE_ANALOG].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_ANALOG];
            else if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF1].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF1];
            else if (tmpSourceObjPtr[INPUT_SOURCE_RJ45].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_RJ45];
            else
                return;
            break;
        case INPUT_SOURCE_SPDIF1:
            if (tmpSourceObjPtr[INPUT_SOURCE_ANALOG].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_ANALOG];
            else if (tmpSourceObjPtr[INPUT_SOURCE_RJ45].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_RJ45];
            else if (tmpSourceObjPtr[INPUT_SOURCE_SPDIF0].isJackIn)
                appState->pSourceObj = &tmpSourceObjPtr[INPUT_SOURCE_SPDIF0];
            else
                return;
        default:
            break;
    }
    MainApp_switchToSource(me, appState, (eAudioJackId)appState->pSourceObj->inputSource);
}



static void MainApp_VolumeLockProcess(cMainApp * const me,tMainAppState* appState)
{
    uint8 vol;
    // when it is muted, unmute it first
    if (appState->pSourceObj->isMuted)
    {
        appState->pSourceObj->isMuted = 0;
        vol = appState->pSourceObj->saved_vol;
        appState->pSourceObj->currentVol = vol;
        Setting_Set(SETID_VOLUME, &vol);
    }
    if (appState->pSourceObj->isVolLocked)
    {
        appState->pSourceObj->isVolLocked=0;
        appState->pSourceObj->currentVol = appState->pSourceObj->backupVol;
    }
    else
    {
        appState->pSourceObj->isVolLocked=1;
        appState->pSourceObj->backupVol = appState->pSourceObj->currentVol;
        appState->pSourceObj->currentVol =MAX_VOLUME;
    }
    Setting_Set(SETID_VOLUME, &appState->pSourceObj->currentVol);
    AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
    MainApp_TurnOnLED((QActive*)me,appState);
}
/*-------------------------------------------------------------------------------*/
/*
static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
}
*/
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    tMainAppState* AppState = &MainAppState;
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
            MainApp_VolumeDown(me, 1);
            LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
            break;
        case VOLUME_UP_KEY:
            MainApp_VolumeUp(me, 1);
            LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
            break;
        case MUTE_KEY:
            MainApp_MuteButtonEvt(me);
            LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
            break;
        case POWER_IR_KEY:
            AppState->PowerOffFlag = TRUE;
            break;
       default:
            break;
    }
}

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    tMainAppState* AppState = &MainAppState;
    switch(e->keyId)
    {
        case POWER_KEY:
            MainApp_MultiFunctionKeyEvt(me, AppState);
            break;
        case SOURCE_SWITCH_IR_KEY:
            MainApp_SwitchSourceKeyEvt(me, AppState);
            LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
            break;
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_KeyLPressHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    uint32 getMagic=0x00000000;
    switch(e->keyId)
    {
        case POWER_KEY:
            LedSrv_SetPatt((QActive*)me, RGB_LED, FACTORY_RESET_PATT_1);
            Setting_Set(SETID_MAGIC_NUMBER, &getMagic);                 // Save Magic Number to mark the default value.
            Setting_Bookkeeping();                                      // Save everything in RAM to Flash
        default:
            break;
    }
}

static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    tMainAppState* appState = &MainAppState ;
    switch(e->keyId)
    {
        case POWER_KEY:
            appState->SystemMode = SYS_MODE_IR_LEARNING_MODE;
            MainApp_MuteDAC(1 , appState);
            MainApp_StartDlg((QActive*)&IrLearningDlg);
            /*Enter IR learning mode*/
        default:
            break;
    }
}


/*

static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    tMainAppState* appState = &MainAppState ;
    switch(e->keyId)
    {
        case POWER_KEY:
            appState->SystemMode = SYS_MODE_IR_LEARNING_MODE;
            MainApp_StartDlg((QActive*)&IrLearningDlg);
            //Enter IR learning mode
        default:
            break;
    }
}



static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
}




static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
}

static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
  switch(e->keyId)
    {
        //if necessary, add the handler for keys here 
        case POWER_KEY:
            break;
        default:
            break;
    }
}
*/

static void MainApp_SwitchAudioChannel(cMainApp * const me, eAudioChannel channel)
{
#ifdef HAS_AUDIO_CONTROL
    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = channel;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
#endif
}

static void MainApp_HijackSignalInput(cMainApp * const me, eAudioJackId channel)
{
    CAST_ME
    MainAppHijackEvt* HijackEvt = Q_NEW(MainAppHijackEvt, MAINAPP_HIJACK_SIG);
    HijackEvt->HijackSrc = channel;
    QACTIVE_POST((QActive *) MainApp, (QEvt*)HijackEvt, 0);
}



static void MainApp_WakeupBySignalInput(cMainApp * const me, eAudioJackId channel)
{
    CAST_ME
    MainAppWakeupEvt* WakeupEvt = Q_NEW(MainAppWakeupEvt, MAINAPP_WAKEUP_SIG);
    WakeupEvt->WakeupSrc = channel;
    QACTIVE_POST((QActive *) MainApp, (QEvt*)WakeupEvt, 0);
}


static void MainApp_MusicDetectSignalHandler(cMainApp * const me, QEvt const * const e,tMainAppState* appState)
{
    AudioMusicDetectStateEvt* evt = (AudioMusicDetectStateEvt*)e;
    tSourceObj* SrcObjPtrRam = (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
    tSourceObj* pTmpSrcObj;
    TP_PRINTF("MusicStream(%d)=%d\r\n", evt->jackId, evt->hasMusicStream);
    switch (evt->jackId)
    {
        case AUXIN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_ANALOG];
            pTmpSrcObj->isHasMusic=evt->hasMusicStream;
            break;
        case SPDIF_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_SPDIF0];
            pTmpSrcObj->isHasMusic=evt->hasMusicStream;
            break;
        case SPDIF1_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_SPDIF1];
            pTmpSrcObj->isHasMusic=evt->hasMusicStream;
            break;
        case RJ45_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_RJ45];
            pTmpSrcObj->isHasMusic=evt->hasMusicStream;
            break;
        default:
            break;
            
    }
    
}

static void MainApp_AudioStateSignalHandler(cMainApp * const me, QEvt const * const e,tMainAppState* appState)
{
    AudioStateEvt* evt = (AudioStateEvt*)e;
    tSourceObj* SrcObjPtrRam = (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
    tSourceObj* pTmpSrcObj;
    //TP_PRINTF("Jack(%d)=%d\r\n", evt->jackId, evt->IsJackIn);
    switch (evt->jackId)
    {
        case AUXIN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_ANALOG];
            break;
        case SPDIF_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_SPDIF0];
            break;
        case SPDIF1_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_SPDIF1];
            break;
        case RJ45_IN_JACK:
            pTmpSrcObj=&SrcObjPtrRam[INPUT_SOURCE_RJ45];
            break;
        default:
            break;
            
    }
    pTmpSrcObj->isJackIn=evt->IsJackIn;        
    if (evt->IsJackIn==0)
    {
        pTmpSrcObj->WasSilent=1;
        pTmpSrcObj->isHasMusic = 0;
    }
    appState->SettingTimeout=SETTING_SAVE_MS;
}

static bool MainApp_signalSilent(cMainApp * const me, eAudioJackId* channel)        // will enter every 10ms
{
    tSourceObj* tmpSrcObj= (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
    for (eAudioJackId i = AUXIN_JACK; i<ArraySize(AudioJackIn); i++)
    {
        if (tmpSrcObj[i].isHasMusic)        
        {
                tmpSrcObj[i].isSilenceCnt=0;
        }
        else        
        {

            if (tmpSrcObj[i].isSilenceCnt<CHECK_SILENCE_VALID_COUNT)
            {
                tmpSrcObj[i].isSilenceCnt++;
                //TP_PRINTF("Counting to Silent(%d) %d\r\n", i, tmpSrcObj[i].isSilenceCnt);
                if (tmpSrcObj[i].isSilenceCnt== CHECK_SILENCE_VALID_COUNT)
                {
                    tmpSrcObj[i].WasSilent=1;
                    *channel = i;
                    //TP_PRINTF("SrcObj[%d] set to Silent\r\n", i);
                    return TRUE;
                }
            }
           

        }
    }
    return (FALSE);
    
}

static bool MainApp_signalValid(cMainApp * const me, eAudioJackId* channel)
{
    tSourceObj* tmpSrcObj= (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);;
    for (eAudioJackId i=AUXIN_JACK; i< ArraySize(AudioJackIn); i++)
    {
        if (tmpSrcObj[i].isHasMusic)        
        {
            if (tmpSrcObj[i].HasMusicCnt<CHECK_MUSIC_VALID_COUNT)
            {
                tmpSrcObj[i].HasMusicCnt++;   // if scanned source has music count++
                if (tmpSrcObj[i].HasMusicCnt==CHECK_MUSIC_VALID_COUNT)       // 10 * TickTimeInterval = 500ms * 10
                {
                    //TP_PRINTF("SrcObj[%d].HasMusicCnt==CHECK_MUSIC_VALID_COUNT\r\n", i);
                    *channel = i;
                    return (TRUE);
                }
            }
        }
        else        
        {
            tmpSrcObj[i].HasMusicCnt=0;
        }
    }
    return (FALSE);
   
}

static void MainApp_StandbyPeriodicTask(cMainApp * const me,tMainAppState* appState, uint16 TickTimeInterval)
{
    eAudioJackId channel;
    MainApp_signalSilent(me, &channel);    
    if (MainApp_signalValid(me, &channel))
    {

            tSourceObj* pSrcObj = (tSourceObj*)Setting_Get(SETID_SOURCE_OBJECT);
            if (pSrcObj[channel].WasSilent)
            {
                pSrcObj[channel].WasSilent=0;
                MainApp_WakeupBySignalInput(me, channel);
            }

    }
}

