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

#include "commonTypes.h"
#include "BluetoothStatus.h"
#include "BluetoothDrv_class_def.h"

void BluetoothDrv_Ctor(cBluetoothDrv *me);
void BluetoothDrv_Xtor(cBluetoothDrv *me);

eBtErrorStatus BluetoothDrv_BtCmd_GoDiscoverable(cBluetoothDrv *me);
eBtErrorStatus BluetoothDrv_BtCmd_GoConnectable (cBluetoothDrv *me);
eBtErrorStatus BluetoothDrv_BtCmd_GoReconnect   (cBluetoothDrv *me);
eBtErrorStatus BluetoothDrv_BtCmd_ResetPDL      (cBluetoothDrv *me);
eBtErrorStatus BluetoothDrv_BtCmd_PowerOn       (cBluetoothDrv *me);
eBtErrorStatus BluetoothDrv_BtCmd_PowerOff      (cBluetoothDrv *me);

eBtStatus BluetoothDrv_GetBtStatus(cBluetoothDrv* me);
bool      BluetoothDrv_IsBusy     (cBluetoothDrv* me);

void BluetoothDrv_RegisterReportCallback(cBluetoothDrv *me, btDrvReportCallback cb);


#endif /* BLUETOOTH_DRIVER_H */


