/**
  ******************************************************************************
  * @file    usb_dcd.c
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    31-January-2014
  * @brief   Device interface layer used by the library to access the core.
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

#include "usb_dcd.h"
#include "usbd_def.h"
#include "usbd_pwr.h"
#include "usbd_req.h"
#include "usbd_dcd.config"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static DCD_DEV  usbDCD;
static DCD_DEV  *pUsbDCD = NULL;

static USB_EP   USB_IN_EP_SET[EP_NUM];
static USB_EP   USB_OUT_EP_SET[EP_NUM];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief Device Initialization
  * @param  pdev: device instance
  * @retval : None
  */
void DCD_Init(USBD_Class_cb_TypeDef *class_cb, cRingBuf *txBuffer)
{
    pUsbDCD = &usbDCD;

    /*Device is in Default State*/
    pUsbDCD->device_status = USB_STA_DEFAULT;
    pUsbDCD->device_address = 0;
    pUsbDCD->DevRemoteWakeup = 0;
    pUsbDCD->speed = USB_SPEED_FULL; /*kept for API compatibility reason*/

    /*Register class and user callbacks */
    pUsbDCD->class_cb = class_cb;

    /*point buffer*/
    pUsbDCD->txBuffer = txBuffer;

    /*register endpoint number*/
    pUsbDCD->in_ep  = USB_IN_EP_SET;
    pUsbDCD->out_ep = USB_OUT_EP_SET;

    /*CNTR_FRES = 1*/
    SetCNTR(CNTR_FRES);

    /*CNTR_FRES = 0*/
    SetCNTR(0);

    /*Clear pending interrupts*/
    SetISTR(0);

    /*Set Btable Address*/
    SetBTABLE(BTABLE_ADDRESS);

    /* Enable the pull-up*/
    DCD_DevConnect();

    /*Set DCD interrupt mask*/
    DCD_SetIMR();
}

void DCD_DeInit(void)
{
    /* disable the pull-up */
    DCD_DevDisconnect();
    DCD_StopDevice();
}

/**
  * @brief Stop device
  * @param  pdev: device instance
  * @retval : None
  */
void DCD_StopDevice(void)
{
    /* disable all interrupts and force USB reset */
    _SetCNTR(CNTR_FRES);
  
    /* clear interrupt status register */
    _SetISTR(0);

    /* switch-off device */
    _SetCNTR(CNTR_FRES + CNTR_PDWN);

    /*Device is in default state*/
    pUsbDCD->device_status  = USB_STA_DEFAULT;
}

/**
  * @brief Configure PMA for EP
  * @param  pdev : Device instance
  * @param  ep_addr: endpoint address
  * @param  ep_Kind: endpoint Kind
  *                @arg USB_SNG_BUF: Single Buffer used
  *                @arg USB_DBL_BUF: Double Buffer used
  * @param  pmaadress: EP address in The PMA: In case of single buffer endpoint
  *                   this parameter is 16-bit value providing the address
  *                   in PMA allocated to endpoint.
  *                   In case of double buffer endpoint this parameter
  *                   is a 32-bit value providing the endpoint buffer 0 address
  *                   in the LSB part of 32-bit value and endpoint buffer 1 address
  *                   in the MSB part of 32-bit value.
  * @retval : status
  */

uint32_t DCD_PMA_Config(uint16_t ep_addr, uint16_t ep_kind, uint32_t pmaadress)
{
    USB_EP *ep;

    /* initialize ep structure*/
    if ((ep_addr & 0x80) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[ep_addr & 0x7F]);
    }
    else
    {
        ep = &(pUsbDCD->out_ep[ep_addr & 0x7F]);
    }

    /* Here we check if the endpoint is single or double Buffer*/
    if (ep_kind == USB_SNG_BUF)
    {
        /*Single Buffer*/
        ep->doublebuffer = 0;
        /*Configure te PMA*/
        ep->pmaadress = (uint16_t)pmaadress;
    }
    else /*USB_DBL_BUF*/
    {
        /*Double Buffer Endpoint*/
        ep->doublebuffer = 1;
        /*Configure the PMA*/
        ep->pmaaddr0 =  pmaadress & 0xFFFF;
        ep->pmaaddr1 =  (pmaadress & 0xFFFF0000) >> 16;
    }

    return USB_OK; 
}

/**
  * @brief Configure an EP
  * @param  pdev : Device instance
  * @param  ep_addr: endpoint address
  * @param  ep_mps: endpoint max packet size
  * @param  ep_type: endpoint Type
  */
uint32_t DCD_EP_Open(uint16_t ep_addr, uint16_t ep_mps, uint8_t ep_type)
{
    USB_EP *ep;

    /* initialize ep structure*/
    if ((ep_addr & 0x80) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[ep_addr & 0x7F]);
        ep->is_in = 1;
    }
    else
    {
        ep = &(pUsbDCD->out_ep[ep_addr & 0x7F]);
        ep->is_in = 0;
    }

    ep->maxpacket = ep_mps;
    ep->type = ep_type;
    ep->num   = ep_addr & 0x7F;

    if (ep->num == 0)
    {
        /* Initialize the control transfer variables*/ 
        ep->ctl_data_len = 0;
        ep->rem_data_len = 0;
        ep->total_data_len = 0;
    }

    /* Initialize the transaction level variables */
    ep->xfer_buff = 0;
    ep->xfer_len = 0;
    ep->xfer_count = 0;
    ep->is_stall = 0;

    /* initialize HW */
    switch (ep->type)
    {
    case USB_EP_CONTROL:
        SetEPType(ep->num, EP_CONTROL);
        break;
    case USB_EP_BULK:
        SetEPType(ep->num, EP_BULK);
        break;
    case USB_EP_INT:
        SetEPType(ep->num, EP_INTERRUPT);
        break;
    case USB_EP_ISOC:
        SetEPType(ep->num, EP_ISOCHRONOUS);
        break;
    } 

    if (ep->doublebuffer == 0) 
    {
        if (ep->is_in)
        {
            /*Set the endpoint Transmit buffer address */
            SetEPTxAddr(ep->num, ep->pmaadress);
            ClearDTOG_TX(ep->num);
            /* Configure NAK status for the Endpoint*/
            SetEPTxStatus(ep->num, EP_TX_NAK); 
        }
        else
        {
            /*Set the endpoint Receive buffer address */
            SetEPRxAddr(ep->num, ep->pmaadress);
            /*Set the endpoint Receive buffer counter*/
            SetEPRxCount(ep->num, ep->maxpacket);
            ClearDTOG_RX(ep->num);
            /* Configure VALID status for the Endpoint*/
            SetEPRxStatus(ep->num, EP_RX_VALID);
        }
    }
    /*Double Buffer*/
    else
    {
        /*Set the endpoint as double buffered*/
        SetEPDoubleBuff(ep->num);
        /*Set buffer address for double buffered mode*/
        SetEPDblBuffAddr(ep->num,ep->pmaaddr0, ep->pmaaddr1);

        if (ep->is_in==0)
        {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            ClearDTOG_RX(ep->num);
            ClearDTOG_TX(ep->num);

            /* Reset value of the data toggle bits for the endpoint out*/
            ToggleDTOG_TX(ep->num);

            SetEPRxStatus(ep->num, EP_RX_VALID);
            SetEPTxStatus(ep->num, EP_TX_DIS);
        }
        else
        {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            ClearDTOG_RX(ep->num);
            ClearDTOG_TX(ep->num);
            ToggleDTOG_RX(ep->num);
            /* Configure DISABLE status for the Endpoint*/
            SetEPTxStatus(ep->num, EP_TX_DIS);
            SetEPRxStatus(ep->num, EP_RX_DIS);
        }
    } 

    return USB_OK; 
}
/**
  * @brief called when an EP is disabled
  * @param  pdev: device instance
  * @param  ep_addr: endpoint address
  * @retval : status
  */
uint32_t DCD_EP_Close(uint8_t  ep_addr)
{
    USB_EP *ep;

    if ((ep_addr & 0x80) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[ep_addr & 0x7F]);
    }
    else
    {
        ep = &(pUsbDCD->out_ep[ep_addr & 0x7F]);
    }

    if (ep->doublebuffer == 0) 
    {
        if (ep->is_in)
        {
            ClearDTOG_TX(ep->num);
            /* Configure DISABLE status for the Endpoint*/
            SetEPTxStatus(ep->num, EP_TX_DIS); 
        }
        else
        {
            ClearDTOG_RX(ep->num);
            /* Configure DISABLE status for the Endpoint*/
            SetEPRxStatus(ep->num, EP_RX_DIS);
        }
    }
    /*Double Buffer*/
    else
    { 
        if (ep->is_in==0)
        {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            ClearDTOG_RX(ep->num);
            ClearDTOG_TX(ep->num);

            /* Reset value of the data toggle bits for the endpoint out*/
            ToggleDTOG_TX(ep->num);

            SetEPRxStatus(ep->num, EP_RX_DIS);
            SetEPTxStatus(ep->num, EP_TX_DIS);
        }
        else
        {
            /* Clear the data toggle bits for the endpoint IN/OUT*/
            ClearDTOG_RX(ep->num);
            ClearDTOG_TX(ep->num);
            ToggleDTOG_RX(ep->num);
            /* Configure DISABLE status for the Endpoint*/
            SetEPTxStatus(ep->num, EP_TX_DIS);
            SetEPRxStatus(ep->num, EP_RX_DIS);
        }
    } 

    return USB_OK;
}


/**
  * @brief DCD_EP_PrepareRx
  * @param  pdev: device instance
  * @param  ep_addr: endpoint address
  * @param  pbuf: pointer to Rx buffer
  * @param  buf_len: data length
  * @retval : status
  */
uint32_t DCD_EP_PrepareRx(uint8_t ep_addr, uint8_t *pbuf, uint16_t buf_len)
{
    __IO uint32_t len = 0; 
    USB_EP *ep;

    ep = &(pUsbDCD->out_ep[ep_addr & 0x7F]);

    /*setup and start the Xfer */
    ep->xfer_buff = pbuf;  
    ep->xfer_len = buf_len;
    ep->xfer_count = 0; 

    /*Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket)
    {
        len=ep->maxpacket;
        ep->xfer_len-=len; 
    }
    else
    {
        len=ep->xfer_len;
        ep->xfer_len =0;
    }

    /* configure and validate Rx endpoint */
    if (ep->doublebuffer == 0) 
    {
        /*Set RX buffer count*/
        SetEPRxCount(ep->num, len);
    }
    else
    {
        /*Set the Double buffer counter*/
        SetEPDblBuffCount(ep->num, ep->is_in, len);
    } 

    SetEPRxStatus(ep->num, EP_RX_VALID);

    return USB_OK;
}

/**
  * @brief Transmit data Buffer
  * @param  pdev: device instance
  * @param  ep_addr: endpoint address
  * @param  pbuf: pointer to Tx buffer
  * @param  buf_len: data length
  * @retval : status
  */
uint32_t DCD_EP_Tx(uint8_t ep_addr, uint8_t *pbuf, uint32_t buf_len)
{
    __IO uint32_t len = 0; 
    USB_EP *ep;

    ep = &(pUsbDCD->in_ep[ep_addr & 0x7F]);

    /*setup and start the Xfer */
    ep->num = ep_addr & 0x7F; 
    ep->xfer_buff = pbuf;  
    ep->xfer_len = buf_len;
    ep->xfer_count = 0; 

    /*Multi packet transfer*/
    if (ep->xfer_len > ep->maxpacket)
    {
        len=ep->maxpacket;
        ep->xfer_len-=len; 
    }
    else
    {
        len=ep->xfer_len;
        ep->xfer_len =0;
    }

    /* configure and validate Tx endpoint */
    if (ep->doublebuffer == 0) 
    {
        /* copy the user data to the PMA buffer */
        UserToPMABufferCopy(ep->xfer_buff, ep->pmaadress, len);
        SetEPTxCount(ep->num, len);
    }
    else
    {
        uint16_t pmabuffer=0;
        /*Set the Double buffer counter*/
        SetEPDblBuffCount(ep->num, ep->is_in, len);

        /*Write the data to the USB endpoint*/
        if (GetENDPOINT(ep->num)&EP_DTOG_TX)
        {
            pmabuffer = ep->pmaaddr1;
        }
        else
        {
            pmabuffer = ep->pmaaddr0;
        }

        UserToPMABufferCopy(ep->xfer_buff, pmabuffer, len);
        FreeUserBuffer(ep->num, ep->is_in);
    }

    /* I guess this is starting trigger to transmit data to PC Host */
    SetEPTxStatus(ep->num, EP_TX_VALID);

    return USB_OK; 
}


/**
  * @brief Stall an endpoint.
  * @param  pdev: device instance
  * @param  epnum: endpoint address
  * @retval : status
  */
uint32_t DCD_EP_Stall(uint8_t epnum)
{
    USB_EP *ep;
    if ((0x80 & epnum) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[epnum & 0x7F]);    
    }
    else
    {
        ep = &(pUsbDCD->out_ep[epnum]);
    }

    if (ep->num ==0)
    {
        /* This macro sets STALL status for RX & TX*/ 
        _SetEPRxTxStatus(ep->num, EP_RX_STALL,EP_TX_STALL); 
        /*Endpoint is stalled */
        ep->is_stall = 1;
        return USB_OK;
    }
    if (ep->is_in)
    {  
        /* IN endpoint */
        ep->is_stall = 1;
        /* IN Endpoint stalled */
        SetEPTxStatus(ep->num , EP_TX_STALL); 
    }
    else
    { 
        ep->is_stall = 1;
        /* OUT Endpoint stalled */
        SetEPRxStatus(ep->num , EP_RX_STALL);
    }

    return USB_OK;
}


/**
  * @brief Clear stall condition on endpoints.
  * @param  pdev: device instance
  * @param  epnum: endpoint address
  * @retval : status
  */
uint32_t DCD_EP_ClrStall(uint8_t epnum)
{
    USB_EP *ep;

    if ((0x80 & epnum) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[epnum & 0x7F]);    
    }
    else
    {
        ep = &(pUsbDCD->out_ep[epnum]);
    } 

    if (ep->is_in)
    {
        ClearDTOG_TX(ep->num);
        SetEPTxStatus(ep->num, EP_TX_VALID);
        ep->is_stall = 0;  
    }
    else
    {
        ClearDTOG_RX(ep->num);
        SetEPRxStatus(ep->num, EP_RX_VALID);
        ep->is_stall = 0;  
    }

    return USB_OK;
}

/**
  * @brief This Function set USB device address
  * @param  pdev: device instance
  * @param  address: new device address
  */
void DCD_EP_SetAddress(uint8_t address)
{
    uint32_t i=0;
    pUsbDCD->device_address = address;

    /* set address in every used endpoint */
    for (i = 0; i < EP_NUM; i++)
    {
        _SetEPAddress((uint8_t)i, (uint8_t)i);
    }

    /* set device address and enable function */
    _SetDADDR(address | DADDR_EF);
}

/**
  * @brief Connect device (enable internal pull-up)
  * @param  pdev: device instance
  * @retval : None
  */
void  DCD_DevConnect(void)
{
    /* Enabling DP Pull-Down bit to Connect internal pull-up on USB DP line */
    *BCDR|=BCDR_DPPU;

    /*Device is in default state*/
    pUsbDCD->device_status  = USB_STA_DEFAULT;
}

/**
  * @brief Disconnect device (disable internal pull-up)
  * @param  pdev: device instance
  * @retval : None
  */
void DCD_DevDisconnect(void)
{
    /* Disable DP Pull-Down bit*/
    *BCDR&=~BCDR_DPPU;

    /*Device is in unconnected state*/
    pUsbDCD->device_status = USB_STA_UNCONNECTED;
}

/**
  * @brief returns the EP Status
  * @param   pdev : Selected device
  *         epnum : endpoint address
  * @retval : EP status
  */

uint32_t DCD_GetEPStatus(uint8_t epnum)
{
    uint16_t Status=0; 
    USB_EP *ep;

    if ((0x80 & epnum) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[epnum & 0x7F]);    
    }
    else
    {
        ep = &(pUsbDCD->out_ep[epnum]);
    } 

    if (ep->is_in)
    {
        Status = GetEPTxStatus(ep->num);
    }
    else
    {
        Status = GetEPRxStatus(ep->num);
    }
  
    return Status; 
}

/**
  * @brief Set the EP Status
  * @param   pdev : Selected device
  *         Status : new Status
  *         epnum : EP address
  * @retval : None
  */
void DCD_SetEPStatus(uint8_t epnum , uint32_t Status)
{
    USB_EP *ep;

    if ((0x80 & epnum) == 0x80)
    {
        ep = &(pUsbDCD->in_ep[epnum & 0x7F]);    
    }
    else
    {
        ep = &(pUsbDCD->out_ep[epnum]);
    } 

    if (ep->is_in)
    {
        SetEPTxStatus(ep->num, (uint16_t)Status);
    }
    else
    {
        SetEPRxStatus(ep->num, (uint16_t)Status);
    }

    if ((Status == EP_RX_STALL) || (Status == EP_TX_STALL))
    {
        ep->is_stall =1;
    }
}

void DCD_SetIMR(void)
{
    /*set wInterrupt_Mask global variable*/
    SetCNTR((uint16_t)IMR_MSK);

    #ifdef LPM_ENABLED
    /* Enable LPM support and enable ACK answer to LPM request*/
    _SetLPMCSR(LPMCSR_LMPEN | LPMCSR_LPMACK);
    #endif
}

uint16_t DCD_GetIMR(void)
{
    return (uint16_t)(IMR_MSK);
}
/******************************************************************************
    Bellow are the functions called when interrupt trigger
*******************************************************************************/
/**
  * @brief  Correct Transfer interrupt's service
  * @param  None
  * @retval None
  */
void DCD_Ctr(void)
{
    USB_EP *ep;
    uint16_t count=0;
    uint8_t EPindex;
    __IO uint16_t wIstr;  
    __IO uint16_t wEPVal = 0;

    /* stay in loop while pending interrupts */
    while(((wIstr = _GetISTR()) & ISTR_CTR) != 0)
    {
        /* extract highest priority endpoint number */
        EPindex = (uint8_t)(wIstr & ISTR_EP_ID);

        if (EPindex == 0)
        {
            /* Decode and service control endpoint interrupt */
            /* DIR bit = origin of the interrupt */   
            if ((wIstr & ISTR_DIR) == 0)
            {
                /* DIR = 0      => IN  int */
                /* DIR = 0 implies that (EP_CTR_TX = 1) always  */
                _ClearEP_CTR_TX(ENDP0);
                ep = &(pUsbDCD->in_ep[0]);

                ep->xfer_count = GetEPTxCount(ep->num);
                ep->xfer_buff += ep->xfer_count;

                /* TX COMPLETE */
                DCD_DataInStage(0x00);
            }
            else
            {
                /* DIR = 1 & CTR_RX       => SETUP or OUT int */
                /* DIR = 1 & (CTR_TX | CTR_RX) => 2 int pending */
                ep =  &(pUsbDCD->out_ep[0]);
                wEPVal = _GetENDPOINT(ENDP0);

                if ((wEPVal & EP_SETUP) != 0)
                {
                    /* Get SETUP Packet*/
                    ep->xfer_count = GetEPRxCount(ep->num);
                    PMAToUserBufferCopy(&(pUsbDCD->setup_packet[0]), ep->pmaadress, ep->xfer_count);
                    /* SETUP bit kept frozen while CTR_RX = 1*/ 
                    _ClearEP_CTR_RX(ENDP0); 

                    /* Process SETUP Packet*/
                    DCD_SetupStage();
                }
                else if ((wEPVal & EP_CTR_RX) != 0)
                {
                    _ClearEP_CTR_RX(ENDP0);
                    /* Get Control Data OUT Packet*/
                    ep->xfer_count = GetEPRxCount(ep->num);

                    if (ep->xfer_count != 0)
                    {
                        PMAToUserBufferCopy(ep->xfer_buff, ep->pmaadress, ep->xfer_count);
                        ep->xfer_buff += ep->xfer_count;
                    }

                    /* Process Control Data OUT Packet*/
                    DCD_DataOutStage(0x00);

                    _SetEPRxCount(ENDP0, ep->maxpacket);
                    _SetEPRxStatus(ENDP0,EP_RX_VALID);
                }
            }
        }/* if(EPindex == 0) */
        else
        {
            /* Decode and service non control endpoints interrupt  */
            /* process related endpoint register */
            wEPVal = _GetENDPOINT(EPindex);
            if ((wEPVal & EP_CTR_RX) != 0)
            {
                /* clear int flag */
                _ClearEP_CTR_RX(EPindex);
                ep = &(pUsbDCD->out_ep[EPindex]);

                /* OUT double Buffering*/
                if (ep->doublebuffer == 0)
                {
                    count = GetEPRxCount(ep->num);
                    if (count != 0)
                    {
                        PMAToUserBufferCopy(ep->xfer_buff, ep->pmaadress, count);
                    }
                }
                else
                {
                    if (GetENDPOINT(ep->num) & EP_DTOG_RX)
                    {
                        /*read from endpoint BUF0Addr buffer*/
                        count = GetEPDblBuf0Count(ep->num);
                        if (count != 0)
                        {
                            PMAToUserBufferCopy(ep->xfer_buff, ep->pmaaddr0, count);
                        }
                    }
                    else
                    {
                        /*read from endpoint BUF1Addr buffer*/
                        count = GetEPDblBuf1Count(ep->num);
                        if (count != 0)
                        {
                            PMAToUserBufferCopy(ep->xfer_buff, ep->pmaaddr1, count);
                        }
                    }

                    FreeUserBuffer(ep->num, EP_DBUF_OUT);
                }

                /*multi-packet on the NON control OUT endpoint*/
                ep->xfer_count += count;
                ep->xfer_buff  += count;

                if ((ep->xfer_len == 0) || (count < ep->maxpacket))
                {
                    /* RX COMPLETE */
                    DCD_DataOutStage(ep->num);
                }
                else
                {
                    DCD_EP_PrepareRx(ep->num, ep->xfer_buff, ep->xfer_len);
                }

            } /* if((wEPVal & EP_CTR_RX) */

            if ((wEPVal & EP_CTR_TX) != 0)
            {
                ep = &(pUsbDCD->in_ep[EPindex]);

                /* clear int flag */
                _ClearEP_CTR_TX(EPindex);

                /* IN double Buffering*/
                if (ep->doublebuffer == 0)
                {
                    ep->xfer_count = GetEPTxCount(ep->num);
                    if (ep->xfer_count != 0)
                    {
                        UserToPMABufferCopy(ep->xfer_buff, ep->pmaadress, ep->xfer_count);
                    }
                }
                else
                {
                    if (GetENDPOINT(ep->num) & EP_DTOG_TX)
                    {
                        /*read from endpoint BUF0Addr buffer*/
                        ep->xfer_count = GetEPDblBuf0Count(ep->num);
                        if (ep->xfer_count != 0)
                        {
                            UserToPMABufferCopy(ep->xfer_buff, ep->pmaaddr0, ep->xfer_count);
                        }
                    }
                    else
                    {
                        /*read from endpoint BUF1Addr buffer*/
                        ep->xfer_count = GetEPDblBuf1Count(ep->num);
                        if (ep->xfer_count != 0)
                        {
                            UserToPMABufferCopy(ep->xfer_buff, ep->pmaaddr1, ep->xfer_count);
                        }
                    }

                    FreeUserBuffer(ep->num, EP_DBUF_IN);
                }

                /*multi-packet on the NON control IN endpoint*/
                ep->xfer_count = GetEPTxCount(ep->num);
                ep->xfer_buff += ep->xfer_count;

                /* Zero Length Packet? */
                if (ep->xfer_len == 0)
                {
                    /* TX COMPLETE */
                    DCD_DataInStage(ep->num);
                }
                else
                {
                    DCD_EP_Tx(ep->num, ep->xfer_buff, ep->xfer_len);
                }

            } /* if((wEPVal & EP_CTR_TX) != 0) */

        }/* if(EPindex == 0) else */
    
  }/* while(...) */
}

void DCD_Reset(void)
{
    /* Set EP0 OUT PMA address */
    DCD_PMA_Config(EP_IN_CTRL, USB_SNG_BUF, ENDP0_RX_ADDRESS);

    /* Set EP0 IN PMA address */
    DCD_PMA_Config(EP_OUT_CTRL, USB_SNG_BUF, ENDP0_TX_ADDRESS);
    
    /* Open EP0 OUT */
    DCD_EP_Open(EP_IN_CTRL, USB_MAX_EP0_SIZE, EP_TYPE_CTRL);
    
    /* Open EP0 IN */
    DCD_EP_Open(EP_OUT_CTRL, USB_MAX_EP0_SIZE, EP_TYPE_CTRL);

    /* Upon Reset call user call back */
    pUsbDCD->device_status = USB_STA_DEFAULT;

    /* set edpoint address*/
    DCD_EP_SetAddress(0x00);
}

void DCD_DmaOverUnderRun(void)
{
    if(NULL != pUsbDCD->class_cb->DmaOver)
    {
        pUsbDCD->class_cb->DmaOver(pUsbDCD); 
    }

}

void DCD_ErrorsHandler(void)
{
    if(NULL != pUsbDCD->class_cb->ErrorsHandler)
    {
        pUsbDCD->class_cb->ErrorsHandler(pUsbDCD); 
    }

}

void DCD_DataInStage(uint8_t ep_num)
{
    USB_EP *ep;

    if(ep_num == 0) 
    {
        ep = &(pUsbDCD->in_ep[0]);
        if (USB_EP0_DATA_IN == pUsbDCD->device_state)
        {
            if(ep->rem_data_len > ep->maxpacket)
            {
                ep->rem_data_len -=  ep->maxpacket;
                USBD_CtlContinueSendData(ep->xfer_buff, ep->rem_data_len);
            }
            else
            { /* last packet is MPS multiple, so send ZLP packet */
                if((ep->total_data_len %  ep->maxpacket == 0) &&
                   (ep->total_data_len >= ep->maxpacket     ) &&
                   (ep->total_data_len <  ep->ctl_data_len  ))
                {
                    USBD_CtlContinueSendData(NULL, 0);
                    ep->ctl_data_len = 0;
                }
                else
                {
                  if((NULL != pUsbDCD->class_cb->EP0_TxSent) &&
                     (USB_STA_CONFIGURED == pUsbDCD->device_status))
                  {
                        pUsbDCD->class_cb->EP0_TxSent(pUsbDCD); 
                  }
        
                  USBD_CtlReceiveStatus(pUsbDCD);
                }
            }
        }
        else if (USB_EP0_STATUS_IN == pUsbDCD->device_state)
        {
            uint32_t device_addr = USBD_GetDeviceAddress();
            if (0x00 != device_addr)
            {
                DCD_EP_SetAddress(device_addr);
                USBD_SetDeviceAddress(0x00);
            }
        }
    }
    else if((NULL != pUsbDCD->class_cb->DataIn) && (USB_STA_CONFIGURED == pUsbDCD->device_status))
    {
        pUsbDCD->class_cb->DataIn(pUsbDCD, ep_num); 
    }

}

void DCD_DataOutStage(uint8_t ep_num)
{
    USB_EP *ep;

    if(ep_num == 0) 
    {
        ep = &pUsbDCD->out_ep[0];
        if (USB_EP0_DATA_OUT == pUsbDCD->device_state)
        {
            if(ep->rem_data_len > ep->maxpacket)
            {
                ep->rem_data_len -= ep->maxpacket;
                USBD_CtlContinueRx(ep->xfer_buff, MIN(ep->rem_data_len, ep->maxpacket));
            }
            else
            {
                if((NULL != pUsbDCD->class_cb->EP0_RxReady) && (USB_STA_CONFIGURED == pUsbDCD->device_status))
                {
                    pUsbDCD->class_cb->EP0_RxReady(pUsbDCD); 
                }

                USBD_CtlSendStatus(pUsbDCD);
            }
        }
    }
    else if((NULL != pUsbDCD->class_cb->DataOut) && (USB_STA_CONFIGURED == pUsbDCD->device_status))
    {
        pUsbDCD->class_cb->DataOut(pUsbDCD, ep_num); 
    }

}

void DCD_SetupStage(void)
{
    USB_SETUP_REQ req;

    /* parse setup package */
    //USBD_ParseSetupRequest(pUsbDCD , &req);
    req.bmRequest     = *(uint8_t *)  (pUsbDCD->setup_packet);
    req.bRequest      = *(uint8_t *)  (pUsbDCD->setup_packet +  1);
    req.wValue        = SWAPBYTE      (pUsbDCD->setup_packet +  2);
    req.wIndex        = SWAPBYTE      (pUsbDCD->setup_packet +  4);
    req.wLength       = SWAPBYTE      (pUsbDCD->setup_packet +  6);
      
    pUsbDCD->in_ep[0].ctl_data_len = req.wLength  ;
    pUsbDCD->device_state = USB_EP0_SETUP;

    switch (req.bmRequest & 0x1F) 
    {
    case USB_REQ_RECIPIENT_DEVICE:   
      USBD_StdDevReq(pUsbDCD, &req);
      break;
      
    case USB_REQ_RECIPIENT_INTERFACE:     
      USBD_StdItfReq(pUsbDCD, &req);
      break;
      
    case USB_REQ_RECIPIENT_ENDPOINT:        
      USBD_StdEPReq(pUsbDCD, &req);   
      break;
      
    default:           
      DCD_EP_Stall(req.bmRequest & 0x80);
      break;
    }  
}

/* Start of Frame */
void DCD_StartOfFrame(void)
{
    if(NULL != pUsbDCD->class_cb->SOF && USB_STA_CONFIGURED == pUsbDCD->device_status)
    {
        pUsbDCD->class_cb->SOF(pUsbDCD); 
    }
}

void DCD_EndOfFrame(void)
{
    /* resume handling timing is made with ESOFs */
    Resume(RESUME_ESOF); /* request without change of the machine state */
}

void DCD_LpmRequest(void)
{
    /* enter macrocell in suspend and system in low power mode (STOP mode) when 
    USB_DEVICE_LOW_PWR_MGMT_SUPPORT defined in usb_conf.h.*/   
    /* Please note that in this example we enter in STOP mode during L1 state independently
    from value read in BESL (even for BESL= 0 which corresponds to 50us delay) because
    STM32F072 can wakeup system from STOP mode in less than 50 us */
    DCD_Suspend();

}

void DCD_Suspend(void)
{
    pUsbDCD->device_old_status = pUsbDCD->device_status;
    /*Device is in Suspended State*/
    pUsbDCD->device_status  = USB_STA_SUSPENDED;
    
    if(NULL != pUsbDCD->class_cb->Suspend)
    {
        pUsbDCD->class_cb->Suspend(pUsbDCD);
    }

    /* enter macrocell in suspend and system in low power mode when 
        USB_DEVICE_LOW_PWR_MGMT_SUPPORT defined in usb_conf.h
     */
    Suspend();

}

void DCD_Resume(void)
{
    pUsbDCD->device_status = pUsbDCD->device_old_status;

    /* Handle Resume state machine */  
    Resume(RESUME_EXTERNAL);
}

eUsbDeviceStatus DCD_GetStatus(void)
{
    return pUsbDCD->device_status;
}


/**
  * @brief  USBD_SetCfg 
  *        Configure device and start the interface
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status
  */

USBD_Status USBD_SetCfg(DCD_DEV *pdev, uint8_t cfgidx)
{
  pdev->class_cb->Init(pdev, cfgidx); 
  
  return USBD_OK; 
}

/**
  * @brief  USBD_ClrCfg 
  *         Clear current configuration
  * @param  pdev: device instance
  * @param  cfgidx: configuration index
  * @retval status: USBD_Status
  */
USBD_Status USBD_ClrCfg(DCD_DEV *pdev, uint8_t cfgidx)
{
  pdev->class_cb->DeInit(pdev, cfgidx);   
  return USBD_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
