/**
 * @file        UsbSrv_priv.h
 * @brief       it's the server to control the USB module
 * @author      Neo Gan
 * @date        2016-09-08
 * @copyright   Tymphany Ltd.
 */

#ifndef USB_SERVER_PRIVATE_H
#define USB_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

enum privateSignals
{
    USB_TIMEOUT_SIG = MAX_SIG,
};

QState UsbSrv_StateHandler(cUsbSrv * const me,  UsbSrvState *pState);
QState UsbSrv_RecvDataHandler(cUsbSrv * const me);
QState UsbSrv_SendDataHnadler(cUsbSrv * const me);

#ifdef __cplusplus
}
#endif

#endif /* USB_SERVER_PRIVATE_H */
