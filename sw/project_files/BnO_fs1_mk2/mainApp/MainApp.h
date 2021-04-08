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
#include "AseNgSrv.h"
#include "deviceTypes.h"

typedef enum {
    AUDIO_SOURCE_MIN = 0,
    AUDIO_SOURCE_ASE = AUDIO_SOURCE_MIN,
    AUDIO_SOURCE_EXT_SOURCE, //fs1:SPDIF, fs1:LINE-IN
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

typedef struct _KeyEvtToFepAseCommand
{
    eKeyID keyId;
    eKeyEvent keyEvent;
    Proto_Core_FepAseMessage command;
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
    SYSTEM_STA_IDLE,
    SYSTEM_STA_STANDBY_HIGH,
    SYSTEM_STA_STANDBY_LOW,
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
    eLedIndID           ledPowerDownConnId;
    eLedIndID           ledPowerDownProdId;
    
    //status variable, must inittialize again when enter Active
    bool                audioPassEnable;
    int8                relativeVol;
    Proto_AseFep_Event_Type  aseNetMode;
    bool                tonePlaying;
    bool                asetkMute;
    bool                audioLocal;
    eSystemStatus       systemStatus;
    bool                aseBtEnable;
    bool                aseBtleEnable;
    bool                bIsBtConnected;
    bool                isTonePlaying;
    bool                musicPlaying;
    Proto_Player_Data_Source playSource;
    PowerSrvInfoEvt     powerEvt;
    eTempLevel          tempLevelSystem;
    eAudioMode          audioMode;
    int8                absoluteVol;
    bool                bDcOn;
#ifdef  DEBUG_DEMO_FEATURE
    uint32              timeLastSoftApKey;
#endif    
METHODS

    /* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_H */
