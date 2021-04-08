/**
 * @file        CommSrv_priv.h
 * @brief       It's the server to communicate with SoC
 * @author      Eason Huang 
 * @date        2016-06-02
 * @copyright   Tymphany Ltd.
 */

#ifndef COMMSRV_PRIV_H
#define COMMSRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "CommSrv.h"

/* private state / private functions */
static QState CommSrv_Initial(cCommSrv * const me, QEvt const * const e);
static QState CommSrv_PreActive(cCommSrv * const me, QEvt const * const e);
static QState CommSrv_Active(cCommSrv *const me, QEvt const *const e);
static QState CommSrv_DeActive(cCommSrv *const me, QEvt const *const e);

static void CommSrv_CtorDrvObjs(cCommSrv *const me);
static void CommSrv_XtorDrvObjs(cCommSrv *const me);
static void CommSrv_SendReq(commSendCmdEvt* pEvt);
static void CommSrv_SendCommEvt(eSocEvt evtId, uint32 param);
static void CommSrv_HandleCommand(cCommSrv *const me, QEvt const * const e);

#ifdef __cplusplus
}
#endif

#endif /* COMMSRV_PRIV_H */
 
