/**
*  @file      LedStripDrv.h
*  @brief     Public declaration for led strip driver
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/12/12
*  @copyright Tymphany Ltd.
*/
#ifndef LED_STRIP_DRV_H
#define LED_STRIP_DRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "pattern.h"
#include "cplus.h"
#include "LedDrv.h"

typedef enum {
	GOOGLE_LED,
	OTHER_STRIP_LED
}eLedStripType;

CLASS(cLedStripDrv)
    eLedStripType stripType;
    void (*Strip_SetPatt)(void * me, eStripPattern patt);
    void (*Strip_Update)(void * me, cLedDrv** ledDrvList);
METHODS
/* public functions */
cLedStripDrv* LedStripDrv_GetStripDrvByIndex(uint8 index);
void LedStripDrv_Ctor(cLedStripDrv* me);
void LedStripDrv_Xtor(cLedStripDrv* me);
END_CLASS

#ifdef	__cplusplus
}
#endif

#endif
