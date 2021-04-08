/**
 * @file        A2bSrv_priv.h
 * @brief       A2B Server
 * @author      Edmond Sung
 * @date        2016-12-23
 * @copyright   Tymphany Ltd.
 */

#ifndef A2BSRV_PRIV_H
#define A2BSRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "A2bSrv.h"

/* private state functions */
QState A2bSrv_Initial(cA2bSrv * const me);
QState A2bSrv_Active(cA2bSrv * const me, QEvt const * const e);
QState A2bSrv_DeActive(cA2bSrv * const me, QEvt const * const e);


/*****************************************************************
 * Private function declaration
 *****************************************************************/
static void A2bSrv_PeriodicTask(cA2bSrv * const me);
static bool A2bSrv_DrvInitAndTick(cA2bSrv * const me);



#ifdef __cplusplus
}
#endif

#endif /* A2BSRV_PRIV_H */

