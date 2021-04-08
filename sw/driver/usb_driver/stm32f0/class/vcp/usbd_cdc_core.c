/**
  ******************************************************************************
  * @file    usbd_cdc_core.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB CDC Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as CDC Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class

  *           @note
  *             For the Abstract Control Model, this core allows only transmitting the requests to
  *             lower layer dispatcher (ie. usbd_cdc_vcp.c/.h) which should manage each request and
  *             perform relative actions.
  *
  *           These aspects may be enriched or modified for a specific user application.
  *          
  *            This driver doesn't implement the following aspects of the specification
  *            (but it is possible to manage these features with some modifications on this driver):
  *             - Any class-specific aspect relative to communication classes should be managed by user application.
  *             - All communication classes other than PSTN are not managed
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
#include "commonTypes.h"

#include "usbd_cdc_core.h"
#include "usbd_req.h"
#include "usbd_def.h"
#include "ringbuf.h"
#include "UsbDrv.h"
#include "usbd_dcd.config"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance.
 * these parameters shall be same as Usb_Descriptors.Config
 */

#define CDC_DATA_IN_PACKET_SIZE                DATA_MAX_PACKET_SIZE
#define CDC_DATA_OUT_PACKET_SIZE               DATA_MAX_PACKET_SIZE

/* Private variables ---------------------------------------------------------*/
static DCD_DEV *pDCD;
static uint8_t rxBuf[CDC_DATA_OUT_PACKET_SIZE];
static uint8_t txBuf[CDC_DATA_IN_PACKET_SIZE];

static __IO uint32_t  usbd_cdc_AltSet  = 0;
static uint8_t setupBuf[8];

static lineCoding_type lineCode =
{
  115200, /* baud rate*/
  0x00,   /* stop bits-1*/
  0x00,   /* parity - none*/
  0x08    /* nb. of bits 8*/
};

/*********************************************
   CDC specific management functions
 *********************************************/

/* CDC interface class callbacks structure */
USBD_Class_cb_TypeDef USBD_CDC_cb = 
{
    .Init       = usbd_cdc_Init,
    .DeInit     = usbd_cdc_DeInit,
    .Setup      = usbd_cdc_Setup,
    .EP0_TxSent = NULL,
    .EP0_RxReady= usbd_cdc_EP0_RxReady,
    .DataIn     = usbd_cdc_DataIn,
    .DataOut    = usbd_cdc_DataOut,
    .SOF        = usbd_cdc_SOF,
    .Suspend    = usbd_cdc_Suspend,
    .DmaOver    = NULL,
    .ErrorsHandler = NULL,
};

/* Private function prototypes -----------------------------------------------*/
static void usbd_cdc_BlockingWrite(void);
static void usbd_cdc_Setup_Request(uint8_t request, uint8_t *buff);
/* Private function ----------------------------------------------------------*/
/**
  * @brief  usbd_cdc_Init
  *         Initialize the CDC interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
uint8_t usbd_cdc_Init(void *pdev, uint8_t cfgidx)
{
    pDCD = (DCD_DEV *)pdev;

    /* Configure address */
    DCD_PMA_Config(CDC_IN_EP,   USB_SNG_BUF,    BULK_IN_TX_ADDRESS);
    DCD_PMA_Config(CDC_CMD_EP,  USB_SNG_BUF,    INT_IN_TX_ADDRESS);
    DCD_PMA_Config(CDC_OUT_EP,  USB_SNG_BUF,    BULK_OUT_RX_ADDRESS);

    /* Open EP IN */
    DCD_EP_Open(CDC_IN_EP,  CDC_DATA_IN_PACKET_SIZE, USB_EP_BULK);

    /* Open EP OUT */
    DCD_EP_Open(CDC_OUT_EP, CDC_DATA_OUT_PACKET_SIZE, USB_EP_BULK);

    /* Open Command IN EP */
    DCD_EP_Open(CDC_CMD_EP, CMD_PACKET_SZE, USB_EP_INT);

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(CDC_OUT_EP, rxBuf, CDC_DATA_OUT_PACKET_SIZE);

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_Init
  *         DeInitialize the CDC layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
uint8_t usbd_cdc_DeInit(void *pdev, uint8_t cfgidx)
{
    /* Open EP IN */
    DCD_EP_Close(CDC_IN_EP);

    /* Open EP OUT */
    DCD_EP_Close(CDC_OUT_EP);

    /* Open Command IN EP */
    DCD_EP_Close(CDC_CMD_EP);

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_Setup
  *         Handle the CDC specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
uint8_t usbd_cdc_Setup(void *pdev, USB_SETUP_REQ *req)
{
    uint16_t len = USB_CDC_DESC_SIZ;
    uint8_t  *pbuf= (uint8_t*)USBD_GetConfigDescriptor() + 9;

    switch (req->bmRequest & USB_REQ_TYPE_MASK)
    {
        /* CDC Class Requests -------------------------------*/
        case USB_REQ_TYPE_CLASS:
        {
            if (req->wLength) /* Check if the request is a data setup packet */
            {
                /* Check if the request is Device-to-Host */
                if (req->bmRequest & 0x80)
                {
                    /* Get the data to be sent to Host from interface layer */
                    setupBuf[0] = (uint8_t)(lineCode.bitrate);
                    setupBuf[1] = (uint8_t)(lineCode.bitrate >> 8);
                    setupBuf[2] = (uint8_t)(lineCode.bitrate >> 16);
                    setupBuf[3] = (uint8_t)(lineCode.bitrate >> 24);
                    setupBuf[4] = (uint8_t)lineCode.format;
                    setupBuf[5] = (uint8_t)lineCode.paritytype;
                    setupBuf[6] = (uint8_t)lineCode.datatype;
                    usbd_cdc_Setup_Request(req->bRequest, setupBuf);

                    /* Send the data to the host */
                    USBD_CtlSendData(pdev, setupBuf, req->wLength);
                }
                else /* Host-to-Device requeset */
                {
                    /* Set the value of the current command to be processed */

                    /* Prepare the reception of the buffer over EP0
                    Next step: the received data will be managed in usbd_cdc_EP0_TxSent()
                    function. */
                    USBD_CtlPrepareRx(pdev, setupBuf, req->wLength);
                }
            }
            else /* No Data request */
            {
                /* Transfer the command to the interface layer */
            }
            
            return USBD_OK;
        }
        default:
        {
            USBD_CtlError(pdev, req);
            return USBD_FAIL;
        }

        /*-------- Standard Requests -------------------------------*/
        case USB_REQ_TYPE_STANDARD:
        {
            switch (req->bRequest)
            {
                case USB_REQ_GET_DESCRIPTOR:
                {
                    if( (req->wValue >> 8) == CDC_DESCRIPTOR_TYPE)
                    {
                        pbuf = (uint8_t*)USBD_GetConfigDescriptor() + 9 + (9 * USBD_ITF_MAX_NUM);
                        len = MIN(USB_CDC_DESC_SIZ , req->wLength);
                    }

                    USBD_CtlSendData(pdev, pbuf, len);
                    break;
                }
                case USB_REQ_GET_INTERFACE :
                {
                    USBD_CtlSendData (pdev, (uint8_t *)&usbd_cdc_AltSet, 1);
                    break;
                }
                case USB_REQ_SET_INTERFACE :
                {
                    if ((uint8_t)(req->wValue) < USBD_ITF_MAX_NUM)
                    {
                        usbd_cdc_AltSet = (uint8_t)(req->wValue);
                    }
                    else
                    {
                        /* Call the error management function (command will be nacked */
                        USBD_CtlError(pdev, req);
                    }
                    break;
                }
           }
        }
    }

    return USBD_OK;
}

/**
  * @brief  usbd_cdc_EP0_RxReady
  *         Data received on control endpoint
  * @param  pdev: device device instance
  * @retval status
  */
uint8_t  usbd_cdc_EP0_RxReady(void *pdev)
{
//    if (cdcCmd != NO_CMD)
//    {
//        /* Process the data */
//        APP_FOPS.pIf_Ctrl(cdcCmd, CmdBuff, cdcLen);

//        /* Reset the command variable to default value */
//        cdcCmd = NO_CMD;
//    }

    return USBD_OK;
}

/**
  * @brief  usbd_audio_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  usbd_cdc_DataIn(void *pdev, uint8_t epnum)
{
    usbd_cdc_BlockingWrite();
    return USBD_OK;
}

/**
  * @brief  usbd_cdc_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  usbd_cdc_DataOut(void *pdev, uint8_t epnum)
{
    uint16_t USB_Rx_Cnt;

    /* Get the received data buffer and update the counter */
    USB_Rx_Cnt = pDCD->out_ep[epnum].xfer_count;

    /* USB data will be immediately processed, this allow next USB traffic being
     NAKed till the end of the application Xfer */
    //RingBuf_Push(pDCD->rxBuffer, rxBuf, USB_Rx_Cnt);
    UsbDrv_RecvData(rxBuf, USB_Rx_Cnt);

    /* Prepare Out endpoint to receive next packet */
    DCD_EP_PrepareRx(CDC_OUT_EP, rxBuf, CDC_DATA_OUT_PACKET_SIZE);

    return USBD_OK;
}

/**
  * @brief  usbd_CDC_SOF
  *         Start Of Frame event management
  * @param  pdev: instance
  * @param  epnum: endpoint number
  * @retval status
  */
uint8_t  usbd_cdc_SOF(void *pdev)
{
    static uint32_t FrameCount = 0;

    if (FrameCount++ == IN_FRAME_INTERVAL)
    {
        /* Reset the frame counter */
        FrameCount = 0;

        /* Check the data to be sent through IN pipe */
        usbd_cdc_BlockingWrite();
    }

    return USBD_OK;
}

uint8_t usbd_cdc_Suspend(void *pdev)
{
    /* notify the server USB is disable */

    return USBD_OK;
}

/*
* we have to judge the last byte flag if the size is equal the CDC_DATA_IN_PACKET_SIZE 
*/

static void usbd_cdc_BlockingWrite(void)
{
    static bool lastPacketFlag = FALSE;
    uint32_t usedSize;
    uint32_t length;

    /* Get the size of used buffer */
    usedSize = RingBuf_GetUsedSize(pDCD->txBuffer);

    /* Check if the previous package is the last package */
    if(0 == usedSize && lastPacketFlag)
    {
        /* clean the flag and flush the buffer */
        lastPacketFlag = FALSE;
        DCD_EP_Tx(CDC_IN_EP, NULL, 0);
    }
    else
    {
        if(CDC_DATA_IN_PACKET_SIZE == usedSize)
        {
            /* this is the last package */
            lastPacketFlag = TRUE;
        }

        length = MIN(usedSize, CDC_DATA_IN_PACKET_SIZE);
        if(length)
        {
            RingBuf_Pop(pDCD->txBuffer, txBuf, length);
            DCD_EP_Tx(CDC_IN_EP, txBuf, length);
        }
    }
}

static void usbd_cdc_Setup_Request(uint8_t request, uint8_t *buff)
{
    switch(request)
    {
        case SEND_ENCAPSULATED_COMMAND:
            /* Not  needed for this driver */
            break;

        case GET_ENCAPSULATED_RESPONSE:
            /* Not  needed for this driver */
            break;

        case SET_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case GET_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case CLEAR_COMM_FEATURE:
            /* Not  needed for this driver */
            break;

        case SET_LINE_CODING:
            lineCode.bitrate = (uint32_t)(buff[0] | (buff[1] << 8) | (buff[2] << 16) | (buff[3] << 24));
            lineCode.format = buff[4];
            lineCode.paritytype = buff[5];
            lineCode.datatype = buff[6];
            /* Set the new configuration */
            break;

        case GET_LINE_CODING:
            buff[0] = (uint8_t)(lineCode.bitrate);
            buff[1] = (uint8_t)(lineCode.bitrate >> 8);
            buff[2] = (uint8_t)(lineCode.bitrate >> 16);
            buff[3] = (uint8_t)(lineCode.bitrate >> 24);
            buff[4] = lineCode.format;
            buff[5] = lineCode.paritytype;
            buff[6] = lineCode.datatype;
            break;

        case SET_CONTROL_LINE_STATE:
            /* Not  needed for this driver */
            break;

        case SEND_BREAK:
            /* Not  needed for this driver */
            break;

        default:
            break;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
