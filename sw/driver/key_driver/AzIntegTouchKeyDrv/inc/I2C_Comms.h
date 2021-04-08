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
* @date 	02/06/2015                                                     	  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __I2C_COMMS_H
#define __I2C_COMMS_H

// User includes
#include "I2CDrv.h"
#include "tch_defines.h"
#ifdef HAS_AZOTEQ_PRIV
#include "State_Machine_priv.h"
#else
#include "State_Machine.h"
#endif

//Tymphany includes
#include "product.config"  //HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR
#ifdef HAS_AZOTEQ_PRIV
#include "AzIntegTouchKeyDrv_priv.h"
#else
#include "AzIntegTouchKeyDrv.h"
#endif


#include "setting_id.h"


#define COMMS_FREQ_FAST_I2C     400000  // 400 kHz
#define COMMS_FREQ_MED_I2C      100000  // 100 kHz
#define COMMS_FREQ_SLOW_I2C     40000   //  40 kHz

#define IC_BUFFER_SIZE 			80		// Create a data buffer of this size

/** Enum to indicate if this IQS device uses RDY Active High or Low */
typedef enum I2C_RDY{
	RDY_Active_High,
	RDY_Active_Low
}I2C_RDY_t;

/** Enum to indicate whether this device should be polled or not */
typedef enum I2C_Poll {
	Polling,
	Use_RDY
}I2C_Poll_t;

/** Enum to indicate whether a Stop or Repeat Start should be sent */
typedef enum I2C_Stop{
	I2C_Stop,
	I2C_Repeat_Start
}I2C_Stop_t;

/** Enum for indicating whether Event mode is active or inactive */
typedef enum Event_Mode{
	Active,
	Inactive
}Event_Mode_t;

/** Enum for the speed at which the device should be polled */
typedef enum Poll_Speed{
	Slow,
	Medium,
	Fast
}Poll_Speed_t;

/** Enum for the I2C Errors/Status that may occur */
typedef enum I2C_Status {
	Device_not_ready,						// Device is not ready for chatting
	Nack,									// Device Nacked
	Err,									// Some error occured
	Timeout,								// Timeout occured
	Success									// This transfer was successful
}I2C_Status_t;

/** Struct to Define an I2C Device - specifically with reference to IQS devices */
typedef struct {
    I2C_RDY_t Type;             // Should RDY be High or Low?
    I2C_Poll_t Poll;            // Should we Poll this device or use RDY?
    I2C_Stop_t Stop;            // Should a I2C Stop or Repeat Start be sent?
    Event_Mode_t Event_Mode;    // Is this device currently in Event mode or not?
    Poll_Speed_t Poll_Speed;    // Speed at which polling will commence
    volatile uint8_t RDY_Window; // The RDY window for this I2C Device
    State_t State;               // The State that the I2C Device is in
    uint8_t Data_Buffer[IC_BUFFER_SIZE];    // Data buffer to store read data
    uint32_t RDY_StartTime;

    /* Tymphany add */
    cI2CDrv * pI2cDrv;  
    cI2CDrv * pBlI2cDrv;        // only for IQS572
    cGpioDrv * gpioObj;
    eGPIOId   RDY_GpioPin;
    eSettingId settingId; //Setting ID to store device status
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
    volatile int32_t miss_evt_num;
    volatile int32_t total_evt_num;
#endif
}I2C_Device_t;

/* Function Prototypes -------------------------------------*/
//uint8_t Setup_I2C(void);
void Setup_I2C(void);
uint8_t I2C_Write (I2C_Device_t* iqs, uint8_t write_address, uint8_t* buffer, uint8_t length_bytes);
uint8_t I2C_Read_Address(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes);
uint8_t I2C_Read(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes);
uint8_t I2C_Event_Mode_Handshake(I2C_Device_t* iqs);
uint8_t I2C_Check_Ready(I2C_Device_t* iqs);
uint8_t I2C_Start(I2C_Device_t* iqs, uint8_t direction);
uint8_t I2C_Stop_transfer(I2C_Device_t* iqs);
void I2C_Start_Timeout(uint16_t timeout);

/* Tymphany Add */
uint8_t I2C_BL_Read_Address(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes);
uint8_t I2C_BL_Write(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes);

#endif /* __I2C_COMMS_H */

