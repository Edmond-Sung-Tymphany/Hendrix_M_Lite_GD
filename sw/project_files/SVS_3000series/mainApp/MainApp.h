/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef MAINAPP_H
#define	MAINAPP_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "product.config"
#include "commonTypes.h"
#include "application.h"
#include "audioSrv.h"
#include "pattern.h"
#include "setting_id.h"
#include "PowerSrv.h"

typedef enum {
    AUDIO_SOURCE_MIN = 0,
    AUDIO_SOURCE_ASE = AUDIO_SOURCE_MIN,
#ifdef HAS_AUXIN
    AUDIO_SOURCE_AUXIN,
#endif
#ifdef HAS_SPDIF_IN
    AUDIO_SOURCE_SPDIF_IN,
#endif
#ifdef HAS_DSP_TUNING_MODE
    AUDIO_SOURCE_DSP_TUNING,
#endif
    AUDIO_SOURCE_MAX
}eAudioSource;


typedef struct {
    QStateHandler *stateHandler;
    eAudioChannel audioChannel;
    eLedIndID     ledInd;
    bool          bIsValid;
} tSourceHandlerList;

 /** \brief pointer to a tick-handler function */
typedef QState (*TickHandler)(void * const me, QEvt const * const e);
typedef struct {
    int32 timer;
    TickHandler timerHandler;
} tTickHandlerList;

/*************************************************/
/********* Publish the system status *******************/
/*************************************************/
typedef enum
{
    SYSTEM_STA_ON,
    SYSTEM_STA_IDLE,
    SYSTEM_STA_STANDBY,
    MAX_SYSTEM_STA,
}eSystemStatus;

typedef enum
{
    UPGRADE_NONE,
    UPGRADE_BY_USB,
    UPGRADE_BY_APP,
}eUpgradeMethod;

typedef enum
{
    JACK_LEVEL_NONE,
    JACK_LEVEL_HIGH,
    JACK_LEVEL_LOW,
}eJackLevel;

#define STANDBY_MODE_AUTO       0
#define STANDBY_MODE_TRIGGER    10
#define STANDBY_MODE_ON         20

REQ_EVT(MainAppReqEvt)
    int16 value;
END_REQ_EVT(MainAppReqEvt)

RESP_EVT(MainAppRespEvt)
    int16 value;
END_RESP_EVT(MainAppRespEvt)

/*************************************************/
/********* MainApp class definition     *******************/
/*************************************************/

SUBCLASS(cMainApp, cApplication)
    eSystemStatus       systemStatus;       // current system status
    ePageSettingId      pageSetting;        // current page setting
    int16               *pMenuData;         // point to the Menu Data
    int16               standbyMode;
    int16               prevStandbyMode;    /* record the standby mode before entering TRIGGER standby mode,
                                               restore after TRIGGER cable plug out*/
    tTickHandlerList    *tickHandlers;

    eAmpTempLevel       sysTempLevel;
    bool                musicPlaying;
    eUpgradeMethod      upgradeMethod;
    eJackLevel          jackLevelStatus;
    eDcInSta            dcSense;   // TRUE: power switch to on, FALSE power switch to off
METHODS

    /* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_DeActive(cMainApp * const me, QEvt const * const e);
QState MainApp_Idle(cMainApp * const me, QEvt const * const e);
QState MainApp_Standby(cMainApp * const me, QEvt const * const e);
QState MainApp_FactoryReset(cMainApp * const me, QEvt const * const e);
QState MainApp_EnterUpgrading(cMainApp * const me, QEvt const * const e);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_H */
