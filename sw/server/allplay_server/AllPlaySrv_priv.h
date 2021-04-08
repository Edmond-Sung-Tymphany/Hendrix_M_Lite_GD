/*****************************************************************************
*  @file      AllPlaySrv_priv.h
*  @brief     Private header file for base allplay server
*  @author    Christopher Alexander
*  @date      21-Jan-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef ALLPLAYSRV_PRIV_H
#define	ALLPLAYSRV_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "AllPlaySrv.h"

/* private: */
static QState AllPlaySrv_Initial(cAllPlaySrv * const me, QEvt const * const e);
static QState AllPlaySrv_Active(cAllPlaySrv * const me, QEvt const * const e);
static QState AllPlaySrv_DeActive(cAllPlaySrv * const me, QEvt const * const e);
static void AllPlaySrv_SaveInfo(cAllPlaySrv * const me);

#ifdef	__cplusplus
}
#endif

#endif	/* ALLPLAYSRV_PRIV_H */
