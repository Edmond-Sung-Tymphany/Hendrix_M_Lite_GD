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
* @file     IQS333.c                                                          *
* @brief    Implementation for the IQS333 "object" specific commands          *
* @author   AJ van der Merwe - Azoteq PTY Ltd                                 *
* @version  V1.0.0                                                            *
* @date     24/08/2015                                                        *
*******************************************************************************/

// User includes
#include "deviceTypes.h"
#include "IQS333.h"
//#include "IQS333_Init.h"
#include "AzIntegTouchKeyDrv.config"
#include "I2C_Comms.h"
#include "tch_defines.h"

// C includes
#include <stdio.h>
#include <string.h>

/* Private Defines */
#define LTA_OFFSET        (uint8_t)(IQS333_NR_OF_ACTIVE_CHANNELS*2)        // Get an LTA Buffer offset
/* Create Memory for the IQS333 object - this could also be done dynamically */
I2C_Device_t iqs333 = {0};

/* Version Info */
uint8_t version_info[2] = {0};
uint8_t system_flags = 0;        // Save the IQS333 System Flags value

/**
 * @brief    Get the pointer to this IQS333 device - everything here could be
 *             done dynamically, but seeing as it is only going to be 1 instance
 *             of this device, we can create it manually
 * @param    None
 * @retval    [I2C_Device_t*] pointer to the I2C_device
 */
I2C_Device_t* IQS333_Get_Device (void)
{
    return (&iqs333);    // return the memory location
}

/**
 * @brief    Setup the IQS333 for this application
 * @param    None
 * @retval    [uint8_t] Status of the setup
 */
uint8_t IQS333_Init(void)
{
    uint8_t res = RETURN_OK;

    /* Setup the IQS333 according to the product and read the version info */

    /* More data can be setup here - Data is obtained from the IQS333_Init.h */

    // We do not want to stop after the transfer these transfers
    iqs333.Stop = I2C_Repeat_Start;
    
    /* Read Version Info */
    res = IQS333_Check_Version();

    /* Error Checking */
    if (res)
    {
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }
    /* Set Targets */
    res = IQS333_Set_Targets();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Set_Targets fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    /* Set Base Values */
    res = IQS333_Set_Base();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Set_Basefail fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    /* Set Thresholds */
    res = IQS333_Set_Thresholds();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Thresholds fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    /* Set Timings */
    res = IQS333_Set_Timings();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Set_Timing fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    // Stop Bit enable
    iqs333.Stop = I2C_Stop;

    /* Set ProxSettings */
    res = IQS333_Set_ProxSettings();

    /* Error Checking */
    if (res)
    {
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    /* Setup Complete */
    /* Now redo ATI if setup was successful */
    iqs333.State = Redo_ATI;

    return res;
}

/*********************         IQS333 Read functions         ***************************/

/**
 * @brief    Read the System Flags byte to especially watch for a reset of the IQS333
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Read_Flags(void)
{
    uint8_t res = RETURN_OK;

    /* Read System Flags */
    res = I2C_Read_Address(&iqs333, FLAGS, (uint8_t*)(&iqs333.Data_Buffer), 1);
    system_flags = iqs333.Data_Buffer[0];    // get the System Flags Value

    /* Return Status */
    return res;
}

/**
 * @brief    Read the necessary data from the IQS333. This is LTA and Counts values for
 *             determining the Delta's for the algortihm
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */

uint8_t IQS333_Read_Data(uint8_t* buffer)
{
    uint8_t res = RETURN_OK;    

    /* Clear Buffers */
    //IQS333_Clear_Buffers();

    // Disable stop bit
    iqs333.Stop = I2C_Repeat_Start;

    // First read System Flags
    res = IQS333_Read_Flags(); //40ms\

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (IQS333 read data fail)\r\n");

        /* When I2C fail, original sample code change state to Set_Active_Channels (re-init).
         * But Azoteq Anson suggest not re-init, because I2C fail is not serious problem,
         * we still follow original sample code
         */
        //return RETURN_OK;
        //iqs333.State = Set_Active_Channels;    // re-init IQS333 <== original sample code
        return ERR_DEVICE_NOT_READY;
    }
    res = I2C_Read_Address(&iqs333, TOUCH_BYTES, (uint8_t*)(&iqs333.Data_Buffer), 1);
    buffer[0] = iqs333.Data_Buffer[0];

    iqs333.Stop = I2C_Stop;

    res = I2C_Read_Address(&iqs333, WHEEL_COORDS, (uint8_t*)(&iqs333.Data_Buffer), 4);
    for(int i=0; i<4; i++)
    {
        buffer[i+1] = iqs333.Data_Buffer[i];
    }

    /* IQS333 has Reset - re-init the device */
    if (system_flags&SHOW_RESET)
    {
        ATOUCH_PRINTF("IQS333 *** SHOW_RESET *** maybe voltage was drop\r\n"); 
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return ERR_IQS333_RESET;    // got a reset
    }

    /* Copy data to the buffer that is returned */
    //IQS333_Copy_Buffer(buffer);
    
    return res;
}

/**
 * @brief    Read and check the IQS333 version info, to ensure correct device
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Check_Version(void)
{
    uint8_t res = RETURN_OK;

    /* Read Version Info */
    res = I2C_Read_Address(&iqs333, VERSION_INFO, (uint8_t*)(version_info), 2);
    if(res==RETURN_OK){
        uint32 product_ver= version_info[BYTE_0];
        uint32 project_ver= version_info[BYTE_1];
        ATOUCH_PRINTF("  (IQS333 version: %d.%d)\r\n", product_ver, project_ver);
        
        /* Wrong version of IQS333 (or incorrect IC) - this is not going to work */
        if (version_info[BYTE_0] != 54 || version_info[BYTE_1] != 2)
        {
           // ASSERT(0);
            res= ERR_NO_DEVICE;
        }
    }
    else {
        ATOUCH_PRINTF("IQS333 Init ==> Version read fail\r\n");
    }

    return res;
}

/**
 * @brief    Read the System Flags byte to check the ATI status of the IQS333
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_ATI_Status(void)
{
    uint8_t res = RETURN_OK;
    uint8_t system_flags = 0;
    uint8_t prox_settings = 0;    // store proxsettings byte 1 to check error bit

    /* Read System Flags */
    // While ATI Busy, check the Error Bit
    
    // Disable stop bit
    iqs333.Stop = I2C_Repeat_Start;

    res = IQS333_Read_Flags();        // Read flags to see if we are still busy

    system_flags = iqs333.Data_Buffer[BYTE_0];
    // set stop bit
    iqs333.Stop = I2C_Stop;
    
    res = I2C_Read_Address(&iqs333, PROXSETTINGS, (uint8_t*)(&iqs333.Data_Buffer), 2);

    prox_settings = iqs333.Data_Buffer[BYTE_1];    // Get byte 1

    /* ATI Failed - Re-Init */
    // TODO Add reset
    if (prox_settings&IQS_ERROR)
    {
        ATOUCH_PRINTF("  (IQS_ERROR)\r\n");
        iqs333.State = Set_Active_Channels;
        res = ERR_IQS333_ATI_FAIL;
    }
    //else if (system_flags&SHOW_RESET)
    //{
    //    ATOUCH_PRINTF("IQS333 *** SHOW_RESET *** maybe voltage was drop\r\n"); 
    //    iqs333.State = Set_Active_Channels;
    //    res = ERR_IQS333_RESET;
    //}
    // ATI Is still busy, check again
    else if (system_flags&ATI_BUSY)
    {
        ATOUCH_PRINTF("  (ATI busy)\r\n");
        iqs333.State = Check_ATI;    // we need to still go through ATI check
        res = RETURN_OK;
    }
    // ATI Done and successful
    else {
        ATOUCH_PRINTF("  (ATI successful)\r\n");
        iqs333.Stop = I2C_Stop;
        iqs333.State = Run;
#ifdef HAS_AZOTEQ_INT_TOUCH_KEY_EVT_MINOTOR 
        IQS_Event_Init(&iqs333); //Tymphany add
#endif        
        res = RETURN_OK;
    }

    /* Return Status */
    return res;
}


/*********************         IQS333 setup functions         ***************************/

/**
 * @brief    Set the Active channels on the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_Active_Channels(void)
{
    uint8_t res = RETURN_OK;

    // We do not want to stap after the transfer these transfers    
    iqs333.Stop = I2C_Repeat_Start;

    /* Read Version Info */
    res = IQS333_Check_Version();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Get Version fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }

    // Enable Stop Bit  
    iqs333.Stop = I2C_Stop;

    /* Set Active Channels: CH0~CH4 */
    iqs333.Data_Buffer[BYTE_0] = IQS333_ACTIVE_CH0;        // Set the Active channels on the IQS333 byte 1
    iqs333.Data_Buffer[BYTE_1] = IQS333_ACTIVE_CH1;        // Set the Active channels on the IQS333 byte 2
    res = I2C_Write(&iqs333, ACTIVE_CHANNELS, (uint8_t*)(&iqs333.Data_Buffer), 2);

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Set_Active_Channels fail)\r\n");
        iqs333.State = Set_Active_Channels;    // re-init IQS333
        return res;
    }
    else
    {
        iqs333.State = Init;    // Init IQS333
    }

    /* Return Status */
    return res;
}


/**
 * @brief    Set the Target values of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_Targets(void)
{
    uint8_t res = RETURN_OK;

    /* Set Targets */
    iqs333.Data_Buffer[BYTE_0] = IQS333_ATI_TARGET_CH0;        // Targets for Prox Channel
    iqs333.Data_Buffer[BYTE_1] = IQS333_ATI_TARGET_CH0_9;        // Targets for Touch Channels
    res = I2C_Write(&iqs333, ATI_TARGETS, (uint8_t*)(&iqs333.Data_Buffer), 2);

    /* Return Status */
    return res;
}

/**
 * @brief    Set the Base values of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_Base(void)
{
    uint8_t res = RETURN_OK;

    /* Set Base Values */
    iqs333.Data_Buffer[BYTE_0] = BASE_75;       // Base for Ch0
    iqs333.Data_Buffer[BYTE_1] = BASE_75;        // Base for Ch1
    iqs333.Data_Buffer[BYTE_2] = BASE_75;        // Base for Ch2
    iqs333.Data_Buffer[BYTE_3] = BASE_75;        // Base for Ch3
    iqs333.Data_Buffer[BYTE_4] = BASE_75;        // Base for Ch4
    iqs333.Data_Buffer[BYTE_5] = BASE_75;        // Base for Ch5
    iqs333.Data_Buffer[BYTE_6] = BASE_75;        // Base for Ch6
    res = I2C_Write(&iqs333, MULTIPLIERS, (uint8_t*)(&iqs333.Data_Buffer), IQS333_NR_OF_ACTIVE_CHANNELS);

    /* Return Status */
    return res;
}


/**
 * @brief    Set the Thresholds of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_Thresholds(void)
{
    uint8_t res = RETURN_OK;

    /* Set Thresholds */
    iqs333.Data_Buffer[BYTE_0] = IQS333_PROX_THRESHOLD;         // Threshold for Prox Channel
    iqs333.Data_Buffer[BYTE_1] = IQS333_TOUCH_THRESHOLD_CH1;    // Threshold for Channel 1
    iqs333.Data_Buffer[BYTE_2] = IQS333_TOUCH_THRESHOLD_CH2;    // Threshold for Channel 2
    iqs333.Data_Buffer[BYTE_3] = IQS333_TOUCH_THRESHOLD_CH3;    // Threshold for Channel 3
    iqs333.Data_Buffer[BYTE_4] = IQS333_TOUCH_THRESHOLD_CH4;    // Threshold for Channel 4
    iqs333.Data_Buffer[BYTE_5] = IQS333_TOUCH_THRESHOLD_CH5;    // Threshold for Channel 4
    iqs333.Data_Buffer[BYTE_6] = IQS333_TOUCH_THRESHOLD_CH6;    // Threshold for Channel 4
    res = I2C_Write(&iqs333,THRESHOLDS, (uint8_t*)(&iqs333.Data_Buffer), IQS333_NR_OF_ACTIVE_CHANNELS);

    /* Return Status */
    return res;
}

/**
 * @brief    Set the ProxSettings of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_ProxSettings(void)
{
    uint8_t res = RETURN_OK;

    /* Set ProxSettings */
    iqs333.Data_Buffer[BYTE_0] = IQS333_PROXSETTINGS0_VAL;                // ProxSettings 0
    iqs333.Data_Buffer[BYTE_1] = IQS333_PROXSETTINGS1_VAL;                // ProxSettings 1
    iqs333.Data_Buffer[BYTE_2] = IQS333_PROXSETTINGS2_VAL;                // ProxSettings 2
    iqs333.Data_Buffer[BYTE_3] = IQS333_PROXSETTINGS3_VAL|ACK_RESET;    // ProxSettings 3 and Clear Reset flag
    iqs333.Data_Buffer[BYTE_4] = IQS333_PROXSETTINGS4_VAL;                // ProxSettings 4
    iqs333.Data_Buffer[BYTE_5] = IQS333_PROXSETTINGS5_VAL;                // ProxSettings 5
    res = I2C_Write(&iqs333, PROXSETTINGS, (uint8_t*)(&iqs333.Data_Buffer), 6);

    /* Return Status */
    return res;
}


/**
 * @brief    Set the Timing values of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_Timings(void)
{
    uint8_t res = RETURN_OK;

    /* Set Timings */
    iqs333.Data_Buffer[BYTE_0] = IQS333_FILTER_HALT;           // Filter Halt Values
    iqs333.Data_Buffer[BYTE_1] = IQS333_POWER_MODE;            // LP Mode
    iqs333.Data_Buffer[BYTE_2] = IQS333_TIMEOUT_PERIOD;        // Timeout Period
    iqs333.Data_Buffer[BYTE_3] = IQS333_CH0_ACF_BETA;          // CH0 ACF Beta
    iqs333.Data_Buffer[BYTE_4] = IQS333_CH0_9_ACF_BET1;        // CH1 - CH9 ACF Beta
    res = I2C_Write(&iqs333, TIMINGS, (uint8_t*)(&iqs333.Data_Buffer), 5);

    /* Return Status */
    return res;
}

/**
 * @brief    Switch the IQS333 Event Mode Active or Inactive
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Set_EventMode(Event_Mode_t event_on_off)
{
    uint8_t res = RETURN_OK;

    /* Event Mode Active or Inactive */

    // Need to Iterate over all the previous values
    iqs333.Data_Buffer[BYTE_0] = IQS333_PROXSETTINGS0_VAL;
    iqs333.Data_Buffer[BYTE_1] = IQS333_PROXSETTINGS1_VAL;

    // Active
    if (event_on_off == Active)
    {
        iqs333.Data_Buffer[BYTE_2] = IQS333_PROXSETTINGS2_VAL|EVENT_MODE;                        // Event Mode Active
    }
    // Inactive
    else {
        iqs333.Data_Buffer[BYTE_2] = (uint8_t)(IQS333_PROXSETTINGS2_VAL & (~EVENT_MODE));        // Event Mode Inactive
    }
    res = I2C_Write(&iqs333, PROXSETTINGS, (uint8_t*)(&iqs333.Data_Buffer), 3);

    iqs333.Event_Mode = event_on_off;    // set the new status

    /* Return Status */
    return res;
}

/**
 * @brief    Set the Target values of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Redo_ATI(void)
{
    uint8_t res = RETURN_OK;

    // set stop bit
    iqs333.Stop = I2C_Stop;

    /* Redo-ATI */
    iqs333.Data_Buffer[BYTE_0] = IQS333_PROXSETTINGS0_VAL|REDO_ATI;        // Redo ATI
    res = I2C_Write(&iqs333, PROXSETTINGS, (uint8_t*)(&iqs333.Data_Buffer), 1);

    // Check ATI Status
    iqs333.State = Check_ATI;

    /* Return Status */
    return res;
}

/**
 * @brief    Set the Target values of the IQS333 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS333_Reseed(void)
{
    uint8_t res = RETURN_OK;

    /* Reseed */
    iqs333.Data_Buffer[BYTE_0] = IQS333_PROXSETTINGS0_VAL|RESEED;        // Reseed Filters
    res = I2C_Write(&iqs333, PROXSETTINGS, (uint8_t*)(&iqs333.Data_Buffer), 1);

    /* Return Status */ 
    return res;
}

/**
 * @brief    Clear the IQS333 Data Buffer
 * @param    None
 * @retval    None
 */
void IQS333_Clear_Buffers(void)
{
    memset((void *)(&iqs333.Data_Buffer), 0, sizeof(iqs333.Data_Buffer));    // Clear memory
}

/**
 * @brief    Copy the IQS333 Data Buffer to the storage buffer for later
 *             processing - this can be a hard coded function
 * @param    None
 * @retval    None
 */
void IQS333_Copy_Buffer(uint8_t* buffer)
{
    memcpy((void*)buffer, (void*)(&iqs333.Data_Buffer[0]), sizeof(uint8_t)*(IQS333_NR_OF_ACTIVE_CHANNELS*4)); // take into account the Counts and LTA
}
