#ifndef LED_STRIP_GOOGLE_H
#define LED_STRIP_GOOGLE_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "LedStripDrv.h"
#include "deviceTypes_v2.h"

#define GOOGLE_LED_NUM (4)

typedef struct tGoogleLedConfig
{
    const uint8 ledNum;
    const eLedMask * leds;
    uint16 fadingTime;
    uint16 intervalTime;//fot THINK
    uint16 holdingTime;//for HOTWORD
    Color color;
} tGoogleLedConfig;

SUBCLASS(cGoogleLedDrv, cLedStripDrv)
    tGoogleLedConfig * stripConfig;
    eStripPattern stripPattNow;
    eStripPattern stripPattNext;
    uint8 ledIndex;
    uint32 holdingTime;
    bool isFadingIn;
    Color colors[GOOGLE_LED_NUM];
    uint32 startTime;
METHODS
void LedStripGoogle_Ctor(cGoogleLedDrv * me);
void LedStripGoogle_Xtor(cGoogleLedDrv * me);
void LedStripGoogle_SetPatt(void * me, eStripPattern patt);
void LedStripGoogle_Update(void * me, cLedDrv** ledDrvList);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif
