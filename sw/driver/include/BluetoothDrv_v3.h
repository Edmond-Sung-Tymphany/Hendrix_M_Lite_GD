/**
 * @file        BluetoothDrv.h
 * @brief       BT Driver header file
 * @author      Edmond Sung
 * @date        2014-02-10
 * @copyright   Tymphany Ltd.

 
 Change History:
 VERSION    : 1    DRAFT      2014-02-10     Edmond Sung
 DESCRIPTION: First Draft.
 SCO/ERROR  :
 
 VERSION    : 2    DRAFT      2014-05-13     Johnny Fan
 DESCRIPTION: second Draft. To implement the driver for ROM based CSR module
 SCO/ERROR  :


 */

#ifndef BLUETOOTH_DRIVER_H
#define BLUETOOTH_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "qp_port.h"
#include "signals.h"
#include "GpioDrv.h"
#include "seq.h"
#include "bt_status_cmd.h"


/* BT cmd done event*/
IND_EVT(BtDrvCmdEvt)
    eBtCmd cmd;
END_IND_EVT(BtDrvCmdEvt)

/* BT codec status*/
IND_EVT(BtDrvCodecStatusEvt)
    uint8 codecStatus;
END_IND_EVT(BtDrvCodecStatusEvt)

typedef void(*rxCb_f)(eBtReceivedMsgType type, uint8 *pData);


CLASS(cBluetoothDrv)   
    /* private data */
    eBtCmd      cmd;
    rxCb_f      rxCb;
    cSeq        seq;
METHODS

/* public functions */
void BluetoothDrv_Ctor(cBluetoothDrv *me);
void BluetoothDrv_Xtor(cBluetoothDrv *me);


/**
* send the BT command to BT module 
* @param[in]      me         bluetooth driver object
* @param[in]      cmd       the BT command 
*/
void BluetoothDrv_ExecuteCmd(cBluetoothDrv *me, eBtCmd cmd, uint8 length,uint8* param);

/**
* register to get the BT module status signal from driver 
* clent can reset the receiver by calling this function
* @param[in]      me                 bluetooth driver object
* @param[in]      req                the object (BT server) to receive the signal     
*/
void BluetoothDrv_RegisterRxMsgCb(cBluetoothDrv* me, rxCb_f cb);


void BtDrv_PowerOnSeqInit(cBluetoothDrv* me);
void BtDrv_PowerOffSeqInit(cBluetoothDrv* me);
bool BtDrv_isSeqFinished(cBluetoothDrv* me);
uint32 BtDrv_SeqRefresh(cBluetoothDrv *me, uint32 time);

void BtDrv_UpdateStatus(cBluetoothDrv *btDrv);


END_CLASS


#endif /* BLUETOOTH_DRIVER_H */

