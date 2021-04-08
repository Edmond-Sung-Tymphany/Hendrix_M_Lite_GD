/*****************************************************************************
*  @file      IdleDlg_priv.h
*  @brief     Private header file to idle delegate.
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef IDLE_DLG_PRIV_H
#define IDLE_DLG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "IdleDlg.h"

IND_EVT(IdleDlgIndEvt)
END_IND_EVT(IdleDlgIndEvt)

static QState IdleDlg_Initial(cIdleDlg * const me);
static QState IdleDlg_PreActive(cIdleDlg * const me, QEvt const * const e);
static QState IdleDlg_NonIdleMode(cIdleDlg * const me, QEvt const * const e);
static QState IdleDlg_IdleMode(cIdleDlg * const me, QEvt const * const e);

#ifdef __cplusplus
}
#endif

#endif /* IDLE_DLG_PRIV_H */
