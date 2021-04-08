/**
 * @file        PwmLedDrv.h
 * @brief       Led Drvier using the PWM
 * @author      Wesley Lee 
 * @date        2014-07-11
 * @copyright   Tymphany Ltd.
 */

#ifndef PWM_DRV_H
#define PWM_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "LedDrv.h"

SUBCLASS(cPwmLedDrv, cLedDrv)
    /* private data */
    const tPwmLedMap    *pPwmLedConfig;
METHODS
    /* public functions */
void PwmLedDrv_Ctor(cPwmLedDrv *me, const tDevice* pConfig, uint8 index);
void PwmLedDrv_Xtor(cLedDrv *me);

void PwmLedDrv_On(cLedDrv *me);
void PwmLedDrv_Off(cLedDrv *me);
void PwmLedDrv_SetColor(cLedDrv *me, Color color);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* PWM_DRV_H */

