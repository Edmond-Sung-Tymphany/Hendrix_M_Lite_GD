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
* @file 	IQS_Commands.h								 				      *
* @brief 	Header for commands specific to IQS devices				          *
* @author 	AJ van der Merwe - Azoteq (PTY) Ltd                            	  *
* @version 	V1.0.0                                                        	  *
* @date 	24/08/2015                                                     	  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IQS_COMMANDS_H
#define __IQS_COMMANDS_H

// User includes
#include "I2C_Comms.h"
#include "tch_defines.h"

/* Function Prototypes -------------------------------------*/
#ifndef HAS_AZOTEQ_PRIV
uint8_t IQS_Setup_Devices(void);
#endif
void IQS_RDY_window(I2C_Device_t* iqs);
void IQS_Check_EXTI(void);
uint8_t IQS_Get_RDY_Status(I2C_Device_t* iqs);
bool IQS_Get_RDY_WindowsOpen(I2C_Device_t* iqs);

/* Tymphany add */
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
void IQS_Event_Init(I2C_Device_t* iqs);
void IQS_Event_Consumer(I2C_Device_t* iqs, const int32 evt_num_start_check, const int32 miss_evt_precent_limit, const int32 total_evt_max);
void IQS_Event_Generator(I2C_Device_t* iqs);
#endif

//uint8_t IQS_Power(uint8_t onOff);
//uint8_t IQS_Power_On(void);
//uint8_t IQS_Power_Off(void);


#endif /* __IQS_COMMANDS_H */
