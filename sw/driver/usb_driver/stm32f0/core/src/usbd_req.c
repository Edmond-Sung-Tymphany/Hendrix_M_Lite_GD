/**
  ******************************************************************************
  * @file    usbd_req.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides the standard USB requests following chapter 9.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/
#include "commonTypes.h"
#include "deviceTypes.h"

#include "usbd_def.h"
#include "usbd_req.h"
#include "usbd_cdc_desc.config"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint32_t usb_device_address = 0x00;
uint8_t USBD_StrDesc[USB_MAX_STR_DESC_SIZ];

/* Private function prototypes -----------------------------------------------*/
static void USBD_GetDescriptor(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_SetAddress(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_SetConfig(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_GetConfig(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_GetStatus(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_SetFeature(DCD_DEV *pdev, USB_SETUP_REQ *req);
static void USBD_ClrFeature(DCD_DEV *pdev, USB_SETUP_REQ *req);
static uint8_t USBD_GetLen(uint8_t *buf);
static void USBD_GetSerialNum(void);
static void USBD_IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  USBD_StdDevReq
  *         Handle standard usb device requests
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
USBD_Status USBD_StdDevReq(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
    USBD_Status ret = USBD_OK;  

    switch (req->bRequest) 
    {
    case USB_REQ_GET_DESCRIPTOR: 
        USBD_GetDescriptor (pdev, req) ;
        break;

    case USB_REQ_SET_ADDRESS:                      
        USBD_SetAddress(pdev, req);
        break;

    case USB_REQ_SET_CONFIGURATION:                    
        USBD_SetConfig (pdev , req);
        break;

    case USB_REQ_GET_CONFIGURATION:                 
        USBD_GetConfig (pdev , req);
        break;

    case USB_REQ_GET_STATUS:                                  
        USBD_GetStatus (pdev , req);
        break;

    case USB_REQ_SET_FEATURE:   
        USBD_SetFeature (pdev , req);    
        break;

    case USB_REQ_CLEAR_FEATURE:                                   
        USBD_ClrFeature (pdev , req);
        break;

    default:  
        USBD_CtlError(pdev , req);
        break;
    }

    return ret;
}

/**
  * @brief  USBD_StdItfReq
  *         Handle standard usb interface requests
  * @param  pdev: USB device instance
  * @param  req: usb request
  * @retval status
  */
USBD_Status USBD_StdItfReq(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
    USBD_Status ret = USBD_OK; 

    switch (pdev->device_status)
    {
    case USB_STA_CONFIGURED:
        if (LOBYTE(req->wIndex) <= USBD_ITF_MAX_NUM) 
        {
          ret = (USBD_Status) (pdev->class_cb->Setup (pdev, req));
          
          if((req->wLength == 0) && (ret == USBD_OK))
          {
             USBD_CtlSendStatus(pdev);
          }
        } 
        else 
        {                                               
           USBD_CtlError(pdev , req);
        }
        break;

    default:
        USBD_CtlError(pdev , req);
        break;
    }

    return ret;
}

/**
  * @brief  USBD_StdEPReq
  *         Handle standard usb endpoint requests
  * @param  pdev: USB device instance
  * @param  req: usb request
  * @retval status
  */
USBD_Status USBD_StdEPReq(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
    uint8_t   ep_addr;
    uint32_t USBD_ep_status  = 0; 
    USBD_Status ret = USBD_OK; 

    ep_addr  = LOBYTE(req->wIndex);   

    switch (req->bRequest) 
    {  
    case USB_REQ_SET_FEATURE :
        switch (pdev->device_status)
        {
        case USB_STA_ADDRESSED:          
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) 
            {
                DCD_EP_Stall(ep_addr);
            }
            break;	

        case USB_STA_CONFIGURED:   
            if (req->wValue == USB_FEATURE_EP_HALT)
            {
                if ((ep_addr != 0x00) && (ep_addr != 0x80)) 
                { 
                    DCD_EP_Stall(ep_addr);
                }
            }
            pdev->class_cb->Setup(pdev, req);
            USBD_CtlSendStatus(pdev);
            break;

        default:                         
            USBD_CtlError(pdev, req);
            break;    
        }
        break;

    case USB_REQ_CLEAR_FEATURE :
        switch (pdev->device_status)
        {
        case USB_STA_ADDRESSED:          
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) 
            {
                DCD_EP_Stall(ep_addr);
            }
            break;	

        case USB_STA_CONFIGURED:   
            if (req->wValue == USB_FEATURE_EP_HALT)
            {
                if ((ep_addr != 0x00) && (ep_addr != 0x80)) 
                {        
                    DCD_EP_ClrStall(ep_addr);
                }
            }
            pdev->class_cb->Setup (pdev, req);
            USBD_CtlSendStatus(pdev);
            break;

        default:                         
            USBD_CtlError(pdev , req);
            break;    
        }
        break;

    case USB_REQ_GET_STATUS:                  
        switch (pdev->device_status)
        {
        case USB_STA_ADDRESSED:          
            if ((ep_addr != 0x00) && (ep_addr != 0x80)) 
            {
                DCD_EP_Stall(ep_addr);
            }
            break;	

        case USB_STA_CONFIGURED:         
            if ((ep_addr & 0x80)== 0x80)
            {
                if(pdev->in_ep[ep_addr & 0x7F].is_stall)
                {
                    USBD_ep_status = 0x0001;     
                }
                else
                {
                    USBD_ep_status = 0x0000;  
                }
            }
            else if ((ep_addr & 0x80)== 0x00)
            {
                if(pdev->out_ep[ep_addr].is_stall)
                {
                    USBD_ep_status = 0x0001;     
                }
                else 
                {
                    USBD_ep_status = 0x0000;     
                }      
            }

            USBD_CtlSendData(pdev, (uint8_t *)&USBD_ep_status, 2);
            break;

        default:                         
            USBD_CtlError(pdev, req);
            break;
        }
        break;

    default:
        break;
    }

    return ret;
}
/**
  * @brief  USBD_GetDescriptor
  *         Handle Get Descriptor requests
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_GetDescriptor(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
    uint16_t len;
    uint8_t *pbuf;

    switch (req->wValue >> 8)
    {
    case USB_DESC_TYPE_BOS:

        break;

    case USB_DESC_TYPE_DEVICE:
        pbuf = (uint8_t*)USBD_DeviceDesc;
        len  = sizeof(USBD_DeviceDesc);
        if (req->wLength == 64)
        {
            len = 8;
        }
        break;

    case USB_DESC_TYPE_CONFIGURATION:
        pbuf = (uint8_t*)USBD_ConfigDesc;
        len  = sizeof(USBD_ConfigDesc);
        break;

    case USB_DESC_TYPE_STRING:
        switch ((uint8_t)(req->wValue))
        {
            case USBD_IDX_LANGID_STR:
                pbuf = (uint8_t*)USBD_LangIDDesc;
                len  = sizeof(USBD_LangIDDesc);
                break;

            case USBD_IDX_MFC_STR:
                USBD_GetString((uint8_t*)USBD_MANUFACTURER_STRING, USBD_StrDesc, &len);
                pbuf = (uint8_t*)USBD_StrDesc;
                break;

            case USBD_IDX_PRODUCT_STR:
                USBD_GetString((uint8_t*)USBD_PRODUCT_FS_STRING, USBD_StrDesc, &len);
                pbuf = (uint8_t*)USBD_StrDesc;
                break;

            case USBD_IDX_SERIAL_STR:
                USBD_GetSerialNum();
                pbuf = (uint8_t*)USBD_StringSerial;
                len  = sizeof(USBD_StringSerial);
                break;

            case USBD_IDX_CONFIG_STR:
                USBD_GetString((uint8_t*)USBD_CONFIGURATION_FS_STRING, USBD_StrDesc, &len);
                pbuf = (uint8_t*)USBD_StrDesc;
                break;

            case USBD_IDX_INTERFACE_STR:
                USBD_GetString((uint8_t*)USBD_INTERFACE_FS_STRING, USBD_StrDesc, &len);
                pbuf = (uint8_t*)USBD_StrDesc;
                break;

            default:
                USBD_CtlError(pdev, req);
                return;
        }
        break;

    case USB_DESC_TYPE_DEVICE_QUALIFIER:
        USBD_CtlError(pdev, req);
        return;

    case USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION:
        USBD_CtlError(pdev, req);
        return;

    default:
        USBD_CtlError(pdev , req);
        return;
    }

    if((len != 0) && (req->wLength != 0))
    {
        len = MIN(len , req->wLength);
        USBD_CtlSendData(pdev, pbuf, len);
    }

}
/**
  * @brief  USBD_SetAddress
  *         Set device address
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_SetAddress(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
  uint8_t  dev_addr; 
  
  if ((req->wIndex == 0) && (req->wLength == 0)) 
  {
    dev_addr = (uint8_t)(req->wValue) & 0x7F;
    
    if (pdev->device_status == USB_STA_CONFIGURED)
    {
      USBD_CtlError(pdev , req);
    } 
    else 
    {
      pdev->device_address = dev_addr;
      USBD_SetDeviceAddress(dev_addr);
      USBD_CtlSendStatus(pdev);        

      if (dev_addr != 0) 
      {
        pdev->device_status  = USB_STA_ADDRESSED;
      } 
      else 
      {
        pdev->device_status  = USB_STA_DEFAULT;
      }
    }
  } 
  else 
  {
     USBD_CtlError(pdev, req);
  } 
}

/**
  * @brief  USBD_SetConfig
  *         Handle Set device configuration request
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_SetConfig(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
  
  static uint8_t  cfgidx;
  
  cfgidx = (uint8_t)(req->wValue);                 
  
  if (cfgidx > USBD_CFG_MAX_NUM ) 
  {
     USBD_CtlError(pdev, req);         
  } 
  else 
  {
    switch (pdev->device_status)
    {
    case USB_STA_ADDRESSED:
      if (cfgidx) 
      {                                			   							   							   				
        pdev->device_config = cfgidx;
        pdev->device_status = USB_STA_CONFIGURED;
        USBD_SetCfg(pdev, cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else 
      {
         USBD_CtlSendStatus(pdev);
      }
      break;
      
    case USB_STA_CONFIGURED:
      if (cfgidx == 0) 
      {                           
        pdev->device_status = USB_STA_ADDRESSED;
        pdev->device_config = cfgidx;          
        USBD_ClrCfg(pdev, cfgidx);
        USBD_CtlSendStatus(pdev);
      } 
      else  if (cfgidx != pdev->device_config)
      {
        /* Clear old configuration */
        USBD_ClrCfg(pdev , pdev->device_config);
        
        /* set new configuration */
        pdev->device_config = cfgidx;
        USBD_SetCfg(pdev, cfgidx);
        USBD_CtlSendStatus(pdev);
      }
      else
      {
        USBD_CtlSendStatus(pdev);
      }
      break;
      
    default:					
       USBD_CtlError(pdev , req);                     
      break;
    }
  }
}

/**
  * @brief  USBD_GetConfig
  *         Handle Get device configuration request
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_GetConfig(DCD_DEV  *pdev, USB_SETUP_REQ *req)
{
  uint32_t  USBD_default_cfg  = 0;
 
  if (req->wLength != 1) 
  {                   
     USBD_CtlError(pdev , req);
  }
  else 
  {
    switch (pdev->device_status )  
    {
    case USB_STA_ADDRESSED:                     
      USBD_CtlSendData (pdev, (uint8_t *)&USBD_default_cfg, 1);
      break;
      
    case USB_STA_CONFIGURED:
      USBD_CtlSendData (pdev, &pdev->device_config, 1);
      break;
      
    default:
       USBD_CtlError(pdev , req);
      break;
    }
  }
}

/**
  * @brief  USBD_GetStatus
  *         Handle Get Status request
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_GetStatus(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
  uint32_t  USBD_cfg_status = 0;  
  switch (pdev->device_status) 
  {
  case USB_STA_ADDRESSED:
  case USB_STA_CONFIGURED:
    
#ifdef USBD_SELF_POWERED
    USBD_cfg_status = USB_CONFIG_SELF_POWERED;                                    
#else
    USBD_cfg_status = 0x00;                                    
#endif
                      
    if (pdev->DevRemoteWakeup) 
    {
      USBD_cfg_status |= USB_CONFIG_REMOTE_WAKEUP;                                
    }
    
    USBD_CtlSendData (pdev, (uint8_t *)&USBD_cfg_status, 2);
    break;
    
  default :
    USBD_CtlError(pdev , req);                        
    break;
  }
}


/**
  * @brief  USBD_SetFeature
  *         Handle Set device feature request
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_SetFeature(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
 
  if (req->wValue == USB_FEATURE_REMOTE_WAKEUP)
  {
    pdev->DevRemoteWakeup = 1;  
    pdev->class_cb->Setup (pdev, req);   
    USBD_CtlSendStatus(pdev);
  }
}


/**
  * @brief  USBD_ClrFeature
  *         Handle clear device feature request
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval status
  */
static void USBD_ClrFeature(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
  switch (pdev->device_status)
  {
  case USB_STA_ADDRESSED:
  case USB_STA_CONFIGURED:
    if (req->wValue == USB_FEATURE_REMOTE_WAKEUP) 
    {
      pdev->DevRemoteWakeup = 0;
      pdev->class_cb->Setup (pdev, req);
      USBD_CtlSendStatus(pdev);
    }
    break;
    
  default :
     USBD_CtlError(pdev , req);
    break;
  }
}

/**
  * @brief  USBD_ParseSetupRequest 
  *         Copy buffer into setup structure
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval None
  */

void USBD_ParseSetupRequest(DCD_DEV *pUsbDCD, USB_SETUP_REQ *req)
{
  req->bmRequest     = *(uint8_t *)  (pUsbDCD->setup_packet);
  req->bRequest      = *(uint8_t *)  (pUsbDCD->setup_packet +  1);
  req->wValue        = SWAPBYTE      (pUsbDCD->setup_packet +  2);
  req->wIndex        = SWAPBYTE      (pUsbDCD->setup_packet +  4);
  req->wLength       = SWAPBYTE      (pUsbDCD->setup_packet +  6);
}

/**
  * @brief  USBD_CtlError 
  *         Handle USB low level Error
  * @param  pdev: device instance
  * @param  req: usb request
  * @retval None
  */

void USBD_CtlError(DCD_DEV *pdev, USB_SETUP_REQ *req)
{
  DCD_EP_Stall(0);
}


/**
  * @brief  USBD_GetString
  *         Convert Ascii string into unicode one
  * @param  desc : descriptor buffer
  * @param  unicode : Formatted string buffer (unicode)
  * @param  len : descriptor length
  * @retval None
  */
void USBD_GetString(uint8_t *desc, uint8_t *unicode, uint16_t *len)
{
  uint8_t idx = 0;
  
  if (desc != NULL) 
  {
    *len =  USBD_GetLen(desc) * 2 + 2;    
    unicode[idx++] = *len;
    unicode[idx++] =  USB_DESC_TYPE_STRING;
    
    while (*desc != NULL) 
    {
      unicode[idx++] = *desc++;
      unicode[idx++] =  0x00;
    }
  } 
}

/**
  * @brief  USBD_GetLen
  *         return the string length
   * @param  buf : pointer to the ascii string buffer
  * @retval string length
  */
static uint8_t USBD_GetLen(uint8_t *buf)
{
    uint8_t  len = 0;

    while (*buf != NULL) 
    {
        len++;
        buf++;
    }

    return len;
}


/**
  * @brief  Create the serial number string descriptor 
  * @param  None 
  * @retval None
  */
static void USBD_GetSerialNum(void)
{
  uint32_t Device_Serial0, Device_Serial1, Device_Serial2;
  
  Device_Serial0 = *(uint32_t*)Device1_Identifier;
  Device_Serial1 = *(uint32_t*)Device2_Identifier;
  Device_Serial2 = *(uint32_t*)Device3_Identifier;
  
  Device_Serial0 += Device_Serial2;
  
  if (Device_Serial0 != 0)
  {
    USBD_IntToUnicode (Device_Serial0, &USBD_StringSerial[2] ,8);
    USBD_IntToUnicode (Device_Serial1, &USBD_StringSerial[18] ,4);
  }
}

/**
  * @brief  Convert Hex 32Bits value into char 
  * @param  value: value to convert
  * @param  pbuf: pointer to the buffer 
  * @param  len: buffer length
  * @retval None
  */
static void USBD_IntToUnicode(uint32_t value, uint8_t *pbuf, uint8_t len)
{
  uint8_t idx = 0;
  
  for( idx = 0 ; idx < len ; idx ++)
  {
    if( ((value >> 28)) < 0xA )
    {
      pbuf[ 2* idx] = (value >> 28) + '0';
    }
    else
    {
      pbuf[2* idx] = (value >> 28) + 'A' - 10; 
    }
    
    value = value << 4;
    
    pbuf[ 2* idx + 1] = 0;
  }
}

uint8_t * USBD_GetConfigDescriptor(void)
{
    return (uint8_t*)USBD_ConfigDesc;
}

/**
  * @brief  USBD_CtlSendData
  *         send data on the ctl pipe
  * @param  pdev: device instance
  * @param  buff: pointer to data buffer
  * @param  len: length of data to be sent
  * @retval status
  */
USBD_Status USBD_CtlSendData(DCD_DEV *pdev, uint8_t *pbuf, uint16_t len)
{
    pdev->in_ep[0].total_data_len = len;
    pdev->in_ep[0].rem_data_len   = len;
    pdev->device_state = USB_EP0_DATA_IN;

    DCD_EP_Tx(EP_IN_CTRL, pbuf, len);
    return USBD_OK;
}

/**
  * @brief  USBD_CtlContinueSendData
  *         continue sending data on the ctl pipe
  * @param  pdev: device instance
  * @param  buff: pointer to data buffer
  * @param  len: length of data to be sent
  * @retval status
  */
USBD_Status USBD_CtlContinueSendData(uint8_t *pbuf, uint16_t len)
{
    DCD_EP_Tx(EP_IN_CTRL, pbuf, len);
    return USBD_OK;
}

/**
  * @brief  USBD_CtlPrepareRx
  *         receive data on the ctl pipe
  * @param  pdev: USB device instance
  * @param  buff: pointer to data buffer
  * @param  len: length of data to be received
  * @retval status
  */
USBD_Status USBD_CtlPrepareRx(DCD_DEV *pdev, uint8_t *pbuf, uint16_t len)
{
    pdev->out_ep[0].total_data_len = len;
    pdev->out_ep[0].rem_data_len   = len;
    pdev->device_state = USB_EP0_DATA_OUT;

    DCD_EP_PrepareRx(EP_IN_CTRL, pbuf, len);
    return USBD_OK;
}

/**
  * @brief  USBD_CtlContinueRx
  *         continue receive data on the ctl pipe
  * @param  pdev: USB device instance
  * @param  buff: pointer to data buffer
  * @param  len: length of data to be received
  * @retval status
  */
USBD_Status USBD_CtlContinueRx(uint8_t *pbuf, uint16_t len)
{
    DCD_EP_PrepareRx(EP_IN_CTRL, pbuf, len);
    return USBD_OK;
}
/**
  * @brief  USBD_CtlSendStatus
  *         send zero length packet on the ctl pipe
  * @param  pdev: USB device instance
  * @retval status
  */
USBD_Status USBD_CtlSendStatus(DCD_DEV *pdev)
{
    pdev->device_state = USB_EP0_STATUS_IN;

    DCD_EP_Tx(EP_IN_CTRL, NULL, 0);
    return USBD_OK;
}

/**
  * @brief  USBD_CtlReceiveStatus
  *         receive zero length packet on the ctl pipe
  * @param  pdev: USB device instance
  * @retval status
  */
USBD_Status USBD_CtlReceiveStatus(DCD_DEV *pdev)
{
    pdev->device_state = USB_EP0_STATUS_OUT;

    DCD_EP_PrepareRx(EP_IN_CTRL, NULL, 0);
    return USBD_OK;
}


/**
  * @brief  USBD_GetRxCount
  *         returns the received data length
  * @param  pdev: USB device instance
  *         epnum: endpoint index
  * @retval Rx Data blength
  */
uint16_t USBD_GetRxCount(DCD_DEV *pdev, uint8_t epnum)
{
    return pdev->out_ep[epnum].xfer_count;
}


uint32_t USBD_GetDeviceAddress(void)
{
    return usb_device_address;
}

void USBD_SetDeviceAddress(uint32_t addr)
{
    usb_device_address = addr;
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
