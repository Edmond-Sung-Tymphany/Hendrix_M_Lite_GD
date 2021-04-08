/*****************************************************************************
*  @file      IdleDlg.h
*  @brief     Header file to idle delegate.
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef BT_DLG_H
#define BT_DLG_H

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
#include "AudioSrv.h"
#include "BluetoothSrv.h"


#ifdef HAS_BT_DELEGATE
  
typedef enum
{
    NO_CALL_STA,
    ONE_CALL_STA,
    TWO_CALL_STA,
}eCallState;


SUBCLASS(cBluetoothDlg, cDelegate)
    int32         calTimer;
    bool          isAudioJackIn;
    eAudioChannel channel;     
    eBtStatus     myBtStatus;
    QTimeEvt      timeEvt;
METHODS
/* public functions */
cBluetoothDlg * BluetoothDlg_Ctor(cBluetoothDlg * me, QActive *ownerObj);
void BluetoothDlg_Xtor(cBluetoothDlg * me);
void BluetoothDlg_GoConnectable( QActive * me);


END_CLASS

#endif /* HAS_BT_DELEGATE */

#ifdef __cplusplus
}
#endif

#endif /* BT_DLG_H */

