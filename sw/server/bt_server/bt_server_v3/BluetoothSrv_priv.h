/**
 * @file        BluetoothSrv_priv.h
 * @brief       it's the server to control the BT module
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */

#ifndef BLUETOOTH_SERVER_PRIVATE_H
#define BLUETOOTH_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "BluetoothSrv.h"
#include "BluetoothSrv.Config"
#include "bsp.h"
#include "BluetoothDrv.h"
#include "controller.h"
#include "trace.h"

typedef struct
{
    bool countEnable;
    bool delayEnable;
    uint32 countInMs;
    int32 delayInMs;
} tBtTimeStruct;


/* State function definitions */
static QState BluetoothSrv_Initial(cBluetoothSrv * const me, QEvt const * const e);
static QState BluetoothSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e);
static QState BluetoothSrv_Active(cBluetoothSrv * const me, QEvt const * const e);
static QState BluetoothSrv_PreDeActive(cBluetoothSrv * const me, QEvt const * const e);
static QState BluetoothSrv_DeActive(cBluetoothSrv * const me, QEvt const * const e);
static QState BluetoothSrv_ExecutingCmd(cBluetoothSrv * const me, QEvt const * const e);

static void BluetoothSrv_HandleBtStatus(cBluetoothSrv * const me, BtRxEvt* e);
static void BluetoothSrv_ReveiveMsgCb(eBtReceivedMsgType type, uint8 *pData);
static void BluetoothSrv_ExecuteCmd(BtCmdEvt* pEvt);
static void BluetoothSrv_ResendCmd(cBluetoothSrv * const me);


#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_PRIVATE_H */
