/**
 * @file        DebugSrv_priv.h
 * @brief       this\ is\ debug\ \ server
 * @author      Dmitry.Abdulov 
 * @date        2014-02-12
 * @copyright   Tymphany Ltd.
 */

#ifndef DEBUGSRV_PRIV_H
#define DEBUGSRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "DebugSrv.h"

uint32 DebugSrvOutput(char* s, uint8 len);

#ifdef __cplusplus
}
#endif

#endif /* DEBUGSRV_PRIV_H */
 