/* 
 * File:   MainApp_priv.h
 * Author: Johnny.Fan
 *
 * Created on March 13, 2014, 2:11 PM
 */

#ifndef MAINAPP_PRIV_H
#define	MAINAPP_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "MainApp.h"
#include "KeySrv.h"

/* battery level states*/
typedef enum
{
    FULL_BATT_STA,
    NORM_BATT_STA,
    LOW_BATT_STA,
    CRITICAL_BATT_STA,
    SHUTDOWN_BATT_STA,
    BATTERY_STATE_MAX
} eBatteryStateSave;

/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
static QState MainApp_PreActive(cMainApp * const me, QEvt const * const e);
static QState MainApp_Active(cMainApp * const me, QEvt const * const e);
static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);
static QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);
static QState MainApp_Standby(cMainApp * const me, QEvt const * const e);
static QState MainApp_Sleep(cMainApp * const me, QEvt const * const e);
static QState MainApp_SoftReset(cMainApp * const me, QEvt const * const e);
static QState MainApp_Active(cMainApp * const me, QEvt const * const e);
    static QState MainApp_Auxin(cMainApp * const me, QEvt const * const e);
    static QState MainApp_AllPlay(cMainApp * const me, QEvt const * const e);
        static QState MainApp_AllPlay_APMode(cMainApp * const me, QEvt const * const e);
        static QState MainApp_AllPlay_DirectMode(cMainApp * const me, QEvt const * const e);

/************************************************************************************/
/* Key Functions*/
static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);

/************************************************************************************/
/* Power Functions */
static bool MainApp_LedDisplayBatteryState(cMainApp * const me, tBatteryInfo *pBattInfo);
static void MainApp_LedDisplayChargerState(cMainApp * const me, eChargerState chargerState);

/************************************************************************************/
/* Allplay functions */
static void MainApp_ParseAllPlayState(cMainApp * const me, QEvt const * const e);
static void MainApp_DisplayAllplaySystemMode(cMainApp * const me, enum allplay_system_mode_value currentSystemMode);

/************************************************************************************/

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */

