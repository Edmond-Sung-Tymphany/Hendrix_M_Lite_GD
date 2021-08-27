/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/
/**
 *  @file      I2CDrv.c
 *  @brief     This file contains the stm32f0xx I2C driver implementation.
 *  @author    Edmond Sung
 *  @date      04-May-2013
 *  @copyright Tymphany Ltd.
 */
#include "stdbool.h"
#include "stm32f0xx.h"
#include "commonTypes.h"
#include "I2CDrv.h"
#include "./I2CDrv_priv.h"
#include "trace.h"
#include "assert.h"
#include "attachedDevices.h"
#include "bsp.h"

/*   TODO: consider I2C speed to design timeout
 */
#define I2C_TIMEOUT_MS    20
#define I2C_TIMEOUT                               0xff
uint32_t I2C_Timeout = I2C_TIMEOUT;

typedef enum
{
  I2C_OK                                          = 0,
  I2C_FAIL                                        = 1
}I2C_Status;

#ifdef SYSCLK_RUN_AT_32M
const static tI2CSpeedMap i2cSpeedMap[] = 
{
    {400, 0x00601B28},
    {350, 0x00601936},
    {100, 0x2020174E},
};
#else
/* Note we assume main clock is 8MHz. 
 * Chnage clock need to modify i2cSpeedMap[]
 */

#define SCL_400     (400)
#define PRESC_400   (0x00 << 28)   // timing prescalr, referring to datasheet RM0360 in page 584
#define SCLDEL_400  (0x01 << 20)   // data setup time.
#define SDADEL_400  (0x00 << 16)   // Data hold time.
#define SCLH_400    (0x03 << 8)    //SCL high period
#define SCLL_400    (0x08)         //SCL low period

#define SCL_350     (350)          //practical test for near 348.2khz
#define PRESC_350   (0x00 << 28)   // timing prescalr,referring to datasheet RM0360 in page 584
#define SCLDEL_350  (0x01 << 20)   // data setup time.Practical test for near 1.12us
#define SDADEL_350  (0x00 << 16)   // Data hold time.Practical test for near 500ns
#define SCLH_350    (0x05 << 8)    //SCL high period.Practical test for near 1.1us
#define SCLL_350    (0x09)         //SCL low period.Practical test for near 1.6us

#define SCL_100     (100)
#define PRESC_100   (0x01 << 28)   // timing prescalr, referring to datasheet RM0360 in page 584
#define SCLDEL_100  (0x04 << 20)   // data setup time.
#define SDADEL_100  (0x02 << 16)   // Data hold time.
#define SCLH_100    (0x0f << 8)    //SCL high period
#define SCLL_100    (0x13)         //SCL low period

const static tI2CSpeedMap i2cSpeedMap[] = 
{
    {SCL_400, PRESC_400 | SCLDEL_400 | SDADEL_400 | SCLH_400 | SCLL_400}, //400KHz for 8MHz CPU frequency
    {SCL_350, PRESC_350 | SCLDEL_350 | SDADEL_350 | SCLH_350 | SCLL_350}, //350KHz for 8MHz CPU frequency
    {SCL_100, PRESC_100 | SCLDEL_100 | SDADEL_100 | SCLH_100 | SCLL_100}  //100KHz for 8MHz CPU frequency
};
#endif

static BOOL I2CDrv_WaitReset(I2C_TypeDef * I2Cx, uint32_t resetId, uint32 timeOut);

/*============================================================================*/
/* Forward declaration of private functions */
/*============================================================================*/
/* PUBLIC FUNCTIONS */

static bool bIsCh1Ready=0, bIsCh2Ready=0;
static uint32 ch1BaudRate=0, ch2BaudRate=0;
static uint16 ch1UserCount=0, ch2UserCount=0;
 uint8_t write_Num;
 
__weak void I2C2_LowLevel_Deinit(void)
{
    // projects should have the own implementation
}


/**
 * Construct the i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void I2CDrv_Ctor(cI2CDrv * me, tI2CDevice * pConfig)
{
    
    ASSERT(me && pConfig->deviceInfo.deviceType==I2C_DEV_TYPE);
   
    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
     * chXUserCount is never 0, and never xtor I2C
     */
    ASSERT(!me->isReady);
    
    I2C_TypeDef* I2Cx;
    me->pConfig = pConfig;
    me->stopEnable= TRUE;
    
	
    if(pConfig->channel ==I2C_CHANNEL_ONE)
    {      
        ch1UserCount++;
        if (bIsCh1Ready == FALSE)
        {
            I2Cx=I2C1;
            I2CDrv_LowLevelInit(me);
            ch1BaudRate= me->pConfig->baudRate;
            bIsCh1Ready = TRUE;
        }
        
        /* All configuration on a I2C bus should have the same baut rate.
         * But to avoid wrong setting, we double check here.
         */
        ASSERT(ch1BaudRate==me->pConfig->baudRate);
    }
    else if(pConfig->channel ==I2C_CHANNEL_TWO)
    {      
        ch2UserCount++;
        if (bIsCh2Ready == FALSE)
        {	     
		
            I2Cx=I2C2;
            I2CDrv_LowLevelInit(me);
            ch2BaudRate= me->pConfig->baudRate;
            bIsCh2Ready = TRUE;	    	
        }
        
        /* All configuration on a I2C bus should have the same baut rate.
         * But to avoid wrong setting, we double check here.
         */
        ASSERT(ch2BaudRate==me->pConfig->baudRate);
    }
    else
    {
        /* Wrong I2C bus parameter */
        ASSERT(0);
    }
    
    me->isReady = TRUE;
    (void)I2Cx;  // to supress compiler warning

}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void I2CDrv_Xtor(cI2CDrv * me)
{
    ASSERT(me);
        
    /* Duplicate xtor is invalid. If duplicate xtor but no dulipcate ctor,
     * chXUserCount is wrong
     */
    ASSERT(me->isReady);
    
    uint16 *pUserCount= &ch1UserCount;
    
    I2C_TypeDef* I2Cx;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        ASSERT(ch1UserCount>0);
        ch1UserCount--;
        I2Cx=I2C1;
        bIsCh1Ready = FALSE;
        pUserCount= &ch1UserCount;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        ASSERT(ch2UserCount>0);
        ch2UserCount--;
        I2Cx=I2C2;
        bIsCh2Ready = FALSE;
        pUserCount= &ch2UserCount;
    }
    else
    {
        ASSERT(0);
    }

    //When we are last xtor on this I2C bus, xtor it!
    if(*pUserCount==0)
    {
        /* GPIO deinitialization */
        //sEE_LowLevel_Init();
        I2C_DeInit(I2Cx);
        I2C_Cmd(I2Cx, DISABLE);
        if (me->pConfig->channel == I2C_CHANNEL_TWO)
        {
            I2C2_LowLevel_Deinit();
        }
    }
    
    me->isReady = FALSE;
}


/* Set if send stop bit on end of serial commands
 * parameter stopEnable:
 *    TRUE: send STOP on end of serial commands
 *    FALSE: do not send STOP on end of serial commands
 */
void I2CDrv_SetStopOperation(cI2CDrv * me, bool stopEnable)
{
    if(stopEnable)
        me->stopEnable= TRUE;
    else
        me->stopEnable= FALSE;
}


static void I2CDrv_LowLevelInit(cI2CDrv * me)
{
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_TypeDef* I2Cx;
    
    /* GPIO initialization */
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        
        I2C1_LowLevel_Init();
        I2Cx=I2C1;
        I2C_DeInit(I2C1); 
	
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        
        I2C2_LowLevel_Init();
        I2Cx=I2C2;
        I2C_DeInit(I2C2);
	 
    }
    
    /*!< I2C configuration */
    /* sEE_I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    //I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; 
    //I2C_InitStructure.I2C_DigitalFilter = 0x00;
    I2C_InitStructure.I2C_OwnAddress1 = 0x30;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;         //100K??
    #if 0
    uint8 i = 0;
    for(i = 0; i < ArraySize(i2cSpeedMap); i++)
    {
        if(me->pConfig->baudRate == i2cSpeedMap[i].speedInKHz)
        {
          I2C_InitStructure.I2C_Timing = i2cSpeedMap[i].timing;
          break;
        }
    }
    
    ASSERT(i < ArraySize(i2cSpeedMap));
    #endif 
    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(I2Cx, &I2C_InitStructure);
    I2C_AcknowledgeConfig(I2Cx, ENABLE); 
    I2C_Cmd(I2Cx, ENABLE); 
    /* sEE_I2C Peripheral Enable */
    //I2C_Cmd(I2Cx, ENABLE);
}


/**
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg const * const  msg
 *      representing structure: length of the array to be sent
 *      and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */


#if 0
eTpRet I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg)
{
    uint8_t write_Num;

    I2C_TypeDef*    I2Cx;
    uint8*          pMsg = msg->pMsg;
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); /* Support I2C1/I2C2 currently */
        return TP_FAIL;
    }
    
/*----------------------------------------------------------------------------------------------*/  
    /* Configure slave address, nbytes, reload, end mode and start or stop generation */

  I2C_Timeout = I2C_TIMEOUT;
	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))//1
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }

  I2C_GenerateSTART(I2Cx, ENABLE);
	I2C_Timeout = I2C_TIMEOUT;
	//while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//2
       while(!I2C_CheckEvent(I2Cx, 0x00000001))   
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }

	I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); //driver_Addr<<1
	I2C_Timeout = I2C_TIMEOUT;

  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//3
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }


  for(write_Num = 0; write_Num < msg->length; write_Num++)
	{
		  I2C_SendData(I2Cx, pMsg[write_Num]); 
      I2C_Timeout = I2C_TIMEOUT;
			  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//5
				{
					if((I2C_Timeout--) == 0)
					{
						return I2C_FAIL;
					}
				}
    }

	I2C_GenerateSTOP(I2Cx, ENABLE);		

    return (TP_SUCCESS);
}

#else 

static bool IICget_status(I2C_TypeDef* I2Cx, uint32_t I2C_EVENT)@"IICget_status"
{
  uint16_t flag1 = 0, flag2 = 0;
  uint32_t flag = 0;
  
  bool status=ERROR;
  flag1 = I2Cx->SR1&0x00ff;
    flag2 = I2Cx->SR2&0x00ff;
//  flag2 = flag2 << 16;


 flag = (uint32_t)( (flag1)|((uint32_t)flag2<<16));
    
  if ((flag & I2C_EVENT) == I2C_EVENT)
  {
    /* SUCCESS: last event is equal to I2C_EVENT */
    status = SUCCESS;
  }
  else
  {
    /* ERROR: last event is different from I2C_EVENT */
    status = ERROR;
  }
  
  return status;
}

static void reinit_IIC(I2C_TypeDef* I2Cx)
{
    I2C_InitTypeDef  I2C_InitStructure;
    
    I2C_DeInit(I2Cx);
    if(I2Cx==I2C1)I2C1_LowLevel_Init();
    else if(I2Cx==I2C2)I2C2_LowLevel_Init();
    
    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; 
//    I2C_InitStructure.I2C_OwnAddress1 = 0x30;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000;         //100K??

    I2C_Init(I2Cx, &I2C_InitStructure);
    I2C_AcknowledgeConfig(I2Cx, ENABLE); 
    I2C_Cmd(I2Cx, ENABLE); 
}

#define I2C_TIMEOUT     0xfff;

eTpRet I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg){

    uint8_t write_Num;

    I2C_TypeDef*    I2Cx;
    uint8*          pMsg = msg->pMsg;
    uint16  i2c_timeout = I2C_TIMEOUT;

  
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); /* Support I2C1/I2C2 currently */
        return TP_FAIL;
    }
   	i2c_timeout = I2C_TIMEOUT;
   while(IICget_status(I2Cx, I2C_FLAG_BUSY) && (i2c_timeout--))
    {
      reinit_IIC(I2Cx);
    }
    
    /* send a start condition to I2C bus */
    I2C_GenerateSTART(I2Cx, ENABLE);
    /* wait until SBSEND bit is set */
    
	i2c_timeout = I2C_TIMEOUT;
   while(!IICget_status(I2Cx, 0x00000001) && (i2c_timeout--));
   if (i2c_timeout<=0)
    {
        return (TP_FAIL);
    }
     //  while(!((I2Cx->SR1&0x00ff)==0x00000001));
	
    /* send slave address to I2C bus */
    I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); 
    
    /* wait until ADDSEND bit is set */
	i2c_timeout = I2C_TIMEOUT;
   while(!IICget_status(I2Cx, 0x00000002) && (i2c_timeout--));
    if (i2c_timeout<=0)
     {
         return (TP_FAIL);
     }
    
    /* clear ADDSEND bit */
    if(I2Cx==I2C1){
    (*(volatile uint32_t *)(uint32_t)(0x40005414));
    (*(volatile uint32_t *)(uint32_t)(0x40005418));
    }
    else  if(I2Cx==I2C2){
    (*(volatile uint32_t *)(uint32_t)(0x40005814));
    (*(volatile uint32_t *)(uint32_t)(0x40005818));
    }
    
    /* wait until the transmit data buffer is empty */
    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000080)&&(i2c_timeout--));
    if (i2c_timeout<=0)
     {
         return (TP_FAIL);
     }

    for(write_Num = 0; write_Num < msg->length; write_Num++){
        /* data transmission */
        I2C_SendData(I2Cx, pMsg[write_Num]);
        /* wait until the TBE bit is set */
        i2c_timeout = I2C_TIMEOUT;
        while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
        if (i2c_timeout<=0)
         {
             return (TP_FAIL);
         }
    }
    /* send a stop condition to I2C bus */
    I2C_GenerateSTOP(I2Cx, ENABLE);
    i2c_timeout = I2C_TIMEOUT;
    while((I2Cx->CR1)&0x0200 && (i2c_timeout--));
    if (i2c_timeout<=0)
     {
         return (TP_FAIL);
     }

  return (TP_SUCCESS);
}

#endif 
/*
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * The chip address is one byte, register address is two bytes, data are in byte
 * @param cI2CDrv * me - pointer to driver instance
 * @param tI2CMsg * const pI2CData - msg representing structure: length of the array to be sent.
 * and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TRUE on success, otherwise FALSE
 */
eTpRet I2CDrv_MasterWriteWith2ByteRegAddress(cI2CDrv * me, tI2CMsg * const msg)
{
    I2C_TypeDef*    I2Cx;
    uint8_t *pMsg = (uint8_t *)(msg->pMsg);
    uint8_t u8_addr;
    uint16_t len, reg_addr;
    uint16  i2c_timeout = I2C_TIMEOUT;

    len = msg->length;
    reg_addr = (uint16_t)msg->regAddr;

    ASSERT( me->pConfig->regAddrLen == REG_LEN_16BITS );
    ASSERT(len < 251);  // if len > 255, we need to set to special I2C register, don't support > 255 case now.


    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); /* Support I2C1/I2C2 currently */
        return TP_FAIL;
    }

    i2c_timeout = I2C_TIMEOUT;

    //while(IICget_status(I2Cx, I2C_FLAG_BUSY));
    while(IICget_status(I2Cx, I2C_FLAG_BUSY) && (i2c_timeout--))
    {
      reinit_IIC(I2Cx);
    }
    
    /* send a start condition to I2C bus */
    I2C_GenerateSTART(I2Cx, ENABLE);
    /* wait until SBSEND bit is set */
    i2c_timeout = I2C_TIMEOUT;    
    while(!IICget_status(I2Cx, 0x00000001) && (i2c_timeout--));
    /* send slave address to I2C bus */
    I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); 
    
    /* wait until ADDSEND bit is set */
    i2c_timeout = I2C_TIMEOUT;    
    while(!IICget_status(I2Cx, 0x00000002) && (i2c_timeout--));
    
    /* clear ADDSEND bit */
    if(I2Cx==I2C1){
    (*(volatile uint32_t *)(uint32_t)(0x40005414));
    (*(volatile uint32_t *)(uint32_t)(0x40005418));
    }
    else  if(I2Cx==I2C2){
    (*(volatile uint32_t *)(uint32_t)(0x40005814));
    (*(volatile uint32_t *)(uint32_t)(0x40005818));
    }
    
    /* wait until the transmit data buffer is empty */
    i2c_timeout = I2C_TIMEOUT;    
    while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));


     //send reg adress
    I2C_SendData(I2Cx, msg->regAddr>>8);
    i2c_timeout = I2C_TIMEOUT;    
    while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
	
    //send reg adress
    I2C_SendData(I2Cx, msg->regAddr & 0x00ff);
    i2c_timeout = I2C_TIMEOUT;   
    while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
    
    for(write_Num = 0; write_Num < msg->length; write_Num++){
        /* data transmission */
        I2C_SendData(I2Cx, pMsg[write_Num]);
        /* wait until the TBE bit is set */
        i2c_timeout = I2C_TIMEOUT;    
        while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
    }
    /* send a stop condition to I2C bus */
    I2C_GenerateSTOP(I2Cx, ENABLE);	
    
    i2c_timeout = I2C_TIMEOUT;    
    while((I2Cx->CR1)&0x0200 && (i2c_timeout--));
    #if 0
    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); /* Support I2C1/I2C2 currently */
        return TP_FAIL;
    }

    /*----------------------------------------------------------------------------------------------*/  
    /* Configure slave address, nbytes, reload, end mode and start or stop generation */


	while(I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY))//1
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }

  I2C_GenerateSTART(I2Cx, ENABLE);
	I2C_Timeout = I2C_TIMEOUT;
	while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT))//2
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }

	I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); //driver_Addr<<1
	I2C_Timeout = I2C_TIMEOUT;
  while(!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))//3
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }


    
    u8_addr = (uint8_t)((reg_addr>>8) & 0x00ff);

    I2C_SendData(I2Cx, u8_addr); 
       while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//5
	{
		if((I2C_Timeout--) == 0)
		{
			return I2C_FAIL;
		}
	}


      u8_addr = (uint8_t)(reg_addr & 0x00ff);

    I2C_SendData(I2Cx, u8_addr); 
       while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//5
	{
		if((I2C_Timeout--) == 0)
		{
			return I2C_FAIL;
		}
	}
	/*I2C_SendData(I2Cx, start_Addr);
	I2C_Timeout = I2C_TIMEOUT;
while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//4
  {
    if((I2C_Timeout--) == 0)
    {
      return I2C_FAIL;
    }
  }*/

  for(write_Num = 0; write_Num < msg->length; write_Num++)
	{
		  I2C_SendData(I2Cx, pMsg[write_Num]); 
      I2C_Timeout = I2C_TIMEOUT;
			  while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))//5
				{
					if((I2C_Timeout--) == 0)
					{
						return I2C_FAIL;
					}
				}
    }

	I2C_GenerateSTOP(I2Cx, ENABLE);	
    #endif
    return (TP_SUCCESS);
}

eTpRet I2CDrv_MasterWriteWithRegAddress(cI2CDrv * me, tI2CMsg const * const  msg)
{
    uint8_t write_Num;

    I2C_TypeDef*    I2Cx;
    uint8*          pMsg = msg->pMsg;
    uint16 i2c_timeout = I2C_TIMEOUT;   

    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); /* Support I2C1/I2C2 currently */
        return TP_FAIL;
    }

    //while(IICget_status(I2Cx, I2C_FLAG_BUSY));
    
    i2c_timeout = I2C_TIMEOUT;   
    while(IICget_status(I2Cx, I2C_FLAG_BUSY) && (i2c_timeout--))
    {
      reinit_IIC(I2Cx);
    }
    
    /* send a start condition to I2C bus */
    I2C_GenerateSTART(I2Cx, ENABLE);
    /* wait until SBSEND bit is set */
    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000001) && (i2c_timeout--));
    /* send slave address to I2C bus */
    I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); 
    
    /* wait until ADDSEND bit is set */
    i2c_timeout = I2C_TIMEOUT;   
    while(!IICget_status(I2Cx, 0x00000002) && (i2c_timeout--));
    
    /* clear ADDSEND bit */
    if(I2Cx==I2C1){
    (*(volatile uint32_t *)(uint32_t)(0x40005414));
    (*(volatile uint32_t *)(uint32_t)(0x40005418));
    }
    else  if(I2Cx==I2C2){
    (*(volatile uint32_t *)(uint32_t)(0x40005814));
    (*(volatile uint32_t *)(uint32_t)(0x40005818));
    }
    
    /* wait until the transmit data buffer is empty */
    i2c_timeout = I2C_TIMEOUT;   
    while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));

    //send reg adress
    I2C_SendData(I2Cx, msg->regAddr);
    i2c_timeout = I2C_TIMEOUT;   
    while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
    
    for(write_Num = 0; write_Num < msg->length; write_Num++){
        /* data transmission */
        I2C_SendData(I2Cx, pMsg[write_Num]);
        /* wait until the TBE bit is set */
        i2c_timeout = I2C_TIMEOUT;   
        while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
    }
    /* send a stop condition to I2C bus */
    I2C_GenerateSTOP(I2Cx, ENABLE);	
    
    i2c_timeout = I2C_TIMEOUT;   
    while((I2Cx->CR1)&0x0200 && (i2c_timeout--));
              
    return (TP_SUCCESS);

}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg * msg)
{
    uint8*          pMsg = msg->pMsg;
    I2C_TypeDef* I2Cx;
    uint16 i2c_timeout = I2C_TIMEOUT;   

    if (me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        I2Cx=I2C1;
    }
    else if (me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        I2Cx=I2C2;
    }
    else
    {
        ASSERT(0); //not support for more I2C bus
    }

    
    
    //while(IICget_status(I2Cx, I2C_FLAG_BUSY));
    i2c_timeout = I2C_TIMEOUT;
    while(IICget_status(I2Cx, I2C_FLAG_BUSY) && (i2c_timeout--))
    {
      reinit_IIC(I2Cx);
    }
    
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    /* send a start condition to I2C bus */
    I2C_GenerateSTART(I2Cx, ENABLE);
    /* wait until SBSEND bit is set */

    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000001) && (i2c_timeout--));
    /* send slave address to I2C bus */
    I2C_Send7bitAddress(I2Cx, msg->devAddr, I2C_Direction_Transmitter); 
    
    /* wait until ADDSEND bit is set */

    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000002) && (i2c_timeout--));
    
    /* clear ADDSEND bit */
    if(I2Cx==I2C1){
    (*(volatile uint32_t *)(uint32_t)(0x40005414));
    (*(volatile uint32_t *)(uint32_t)(0x40005418));
    }
    else  if(I2Cx==I2C2){
    (*(volatile uint32_t *)(uint32_t)(0x40005814));
    (*(volatile uint32_t *)(uint32_t)(0x40005818));
    }
    
/*-------------------------------------------------------------*/
    switch(me->pConfig->regAddrLen)
    {
      case REG_LEN_NONE:
        //do nothing
        break;
      case REG_LEN_8BITS:
                I2C_SendData(I2Cx, (uint8_t)msg->regAddr);
                /* wait until the TBE bit is set */
                i2c_timeout = I2C_TIMEOUT;
                while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
        break;        
        
      case REG_LEN_16BITS:
                 I2C_SendData(I2Cx, (uint8_t)((msg->regAddr&0xff00)>>8));
                /* wait until the TBE bit is set */
                i2c_timeout = I2C_TIMEOUT;
                while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));
                
                 I2C_SendData(I2Cx, (uint8_t)(msg->regAddr&0x00ff));
                /* wait until the TBE bit is set */
                i2c_timeout = I2C_TIMEOUT;
                while(!IICget_status(I2Cx, 0x00000080) && (i2c_timeout--));        
        break;
      case REG_LEN_24BITS:
        break;
      default:
        ASSERT(0); //wrong register length setting
        break;
    }
//    I2C_GenerateSTOP(I2Cx, ENABLE);
//    while((I2Cx->CR1)&0x0200);
    
   I2C_GenerateSTART(I2Cx, ENABLE);
   i2c_timeout = I2C_TIMEOUT;
   while(!IICget_status(I2Cx, 0x00000001) && (i2c_timeout--));
        
    I2C_Send7bitAddress(I2Cx, msg->devAddr,I2C_Direction_Receiver);//driver_Addr<<1
    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000002) && (i2c_timeout--));

 /*-------------------------------------------------------------*/  
  /* Read data from RXDR */

    uint32_t DataNum = 0;
    while(DataNum != (msg->length-1))
    {   
         i2c_timeout = I2C_TIMEOUT;
         while(!IICget_status(I2Cx, 0x00000040) && (i2c_timeout--));
       *(pMsg) = I2C_ReceiveData(I2Cx);
	 /* Update number of received data */
        DataNum++;
        if (NULL != (pMsg))
        {
            pMsg++;
        }
    }

    I2C_AcknowledgeConfig(I2Cx, DISABLE);	
    i2c_timeout = I2C_TIMEOUT;
    while(!IICget_status(I2Cx, 0x00000040) && (i2c_timeout--));
     *(pMsg) = I2C_ReceiveData(I2Cx);
       
    I2C_GenerateSTOP(I2Cx, ENABLE);
    i2c_timeout = I2C_TIMEOUT;
    while((I2Cx->CR1)&0x0200 && (i2c_timeout--));	
	
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
    /*!< Return Register value */
    return (TP_SUCCESS);
}

/*============================================================================*/
/* PRIVATE FUNCTIONS */
#if 0
static BOOL I2CDrv_WaitReset(I2C_TypeDef* I2Cx, uint32_t resetId, uint32 ms)
{
    eTpRet ret= TP_SUCCESS;
    uint32_t tickstart = getSysTime();
#ifdef I2C_ESTIMATE_TIMEOUT
    int32 us= 1000 * ms;
#endif    
    
    while(I2C_GetFlagStatus(I2Cx, resetId) == RESET)
    {
#ifdef I2C_ESTIMATE_TIMEOUT
        //based on measurement with 8MHz CPU freq., not accurary, not dependent with tick timer
        us--;
        if( us <= 0 ) 
        {
            ret= TP_FAIL;
            break;
        }
#else    
        //based on tick timer, get accurary timeout
        if( (getSysTime() - tickstart) >= ms)
        {
            ret= TP_FAIL;
            break;
        }
#endif
    }
    
#ifndef HAS_I2C_ERROR_RECOVERY
    ASSERT(ret==TP_SUCCESS);
#endif
    
    return ret;
}



/* 
 * [Problem]
 *   If I2C bus have noise and let MCU sense START but no STOP,
 *   it will stay on BUSY status and fail to transmit any data.
 *
 * [Solution]
 *   According to http://www.stmcu.org/module/forum/thread-588340-1-1.html
 *   MCU need to reset I2C to recover it from BUSY to FREE status
 */
static void I2CDrv_RecoverFromBusy(I2C_TypeDef* I2Cx)
{
    I2C_Cmd(I2Cx, DISABLE);
        
    /* STM32 Reference Manual said: When cleared, PE must be kept low for at least 3 APB clock cycles.
     * But while test, the recovery work good even no delay here. Thus remove delay.
     */
    //BSP_BlockingDelayMs(1);
        
    I2C_Cmd(I2Cx, ENABLE);
        
    //Check if recover successful
    ASSERT( !(I2Cx->ISR & I2C_ISR_BUSY) );
}
 #endif     
/*============================================================================*/


/*____________________________________________________________________________*/




