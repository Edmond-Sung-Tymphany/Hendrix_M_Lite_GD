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
#include "BluetoothSrv_v2.h"
#include "ringbuf.h"
typedef enum
{
    AUDIO_SOURCE_MIN = 0,
    AUDIO_SOURCE_AUXIN = AUDIO_CHANNEL_AUXIN,
    AUDIO_SOURCE_BT = AUDIO_CHANNEL_BT,
    AUDIO_SOURCE_MAX
} eAudioSource;

typedef struct
{
    QStateHandler *stateHandler;
    eAudioChannel audioChannel;
    eLedIndID     ledInd;
    bool          bIsValid;
} tSourceHandlerList;

/** \brief pointer to a tick-handler function */
typedef QState (*TickHandler)(void * const me, QEvt const * const e);
typedef struct
{
    int32 timer;
    TickHandler timerHandler;
} tTickHandlerList;




/*************************************************/
/********* Publish the system status *******************/
/*************************************************/
typedef enum
{
    SYSTEM_STA_OFF,
    SYSTEM_STA_OFF_CHARGING,
    SYSTEM_STA_POWERING_UP,
    SYSTEM_STA_WAIT_BT_UP,
    SYSTEM_STA_ON,
    SYSTEM_STA_STANDBY,
    SYSTEM_STA_SLEEP,
    SYSTEM_STA_POWERING_DOWN,
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
const tLedInd       *ledInds;
eBtStatus           CurrBTStatus;

uint8               btReBootCnt;
int32               absoluteVol;
#ifdef HAS_SYSTEM_GAIN_CONTROL
int32               currSysGain;
#endif
cRingBuf            btCueRBufObj;
uint8               btCueBuf[BT_CUE_QUEUE_SIZE];

#ifdef HAS_BATTERY
BatteryStatus       battStatus;
uint8               btBattStatus;
#ifdef HAS_BATTERY_NTC
eTempLevel          battChgTemp_bk;
eTempLevel          battDischgTemp_bk;
bool                isChargeStopByNTC :1;
bool                isChgEnable       :1;
bool                isChgComplete     :1;
#endif
#endif
eSystemStatus       systemStatus;
int8                currBattLed;
bool                isBTStreaming     :1;
bool                isBTenabled       :1;
bool                isCriticalTemp    :1;
bool                isAmpFault        :1;
bool                isCuePlaying      :1;
bool                isCueChanel       :1; //workround Hendrix L [IN:014546]
bool                isNoButtonChanel  :1; //workround Hendrix L [IN:014546]
bool                BQBTestPlay       :1; //just for BQB test
bool                BQBTestVol        :1; //just for BQB test
bool                ConnectedCue      :1; //ConnectedCueTimeOut
METHODS

/* public functions */
void MainApp_StartUp( cPersistantObj *me);
void MainApp_ShutDown( cPersistantObj *me);

END_CLASS

#ifdef  __cplusplus
}
#endif

#endif  /* MAINAPP_H */
