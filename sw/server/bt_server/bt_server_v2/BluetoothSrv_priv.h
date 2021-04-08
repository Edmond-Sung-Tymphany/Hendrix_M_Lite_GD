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
#include "BluetoothDrv.h"
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

static uint32 BluetoothSrv_SetTimeReq(eTimeType reqTimeType, int32 timeInMs);
static void BluetoothSrv_UpdateTime(cBluetoothSrv * const me);
static void BluetoothSrv_PublishStatus(cBluetoothSrv * const me, bool isBtStatus);
static void BluetoothSrv_ExecuteCmd(BtCmdEvt* pEvt);
static void BluetoothSrv_CheckQueueAndRecallCmdEvt(cBluetoothSrv * const me);
static void BluetoothSrv_RefreshTick(cBluetoothSrv * const me);
#ifdef HAS_FIX_VOL_AUDIO_CUE
static void BluetoothSrv_DelayStartAudioCue(cBluetoothSrv * const me);
static void BluetoothSrv_CheckAudioCuePlaying(cBluetoothSrv * const me);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_SERVER_PRIVATE_H */
