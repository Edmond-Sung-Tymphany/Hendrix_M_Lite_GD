/**
*  @file      LedStripGoogleDrv_priv.h
*  @brief     Private funciton declaration of Google LED
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/12/20
*  @copyright Tymphany Ltd.
*/

#ifndef LED_STRIP_GOOGLE_DRV_PRIV_H
#define LED_STRIP_GOOGLE_DRV_PRIV_H

#include "LedStripGoogleDrv.h"

static void LedStripGoogle_HotWord(cGoogleLedDrv * me);
static void LedStripGoogle_Thinking(cGoogleLedDrv * me);
static void LedStripGoogle_Reset(cGoogleLedDrv * me);
static void LedStripGoogle_Listening_Responding(cGoogleLedDrv * me);

#endif
