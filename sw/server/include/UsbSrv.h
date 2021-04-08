/**
 * @file        UsbSrv.h
 * @brief       it's the server to control the USB module
 * @author      Neo Gan
 * @date        2016-10-10
 * @copyright   Tymphany Ltd.
 */


#ifndef USB_SERVER_H
#define USB_SERVER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "server.h"



typedef enum
{
    /* receive data request */
    USB_START_TIMER_REQ,
    USB_RECV_TIMEOUT_REQ,
    USB_RECV_FRAME_REQ,

    /* send data request */
    USB_RESPONSE_TIMEOUT_REQ,

    /* control request */
    USB_CTRL_REQ,
}eUsbRequestType;

#ifdef HW_GPIO_DEBUG
typedef enum
{
    GPIO_PORT_READ,
    GPIO_PORT_WRITE,
}eGpioPortReadWriteAction;

REQ_EVT(GpioDebugReqEvt)
    eIoPort                     port;
    eIoBit                      bit;
    eGPIOInitAttr               action; //Indicate GPIO_ACTIVE_xxx for input pin, GPIO_DEFAULT_OUT_xxx for output pin
    eGpioPortReadWriteAction    mode;
    eGPIODrection               direction;
END_REQ_EVT(GpioDebugReqEvt)


RESP_EVT(GpioDebugRespEvt)
    uint8                       value;
END_RESP_EVT(GpioDebugRespEvt)
#endif

#ifdef PT_ADC_DEBUG
REQ_EVT(AdcReqEvt)
    eIoPort                     port;
    eIoBit                      bit;
END_REQ_EVT(AdcReqEvt)

RESP_EVT(AdcRespEvt)
    eIoPort                     port;
    eIoBit                      bit;
    uint16                      value;
END_RESP_EVT(AdcRespEvt)
#endif

#ifdef PT_I2C_DEBUG
typedef enum
{
    IIC_READ,
    IIC_WRITE
}eIICReadWrite;

#define SETTING_CHUNK_SIZE  (SIZE_OF_LARGE_EVENTS - 8)
#define IIC_DATA_MAX_SIZE   (SETTING_CHUNK_SIZE -  sizeof(eIICReadWrite)  - sizeof(eI2C_Channel)- sizeof(uint32)-sizeof(eI2CRegAddLen) - sizeof(uint16) - sizeof(uint8) - sizeof(eEvtReturn))  // 73
REQ_EVT(IICReqEvt)
    eIICReadWrite               mode;
    eI2C_Channel                channel;
    uint32                      regAddr;
    eI2CRegAddLen               regAddrLen;
    uint16                      length;
    uint8                       devAddr;
    uint8                       data[IIC_DATA_MAX_SIZE];
END_REQ_EVT(IICReqEvt)

RESP_EVT(IICRespEvt)
    eIICReadWrite               mode;
    eI2C_Channel                channel;
    uint32                      regAddr;
    eI2CRegAddLen               regAddrLen;
    uint16                      length;
    uint8                       devAddr;
    uint8                       data[IIC_DATA_MAX_SIZE];
END_RESP_EVT(IICRespEvt)
#endif

REQ_EVT(UsbReqEvt)    
    eUsbRequestType type;
    bool enable;
    uint32 param;
END_REQ_EVT(UsbReqEvt)

SUBCLASS(cUsbSrv, cServer)
    /* private data */
    uint32_t timeCount;

    /**/
    eUsbDeviceStatus deviceStatus;
    
    /* record message is sending or not */
    bool isBusy;
METHODS
    /* public functions */
END_CLASS

IND_EVT(UsbSrvState)
    eUsbDeviceStatus deviceStatus;
END_IND_EVT(UsbSrvState)

REQ_EVT(usbCmdEvt)
END_REQ_EVT(usbCmdEvt)

void UsbSrv_StartUp(cPersistantObj *me);
void UsbSrv_ShutDown(cPersistantObj *me);

  /* State function definitions */
QState UsbSrv_Initial(cUsbSrv * const me, QEvt const * const e);
QState UsbSrv_Active(cUsbSrv * const me, QEvt const * const e);
QState UsbSrv_DeActive(cUsbSrv * const me, QEvt const * const e);
void usbDataReceive(uint8_t *src, uint32_t src_len);
bool UsbSrv_isDeviceActive(void);

#ifdef __cplusplus
}
#endif

#endif /* USB_SERVER_H */

