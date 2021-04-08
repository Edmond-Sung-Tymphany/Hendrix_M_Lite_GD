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

/*****************************************************************
 * Type Definition
 *****************************************************************/
typedef enum
{
    EXCEPTION_CODE_INVALID = 0,
    EXCEPTION_CODE_UPGRADE_TIMEOUT,
    EXCEPTION_CODE_PREACTIVE_TIMEOUT,
    EXCEPTION_CODE_POWERING_UP_TIMEOUT,
    EXCEPTION_CODE_POWERING_DOWN_TIMEOUT,
} eRebootCode;

typedef enum
{
    NET_RESET_IN_L_HOLD     = 0x01,
    NET_RESET_IN_V_L_HOLD   = 0x02,
    POWER_IN_L_HOLD         = 0x04,
    POWER_IN_V_L_HOLD       = 0x08,
    FACTORY_RESET_TRIGGER   = NET_RESET_IN_L_HOLD | NET_RESET_IN_V_L_HOLD | POWER_IN_L_HOLD | POWER_IN_V_L_HOLD,
} eFactoryResetCombine;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
};

/*****************************************************************
 * Definition
 *****************************************************************/
#define MAINAPP_PREACTIVE_TIMEOUT_IN_MS         (10000) /* preactive state timeout: 10 sec */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS           (90*60*1000) /*upgrade timeout: 90 min */
#define MAINAPP_TIMEOUT_IN_MS                   (10000) /*10 sec */
#define MAINAPP_DELAY_BEFORE_SOFTRESET_IN_MS    (4000) /*4 sec */

#define ALLPLAY_MAX_VOLUME              (100)  /*the value should be change according to setting in SAM.*/
#define MainApp_mcuVol2AllplayVol(vol)  ((vol)*(me->allplayMaxVol)/MAX_VOLUME)
#define MainApp_allplayVol2McuVol(vol)  ((vol)*MAX_VOLUME/(me->allplayMaxVol))
#define MAINAPP_UPDATE_SAM_VOL_INTERVAL (2) /* update sam vol every 2 steps duration when P&H vol key */

#define TRIGGER_SLEEP(bi)          ((CHARGER_BATT_STA==(bi)->chargerState) || (CHARGER_CHARGING_DONE_STA==(bi)->chargerState))
#define TRIGGER_BATT_SHUTDOWN(bi)  ((bi)->isDataValid && CHARGER_BATT_STA==(bi)->chargerState && (bi)->intBattState==SHUTDOWN_BATT_STA) //critical or shutdown level
#define UI_BATT_CRITICAL(bi)       ((bi)->isDataValid && CHARGER_BATT_STA==(bi)->chargerState && (bi)->intBattState>=CRITICAL_BATT_STA) //critical level only

#define MAINAPP_PAT_AUXIN(bi)         ( UI_BATT_CRITICAL(bi) ? SYS_ON_CRITICAL_BAT_AUXIN_PAT  : COLOR_BLUE         )
#define MAINAPP_PAT_SYS_ON_AP(bi)     ( UI_BATT_CRITICAL(bi) ? SYS_ON_CRITICAL_BAT_AP_PAT     : COLOR_WHITE        )
#define MAINAPP_PAT_SYS_ON_DIRECT(bi) ( UI_BATT_CRITICAL(bi) ? SYS_ON_CRITICAL_BAT_DIRECT_PAT : COLOR_PURPLE       )
#define MAINAPP_PAT_DIRECT_INIT(bi)   ( UI_BATT_CRITICAL(bi) ? DIRECT_CRITICAL_BAT_INIT_PAT   : DIRECT_INIT_PAT    )
#define MAINAPP_PAT_NET_CONN(bi)      ( UI_BATT_CRITICAL(bi) ? NET_CONNECTING_CRITICAL_BAT_PAT: NET_CONNECTING_PAT )

#define MAINAPP_DEBUG

/* TODO: move the tracing string into trace.c */
#ifdef MAINAPP_DEBUG
    const static char *mainApp_debug = "[MainApp_Debug] ";
    //const static char *mainApp_entry = " Entry ";
    //const static char *mainApp_exit = " Exit ";
    #define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
    //#define ENTRY_STATE() MAINAPP_DEBUG_MSG("%s%s\n", mainApp_entry, __FUNCTION__)
    //#define EXIT_STATE()  MAINAPP_DEBUG_MSG("%s%s\n", mainApp_exit, __FUNCTION__)
    #define ENTRY_STATE()
    #define EXIT_STATE()
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
    #define MAINAPP_DEBUG_MSG(...)
    #define ENTRY_STATE()
    #define EXIT_STATE()
    #define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME
#endif

extern tPatternData patternConfig[PAT_MAX_NUMBER];

/*****************************************************************
 * Global Variable
 *****************************************************************/
static eFactoryResetCombine MainApp_factoryReset = 0;
static const uint16 MAINAPP_TICK_TIME = GET_TICKS_IN_MS(MAINAPP_TIMEOUT_IN_MS);
/* Internal evt queue */
static QEvt const *eventQue[10];
static cIdleDlg IdleDlg = {{0}};
static cIdleDlg *pIdleDlg = NULL;

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static void MainApp_SendLedReq(cMainApp* me, ePattern patternId)
{
    LedSrv_SetPatt((QActive*)me, ALL_LED, patternId);
}

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            TYM_CLR_BIT(MainApp_factoryReset, POWER_IN_L_HOLD);
            TYM_CLR_BIT(MainApp_factoryReset, POWER_IN_V_L_HOLD);
            break;
        }
        case NET_RESET_KEY:
        {
            TYM_CLR_BIT(MainApp_factoryReset, NET_RESET_IN_L_HOLD);
            TYM_CLR_BIT(MainApp_factoryReset, NET_RESET_IN_V_L_HOLD);
            break;
        }
        case VOLUME_DOWN_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(0 >= vol)
            {
                MainApp_SendLedReq(me, VOL_MIN_PAT);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(MAX_VOLUME <= vol)
            {
                MainApp_SendLedReq(me, VOL_MAX_PAT);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(0 < vol)
            {
                vol--;
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            if(0 >= vol)
            {
                MainApp_SendLedReq(me, VOL_MIN_PAT);
            }

            break;
        }
        case VOLUME_UP_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(MAX_VOLUME > vol)
            {
                vol++;
                AudioSrv_SetVolume(vol);
            }
            if(MAX_VOLUME <= vol)
            {
                vol = MAX_VOLUME;
                MainApp_SendLedReq(me, VOL_MAX_PAT);
            }
            break;
        }
        case NEXT_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_NEXT);
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_PLAY_PAUSE);
            break;
        }
        case PREV_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_PREV);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            TYM_SET_BIT(MainApp_factoryReset, POWER_IN_L_HOLD);
            break;
        }
        case NET_RESET_KEY:
        {
            TYM_SET_BIT(MainApp_factoryReset, NET_RESET_IN_L_HOLD);
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}

static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(0 >= vol)
            {
                MainApp_SendLedReq(me, VOL_MIN_OFF_PAT);
            }
            else
            {
                vol = (vol < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                    0 : (vol - MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(MAX_VOLUME <= vol)
            {
                vol = MAX_VOLUME;
                MainApp_SendLedReq(me, VOL_MAX_OFF_PAT);
            }
            else
            {
                vol = ((MAX_VOLUME - vol) < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                    MAX_VOLUME : (vol + MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
            TYM_SET_BIT(MainApp_factoryReset, POWER_IN_V_L_HOLD);
            break;
        case NET_RESET_KEY:
            TYM_SET_BIT(MainApp_factoryReset, NET_RESET_IN_V_L_HOLD);
            break;
        /*if necessary, add the handler for more keys here */
        default:break;
    }
    if (MainApp_factoryReset == FACTORY_RESET_TRIGGER)
    {
        AllPlaySrv_resetToFactory();
        MainApp_SendLedReq(me, RST_IN_PROG_PAT);
    }
    else if(MainApp_factoryReset == (NET_RESET_IN_L_HOLD | NET_RESET_IN_V_L_HOLD))
    {
        TYM_CLR_BIT(MainApp_factoryReset, NET_RESET_IN_L_HOLD);
        TYM_CLR_BIT(MainApp_factoryReset, NET_RESET_IN_V_L_HOLD);
        AllPlaySrv_netReset();
    }
}

static void MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            MainApp_KeyUpEvtAction(me, evt);
            break;
        case KEY_EVT_DOWN:
            MainApp_KeyDownEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            MainApp_KeyLPressEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            MainApp_KeyVLPressEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            MainApp_KeyVeryLHoldEvtAction(me, evt);
            break;
        default:break;
    }
}

static void MainApp_AllPlayKeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_SHORT_PRESS:
        {
            switch(evt->keyId)
            {
                case VOLUME_DOWN_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(0 < vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol-1);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                case VOLUME_UP_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(MAX_VOLUME > vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol+1);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case KEY_EVT_REPEAT:
        {
            switch(evt->keyId)
            {
                case VOLUME_DOWN_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    /* update allplay for the mobile app. vol. bar display */
                    vol = (vol < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                        0 : (vol - MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                    uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol);
                    AllPlaySrv_SetVol(allplayVol);
                    break;
                }
                case VOLUME_UP_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(MAX_VOLUME > vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        vol = ((MAX_VOLUME - vol) < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                            MAX_VOLUME : (vol + MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case KEY_EVT_VERY_LONG_HOLD:
        {
            switch(evt->keyId)
            {
                case NET_RESET_KEY:
                {
                    MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_UNCONFIGURED);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}

//Display on powering up/down
static bool MainApp_LedDisplayBatteryState(cMainApp * const me, tBatteryInfo *bi)
{
    bool ret= TRUE;

    /* We does not consider (CHARGER_ERROR_STA==chargerState) condition
     * becuase it will cause isDataValid=FALSE
     */
    if( bi->isDataValid )
    {
        switch(bi->intBattState)
        {
            case FULL_BATT_STA:
            {
                MainApp_SendLedReq(me, BAT_FULL_PAT); //next: LED_STATE_SIG
            }
            break;
            case NORM_BATT_STA:
            {
                MainApp_SendLedReq(me, BAT_NORMAL_PAT); //next: LED_STATE_SIG
            }
            break;
            case LOW_BATT_STA:
            {
                MainApp_SendLedReq(me, BAT_LOW_PAT); //next: LED_STATE_SIG
            }
            break;

            //Critical and shutdown state use the same LED pattern
            case CRITICAL_BATT_STA:
            case SHUTDOWN_BATT_STA:
            {
                MainApp_SendLedReq(me, BAT_CRITICAL_PAT); //next: LED_STATE_SIG
            }
            break;

            default:
            {
                ASSERT(0);
                ret= FALSE;
                break;
            }
	    }
    }
    else
    {
        /* Battery is invalid may be due to:
         * 1. Reboot after upgrade/factory reset ==> battery information is not ready here
         * 2. No/Broken battery
         * In these two case, we should not show LED of battery level.
         */
        ret= FALSE;
    }
    return ret;
}

//Display on standby mode
static void MainApp_LedDisplayChargerState(cMainApp * const me, eChargerState chargerState)
{
    switch(chargerState)
    {
        case CHARGER_BATT_STA:
            MainApp_SendLedReq(me, OFF_PATT);
            break;
        case CHARGER_ERROR_STA:
            MainApp_SendLedReq(me, HW_ERR_PAT);
            break;
        case CHARGER_CHARGING_STA:
            MainApp_SendLedReq(me, COLOR_AMBER);
            break;
        case CHARGER_CHARGING_DONE_STA:
            MainApp_SendLedReq(me, COLOR_GREEN);
            break;
        default:
            ASSERT(0);
            break;
    }
}


static void MainApp_DisplayAllplaySystemMode(cMainApp * const me, enum allplay_system_mode_value currentSystemMode)
{
    tBatteryInfo *bi= (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
    TP_PRINTF("[%s] mode %d\r\n", __func__, currentSystemMode);
    switch(currentSystemMode)
    {
        case ALLPLAY_SYSTEM_MODE_CONFIGURED:
            MainApp_SendLedReq(me, MAINAPP_PAT_SYS_ON_AP(bi));
            break;
        case ALLPLAY_SYSTEM_MODE_CONFIGURING:
            //do nothing here
            break;
        case ALLPLAY_SYSTEM_MODE_DIRECT:
            MainApp_SendLedReq(me, MAINAPP_PAT_SYS_ON_DIRECT(bi));
            break;
        case ALLPLAY_SYSTEM_MODE_UNCONFIGURED:
            MainApp_SendLedReq(me, MAINAPP_PAT_NET_CONN(bi));
            break;
        case ALLPLAY_SYSTEM_MODE_UPDATING:
            MainApp_SendLedReq(me, DFU_PROG_PAT);
            break;
        default:
            ASSERT(0);
            break;
    }
}

static void MainApp_ParseAllPlayState(cMainApp * const me, QEvt const * const e)
{
    AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;

    switch(allPlayEvt->allPlayState)
    {
        /*if necessary, add the handler for more allPlayState here */
        case ALLPLAY_STATE_MCU_SAM_CONNECTED:
            MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_CONFIGURED);
            break;
        case ALLPLAY_STATE_MCU_SAM_DISCONNECTED:
            break;
        case ALLPLAY_STATE_SYSTEM_MODE_CHANGED:
            MainApp_DisplayAllplaySystemMode(me, allPlayEvt->payload.currentSystemMode);
            break;
        case ALLPLAY_STATE_VOLUME_INFO:
        {   /* re-sync sam's volume to mcu's volume */
            uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
            int     volCurr     = allPlayEvt->payload.volChangeInfo.volume;
            uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);

            me->allplayMaxVol = allPlayEvt->payload.volChangeInfo.max_volume;
            if(allplayVol != volCurr)
            {
                AllPlaySrv_SetVol(allplayVol);
            }
            break;
        }
        case ALLPLAY_STATE_VOLUME_CHANGED:
        {
            uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
            int     volCurr     = allPlayEvt->payload.volChangeInfo.volume;
            uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);

            me->allplayMaxVol = allPlayEvt->payload.volChangeInfo.max_volume;
            if(allplayVol != volCurr)
            {
                vol  = MainApp_allplayVol2McuVol(volCurr);
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        default:
            break;
    }
}

static QStateHandler* MainApp_ParseAudioState(cMainApp * const me, QEvt const * const e)
{
    AudioStateEvt* pAudioStateEvt = (AudioStateEvt*) e;
    if(pAudioStateEvt->IsJackIn == 1)
    {
        AudioSrv_SendSwitchChannelReq((QActive*)me, AUDIO_CHANNEL_1);
        return ((QStateHandler*)&MainApp_Auxin);
    }
    else
    {
        AudioSrv_SendSwitchChannelReq((QActive*)me, AUDIO_CHANNEL_0);
        return ((QStateHandler*)&MainApp_AllPlay);
    }
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
    QS_OBJ_DICTIONARY(MainApp_PreActive);

    (void)e; /* suppress the compiler warning about unused parameter */
    /* Subsrcribe to all the SIGS */
    QActive_subscribe((QActive*)me, POWER_STATE_SIG);
    QActive_subscribe((QActive*)me, ALLPLAY_STATE_SIG);
    QActive_subscribe((QActive*)me, AUDIO_STATE_SIG);
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_BATT_STATE_SIG);
    QActive_subscribe((QActive*)me, POWER_WAKE_UP_SIG);
    QActive_subscribe((QActive*)me, LED_STATE_SIG);

    return Q_TRAN(&MainApp_PreActive);
}

static QState MainApp_PreActive(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();            
            me->allplayMaxVol = ALLPLAY_MAX_VOLUME;
            /* send request to power server for charging status */
            //Johnny: replace sending signal to power server by read setting in timeout signal
            PersistantObj_RefreshTick((cPersistantObj*)me, 100); /*set a 100 ms timer*/
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_RESP_SIG", e->sig);
            /* ignore power key detection when after firmware upgrade and factory reset. */
            uint32 nvm_ignore_pwr_key= 0;
            bool nvm_ret= NvmDrv_ReadWord(NVM_STORAGE_ADDR_IGNORE_PWR_KEY, &nvm_ignore_pwr_key);
            if(nvm_ret && nvm_ignore_pwr_key==0x1) //when after firmware upgrade
            {
                NVM_STORAGE_VALUE_CLEAR(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                return Q_TRAN(&MainApp_PoweringUp);
            }
            else
            {
                tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                if( TRIGGER_SLEEP(bi) )
                {
                    return Q_TRAN(&MainApp_Sleep);
                }
                else
                {
                    return Q_TRAN(&MainApp_Standby);
                }
            }
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Standby state*/
static QState MainApp_Standby(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
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
            return Q_HANDLED();
        }
        case POWER_BATT_STATE_SIG:
        {   /* receive the charging state signal from Power server */
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo ;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            MainApp_LedDisplayChargerState(me, bi->chargerState);
            if( TRIGGER_SLEEP(bi) )
            {
                return Q_TRAN(&MainApp_Sleep);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_SHORT_PRESS == (evt->keyEvent) || KEY_EVT_LONG_PRESS == (evt->keyEvent)) &&
                POWER_KEY == (evt->keyId))
            {
                return Q_TRAN(&MainApp_PoweringUp);
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
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Sleep state*/
static QState MainApp_Sleep(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            MainApp_SwitchMode(me, SLEEP_MODE); //next: SYSTEM_MODE_RESP_SIG

            /* Known issue [IN:0061263]:
             *   When MainApp_Sleep is waiting for SYSTEM_MODE_RESP_SIG(SLEEP_MODE), and user press power key at this moment,
             *   MainApp will transfer to MainApp_PoweringUp and send SYSTEM_MODE_RESP_SIG(POWERINGUP_MODE) again.
             *   Then assert occurs: controller.c [178]
             */
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( SLEEP_MODE != evt->modeId ) {
                //ASSERT(0);
            }
            else {
                tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                if( TRIGGER_SLEEP(bi) )
                {
                    MainApp_LedDisplayChargerState(me, bi->chargerState);
                    Application_SendSigToServer((cApplication *)me, POWER_SRV_ID, POWER_MCU_SLEEP_SIG);
                }
                else
                {
                    return Q_TRAN(&MainApp_Standby);
                }
            }
            return Q_HANDLED();
        }
        case POWER_WAKE_UP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_WAKE_UP_SIG", e->sig);

            PowerSrvWakeUpEvent* pEvt = (PowerSrvWakeUpEvent*) e;
            if(pEvt->powerSrvWakeUpType == AC_PLUG_IN_WAKE_UP_TYPE )
            {
                return Q_TRAN(&MainApp_Standby);
            }
            else if(pEvt->powerSrvWakeUpType == AC_PLUG_OUT_WAKE_UP_TYPE )
            {
                MainApp_SendLedReq(me, OFF_PATT);
                Application_SendSigToServer((cApplication *)me, POWER_SRV_ID, POWER_MCU_SLEEP_SIG);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);

            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_SHORT_PRESS == (evt->keyEvent) || KEY_EVT_LONG_PRESS == (evt->keyEvent)) &&
                POWER_KEY == (evt->keyId))
            {
                return Q_TRAN(&MainApp_PoweringUp);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Powering Up state */
static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();

            // Set default volume
            uint8 vol = DEFAULT_VOLUME;
            if(Setting_IsReady(SETID_VOLUME))
            {
                // if the volume is larger than default, keep the volume low
                vol = *(uint8*)Setting_Get(SETID_VOLUME);
                if (vol > DEFAULT_VOLUME)
                {
                    vol = DEFAULT_VOLUME;
                }
            }
            Setting_Set(SETID_VOLUME, &vol);

            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS); /*set a 10 sec timer for debug */
            MainApp_SwitchMode(me, POWERING_UP_MODE);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( POWERING_UP_MODE != evt->modeId ) {
                //ignore unexpected SYSTEM_MODE_RESP_SIG (Low battery booting always get here)
                //ASSERT(0);
            }
            else {
                MainApp_SendLedReq(me, OFF_PATT);
                tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);

                if( TRIGGER_BATT_SHUTDOWN(bi) ) {   //Battery voltage is shutdown level w/o AC appear
                    TYMQP_DUMP_QUEUE_WITH_LOG(me, "======> low battery shutdown\r\n\r\n");
                    MainApp_SendLedReq(me, BAT_CRITICAL_PAT); //next: LED_STATE_SIG
                    //return Q_TRAN(&MainApp_Sleep);
                    break;
                }
                else {
                    MainApp_LedDisplayBatteryState(me, bi); //next signal: LED_STATE_SIG
                    return Q_TRAN(&MainApp_Active);
                }
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            return Q_TRAN(&MainApp_Sleep);
        }
        case MAINAPP_TIMEOUT_SIG:
        {   /* system should never come to here in normal cases. */
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            Nvm_StoreExceptionCode(EXCEPTION_CODE_POWERING_UP_TIMEOUT);
            ASSERT(0); /* for debug build */
            return Q_TRAN(&MainApp_Active);/* for release build */
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Powering Down state*/
static QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            PersistantObj_RefreshTick((cPersistantObj*)me, MAINAPP_TIMEOUT_IN_MS); /*set a 10 sec timer for debug */
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
                tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                MainApp_LedDisplayChargerState(me, bi->chargerState);

                /* if there is a charger error status, battery absent is considered *
                * as the reason. So the led indication of bettery level should not be *
                * displayed.*/
                if( !MainApp_LedDisplayBatteryState(me, bi) ) //next: LED_STATE_SIG-step1
                {
                    MainApp_SendLedReq(me, POWER_DOWN_BLINK_PAT); //next: LED_STATE_SIG-step2
                }
            }
            return Q_HANDLED();
        }
        case LED_STATE_SIG:
        {
            LedStateEvt* pLedEvt = (LedStateEvt*) e;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)LED_STATE_SIG patt=%d", e->sig, pLedEvt->patternId);
            /* system will not go to stanby/sleep mode until*
             * completed showing LED indication for powering down */

            if( POWER_DOWN_BLINK_PAT != pLedEvt->patternId ) // step1
            {
                MainApp_SendLedReq(me, POWER_DOWN_BLINK_PAT); //LED_STATE_SIG
            }
            else //step2
            {
                tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                if( TRIGGER_SLEEP(bi) )
                {
                    return Q_TRAN(&MainApp_Sleep);
                }
                else
                {
                    return Q_TRAN(&MainApp_Standby);
                }
            }
            return Q_HANDLED();
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
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


/* Upgrade Mode */
static QState MainApp_Upgrading(cMainApp * const me, QEvt const * const e)
{
    static int32 upgradeTimer = 0;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            /*set a 90 min timer for debug */
            upgradeTimer = MAINAPP_UPGRADE_TIMEOUT_IN_MS;
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
            MainApp_ParseAllPlayState(me,e);
            /* The result of SAM updating is expected. *
            * if SAM updating is successful, mainApp will switch to soft-reset state. *
            * TODO: if SAM updating is failed, mainApp should switch to active state. */
            if( allPlayEvt->allPlayState == ALLPLAY_STATE_SAM_FIRMWARE_UPDATE_SUCCESSFUL )
            {
                NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                return Q_TRAN(&MainApp_SoftReset);
            }
            else
            {
                return Q_TRAN(&MainApp_Active);
            }
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)MAINAPP_TIMEOUT_SIG", e->sig);
            if( 0 > (upgradeTimer -= MAINAPP_TIMEOUT_IN_MS))
            {
                /* system should never come to here in normal cases. */
                upgradeTimer = 0;
                Nvm_StoreExceptionCode(EXCEPTION_CODE_UPGRADE_TIMEOUT);
                ASSERT(0); /* for debug build */
                ProjBsp_SoftReboot(); /* for release build */
            }
            return Q_HANDLED();
        }
        case POWER_BATT_STATE_SIG:
        {   /* receive the charging state signal from Power server */
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo  ;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            if( TRIGGER_BATT_SHUTDOWN(bi) )
            {   //AC is not exist, and battery is critical level
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "======> low battery shutdown\r\n\r\n");
                return Q_TRAN(&MainApp_PoweringDown);
            }
            return Q_HANDLED();
        }

        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            upgradeTimer = 0;
            QTimeEvt_disarm(TIME_EVT_OF(me));
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Soft Reset state*/
static QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            ENTRY_STATE();
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
                Application_SendSigToServer((cApplication *)me, POWER_SRV_ID, POWER_MCU_RESET_SIG);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state*/
static QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            MainApp_SwitchMode(me, NORMAL_MODE);
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            tBatteryInfo* battInfo = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
            printf("Active timeout signal\r\n");
            PersistantObj_RefreshTick((cPersistantObj*)me, 1000);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_MODE_RESP_SIG", e->sig);
            SwitchModeRespEvt* evt = (SwitchModeRespEvt*)e;
            if( NORMAL_MODE != evt->modeId ) { //ignore unexpected SYSTEM_MODE_RESP_SIG
                //ASSERT(0);
            }
            else {
                BOOL isAuxIn = *(BOOL*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
                if(isAuxIn)
                {
                    AudioSrv_SendSwitchChannelReq((QActive*)me, AUDIO_CHANNEL_1);
                    return Q_TRAN(&MainApp_Auxin);
                }
                else
                {
                    AudioSrv_SendSwitchChannelReq((QActive*)me, AUDIO_CHANNEL_0);
                    return Q_TRAN(&MainApp_AllPlay);
                }
                /* Set volume level */
                AudioSrv_Set(DSP_VOLUME_SETT_ID,TRUE);
                pIdleDlg = &IdleDlg;
                IdleDlg_Ctor(pIdleDlg, (QActive *)me);
            }
            return Q_HANDLED();
        }
        
        case POWER_BATT_STATE_SIG:
        {   /* receive the charging state signal from Power server */
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo  ;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            if( TRIGGER_BATT_SHUTDOWN(bi) ) {   //Battery voltage is shutdown level w/o AC appear
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "======> low battery shutdown\r\n\r\n");
                return Q_TRAN(&MainApp_PoweringDown);
            }
            return Q_HANDLED();
        }
        case AUDIO_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)AUDIO_STATE_SIG", e->sig);
            return Q_TRAN(MainApp_ParseAudioState(me, e));
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

            /* if SAM factory reset is done, mainApp will switch to soft-reset state. */
            if(ALLPLAY_STATE_SAM_FACTORY_RESET_DONE == allPlayEvt->allPlayState)
            {
                NvmDrv_EraseAll(NULL);
                NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                return Q_TRAN(&MainApp_SoftReset);
            }
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            MainApp_KeyHandler(me,e);
            /* A temp workaround for transfering to powering down mode *
              * TODO: fix the issue later.*/
            if(KEY_EVT_SHORT_PRESS == (evt->keyEvent) &&
                POWER_KEY == (evt->keyId))
            {
                return Q_TRAN(&MainApp_PoweringDown);
            }

            /*TODO: The below code should be removed after QualComm fixing the issue *
            * about no response for factory reset*/
            if (MainApp_factoryReset == FACTORY_RESET_TRIGGER)
            {
                TYM_CLR_BIT(MainApp_factoryReset, FACTORY_RESET_TRIGGER);
                NvmDrv_EraseAll(NULL);
                NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
                return Q_TRAN(&MainApp_SoftReset);
            }
            return Q_HANDLED();
        }
        case IDLE_TIMEOUT_SIG:
        {
            MAINAPP_DEBUG_MSG(" idleTimer is timeout now. System is going to sleep now!!!  \n");
            return Q_TRAN(&MainApp_PoweringDown);
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
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Auxin state*/
static QState MainApp_Auxin(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            tBatteryInfo *bi = (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
            MainApp_SendLedReq(me, MAINAPP_PAT_AUXIN(bi));
            return Q_HANDLED();
        }
        case POWER_BATT_STATE_SIG:
        {   /* receive the charging state signal from Power server */
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            MainApp_SendLedReq(me, MAINAPP_PAT_AUXIN(bi) ); //next: N/A
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_SHORT_PRESS == evt->keyEvent) &&
                (((evt->keyId)== NEXT_KEY) || ((evt->keyId)== PREV_KEY)
                || ((evt->keyId)== PLAY_PAUSE_KEY)))
            {
                return Q_HANDLED();
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            QTimeEvt_disarm(TIME_EVT_OF(me));
            EXIT_STATE();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_Active);
}

/* AllPlay state*/
static QState MainApp_AllPlay(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG: //next: Q_INIT_SIG
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            ENTRY_STATE();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: //must alwasy include the Q_TRAN() to designate the default substate of the current state
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_INIT_SIG", e->sig);
            AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            if(TRUE == allPlayInfo->bAllplayConnected) /* MCU is  connected to SAM */
            {
                uint8   vol         = *(uint8*)Setting_Get(SETID_VOLUME);
                uint16  allplayVol  = MainApp_mcuVol2AllplayVol(vol);
                AllPlaySrv_SetVol(allplayVol);
                MainApp_DisplayAllplaySystemMode(me, allPlayInfo->eSystemMode);
                if(ALLPLAY_SYSTEM_MODE_DIRECT == allPlayInfo->eSystemMode)
                {
                    return Q_TRAN(&MainApp_AllPlay_DirectMode);
                }
                else
                {
                    return Q_TRAN(&MainApp_AllPlay_APMode);
                }
            }
            else
            {
                MainApp_SendLedReq(me, POWER_UP_BLINK_PAT);
                return Q_TRAN(&MainApp_AllPlay_APMode);
            }
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            MainApp_ParseAllPlayState(me,e);
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            MainApp_AllPlayKeyHandler(me, e);
            break;
        }
        case Q_EXIT_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            EXIT_STATE();
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
            ENTRY_STATE();
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            if((ALLPLAY_STATE_SYSTEM_MODE_CHANGED == allPlayEvt->allPlayState) &&
                (ALLPLAY_SYSTEM_MODE_DIRECT == allPlayEvt->payload.currentSystemMode))
            {
                MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_DIRECT);
                return Q_TRAN(&MainApp_AllPlay_DirectMode);
            }
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_HOLD == evt->keyEvent) &&
                (DIRECT_MODE_KEY == evt->keyId))
            {
                tBatteryInfo *bi= (tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
                MainApp_SendLedReq(me, MAINAPP_PAT_DIRECT_INIT(bi));
                AllPlaySrv_enableDirectMode(TRUE);
                return Q_TRAN(&MainApp_AllPlay_DirectMode);
            }
            break;
        }
        case POWER_BATT_STATE_SIG:
        {
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            AllPlaySrvInfoEvt *pAllplayInfo = (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
            ASSERT(pAllplayInfo);
            MainApp_DisplayAllplaySystemMode(me, pAllplayInfo->eSystemMode);
            break;
        }
        case Q_EXIT_SIG:
        {
            EXIT_STATE();
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_AllPlay);
}



/* AllPlay direct mode state*/
static QState MainApp_AllPlay_DirectMode(cMainApp * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            ENTRY_STATE();
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            return Q_HANDLED();
        }
        case ALLPLAY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)ALLPLAY_STATE_SIG", e->sig);
            AllPlayStateIndEvt* allPlayEvt = (AllPlayStateIndEvt*) e;
            if((ALLPLAY_STATE_SYSTEM_MODE_CHANGED == allPlayEvt->allPlayState) &&
                (ALLPLAY_SYSTEM_MODE_DIRECT != allPlayEvt->payload.currentSystemMode))
            {
                MainApp_DisplayAllplaySystemMode(me, allPlayEvt->payload.currentSystemMode);
                return Q_TRAN(&MainApp_AllPlay_APMode);
            }
            break;
        }
        case POWER_BATT_STATE_SIG:
        {
            tBatteryInfo *bi = &((PowerSrvInfoEvt*)e)->batteryInfo  ;
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)POWER_BATT_STATE_SIG ch:%d, batt:V=%d,L=%d", e->sig,  bi->chargerState, bi->isDataValid, bi->intBattState);
            MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_DIRECT);
            break;
        }
        case KEY_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)KEY_STATE_SIG", e->sig);
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((KEY_EVT_HOLD == evt->keyEvent) &&
                (DIRECT_MODE_KEY == evt->keyId))
            {
                MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_CONFIGURED);
                AllPlaySrv_enableDirectMode(FALSE);
                return Q_TRAN(&MainApp_AllPlay_APMode);
            }
            break;
        }
        case Q_EXIT_SIG:
        {
            EXIT_STATE();
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&MainApp_AllPlay);
}

