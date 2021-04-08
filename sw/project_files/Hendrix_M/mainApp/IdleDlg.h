/*****************************************************************************
*  @file      IdleDlg.h
*  @brief     Header file to idle delegate.
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef IDLE_DLG_H
#define IDLE_DLG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qp_port.h"
#include "qf.h"
#include "signals.h"
#include "cplus.h"
#include "commonTypes.h"
#include "product.config"
#include "delegate.h"

#ifdef HAS_IDLE_DELEGATE

SUBCLASS(cIdleDlg, cDelegate)
QTimeEvt    timeEvt;
int32       idleTimer;
METHODS
/* public functions */
cIdleDlg * IdleDlg_Ctor(cIdleDlg * me, QActive *ownerObj);
void IdleDlg_Xtor(cIdleDlg * me);
END_CLASS

#endif /* HAS_IDLE_DELEGATE */

#ifdef __cplusplus
}
#endif

#endif /* IDLE_DLG_H */

