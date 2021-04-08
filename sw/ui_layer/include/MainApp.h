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


typedef enum
{
    SOURCE_INPUT_AUX_IN_MODE,
    SOURCE_INPUT_ALLPLAY_MODE,
    END_OF_SOURCE_INPUT_MODE
}eSourceInput;

#ifdef HAS_ALLPLAY
typedef enum {
    ALLPLAY_AP_MODE,
#ifdef HAS_ALLPLAY_DIRECTMODE
    ALLPLAY_DIRECT_MODE,
#endif
    AUXIN,
    RCA_IN,
    BLUETOOTH,
    OPT_IN,
    MAX_SOURCE
}eAudioSource;

typedef struct {
    QStateHandler *stateHandler;
    char          *pSourceName;
    eAudioChannel audioChannel;
    eAudioChannel multiRoomAudioChannel;
    Color         sourceColor;
    Color         dimColor;
    bool          bIsValid;
} tSourceHandlerList;

 /** \brief pointer to a tick-handler function */
typedef void (*TickHandler)(void * const me, QEvt const * const e);
typedef struct {
    int32 timer;
    TickHandler timerHandler;
} tTickHandlerList;
#endif
/*************************************************/
/********* Publish the system status *******************/
/*************************************************/
typedef enum
{
    SYSTEM_ACTIVE_STA,
    SYSTEM_SLEEP_STA,
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
#ifdef HAS_ALLPLAY
    uint8               allplayMaxVol;
    QStateHandler*      nextState;
    eAudioSource        audioSource;
    tSourceHandlerList *sourceHandler;
    tTickHandlerList   *tickHandlers;
#endif
    uint8       vol;
    QTimeEvt disCtrTimeEvt;
METHODS

    /* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_H */
