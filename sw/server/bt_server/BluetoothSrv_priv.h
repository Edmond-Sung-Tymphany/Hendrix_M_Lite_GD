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
#include "BluetoothDrv.h"
#ifdef USE_DEBUG_SETTING_SERVER 
#include "DebugSettSrv.h"
#else
#include "SettingSrv.h"
#endif
#include "BluetoothSrv.Config"
#include "bsp.h"
#include "controller.h"
#include "trace.h"
#include "AudioDrv.h"

static void BluetoothSrv_ExecuteCmd(BtCmdEvt* pEvt);
static void BluetoothSrv_CheckQueueAndRecallCmdEvt(cBluetoothSrv * const me);
static void BluetoothSrv_DriverReportCallback(eBtReportType type, uint32 reportData);

  /* State function definitions */
QState BluetoothSrv_Initial(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_PreActive(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_Active(cBluetoothSrv * const me, QEvt const * const e);
QState BluetoothSrv_DeActive(cBluetoothSrv * const me, QEvt const * const e);


#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_PRIVATE_H */
