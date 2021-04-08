/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/**
 * @file        KeySrv.h
 * @brief       This file defines the interfaces of the key server and the implementation of the interfaces.
 * @author      Bob.Xu
 * @date        2014-02-17
 * @copyright   Tymphany Ltd.
 */


#ifndef KEYSRV_H
#define	KEYSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"
#include "KeyDrv.h"

/* Structures for Key combinations */
#ifdef HAS_COMB_KEY
typedef enum
{
    SEQ_KEY_PRESS_COMB_TYPE, /* Sequential key press, for example: A press->B press->C press */
    SEQ_KEY_SHIFT_COMB_TYPE, /* Shift like key,for example A hold->B hold->C hold or A hold-> B press */
    NORMAL_KEY_COMB_TYPE     /* No sequential key hold, for example, A hold, B hold, C hold */
}eCombKeyType;

typedef struct tCombKeyElem
{
    eKeyID          keyId;          /* Key ID */
    eKeyEvent       combKeyTrigEvt; /* The EVT to trigger combkey EVT */
    BOOL            matchFlag;      /* to mark if both KEY ID and EVT is matched */
}tCombKeyElem;

typedef struct tCombKeyDef
{
    eKeyID          combKeyId;     /* The ID of the comb key */
    eCombKeyType    combKeyType;   /* The comb key type */
    uint8           numOfKeyElem;  /* How many keys are combined */
    uint32          timeElapse;    /* The time elapsed from the previous matched evt to the current matched key evt */
    uint32          resetTime;     /* The time threshold to clean all the match flags */
    tCombKeyElem    *pCombKeyElems;/* A pointer point to the keys of the comb key */
}tCombDef;
#endif
/* End of Structures for Key combinations */

typedef enum
{
    DEBUG_NO_REQ,
    DEBUG_RAW_DATA_REQ,
    DEBUG_KEY_EVT_SIMULATION,
} eDbgKeyReq;

REQ_EVT(KeyEvtFromDrv)
    eKeyID keyId;
    eKeyEvent keyEvent;
END_REQ_EVT(KeyEvtFromDrv)

IND_EVT(KeyStateEvt)
    eKeyID keyId;
    eKeyEvent keyEvent;
#if defined HAS_LINEAR_ADC_KNOB_KEY || defined GPIO_ENCODER_KEY
    uint8 index;
#ifdef HAS_DEBUG
    uint16 adcRawValue;
#endif
#endif
#ifdef HAS_PARAM_KEY
    int32 param;
#endif
END_IND_EVT(KeyStateEvt)

#ifdef KEY_SRV_HAS_DEBUG
REQ_EVT(KeyDebugReqEvt)
    eDbgKeyReq req;
    eKeyEvent keyEvent;
    eKeyID keyId;
END_REQ_EVT(KeyDebugReqEvt)

RESP_EVT(KeyDebugRespEvt)
    eKeyID keyId;
    eEvtReturn serviceState;
    int16 rawData;
END_RESP_EVT(KeyDebugRespEvt)
#endif

/* keySrv config structures */
typedef struct tKeyboard/* tell server how many keyboard to be handled */
{
    eDeviceType         deviceType;
    cKeyDrv             *pKeySet;   /* keys belong to this keyboard */
    uint8               sizeOfKeyType;/* For casting */
    void                *pKeyboardAttachedObj;
}tKeyboard;

typedef struct
{
    struct
    {
        uint32 debounceTime;
        uint32 repeatStartTime;
        uint32 longPressTime;
        uint32 veryLongPressTime;
        uint32 repeatHoldTime[NORMAL_KEY_ID_MAX];
    } timing;
    uint32  keyboardNum;
    tKeyboard keyboardSet[KEYBOARD_NUMBER];
} tKeySrvConfig;
/* End of keySrv config structures */

/*
 * Power key interrupt info structures
 * This structure stores the power key information including:
 * debouceStart: if there is a POWER_KEY_INTERRUPT_SIG,assign TRUE
 * isRealPress: if it is not a real press, assign FALSE
 */
#ifdef HAS_INTERRUPT_WAKE_UP_KEY
typedef struct tPowerKeyIntInfoCtr
{
    bool isRealPress;
    bool debouceStart;
}tWakeUpKeyIntInfoCtr;
/* End of Power key interrupt control structures */
#endif

typedef struct tEvtSendCtr
{
    bool        holdEvtSent;
    bool        veryLongholdEvtSent;
    bool        keyDownEvtSent;
    eKeyEvent   currEvtState;/* current state */
    int16       repeatEvtSentNum;
}tEvtSendCtr;

SUBCLASS(cKeySrv, cServer)
    const tKeySrvConfig     *pConfig;
    cKeyDrv                 *pKeyObj[NUM_OF_ALL_KEY];
    uint32                  keyTimer[NUM_OF_ALL_KEY];
    eKeyState               keyAnlysStage[NUM_OF_ALL_KEY];
    tEvtSendCtr             keyEvtCtr[NUM_OF_ALL_KEY];
    uint8                   numOfKeyDown;
    bool                    isReady;
METHODS
    /* public functions */
END_CLASS

/* Implement these so the controller can launch the server */
void KeySrv_StartUp(cPersistantObj *me);
void KeySrv_ShutDown(cPersistantObj *me);
void KeySrv_SendEvtToKeySrv(eKeyID keyId, eKeyEvent evt);

#ifdef HAS_LINEAR_ADC_KNOB_KEY
void KeySrv_ResetKnobKeyIndex(void);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* KEYSRV_H */

