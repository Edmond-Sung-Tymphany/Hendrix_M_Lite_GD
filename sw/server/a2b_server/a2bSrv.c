/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  A2B Server
                  -------------------------

                  SW Module Document




@file        A2BSrv
@brief       A2B control Server
@author      Edmond Sung
@date        2016-12-23
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:

-------------------------------------------------------------------------------
*/
#include "product.config"
#include "bsp.h"    // for BSP_TICKS_PER_SEC
#include "controller.h"
#include "trace.h"
#include "SystemDrv.h"
#include "I2CDrv.h"
#include "a2bSrv.h"
#include "a2bSrv_priv.h"
#include "A2bSrv.Config"
#include "adi_ad24xx.h"

/*****************************************************************************
 * Definition
 *****************************************************************************/
//#define A2BSRV_DEBUG

#ifdef A2BSRV_DEBUG
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
    #define A2BSRV_DEBUG_MSG TP_PRINTF
#else
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...)
    #define A2BSRV_DEBUG_MSG(...)
    #undef   TP_PRINTF
    #define  TP_PRINTF(...)
#endif

//Q_DEFINE_THIS_FILE

#define CAST_ME cA2bSrv * a2bSrv = (cA2bSrv *) me;


/* internal signals */
enum InternalSignals
{
    TIMEOUT_SIG = MAX_SIG,

};
/*****************************************************************
 * Global Variable
 *****************************************************************/
/* Internal event queue - Size as needed */
static QEvt const *A2BEvtBuf[A2B_SRV_EVENT_Q_SIZE];

static cAdiAD2410Drv A2bDrv;

#define MUTE_DELAY_TIME  10 /*5sec delay*/

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void A2bSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(cA2bSrv);
    QS_OBJ_DICTIONARY(A2bSrv_DeActive);
    QS_OBJ_DICTIONARY(A2bSrv_PreActive);

    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&A2bSrv_Initial), TIMEOUT_SIG,
                                A2BEvtBuf, Q_DIM(A2BEvtBuf),A2B_SRV_ID);
    
    /* Subscribe */
}

void A2bSrv_ShutDown(cPersistantObj *me)
{
    /* Destruct DSP driver */
    /* Clean memory and shut-down.*/
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
QState A2bSrv_Initial(cA2bSrv * const me)
{
    return Q_TRAN(&A2bSrv_DeActive);
}

QState A2bSrv_DeActive(cA2bSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
//            TP_PRINTF("Enter %s\r\n", __FUNCTION__);
            PersistantObj_RefreshTick((cPersistantObj*)me, A2B_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            if( SystemDrv_A2BModeIsMaster() )
            {   // if system is in master mode
                return Q_TRAN(&A2bSrv_Active);
            }
            else
            {
                // nothing to do when system in non-MASTER mode
                return Q_HANDLED();
            }
        }
        case TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, A2B_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
            
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
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

/*\brief Super state
 * The super state of A2bSrv_SwitchChannel,A2bSrv_MainHandler,A2bSrv_PlayToneStat,A2bSrv_Mute
 */
QState A2bSrv_Active(cA2bSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
        {
#ifndef ENABLE_ONLY_STANDALONE_MODE
            adi_a2b_Ctor(&A2bDrv);
//            adi_a2b_drv_init(&A2bDrv);
#endif            
            PersistantObj_RefreshTick((cPersistantObj*)me, A2B_SRV_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
    /* Sometimes system have wrong sequence (ex. send continuous two SYSTEM_ACTIVE_REQ_SIG to AudioSrv),
         * and cause potential issues. The ASSERT here could protect these problems. */
    case SYSTEM_ACTIVE_REQ_SIG:
        {
            TYMQP_DUMP_QUEUE_WITH_LOG(me, "(%d)SYSTEM_ACTIVE_REQ_SIG", e->sig);
            ASSERT(0);
            return Q_HANDLED();
        }

    case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
            adi_a2b_Xtor(&A2bDrv);
            return Q_TRAN(A2bSrv_DeActive);
        }
    case ALLPLAY_RESET_SIG: // re-init the A2B
        {
#ifndef ENABLE_ONLY_STANDALONE_MODE
            adi_a2b_drv_init(&A2bDrv);
            SystemDrv_SetTotalNodes(A2bDrv.pFrameworkHandle->pgraph->NumberOfSlaveDiscovered + 1);
#endif
            return Q_HANDLED();
        }
    case TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)me, A2B_SRV_TIMEOUT_IN_MS);
            A2bSrv_PeriodicTask(me);
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

/*****************************************************************************************************************
 *
 * public interface functions
 *
 *****************************************************************************************************************/

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void A2bSrv_PeriodicTask(cA2bSrv * const me)
{
    (void)me;
    //TODO:
}

void A2bSrv_I2cWrite(uint32_t node, uint8_t reg_addr, uint32_t len, uint8_t *p_buf)
{
    ADI_A2B_PERI_CONFIG_UNIT write_config;

    write_config.eOpCode = A2B_WRITE_OP;
    write_config.nAddrWidth = 1;
    write_config.nAddr = (uint32_t)reg_addr;
    write_config.nDataWidth = 1;
    write_config.nDataCount = len;
    write_config.paConfigData = p_buf;

    adi_a2b_slave_peripheral_config(&A2bDrv, (node-1), (I2C_SLAVE_DEVICE_ADDR>>1), &write_config);
}

void A2bSrv_I2cRead(uint32_t node, uint8_t reg_addr, uint32_t len, uint8_t *p_buf)
{
    ADI_A2B_PERI_CONFIG_UNIT read_config;
    
    read_config.eOpCode = A2B_READ_OP;
    read_config.nAddrWidth = 1;
    read_config.nAddr = (uint32_t)reg_addr;
    read_config.nDataWidth = 1;
    read_config.nDataCount = len;
    read_config.paConfigData = p_buf;
    
    adi_a2b_slave_peripheral_config(&A2bDrv, (node-1), (I2C_SLAVE_DEVICE_ADDR>>1), &read_config);
}

/*
static bool A2bSrv_DrvInitAndTick(cA2bSrv * const me)
{
    bool ret = FALSE;
    uint16 delaytime =0;
    if(!A2bDrv.isCreated)
    {
        return ret;
    }
    //uint16 delaytime = adi_a2b_node_init(&A2bDrv);
    if (0 == delaytime)
    {
        ret = TRUE;

    }
    else
    {
        PersistantObj_RefreshTick((cPersistantObj*)me, delaytime*10);
    }
    return ret;
}
*/

