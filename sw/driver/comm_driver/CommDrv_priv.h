/**
 * @file        CommDrv_priv.h
 * @brief       It's the driver to communicate with SoC
 * @author      Eason Huang 
 * @date        2016-06-02
 * @copyright   Tymphany Ltd.
 */
#ifndef COMMDRV_PRIV_H
#define COMMDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "CommDrv.h"

/* private functions / data */
static void CommDrv_OnReceive(void* p);
static void CommDrv_ACK(void);

#ifdef __cplusplus
}
#endif

#endif /* COMMDRV_PRIV_H */