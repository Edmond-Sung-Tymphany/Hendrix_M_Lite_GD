/**
  ******************************************************************************
  * @file    usbd_hid_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides the HID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                HID Class  Description
  *          =================================================================== 
  *           This module manages the HID class V1.11 following the "Device Class Definition
  *           for Human Interface Devices (HID) Version 1.11 Jun 27, 2001".
  *           This driver implements the following aspects of the specification:
  *             - The Boot Interface Subclass
  *             - The Mouse protocol
  *             - Usage Page : Generic Desktop
  *             - Usage : Joystick
  *             - Collection : Application 
  *           
  *      
  *  @endverbatim
  *
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
#include "usbd_hid_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/ 
static uint8_t USBD_HID_Init(void  *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_DeInit(void  *pdev, uint8_t cfgidx);
static uint8_t USBD_HID_Setup(void  *pdev, USB_SETUP_REQ *req);
static uint8_t *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length);

USBD_Class_cb_TypeDef  USBD_HID_cb = 
{
    USBD_HID_Init,
    USBD_HID_DeInit,
    USBD_HID_Setup,
    NULL, /*EP0_TxSent*/  
    NULL, /*EP0_RxReady*/
    NULL, /*DataIn*/
    NULL, /*DataOut*/
    NULL, /*SOF */    
    USBD_HID_GetCfgDesc, 
};
      
static uint32_t  USBD_HID_AltSet = 0;

static uint32_t  USBD_HID_Protocol = 0;

static uint32_t  USBD_HID_IdleState  = 0;

/* Private function ----------------------------------------------------------*/ 
/**
  * @brief  USBD_HID_Init
  *         Initialize the HID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_Init(void *pdev, uint8_t cfgidx)
{
    DCD_PMA_Config(pdev , HID_IN_EP,USB_SNG_BUF,HID_IN_TX_ADDRESS);

    /* Open EP IN */
    DCD_EP_Open(pdev, HID_IN_EP, HID_IN_PACKET, USB_EP_INT);

    /* Open EP OUT */
    DCD_EP_Open(pdev, HID_OUT_EP, HID_OUT_PACKET, USB_EP_INT);

    return USBD_OK;
}

/**
  * @brief  USBD_HID_Init
  *         DeInitialize the HID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_HID_DeInit(void *pdev,  uint8_t cfgidx)
{
    /* Close HID EPs */
    DCD_EP_Close (pdev , HID_IN_EP);
    DCD_EP_Close (pdev , HID_OUT_EP);

    return USBD_OK;
}

/**
  * @brief  USBD_HID_Setup
  *         Handle the HID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_HID_Setup(void *pdev, USB_SETUP_REQ *req)
{
    uint16_t len = 0;
    uint8_t  *pbuf = NULL;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    case USB_REQ_TYPE_CLASS :  
        switch (req->bRequest)
        {
            case HID_REQ_SET_PROTOCOL:
            USBD_HID_Protocol = (uint8_t)(req->wValue);
            break;

            case HID_REQ_GET_PROTOCOL:
            USBD_CtlSendData(pdev, (uint8_t *)&USBD_HID_Protocol, 1);    
            break;

            case HID_REQ_SET_IDLE:
            USBD_HID_IdleState = (uint8_t)(req->wValue >> 8);
            break;

            case HID_REQ_GET_IDLE:
            USBD_CtlSendData(pdev, (uint8_t *)&USBD_HID_IdleState, 1);        
            break;      

            default:
            USBD_CtlError (pdev, req);
            return USBD_FAIL; 
        }
        break;

    case USB_REQ_TYPE_STANDARD:
        switch (req->bRequest)
        {
            case USB_REQ_GET_DESCRIPTOR: 
            if( req->wValue >> 8 == HID_REPORT_DESC)
            {
                len = MIN(HID_MOUSE_REPORT_DESC_SIZE , req->wLength);
                pbuf = (uint8_t *)HID_MOUSE_ReportDesc;
            }
            else if( req->wValue >> 8 == HID_DESCRIPTOR_TYPE)
            {
                pbuf = (uint8_t *)USBD_HID_CfgDesc + 0x12;
                len = MIN(USB_HID_DESC_SIZ , req->wLength);
            }

            USBD_CtlSendData(pdev, pbuf, len);
            break;

            case USB_REQ_GET_INTERFACE :
            USBD_CtlSendData(pdev, (uint8_t *)&USBD_HID_AltSet, 1);
            break;

            case USB_REQ_SET_INTERFACE :
            USBD_HID_AltSet = (uint8_t)(req->wValue);
            break;
        }
    }
  return USBD_OK;
}

/**
  * @brief  USBD_HID_SendReport 
  *         Send HID Report
  * @param  pdev: device instance
  * @param  buff: pointer to report
  * @retval status
  */
uint8_t USBD_HID_SendReport (USB_CORE_HANDLE *pdev,  uint8_t *report, uint16_t len)
{
    /* Check if USB is configured */
    if (pdev->dev.device_status == USB_CONFIGURED )
    {
        DCD_EP_Tx (pdev, HID_IN_EP, report, len);
    }

    return USBD_OK;
}

/**
  * @brief  USBD_HID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_HID_GetCfgDesc(uint8_t speed, uint16_t *length)
{
    *length = sizeof (USBD_HID_CfgDesc);
    return (uint8_t *)USBD_HID_CfgDesc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
