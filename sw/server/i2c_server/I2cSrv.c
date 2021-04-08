/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  I2C Slave Server
                  -------------------------
                  SW Module Document
@file        I2cSrv.c
@brief       This file implements a i2c slave server.
@author      Viking Wang
@date        2016-11-22
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "controller.h"
#include "signals.h"
#include "server.h"
#include "I2cSrv.h"
#include "I2cSlaveDrv.h"

/* Private functions / variables. Declare and drivers here */
/* Internal event queue - Size as needed */
static QEvt const *I2cEvtQue[I2C_SRV_EVENT_Q_SIZE];
static bool i2c_slave_enable=FALSE;


/* internal signals */
enum InternalSignals
{
    TIMEOUT_SIG = MAX_SIG,
};

/* State function definitions */
static QState I2cSrv_Initial(cI2cSrv * const me);
static QState I2cSrv_Active(cI2cSrv * const me, QEvt const * const e);
static QState I2cSrv_DeActive(cI2cSrv * const me, QEvt const * const e);

void I2cSrv_Enable(bool enable)
{   
    i2c_slave_enable = enable;
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void I2cSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(I2cSrv_Active);
    QS_OBJ_DICTIONARY(I2cSrv_DeActive);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&I2cSrv_Initial), TIMEOUT_SIG,
                                I2cEvtQue, Q_DIM(I2cEvtQue), I2C_SRV_ID);

    /* Subscribe */
}

void I2cSrv_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState I2cSrv_Initial(cI2cSrv * const me)
{
    return Q_TRAN(&I2cSrv_DeActive);
}

/*   Active state - first state where "normal" service begins  */
static QState I2cSrv_Active(cI2cSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        {
            I2cSlaveDrv_Ctor();
            I2cSlaveDrv_Init();
            PersistantObj_RefreshTick((cPersistantObj*)me, I2C_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
    case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_HANDLED();
        }
    case TIMEOUT_SIG:
        {
            I2cSlaveDrv_Service();
            PersistantObj_RefreshTick((cPersistantObj*)me, I2C_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
    case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            return Q_TRAN(&I2cSrv_DeActive);
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

/*   DeActive state */
static QState I2cSrv_DeActive(cI2cSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        {
            I2cSlaveDrv_Deinit();
            return Q_HANDLED();
        }
    case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            if( i2c_slave_enable )
            {
                return Q_TRAN(&I2cSrv_Active);
            }
            else
            {
                return Q_HANDLED();
            }
        }
    case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
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

