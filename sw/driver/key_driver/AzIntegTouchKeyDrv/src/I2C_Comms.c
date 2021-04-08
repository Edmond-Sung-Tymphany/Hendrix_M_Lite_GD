/******************************************************************************
*                                                                             *
*                                                                             *
*                                Copyright by                                 *
*                                                                             *
*                              Azoteq (Pty) Ltd                               *
*                          Republic of South Africa                           *
*                                                                             *
*                           Tel: +27(0)21 863 0033                            *
*                          E-mail: info@azoteq.com                            *
*                                                                             *
*=============================================================================*
* @file 	main.c									 					      *
* @brief 	IQS360 Main Tester - Do tests on the IQS360				          *
* @author 	AJ van der Merwe - Azoteq PTY Ltd                             	  *
* @version 	V1.0.0                                                        	  *
* @date 	24/08/2015                                                     	  *
*******************************************************************************/

// User includes
#include "tch_defines.h" 
#include "IQS_Commands.h"
#include "I2C_Comms.h"
#include "I2cDrv.h"
#include "GPIODrv.h"
#include "deviceTypes.h"
#include "IQS5xx.h"

/* I2C Timeouts */
//#define I2C_TIMEOUT			8000
//#define POLL_TIMEOUT		1000

//extern volatile u8 Timer1Expired;
//extern volatile u8 Timer2Expired;
//extern volatile u8 Timer3Expired;
//extern volatile u8 Timer4Expired;

/**
 * @brief	Do a setup of the I2C
 * @param	None
 * @resval	[uint8_t]  success or not
 */
//void Setup_I2C(void)
//{
//	I2C_make_alternate();
//	return SUCCESS;
//}
//Function Prototype


//Tymphany add
static uint8 I2C_Before_Access_Bus(I2C_Device_t* iqs);
static void I2C_After_Access_Bus(I2C_Device_t* iqs);


/*****************************************************************************/
uint8_t I2C_Write (I2C_Device_t* iqs, uint8_t write_address, uint8_t* buffer, uint8_t length_bytes)
/*****************************************************************************/
{
//	//uint16_t i;
//	//uint8_t SendByte;
//	uint8_t res;
//	//uint8_t DeviceReady = 0; // check if device is ready
//
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	I2C_Start_Timeout(I2C_TIMEOUT);
//
//	I2C_xInit(COMMS_FREQ_FAST_I2C);
//
//	/* Get the RDY state of this Device */
//	res = I2C_Check_Ready(iqs);
//
//	/* If device isn't ready, quit */
//	if (res)
//	{
//		return ERR_DEVICE_NOT_READY;
//	}
//
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	I2C_Start_Timeout(I2C_TIMEOUT);
//
//	/* Do I2C Start Transfer */
//	res = I2C_Start(iqs, I2C_Direction_Transmitter);
//
//	/* If device isn't ready, quit */
//	if (res)
//	{
//		return ERR_DEVICE_NOT_READY;
//	}
//
//	/**********************************************************************************************************************/
//	/* Send the register address */
//	I2C_SendData(I2C, write_address);
//
//	/* Test on EV8 and clear it */
//	while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && !Timer4Expired && !DeviceReady)
//	{
//		/* Check For a NACK from the IC and try again */
//		if(I2C_GetFlagStatus(I2C, I2C_FLAG_AF))
//		{
//			Timer4Expired = 0;
//
//			I2C_ClearFlag(I2C, I2C_FLAG_AF);
//			/* Send START condition */
//			I2C_GenerateSTART(I2C, ENABLE);
//			/* Test on EV5 and clear it */
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired)
//			{
//			}
//			if (!Timer4Expired)
//			{
//				/* Send device address */
//				I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Transmitter);
//				while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer4Expired);
//			}

//			DeviceReady = I2C_Event_Mode_Handshake(iqs);
//
//			// Resend the Byte
//			if(!Timer4Expired && DeviceReady)
//			{
//				I2C_SendData(I2C, write_address);
//			}
//			else {
//				return ERR_DEVICE_NOT_READY;
//			}
//		}
//	}
//
//	while (!(I2C->SR1 & 0x0004)&& !Timer4Expired);				//needed a quicker check for the TS50
//
//	if(Timer4Expired)
//	{
//		I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//		I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//		return ERR_DEVICE_NOT_READY;
//	}
//	/**********************************************************************************************************************/
//
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	I2C_Start_Timeout(I2C_TIMEOUT);
//
//	for (i = 0; i < length_bytes; i++)
//	{
//		/* Send the data */
//		SendByte = buffer[i];
//		I2C_SendData(I2C, SendByte);
//
//		/* Test on EV8 and clear it */
//		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && !Timer4Expired);
//
//		while (!(I2C->SR1 & 0x0004)&& !Timer4Expired);				//needed a quicker check for the TS50
//
//		if(Timer4Expired)
//		{
//			I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//			I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//			return ERR_DEVICE_NOT_READY;
//		}
//	}
//
//	/* Stop I2C Transfer */
//	res = I2C_Stop_transfer(iqs);
    
  	uint8_t res;
    uint8 buf[16];
    memset(buf, 0, sizeof(buf));
    ASSERT( (length_bytes+1) <= sizeof(buf));
    buf[0]= write_address;
    memcpy(&buf[1], buffer, length_bytes);

    tI2CMsg i2cMsg=
    {
        .devAddr = iqs->pI2cDrv->pConfig->devAddress,
        .regAddr = NULL,  /* write operation do not handle regAddr */
        .length = length_bytes+1,
        .pMsg = (uint8*)buf
    };
      
    /* Restore stop */
    bool stopTmp= iqs->pI2cDrv->stopEnable;
    ASSERT(iqs->Stop==I2C_Stop || iqs->Stop==I2C_Repeat_Start);
    bool stopIqs= (iqs->Stop==I2C_Stop)?(TRUE):(FALSE);
    I2CDrv_SetStopOperation(iqs->pI2cDrv, stopIqs);   
    
    res= I2C_Before_Access_Bus(iqs);
    if(res==RETURN_OK)
    {
        if (TP_SUCCESS==I2CDrv_MasterWrite(iqs->pI2cDrv, &i2cMsg) )
            res= RETURN_OK;
        else
            res= ERR_DEVICE_NOT_READY;
    }        
    I2C_After_Access_Bus(iqs);
    
    /* Restore stop */
    I2CDrv_SetStopOperation(iqs->pI2cDrv, stopTmp);
    
    return res;
}


///*****************************************************************************/
///**
// * @brief	Start the I2C Transfer and check whether the device needs to be
// * 			polled
// * @param	[I2C_Device_t*] pointer to the I2C Device
// * @resval	[uint8_t] Success or Not
// */
//uint8_t I2C_Start(I2C_Device_t* iqs, uint8_t direction)
///*****************************************************************************/
//{
//	uint8_t res = RETURN_OK;
//	uint16_t PollingTimeOut = 1200;
//	uint8_t DeviceReady = 0;
//
//	/* Only if we are not in Event mode, otherwise we send 2 time the Address
//	 * We are already polling
//	 */
//	if (iqs->Poll == Polling)
//	{
//		/* Instead of keeping the RDY Low for a fixed time, poll */
//		switch(iqs->Poll_Speed)
//		{
//			case Slow:
//				PollingTimeOut = 3000;    // reload value * 1 us (3 ms)
//				break;
//			case Medium:
//				PollingTimeOut = 1200;    // reload value * 1 us (1.2 ms)
//				break;
//			case Fast:
//				PollingTimeOut = 800;     // reload value * 1 us (800 us)
//				break;
//		}
//
//		//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//		I2C_Start_Timeout(POLL_TIMEOUT);
//
//		DeviceReady = 0x00;
//		do
//		{
//			/* Send START condition */
//			I2C_GenerateSTART(I2C, ENABLE);
//			//Set up timer2 for time-out operation (Timer2 preconfigured for 250ns resolution up-counter)
//			TIM2->ARR = (PollingTimeOut);   // reload value * 250ns
//			TIM2->EGR = TIM2->EGR | 0x01;   // set UG bit to force an event to load the new ARR value and reset counter to zero
//			TIM2->SR = TIM2->SR & 0xFE;     // reset update interrupt flag
//			Timer2Expired = 0;
//			TIM2->CR1 = TIM2->CR1 | 0x01;   // start counter2
//			/* Test on EV5 and clear it */
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer2Expired);
//			/* Send device address */
//			if(!Timer2Expired)
//			{
//				I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, direction);
//
//				/* Check the start conditions */
//				/* Transmitter */
//				if (direction == I2C_Direction_Transmitter)
//				{
//					while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer2Expired)
//					{
//						/* Check For a NACK from the IC and try again */
//						if(I2C_GetFlagStatus(I2C, I2C_FLAG_AF))
//						{
//							Timer2Expired = 0;
//							Timer4Expired = 0;
//
//							I2C_ClearFlag(I2C, I2C_FLAG_AF);
//							/* Send START condition */
//							I2C_GenerateSTART(I2C, ENABLE);
//							/* Test on EV5 and clear it */
//							while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired && !Timer2Expired)
//							{
//							}
//							if (!Timer4Expired && !Timer2Expired )
//							{
//								/* Send device address */
//								I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Transmitter);
//								while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer4Expired && !Timer2Expired);
//							}
//							else {
//								return ERR_DEVICE_NOT_READY;
//							}
//						}
//					}
//				}
//				/* Receiver */
//				else {
//					while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer2Expired)
//					{
//						/* Check For a NACK from the IC and try again */
//						if(I2C_GetFlagStatus(I2C, I2C_FLAG_AF))
//						{
//							Timer2Expired = 0;
//							Timer4Expired = 0;
//
//							I2C_ClearFlag(I2C, I2C_FLAG_AF);
//							/* Send START condition */
//							I2C_GenerateSTART(I2C, ENABLE);
//							/* Test on EV5 and clear it */
//							while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired && !Timer2Expired)
//							{
//							}
//							if (!Timer4Expired && !Timer2Expired)
//							{
//								/* Send device address */
//								I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Receiver);
//								while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer4Expired && !Timer2Expired);
//							}
//						}
//					}
//				}
//			}
//			if(!Timer2Expired)
//			{
//				DeviceReady = 0xFF;
//			}
//			else
//			{
//				I2C_xInit(COMMS_FREQ_FAST_I2C);
//			}
//		} while(!DeviceReady && !Timer4Expired);
//	}
//	else
//	{
//		DeviceReady = 0;
//
//		/* Send START condition */
//		I2C_GenerateSTART(I2C, ENABLE);
//		/* Test on EV5 and clear it */
//		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired);
//		/* Send device address */
//		I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, direction);
//
//		/* Test on EV6 and clear it */
//		/* Check the start conditions */
//		/* Transmitter */
//		if (direction == I2C_Direction_Transmitter)
//		{
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer4Expired && !DeviceReady)
//			{
//				/* Check For a NACK from the IC and try again */
//				if(I2C_GetFlagStatus(I2C, I2C_FLAG_AF))
//				{
//					Timer4Expired = 0;
//
//					I2C_ClearFlag(I2C, I2C_FLAG_AF);
////					/* Send START condition */
////					I2C_GenerateSTART(I2C, ENABLE);
////					/* Test on EV5 and clear it */
////					while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired)
////					{
////					}
////					if (!Timer4Expired)
////					{
////						/* Send device address */
////						I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Transmitter);
////						while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer4Expired);
////					}
////					else {
////						return ERR_DEVICE_NOT_READY;
////					}
//
//					/* Do Event mode handshake - new way */
//					DeviceReady = I2C_Event_Mode_Handshake(iqs);
//				}
//			}
//		}
//		/* Receiver */
//		else {
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer4Expired && !DeviceReady)
//			{
//				/* Check For a NACK from the IC and try again */
//				if(I2C_GetFlagStatus(I2C, I2C_FLAG_AF))
//				{
//					Timer4Expired = 0;
//
//					I2C_ClearFlag(I2C, I2C_FLAG_AF);
////					/* Send START condition */
////					I2C_GenerateSTART(I2C, ENABLE);
////					/* Test on EV5 and clear it */
////					while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired)
////					{
////					}
////					if (!Timer4Expired && !Timer2Expired)
////					{
////						/* Send device address */
////						I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Receiver);
////						while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer4Expired);
////					}
//					/* Do Event mode handshake - new way */
//					DeviceReady = I2C_Event_Mode_Handshake(iqs);
//				}
//			}
//		}
//	}
//
//
//	if(Timer4Expired)
//	{
//		I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//		I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//		return ERR_DEVICE_NOT_READY;
//	}
//
//	return res;
//}
//
//
///*****************************************************************************/
///**
// * @brief	Stop the I2C Transfer if necessary and wait for RDY
// * @param	[I2C_Device_t*] pointer to the I2C Device
// * @resval	[uint8_t] Success or Not
// */
//uint8_t I2C_Stop_transfer(I2C_Device_t* iqs)
///*****************************************************************************/
//{
//	uint8_t res = RETURN_OK;
//
//	if (iqs->Stop == I2C_Stop)
//	{
//		/* Send STOP Condition */
//		I2C_GenerateSTOP(I2C, ENABLE);
//		if (iqs->Type == RDY_Active_Low) //I2C wait for READY to go low again
//		{
//			while(!GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin) && !Timer4Expired);
//		}
//		if (iqs->Type == RDY_Active_High) //I2C wait for READY to go high again
//		{
//			while(GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin) && !Timer4Expired);
//		}
//		if(Timer4Expired)
//		{
//			I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//			I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//			return ERR_DEVICE_NOT_READY;
//		}
//	}
//
//	return res;
//}


/*****************************************************************************/
/**
 * @brief	A specific register value, that was given
 * @param	[I2C_Device_t*] pointer to the I2C Device
 * @resval	[uint8_t] Success or Not
 */
uint8_t I2C_Check_Ready(I2C_Device_t* iqs)
/*****************************************************************************/
{
    /* Tymphany add */
    if ( IQS_Get_RDY_WindowsOpen(iqs) )
    {
        return RETURN_OK;
    }
    else 
    {
        /* Do not access I2C when window is closed
         * Note step-by-step debuging is easy to triiger this ASSERT.
         */
        ATOUCH_PRINTF("ERROR: IQS%s access I2C when windows is closed\r\n", (iqs->Type==RDY_Active_High)?"572":"360");
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_RDY_CHECK
        ASSERT(0);
#endif        
        return ERR_DEVICE_NOT_READY;
    }


//	uint8_t res = RETURN_OK;
//
//	if ((iqs->Type == RDY_Active_High) && (iqs->Poll != Polling)) //I2C wait for READY (active high)
//	{
//	  //if we are in event mode, check whether we first need to wake the device before sending command
//	  if (iqs->Event_Mode == Active)
//	  {
//		  if (!GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin))
//		  {
//			  /* Do Event mode handshake - new way */
//			  I2C_Event_Mode_Handshake(iqs);
//		  }
//	  }
//
//	  while(!GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin) && !Timer4Expired);
//	}
//
//	if ((iqs->Type == RDY_Active_Low) && (iqs->Poll != Polling)) //I2C wait for READY (active low)
//	{
//	  //if we are in event mode, check whether we first need to wake the device before sending command
//	  if (iqs->Event_Mode == Active)
//	  {
//		  if (GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin))
//		  {
//			  /* Do Event mode handshake - new way */
//			  I2C_Event_Mode_Handshake(iqs);
//		  }
//	  }
//
//	  while(GPIO_ReadInputDataBit(iqs->RDY_Port, iqs->RDY_Pin) && !Timer4Expired);
//	}
//
//	/* Device isn't ready */
//	if (res)
//	{
//		res = ERR_DEVICE_NOT_READY;
//	}
//
//	return res;
}

/*****************************************************************************/
/**
 * @brief	A specific register value, that was given
 * @param	[uint8_t*] pointer to the USB buffer
		* 		@arg	Buffer[MES_CMD]		:	MES_I2C_READ				->	0x12
		* 		@arg	Buffer[MES_LEN]		:	MES_CONFIG_OW_COMMS_LENGTH 	->	0x00
		* 		@arg	Buffer[MES_DATA]	:	Data Length Bytes			->  0 - 255?
		* 		@arg	Buffer[MES_DATA+1]	:
 * @resval	[uint8_t] Success or Not
 */
uint8_t I2C_Read_Address(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes)
/*****************************************************************************/
{
    //I2C_Stop_t StopValue;
    uint8_t res;

    /* Store the stop command */
    
    //StopValue = iqs->Stop;   

    /* Switch off Stop - Repeat Start */
    //I2C_Set_StopOperation(iqs, I2C_Repeat_Start);
    //iqs->Stop= I2C_Repeat_Start;    
    //res= I2C_Read(iqs, iqs->pI2cDrv, read_address, buffer, length_bytes);
    //iqs->Stop= StopValue;
    
    /* Restore stop */
    //I2C_Set_StopOperation(iqs, StopValue);
    //return res;

//    /* Send the address
//     * @note	We need to send a 0 length, because the address is apart from length
//     */
//    res = I2C_Write(iqs, read_address, buffer, 0);
//
//    if (res != 0)
//    {
//        return res;
//    }
//
//    Delay_us(15);
//
//    /* Restore stop */
//	//iqs->Stop = StopValue;
//
//
//    res = I2C_Read(iqs, read_address, buffer, length_bytes);
    tI2CMsg i2cMsg =
    {
        .devAddr = iqs->pI2cDrv->pConfig->devAddress,
        .regAddr = read_address,
        .length  = length_bytes,
        .pMsg    = (uint8*)buffer
    };
      
    /* Restore stop */
    bool stopTmp= iqs->pI2cDrv->stopEnable;
    ASSERT(iqs->Stop==I2C_Stop || iqs->Stop==I2C_Repeat_Start);
    bool stopIqs= (iqs->Stop==I2C_Stop)?(TRUE):(FALSE);
    I2CDrv_SetStopOperation(iqs->pI2cDrv, stopIqs);   
    
    res= I2C_Before_Access_Bus(iqs);
    if(res==RETURN_OK)
    {
        if(TP_SUCCESS==I2CDrv_MasterRead(iqs->pI2cDrv, &i2cMsg))
            res= RETURN_OK;
        else
            res= ERR_DEVICE_NOT_READY;
    }        
    I2C_After_Access_Bus(iqs);
    
    /* Restore stop */
    I2CDrv_SetStopOperation(iqs->pI2cDrv, stopTmp);
    
    return res;
}


/*****************************************************************************/
//uint8_t I2C_Read(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes)
///*****************************************************************************/
//{
//	uint16_t i;
//	uint8_t ReceivedByte;
//	uint8_t res = RETURN_OK;
//
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	I2C_Start_Timeout(I2C_TIMEOUT);
//
//	/* De-init and re-init I2C Peripheral */
//	I2C_xInit(COMMS_FREQ_FAST_I2C);
//
////	GPIOB->CRL |= 0xFF000000;		//make sure AF_OD

//	if (iqs->Type == RDY_Active_High) //I2C wait for READY (active high)
//	{
//		while(!ReadInput(iqs->RDY_Pin) && !Timer4Expired);
//		if (Timer4Expired && (iqs->Event_Mode == Inactive))
//		{
////			Event_Mode_Ready_Timeout = 1;
//		}
//	}
//	if (iqs->Type == RDY_Active_Low) //I2C wait for READY (active low)
//	{
//		while(ReadInput(iqs->RDY_Pin) && !Timer4Expired);
//		if (Timer4Expired && (iqs->Event_Mode == Inactive))
//		{
////			Event_Mode_Ready_Timeout = 1;
//		}
//	}

//	/* Check if device is ready */
//	res = I2C_Check_Ready(iqs);
//
//	if(Timer4Expired)
//	{
//		I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//		I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//		return ERR_DEVICE_NOT_READY;
//	}
//	else
//	{
//		TIM4->CR1 = TIM4->CR1 & 0xFE;   // stop counter4
//
//		//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//		I2C_Start_Timeout(I2C_TIMEOUT);
//	}

//		if (iqs->Poll == Polling)
//		{
//			switch(iqs->Poll_Speed)
//			{
//				case Slow:
//					PollingTimeOut = 800;     // reload value * 250ns (=100us)
//					break;
//				case Medium:
//					PollingTimeOut = 1200;    // reload value * 250ns (=300us)
//					break;
//				case Fast:
//					PollingTimeOut = 3000;    // reload value * 250ns (=750us)
//					break;
//			}
//			DeviceReady = 0x00;
//			do
//			{
//				/* Send START condition */
//				I2C_GenerateSTART(I2C, ENABLE);
//				//Set up timer2 for time-out operation (Timer2 preconfigured for 250ns resolution up-counter)
//				TIM2->ARR = (PollingTimeOut);   // reload value * 250ns
//				TIM2->EGR = TIM2->EGR | 0x01;   // set UG bit to force an event to load the new ARR value and reset counter to zero
//				TIM2->SR = TIM2->SR & 0xFE;     // reset update interrupt flag
//				Timer2Expired = 0;
//				TIM2->CR1 = TIM2->CR1 | 0x01;   // start counter2
//				/* Test on EV5 and clear it */
//				while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer2Expired);
//				/* Send device address */
//				if(!Timer2Expired)
//				{
//					I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Receiver);
//					while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer2Expired);
//				}
//				if(!Timer2Expired)
//				{
//					DeviceReady = 0xFF;
//				}
//				else
//				{
//					I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//					I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//					I2C_xInit(COMMS_FREQ_FAST_I2C);
//				}
//			} while(!DeviceReady && !Timer4Expired);
//		}
//		else
//		{
//			/* Send START condition */
//			I2C_GenerateSTART(I2C, ENABLE);
//			/* Test on EV5 and clear it */
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer4Expired);
//			/* Send device address */
//
//			I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Receiver);
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && !Timer4Expired);
//		}
//
//	/* Start Transfer as receiver */
//	res = I2C_Start(iqs, I2C_Direction_Receiver);
//
//	if(Timer4Expired || res)
//	{
//		I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//		I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//		return ERR_DEVICE_NOT_READY;
//	}
//
//	I2C_AcknowledgeConfig(I2C, ENABLE);
//
//	for (i = 0; i < length_bytes; i++)
//	{
//
//		I2C_AcknowledgeConfig(I2C, ENABLE);
//
//		if (i == (length_bytes - 1)) //on last byte to read
//		{
//			I2C_AcknowledgeConfig(I2C, DISABLE);
//			if (iqs->Stop == I2C_Stop)
//			{
//				// Send STOP condition on I2C
//				I2C_GenerateSTOP(I2C, ENABLE);
//			}
//		}
//
//		while (!(I2C->SR1 & 0x0040) && !Timer4Expired);				//needed a quicker check for the TS50
//		if ((i == (length_bytes - 1)) && (iqs->Stop == I2C_Repeat_Start)) //on last byte to read with no stop bit set
//		{
//			GPIOB->CRL &= 0x77FFFFFF;		//prevent stm32 to clock wrong clocks after nack without a stop/restart
//			GPIOB->BSRR |= GPIO_Pin_7;
//			GPIOB->BSRR |= GPIO_Pin_6;
//		}
//
//		if(Timer4Expired)
//		{
//			I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//			I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//			return ERR_DEVICE_NOT_READY;
//		}
//
//		// Read the data on I2C2
//		ReceivedByte = I2C_ReceiveData(I2C);
//		buffer[i] = ReceivedByte;
//	}
//
//	/* Enable Acknowledge bit to be ready for another reception */
//	if (i != (length_bytes - 1)) //on last byte to read
//	{
//		I2C_AcknowledgeConfig(I2C, ENABLE);
//	}
//

//	if ((iqs->Stop == I2C_Stop))
//	{
//		/* Send STOP Condition */
//		I2C_GenerateSTOP(I2C, ENABLE);
//		// Delay_us(10); //!!!! confirm how long this delay should be
//		if (iqs->Type == RDY_Active_High) //I2C wait for READY to go low again
//		{
//			while(ReadInput(iqs->RDY_Pin) && !Timer4Expired);
//		}
//		if (iqs->Type == RDY_Active_Low) //I2C wait for READY to go high again
//		{
//			while(!ReadInput(iqs->RDY_Pin) && !Timer4Expired);
//		}
//		if(Timer4Expired)
//		{
//			I2C_DeInit(I2C); //double DeInit necessary to clear busy status flag properly
//			I2C_DeInit(I2C); //see http://www.st.com/mcu/forums-cat-6151-23.html
//			return ERR_DEVICE_NOT_READY;
//		}
//	}
//
//	/* If necessary stop the transfer */
//	res = I2C_Stop_transfer(iqs);
//
//	return res;
//}


/**
 * @brief	Do an Event Mode handshake by pulling RDY Low and Polling
 * @param	[I2C_Device_t] iqs - pointer to the I2C device to poll
 * @resval	[uint8_t] Device Ready - 0 is not ready and >0 is ready
 */
//uint8_t I2C_Event_Mode_Handshake(I2C_Device_t* iqs)
//{
//	uint8_t DeviceReady;
//	uint16_t PollingTimeOut = 800;
//	uint16_t eventHandshakeTimeout = 1000;	// Different I2C Timeout while polling
//
//	/* Pull the RDY line Low/High to indicate start of comms handshake */
//	GPIO_Config_Output(iqs->RDY_Port, iqs->RDY_Pin);
//
//	/* Check if it should be Low Or High */
//	// Active High
//	if (iqs->Type == RDY_Active_High)
//	{
//		GPIO_SetBits(iqs->RDY_Port, iqs->RDY_Pin);
//	}
//	// Active Low
//	else {
//		GPIO_ResetBits(iqs->RDY_Port, iqs->RDY_Pin);
//	}
//
//	/* We are using the RDY, so RDY window will be opened, now close it before polling */
//	iqs->RDY_Window = 0;
//
//	/* Instead of keeping the RDY Low for a fixed time, poll */
//	switch(iqs->Poll_Speed)
//	{
//		case Slow:
//			PollingTimeOut = 3000;    // reload value * 1 us (3 ms)
//			break;
//		case Medium:
//			PollingTimeOut = 1200;    // reload value * 1 us (1.2 ms)
//			break;
//		case Fast:
//			PollingTimeOut = 800;     // reload value * 1 us (800 us)
//			break;
//	}
//
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	I2C_Start_Timeout(eventHandshakeTimeout);
//
//	DeviceReady = 0x00;
//	do
//	{
//		/* Send START condition */
//		I2C_GenerateSTART(I2C, ENABLE);
//		//Set up timer2 for time-out operation (Timer2 preconfigured for 250ns resolution up-counter)
//		TIM2->ARR = (PollingTimeOut);   // reload value * 250ns
//		TIM2->EGR = TIM2->EGR | 0x01;   // set UG bit to force an event to load the new ARR value and reset counter to zero
//		TIM2->SR = TIM2->SR & 0xFE;     // reset update interrupt flag
//		Timer2Expired = 0;
//		TIM2->CR1 = TIM2->CR1 | 0x01;   // start counter2
//		/* Test on EV5 and clear it */
//		while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_MODE_SELECT) && !Timer2Expired);
//		/* Send device address */
//		if(!Timer2Expired)
//		{
//			I2C_Send7bitAddress(I2C, iqs->I2C_Address<<1, I2C_Direction_Transmitter);
//			while(!I2C_CheckEvent(I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && !Timer2Expired);
//		}
//		/* Device did ACK in time */
//		if(!Timer2Expired)
//		{
//			DeviceReady = 0xFF;
//			// Open Window
//			iqs->RDY_Window = 1;
//		}
//		else
//		{
//			/* De-Init and Re-Init I2C Peripheral */
//			I2C_xInit(COMMS_FREQ_FAST_I2C);
//		}
//	} while(!DeviceReady && !Timer4Expired);
//
//	// Active High
//	if (iqs->Type == RDY_Active_High)
//	{
//		GPIO_ResetBits(iqs->RDY_Port, iqs->RDY_Pin);
//	}
//	// Active Low
//	else {
//		GPIO_SetBits(iqs->RDY_Port, iqs->RDY_Pin);
//	}
//
//	/* Now Release the Line */
//	GPIO_Config_Input(iqs->RDY_Port, iqs->RDY_Pin);
//
//	return DeviceReady;
//}

/**
 * @brief	Start the Timeout timer for I2C transfers
 * @param	None
 * @resval	None
 */
//void I2C_Start_Timeout(uint16_t timeout)
//{
//	//Set up timer4 for time-out operation (Timer4 preconfigured for 100us resolution up-counter)
//	TIM4->ARR = (timeout);           	// reload value * 100us (=800ms)
//	TIM4->EGR = TIM4->EGR | 0x01;   	// set UG bit to force an event to load the new ARR value and reset counter to zero
//	TIM4->SR = TIM4->SR & 0xFE;     	// reset update interrupt flag
//	Timer4Expired = 0;
//	TIM4->CR1 = TIM4->CR1 | 0x01;   	// start counter4
//}





/*****************************************************
 *   Tymphany add                                    *
 *****************************************************/

static uint8 I2C_Before_Access_Bus(I2C_Device_t* iqs)
{
    /* Wait RDY, but return after timeeout */
    uint8 res= I2C_Check_Ready(iqs);

    /* Tymphany:
     *   Original sample code have handshake, but Azoteq Anson say it is NOT a good way,
     *   thus we disable handshke, and never verify handshake.
     *   when enable, please careful to verify
     */
#ifdef AZOTEQ_INTERGRATE_TOUCH_HANDSHAKE_ENABLE
    if(iqs->Poll!=Polling) 
    {
        GpioDrv_SetDigitalOut(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
        if (iqs->Type == RDY_Active_High) 
            GpioDrv_SetBit(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
        else
            GpioDrv_ClearBit(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
        
        /* Tymphany: Must add Delay_ms(500) here, or IQS333 will read fail
         * Delay_ms(400) is also fail
         */    
        Delay_ms(500);
    }  
#endif
    
    return res;
}

static void I2C_After_Access_Bus(I2C_Device_t* iqs)
{
#ifdef AZOTEQ_INTERGRATE_TOUCH_HANDSHAKE_ENABLE
    if(iqs->Poll!=Polling)
    {
        GpioDrv_SetDigitalIn(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
    }  
    /*** Tymphany: Must add Delay_ms(100) here, or IQS333 will read fail
     ***/    
     Delay_ms(100);
#endif
}