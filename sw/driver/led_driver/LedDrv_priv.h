/**
 *  @file      LedDrv_priv.h
 *  @brief     This file defines a platform independent interface to LED
 *  @author    Johnny Fan
 *  @date      11-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef LED_DRIVER_PRIVATE_H
#define	LED_DRIVER_PRIVATE_H


#include "assert.h"
#include "LedDrv.h"

#ifdef	__cplusplus
extern "C" {
#endif

static bool isPattForever(const tPatternData *patt);
static ePattern getPattId(const tPatternData *patt);
static eLedLayer getPattLayer(const tPatternData *patt);
static Color getFadeColor(const tPatternData *patt, uint32 elapsed);
static Color getFadeInColor(const tPatternData *patt, uint32 elapsed);
static Color getFadeOutColor(const tPatternData *patt, uint32 elapsed);
static Color getFadeInExColor(const tPatternData *patt, uint32 elapsed, Color color);
static Color getFadeOutExColor(const tPatternData *patt, uint32 elapsed, Color color);


static void LedDrv_UpdateColor(cLedDrv *me);

#ifdef	__cplusplus
}
#endif

#endif	/* LED_DRIVER_PRIVATE_H */

