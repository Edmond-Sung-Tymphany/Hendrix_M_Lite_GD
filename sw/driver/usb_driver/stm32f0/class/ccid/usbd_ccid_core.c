/**
  ******************************************************************************
  * @file    usbd_ccid_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides all the CCID core functions.
  *
  * @verbatim
  *      
  *          ===================================================================      
  *                                CCID Class  Description
  *          =================================================================== 
  *           This module manages the Specification for Integrated Circuit(s) 
  *             Cards Interface Revision 1.1
  *           This driver implements the following aspects of the specification:
  *             - Bulk Transfers 
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
#include "usbd_ccid_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/ 
static uint8_t USBD_CCID_Init(void *pdev, uint8_t cfgidx);
static uint8_t USBD_CCID_DeInit(void *pdev, uint8_t cfgidx);
static uint8_t USBD_CCID_Setup(void *pdev, USB_SETUP_REQ *req);
static uint8_t USBD_CCID_DataIn(void *pdev, uint8_t epnum);
static uint8_t USBD_CCID_DataOut(void *pdev, uint8_t epnum);
static uint8_t *USBD_CCID_GetCfgDesc(uint8_t speed, uint16_t *length);

static uint8_t USBD_CCID_EP0_Buff[CCID_EP0_BUFF_SIZ] ;

USBD_Class_cb_TypeDef  USBD_CCID_cb = 
{
    USBD_CCID_Init,
    USBD_CCID_DeInit,
    USBD_CCID_Setup,
    NULL, /*EP0_TxSent*/  
    NULL, /*EP0_RxReady*/
    USBD_CCID_DataIn,
    USBD_CCID_DataOut,
    NULL, /*SOF */      
    USBD_CCID_GetCfgDesc,
};

/* Private function ----------------------------------------------------------*/ 

/**
  * @brief  USBD_CCID_Init
  *         Initialize  the USB CCID Interface 
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
static uint8_t USBD_CCID_Init(void *pdev, uint8_t cfgidx)
{
    DCD_PMA_Config(pdev, CCID_BULK_IN_EP,  USB_SNG_BUF, CCID_BULK_TX_ADDRESS);
    DCD_PMA_Config(pdev, CCID_INTR_IN_EP,  USB_SNG_BUF, CCID_INT_TX_ADDRESS);
    DCD_PMA_Config(pdev, CCID_BULK_OUT_EP, USB_SNG_BUF, CCID_BULK_RX_ADDRESS);

    /* Open EP IN */
    DCD_EP_Open(pdev, CCID_BULK_IN_EP, CCID_BULK_EPIN_SIZE, USB_EP_BULK);

    /* Open EP OUT */
    DCD_EP_Open(pdev, CCID_BULK_OUT_EP, CCID_BULK_EPOUT_SIZE, USB_EP_BULK);

    /* Open INTR EP IN */
    DCD_EP_Open(pdev, CCID_INTR_IN_EP, CCID_INTR_EPIN_SIZE, USB_EP_INT);

    /* Init the CCID  layer */
    CCID_Init(pdev); 

    return USBD_OK;
}

/**
  * @brief  USBD_CCID_DeInit
  *         DeInitilaize the usb ccid configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */
static uint8_t USBD_CCID_DeInit(void  *pdev, uint8_t cfgidx)
{
    /* Close CCID EPs */
    DCD_EP_Close (pdev, CCID_BULK_IN_EP);
    DCD_EP_Close (pdev, CCID_BULK_OUT_EP);
    DCD_EP_Close (pdev, CCID_INTR_IN_EP);

    /* Un Init the CCID layer */
    CCID_DeInit(pdev);   
    return USBD_OK;
}

/**
  * @brief  USBD_CCID_Setup
  *         Handle the CCID specific requests
  * @param  pdev: device instance
  * @param  req: USB request
  * @retval status
  */
static uint8_t USBD_CCID_Setup(void *pdev, USB_SETUP_REQ *req)
{
    uint8_t slot_nb;
    uint8_t seq_nb;
    uint16_t len;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
    /* Class request */
    case USB_REQ_TYPE_CLASS :
        switch (req->bRequest)
        {
        case REQUEST_ABORT :

            if ((req->wLength == 0) &&
            ((req->bmRequest & 0x80) != 0x80))
            { /* Check bmRequest : No Data-In stage. 0x80 is Data Direction */

                /* The wValue field contains the slot number (bSlot) in the low byte 
                and the sequence number (bSeq) in the high byte.*/
                slot_nb = (uint8_t) ((req->wValue) & 0x00ff);
                seq_nb = (uint8_t) (((req->wValue) & 0xff00)>>8);

                if ( CCID_CmdAbort(slot_nb, seq_nb) != 0 )
                { /* If error is returned by lower layer : 
                    Generally Slot# may not have matched */
                    USBD_CtlError(pdev , req);
                    return USBD_FAIL; 
                }
            }
            else
            {
                USBD_CtlError(pdev , req);
                return USBD_FAIL; 
            }
            break;

        case REQUEST_GET_CLOCK_FREQUENCIES :
            if((req->wValue  == 0) && 
            (req->wLength != 0) &&
            ((req->bmRequest & 0x80) == 0x80))
            {   /* Check bmRequest : Data-In stage. 0x80 is Data Direction */

                if ( SC_Request_GetClockFrequencies(USBD_CCID_EP0_Buff, &len) != 0 )
                { /* If error is returned by lower layer */
                    USBD_CtlError(pdev , req);
                    return USBD_FAIL; 
                }
                else
                {
                    if (len > CCID_EP0_BUFF_SIZ)
                    {
                        len = CCID_EP0_BUFF_SIZ;
                    }

                    USBD_CtlSendData (pdev, USBD_CCID_EP0_Buff, len);
                }
            }
            else
            {
                USBD_CtlError(pdev , req);
                return USBD_FAIL; 
            }
            break;

        case REQUEST_GET_DATA_RATES :
            if((req->wValue  == 0) && 
            (req->wLength != 0) &&
            ((req->bmRequest & 0x80) == 0x80))
            {  /* Check bmRequest : Data-In stage. 0x80 is Data Direction */

                if ( SC_Request_GetDataRates(USBD_CCID_EP0_Buff, &len) != 0 )
                { /* If error is returned by lower layer */
                    USBD_CtlError(pdev , req);
                    return USBD_FAIL; 
                }
                else
                {
                    if (len > CCID_EP0_BUFF_SIZ)
                    {
                        len = CCID_EP0_BUFF_SIZ;
                    }

                    USBD_CtlSendData (pdev, USBD_CCID_EP0_Buff, len);
                } 
            }
            else
            {
                USBD_CtlError(pdev , req);
                return USBD_FAIL; 
            }
            break;

        default:
            USBD_CtlError(pdev , req);
            return USBD_FAIL; 
        }
        break;

    /* Interface & Endpoint request */
    case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {
        case USB_REQ_GET_INTERFACE :
        break;

        case USB_REQ_SET_INTERFACE :
        break;

        case USB_REQ_CLEAR_FEATURE:  
            /* Re-activate the EP */      
            DCD_EP_Close (pdev , (uint8_t)req->wIndex);
            if((((uint8_t)req->wIndex) & 0x80) == 0x80)
            {
                DCD_EP_Open(pdev, ((uint8_t)req->wIndex), CCID_BULK_EPIN_SIZE, USB_EP_BULK);
            }
            else
            {
                DCD_EP_Open(pdev, ((uint8_t)req->wIndex), CCID_BULK_EPOUT_SIZE, USB_EP_BULK);
            }

            break;
    }  
    break;

    default:
        break;
    }

    return USBD_OK;
}

/**
  * @brief  USBD_CCID_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CCID_DataIn(void *pdev, uint8_t epnum)
{
    CCID_BulkMessage_In(pdev , epnum);
    return USBD_OK;
}

/**
  * @brief  USBD_CCID_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_CCID_DataOut (void *pdev, uint8_t epnum)
{
    CCID_BulkMessage_Out(pdev , epnum);
    return USBD_OK;
}

/**
  * @brief  USBD_CCID_GetCfgDesc 
  *         return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CCID_GetCfgDesc (uint8_t speed, uint16_t *length)
{
    *length = sizeof (USBD_CCID_CfgDesc);
    return (uint8_t*)USBD_CCID_CfgDesc;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
