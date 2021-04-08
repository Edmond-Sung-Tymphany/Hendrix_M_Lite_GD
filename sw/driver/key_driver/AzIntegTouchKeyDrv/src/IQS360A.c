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
* @file 	IQS360A.c													      *
* @brief 	Implementation for the IQS360A "object" specific commands	      *
* @author 	JD Loy - Azoteq PTY Ltd        		   		                  	  *
* @version 	V1.0.0                                                        	  *
* @date 	10/03/2016                                                     	  *
*******************************************************************************/

// User includes
#include "IQS360A.h"
#include "IQS360A_Init.h"
#include "I2C_Comms.h"
#include "tch_defines.h"

// C includes
#include <stdio.h>
#include <string.h>

/* Private Defines */
#define LTA_OFFSET		(uint8_t)(NR_OF_ACTIVE_CHANNELS*2)		// Get an LTA Buffer offset

/* Create Memory for the IQS360A object - this could also be done dynamically */
I2C_Device_t iqs360a = {0};

/* Private Variables */

/* Version Info */
uint8_t version_info[2] = {0};
uint8_t system_flags[2] = {0};		// Save the IQS360A System Flags value

/**
 * @brief	Get the pointer to this IQS360A device - everything here could be
 * 			done dynamically, but seeing as it is only going to be 1 instance
 * 			of this device, we can create it manually
 * @param	None
 * @retval	[I2C_Device_t*] pointer to the I2C_device
 */
I2C_Device_t* IQS360A_Get_Device (void)
{
	return (&iqs360a);	// return the memory location
}

/**
 * @brief	Setup the IQS360A for this application
 * @param	None
 * @retval	[uint8_t] Status of the setup
 */
uint8_t IQS360A_Init(void)
{
	uint8_t res = RETURN_OK;

	/* Setup the IQS360A according to the product and read the version info */

	/* More data can be setup here - Data is obtained from the IQS360A_Init.h */

	iqs360a.Stop = I2C_Repeat_Start;	// We do not want to stap after the transfer these transfers   


	/* Read Version Info */
	res = IQS360A_Check_Version();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	/* Set Targets */
	res = IQS360A_Set_Targets();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	/* Set Base Values */
	res = IQS360A_Set_Base();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}


	/* Set Thresholds */
	res = IQS360A_Set_Thresholds();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	/* Set Timings */
	res = IQS360A_Set_Timings();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	iqs360a.Stop = I2C_Stop;		// Stop Bit enable

	/* Set ProxSettings */
	res = IQS360A_Set_ProxSettings();

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	/* Setup Complete */
	/* Now redo ATI if setup was successful */
	iqs360a.State = Redo_ATI;

	return res;
}

/********************* 		IQS360A Read functions 		***************************/

/**
 * @brief	Read the System Flags byte to especially watch for a reset of the IQS360A
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Read_Flags(I2C_Stop_t i)
{
	uint8_t res = RETURN_OK;
	I2C_Stop_t tempStop;	// save the current stop bit value

    
	tempStop = iqs360a.Stop;
	iqs360a.Stop = i;
    
    
	/* Read System Flags */
	res = I2C_Read_Address(&iqs360a, FLAGS, (uint8_t*)(&iqs360a.Data_Buffer), 2);

	system_flags[0] = iqs360a.Data_Buffer[0];	// get the System Flags Value
	system_flags[1] = iqs360a.Data_Buffer[1];	// get the System Flags Value

	iqs360a.Stop = tempStop;	// restore stop bit

	/* Return Status */
	return res;
}

/**
 * @brief	Read the necessary data from the IQS360A. This is LTA and Counts values for
 * 			determining the Delta's for the algortihm
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Read_Data(uint8_t* buffer)
{
	uint8_t res = RETURN_OK;
	//I2C_Stop_t tempStop;	// temp storage for the stop bit control

	/* Clear Buffers */
	IQS360A_Clear_Buffers();

	/* Tymphany: do not record temp, just set STOP on end of read data
         */
	// Get the stop bit state
	//tempStop = iqs360a.Stop;

	iqs360a.Stop = I2C_Repeat_Start;	// Disable stop bit

	// First read System Flags
	res = IQS360A_Read_Flags(I2C_Repeat_Start);

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	/* Read LTA and CS values - Streaming it out */

	// Read Counts
	res = I2C_Read_Address(&iqs360a, COUNTS, (uint8_t*)(&iqs360a.Data_Buffer), (uint8_t)(NR_OF_ACTIVE_CHANNELS*2));

    
	/* Tymphany: Before end of read data, IQS360 should set STOP
	 */
	//iqs360a.Stop = tempStop;	// Restore Stop Bit
	iqs360a.Stop = I2C_Stop;		// Stop Bit enable
    

	// Read LTA
	res = I2C_Read_Address(&iqs360a, LTA_REG, (uint8_t*)(&iqs360a.Data_Buffer[LTA_OFFSET]), (uint8_t)(NR_OF_ACTIVE_CHANNELS*2));

	/* IQS360A has Reset - re-init the device */
	if (system_flags[0] & SHOW_RESET)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return ERR_IQS360A_RESET;	// got a reset
	}

	/* Copy data to the buffer that is returned */
	IQS360A_Copy_Buffer(buffer);

	/* Return Status */
	return res;
}

/**
 * @brief	Read and check the IQS360A version info, to ensure correct device
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Check_Version(void)
{
	uint8_t res = RETURN_OK;

	/* Read Version Info */
	res = I2C_Read_Address(&iqs360a, VERSION_INFO, (uint8_t*)(version_info), 2);

	/* Wrong version of IQS360A (or incorrect IC) - this is not going to work */
	if (version_info[BYTE_0] != 55 || version_info[BYTE_1] != 2)
	{
		return ERR_NO_DEVICE;
	}

	return res;
}

/**
 * @brief	Read the System Flags byte to check the ATI status of the IQS360A
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_ATI_Status(void)
{
	uint8_t res = RETURN_OK;
	uint8_t system_flags[2] = {0};
        
        /* Tymphany, remove unused variable, to avoid compile warning */
	//uint8_t prox_settings = 0;	// store proxsettings byte 1 to check error bit

	/* Read System Flags */
	// While ATI Busy, check the Error Bit
	iqs360a.Stop = I2C_Repeat_Start; // Disable stop bit

	res = IQS360A_Read_Flags(I2C_Stop);		// Read flags to see if we are still busy

	system_flags[BYTE_0] = iqs360a.Data_Buffer[BYTE_0];
	system_flags[BYTE_1] = iqs360a.Data_Buffer[BYTE_1];

	iqs360a.Stop = I2C_Stop; 		// set stop bit


	/* ATI Failed - Re-Init */
	// TODO Add reset
	if (system_flags[BYTE_1] & ATI_ERROR)
	{
		iqs360a.State = Set_Active_Channels;
		res = ERR_IQS360A_ATI_FAIL;
	}
	else if (system_flags[BYTE_0] & SHOW_RESET)
	{
		iqs360a.State = Set_Active_Channels;
		res = ERR_IQS360A_RESET;
	}
	// ATI Is still busy, check again
	else if (system_flags[BYTE_0] & ATI_BUSY)
	{
		iqs360a.State = Check_ATI;	// we need to still go through ATI check
		res = RETURN_OK;
	}
	// ATI Done and successful
	else
	{
		iqs360a.Stop = I2C_Stop; 		// set stop bit

                
		iqs360a.State = Run;
		res = RETURN_OK;
	}

	/* Return Status */
	return res;
}


/********************* 		IQS360A setup functions 		***************************/

/**
 * @brief	Set the Active channels on the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_Active_Channels(void)
{
	uint8_t res = RETURN_OK;

	iqs360a.Stop = I2C_Repeat_Start;	// We do not want to stap after the transfer these transfers

	/* Read Version Info */
	res = IQS360A_Check_Version();

	/* Error Checking */
	if (res)
	{
        /* if your function comes to here, please note you can not use break
        *  pointer to debug here becase you can only read the info in ready window
        */
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}

	iqs360a.Stop = I2C_Stop; // Enable Stop Bit


	/* Set Active Channels */
	iqs360a.Data_Buffer[BYTE_0] = ACTIVE_CH0;		// Set the Active channels on the IQS360A byte 1
	iqs360a.Data_Buffer[BYTE_1] = ACTIVE_CH1;		// Set the Active channels on the IQS360A byte 2
	res = I2C_Write(&iqs360a, ACTIVE_CHANNELS, (uint8_t*)(&iqs360a.Data_Buffer), 2);

	/* Error Checking */
	if (res)
	{
		iqs360a.State = Set_Active_Channels;	// re-init IQS360A
		return res;
	}
	else
	{
		iqs360a.State = Init;	// Init IQS360A
	}

	/* Return Status */
	return res;
}


/**
 * @brief	Set the Target values of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_Targets(void)
{
	uint8_t res = RETURN_OK;

	/* Set Targets */
	iqs360a.Data_Buffer[BYTE_0] = ATI_TARGET_CH0;		// Targets for Prox Channel
	iqs360a.Data_Buffer[BYTE_1] = ATI_TARGET_CH0_9;		// Targets for Touch Channels
	res = I2C_Write(&iqs360a, ATI_TARGETS, (uint8_t*)(&iqs360a.Data_Buffer), 2);

	/* Return Status */
	return res;
}

/**
 * @brief	Set the Base values of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_Base(void)
{
	uint8_t res = RETURN_OK;

	/* Set Base Values */
	iqs360a.Data_Buffer[BYTE_0] = BASE_200;		// Base for Ch0
	iqs360a.Data_Buffer[BYTE_1] = BASE_100;		// Base for Ch1
	iqs360a.Data_Buffer[BYTE_2] = BASE_100;		// Base for Ch2
	iqs360a.Data_Buffer[BYTE_3] = BASE_100;		// Base for Ch3
	iqs360a.Data_Buffer[BYTE_4] = BASE_100;		// Base for Ch4
	iqs360a.Data_Buffer[BYTE_5] = BASE_100;		// Base for Ch5
	iqs360a.Data_Buffer[BYTE_6] = BASE_100;		// Base for Ch6
	iqs360a.Data_Buffer[BYTE_7] = BASE_100;		// Base for Ch7
	iqs360a.Data_Buffer[BYTE_8] = BASE_100;		// Base for Ch8
	iqs360a.Data_Buffer[BYTE_9] = BASE_100;		// Base for Ch9
	res = I2C_Write(&iqs360a, MULTIPLIERS, (uint8_t*)(&iqs360a.Data_Buffer), NR_OF_ACTIVE_CHANNELS);

	/* Return Status */
	return res;
}


/**
 * @brief	Set the Thresholds of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_Thresholds(void)
{
	uint8_t res = RETURN_OK;

	/* Set Thresholds */
	iqs360a.Data_Buffer[BYTE_0] = PROX_THRESHOLD;		// Threshold for Prox Channel
	iqs360a.Data_Buffer[BYTE_1] = TOUCH_THRESHOLD_CH1;	// Threshold for Channel 1
	iqs360a.Data_Buffer[BYTE_2] = TOUCH_THRESHOLD_CH2;	// Threshold for Channel 2
	iqs360a.Data_Buffer[BYTE_3] = TOUCH_THRESHOLD_CH3;	// Threshold for Channel 3
	iqs360a.Data_Buffer[BYTE_4] = TOUCH_THRESHOLD_CH4;	// Threshold for Channel 4
	res = I2C_Write(&iqs360a, THRESHOLDS, (uint8_t*)(&iqs360a.Data_Buffer), NR_OF_ACTIVE_CHANNELS);

	/* Return Status */
	return res;
}

/**
 * @brief	Set the ProxSettings of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_ProxSettings(void)
{
	uint8_t res = RETURN_OK;

	/* Set ProxSettings */
	iqs360a.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL;				// ProxSettings 0
	iqs360a.Data_Buffer[BYTE_1] = PROXSETTINGS1_VAL|ACK_RESET;				// ProxSettings 1 and Clear Reset flag
	iqs360a.Data_Buffer[BYTE_2] = PROXSETTINGS2_VAL;				// ProxSettings 2
	iqs360a.Data_Buffer[BYTE_3] = PROXSETTINGS3_VAL;				// ProxSettings 3
	iqs360a.Data_Buffer[BYTE_4] = PROXSETTINGS4_VAL;				// ProxSettings 4
	iqs360a.Data_Buffer[BYTE_5] = PROXSETTINGS5_VAL;				// ProxSettings 5
	res = I2C_Write(&iqs360a, PROXSETTINGS, (uint8_t*)(&iqs360a.Data_Buffer), 6);

	/* Return Status */
	return res;
}


/**
 * @brief	Set the Timing values of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_Timings(void)
{
	uint8_t res = RETURN_OK;

	/* Set Timings */
	iqs360a.Data_Buffer[BYTE_0] = FILTER_HALT;			// Filter Halt Values
	iqs360a.Data_Buffer[BYTE_1] = POWER_MODE;			// LP Mode
	iqs360a.Data_Buffer[BYTE_2] = TIMEOUT_PERIOD;		// Timeout Period
	res = I2C_Write(&iqs360a, TIMINGS, (uint8_t*)(&iqs360a.Data_Buffer), 3);

	/* Return Status */
	return res;
}

/**
 * @brief	Switch the IQS360A Event Mode Active or Inactive
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Set_EventMode(Event_Mode_t event_on_off)
{
	uint8_t res = RETURN_OK;

	/* Event Mode Active or Inactive */

	// Need to Iterate over all the previous values
	iqs360a.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL;
	iqs360a.Data_Buffer[BYTE_1] = PROXSETTINGS1_VAL;

	// Active
	if (event_on_off == Active)
	{
		iqs360a.Data_Buffer[BYTE_2] = PROXSETTINGS2_VAL|EVENT_MODE;						// Event Mode Active
	}
	// Inactive
	else {
		iqs360a.Data_Buffer[BYTE_2] = (uint8_t)(PROXSETTINGS2_VAL & (~EVENT_MODE));		// Event Mode Inactive
	}
	res = I2C_Write(&iqs360a, PROXSETTINGS, (uint8_t*)(&iqs360a.Data_Buffer), 3);

	iqs360a.Event_Mode = event_on_off;	// set the new status

	/* Return Status */
	return res;
}

/**
 * @brief	Set the Target values of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Redo_ATI(void)
{
	uint8_t res = RETURN_OK;

	iqs360a.Stop = I2C_Stop;	// set stop bit

	/* Redo-ATI */
	iqs360a.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL|REDO_ATI;		// Redo ATI
	res = I2C_Write(&iqs360a, PROXSETTINGS, (uint8_t*)(&iqs360a.Data_Buffer), 1);

	// Check ATI Status
	iqs360a.State = Check_ATI;

	/* Return Status */
	return res;
}

/**
 * @brief	Set the Target values of the IQS360A helper function
 * @param	None
 * @retval	[uint8_t] Status of the I2C transfer
 */
uint8_t IQS360A_Reseed(void)
{
	uint8_t res = RETURN_OK;

	/* Reseed */
	iqs360a.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL|RESEED;		// Reseed Filters
	res = I2C_Write(&iqs360a, PROXSETTINGS, (uint8_t*)(&iqs360a.Data_Buffer), 1);

	/* Return Status */
	return res;
}

/**
 * @brief	Clear the IQS360A Data Buffer
 * @param	None
 * @retval	None
 */
void IQS360A_Clear_Buffers(void)
{
	memset((void *)(&iqs360a.Data_Buffer), 0, sizeof(iqs360a.Data_Buffer));	// Clear memory
}

/**
 * @brief	Copy the IQS360A Data Buffer to the storage buffer for later
 * 			processing - this can be a hard coded function
 * @param	None
 * @retval	None
 */
void IQS360A_Copy_Buffer(uint8_t* buffer)
{
	memcpy((void*)buffer, (void*)(&iqs360a.Data_Buffer[0]), sizeof(uint8_t)*(NR_OF_ACTIVE_CHANNELS*4)); // take into account the Counts and LTA
}
