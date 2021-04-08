/**
 * @file        BleSrv_priv.h
 * @brief       it's the server to control the BT module
 * @author      Johnny Fan
 * @date        2014-05-11
 * @copyright   Tymphany Ltd.
 */

#ifndef BLESRV_PRIV_H
#define BLESRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"
#include "bsp.h"  
#include "controller.h"
#include "trace.h"
 
#include "BleSrv.h"
#include "BleSrv.Config"
#include "BleDrv.h"


enum InternalSignals
{
    BLE_TIMEOUT_SIG = MAX_SIG,
};

typedef struct
{
    bool countEnable;
    bool delayEnable;
    uint32 countInMs;
    int32 delayInMs;
} tBtTimeStruct;


/* State function definitions */
static QState BleSrv_Initial(cBleSrv * const me, QEvt const * const e);
static QState BleSrv_PreActive(cBleSrv * const me, QEvt const * const e);
static QState BleSrv_Active(cBleSrv * const me, QEvt const * const e);
static QState BleSrv_PreDeActive(cBleSrv * const me, QEvt const * const e);
static QState BleSrv_DeActive(cBleSrv * const me, QEvt const * const e);

#ifdef __cplusplus
}
#endif

#endif /* BLESRV_PRIV_H */
