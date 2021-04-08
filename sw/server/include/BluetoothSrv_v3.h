/**
 * @file        BluetoothSrv.h
 * @brief       it's the server to control the BT module
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */


#ifndef BLUETOOTH_SERVER_H
#define BLUETOOTH_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "bt_status_cmd.h"



/* BT status event*/
IND_EVT(BtRxEvt)
eBtReceivedMsgType  type;
uint8               data[BT_RX_EVT_DATA_LEN];
END_IND_EVT(BtRxEvt)

/* BT cmd event*/
REQ_EVT(BtCmdEvt)
eBtCmd btCmd;
uint8 length;
uint8 param[BT_MAX_EVT_PARAM_LENGTH];
END_REQ_EVT(BtCmdEvt)

SUBCLASS(cBluetoothSrv, cServer)
/* private data */
QActive     *pSender;
QTimeEvt    timeEvt;
uint32      timeCount;
eBtCmd      executingCmd;
uint8       executingCmdLength;
uint8       executingCmdParam[BT_MAX_EVT_PARAM_LENGTH];
METHODS
/* public functions */
END_CLASS

/* Implement these so the controller can launch the server */
void BluetoothSrv_StartUp(cPersistantObj *me);
void BluetoothSrv_ShutDown(cPersistantObj *me);

void BluetoothSrv_SendBtCmd(QActive* me, eBtCmd cmd);
void BluetoothSrv_SendBtCmdWithParam(QActive* me, eBtCmd cmd, uint8 length ,uint8* param);


#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_H */

