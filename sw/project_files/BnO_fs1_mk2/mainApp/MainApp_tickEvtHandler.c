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
#include "AseNgSrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "bl_common.h"

static QState MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_ErrorUiCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_ErrorSafetyCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_EnterStorageModeTimeoutHandler(cMainApp * const me, QEvt const * const e);
static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e);

#ifdef BnO_fs1
static QState MainApp_PowerStatusUpdaetTimeoutHandler(cMainApp * const me, QEvt const * const e);
#endif

static QState MainApp_SysHealthCheckTimeoutHandler(cMainApp * const me, QEvt const * const e);

#ifdef HAS_DYNAMIC_MEMORY_CONTROL
static QState MainApp_DynamicAnalyzeTimeoutHandler(cMainApp * const me, QEvt const * const e);
#endif

#ifdef  DEBUG_DEMO_FEATURE 
static QState MainApp_DoublePressFactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e);
#endif


/*****************************************************************
 * Global Variable
 *****************************************************************/
/*  Default value of tick handler list */
static tTickHandlerList tickHandlerlist[TIMER_ID_MAX] =
{
    {0, (TickHandler)MainApp_AseTkBootingTimeoutHandler},       //TIMER_ID_ASE_TK_BOOTING_TIMEOUT
    {0, (TickHandler)MainApp_DelayedErrorRebootTimeoutHandler}, //TIMER_ID_DELAYED_ERROR_REBOOT
    {0, (TickHandler)MainApp_UpgradeTimeoutTimerHandler},       //TIMER_ID_UPGRADE_TIMEOUT
    {0, (TickHandler)MainApp_SwitchChannelTimerHandler},        //TIMER_ID_SW_CH
    {0, (TickHandler)MainApp_FactoryResetTimeoutHandler},       //TIMER_ID_FACTORY_RESET_TIMEOUT
    {0, (TickHandler)MainApp_PowerDownTimeoutHandler},          //TIMER_ID_POWER_DOWN_TIMEOUT
    {0, (TickHandler)MainApp_ErrorUiCheckTimeoutHandler},       //TIMER_ID_ERROR_UI_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_ErrorSafetyCheckTimeoutHandler},   //TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT
    {0, (TickHandler)MainApp_EnterStorageModeTimeoutHandler},   //TIMER_ID_ENTER_STORAGE_MODE_TIMEOUT
    {0, (TickHandler)MainApp_VolumeChangeTimeoutHandler},       //TIMER_ID_VOLUME_CHANGE_TIMEOUT
 
#ifdef BnO_fs1    
    {0, (TickHandler)MainApp_PowerStatusUpdaetTimeoutHandler},  //TIMER_ID_POWER_STATUS_UPDATE_TIMEOUT
#endif

    {0, (TickHandler)MainApp_SysHealthCheckTimeoutHandler},     //TIMER_ID_SYS_HEALTH_CHECK_TIMEOUT
    
#ifdef HAS_DYNAMIC_MEMORY_CONTROL
    {0, (TickHandler)MainApp_DynamicAnalyzeTimeoutHandler},     //TIMER_ID_DYNAMIC_ANALYZE_TIMEOUT
#endif    
    
#ifdef  DEBUG_DEMO_FEATURE 
    {0, (TickHandler)MainApp_DoublePressFactoryResetTimeoutHandler},     //TIMER_ID_DOUBLE_PRESS_RESET_TIMEOUT         
#endif            
};


/*****************************************************************
 * Function Implemenation
 *****************************************************************/
tTickHandlerList* MainApp_GetTickHandlerList(void)
{
    return tickHandlerlist;
}


static QState MainApp_ErrorUiCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    uint32 errorReason= 0;
    uint32 errorReasonPrev= 0;
    bool amp_health= TRUE;
    bool connected572= TRUE;  //for production test
    bool connected360= TRUE;  //for production test
    uint16 tempLevelAudio= TL_NORMAL;
#ifdef BnO_fs1
    uint8 chargeStatus= CHARGER_STA_CHARGING;
    uint16 battTempLevel= TL_NORMAL;
    bool battExist= FALSE;
    //uint8 battHealthLevel= BATT_HEALTH_UNKNOWN;
#endif   
    
    //Repeatedly execute
    me->tickHandlers[TIMER_ID_ERROR_UI_CHECK_TIMEOUT].timer = MAINAPP_ERROR_UI_CHECK_TIMEOUT_IN_MS;
      
    //Get information for error    
    connected572=   *(bool*)Setting_GetEx(SETID_IQS572_CONNECTED, &connected572);
    connected360=   *(bool*)Setting_GetEx(SETID_IQS360A_CONNECTED, &connected360);
    tempLevelAudio= *(uint16*)Setting_GetEx(SETID_TEMP_LEVEL_AUDIO, &tempLevelAudio);
    amp_health=     *(bool*)Setting_GetEx(SETID_AMP_HEALTH, &amp_health);

    if(!connected572)
        TYM_SET_BIT(errorReason, (1<<0) );
    if(!connected360)
        TYM_SET_BIT(errorReason, (1<<1) );
    if(tempLevelAudio<=TL_SERIOUS)
        TYM_SET_BIT(errorReason, (1<<2) );

#ifdef BnO_fs1
    chargeStatus=    *(uint8*)Setting_GetEx(SETID_CHARGER_STATUS, &chargeStatus);
    battExist=       *(bool*)Setting_GetEx(SETID_BATTERY_EXIST, &battExist);
    //battHealthLevel= *(uint8*)Setting_GetEx(SETID_BATTERY_HEALTH_LEVEL, &battHealthLevel);
    battTempLevel=   *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP_LEVEL, &battTempLevel);
    
    if(chargeStatus==CHARGER_STA_ERROR)
        TYM_SET_BIT(errorReason, (1<<3) );
    if(!battExist)
        TYM_SET_BIT(errorReason, (1<<4) );
    if(battTempLevel<=TL_SERIOUS)
        TYM_SET_BIT(errorReason, (1<<5) );
#endif

    
    //Update Error LED
    errorReasonPrev= *(uint32*)Setting_GetEx(SETID_ERROR_REASON, &errorReasonPrev);
    if(errorReason!=errorReasonPrev)
    {
        Setting_Set(SETID_ERROR_REASON, &errorReason);
        
#ifdef BnO_fs1
        TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: error=0x%x (amp_health=%d, battExist=%d, ch=%d, tempLevelAudio=[L%d],battTempLevel=[L%d],572=%d,360=%d) ***\r\n\r\n\r\n", 
                  errorReason, amp_health, battExist, chargeStatus, tempLevelAudio, battTempLevel, connected572, connected360);
#else        
        TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: error=0x%x (amp_health=%d, tempLevelAudio=[L%d],572=%d,360=%d) ***\r\n\r\n\r\n", 
                  errorReason, amp_health, tempLevelAudio, connected572, connected360);
#endif        
        if(errorReason)
        {
            MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE); //stop connective LED
            MainApp_SendLedReq(me, LED_IND_ID_ALL_FG_OFF, /*force:*/TRUE); //stop connective LED
            MainApp_SendLedReq(me, LED_IND_ID_ERROR, /*force:*/TRUE); //flash red on product LED
            me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_CONN_RED;
            me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_PROD_RED;
        }
        else
        {
            MainApp_SendLedReq(me, LED_IND_ID_ALL_BG_OFF, /*force:*/TRUE); //stop connective LED
            MainApp_SendLedReq(me, LED_IND_ID_ALL_FG_OFF, /*force:*/TRUE); //stop connective LED
            MainApp_UpdateProdLed(me);
            MainApp_UpdateConnLed(me);
        }           
    }

    return Q_UNHANDLED(); //do not transit state
}



static QState MainApp_ErrorSafetyCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret= Q_UNHANDLED();
    
#ifdef USE_OVERHEAT_PROTECT
    uint16 tempLevelAudio= TL_NORMAL;
    
    //Repeatedly execute
    me->tickHandlers[TIMER_ID_ERROR_SAFETY_CHECK_TIMEOUT].timer = MAINAPP_ERROR_SAFETY_CHECK_TIMEOUT_IN_MS;
      
    //Get information for error
    tempLevelAudio= *(uint16*)Setting_GetEx(SETID_TEMP_LEVEL_AUDIO, &tempLevelAudio);
#ifdef BnO_fs1
    uint16 battTempLevel= TL_NORMAL;
    //battHealthLevel= *(uint8*)Setting_GetEx(SETID_BATTERY_HEALTH_LEVEL, &battHealthLevel);
    battTempLevel=   *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP_LEVEL, &battTempLevel);
    uint16 tempLevelSystem= MIN( tempLevelAudio , battTempLevel );
#else
    uint16 tempLevelSystem= (eTempLevel)tempLevelAudio;
#endif    
    Setting_Set(SETID_TEMP_LEVEL_SYSTEM, &tempLevelSystem);
    

    //Over-heat handling
    if(tempLevelSystem!=me->tempLevelSystem)
    {
        //if(tempLevelSystem==TL_CRITICAL && !(*(bool*)Setting_Get(SETID_SYSTEM_SLEEP)))
        if(tempLevelSystem==TL_CRITICAL)            
        {
            TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: temp level %d -> %d (CRITICAL) ==> Stop charging, Over-heat shutdown ***\r\n\r\n\r\n", me->tempLevelSystem, tempLevelSystem);
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, TRUE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier
            PowerSrv_Set((QActive *)me, POWER_SET_ID_OVERHEAT, /*overheat*/TRUE); //stop charging

            //Power down LED
            me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_ERROR;
            me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_ERROR;

            //Set a invalide value, then next boot do find temp change and decide again
            me->tempLevelSystem= TL_NUM;
            
            tempLevelAudio = TL_NUM;
            Setting_Set(SETID_TEMP_LEVEL_AUDIO, &tempLevelAudio);

#ifdef BnO_fs1            
            battTempLevel = TL_NUM;
            Setting_Set(SETID_BATTERY_TEMP_LEVEL, &battTempLevel);
#endif
            
            ret= Q_TRAN(&MainApp_PoweringDown);
        }
        else if(tempLevelSystem==TL_SERIOUS )
        {
            TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: temp level %d to %d (SERIOUS) ==> Stop charging, shutdown amplifier ***\r\n\r\n\r\n", me->tempLevelSystem, tempLevelSystem);
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/TRUE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier
            PowerSrv_Set((QActive *)me, POWER_SET_ID_OVERHEAT, /*overheat*/TRUE); //stop charging
            me->tempLevelSystem= (eTempLevel)tempLevelSystem;
        }
        else if(tempLevelSystem==TL_WARN)
        {
            TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: temp level %d -> %d (WARN) ==> Stop charging ***\r\n\r\n\r\n", me->tempLevelSystem, tempLevelSystem);
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/FALSE, /* NO USED*/0, /* NO USED*/0); //shutdown amplifier
            PowerSrv_Set((QActive *)me, POWER_SET_ID_OVERHEAT, /*overheat*/TRUE); //stop charging
            me->tempLevelSystem= (eTempLevel)tempLevelSystem;
        }
        else //NORMAL, TL_SUBNORMAL, TL_CRITICAL_COLD
        {
            TP_PRINTF("\r\n\r\n*** MainApp_ErrorCheck: temp level %d to %d (NORMAL) ***\r\n\r\n\r\n", me->tempLevelSystem, tempLevelSystem);           
            AudioSrv_SetAudio(AUDIO_OVERHEAT_MODE_ID, /*overheat*/FALSE, /* NO USED*/0, /* NO USED*/0); //wakeup audio
            PowerSrv_Set((QActive *)me, POWER_SET_ID_OVERHEAT, /*overheat*/FALSE); //allow charging
            me->tempLevelSystem= (eTempLevel)tempLevelSystem;
        }
    }

    
    //Amplifier health checking
    bool amp_health= TRUE;
    amp_health= *(bool*)Setting_GetEx(SETID_AMP_HEALTH, &amp_health);
    if( !amp_health
        /* When SigmalStudio program DSP flow, sometimes Amplifier failture pin report fail,
         * thus we disable amplifier failture shutdown on DSP-TUNING mode
         */
        && me->audioMode!=AUDIO_MODE_DSP_ONLINE_TUNING 
    )
    {
        //Set a valide value, then next boot will check amplifier health again
        amp_health= TRUE;
        Setting_Set(SETID_AMP_HEALTH, &amp_health);
        
        TP_PRINTF("\r\n\r\n\r\n*** Amplifier failture shutdown!!! ***\r\n\r\n\r\n");
        me->ledPowerDownConnId= LED_IND_ID_POWERING_DOWN_ERROR;
        me->ledPowerDownProdId= LED_IND_ID_POWERING_DOWN_ERROR;
        return Q_TRAN(&MainApp_PoweringDown);
    }
            
#endif /* #ifdef USE_OVERHEAT_PROTECT */

    return ret;
}



static QState MainApp_EnterStorageModeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    TP_PRINTF("\r\n\r\n*** MainApp_EnterStorageModeTimeoutHandler: ASE-TK forget to reply AseFepEvent_Event_SYSTEM_STATUS_STORAGE ***\r\n"); 
    ASSERT(0);

    //TODO: enter storage mode, do not wait ASE-TK

    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_VolumeChangeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_VOLUME_CHANGE_TIMEOUT].timer = MAINAPP_VOLUME_CHANGE_TIMEOUT_IN_MS;
    
    if(me->relativeVol >= VOLUME_SEND_TO_ASETK) 
    {
        AseNgSrv_SendFepAseCmdRelativeVol(me->relativeVol);
        me->relativeVol= 0;
    }
    else if(me->relativeVol <= -VOLUME_SEND_TO_ASETK) 
    {
        AseNgSrv_SendFepAseCmdRelativeVol(me->relativeVol);		
        me->relativeVol= 0;		
    }
    
    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_DelayedErrorRebootTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    MainApp_WriteBootRequest(me, BOOT_REQ_POWER_UP);    
    BSP_SoftReboot(); /* for release build */
    return Q_UNHANDLED(); //do not transit state
}

static QState MainApp_AseTkBootingTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if sam is booting time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_ASE_BOOTING_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */

    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_UpgradeTimeoutTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* The function will be called if system is upgrading time out. */
    //Nvm_StoreExceptionCode(REBOOT_CODE_UPGRADE_TIMEOUT);
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */

    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_SwitchChannelTimerHandler(cMainApp * const me, QEvt const * const e)
{
    /* Unmute system after switch audio channel.*/
    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_MUTE, FALSE);

    return Q_UNHANDLED(); //do not transit state
}


//Timeout: MAINAPP_FACTORY_RESET_TIMEOUT_MS
static QState MainApp_FactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */

    return Q_UNHANDLED(); //do not transit state
}


static QState MainApp_PowerDownTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    ASSERT(0); /* for debug build */
    MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    return Q_UNHANDLED(); //do not transit state
}


#ifdef BnO_fs1
static QState MainApp_PowerStatusUpdaetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    me->tickHandlers[TIMER_ID_POWER_STATUS_UPDATE_TIMEOUT].timer = MAINAPP_POWER_STATUS_UPDATE_TIMEOUT_IN_MS;
    AseNgSrv_SendPowerStatus();
    return Q_UNHANDLED(); //do not transit state
}
#endif


//void stack_overflow_test(int level)
//{
//    char buf[100];
//    memset(buf, 0, sizeof(buf));    
//    if(level>0)
//    {
//        stack_overflow_test(level-1);
//    }
//}


static QState MainApp_SysHealthCheckTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    //Repeatedly execute
    me->tickHandlers[TIMER_ID_SYS_HEALTH_CHECK_TIMEOUT].timer = MAINAPP_SYS_HEALTH_CHECK_TIMEOUT_IN_MS;

    bool stackOverflow= stackOverflowCheck();
    ASSERT(!stackOverflow);
    if( stackOverflow )
    {
        TP_PRINTF("\r\n\r\n\r\n *** ERROR: stack overflow ***\r\n\r\n\r\n\r\n");
        MainApp_DelayedErrorReboot(me); /* for release build: flash red LED then reboot */
    }

    uint32 stack_max_usage= stackMaxUsage();
    const uint32 stack_size= stackSize();
    uint32 stack_usage_percent= stack_max_usage*100/stack_size;
    Setting_Set(SETID_MAX_STACK_USAGE, &stack_max_usage);
    //_PRINTF("\r\nMax stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", stack_max_usage, stack_size, stack_usage_percent);
    if(stack_usage_percent>=STACK_OVERFLOW_HIGH_LEVEL_USAGE_LIMIT)
    {   //debug build
        TP_PRINTF("\r\n\r\n\r\n*** WARNING: stack usage trigger %d%%, please increase stack size ***\r\n\r\n\r\n", STACK_OVERFLOW_HIGH_LEVEL_USAGE_LIMIT);
        ASSERT(0);
    }
    
//    //test stack overflow
//    static int level=18;
//    stack_overflow_test(level++);
//    stack_max_usage= stackMaxUsage();
//    stack_usage_percent= stack_max_usage*100/stack_size;
//    printf("Level=%d, max stack usage: %d bytes / %d bytes (%d%%)\r\n\r\n", level, stack_max_usage, stack_size, stack_usage_percent);
//        
    return Q_UNHANDLED(); //do not transit state
}



#ifdef HAS_DYNAMIC_MEMORY_CONTROL
static QState MainApp_DynamicAnalyzeTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    //Repeatedly execute
    me->tickHandlers[TIMER_ID_DYNAMIC_ANALYZE_TIMEOUT].timer = MAINAPP_DYNAMIC_ANALYZE_IN_MS;
    DynamicAnalysis();
    return Q_UNHANDLED(); //do not transit state
}
#endif



#ifdef  DEBUG_DEMO_FEATURE 
static QState MainApp_DoublePressFactoryResetTimeoutHandler(cMainApp * const me, QEvt const * const e)
{
    //Single press Factory RESET key handler
    //do nothing
    return Q_UNHANDLED(); //do not transit state
}     
#endif  
    
    
    

QState MainApp_ActiveTickEvtHandler(cMainApp * const me, QEvt const * const e)
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
                /* If any tick want to transit state, we handle state transit first,
                 * and pospone next tick handling
                 */
                QState trans_state= me->tickHandlers[i].timerHandler(me, e);
                if(trans_state!=Q_UNHANDLED()) 
                {
                    return trans_state;
                }
            }
            else
            {
                ASSERT(0);
            }
        }
    }
    
    return Q_UNHANDLED();
}

