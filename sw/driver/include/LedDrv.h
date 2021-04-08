/**
 *  @file      LedDrv.h
 *  @brief     This file defines a platform independent interface to LED
 *  @author    Johnny Fan
 *  @date      11-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef LEDDRV_H
#define LEDDRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "attachedDevices.h"
#include "pattern.h"

typedef enum
{
    BLINK_STYLE=0,
    FADE_STYLE,
    SOLID_STYLE,
    FADE_IN_STYLE,
    FADE_OUT_STYLE,
    FADE_IN_EX_STYLE,
    FADE_OUT_EX_STYLE
}ePatStyle;

typedef struct tPatternData
{
    uint32      periodTime;
    uint32      onTime;
    uint32      duration;
    Color       color;
    ePattern    nextPattern;
    ePatStyle   style;
    const uint8 *fading_data; //only valid if style=FADE_STYLE, FADE_IN_STYLE, FADE_OUT_STYLE
}tPatternData;

typedef struct tLedFunc tLedFunc;

typedef enum
{
    LED_LAYER_BACK = 0,
    LED_LAYER_FORE,
    MAX_LED_LAYER
}eLedLayer;

CLASS(cLedDrv)
    const tDevice   *pConfig;
    const tLedFunc  *pLedFunc;
    const tPatternData  *patt[MAX_LED_LAYER];
    eLedLayer       layer:8;
    Color           prevPattColor;
    Color           prevColor;
    Color           color;
    uint32          pattStart;      /**< pattern start time in tick */
    bool            isPatternRunning:1;
METHODS
/* public functions */
void LedDrv_Ctor(cLedDrv* me, const tDevice* pConfig);
void LedDrv_Xtor(cLedDrv* me);

void LedDrv_PattSet(cLedDrv *me, ePattern pattId);
void LedDrv_PattSetCustom(cLedDrv *me, tPatternData *patt);
void LedDrv_PattStop(cLedDrv *me);
ePattern LedDrv_PattShow(cLedDrv *me);

END_CLASS

struct tLedFunc
{
    void (*LedOn)(cLedDrv* me);
    void (*LedOff)(cLedDrv* me);
    void (*LedSetColor)(cLedDrv* me, Color color);
    void (*LedXtor)(cLedDrv* me);
};

#ifdef	__cplusplus
}
#endif

#endif	/* LedDrv.h */

