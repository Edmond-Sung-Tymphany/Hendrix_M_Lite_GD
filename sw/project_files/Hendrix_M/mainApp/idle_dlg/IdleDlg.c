/**
*  @file      idle_dlg.c
*  @brief     Source file for Idle Delegate class
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*/

#include "IdleDlg.config"
#include "product.config"
#include "bsp.h"
#include "trace.h"
#include "controller.h"
#include "SettingSrv.h"
#include "IdleDlg_priv.h"
#include "AudioSrv.h"
#include "PowerSrv.h"

#ifdef HAS_IDLE_DELEGATE

#ifdef Q_SPY
#define CAST_ME cIdleDlg * idleDlg = (cIdleDlg *) me;
#else
#define CAST_ME
#endif
/* TODO: move the tracing string into trace.c */
#define IDLEDLG_DEBUG
#ifdef IDLEDLG_DEBUG
#define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
#define IDLEDLG_DEBUG_MSG(...)
#define ENTRY_STATE()
#define EXIT_STATE()
#endif

/* internal signals */
enum IdleDlgPriSignals /* idle delegate private signals */
{
    IDLEDLG_TIMEOUT_SIG = MAX_SIG,
};

#define IDLEDLG_TIMEOUT_IN_MS  1000

static eIdleTimerTriggerSource triggerSource = 0;

/* Internal evt queue */
static QEvt const *idleEventQue[IDLE_EVT_QUE_SIZE];


static void IdleDlg_RefreshTick(cIdleDlg* const me, uint16 TickTimeInterval)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, TickTimeInterval, 0);
}

static void IdleDlg_ResetIdleTimer(cIdleDlg* const me)
{
    me->idleTimer = IDLE_TIMEOUT_IN_MS;
    TYMQP_DUMP_QUEUE_WITH_LOG(me, " idleTimer reset to %d min", IDLE_TIMEOUT_IN_MS/(60*1000));
}

/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/

#ifdef HAS_IDLE_DELEGATE
/* Start function*/
cIdleDlg * IdleDlg_Ctor(cIdleDlg * me, QActive *ownerObj) /*This Ctor could take parameters*/
{
    me = (cIdleDlg *)CREATE_DLG(me, cIdleDlg, ownerObj, &IdleDlg_Initial);
    QTimeEvt_ctorX(&me->timeEvt, (QActive*)me, IDLEDLG_TIMEOUT_SIG, 0);
    Delegate_Start((cDelegate*)me, idleEventQue, Q_DIM(idleEventQue));
    /* subscribe & initiate*/
    return me;
}
/* Shut down function*/
void IdleDlg_Xtor(cIdleDlg * me)
{
    QTimeEvt_disarm(&me->timeEvt);
    QActive_unsubscribeAll((QActive *)me);  /* unsubscribe from all signals */
    CLR_DLG_OWNER(me);
    /* free / zero any memory */
    DESTROY_DLG(me);
}
#endif /* HAS_IDLE_DELEGATE */
/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState IdleDlg_Initial(cIdleDlg * const me)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(idleDlg);
    QS_OBJ_DICTIONARY(IdleDlg_PreActive);
    QS_OBJ_DICTIONARY(IdleDlg_NonIdleMode);
    QS_OBJ_DICTIONARY(IdleDlg_IdleMode);

    /* Subsrcribe to all the SIGS */
#ifdef HAS_BATTERY
    QActive_subscribe((QActive*)me, POWER_BATT_STATE_SIG);
#endif
#ifdef HAS_AUDIO_CONTROL
    QActive_subscribe((QActive*)me, AUDIO_MUSIC_STREAM_STATE_SIG);
#endif
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*)me, BT_STATE_SIG);
#endif
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);

    return Q_TRAN(&IdleDlg_PreActive);
}

static QState IdleDlg_PreActive(cIdleDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            IdleDlg_RefreshTick(me, 1000); /*set a 1 sec timer for debug */
            return Q_HANDLED();
        }
        case IDLEDLG_TIMEOUT_SIG:
        {
#ifdef HAS_BATTERY
            bool isAcPlugIn = FALSE;
            if (Setting_IsReady(SETID_IS_DC_PLUG_IN))
            {
                isAcPlugIn = *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN);
            }
            eChargerState isCharging = CHARGER_STA_MAX;
            if(Setting_IsReady(SETID_CHARGING_STATUS))
            {
                isCharging = *(uint8*)Setting_Get(SETID_CHARGING_STATUS);
            }
            if(FALSE == isAcPlugIn || isCharging == CHARGER_STA_CHARGING_DONE)
            {
                TYM_SET_BIT(triggerSource, BATTERY_MODE);
            }
            else
            {
                TYM_CLR_BIT(triggerSource, BATTERY_MODE);
            }
#endif
#ifdef HAS_AUDIO_CONTROL
            uint8 isMusicPlaying = FALSE;
            if (Setting_IsReady(SETID_MUSIC_STATUS))
            {
                isMusicPlaying = *(uint8*)Setting_Get(SETID_MUSIC_STATUS);
            }

            if (FALSE == isMusicPlaying)
            {
                TYM_SET_BIT(triggerSource, NO_AUDIO_PLAYBACK);
            }
            else
            {
                TYM_CLR_BIT(triggerSource, NO_AUDIO_PLAYBACK);
            }
#endif
#ifdef HAS_BLUETOOTH
            eBtStatus isBTStreaming = BT_MAX_STA;
            if (Setting_IsReady(SETID_BT_STATUS))
            {
                isBTStreaming = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
            }

            if (BT_STREAMING_A2DP_STA == isBTStreaming)
            {
                TYM_CLR_BIT(triggerSource, NO_BT_STREAMING);
            }
            else
            {
                TYM_SET_BIT(triggerSource, NO_BT_STREAMING);
            }
#endif

            if(IDLE_TIMER_TRIGGER == triggerSource)
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Go IDLE", e->sig);
                return Q_TRAN(&IdleDlg_IdleMode);
            }
            else
            {
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Go No_IDLE", e->sig);
                return Q_TRAN(&IdleDlg_NonIdleMode);
            }
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState IdleDlg_NonIdleMode(cIdleDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_BATT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "NoIDLEMode  POWER_BATT_STATE_SIG");
            if(FALSE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN) ||
               CHARGER_STA_CHARGING_DONE == *(uint8*)Setting_Get(SETID_CHARGING_STATUS))
            {
                TYM_SET_BIT(triggerSource, BATTERY_MODE);
                if(IDLE_TIMER_TRIGGER == triggerSource)
                {
                    return Q_TRAN(&IdleDlg_IdleMode);
                }
            }
            else
            {
                TYM_CLR_BIT(triggerSource, BATTERY_MODE);
            }
            return Q_HANDLED();
        }
#endif
#ifdef HAS_AUDIO_CONTROL
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "NoIDLEMode  AUDIO_MUSIC_STREAM_STATE_SIG");
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            if (!(pAudioMusicStateEvt->hasMusicStream))
            {
                TYM_SET_BIT(triggerSource, NO_AUDIO_PLAYBACK);
                if(IDLE_TIMER_TRIGGER == triggerSource)
                {
                    return Q_TRAN(&IdleDlg_IdleMode);
                }
            }
            else
            {
                TYM_CLR_BIT(triggerSource, NO_AUDIO_PLAYBACK);
            }
            return Q_HANDLED();
        }
#endif
#ifdef HAS_BLUETOOTH
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "NoIDLEMode  BT_STATE_SIG");
            BtStatusEvt *evt = (BtStatusEvt*)e;
            if(evt->btStatus > BT_STREAMING_A2DP_STA)
                return Q_HANDLED();
            if (BT_STREAMING_A2DP_STA != evt->btStatus)
            {
                TYM_SET_BIT(triggerSource, NO_BT_STREAMING);
                if(IDLE_TIMER_TRIGGER == triggerSource)
                {
                    return Q_TRAN(&IdleDlg_IdleMode);
                }
            }
            else
            {
                TYM_CLR_BIT(triggerSource, NO_BT_STREAMING);
            }
            return Q_HANDLED();
        }
#endif

        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState IdleDlg_IdleMode(cIdleDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_ENTRY_SIG", e->sig);
            IdleDlg_ResetIdleTimer(me);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "Idle timer is triggered. Idle timeout duration: %d min.", IDLE_TIMEOUT_IN_MS/(60*1000));
            IdleDlg_RefreshTick(me, IDLEDLG_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_BATT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, " IDLEMode  POWER_BATT_STATE_SIG");
            PowerSrvInfoEvt* pBatEvt = (PowerSrvInfoEvt*) e;
            if(TRUE == *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN) &&
               CHARGER_STA_CHARGING_DONE != *(uint8*)Setting_Get(SETID_CHARGING_STATUS))
            {
                TYM_CLR_BIT(triggerSource, BATTERY_MODE);
                return Q_TRAN(&IdleDlg_NonIdleMode);
            }
            return Q_HANDLED();
        }
#endif
#ifdef HAS_AUDIO_CONTROL
        case AUDIO_MUSIC_STREAM_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, " IDLEMode  AUDIO_MUSIC_STREAM_STATE_SIG");
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            if (pAudioMusicStateEvt->hasMusicStream)
            {
                TYM_CLR_BIT(triggerSource, NO_AUDIO_PLAYBACK);
                return Q_TRAN(&IdleDlg_NonIdleMode);
            }
            return Q_HANDLED();
        }
#endif
#ifdef HAS_BLUETOOTH
        case BT_STATE_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "IDLEMode  BT_STATE_SIG");
            BtStatusEvt *evt = (BtStatusEvt*)e;
            //ignore no use bt status
            if( evt->btStatus > BT_STREAMING_A2DP_STA)
                return Q_HANDLED();
            if (BT_STREAMING_A2DP_STA == evt->btStatus)
            {
                TYM_CLR_BIT(triggerSource, NO_BT_STREAMING);
                return Q_TRAN(&IdleDlg_NonIdleMode);
            }
            return Q_HANDLED();
        }
#endif

        case KEY_STATE_SIG:
        {
            IdleDlg_ResetIdleTimer(me);
            return Q_HANDLED();
        }
        case IDLEDLG_TIMEOUT_SIG:
        {
            if((me->idleTimer -= IDLEDLG_TIMEOUT_IN_MS) <= 0)
            {
                if(NULL != GET_DLG_OWNER(me))
                {
                    /* The owner of delegate should not be NULL. */
                    IdleDlgIndEvt *pe = Q_NEW(IdleDlgIndEvt, IDLE_TIMEOUT_SIG);
                    QACTIVE_POST((QActive *)GET_DLG_OWNER(me), (QEvt *)pe, me);
                }
                else
                {
                    ASSERT(0); /* for debug build */
                }
                IdleDlg_ResetIdleTimer(me);
                TYMQP_DUMP_QUEUE_WITH_LOG(me, "idleTimer is timeout now.");
            }
            else
            {
                IdleDlg_RefreshTick(me, IDLEDLG_TIMEOUT_IN_MS);
                //TP_PRINTF(" idleTimer: %ld seconds", me->idleTimer/1000);
                printf(" idleTimer: %ld seconds\n", me->idleTimer/1000);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)Q_EXIT_SIG", e->sig);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

#endif //#ifdef HAS_IDLE_DELEGATE

