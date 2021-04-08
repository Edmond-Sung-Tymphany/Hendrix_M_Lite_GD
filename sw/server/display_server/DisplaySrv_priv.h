/**
 *  @file      DisplaySrv_priv.h
 *  @brief     the private inteface file for display server
 *  @author    Edmond Sung
 *  @date      20-Apr-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef DISPLAY_SERVER_PRIV_H
#define	DISPLAY_SERVER_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "DisplaySrv.h"
#define RESET_TICK 0
typedef struct tPattDisplayCtrl
{
    uint8 const  *pString;
    uint8 const  *pStringHead;
    BOOL    patternStart;
    uint16  currentTicks;
    uint8   strTempBuff[NUM_OF_SCREEN_DIGIT];
}tPattDisplayCtrl;

#ifdef	__cplusplus
}
#endif

#endif	/* DISPLAY_SERVER_PRIV_H */

