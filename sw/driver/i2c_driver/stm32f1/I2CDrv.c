/**
 *  @file      I2CDrv.c
 *  @brief     This file contains the stm32f0xx I2C driver implementation.
 *  @author    Edmond Sung
 *  @date      04-May-2013
 *  @copyright Tymphany Ltd.
 */
#include "stdbool.h"
#include "stm32f10x.h"
//#include "stm32f0_discovery.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "./I2CDrv_priv.h"
#include "trace.h"
#include "assert.h"
#include "attachedDevices.h"


//#include "stm32f0xx_i2c.h"
//#include "stm32f0xx_rcc.h"

#define I2C_TIMEOUT     0xffff;
uint32 i2c_Timeout=I2C_TIMEOUT;


/*============================================================================*/
/* Forward declaration of private functions */
/*============================================================================*/
/* PUBLIC FUNCTIONS */


/**
 * Construct the i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void I2CDrv_Ctor(cI2CDrv * me, tI2CDevice * pConfig)
{
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_TypeDef* I2Cx;
    me->pConfig = pConfig;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    
  /* GPIO initialization */
    sEE_LowLevel_Init();

    if (me->isReady == FALSE)
    {
      /*!< I2C configuration */
      /* sEE_I2C configuration */
#ifdef PORT_TO_STM32F1
      I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
      I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
      I2C_InitStructure.I2C_DigitalFilter = 0x00;
      I2C_InitStructure.I2C_OwnAddress1 = 0x00;
      I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
      I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
      I2C_InitStructure.I2C_Timing = I2C_TIMING;
#endif 
      /* Apply sEE_I2C configuration after enabling it */
      I2C_Init(I2Cx, &I2C_InitStructure);
       
      /* sEE_I2C Peripheral Enable */
      I2C_Cmd(I2Cx, ENABLE);
      me->isReady = TRUE;
    }

}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void I2CDrv_Xtor(cI2CDrv * me)
{
    I2C_TypeDef* I2Cx;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
        I2Cx=I2C1;
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
        I2Cx=I2C2;
  /* GPIO deinitialization */
    //sEE_LowLevel_Init();
    I2C_DeInit(I2Cx);
    I2C_Cmd(I2Cx, DISABLE);
    me->isReady = FALSE;
}

/**
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg const * const  msg
 *      representing structure: length of the array to be sent
 *      and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TRUE on success, otherwise FALSE
 */
eTpRet I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg)
{
    I2C_TypeDef*    I2Cx;
    uint8*          pMsg = msg->pMsg;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
        I2Cx=I2C1;
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
        I2Cx=I2C2;
#ifdef PORT_TO_STM32F1
/*----------------------------------------------------------------------------------------------*/  
    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, msg->devAddr, 1, I2C_Reload_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    i2c_Timeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)   
    {
        if((i2c_Timeout--) == 0) 
            return ERROR;
    }

    /* Send Register address */
    I2C_SendData(I2Cx, (uint8_t)(*pMsg));
    pMsg++;

/*----------------------------------------------------------------------------------------------*/  
  
    /* Wait until TCR flag is set */
    i2c_Timeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TCR) == RESET)
    {
        if((i2c_Timeout--) == 0) 
            return ERROR;
    }

    uint32_t DataNum = 0;
    I2C_TransferHandling(I2Cx, msg->devAddr, (uint8_t)(msg->length-1), I2C_AutoEnd_Mode, I2C_No_StartStop);
    while (DataNum != (msg->length-1))
    {      
        /* Wait until TXIS flag is set */
        i2c_Timeout = I2C_TIMEOUT;
        while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
        {
            if((i2c_Timeout--) == 0) 
                return ERROR;
        }  

        /* Write data to TXDR */
        I2C_SendData(I2Cx, (uint8_t)(*pMsg));
        pMsg++;
        /* Update number of transmitted data */
        DataNum++;   
    }  

    /* Wait until STOPF flag is set */
    i2c_Timeout = 10*I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET)
    {
        if((i2c_Timeout--) == 0)
            return ERROR;
    }

    /* Clear STOPF flag */
    I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);
#endif
    return (TP_SUCCESS);
}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */
eTpRet I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg)
{
    uint8 i;
    uint8*          pMsg = msg->pMsg;
    I2C_TypeDef* I2Cx;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
        I2Cx=I2C1;
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
        I2Cx=I2C2;
    
#ifdef PORT_TO_STM32F1
/*-------------------------------------------------------------*/  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(I2Cx, msg->devAddr, 1, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
  
  /* Wait until TXIS flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
  {
    if((i2c_Timeout--) == 0) return ERROR;
  }
  
  /* Send Register address */
  I2C_SendData(I2Cx, (uint8_t)msg->regAddr);
  
  /* Wait until TC flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TC) == RESET)
  {
    if((i2c_Timeout--) == 0) return (-1);
  }  
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(I2Cx, msg->devAddr, (msg->length), I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
/*-------------------------------------------------------------*/  
  /* Read data from RXDR */

    for (i = 0; i < (msg->length); i++)
    {
        /* Wait until RXNE flag is set */
          i2c_Timeout = I2C_TIMEOUT;
        while(I2C_GetFlagStatus(I2Cx, I2C_ISR_RXNE) == RESET)
        {
          if((i2c_Timeout--) == 0) return ERROR;
        }
        
        /* Read data from RXDR */
        *(pMsg) = I2C_ReceiveData(I2Cx);

        if (NULL != (pMsg))
        {
            (pMsg)++;
        }
        /* Update number of received data */
    }// end of for



  
  /* Wait until STOPF flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET) 
  {
    if((i2c_Timeout--) == 0) return (-1);
  }
  
  /* Clear STOPF flag */
  I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);
#endif
  /*!< Return Register value */
  return (TP_SUCCESS);
}

eTpRet I2CDrv_MasterWriteWith2ByteRegAddress(cI2CDrv * me, tI2CMsg * const msg)
{
    I2C_TypeDef* I2Cx;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
        I2Cx=I2C1;
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
        I2Cx=I2C2;
    
#ifdef PORT_TO_STM32F1
/*----------------------------------------------------------------------------------------------*/  
    /* Configure slave address, nbytes, reload, end mode and start or stop generation */
    I2C_TransferHandling(I2Cx, msg->devAddr, 2, I2C_Reload_Mode, I2C_Generate_Start_Write);

    /* Wait until TXIS flag is set */
    i2c_Timeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)   
    {
        if((i2c_Timeout--) == 0)
        {
            ASSERT(0);
            return ERROR;
        }
    }

    /* Send Register address */
    I2C_SendData(I2Cx, (uint8_t)(( msg->regAddr&0xff00)>>8));


    /* Wait until TXIS flag is set */
    i2c_Timeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)   
    {
        if((i2c_Timeout--) == 0)
        {
            ASSERT(0);
            return ERROR;
        }
    }

    /* Send Register address */
    I2C_SendData(I2Cx, (uint8_t)( msg->regAddr&0x00ff));

/*----------------------------------------------------------------------------------------------*/  

  
    /* Wait until TCR flag is set */
    i2c_Timeout = I2C_TIMEOUT;
    while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TCR) == RESET)
    {
        if((i2c_Timeout--) == 0)
        {
            ASSERT(0);
            return ERROR;
        }
    }

    uint32_t DataNum = 0;
    I2C_TransferHandling(I2Cx, msg->devAddr, (uint8_t)(msg->length), I2C_AutoEnd_Mode, I2C_No_StartStop);
    while (DataNum != (msg->length))
    {      
        /* Wait until TXIS flag is set */
        i2c_Timeout = I2C_TIMEOUT;
        while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
        {
            if((i2c_Timeout--) == 0)
            {
                ASSERT(0);
                return ERROR;
            }
        }  

        /* Write data to TXDR */
        I2C_SendData(I2Cx, (uint8_t)(*( msg->pMsg)));
        DataNum++;
        (msg->pMsg)++;
        /* Update number of transmitted data */
    }

  /* Wait until STOPF flag is set */
  i2c_Timeout = 10*I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET)
  {
    if((i2c_Timeout--) == 0)
    {
        ASSERT(0);
        return ERROR;
    }
  }   
  
  /* Clear STOPF flag */
  I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);
#endif
  
  return (TP_SUCCESS);
}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */
eTpRet I2CDrv_MasterReadWith2ByteRegAddress( cI2CDrv * const me, tI2CMsg * msg)
{
    I2C_TypeDef* I2Cx;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
        I2Cx=I2C1;
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
        I2Cx=I2C2;
    
#ifdef PORT_TO_STM32F1
  /* Test on BUSY Flag */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_BUSY) != RESET)
  {
    if((i2c_Timeout--) == 0) return (-1);
  }
/*-------------------------------------------------------------------------------------------*/  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(I2Cx, msg->devAddr, 2, I2C_SoftEnd_Mode, I2C_Generate_Start_Write);
  
  /* Wait until TXIS flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
  {
    if((i2c_Timeout--) == 0)
    {
       ASSERT(0);
       return (-1);
    }
  }
  
  /* Send Register address */
  I2C_SendData(I2Cx, (uint8_t)((msg->regAddr&0xff00)>>8));

  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TXIS) == RESET)
  {
    if((i2c_Timeout--) == 0)
    {
      ASSERT(0);
      return (-1);
    }
}
  
  /* Send Register address */
  I2C_SendData(I2Cx, (uint8_t)(msg->regAddr&0x00ff));

/*-------------------------------------------------------------------------------------------*/  
  
  /* Wait until TC flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_TC) == RESET)
  {
    if((i2c_Timeout--) == 0)
    {
      ASSERT(0);
      return (-1);
    }
  }  
  
  /* Configure slave address, nbytes, reload, end mode and start or stop generation */
  I2C_TransferHandling(I2Cx, msg->devAddr, (uint8_t)(msg->length), I2C_AutoEnd_Mode, I2C_Generate_Start_Read);
  
  /* Read data from RXDR */
    uint32_t DataNum = 0;
    while(DataNum != (msg->length))
    {
        /* Wait until RXNE flag is set */
        i2c_Timeout = I2C_TIMEOUT;
        while(I2C_GetFlagStatus(I2Cx, I2C_ISR_RXNE) == RESET)
        {
          if((i2c_Timeout--) == 0)
          {
             ASSERT(0);
             return (-1); 
          }
        }
        /* Read data from RXDR */
        *(msg->pMsg) = I2C_ReceiveData(I2Cx);
        /* Update number of received data */
        DataNum++;
        if (NULL != (msg->pMsg))
        {
            (msg->pMsg)++;
        }
    }


  
  /* Wait until STOPF flag is set */
  i2c_Timeout = I2C_TIMEOUT;
  while(I2C_GetFlagStatus(I2Cx, I2C_ISR_STOPF) == RESET) 
  {
    if((i2c_Timeout--) == 0) 
    {
      ASSERT(0);
      return (-1);
    }
  }
  
  /* Clear STOPF flag */
  I2C_ClearFlag(I2Cx, I2C_ICR_STOPCF);
  
  /*!< Return Register value */
  return (1);

#endif

}

/*============================================================================*/
/* PRIVATE FUNCTIONS */




/*============================================================================*/


/*____________________________________________________________________________*/




