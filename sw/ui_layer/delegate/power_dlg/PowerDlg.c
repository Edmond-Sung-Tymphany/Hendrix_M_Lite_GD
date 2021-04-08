
/**
*  @file      PowerDlg.c
*  @brief     Source file for Power Delegate class
*  @author    Johnny Fan
*  @date      14-JUL-2014
*  @copyright Tymphany Ltd.
*/

#include "PowerDlg_priv.h"
#include "trace.h"

#ifdef HAS_POWER_DELEGATE

#ifdef Q_SPY
#define CAST_ME cPowerDlg * powerDlg = (cPowerDlg *) me;
#else
#define CAST_ME
#endif

/* Internal evt queue */
static QEvt const *PowerDlgEventQue[3];

enum PowerDlgPriSignals /* idle delegate private signals */
{
    PowerDlg_TIMEOUT_SIG = MAX_SIG,
};

/* the time (ms) per timeout signal */
#define POWER_DLG_TIMEOUT_IN_MS  20
/* the number of ticks for QP to trigger timer out signal*/
const uint16 POWER_DLG_TICK_TIME = GET_TICKS_IN_MS(POWER_DLG_TIMEOUT_IN_MS);

#ifdef POWER_DEBUG
const char* power_mode[] =
{
    "DC9V_MODE",
    "USB5V_CHARGE_INT_BATT_MODE",
    "USB5V_CHARGE_EXT_BATT_MODE",
    "NORMAL_BATTERY_MODE",
    "LOW_BATTERY_MODE",
    "SHUTDOWN_MODE",
    "EMERGENCY_MODE",
    "MAX_POWER_MODE",
};
#endif

/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/

#ifdef HAS_POWER_DELEGATE
/* Start function*/
cPowerDlg * PowerDlg_Ctor(cPowerDlg * me, QActive *ownerObj) /*This Ctor could take parameters*/
{
    me = (cPowerDlg *)CREATE_DLG(me, cPowerDlg, ownerObj, &PowerDlg_Initial);
    QTimeEvt_ctorX(&me->timeEvt, (QActive*)me, PowerDlg_TIMEOUT_SIG, 0);
    Delegate_Start((cDelegate*)me, PowerDlgEventQue, Q_DIM(PowerDlgEventQue));
    ASSERT(me->super_.delegateOwner);
    /* subscribe & initiate*/
    QActive_subscribe((QActive*) me, SYSTEM_STATE_SIG);
    return me;
}
/* Shut down function*/
void PowerDlg_Xtor(cPowerDlg * me)
{
    QTimeEvt_disarm(&me->timeEvt);
    QActive_unsubscribeAll((QActive *)me);  /* unsubscribe from all signals */
    CLR_DLG_OWNER(me);
    /* free / zero any memory */
    DESTROY_DLG(me);
}
#endif /* HAS_POWER_DELEGATE */


/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState PowerDlg_Initial(cPowerDlg * const me)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(PowerDlg);
    QS_OBJ_DICTIONARY(PowerDlg_Active);
    PowerDlg_InitialVariable(me);
    return Q_TRAN(&PowerDlg_Active);
}

static QState PowerDlg_Active(cPowerDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->isEnterEmergencyReq = FALSE;
            me->isExitEmergencyReq = FALSE;
            PowerDlg_SetInputAndBattSource(me, SET_DC_INPUT, SET_INT_BATTERY);
            PowerDlg_RefleshTick(me, POWER_DLG_TICK_TIME);
            return Q_HANDLED();
        }
        case POWER_EMERGENCY_MODE_REQ_SIG:
        {
            PowerDlgSetEvt* req = (PowerDlgSetEvt*)e;
            if(req->cmd == ENTER_EMERGENCY_MODE_CMD)
            {
                me->isEnterEmergencyReq = TRUE;
            }
            PowerDlg_Resp(req->sender, POWER_EMERGENCY_MODE_RESP_SIG, RET_SUCCESS);
            return Q_HANDLED();
        }
        case PowerDlg_TIMEOUT_SIG:
        {
            tBatteryInfo battInfo;
            /* update the batt info */
            battInfo = *(tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
#ifdef POWER_EXT_BATTERY_TONE_ENABLE
            PowerDlg_ParseExtBattStateAndPlayTone(me, &battInfo);
#endif
            PowerDlg_ParseInputSourceAndPlayTone(me, &battInfo);
            /* parse the system power mode */
            PowerDlg_ParsePowerMode(me, &battInfo);
            /* implement power control according to power mode */
            PowerDlg_ImplementPowerMode(me, &battInfo);
#ifdef POWER_DEBUG
            PowerDlg_DebugBatteryInfo(me, &battInfo, FALSE);
#endif
            if(me->powerMode == EMERGENCY_MODE)
            {
                return Q_TRAN(&PowerDlg_Emergency);
            }
            
            PowerDlg_RefleshTick(me, POWER_DLG_TICK_TIME);
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


static QState PowerDlg_Emergency(cPowerDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->isEnterEmergencyReq = FALSE;
            me->isExitEmergencyReq = FALSE;
            PowerDlg_SendAudioMuteSignal(me,TRUE);
            PowerDlg_RefleshTick(me, POWER_DLG_TICK_TIME);
            return Q_HANDLED();
        }
        case POWER_EMERGENCY_MODE_REQ_SIG:
        {
            PowerDlgSetEvt* req = (PowerDlgSetEvt*)e;
            if(req->cmd == EXIT_EMERGENCY_MODE_CMD)
            {
                me->isExitEmergencyReq = TRUE;
            }
            PowerDlg_Resp(req->sender, POWER_EMERGENCY_MODE_RESP_SIG, RET_SUCCESS);
            return Q_HANDLED();
        }
        case PowerDlg_TIMEOUT_SIG:
        {
            if(me->isExitEmergencyReq == TRUE)   //user press exit emergency
            {
                /* turn off the SOL to stop charging ext USB device by internal battery */
                PowerDlg_SetExtraCommand(me, DISABLE_SOL_CMD);
                return Q_TRAN(&PowerDlg_Active);
            }
#ifdef POWER_DEBUG
            tBatteryInfo battInfo;
            battInfo = *(tBatteryInfo*)Setting_Get(SETID_BATT_INFO);
            PowerDlg_DebugBatteryInfo(me, &battInfo, FALSE);
#endif
            PowerDlg_RefleshTick(me, POWER_DLG_TICK_TIME);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            PowerDlg_SendAudioMuteSignal(me,FALSE);
            /* set A_5V_CTL output low when leave emergence mode  */
            PowerDlg_SetExtraCommand(me, CLR_A_5V_CTL_CMD);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*****************************************************************************************************************
 *
 * local functions
 *
 *****************************************************************************************************************/
static void PowerDlg_InitialVariable(cPowerDlg * const me)
{
    me->isExtBatteryDock = FALSE;
    me->isDcPlugIn = TRUE;
    me->isEnterEmergencyReq = FALSE;
    me->isExitEmergencyReq = FALSE;
    me->powerMode = MAX_POWER_MODE;
    Setting_Set(SETID_POWER_MODE, &me->powerMode);
}

static void PowerDlg_RefleshTick(cPowerDlg * const me, const uint16 tickTime)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, tickTime, 0);
}

#ifdef POWER_EXT_BATTERY_TONE_ENABLE
static void PowerDlg_ParseExtBattStateAndPlayTone(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    if(battInfo->extBattState == EJECT_BATT_STA)
    {
        if(me->isExtBatteryDock == TRUE)
        {
            me->isExtBatteryDock = FALSE;
            AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_BAT_EJECT_CMD);
            TP_PRINTF("battery is ejected");
            TP_PRINTF("ext battery voltage (%d)\r\n", battInfo->voltage.extBatteryVol);
        }
    }
    else
    {
        if(me->isExtBatteryDock == FALSE)
        {
            me->isExtBatteryDock = TRUE;
            AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_BAT_DOCK_CMD);
            TP_PRINTF("battery is dock");
            TP_PRINTF("ext battery voltage (%d)\r\n", battInfo->voltage.extBatteryVol);
        }
    }
}
#endif

static void PowerDlg_ParseInputSourceAndPlayTone(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    if(battInfo->inputSourceState.isDcPlugIn)
    {
        if(!me->isDcPlugIn)
        {
            //AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_AC_IN_CMD);
            TP_PRINTF("DC is plugged in with (%d)\r\n", battInfo->inputSourceState.isDcPlugIn);
        }
    }
    me->isDcPlugIn = battInfo->inputSourceState.isDcPlugIn;
}

static void PowerDlg_ParsePowerMode(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    if(battInfo->inputSourceState.isDcPlugIn)
    { // when DC is in, it's in DC9v mode
        me->powerMode = DC9V_MODE;
    }
    else if ((battInfo->inputSourceState.isUsbPlugIn) && (battInfo->extBattState == BATT_95_100_PERT_STA))
    { // only use USB 5V when external battery is full
        me->powerMode = USB5V_CHARGE_INT_BATT_MODE;
    }
    else if (battInfo->inputSourceState.isUsbPlugIn)
    { // only use USB 5V when charging external battery
        me->powerMode = USB5V_CHARGE_EXT_BATT_MODE;
    }
    else
    {
        if((battInfo->intBattState==BATTERY_STATE_MAX)||
           (battInfo->extBattState==BATTERY_STATE_MAX))
        { // if the battery state has comes out when powering up, set normal battery mode
            me->powerMode = NORMAL_BATTERY_MODE;
            return;
        }
#ifdef CUSTOMER_POWER_REQUIREMENT
        if(battInfo->intBattState < BATT_0_15_PERT_STA)
        { // int battery >15%
            me->powerMode = NORMAL_BATTERY_MODE;
        }
        else
        { // int battery <15%
            if(battInfo->extBattState < BATT_0_15_PERT_STA)
            { //ext battery >15%
                me->powerMode = LOW_BATTERY_MODE;
            }
            else
            { //ext battery <15%
                me->powerMode = SHUTDOWN_MODE;
            }
        }
#else
        if((battInfo->intBattState < BATT_26_50_PERT_STA)||(battInfo->extBattState < BATT_26_50_PERT_STA))
        { // int battery >51% || ext battery >51%
            me->powerMode = NORMAL_BATTERY_MODE;
        }
        else if((battInfo->intBattState < BATT_5_15_PERT_STA)||(battInfo->extBattState < BATT_5_15_PERT_STA))
        { // int battery >16% || ext battery >16%
            me->powerMode = LOW_BATTERY_MODE;
        }
        else
        { // int battery <15%  &&  ext battery <15% (or ext battery's out)
            me->powerMode = SHUTDOWN_MODE;
        }
#endif
        if(me->isEnterEmergencyReq)
        { // if user request emergency mode, check battery level and enter if it meets the requirement
            me->powerMode = EMERGENCY_MODE;
        }
    }
    // clear emergency mode req
    me->isEnterEmergencyReq = FALSE;
    Setting_Set(SETID_POWER_MODE, &me->powerMode);
#ifdef POWER_DEBUG
        //print out the message when power mode is changed
        static ePowerMode prePowerMode = MAX_POWER_MODE;
        if(prePowerMode!= me->powerMode)
        {
            prePowerMode = me->powerMode;
            TP_PRINTF("\r\n Power Mode change to %s\r\n", power_mode[me->powerMode]);
            PowerDlg_DebugBatteryInfo(me, battInfo, TRUE);
        }
#endif
}

static void PowerDlg_ImplementPowerMode(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    uint8 maxVol = MAX_VOLUME;
    switch(me->powerMode)
    {
        case DC9V_MODE:
        { // set DC input, also disable int/ext battery to avoid pulling DC voltage
            PowerDlg_SetInputAndBattSource(me, SET_DC_INPUT, SET_NO_BATTERY);
            if(battInfo->extBattState < EJECT_BATT_STA)
            {// Enable TPS54328 (EXT_5V_EN High) to allow DC 9V to charge ext battery
                PowerDlg_SetExtraCommand(me, ENABLE_TPS54328_CMD);
            }
            else
            {// Disable TPS54328 (EXT_5V_EN: Low) if ext batt is out
                PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            }
            break;
        }
        case USB5V_CHARGE_INT_BATT_MODE:
        {   // DC9V_Mic5V_CTL: Low, Mini_5V_CTL: Low,  EXT_BAT: ON,  INT_BAT: OFF
            PowerDlg_SetInputAndBattSource(me, SET_USB_INPUT, SET_EXT_BATTERY);
            // Disable TPS54328 (EXT_5V_EN: Low) to avoid USB 5V to route back to ext battery
            PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            break;
        }
        case USB5V_CHARGE_EXT_BATT_MODE:
        {   // DC9V/Mic5V: Low, Mini_5V_CTL: High,  EXT_BAT: OFF,  INT_BAT: ON
            PowerDlg_SetInputAndBattSource(me, SET_USB_NONE_INPUT, SET_INT_BATTERY);
            // Disable TPS54328 (EXT_5V_EN: Low) to avoid USB 5V to route back to ext battery
            PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            break;
        }
        case NORMAL_BATTERY_MODE:
        {// check and determine which battery source to detect
            PowerDlg_JudgeAndSetBatterySource(me, battInfo);
            PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            break;
        }
        case LOW_BATTERY_MODE:
        {
            PowerDlg_JudgeAndSetBatterySource(me, battInfo);
            /* Disable TPS54328 (EXT_5V_EN pin) to avoid USB 5V to route back to ext battery*/
            PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            break;
        }
        case EMERGENCY_MODE:
        {  /* set internal batt source to avoid current go from ext batt and back to ext USB device*/
            PowerDlg_SetInputAndBattSource(me, SET_NONE_INPUT, SET_INT_BATTERY);
            /* Disable TPS54328 (EXT_5V_EN pin) to avoid USB 5V to route back to ext battery*/
            PowerDlg_SetExtraCommand(me, DISABLE_TPS54328_CMD);
            /* turn on the SOL to charge ext USB device */
            PowerDlg_SetExtraCommand(me, ENABLE_SOL_CMD);
            /* set A_5V_CTL output high  */
            PowerDlg_SetExtraCommand(me, SET_A_5V_CTL_CMD);
            break;
        }
        case SHUTDOWN_MODE:
        {   //no playback, can't charge ext battery.
            // TODO: where to mute the music? Need more work on shutdown mode
            /* comment the below code for now, as customer don't want to shutdown the system*/
            //QEvt* req = Q_NEW(QEvt, POWER_OFF_INDICATE_SIG);
            //QACTIVE_POST(me->super_.delegateOwner, (QEvt*)req, 0);
            break;
        }
        default:
            break;
    }
    Setting_Set(SETID_MAX_VOLUME, &maxVol);
}

static void PowerDlg_SetInputAndBattSource(cPowerDlg * const me, eInputSource inputSource, eBatterySource batterySource)
{
    PowerSrvSetEvt* req = Q_NEW(PowerSrvSetEvt, POWER_SET_SIG);
    req->inputSource = inputSource;
    req->batterySource = batterySource;
    req->extraCommand = IGNORE_EXTRA_CMD;
    req->sender = (QActive*)me;
    SendToServer(POWER_SRV_ID, (QEvt*)req);
}

static void PowerDlg_SetExtraCommand(cPowerDlg * const me, eExtraCommand extraCommand)
{
    PowerSrvSetEvt* req = Q_NEW(PowerSrvSetEvt, POWER_SET_SIG);
    req->inputSource = IGNORE_INPUT_SET;
    req->batterySource = IGNORE_BATTERY_SET;
    req->extraCommand = extraCommand;
    req->sender = (QActive*)me;
    SendToServer(POWER_SRV_ID, (QEvt*)req);
}

static void PowerDlg_JudgeAndSetBatterySource(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    if(battInfo->extBattState != EJECT_BATT_STA)
    {
	// determine which battery to use
        if(PowerDlg_IsIntBatteryVolHigher(me, battInfo))
        {

            PowerDlg_SetInputAndBattSource(me, SET_NONE_INPUT, SET_INT_BATTERY);
        }
        else
        {
            PowerDlg_SetInputAndBattSource(me, SET_NONE_INPUT, SET_EXT_BATTERY);
        }
    }
    else
    {
        PowerDlg_SetInputAndBattSource(me, SET_NONE_INPUT, SET_INT_BATTERY);
    }
}

static bool PowerDlg_IsIntBatteryVolHigher(cPowerDlg * const me, tBatteryInfo* battInfo)
{
    static int8 batteryComparisonCounter = (int8)BATTERY_COMPARISON_LENGTH;
    static bool isIntBattVolHigher = TRUE;
    bool ret;
    if(isIntBattVolHigher)
    {
        if(battInfo->voltage.intBatteryVol < battInfo->voltage.extBatteryVol)
        {
            batteryComparisonCounter--;
            if(batteryComparisonCounter<0)
            {
                isIntBattVolHigher = FALSE;
                batteryComparisonCounter = (int8)BATTERY_COMPARISON_LENGTH;
            }
        }
        else
        {
            batteryComparisonCounter = (int8)BATTERY_COMPARISON_LENGTH;
        }
    }
    else
    {
        if(battInfo->voltage.intBatteryVol > battInfo->voltage.extBatteryVol)
        {
            batteryComparisonCounter--;
            if(batteryComparisonCounter<0)
            {
                isIntBattVolHigher = TRUE;
                batteryComparisonCounter = (int8)BATTERY_COMPARISON_LENGTH;
            }
        }
        else
        {
            batteryComparisonCounter = (int8)BATTERY_COMPARISON_LENGTH;
        }
    }
    ret = isIntBattVolHigher;
    return ret;
}
static void PowerDlg_Resp(QActive* sender, eSignal signal, eEvtReturn  evtReturn)
{
    if(sender)
    {
        PowerDlgSetRespEvt* resp = Q_NEW(PowerDlgSetRespEvt, signal);
        resp->evtReturn = evtReturn;
        QACTIVE_POST(sender, (QEvt*)resp, 0);
    }
}

static void PowerDlg_SendAudioMuteSignal(cPowerDlg * const me, bool mute)
{
    AudioMuteReqEvt* req = Q_NEW(AudioMuteReqEvt, AUDIO_MUTE_SIG);
    req->mute = mute;
    req->sender = (QActive*) me;
    SendToServer(AUDIO_SRV_ID,(QEvt *) req);
}


/*****************************************************************************************************************
 *
 * debug function
 *
 *****************************************************************************************************************/
#ifdef POWER_DEBUG
#define DEBUG_MESSAGE_INTERVAL_MS   3000
const char* int_batt_state[] =
{
    "BATT_95_100_PERT_STA",
    "BATT_76_94_PERT_STA",
    "BATT_51_75_PERT_STA",
    "BATT_26_50_PERT_STA",
    "BATT_16_25_PERT_STA",
    "BATT_5_15_PERT_STA",
    "BATT_0_5_PERT_STA",
    "EJECT_BATT_STA",
    "BATTERY_STATE_MAX",
};

const char* charger_state[] =
{
    "CHARGER_BATT_STA",
    "CHARGER_ERROR_STA",
    "CHARGER_CHARGING_STA",
    "CHARGER_CHARGING_DONE_STA",
    "SHUTDOWN_BATT_STA",
    "CHARGER_STATUS_MAX",
};

const char* dc_state[] =
{
    "ac is plug out",
    "ad is plug in",
};
const char* usb_state[] =
{
    "usb is plug out",
    "usb is plug in",
};

static void PowerDlg_DebugBatteryInfo(cPowerDlg * const me, tBatteryInfo* battInfo, bool sendAtOnce)
{
    static int16 timer = 0;
    timer -=POWER_DLG_TIMEOUT_IN_MS;
    if((timer<= 0) || (sendAtOnce==TRUE))
    {
        timer = DEBUG_MESSAGE_INTERVAL_MS;
        TP_PRINTF ("\r\nInternal Battery voltage is (%d)\r\n",
             battInfo->voltage.intBatteryVol);

        TP_PRINTF ("Internal Battery state is (%s)\r\n",
            int_batt_state[battInfo->intBattState]);

        TP_PRINTF ("External Battery voltage is (%d)\r\n",
            battInfo->voltage.extBatteryVol);
        TP_PRINTF ("External Battery state is (%s)\r\n",
            int_batt_state[battInfo->extBattState]);

        TP_PRINTF ("%s\r\n", dc_state[battInfo->inputSourceState.isDcPlugIn]);
        TP_PRINTF ("%s\r\n", usb_state[battInfo->inputSourceState.isUsbPlugIn]);
        TP_PRINTF ("Charger stat pin (isChargingDone) is (%d)\r\n",
            battInfo->inputSourceState.isChargingDone);

        TP_PRINTF ("Charger state is (%s)\r\n", charger_state[battInfo->chargerState]);
        TP_PRINTF ("Power Mode is (%s)\r\n", power_mode[me->powerMode]);

        uint8 maxVol = *(uint8*)Setting_Get(SETID_MAX_VOLUME);
        TP_PRINTF ("Current max volume is (%d)\r\n", maxVol);
        uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
        TP_PRINTF ("Current volume is (%d)\r\n", vol);
    }

}
#endif // endof #ifdef POWER_DEBUG


#endif //#ifdef HAS_POWER_DELEGATE

