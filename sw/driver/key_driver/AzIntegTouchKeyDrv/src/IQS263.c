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
* @file     IQS263.c                                                          *
* @brief    Implementation for the IQS263 "object" specific commands          *
* @author   AJ van der Merwe - Azoteq PTY Ltd                                 *
* @version  V1.0.0                                                            *
* @date     24/08/2015                                                        *
*******************************************************************************/

// User includes
#include "deviceTypes.h"
#include "IQS263.h"
//#include "IQS263_Init.h"
#include "AzIntegTouchKeyDrv.config"
#include "I2C_Comms.h"
#include "tch_defines.h"

// C includes
#include <stdio.h>
#include <string.h>

/* Private Defines */
#define LTA_OFFSET        (uint8_t)(NR_OF_263_ACTIVE_CHANNELS*2)        // Get an LTA Buffer offset

/* Create Memory for the IQS263 object - this could also be done dynamically */
I2C_Device_t iqs263 = {0};

/* Private Variables */

/* Version Info */
uint8_t iqs263_version_info[2] = {0};
uint8_t iqs263_system_flags = 0;        // Save the IQS263 System Flags value

/**
 * @brief    Get the pointer to this IQS263 device - everything here could be
 *             done dynamically, but seeing as it is only going to be 1 instance
 *             of this device, we can create it manually
 * @param    None
 * @retval    [I2C_Device_t*] pointer to the I2C_device
 */
I2C_Device_t* IQS263_Get_Device (void)
{
    return (&iqs263);    // return the memory location
}

/**
 * @brief    Setup the IQS263 for this application
 * @param    None
 * @retval    [uint8_t] Status of the setup
 */

uint8_t IQS263_Init(void)
{
    uint8_t res = RETURN_OK;

    /* Setup the IQS263 according to the product and read the version info */

    /* More data can be setup here - Data is obtained from the IQS263_Init.h */

    // We do not want to stap after the transfer these transfers
    iqs263.Stop = I2C_Repeat_Start;
    
    /* Read Version Info */
    res = IQS263_Check_Version();

    /* Error Checking */
    if (res)
    {    
        ATOUCH_PRINTF("\r\n IQS263_Check_Version fail\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }

    /* Set Targets */
    res = IQS263_Set_Targets();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("\r\n IQS263_Set_Targets fail\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }

    /* Set Base Values */
    res = IQS263_Set_Base();

    /* Error Checking */
    if (res)
    {   
        ATOUCH_PRINTF("\r\n IQS263_Set_Base fail\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }

    /* Set Thresholds */
    res = IQS263_Set_Thresholds();

    /* Error Checking */
    if (res)
    { 
        ATOUCH_PRINTF("\r\n IQS263_Set_Thresholds fail\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }

    // Stop Bit enable
    iqs263.Stop = I2C_Stop;

    /* Set ProxSettings */
    res = IQS263_Set_ProxSettings();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("\r\n IQS263_Set_ProxSettings fail\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }

    /* Setup Complete */
    /* Now redo ATI if setup was successful */
    iqs263.State = Redo_ATI;

    return res;
}

///*********************         IQS263 Read functions         ***************************/
//
///**
// * @brief    Read the System Flags byte to especially watch for a reset of the IQS263
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Read_wheel(uint8_t* buffer)
{
    uint8_t res = RETURN_OK;
    
    iqs263.Stop = I2C_Repeat_Start;
    //IQS263_Clear_Buffers();
    res = I2C_Read_Address(&iqs263, IQS263_TOUCH_BYTES, (uint8_t*)(&iqs263.Data_Buffer), 1);
    buffer[0] = iqs263.Data_Buffer[0];
    
    iqs263.Stop = I2C_Stop;
    res = I2C_Read_Address(&iqs263, IQS263_COORDINATES, (uint8_t*)(&iqs263.Data_Buffer), 1);
    buffer[1] = iqs263.Data_Buffer[0];
    
    return res;

}

uint8_t IQS263_Read_Flags(void)
{
    uint8_t res = RETURN_OK;

    /* Read System Flags */
    res = I2C_Read_Address(&iqs263, IQS263_SYS_FLAGS, (uint8_t*)(&iqs263.Data_Buffer), 1);
    iqs263_system_flags = iqs263.Data_Buffer[0];    // get the System Flags Value

    /* Return Status */
    return res;
}
//
///**
// * @brief    Read the necessary data from the IQS263. This is LTA and Counts values for
// *             determining the Delta's for the algortihm
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */

uint8_t IQS263_Read_Data(uint8_t* buffer)
{
    uint8_t res = RETURN_OK;
    I2C_Stop_t tempStop;    // temp storage for the stop bit control
    uint8_t wheel = 0;
    uint8_t touch = 0;
    uint16_t coordinate = 0;

    /* Clear Buffers */
    IQS263_Clear_Buffers();

    // Get the stop bit state
    tempStop = iqs263.Stop;

    // Disable stop bit
    //I2C_Set_StopOperation(&iqs263, I2C_Repeat_Start);
    iqs263.Stop = I2C_Repeat_Start;

    // First read System Flags
    res = IQS263_Read_Flags(); //40ms\

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (IQS263 read data fail)\r\n");

        /* When I2C fail, original sample code change state to Set_Active_Channels (re-init).
         * But Azoteq Anson suggest not re-init, because I2C fail is not serious problem,
         * we still follow original sample code
         */
        //return RETURN_OK;
        //iqs263.State = Set_Active_Channels;    // re-init IQS263 <== original sample code
        return ERR_DEVICE_NOT_READY;
    }

    /* Read LTA and CS values - Streaming it out */

    // Read Counts
    //res = I2C_Read_Address(&iqs263, COUNTS, (uint8_t*)(&iqs263.Data_Buffer), (uint8_t)(NR_OF_263_ACTIVE_CHANNELS*2));
    IQS263_Clear_Buffers();

    res = I2C_Read_Address(&iqs263, IQS263_COORDINATES, (uint8_t*)(&iqs263.Data_Buffer), 3);
    wheel = iqs263.Data_Buffer[0];
    coordinate = (uint16_t)(iqs263.Data_Buffer[1] | iqs263.Data_Buffer[2] << 8);

    IQS263_Clear_Buffers();    
    res = I2C_Read_Address(&iqs263, IQS263_TOUCH_BYTES, (uint8_t*)(&iqs263.Data_Buffer), 1);
    touch = iqs263.Data_Buffer[0];;
    if(touch != 0)
    {
        ATOUCH_PRINTF("\r\nIQS263: DIDO TOUCH_BYTES: 0x%x\r\n",touch);
    }

    //iqs263.Stop = tempStop;    // Restore Stop Bit
    //iqs263.Stop = I2C_Stop;    // Stop Bit enable
    iqs263.Stop = tempStop;

    // Read LTA
    res = I2C_Read_Address(&iqs263, IQS263_LTA, (uint8_t*)(&iqs263.Data_Buffer[LTA_OFFSET]), (uint8_t)(NR_OF_263_ACTIVE_CHANNELS*2));

    /* IQS263 has Reset - re-init the device */
    if (iqs263_system_flags&IQ263_SHOW_RESET)
    {
        ATOUCH_PRINTF("IQS263 *** SHOW_RESET *** maybe voltage was drop\r\n"); 
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return ERR_IQS263_RESET;    // got a reset
    }

    /* Copy data to the buffer that is returned */
    IQS263_Copy_Buffer(buffer);

    /* Return Status */
    return res;
}

/**
 * @brief    Read and check the IQS263 version info, to ensure correct device
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS263_Check_Version(void)
{
    uint8_t res = RETURN_OK;

    /* Read Version Info */
    res = I2C_Read_Address(&iqs263, IQS263_DEVICE_INFO, (uint8_t*)(iqs263_version_info), 2);
    if(res==RETURN_OK){
        uint32 product_ver= iqs263_version_info[BYTE_0];
        uint32 project_ver= iqs263_version_info[BYTE_1];
        ATOUCH_PRINTF("  (IQS263 version: %d.%d)\r\n", product_ver, project_ver);
        
        /* Wrong version of IQS263 (or incorrect IC) - this is not going to work */
        if (iqs263_version_info[BYTE_0] != 60 || iqs263_version_info[BYTE_1] != 0)
        {
            ASSERT(0);
            res= ERR_NO_DEVICE;
        }
    }
    else {
        ATOUCH_PRINTF("IQS263 Init ==> Version read fail\r\n");
    }

    return res;
}

///**
// * @brief    Read the System Flags byte to check the ATI status of the IQS263
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_ATI_Status(void)
{
    uint8_t res = RETURN_OK;
    uint8_t iqs263_system_flags = 0;
    uint8_t prox_settings = 0;    // store proxsettings byte 1 to check error bit

    /* Read System Flags */
    // While ATI Busy, check the Error Bit    
    iqs263.Stop = I2C_Stop;

    res = IQS263_Read_Flags();        // Read flags to see if we are still busy

    iqs263_system_flags = iqs263.Data_Buffer[BYTE_0];

    if (iqs263_system_flags&IQS263_ATI_ERROR)
    {
        iqs263.State = Set_Active_Channels;
        res = ERR_IQS263_ATI_FAIL;
    }
    //else if (iqs263_system_flags&IQS263_SHOW_RESET)
    //{
    //    ATOUCH_PRINTF("IQS263 *** SHOW_RESET *** maybe voltage was drop\r\n"); 
    //    iqs263.State = Set_Active_Channels;
    //    res = ERR_IQS263_RESET;
    //}
    // ATI Is still busy, check again
    else if (iqs263_system_flags&IQS263_ATI_BUSY)
    {
        iqs263.State = Check_ATI;    // we need to still go through ATI check
        res = RETURN_OK;
    }
    // ATI Done and successful
    else {
        ATOUCH_PRINTF("  (ATI successful)\r\n");
        iqs263.Stop = I2C_Stop;
        iqs263.State = Run;
#ifdef HAS_AZOTEQ_INT_TOUCH_KEY_EVT_MINOTOR 
        IQS_Event_Init(&iqs263); //Tymphany add
#endif        
        res = RETURN_OK;
    }

    /* Return Status */
    return res;
}


/*********************         IQS263 setup functions         ***************************/

/**
 * @brief    Set the Active channels on the IQS263 helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS263_Set_Active_Channels(void)
{
    uint8_t res = RETURN_OK;

    // We do not want to stap after the transfer these transfers    
    iqs263.Stop = I2C_Repeat_Start;

    /* Read Version Info */
    res = IQS263_Check_Version();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Get Version fail)\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }
    
    // Enable Stop Bit  
    iqs263.Stop = I2C_Stop;

    /* Set Active Channels: CH0~CH4 */
    ATOUCH_PRINTF("IQS263_Set_Active_Channels ==> Set Active Channels\r\n");   
    iqs263.Data_Buffer[BYTE_0] = IQS263_ACTIVE_CHS;        // Set the Active channels on the IQS263 byte 1
    res = I2C_Write(&iqs263, IQS263_ACTIVE_CHANNELS, (uint8_t*)(&iqs263.Data_Buffer), 1);

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (Set_Active_Channels fail)\r\n");
        iqs263.State = Set_Active_Channels;    // re-init IQS263
        return res;
    }
    else
    {
        iqs263.State = Init;    // Init IQS263
    }

    /* Return Status */
    return res;
}


///**
// * @brief    Set the Target values of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Set_Targets(void)
{
    uint8_t res = RETURN_OK;

    /* Set Targets */
    iqs263.Data_Buffer[BYTE_0] = IQS263_LOW_POWER;        // Targets for Prox Channel
    iqs263.Data_Buffer[BYTE_1] = IQS263_ATI_TARGET_TOUCH;        // Targets for Touch Channels
    iqs263.Data_Buffer[BYTE_1] = IQS263_ATI_TARGET_PROX;        // Targets for Touch Channels
    res = I2C_Write(&iqs263, IQS263_TIMINGS_AND_TARGETS, (uint8_t*)(&iqs263.Data_Buffer), 3);

    /* Return Status */
    return res;
}

///**
// * @brief    Set the Base values of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Set_Base(void)
{
    uint8_t res = RETURN_OK;

    /* Set Base Values */
    iqs263.Data_Buffer[BYTE_0] = IQS263_MULTIPLIERS_CH0;       // Base for Ch0
    iqs263.Data_Buffer[BYTE_1] = IQS263_MULTIPLIERS_CH1;        // Base for Ch1
    iqs263.Data_Buffer[BYTE_2] = IQS263_MULTIPLIERS_CH2;        // Base for Ch2
    iqs263.Data_Buffer[BYTE_3] = IQS263_MULTIPLIERS_CH3;        // Base for Ch3
    iqs263.Data_Buffer[BYTE_4] = IQS263_BASE_VAL;        // Base for Ch4
    res = I2C_Write(&iqs263, IQS263_MULTIPLIERS, (uint8_t*)(&iqs263.Data_Buffer), 5);

    /* Return Status */
    return res;
}


///**
// * @brief    Set the Thresholds of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Set_Thresholds(void)
{
    uint8_t res = RETURN_OK;

    /* Set Thresholds */
    iqs263.Data_Buffer[BYTE_0] = IQS263_PROX_THRESHOLD;         // Threshold for Prox Channel
    iqs263.Data_Buffer[BYTE_1] = IQS263_TOUCH_THRESHOLD_CH1;    // Threshold for Channel 1
    iqs263.Data_Buffer[BYTE_2] = IQS263_TOUCH_THRESHOLD_CH2;    // Threshold for Channel 2
    iqs263.Data_Buffer[BYTE_3] = IQS263_TOUCH_THRESHOLD_CH3;    // Threshold for Channel 3
    res = I2C_Write(&iqs263, IQS263_THRESHOLDS, (uint8_t*)(&iqs263.Data_Buffer), NR_OF_263_ACTIVE_CHANNELS);

    /* Return Status */
    return res;
}

///**
// * @brief    Set the ProxSettings of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Set_ProxSettings(void)
{
    uint8_t res = RETURN_OK;

    /* Set ProxSettings */
    iqs263.Data_Buffer[BYTE_0] = IQS263_PROXSETTINGS0_VAL;                // ProxSettings 0
    iqs263.Data_Buffer[BYTE_1] = IQS263_PROXSETTINGS1_VAL;                // ProxSettings 1
    iqs263.Data_Buffer[BYTE_2] = IQS263_PROXSETTINGS2_VAL;                // ProxSettings 2
    iqs263.Data_Buffer[BYTE_3] = IQS263_PROXSETTINGS3_VAL;    // ProxSettings 3
    iqs263.Data_Buffer[BYTE_4] = IQS263_EVENT_MASK_VAL;                // EVENT_MASK

    res = I2C_Write(&iqs263, IQS263_PROX_SETTINGS, (uint8_t*)(&iqs263.Data_Buffer), 5);

    /* Return Status */
    return res;
}

///**
// * @brief    Switch the IQS263 Event Mode Active or Inactive
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
//uint8_t IQS263_Set_EventMode(Event_Mode_t event_on_off)
//{
//    uint8_t res = RETURN_OK;
//
//    /* Event Mode Active or Inactive */
//
//    // Need to Iterate over all the previous values
//    iqs263.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL;
//    iqs263.Data_Buffer[BYTE_1] = PROXSETTINGS1_VAL;
//
//    // Active
//    if (event_on_off == Active)
//    {
//        iqs263.Data_Buffer[BYTE_2] = PROXSETTINGS2_VAL|EVENT_MODE;                        // Event Mode Active
//    }
//    // Inactive
//    else {
//        iqs263.Data_Buffer[BYTE_2] = (uint8_t)(PROXSETTINGS2_VAL & (~EVENT_MODE));        // Event Mode Inactive
//    }
//    res = I2C_Write(&iqs263, PROX_SETTINGS, (uint8_t*)(&iqs263.Data_Buffer), 3);
//
//    iqs263.Event_Mode = event_on_off;    // set the new status
//
//    /* Return Status */
//    return res;
//}
//
///**
// * @brief    Set the Target values of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
uint8_t IQS263_Redo_ATI(void)
{
    uint8_t res = RETURN_OK;

    // set stop bit
    iqs263.Stop = I2C_Stop;

    /* Redo-ATI */
    iqs263.Data_Buffer[BYTE_0] = IQS263_PROXSETTINGS0_VAL|IQ263_REDO_ATI;        // Redo ATI
    res = I2C_Write(&iqs263, IQS263_PROX_SETTINGS, (uint8_t*)(&iqs263.Data_Buffer), 1);

    // Check ATI Status
    iqs263.State = Check_ATI;

    /* Return Status */
    return res;
}

///**
// * @brief    Set the Target values of the IQS263 helper function
// * @param    None
// * @retval    [uint8_t] Status of the I2C transfer
// */
//uint8_t IQS263_Reseed(void)
//{
//    uint8_t res = RETURN_OK;
//
//    /* Reseed */
//    iqs263.Data_Buffer[BYTE_0] = PROXSETTINGS0_VAL|RESEED;        // Reseed Filters
//    res = I2C_Write(&iqs263, PROX_SETTINGS, (uint8_t*)(&iqs263.Data_Buffer), 1);
//
//    /* Return Status */ 
//    return res;
//}
//
///**
// * @brief    Clear the IQS263 Data Buffer
// * @param    None
// * @retval    None
// */
void IQS263_Clear_Buffers(void)
{
    memset((void *)(&iqs263.Data_Buffer), 0, sizeof(iqs263.Data_Buffer));    // Clear memory
}
//
///**
// * @brief    Copy the IQS263 Data Buffer to the storage buffer for later
// *             processing - this can be a hard coded function
// * @param    None
// * @retval    None
// */
void IQS263_Copy_Buffer(uint8_t* buffer)
{
    memcpy((void*)buffer, (void*)(&iqs263.Data_Buffer[0]), sizeof(uint8_t)*(NR_OF_263_ACTIVE_CHANNELS*4)); // take into account the Counts and LTA
}
