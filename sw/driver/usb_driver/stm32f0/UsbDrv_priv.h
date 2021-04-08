/**
 * @file        UsbDrv_priv.h
 * @brief       It's a driver for USB controller
 * @author      Neo Gan
 * @date        2016-10-10
 * @copyright   Tymphany Ltd.
 */
#ifndef USB_DRIVER_PRIVATE_H
#define USB_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include  "projBsp.h"

#include "UsbDrv.h"

static void UsbDrv_ResetRxBuffer(cUsbDrv *me);

static void UsbDrv_HandleCompleteFrame(cUsbDrv * const me);

static void UsbDrv_RecvStartTimeoutCallBack(void *pCbPara);


#ifdef __cplusplus
}
#endif

#endif /* USB_DRIVER_PRIVATE_H */
