/*****************************************************************************
*  @file      IdleDlg_priv.h
*  @brief     Private header file to idle delegate.
*  @author    Daniel Qin
*  @date      11-JUN-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef BT_DLG_PRIV_H
#define BT_DLG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "bsp.h"
#include "trace.h"
#include "controller.h"
#include "DebugSettSrv.h"
#include "BluetoothDlg.h"
#include "gpioDrv.h"
#include "attachedDevices.h"
    
#include "LedDrv.h"
#include "BluetoothSrv.Config"
#include "LedSrv.h"

typedef struct
{
    bool bIsAutoLinkMode;
    int16 calLinkCmdInternalTimer;
    int32 calTimer;
}tAutoLink;


static QState BluetoothDlg_Initial(cBluetoothDlg * const me);
static QState BluetoothDlg_PreActive(cBluetoothDlg * const me, QEvt const * const e);
static QState BluetoothDlg_Active(cBluetoothDlg * const me, QEvt const * const e);

static void BluetoothDlg_ParseBTEvent(cBluetoothDlg * const me, QEvt const * const e);

static void BluetoothDlg_AudioJackDetect(cBluetoothDlg * const me);
static void BluetoothDlg_AudioChannelAutoSwitch(cBluetoothDlg * const me);
static void BluetoothDlg_SwitchAudioChannel(cBluetoothDlg * const me, eAudioChannel channel);

static bool BluetoothDlg_IsAutoLinkDone(cBluetoothDlg * const me);

static void BluetoothDlg_RefleshTick(cBluetoothDlg * const me, const uint16 tickTime);


#ifdef __cplusplus
}
#endif

#endif /* BT_DLG_PRIV_H */
