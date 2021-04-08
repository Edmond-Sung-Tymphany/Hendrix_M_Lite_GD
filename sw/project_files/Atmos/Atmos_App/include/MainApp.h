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
#include "AseTkSrv.h"

typedef enum {
    AUDIO_SOURCE_MIN = 0,
#ifdef HAS_AUXIN
    AUDIO_SOURCE_AUXIN = AUDIO_SOURCE_MIN,
#endif
#ifdef HAS_SPDIF_IN
    //AUDIO_SOURCE_SPDIF_IN,
    AUDIO_SOURCE_COAXIAL,
    AUDIO_SOURCE_OPTICAL,
    AUDIO_SOURCE_HDMI,
    AUDIO_SOURCE_HDMI_RCA,
#endif
    AUDIO_SOURCE_BLUETOOTH,
    AUDIO_SOURCE_USB,
    AUDIO_SOURCE_CAST,
    //AUDIO_SOURCE_WIFI,
//#ifdef HAS_ETHERNET
    //AUDIO_SOURCE_ETHERNET,
//#endif
    AUDIO_SOURCE_MAX
}eAudioSource;
#define AUDIO_SOURCE_DEFAULT   AUDIO_SOURCE_AUXIN

typedef struct {
    QStateHandler *stateHandler;
    eAudioChannel audioChannel;
    eLedIndID     ledInd;
    bool          bIsValid;
} tSourceHandlerList;

 /** \brief pointer to a tick-handler function */
typedef void (*TickHandler)(void * const me, QEvt const * const e);
typedef struct {
    int32 timer;
    TickHandler timerHandler;
} tTickHandlerList;

typedef struct _KeyEvtToFepAseCommand
{
    eKeyID keyId;
    eKeyEvent keyEvent;
    FepAseCommand_Command command;
} KeyEvtToFepAseCommand;

 
typedef enum
{
    AUDIO_MODE_NORMAL,
    AUDIO_MODE_EXT_SOURCE, //SPDIF for FS1, LINE-IN for FS2
    AUDIO_MODE_DSP_ONLINE_TUNING,
    AUDIO_MODE_NUM,
}eAudioMode;


/*************************************************/
/********* Publish the system status *******************/
/*************************************************/
typedef enum
{
    SYSTEM_STA_ON,
    SYSTEM_STA_STANDBY_HIGH,
    SYSTEM_STA_STANDBY_LOW,
    SYSTEM_STA_SLEEP,
    SYSTEM_STA_OFF,
    MAX_SYSTEM_STA,
}eSystemStatus;



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
    uint16              combinedKey;
    bool                audioPassEnable;
    int32               relativeVol;
    bool                isOverheat;
    eAudioMode          audioMode;
#ifdef BnO_fs1  //TODO: let FS2 support
    eTempLevel          tempLevel;
#endif
    eSystemStatus       systemStatus;
METHODS

    /* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_H */
