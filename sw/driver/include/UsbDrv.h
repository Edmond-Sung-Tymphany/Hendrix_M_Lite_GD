/**
 * @file        UsbDrv.h
 * @brief       Usb Driver header file
 * @author      Neo Gan
 * @date        2016-10-10
 * @copyright   Tymphany Ltd.

 
 Change History:
 VERSION    : 1    DRAFT      2016-10-10     Neo Gan
 DESCRIPTION: First Draft.
 SCO/ERROR  :
 
 */

#ifndef USB_DRIVER_H
#define USB_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "commonTypes.h"
#include "signals.h"
#include "qp_port.h"


typedef struct{
    /* private data */
    QActive *pSender;

    bool drvIsCreated;
    eUsbDeviceStatus deviceStatus;
    uint8_t step;

    BOOL isHeaderReceived;
    uint32_t recvIndex;
    uint32_t recvSize;
}cUsbDrv;



void UsbDrv_Ctor(cUsbDrv *me);
void UsbDrv_Xtor(cUsbDrv *me);

eUsbDeviceStatus UsbDrv_GetStatus(void);
void UsbDrv_RecvData(uint8_t *rxBuf,uint32_t rxSize);
void UsbDrv_SendData(uint8_t *buf, uint32_t size);

void UsbDrv_HandleUnknown(cUsbDrv * const me, QEvt const * const e);

void UsbDrv_RequestHandler(cUsbDrv * const me, QEvt const * const e);


#endif /* USB_DRIVER_H */


