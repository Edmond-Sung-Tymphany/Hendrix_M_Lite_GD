/**
*  @file      idle_dlg.c
*  @brief     Source file for Idle Delegate class
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*/

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
#ifdef IDLEDLG_DEBUG
const static char *idleDlg_debug = "[IdleDlg_Debug] ";
const static char *idleDlg_entry = " Entry ";
const static char *idleDlg_exit = " Exit ";
#define IDLEDLG_DEBUG_MSG TP_PRINTF("\r\n%s", idleDlg_debug); TP_PRINTF
#define ENTRY_STATE() IDLEDLG_DEBUG_MSG("%s%s\n", idleDlg_entry, __FUNCTION__)
#define EXIT_STATE()  IDLEDLG_DEBUG_MSG("%s%s\n", idleDlg_exit, __FUNCTION__)
#else
#define IDLEDLG_DEBUG_MSG(...)
#define ENTRY_STATE()
#define EXIT_STATE()
#endif

#define IDLEDLG_TIMEOUT_IN_MS           (10000) /*10 sec */
/* internal signals */
enum IdleDlgPriSignals /* idle delegate private signals */
{
    IDLEDLG_TIMEOUT_SIG = MAX_SIG,
};

typedef enum
{
#ifdef HAS_BATTERY
    BATTERY_MODE        = 0x01,
#else
    BATTERY_MODE        = 0x00,
#endif
    NO_AUDIO_PLAYBACK   = 0x02,
    IDLE_TIMER_TRIGGER   = BATTERY_MODE | NO_AUDIO_PLAYBACK,
} eIdleTimerTriggerSource;
static eIdleTimerTriggerSource triggerSource = 0;

/* Internal evt queue */
static QEvt const *idleEventQue[5];

static void IdleDlg_RefreshTick(cIdleDlg* const me, uint16 TickTimeInterval)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, TickTimeInterval, 0);
}

static void IdleDlg_ResetIdleTimer(cIdleDlg* const me)
{
    me->idleTimer = IDLE_TIMEOUT_IN_MS;
    IDLEDLG_DEBUG_MSG(" idleTimer reset to %d min. \r\n", IDLE_TIMEOUT_IN_MS/(60*1000));
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
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);

    return Q_TRAN(&IdleDlg_PreActive);
}

static QState IdleDlg_PreActive(cIdleDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            ENTRY_STATE();
            IdleDlg_RefreshTick(me, 1000); /*set a 1 sec timer for debug */
            return Q_HANDLED();
        }
        case IDLEDLG_TIMEOUT_SIG:
        {
#ifdef HAS_BATTERY
            bool isAcPlugIn = FALSE;
            if (Setting_IsReady(SETID_AC_STATUS))
            {
                isAcPlugIn = *(bool*)Setting_Get(SETID_AC_STATUS);
            }

            if(FALSE == isAcPlugIn)
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
            if(IDLE_TIMER_TRIGGER == triggerSource)
            {
                return Q_TRAN(&IdleDlg_IdleMode);
            }
            else
            {
                return Q_TRAN(&IdleDlg_NonIdleMode);
            }
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            EXIT_STATE();
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
            ENTRY_STATE();
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_BATT_STATE_SIG:
        {
            PowerSrvInfoEvt* pBatEvt = (PowerSrvInfoEvt*) e;
            if(CHARGER_BATT_STA == pBatEvt->batteryInfo.chargerState)
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
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            EXIT_STATE();
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
            ENTRY_STATE();
            IdleDlg_ResetIdleTimer(me);
            IDLEDLG_DEBUG_MSG("Idle timer is triggered. Idle timeout duration: %d min. \r\n", IDLE_TIMEOUT_IN_MS/(60*1000));
            IdleDlg_RefreshTick(me, IDLEDLG_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
#ifdef HAS_BATTERY
        case POWER_BATT_STATE_SIG:
        {
            PowerSrvInfoEvt* pBatEvt = (PowerSrvInfoEvt*) e;
            if(CHARGER_BATT_STA != pBatEvt->batteryInfo.chargerState)
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
            AudioMusicDetectStateEvt* pAudioMusicStateEvt = (AudioMusicDetectStateEvt*)e;
            if (pAudioMusicStateEvt->hasMusicStream)
            {
                TYM_CLR_BIT(triggerSource, NO_AUDIO_PLAYBACK);
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
                if(NULL !=GET_DLG_OWNER(me))
                {   /* The owner of delegate should not be NULL. */
                    IdleDlgIndEvt *pe = Q_NEW(IdleDlgIndEvt, IDLE_TIMEOUT_SIG);
                    QACTIVE_POST((QActive *)GET_DLG_OWNER(me), (QEvt *)pe, me);
                }
                else
                {
                    ASSERT(0); /* for debug build */
                }
                IdleDlg_ResetIdleTimer(me);
                IDLEDLG_DEBUG_MSG(" idleTimer is timeout now.\r\n");
            }
            else
            {
                IdleDlg_RefreshTick(me, IDLEDLG_TIMEOUT_IN_MS);
                IDLEDLG_DEBUG_MSG(" idleTimer: %ld seconds \r\n", me->idleTimer/1000);
            }
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(&me->timeEvt);
            EXIT_STATE();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

#endif //#ifdef HAS_IDLE_DELEGATE

