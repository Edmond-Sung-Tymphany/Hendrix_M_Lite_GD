/**
 *  @file      PwmLedDrv.h
 *  @brief     This file defines the pic32 specfic interface to LED
 *  @author    Johnny Fan
 *  @date      11-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef PWMLEDDRV_PRIV_H
#define	PWMLEDDRV_PRIV_H

#include "PwmLedDrv.h"

#ifdef	__cplusplus
extern "C" {
#endif

/* Private function */
static void PwmLedDrv_PwmStart(cPwmLedDrv* me, uint8 brightness);
static void PwmLedDrv_PwmStop(cPwmLedDrv* me);


#ifdef	__cplusplus
}
#endif

#endif	/* PWMLEDDRV_PRIV_H */

