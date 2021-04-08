/**
*  @file      idle_dlg.c
*  @brief     Source file for Idle Delegate class
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*/

#include "BluetoothDlg_priv.h"

#ifdef Q_SPY
#define CAST_ME cBluetoothDlg * bluetoothDlg = (cBluetoothDlg *) me;
#else
#define CAST_ME
#endif

#ifndef BT_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif

/* Internal evt queue */
static QEvt const *BluetoothDlgEventQue[3];

enum BLuetoothDlgPriSignals /* BT delegate private signals */
{
    BluetoothDlg_TIMEOUT_SIG = MAX_SIG,
};

/* the time (ms) per timeout signal */
#define BT_DLG_TIMEOUT_IN_MS  10
/* the number of ticks for QP to trigger timer out signal*/
static const uint16 BT_DLG_TICK_TIME = GET_TICKS_IN_MS(BT_DLG_TIMEOUT_IN_MS);

static tAutoLink autoLink;

static bool bIsCancelDiscoverable;
static bool bIsLinkLoss = FALSE;

static uint8 pairingToneRepeates = 0;
static uint16 pairingToneInterval = 0;

#define PAIR_TONE_REPEAT_TIMES    2       /*play tone 2 times*/
#define PAIR_TONE_INTERVAL        5000    /*delay 5sec to paly tone*/

/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/

#ifdef HAS_BT_DELEGATE
/* Start function*/
cBluetoothDlg * BluetoothDlg_Ctor(cBluetoothDlg * me, QActive *ownerObj) /*This Ctor could take parameters*/
{
    me = (cBluetoothDlg *)CREATE_DLG(me, cBluetoothDlg, ownerObj, &BluetoothDlg_Initial);

    QTimeEvt_ctorX(&me->timeEvt, (QActive*)me, BluetoothDlg_TIMEOUT_SIG, 0);
    Delegate_Start((cDelegate*)me, BluetoothDlgEventQue, Q_DIM(BluetoothDlgEventQue));
    /* subscribe & initiate*/
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*) me, BT_STATE_SIG);
#endif
    return me;
}
/* Shut down function*/
void BluetoothDlg_Xtor(cBluetoothDlg * me)
{
    QTimeEvt_disarm(&me->timeEvt);
    QActive_unsubscribeAll((QActive *)me);  /* unsubscribe from all signals */
    CLR_DLG_OWNER(me);
    /* free / zero any memory */
    DESTROY_DLG(me);
}
#endif /* HAS_IDLE_DELEGATE */

void BluetoothDlg_GoConnectable( QActive * me)
{
    bIsCancelDiscoverable = TRUE;
    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
}


/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState BluetoothDlg_Initial(cBluetoothDlg * const me)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(bluetoothDlg);
    QS_OBJ_DICTIONARY(BluetoothDlg_PreActive);
    QS_OBJ_DICTIONARY(BluetoothDlg_Active);
    return Q_TRAN(&BluetoothDlg_PreActive);
}

static QState BluetoothDlg_PreActive(cBluetoothDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            /* BT variable*/
            me->myBtStatus = BT_MAX_STA;
            /* channel variable*/
            me->isAudioJackIn = FALSE;
            /*Initial auto link variable & LED*/
            autoLink.bIsAutoLinkMode = TRUE;
            autoLink.calLinkCmdInternalTimer = BT_POWER_ON_AUTO_LINKING_INTERNAL_MS;
            autoLink.calTimer = BT_POWER_ON_AUTO_LINKING_TIMEOUT_MS;
            bIsLinkLoss = FALSE;
            return Q_HANDLED();
        }
        case MAINAPP_START_DLG_SIG:
        {
            return Q_TRAN(BluetoothDlg_Active);
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


static QState BluetoothDlg_Active(cBluetoothDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {/* add delay to check aux in for playing power up tone*/
            QTimeEvt_armX(&me->timeEvt, BT_POWER_UP_TONE_DELAY_MS, 0);
            return Q_HANDLED();
        }
        case BT_REQ_SIG:
        { /* put it here to avoid use BT dlg to play tone, debug only, will remove it later*/
            ASSERT(0);
            return Q_HANDLED();
        }
        case BT_STATE_SIG:
        {
            BluetoothDlg_ParseBTEvent(me,e);
            return Q_HANDLED();
        }
        case BluetoothDlg_TIMEOUT_SIG:
        {
            BluetoothDlg_AudioJackDetect(me);
            BluetoothDlg_AudioChannelAutoSwitch(me);
            BluetoothDlg_RefleshTick(me, BT_DLG_TICK_TIME);
            /*Check when to leave auto link LED blinking*/
            if (TRUE==autoLink.bIsAutoLinkMode && me->myBtStatus==BT_CONNECTABLE_STA)
            {
                BluetoothDlg_IsAutoLinkDone(me);
            }
            else if(me->myBtStatus==BT_DISCOVERABLE_STA && pairingToneRepeates>0)
            {
                if(pairingToneInterval<=0)
                {
                    pairingToneInterval = PAIR_TONE_INTERVAL;
                    pairingToneRepeates--;
                    AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIRING_CMD);
                }
                else
                {
                    pairingToneInterval -= BT_DLG_TIMEOUT_IN_MS;
                }
            }
            return Q_HANDLED();
        }
        case MAINAPP_STOP_DLG_SIG:
        {
            return Q_TRAN(BluetoothDlg_PreActive);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static bool BluetoothDlg_IsAutoLinkDone(cBluetoothDlg * const me)
{
    bool ret = FALSE;
    if(autoLink.calTimer>0)
    {
        autoLink.calTimer-=BT_DLG_TIMEOUT_IN_MS;
        if(autoLink.calLinkCmdInternalTimer >0)
        {
            autoLink.calLinkCmdInternalTimer -= BT_DLG_TIMEOUT_IN_MS;
        }
        else
        {
            autoLink.calLinkCmdInternalTimer = BT_POWER_ON_AUTO_LINKING_INTERNAL_MS;
            BluetoothSrv_SendBtCmd((QActive*)me, BT_CONNECT_CMD);
            TP_PRINTF("Send the Connect Command\r\n");
        }
    }
    else
    {
        /*Switch LED to connectable mode - 50% intensity*/
        LedSrv_SetPatt((QActive*)me, 1<<LED_BT, SOLID_DIM_PATT);
        // for test team testing
        DebugSettSrv_PrintStr("BT Autolink finished\r\n");
        autoLink.bIsAutoLinkMode = FALSE;
        ret = TRUE;
    }
    return ret;
}


static void BluetoothDlg_ParseBTEvent(cBluetoothDlg * const me, QEvt const * const e)
{
    static eCallState preCallingState, curCallingState = NO_CALL_STA;
    BtStatusEvt *evt = (BtStatusEvt*)e;
    if(!evt->isBtStatus)
    { /* leave if it's not BT status*/
        switch(evt->btIndEvt)
        {   /*PAIRING FAIL is same as PAIRING TIMEOUT, but different from LINK LOST*/
            case BT_SCL_CONNECTED_EVT:
            {
                pairingToneRepeates = 0;
                bIsLinkLoss = FALSE;
                LedSrv_SetPatt((QActive*)me, 1<<LED_BT, SOLID_ON_PATT);
                AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_CONNECTED_CMD);
                TP_PRINTF("BT_SCL_CONNECTED_EVT\r\n");
                break;
            }
            case BT_PAIRING_FAIL_EVT:
            {
                TP_PRINTF("BT_PAIRING_FAIL_EVT\r\n");
                curCallingState = NO_CALL_STA;
                autoLink.bIsAutoLinkMode = FALSE;
                if(bIsCancelDiscoverable==FALSE)
                {
                    LedSrv_SetPatt((QActive*)me, 1<<LED_BT, BT_PAIRING_FAIL_PATT);
                    AudioSrv_SendAudioSrvToneCmd((QActive*)me, BT_TONE_PAIR_FAIL_CMD);
                }
                else
                {
                    bIsCancelDiscoverable = FALSE;
                }
                break;
            }
            case BT_LINK_LOSS_EVT:
            {
                TP_PRINTF("BT_LINK_LOSS_EVT\r\n");
                bIsLinkLoss = TRUE;
                LedSrv_SetPatt((QActive*)me, 1<<LED_BT, BT_AUTO_LINK_PATT);
                break;
            }
            default:
                break;
        }
        return;
    }
    if ((me->myBtStatus != evt->btStatus))
    {
        switch(evt->btStatus)
        {
            case BT_CONNECTABLE_STA:
            {
                TP_PRINTF("BT_CONNECTABLE_STA\r\n");
                curCallingState = NO_CALL_STA;
                bIsLinkLoss = FALSE;
                if (FALSE == autoLink.bIsAutoLinkMode)
                {
                    LedSrv_SetPatt((QActive*)me, 1<<LED_BT, SOLID_DIM_PATT);
                }
                else if (TRUE == autoLink.bIsAutoLinkMode)
                {
                    LedSrv_SetPatt((QActive*)me, 1<<LED_BT, BT_AUTO_LINK_PATT);
                }
                break;
            }
            case BT_DISCOVERABLE_STA:
            {
                TP_PRINTF("BT_DISCOVERABLE_STA\r\n");
                curCallingState = NO_CALL_STA;
                autoLink.bIsAutoLinkMode = FALSE;
                pairingToneRepeates = PAIR_TONE_REPEAT_TIMES;
                pairingToneInterval = 0;
                bIsLinkLoss = FALSE;
                if(BT_DISCOVERABLE_STA != me->myBtStatus)
                {
                    bIsCancelDiscoverable = FALSE;
                    LedSrv_SetPatt((QActive*)me, 1<<LED_BT, BT_DISCOVERABLE_PATT);
                }
                break;
            }
            case BT_CONNECTED_STA:
            {
                TP_PRINTF("BT_CONNECTED_STA\r\n");
                curCallingState = NO_CALL_STA;
                autoLink.bIsAutoLinkMode = FALSE;
                if(bIsLinkLoss==FALSE)
                {
                    LedSrv_SetPatt((QActive*)me, 1<<LED_BT, SOLID_ON_PATT);
                }
                break;
            }
            case BT_STREAMING_A2DP_STA:
            {
                TP_PRINTF("BT_STREAMING_A2DP_STA\r\n");
                curCallingState = NO_CALL_STA;
                autoLink.bIsAutoLinkMode = FALSE;
                LedSrv_SetPatt((QActive*)me, 1<<LED_BT, SOLID_ON_PATT);
                break;
            }
            case BT_INCOMING_CALL_EST_STA:
            { // the call is coming
                TP_PRINTF("BT_INCOMING_CALL_EST_STA\r\n");
                curCallingState = ONE_CALL_STA;
                LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED), IN_COMING_CALL_PATT);
                LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), IN_COMING_CALL_PATT_GREEN);
                break;
            }
            case BT_ACTIVE_CALL_STA:
            { // the call is established
                TP_PRINTF("BT_ACTIVE_CALL_STA\r\n");
                curCallingState = ONE_CALL_STA;
                if(me->myBtStatus == BT_INCOMING_CALL_EST_STA)
                { /* answer a new call, flash green led */
                    LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), ANSWER_END_CALL_PATT_GREEN);
                }
                LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED), SOLID_ON_PATT);
                LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), SOLID_ON_PATT_GREEN);
                break;
            }
            case BT_TWC_CALL_WAITING_STA:
            { // the 2nd call is coming
                TP_PRINTF("BT_TWC_CALL_WAITING_STA\r\n");
                curCallingState = TWO_CALL_STA;
                LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED), IN_COMING_CALL_PATT);
                LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), IN_COMING_CALL_PATT_GREEN);
                break;
            }
            case BT_TWC_CALL_ON_HOLD_STA:
            { // answer the 3rd call, keep 2nd call on hold
                TP_PRINTF("BT_TWC_CALL_ON_HOLD_STA\r\n");
                if(me->myBtStatus == BT_TWC_CALL_WAITING_STA)
                { /* answer a new call, flash green led */
                    LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), ANSWER_END_CALL_PATT_GREEN);
                }
                LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED), SOLID_ON_PATT);
                LedSrv_SetPatt((QActive*)me, (1<<LED_PLAY_GREEN), SOLID_ON_PATT_GREEN);
                curCallingState = TWO_CALL_STA;
                break;
            }
            case BT_INCOMING_CALL_ON_HOLD_STA:
            { // haven't seen this yet, place holder here
                TP_PRINTF("BT_INCOMING_CALL_ON_HOLD_STA\r\n");
                curCallingState = TWO_CALL_STA;
                break;
            }
            default:
            {
                break;
            }
        }
        me->myBtStatus = evt->btStatus;
    }
    if(curCallingState!=preCallingState)
    {
        if(curCallingState < preCallingState)
        { // end of the call, flash the power red led
            LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED), ANSWER_END_CALL_PATT);
            LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_WHITE)|(1<<LED_PLAY_WHITE), DELAY_KEEP_OFF_PATT);
        }
        if(curCallingState == NO_CALL_STA)
        { /*trun off the red/green led when there's no call*/
            LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_RED)|(1<<LED_PLAY_GREEN), OFF_PATT);
            LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_WHITE)|(1<<LED_PLAY_WHITE), SOLID_ON_PATT);
        }
        else
        { /*trun off the white power play/pause led when there's a call*/
            LedSrv_SetPatt((QActive*)me, (1<<LED_POWER_WHITE)|(1<<LED_PLAY_WHITE), OFF_PATT);
        }
        preCallingState = curCallingState;
    }
    Setting_Set(SETID_CALLING_STATUS, &curCallingState);
}


static void BluetoothDlg_AudioJackDetect(cBluetoothDlg * const me)
{
    if(Setting_IsReady(SETID_IS_AUXIN_PLUG_IN))
    {
        me->isAudioJackIn = *(bool*)Setting_Get(SETID_IS_AUXIN_PLUG_IN);
    }
}

static void BluetoothDlg_AudioChannelAutoSwitch(cBluetoothDlg * const me)
{
    if(!Setting_IsReady(SETID_CHANNEL) || !Setting_IsReady(SETID_CALLING_STATUS))
    {
        return;
    }
    eAudioChannel channel = *(eAudioChannel*)Setting_Get(SETID_CHANNEL);
    eCallState callingState = *(eCallState*)Setting_Get(SETID_CALLING_STATUS);
#ifdef BT_AUDIO_HAS_HIGH_PRIO
    if((me->myBtStatus == BT_STREAMING_A2DP_STA)||(me->isAudioJackIn==FALSE) ||
        callingState !=  NO_CALL_STA)
    { // when there's a calling comming, swithc channel to BT
        if(channel == AUXIN_CHANNEL)
        {
            /* set channel to BT */
            channel = BT_CHANNEL;
            BluetoothDlg_SwitchAudioChannel(me, channel);
        }
    }
    else if(me->isAudioJackIn)
    {
        if(channel == BT_CHANNEL)
        {
            /* set channel to AUX IN */
            channel = AUXIN_CHANNEL;
            BluetoothDlg_SwitchAudioChannel(me, channel);
        }

    }
#endif
#ifdef AUX_IN_HAS_HIGH_PRIO
    if(me->isAudioJackIn)
    {
        if(channel == BT_CHANNEL)
        {
            /* change channel to AUX IN */
            channel = AUXIN_CHANNEL;
            BluetoothDlg_SwitchAudioChannel(me, channel);
        }
    }
    else
    { // AUX is out
        if(channel == AUXIN_CHANNEL)
        {
            /* change channel to BT */
            channel = BT_CHANNEL;
            BluetoothDlg_SwitchAudioChannel(me, channel);
        }
    }
#endif
}

static void BluetoothDlg_SwitchAudioChannel(cBluetoothDlg * const me, eAudioChannel channel)
{
#ifdef HAS_AUDIO_CONTROL
    AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
    audio_switch->channel = channel;
    audio_switch->sender = (QActive *) me;
    SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
#endif
}


static void BluetoothDlg_RefleshTick(cBluetoothDlg * const me, const uint16 tickTime)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, tickTime, 0);
}

