/**
 *  @file      LedSrv_priv.h
 *  @brief     the private inteface file for led server
 *  @author    Johnny Fan
 *  @date      20-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef LEDSRV_PRIV_H
#define	LEDSRV_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "LedSrv.h"

static void LedSrv_CtorAllLedObject(cLedSrv * const me);
static void LedSrv_XtorAllLedObject(cLedSrv * const me);

// Server helper functions
static void LedSrv_HandleReq(cLedSrv* me, LedReqEvt* req);
static void LedSrv_Resp(QActive* sender, eEvtReturn result);

// Led Drv object helper functions
static bool LedSrv_LedIdIsValid(uint32 ledId);
static void LedSrv_PublishState(cLedSrv *me, ledMask mask, ePattern pattId);

QState LedSrv_Initial(cLedSrv * const me, QEvt const * const e);
QState LedSrv_Active(cLedSrv * const me, QEvt const * const e);
QState LedSrv_DeActive(cLedSrv * const me, QEvt const * const e);
#ifdef	__cplusplus
}
#endif

#endif	/* LEDSRV_PRIV_H */

