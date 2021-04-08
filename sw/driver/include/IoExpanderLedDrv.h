/**
 * @file        IoExpanderLedDrv.h
 * @brief       Led Drvier using the IO Expander
 * @author      Wesley Lee 
 * @date        2014-07-11
 * @copyright   Tymphany Ltd.
 */

#ifndef IOEXPANDERLED_DRV_H
#define IOEXPANDERLED_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "IoExpanderDrv.h"
#include "LedDrv.h"

SUBCLASS(cIoExpanderLedDrv, cLedDrv)
    /* private data */
    const tIoExpanderLedMap *pIoeLedConfig;
    cIoExpanderDrv *pIoExpander;
METHODS
    /* public functions */
void IoExpanderLedDrv_Ctor(cIoExpanderLedDrv *me, const tDevice* pConfig, cIoExpanderDrv *ioeDrv, uint8 index);
void IoExpanderLedDrv_Xtor(cLedDrv *me);

void IoExpanderLedDrv_On(cLedDrv *me);
void IoExpanderLedDrv_Off(cLedDrv *me);
void IoExpanderLedDrv_SetColor(cLedDrv *me, Color color);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* IOEXPANDERLED_DRV_H */

