/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef MAINAPP_H
#define MAINAPP_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "product.config"
#include "commonTypes.h"
#include "application.h"
#include "audioSrv.h"
#include "pattern.h"
#include "tym.pb.h"

typedef enum
{
    AUDIO_SOURCE_MIN = 0,
    AUDIO_SOURCE_BT = AUDIO_SOURCE_MIN,
    AUDIO_SOURCE_AUXIN,
#ifdef HAS_RCA_IN
    AUDIO_SOURCE_RCA,
#endif
    AUDIO_SOURCE_ANALOG_MIXED, // For shop mode
    AUDIO_SOURCE_MAX
} eAudioSource;

typedef struct
{
    QStateHandler *stateHandler;
    eAudioChannel audioChannel;
    eLedMask      leds;
    bool          bIsValid;
} tSourceHandlerList;

/** \brief pointer to a tick-handler function */
typedef QState (*TickHandler)(void * const me, QEvt const * const e);
typedef struct
{
    int32 timer;
    TickHandler timerHandler;
} tTickHandlerList;

typedef struct _KeyEvtToFepAseCommand
{
    eKeyID keyId;
    eKeyEvent keyEvent;
} KeyEvtToFepAseCommand;


typedef enum
{
    AUDIO_MODE_NORMAL,
    AUDIO_MODE_EXT_SOURCE, //SPDIF for FS1, LINE-IN for FS2
    AUDIO_MODE_DSP_ONLINE_TUNING,
    AUDIO_MODE_NUM,
} eAudioMode;


/*************************************************/
/********* Publish the system status *******************/
/*************************************************/
typedef enum
{
    SYSTEM_STA_ACTIVE,
    SYSTEM_STA_SEMI_ACTIVE,
    SYSTEM_STA_POWERING_UP,
    SYSTEM_STA_POWERING_DOWN,
    SYSTEM_STA_STANDBY,
    SYSTEM_STA_OFF,
    MAX_SYSTEM_STA,
} eSystemStatus;



/* system status event*/
IND_EVT(SystemStatusEvt)
eSystemStatus systemStatus;
END_IND_EVT(SystemStatusEvt)

#ifdef MUTIPLE_SOURCE_MUSIC_DETECTION

IND_EVT(MainAppWakeupEvt)
eAudioJackId WakeupSrc;
END_IND_EVT(MainAppWakeupEvt)

IND_EVT(MainAppHijackEvt)
eAudioJackId HijackSrc;
END_IND_EVT(MainAppHijackEvt)

REQ_EVT(DlgStartEvt)
END_REQ_EVT(DlgStartEvt)


#endif

/*************************************************/
/********* MainApp class definition     *******************/
/*************************************************/

SUBCLASS(cMainApp, cApplication)
QStateHandler*      nextState;
eAudioSource        audioSource;
tSourceHandlerList  *sourceHandler;
tTickHandlerList    *tickHandlers;
tLedInd             *ledInds;
int32               vol;
int32               bass;
int32               treble;
eSystemStatus       systemStatus;
Proto_BtState_ConnState connState;
eKeyID              combKey;
bool                isVolChanged    :1;
bool                isAmpFault      :1;
bool                isCuePlaying    :1;
bool                muteStatus      :1;
bool                isBtBooted      :1;
bool                isAudioDrvReady :1;

METHODS

/* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);

END_CLASS

#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_H */
