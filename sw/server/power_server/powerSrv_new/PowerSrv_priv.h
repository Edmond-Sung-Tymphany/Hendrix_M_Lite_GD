/*****************************************************************************
*  @file      PowerSrv_priv.h
*  @brief     Private header file for base power server class
*  @author    Johnny Fan
*  @date      25-Feb-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef POWERSRV_PRIV_H
#define	POWERSRV_PRIV_H

#include "../server_priv.h"
#include "bsp.h"      // for GET_TICKS_IN_MS()
#include "trace.h"
#include "controller.h"
#include "SettingSrv.h"
#include "PowerDrv.h"
#include "PowerSrv.Config"
#include "PowerSrv.h"
#include "commontypes.h"

#ifdef	__cplusplus
extern "C" {
#endif


/* private: */
QState PowerSrv_Initial(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_PreActive(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_Active(cPowerSrv * const me, QEvt const * const e);
QState PowerSrv_DeActive(cPowerSrv * const me, QEvt const * const e);

static void PowerSrv_RefreshTick(cPowerSrv * const me);


#ifdef	__cplusplus
}
#endif

#endif	/* POWERSRV_PRIV_H */

