/*****************************************************************************
*  @file      IdleDlg_priv.h
*  @brief     Private header file to idle delegate.
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef IR_LEARNING_DLG_PRIV_H
#define IR_LEARNING_DLG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "bsp.h"
#include "trace.h"
#include "controller.h"
#include "gpioDrv.h"
#include "attachedDevices.h"
#include "IrlearningDlg.h"
#include "IrLearningDrv.h"
#include "keySrv.h"

    
#include "LedDrv.h"
#include "LedSrv.h"



static QState IrLearningDlg_Initial(cIrLearningDlg * const me);
static QState IrLearningDlg_PreActive(cIrLearningDlg * const me, QEvt const * const e);
static QState IrLearningDlg_Active(cIrLearningDlg * const me, QEvt const * const e);
static void IrLearningDlg_StopEvtResp(cIrLearningDlg * me);
static void IrLearningDlg_Reset(cIrLearningDlg * me);


#ifdef __cplusplus
}
#endif

#endif /* IR_LEARNING_DLG_PRIV_H */
