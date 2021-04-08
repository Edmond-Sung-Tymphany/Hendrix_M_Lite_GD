/**
 * @file        BleDrv_priv.h
 * @brief       It's the bluetooth driver to control the ROM base CSR module by toggling GPIO
 * @author      Neo Gan
 * @date        2018-03-23
 * @copyright   Tymphany Ltd.
 */
#ifndef BLEDRV_PRIV_H
#define BLEDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BleDrv.h"

/*Initialize sections*/
static void BleDrv_InitSection0(void *p);

/*De-Initialize sections*/
static void BleDrv_DeInitSection0(void *p);

/**/
static void BleDrv_ResetRxBuffer(cBleDrv *me);

/**/
static void BleDrv_RxCallback(void *p);

/**/
static void BleDrv_HandleCompleteFrame(cBleDrv * const me);

/**/
static void BleDrv_RecvStartTimeoutCallBack(void *pCbPara);

#ifdef __cplusplus
}
#endif

#endif /* BLUETOOTH_DRIVER_PRIVATE_H */
