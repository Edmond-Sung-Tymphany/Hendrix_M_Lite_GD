/*****************************************************************************
*  @file      IrlearningDlg.h
*  @brief     Header file to IR Learning delegate.
*  @author    Edmond Sung
*  @date      23-Jan-2015
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef IRLEARNING_DLG_H
#define IRLEARNING_DLG_H

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

typedef enum {
    LEARN_VOLUME_UP_KEY_INDEX=0,
    LEARN_VOLUME_DOWN_KEY_INDEX,
    LEARN_MUTE_KEY_INDEX,

//total no of learn key support -LEARN_FINISH
    LEARN_FINISH,

    LEARN_NOTHING = 0xff

}LEARNKEYINDEX;

#define NUMBER_OF_IR_LEARNING_KEY   (LEARN_FINISH-1)



RESP_EVT(IrLearnStopEvt)
END_RESP_EVT(IrLearnStopEvt)



#ifdef HAS_IR_LEARNING_DELEGATE
SUBCLASS(cIrLearningDlg, cDelegate)
    QTimeEvt      timeEvt;
    LEARNKEYINDEX bLearnKeyIndex:8;
    uint32	bLearnTimeOut;
    uint16  doublePressTimeout;
    uint8	bLearnTimes;
    bool	bLearnFinishFlag:1;
    bool    bJustSetAMode:1;
    bool    bWaitForNoKey:1;
    bool    bIsFinish:1;
    bool    bJustFinishIRLearn:1;
METHODS
/* public functions */
cIrLearningDlg * IrLearningDlg_Ctor(cIrLearningDlg * me, QActive *ownerObj);
void IrLearningDlg_Xtor(cIrLearningDlg * me);
void IrLearningDlg_EraseLearntCode();




END_CLASS

#endif /* HAS_IR_LEARNING_DELEGATE */

#ifdef __cplusplus
}
#endif

#endif /* IRLEARNING_DLG_H */

