/**
 *  @file      DisplaySrv.h
 *  @brief     This file defines the interface of Displau server
 *              User can use public functions to set Display event and control display
 *  @Functions: 1. turn on/off display
 *               2. set some string or character to display
 *               3. handle scrolling or flipping behavior
 *  @author    Edmond Sung
 *  @date      20-Apr-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef DISPLAYSRV_H
#define	DISPLAYSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"
#include "attachedDevices.h"

typedef enum
{
    DISPLAY_PRIMARY = 0,
    DISPLAY_SECONDARY
}eDisplayPriority;

typedef enum
{
    DISPLAY_STYLE_OFF,
    DISPLAY_STYLE_STATIC,
    DISPLAY_STYLE_SCROLL_LEFT,  //This is the pattern used for the case that when the string length is larger than NUM_OF_SCREEN_DIGIT
    DISPLAY_STYLE_SCROLL_RIGHT,
    DISPLAY_STYLE_SCROLL_UP,
    DISPLAY_STYLE_SCROLL_DOWN
}eDisplayStyle;

typedef enum
{
    SEGMENT_0,
    SEGMENT_1,
    SEGMENT_2,
    SEGMENT_3,
    SEGMENT_4,
    SEGMENT_5,
    SEGMENT_6,
    SEGMENT_7,
    SEGMENT_8,
    SEGMENT_9,
    SEGMENT_10,
    SEGMENT_11,
    SEGMENT_12,
    SEGMENT_13,
    SEGMENT_14,
    ALL_SEGMENT_ON,
    ALL_SEGMENT_OFF,
    SEGMENT_MAX_ID
}eSegmentId;

typedef enum
{
    DIGIT_0,
    DIGIT_1,
    DIGIT_2,
    DIGIT_3,
    DIGIT_4,
    DIGIT_5,
    DIGIT_6,
    DIGIT_7,
    DIGIT_MAX_ID
}eDigitId;

typedef struct tDisplaySrvConfig
{
    eDisplayStyle   displayStyle;
    uint16          frameInterval;  //Should be init with ms
#ifdef HAS_SCREEN_DIM_CTRL
    int32           dimTimeThreshold;
    BOOL            dimEnabled;
#endif
}tDisplaySrvConfig;

REQ_EVT(DisplayReqEvt)
    BOOL            cleanScreen;
    uint8 const    *displayString;
END_REQ_EVT(DisplayReqEvt)

REQ_EVT(DisplaySetDimTimeEvt)
    uint32      dimTime;
    BOOL        enable;
END_REQ_EVT(DisplaySetDimTimeEvt)

REQ_EVT(DisplayDebugReqEvt)
    eDigitId   digitId;
    eSegmentId segmentId;
END_REQ_EVT(DisplayDebugReqEvt)

REQ_EVT(DisplayBrightnessEvt)
    uint16  brightnessLevel;
END_REQ_EVT(DisplayBrightnessEvt)

/* Can use the public function below to set these event autumatically */
SUBCLASS(cDisplaySrv, cServer)
    tDisplaySrvConfig *displaySrvConf;
METHODS
    /* Public Functions */
END_CLASS

void DisplaySrv_StartUp(cPersistantObj *me);
void DisplaySrv_ShutDown(cPersistantObj *me);
void DisplaySrv_SendString(uint8 const *pString);
#ifdef HAS_SCREEN_DIM_CTRL
void DisplaySrv_SetDimTime(uint32 dimTime, BOOL enable);
#endif
void DisplaySrv_CleanScreen();
void DisplaySrv_ResumeScreen();
void DispalySrv_SetBrightnessLevel(uint16 brightnessLevel);

#ifdef	__cplusplus
}
#endif

#endif	/* DISPLAYSRV_H */

