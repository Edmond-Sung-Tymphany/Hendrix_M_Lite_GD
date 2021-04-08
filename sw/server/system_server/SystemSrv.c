/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  System Server : base on Power Server
                  ------------------------------------
                  SW Module Document
@file        SystemSrv.c
@brief       Provides general support for power request/responses
@author      Viking Wang
@date        2016-07-22
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "trace.h"
#include "controller.h"
#include "SettingSrv.h"
#include "SystemSrv.h"
#include "SystemDrv.h"
#include "commontypes.h"

#include "systemsrv.config"

/*****************************************************************************
 * QP related Definition
 *****************************************************************************/
QState SystemSrv_Initial(cSystemSrv *const me, QEvt const * const e);
QState SystemSrv_PoweringUp(cSystemSrv *const me, QEvt const *const e);
QState SystemSrv_Active(cSystemSrv *const me, QEvt const *const e);
QState SystemSrv_DeActive(cSystemSrv *const me, QEvt const *const e);

#define CAST_ME cSystemSrv * systemSrv = (cSystemSrv *) me;

static QEvt const * SystemSrvQueueSto[POWER_SRV_EVENT_Q_SIZE];
static cSystemDrv systemDrv;
static QActive* pRequestor;

enum
{
    SYS_SRV_TIMEOUT_SIG = MAX_SIG ,
}eInternalSig;

/***********************************************************************/
/* Local Used Functions */
/***********************************************************************/

static void SystemSrv_RefreshTick(cSystemSrv * const me)
{
    PersistantObj_RefreshTick((cPersistantObj*)me, SYSTEM_SRV_TIMEOUT_IN_MS);
}

void SystemSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(SystemSrv_PoweringUp);
    QS_OBJ_DICTIONARY(SystemSrv_Active);
    QS_OBJ_DICTIONARY(SystemSrv_DeActive);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&SystemSrv_Initial), SYS_SRV_TIMEOUT_SIG,
                       SystemSrvQueueSto, Q_DIM(SystemSrvQueueSto), POWER_SRV_ID);

}

void SystemSrv_ShutDown(cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    SystemDrv_Xtor(&systemDrv);
    Server_Xtor((cServer*)me);
}

QState SystemSrv_Initial(cSystemSrv * const me, QEvt const * const e)
{
    (void)me;
    (void)e; /* suppress the compiler warning about unused parameter */

    SystemDrv_SetPowerStage(POWER_STAGE_POWER_OFF);
    SystemDrv_Ctor(&systemDrv);

    return Q_TRAN(&SystemSrv_DeActive);
}

/* System DeActive status, ErP-compliance system saving, turn off the system*/
QState SystemSrv_DeActive(cSystemSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
#ifdef PROJECT_BnO_SOUNDWALL
            SystemDrv_DeActiveInit();
#endif
            /*active the reset pin, and turn off external system*/
            SystemDrv_PowerOff(&systemDrv);
            /* response the sleep request */
            SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYS_SRV_TIMEOUT_SIG:
        {
            /* update System driver status */
            SystemDrv_StandbyUpdate(&systemDrv);
            /*Refresh the ticks*/
            SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            pRequestor = ((CommonReqEvt*)e)->sender;
            return Q_TRAN(&SystemSrv_PoweringUp);
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

/* Initial the System Hardware */
QState SystemSrv_PoweringUp(cSystemSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            me->timer = SystemDrv_InitPower(&systemDrv);
            SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYS_SRV_TIMEOUT_SIG:
        {
            me->timer -= SYSTEM_SRV_TIMEOUT_IN_MS;
            if ( me->timer <= 0 )
            {
                me->timer = SystemDrv_InitPower(&systemDrv);
                if(me->timer <= 0)
                {
                    SystemDrv_SetPowerStage(POWER_STAGE_POWER_READY);
                    CommonEvtResp((QActive*)me, pRequestor, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
                    return Q_TRAN(&SystemSrv_Active);
                }
                else
                    SystemSrv_RefreshTick(me);
            }
            else
                SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_TRAN(&SystemSrv_Active);
        }
        case SYSTEM_SLEEP_REQ_SIG:
            // do NOT send sleep REQ signal when powering up.
            ALWAYS_printf("\n\r system status ERROR!");
            ASSERT(0);
            return Q_HANDLED();
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default: break;
    }
    return Q_SUPER(&QHsm_top);
}

/* System Server super state */
QState SystemSrv_Active(cSystemSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
#ifdef PROJECT_BnO_SOUNDWALL
            SystemDrv_ActiveInit();
#endif
            SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*) e;
            CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            return Q_TRAN(&SystemSrv_DeActive);
        }
        case SYSTEM_ACTIVE_REQ_SIG: // just respond and do nothing
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_HANDLED();
        }
        case SYS_SRV_TIMEOUT_SIG:
        {
            /* update System driver status */
            SystemDrv_Update(&systemDrv);
            /*Refresh the ticks*/
            SystemSrv_RefreshTick(me);
            return Q_HANDLED();
        }
        case POWER_SET_SIG:
        {
            SystemSetEvt_t *p_evt = (SystemSetEvt_t*)e;
            SystemDrv_SetIdHandler((int32_t)p_evt->setting_id, p_evt->enable, p_evt->param);
            break;
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

void SystemSrv_Set(SystemSet_ID_t set_id, BOOL enable, uint32_t param)
{
     SystemSetEvt_t *p_system_evt = Q_NEW(SystemSetEvt_t, POWER_SET_SIG);
     p_system_evt->setting_id = set_id;
     p_system_evt->enable = enable;
     p_system_evt->param= *((uint32*)&param);
     SendToServer(POWER_SRV_ID, (QEvt*)p_system_evt);
}

