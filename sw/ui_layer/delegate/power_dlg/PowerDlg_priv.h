/*****************************************************************************
*  @file      PowerDlg_priv.h
*  @brief     Private header file to power delegate.
*  @author    Johnny Fan
*  @date      14-JUL-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef POWER_DLG_PRIV_H
#define POWER_DLG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "bsp.h"
#include "trace.h"
#include "controller.h"
#include "DebugSettSrv.h"
#include "PowerDlg.h"
#include "BluetoothSrv.h"

#ifdef HAS_POWER_DELEGATE

/* Enable ext battery eject/inset tone */
//#define POWER_EXT_BATTERY_TONE_ENABLE

#define BATTERY_COMPARISON_LENGTH   100

static QState PowerDlg_Initial(cPowerDlg * const me);
static QState PowerDlg_Active(cPowerDlg * const me, QEvt const * const e);
static QState PowerDlg_Emergency(cPowerDlg * const me, QEvt const * const e);

static void PowerDlg_InitialVariable(cPowerDlg * const me);
#ifdef POWER_EXT_BATTERY_TONE_ENABLE
static void PowerDlg_ParseExtBattStateAndPlayTone(cPowerDlg * const me, tBatteryInfo* battInfo);
#endif
static void PowerDlg_ParseInputSourceAndPlayTone(cPowerDlg * const me, tBatteryInfo* battInfo);
static void PowerDlg_ParsePowerMode(cPowerDlg * const me, tBatteryInfo* battInfo);
static void PowerDlg_ImplementPowerMode(cPowerDlg * const me, tBatteryInfo* battInfo);
static void PowerDlg_SetInputAndBattSource(cPowerDlg * const me, eInputSource inputSource, eBatterySource batterySource);
static void PowerDlg_SetExtraCommand(cPowerDlg * const me, eExtraCommand extraCommand);
static void PowerDlg_JudgeAndSetBatterySource(cPowerDlg * const me, tBatteryInfo* battInfo);
static bool PowerDlg_IsIntBatteryVolHigher(cPowerDlg * const me, tBatteryInfo* battInfo);
static void PowerDlg_Resp(QActive* sender, eSignal signal, eEvtReturn  evtReturn);
static void PowerDlg_SendAudioMuteSignal(cPowerDlg * const me, bool mute);
#ifdef POWER_DEBUG
static void PowerDlg_DebugBatteryInfo(cPowerDlg * const me, tBatteryInfo* battInfo, bool sendAtOnce);
#endif

static void PowerDlg_RefleshTick(cPowerDlg * const me, const uint16 tickTime);


#endif

#ifdef __cplusplus
}
#endif

#endif /* POWER_DLG_PRIV_H */
