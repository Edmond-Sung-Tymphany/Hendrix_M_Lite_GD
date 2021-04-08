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
* @file 	IQS5xx.c													      *
* @brief 	Implementation for the IQS5xx "object" specific commands	      *
* @author 	AJ van der Merwe - Azoteq PTY Ltd                             	  *
* @version 	V1.0.0                                                        	  *
* @date 	24/08/2015                                                     	  *
*******************************************************************************/

// User includes
#include "IQS5xx.h"
#include "I2C_Comms.h"
#include "tch_defines.h"

#include "product.config"
#ifndef HAS_AZOTEQ_PRIV

#ifdef TOUCH_NOISE_DETECTION
#include "IQS5xx_HEX_Array_5.8.13.14.h" //Note this FW still need to evaluate
#else
#include "IQS5xx_HEX_Array_5.8.13.13.h" //for IQS572 firmware ugprade
#endif

#else
#include "IQS5xx_HEX_Array_priv.h"
#endif
// C includes
#include <stdio.h>
#include <string.h>

//Tymphany Platform
#include "deviceTypes.h"


/* Private Defines */
#define CONTROL_SETTINGS0         0x00    // Default value
// Disable Prox Event, PM Prox Event and Touch event from event mode
//#define CONTROL_SETTINGS1         DIS_PROX_EVENT|DIS_PMPROX_EVENT|DIS_TOUCH_EVENT
#define CONTROL_SETTINGS1         DIS_PROX_EVENT|DIS_PMPROX_EVENT



/* Create Memory for the IQS5xx object - this could also be done dynamically */
I2C_Device_t iqs5xx = {0};

/* Private Variables */

/* Version Info */
uint8_t tp_version_info[10] = {0};

/* Tymphany Add */
uint8 IQS5xx_BL_ReadBootloaderVer(void);
uint8 IQS5XX_BL_ReadBackVerify (void);
uint8 IQS5XX_BL_DumpData (void);
uint8 IQS5xx_BL_JumpToBootloader(void); 
uint8 IQS5XX_BL_PollEnterBootloader(void);
uint8 IQS5XX_BL_CrcVerify(void);
uint8 IQS5XX_BL_WriteFirmware (void);
uint8 IQS5XX_BL_JumpToApp(void);
void  IQS5XX_SetResetPin();
void IQS5XX_CleanResetPin();

/**
 * @brief    Get the pointer to this IQS5xx device - everything here could be
 *             done dynamically, but seeing as it is only going to be 1 instance
 *             of this device, we can create it manually
 * @param    None
 * @retval    [I2C_Device_t*] pointer to the I2C_device
 */
I2C_Device_t* IQS5xx_Get_Device (void)
{
    return (&iqs5xx);    // return the memory location
}


/**
 * @brief    Setup the IQS5xx for this application
 * @param    None
 * @retval    [uint8_t] Status of the setup
 */
uint8_t IQS5xx_Init(void)
{
    uint8_t res = RETURN_OK;

    /* Setup the IQS5xx according to the product and read the version info */
    /* IQS5xx mostly sets itself up, so only minimul setup required at the moment */
    // We do not want to stap after the transfer these transfers    
	iqs5xx.Stop = I2C_Repeat_Start;	// We do not want to stap after the transfer these transfers

    /* Read Version Info */
    ATOUCH_PRINTF("IQS572 Init ==> Check_Version\r\n");
    res = IQS5xx_Check_Version();
    /*
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_IQS572_DFU
    if (res==ERR_DEVICE_NOT_READY)
    {
        uint8 bt_ver_res= IQS5xx_BL_ReadBootloaderVer();
        if(bt_ver_res==RETURN_OK) {
            // IQS572 is executing bootloader, seems previous upgrade is fail,
            // should upgrade again
            ATOUCH_PRINTF("*** IQS572 is on bootloader, seems previuos upgrade is fail, upgrade again ***\r\n");
            return ERR_ON_BOOTLOADER;          
        }
    }
#endif*/
 
    if (res) //ERR_VER_NEED_FW_UPGRADE or ERR_VER_WRONG_PRODUCT or ERR_DEVICE_NOT_READY
    {
        return res;
    }

    /* Set IQS5xx Control Settings */
    ATOUCH_PRINTF("IQS572 Init ==> Set_ControlSettings\r\n");
    res = IQS5xx_Set_ControlSettings();

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (IQS572 Set_ControlSettings fail\r\n");
        iqs5xx.State = Init;    // re-init IQS5xx
        return res;
    }

    // Enable stop bit for last transfer   
	iqs5xx.Stop = I2C_Stop;	// Enable stop bit for last transfer

    /* Set Event Mode Active */
    ATOUCH_PRINTF("IQS572 Init ==> Set_EventMode to Active\r\n");
    res = IQS5xx_Set_EventMode(Active);    // Set event mode active

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("  (IQS572 Set_EventMode to Active fail\r\n");
        iqs5xx.State = Init;    //re-init IQS5xx
        return res;
    }

    /* Now, go to the Run loop */
    iqs5xx.State = Run;

    /* Return the Result */
    return res;
}


/*********************         IQS5xx Read functions         ***************************/

/**
 * @brief    Read the necessary data from the IQS5xx. At the moment this is the
 *             gesture byte, telling the master whether it is a forward, backward,
 *             upward or downward swipe. In future it might be degrees
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS5xx_Read_Data(uint8_t* buffer)
{
    uint8_t res = RETURN_OK;
	I2C_Stop_t tempStop;	// temp storage for the stop bit control
    uint8_t xyInfo[32] = {0};	// temporary storage for XY Info byte, read from IQS5xx

    /* Clear Buffers */
    IQS5xx_Clear_Buffers();

    // Get the stop bit state
	tempStop = iqs5xx.Stop;

    // Enable stop bit
	iqs5xx.Stop = I2C_Stop;	// Enable stop bit

    /* Read XY Info byte */

    // Read Counts
    res = I2C_Read_Address(&iqs5xx, XY_STREAM, (uint8_t*)(&iqs5xx.Data_Buffer), 32);

    /* Error Checking */
    if (res)
    {
        ATOUCH_PRINTF("IQS5XX: Fail to read data, back to init\r\n");
        
        iqs5xx.State = Init;    // re-init IQS5xx
        return res;
    }


    // Restore Stop Bit
	iqs5xx.Stop = tempStop;	// Restore Stop Bit

    /* Copy Data Across */
    // TODO This might become a parameter, so that it can be returned
    memcpy(xyInfo, iqs5xx.Data_Buffer, sizeof(xyInfo));

    /* IQS5xx has Reset - re-init the device */
    if (xyInfo[0]&SHOW_RESET)
    {
        iqs5xx.State = Init;        // re-init IQS333
        ATOUCH_PRINTF("IQS5XX *** SHOW_RESET *** event was loss\r\n");        
        return ERR_IQS5XX_RESET;    // got a reset
    }

    /* Return Buffer */
    memcpy(buffer, iqs5xx.Data_Buffer, sizeof(xyInfo));

    /* Return Status */
    return res;
}

/**
 * @brief    Read and check the IQS5xx application version info, to ensure correct device
 *             We are only going to read the complete version info on startup,
 *             otherwise data transfers become too long
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS5xx_Check_Version(void)
{
    uint8_t res = RETURN_OK;

    /* Read Version Info */
    memset(tp_version_info, 0, sizeof(tp_version_info));
    res = I2C_Read_Address(&iqs5xx, INFO_BYTE, (uint8_t*)(tp_version_info), 6);

    uint32 ver1_prod= 0;
    uint32 ver2_proj= 0;
    uint32 ver3_major= 0;
    uint32 ver4_minor= 0;

    /* TODO: When set  I2C_TIMEOUT_MS to 17, IQS572 will fail on version reading. Set to 20 is ok to read version.
     * On the same time, IQS360 also fail for some command, not sure if IQS572 impact I2C bus after firmware upgrade
     */
    if(res==RETURN_OK) {
        ver1_prod= ((tp_version_info[BYTE_0]<<8)|tp_version_info[BYTE_1]);
        ver2_proj= ((tp_version_info[BYTE_2]<<8)|tp_version_info[BYTE_3]);
        ver3_major= tp_version_info[BYTE_4];
        ver4_minor= tp_version_info[BYTE_5];
        ATOUCH_PRINTF("   (IQS572 Version: %d.%d.%d.%d)\r\n", ver1_prod, ver2_proj, ver3_major, ver4_minor );
    }
    else {
        ATOUCH_PRINTF("   (IQS572 version read fail, firmware may be damaged or bootloader is executing)\r\n");
        return ERR_DEVICE_NOT_READY;
    }
    
    //Version Verify
    if ( ver1_prod!=IQS572_VER1_PROD ) {
        ATOUCH_PRINTF("   (IQS572 product version is wrong, may be I2C error)\r\n");
        
        /* When step-by-step debug, we may access I2C when IQS572 is not ready, 
         * then read terrible version, ex.
         *   (IQS572 Version: 41728.14848.13.1)
         */
        return ERR_VER_WRONG_PRODUCT; 
        //return ERR_NO_DEVICE;  //  <== original sample code
    }

    if (ver2_proj!=IQS572_VER2_PROJ || ver3_major!=IQS572_VER3_MAJOR || ver4_minor!=IQS572_VER4_MINOR )
    {
        ATOUCH_PRINTF("   (IQS572 project/major/minor version is wrong, need firmware upgrade)\r\n");
        
        /* If support firmware upgrade, start firmware upgrade
         * if not support, still keep this firmware
         */
        return ERR_VER_NEED_FW_UPGRADE; 
    }
    
    //Go to here means 4 version numbers are correct
    return res;
}



/*********************         IQS5xx setup functions         ***************************/

/**
 * @brief    Switch the IQS5xx Event Mode Active or Inactive
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS5xx_Set_EventMode(Event_Mode_t event_on_off)
{
    uint8_t res = RETURN_OK;

    /* Event Mode Active or Inactive */

    // Active
    if (event_on_off == Active)
    {
        iqs5xx.Data_Buffer[BYTE_0] = CONTROL_SETTINGS0|IQS5XX_EVENT_MODE;                        // Event Mode Active
    }
    // Inactive
    else {
        iqs5xx.Data_Buffer[BYTE_0] = (uint8_t)(CONTROL_SETTINGS0 & (~IQS5XX_EVENT_MODE));        // Event Mode Inactive
    }

    res = I2C_Write(&iqs5xx, CONTROL_SETTINGS, (uint8_t*)(&iqs5xx.Data_Buffer), 1);

    iqs5xx.Event_Mode = event_on_off;    // set the new status

    /* Return Status */
    return res;
}

/**
 * @brief    Set the Control Settings of the IQS5xx helper function
 * @param    None
 * @retval    [uint8_t] Status of the I2C transfer
 */
uint8_t IQS5xx_Set_ControlSettings(void)
{
    uint8_t res = RETURN_OK;

    /* Set Control Settings */
    iqs5xx.Data_Buffer[BYTE_0] = CONTROL_SETTINGS0|IQS5XX_ACK_RESET;        // Control Settings 0 and Clear Reset flag
    iqs5xx.Data_Buffer[BYTE_1] = CONTROL_SETTINGS1;                            // Control Settings 1
    res = I2C_Write(&iqs5xx, CONTROL_SETTINGS, (uint8_t*)(&iqs5xx.Data_Buffer), 2);

    /* Return Status */
    return res;
}

/**
 * @brief    Clear the IQS5xx Data Buffer
 * @param    None
 * @retval    None
 */
void IQS5xx_Clear_Buffers(void)
{
    memset((void *)(&iqs5xx.Data_Buffer), 0, sizeof(iqs5xx.Data_Buffer));    // Clear memory
}

/**
 * @brief    Copy the IQS5xx Data Buffer to the storage buffer for later
 *             processing - this can be a hard coded function
 * @param    None
 * @retval    None
 */
void IQS5xx_Copy_Buffer(uint8_t* buffer)
{
    memcpy((void*)buffer, (void*)(&iqs5xx.Data_Buffer[0]), sizeof(uint8_t)*2);
}




/********************************************************************
 *             Tymphany add, for firmware upgrade                   *
 ********************************************************************/
  
uint8 data_buffer [IQS5XX_CHECKSUM_SIZE+2]; // hold the i2c data, as well as address

/***************************************************
 *  Bootloader I2C                                 *
 ***************************************************/
uint8_t IQS5XX_BL_I2C_Write(I2C_Device_t* iqs, uint8_t* buffer, uint8_t length_bytes)
{
    ASSERT(iqs->pBlI2cDrv); //IQS333 should not call
    uint8_t res; 
 
    tI2CMsg i2cMsg=
    {
        .devAddr = iqs->pBlI2cDrv->pConfig->devAddress, //0x68
        .regAddr = NULL,  /* write operation do not handle regAddr */
        .length = length_bytes,
        .pMsg = (uint8*)buffer
    };
    
    //Bootlaoder do not wait RDY       
    if (TP_SUCCESS==I2CDrv_MasterWrite(iqs->pBlI2cDrv, &i2cMsg) )
        res= RETURN_OK;
    else
        res= ERR_DEVICE_NOT_READY;
 
    return res;
 }

 
uint8_t IQS5XX_BL_I2C_Read_Address(I2C_Device_t* iqs, uint8_t read_address, uint8_t* buffer, uint8_t length_bytes)
{
    ASSERT(iqs->pBlI2cDrv); //IQS333 should not call
    
    uint8_t res; 
    tI2CMsg i2cMsg =
    {
        .devAddr = iqs->pBlI2cDrv->pConfig->devAddress,
        .regAddr = read_address,
        .length  = length_bytes,
        .pMsg    = (uint8*)buffer
    };
      
    //Bootlaoder use standard, do not check RDY
    if(TP_SUCCESS==I2CDrv_MasterRead(iqs->pBlI2cDrv, &i2cMsg))
        res= RETURN_OK;
    else
        res= ERR_DEVICE_NOT_READY;
       
    return res;
}


uint8_t IQS5XX_BL_I2C_Read_Memory(I2C_Device_t* iqs, uint16_t mem_address, uint8_t* buffer, uint8_t length_bytes)
{
    ASSERT(iqs->pBlI2cDrv); //IQS333 should not call
    ASSERT(length_bytes>=IQS5XX_APP_BLOCK_SIZE);
    ASSERT(buffer);

    const eI2CRegAddLen regAddrLenOrg= iqs->pBlI2cDrv->pConfig->regAddrLen;
    iqs->pBlI2cDrv->pConfig->regAddrLen= REG_LEN_24BITS;
    
    uint8_t res = RETURN_OK;
    tI2CMsg i2cMsg =
    {
        .devAddr = iqs->pBlI2cDrv->pConfig->devAddress,
        .regAddr = (IQS5XX_BL_READ_MEM<<16) | mem_address,
        .length  = length_bytes,
        .pMsg    = (uint8*)buffer
    };
    ASSERT( sizeof(i2cMsg.regAddr)>=4 );
      
    //Bootlaoder use standard I2C, do not check RDY
    if(TP_SUCCESS==I2CDrv_MasterRead(iqs->pBlI2cDrv, &i2cMsg))
        res= RETURN_OK;
    else
        res= ERR_DEVICE_NOT_READY;
 
    iqs->pBlI2cDrv->pConfig->regAddrLen= regAddrLenOrg;
    return res;
}

/*
* The upgrade procedure will have 4 steps.
* Step1: enter 572 bootloader
* Step2: ready 572 app check sum data and app data from 572, compare the data with MCU data
* Step3: upgrade the 572 check sum data and MCU data if Step2 returns a fail
* Step4: check if Step3 OK, if OK, then jump to 572 APP, otherwise restart from Step1
*/
uint8_t IQS5xx_BL_FirmwareUpgrade()
{
    uint8_t res = RETURN_OK;

    //Step1 enter 572 bootloader
    // send the instruction to restart the IQS5xx in bootloader
    res= IQS5XX_BL_PollEnterBootloader();

    if(res!=RETURN_OK) {
        /* Some sample have 572_RST problem and can not enter bootloader. Although 572 firmware may be correct,
         * we muse let factory know this is a bad sample, thus do not allow touch work */
        //iqs5xx.State = Init;
        return res;
    }

    Delay_ms(50);   // create a delay to give the IQS5xx time to reset in bootloader

    /*
    * it is in the bootlooder and we are now able to read the bootloader version
    * this step also make sure that we are in the bootloader
    */
    res= IQS5xx_BL_ReadBootloaderVer();
    if(res!=RETURN_OK)
    {
        ATOUCH_PRINTF("   (IQS572 bootloader version read fail)\r\n");
        return res;
    }

    // Step2
    /*
    * Here we verify 572 check sum data and APP data with the data we had in MCU
    */
    res = IQS5XX_BL_ReadBackVerify();
    if(res != RETURN_OK)
    {
      ATOUCH_PRINTF("572 firmware verification failed\r\n");
      
      //Step3:
      //update 572 firmware.
      res = IQS5XX_BL_WriteFirmware();
      
      //Step4:
      //verify 572 firmware again.
      res = IQS5XX_BL_ReadBackVerify();
      if(res!=RETURN_OK) 
      {
          ATOUCH_PRINTF("(upgrade write fail)\r\n");
          return res;
      }
    }
    else
    {
        ATOUCH_PRINTF("IQS572 DFU:572 firmware verify passed \r\n");
    }

#ifdef IQS572_BOOTLOADER_VERIFY_CHECKSUM
    res= IQS5XX_BL_CrcVerify();
    if(res!=RETURN_OK)
    {
        //ATOUCH_PRINTF("   (CRC check fail)\r\n");
        return res;
    }
#endif /* IQS572_BOOTLOADER_VERIFY_CHECKSUM */

    res= IQS5XX_BL_JumpToApp();
    if(res!=RETURN_OK) {
        ATOUCH_PRINTF("   (jump to app fail)\r\n");
        return res;
    }

    iqs5xx.State = Init;
    return res;
}


/**
 * Read the bootloader version that is on the IQS5xx device. This will indicate
 * a successful entry into the bootloader. The current bootloader version is
 * V2.0 and is received as 0x20, 0x00.
 */
uint8 IQS5xx_BL_ReadBootloaderVer(void)
{  
    ATOUCH_PRINTF("IQS572 DFU: read bootloader version\r\n");
    uint8_t res = IQS5XX_BL_I2C_Read_Address(&iqs5xx, IQS5XX_BL_READ_VER, (uint8_t*)data_buffer, 2);
    Delay_ms(10);
    
    if(res!=RETURN_OK) {
        ATOUCH_PRINTF("    (bootloader version read fail)\r\n");
        return ERR_NO_DEVICE;
    }
    else {
        ATOUCH_PRINTF("    (bootloader version: %d.%d)\r\n",   data_buffer[0], data_buffer[1]);      
        return RETURN_OK;
    }
}

/**
 * If the bootloader couldn't be entered with ACK polling, this command can be
 * sent to force the IQS5xx to restart with the bootloader flag set. Please note
 * that the command, 0xA5 is sent to the APPLICATION and written in register
 * 0xFF. Thus, note that the I2C address is that of the IQS5xx and NOT the
 * bootloader
 */
uint8 IQS5xx_BL_JumpToBootloader(void)
{
    ATOUCH_PRINTF("IQS572 DFU: jump to bootloader\r\n");
    iqs5xx.Data_Buffer[BYTE_0] = 0xA5;        // Control Settings 0 and Clear Reset flag
    uint8_t res = I2C_Write(&iqs5xx, JUMP_TO_BOOTLOADER, (uint8_t*)(&iqs5xx.Data_Buffer), 1);
    
    if(res!=TP_SUCCESS) {
        ATOUCH_PRINTF("    (jump to bootloader fail)\r\n");
        return ERR_NO_DEVICE;
    }

    return RETURN_OK;
}

/**
 * This is a CRC check that the bootloader performs on its firmware. If 0x00 is
 * returned, everything adds up with the firmware. Do this to confirm if the
 * IQS5xx has accepted the firmware
 * @NOTE At the moment this function is not working correctly on the current
 * testing firmware
 * @return
 */
uint8 IQS5XX_BL_CrcVerify(void)
{
    ATOUCH_PRINTF("IQS572 DFU: read crc \r\n");
    
    Delay_ms(50*1000);
    uint8_t res = IQS5XX_BL_I2C_Read_Address(&iqs5xx, IQS5XX_BL_READ_CRC_RESULT, (uint8_t*)data_buffer, 1);

    if(res!=RETURN_OK) {
        ATOUCH_PRINTF("   (fail to read crc)\r\n");
        return ERR_NO_DEVICE;
    }
    
    if(data_buffer[0]!=0)  {
        ATOUCH_PRINTF("   (crc report 1 = MIS-MATCH!!)\r\n");
        return ERR_NO_DEVICE;
    }    
    
    ATOUCH_PRINTF("   (crc report 0 = MATCH)\r\n");
    return RETURN_OK;
}

uint8 IQS5XX_BL_WriteFirmware(void)
{
    uint32 index= 0;
    uint8_t res;
    uint32 i,j;
    uint8 counter = 0; // keep count from where we are
    int address= IQS5XX_APP_START_ADDR;
    
#ifdef IQS572_BOOTLOADER_DUMP_MEM 
    res= IQS5XX_BL_DumpData();
    if(res!=RETURN_OK) {
        ATOUCH_PRINTF("IQS572 DFU: dump memory fail\r\n");
        return ERR_NO_DEVICE;
    }    
#endif

    ATOUCH_PRINTF("IQS572 DFU: write firmware to 0x%08x\r\n", IQS5XX_APP_START_ADDR);
    for (j = 0; j < IQS5XX_APP_WRITE_COUNT; j++)
    {        
        //ATOUCH_PRINTF("IQS572 DFU: write memory page %d / %d \r\n", j+1, IQS5XX_APP_WRITE_COUNT);
        data_buffer[0] = (address & 0xFF00) >> 8; // only MSB of address
        data_buffer[1] = (address & 0x00FF); // only LSB of address
            
        for (i = index; i < (index + IQS5XX_APP_BLOCK_SIZE); i++)
        {
            data_buffer[counter+2] = iqs5xx_hex_array[i];  // copy the hex array to the buffer
            counter++;
        }
        // increase index to the position we are in now - 64 later
        index += counter;
        counter = 0; // reset counter

        //Note when write firmware, register address is 2 bytes and equal to memory adress
        res = IQS5XX_BL_I2C_Write(&iqs5xx, (uint8_t*)&data_buffer[0], IQS5XX_APP_BLOCK_SIZE+2);
        if(res!=RETURN_OK) {
          ATOUCH_PRINTF("IQS572 DFU: write memory fail\r\n");
          return ERR_NO_DEVICE;
        }
    
        /* This delay is necessary to give the bootloader some time to write the
         * 64 byte buffer into memory
         * This delay is only necessary after each block write
         */
        Delay_ms(7); // create a delay to give bootloader some time to write

        address += IQS5XX_APP_BLOCK_SIZE;   // increment the address with [64] bytes
        ATOUCH_PRINTF(".");
    }
    ATOUCH_PRINTF("\r\n");

//#ifdef IQS572_BOOTLOADER_VERIFY_CHECKSUM    
    uint32 addr_crc= IQS5XX_CHECKSUM_START_ADDR;
    data_buffer[0] = (addr_crc & 0xFF00) >> 8; // only MSB of address
    data_buffer[1] = (addr_crc & 0x00FF); // only LSB of address

    ATOUCH_PRINTF("IQS572 DFU: write checksum to 0x%08x\r\n", IQS5XX_CHECKSUM_START_ADDR);
    
    ASSERT(IQS5XX_CHECKSUM_SIZE==IQS5XX_APP_BLOCK_SIZE);
    memcpy(&data_buffer[2], iqs5xx_crc_array, IQS5XX_CHECKSUM_SIZE);
    Delay_ms(10); // create a delay to give bootloader some time to write

    res = IQS5XX_BL_I2C_Write(&iqs5xx, (uint8_t*)&data_buffer[0], IQS5XX_CHECKSUM_SIZE+2);
    if(res!=RETURN_OK) {       
        ATOUCH_PRINTF("  (write checksum fail)\r\n");
        return ERR_NO_DEVICE;
    }        
//#endif

  return RETURN_OK;
}



/**
 * 572 has three parts. bootloader, app checksum and app.
 * This function reads 64 bytes check sum data, have a comparasion
 * with the data had in MCU. Also it will read the app data from 572 and compare
 * the data with the MCU data. 
 * if all comparasion passed, it will return success. otherwise return ERROR
 * @return
 */
uint8 IQS5XX_BL_ReadBackVerify (void)
{
    uint32 i,j;
    uint8 counter = 0; // keep count from where we are    
    uint32 index = 0;          // reset the index - start from the beginning
    int address = IQS5XX_CHECKSUM_START_ADDR;   // restart from the beginning of the firmware

    //Tymphany: we must have delay here, or read command will fail
    Delay_ms(10);
    
    
    /****************************Verify 572 APP checksum data*********************************/
    
    /*
    * TODO:Below crc check is disabled because the last two bytes are always wrong
    * when the system is restarted, we need to ask Azoteq.
    */
//    ATOUCH_PRINTF("IQS572 DFU: read checksum\r\n");
//    memset(data_buffer, 0, ArraySize(data_buffer)*sizeof(uint8));
//    data_buffer[0] = (address & 0xFF00) >> 8; // only MSB of address
//    data_buffer[1] = (address & 0x00FF); // only LSB of address
//    uint8_t res= IQS5XX_BL_I2C_Read_Memory(&iqs5xx, address, &data_buffer[2], IQS5XX_CHECKSUM_SIZE);
//    if(res!=RETURN_OK) {       
//        ATOUCH_PRINTF("IQS572 DFU: read checksum fail\r\n");
//        return ERR_NO_DEVICE;
//    }
//    res = memcmp(&data_buffer[2], iqs5xx_crc_array, IQS5XX_CHECKSUM_SIZE);
//    if(res!=RETURN_OK)
//    {
//        ATOUCH_PRINTF("572 Check sum is wrong\r\n");
//        return ERR_NO_DEVICE;
//    }
//#ifdef IQS572_BOOTLOADER_DUMP_MEM   
//    for(i=0 ; i<4 ; i++)
//    {
//        uint32 addr2= address + i*16;
//        ATOUCH_PRINTF("  MEM 0x%08x: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,   %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,\r\n", 
//                        addr2,
//                        data_buffer[2+i],  data_buffer[3+i],  data_buffer[4+i],  data_buffer[5+i],
//                        data_buffer[6+i],  data_buffer[7+i],  data_buffer[8+i],  data_buffer[9+i],                        
//                        data_buffer[10+i], data_buffer[11+i], data_buffer[12+i], data_buffer[13+i],
//                        data_buffer[14+i], data_buffer[15+i], data_buffer[16+i], data_buffer[17+i]);
//    }        
//    //ATOUCH_PRINTF("\r\n");
//#endif  
    
    /****************************Verify 572 APP data*********************************/
    ATOUCH_PRINTF("IQS572 DFU: read memory\r\n");
    address = IQS5XX_APP_START_ADDR;   // restart from the beginning of the firmware
    for (j = 0; j < IQS5XX_APP_WRITE_COUNT; j ++)
    {

        data_buffer[0] = (address & 0xFF00) >> 8; // only MSB of address
        data_buffer[1] = (address & 0x00FF); // only LSB of address

        /*  After each write, read back and compare to verify the written block */
        //ATOUCH_PRINTF("IQS572 DFU: read memory page %d / %d\r\n", j+1, IQS5XX_APP_WRITE_COUNT);

        //Note when write firmware, register address is 2 bytes and equal to memory adress
        uint8_t res= IQS5XX_BL_I2C_Read_Memory(&iqs5xx, address, &data_buffer[2], IQS5XX_APP_BLOCK_SIZE);
        if(res!=RETURN_OK) {       
          ATOUCH_PRINTF("IQS572 DFU: read memory fail\r\n");
          return ERR_NO_DEVICE;
        }
        
#ifdef IQS572_BOOTLOADER_DUMP_MEM   
        uint32 i;
        for(i=0 ; i<4 ; i++)
        {
            uint32 addr2= address + i*16;
            /*ATOUCH_PRINTF("  MEM 0x%08x: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,   %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,\r\n", 
                        addr2,
                        data_buffer[2+i], data_buffer[3+i],  data_buffer[4+i],  data_buffer[5+i],
                        data_buffer[6+i], data_buffer[7+i],  data_buffer[8+i],  data_buffer[9+i],                        
                        data_buffer[10+i],data_buffer[11+i], data_buffer[12+i], data_buffer[13+i],
                        data_buffer[14+i],data_buffer[15+i], data_buffer[16+i], data_buffer[17+i]);
            */
        }
#endif  /* #ifdef IQS572_BOOTLOADER_DUMP_MEM    */

        ATOUCH_PRINTF(".");
        for (i = index; i < (index + IQS5XX_APP_BLOCK_SIZE); i++)
        {
            if (data_buffer[counter+2] != iqs5xx_hex_array[i])  // copy the hex array to the buffer
                return ERR_NO_DEVICE;   
            counter++;
        }
        
        // increase index to the position we are in now - 64 later
        index += counter;
        counter = 0; // reset counter
        address += IQS5XX_APP_BLOCK_SIZE;   // increment the address with [64] bytes
    }
    ATOUCH_PRINTF("\r\n");
    
    /* Flash lights to indicate readings is done and successful - mainly for debugging */
    Delay_ms(200); // create a delay to give bootloader some time to write
    
    return RETURN_OK;
}




#ifdef IQS572_BOOTLOADER_DUMP_MEM 
uint8 IQS5XX_BL_DumpData (void)
{
    uint32 i,j;
    uint8 counter = 0; // keep count from where we are
    uint32 index = 0;          // reset the index - start from the beginning

    //Tymphany: we must have delay here, or read command will fail
    Delay_ms(1000);

    ATOUCH_PRINTF("IQS572 DFU: read all memory\r\n");
    int address = IQS5XX_CHECKSUM_START_ADDR;   // restart from the beginning of the firmware
    data_buffer[0] = (address & 0xFF00) >> 8; // only MSB of address
    data_buffer[1] = (address & 0x00FF); // only LSB of address
    uint8_t res= IQS5XX_BL_I2C_Read_Memory(&iqs5xx, address, &data_buffer[2], IQS5XX_APP_BLOCK_SIZE);
    if(res!=RETURN_OK) {       
        ATOUCH_PRINTF("IQS572 DFU: read checksum fail\r\n");
        return ERR_NO_DEVICE;
    }
    
    for(i=0 ; i<4 ; i++)
    {
        uint32 addr2= address + i*16;
        /*ATOUCH_PRINTF("  MEM 0x%08x: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,   %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,\r\n", 
                        addr2,
                        data_buffer[2+i],  data_buffer[3+i],  data_buffer[4+i],  data_buffer[5+i],
                        data_buffer[6+i],  data_buffer[7+i],  data_buffer[8+i],  data_buffer[9+i],                        
                        data_buffer[10+i], data_buffer[11+i], data_buffer[12+i], data_buffer[13+i],
                        data_buffer[14+i], data_buffer[15+i], data_buffer[16+i], data_buffer[17+i]);
        */
    }        
    //ATOUCH_PRINTF("\r\n");  
    
    ATOUCH_PRINTF("IQS572 DFU: read memory\r\n");
    address = IQS5XX_APP_START_ADDR;   // restart from the beginning of the firmware
    for (j = 0; j < IQS5XX_APP_WRITE_COUNT; j ++)
    {
        data_buffer[0] = (address & 0xFF00) >> 8; // only MSB of address
        data_buffer[1] = (address & 0x00FF); // only LSB of address

        /*  After each write, read back and compare to verify the written block */
        //ATOUCH_PRINTF("IQS572 DFU: read memory page %d / %d\r\n", j+1, IQS5XX_APP_WRITE_COUNT);

        //Note when write firmware, register address is 2 bytes and equal to memory adress
        uint8_t res= IQS5XX_BL_I2C_Read_Memory(&iqs5xx, address, &data_buffer[2], IQS5XX_APP_BLOCK_SIZE);
        if(res!=RETURN_OK) {       
            ATOUCH_PRINTF("IQS572 DFU: read memory fail\r\n");
            return ERR_NO_DEVICE;
        }
        
        uint32 i;
        for(i=0 ; i<4 ; i++)
        {
            uint32 addr2= address + i*16;
            /*ATOUCH_PRINTF("  MEM 0x%08x: %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,   %02x, %02x, %02x, %02x, %02x, %02x, %02x, %02x,\r\n", 
                        addr2,
                        data_buffer[2+i], data_buffer[3+i],  data_buffer[4+i],  data_buffer[5+i],
                        data_buffer[6+i], data_buffer[7+i],  data_buffer[8+i],  data_buffer[9+i],                        
                        data_buffer[10+i],data_buffer[11+i], data_buffer[12+i], data_buffer[13+i],
                        data_buffer[14+i],data_buffer[15+i], data_buffer[16+i], data_buffer[17+i]);
            */
        } 
        
        ATOUCH_PRINTF(".");
        
        // increase index to the position we are in now - 64 later
        index += counter;
        counter = 0; // reset counter
        address += IQS5XX_APP_BLOCK_SIZE;   // increment the address with [64] bytes
    }

    ATOUCH_PRINTF("\r\n");
    Delay_ms(20); // create a delay to give bootloader some time to write
    
    return RETURN_OK;
}

#endif  /* #ifdef IQS572_BOOTLOADER_DUMP_MEM  */


uint8 IQS5XX_BL_JumpToApp(void)
{
    ATOUCH_PRINTF("IQS572 DFU: jump to app\r\n");
        
    data_buffer[0]= IQS5XX_BL_JUMP_TO_APP;
    uint8_t res = IQS5XX_BL_I2C_Write(&iqs5xx, (uint8_t*)&data_buffer[0], 1);
    if(res!=RETURN_OK) {
        ATOUCH_PRINTF("   (jump to app fail)\r\n");
        return ERR_NO_DEVICE;
    }
        
    return RETURN_OK;
    
}

uint8 IQS5XX_BL_PollEnterBootloader(void)
{
    ATOUCH_PRINTF("IQS572 DFU: IQS5XX_BL_PollEnterBootloader\r\n");
    
    uint32 start_ms = 0;
    uint32 i2c_res= ERR_DEVICE_NOT_READY;

    uint8 pReadBuf[10] = {0};
    const uint32 timeout = 50;  /*ms*/
    const uint32 rst_duration = 30; /*ms*/
    uint8 pollSuccess = 0;
    
    tI2CMsg i2cMsg=
     {
         .devAddr = iqs5xx.pBlI2cDrv->pConfig->devAddress, //0x68
         .regAddr = 0,  /* read register 0 for bootloader version */
         .length = 4,   /* from the datasheet, to enter bootloader, we do polling on the
                           bootloader version. Datasheet says the length is 2, but the
                           CT210 dongle actually reads 4 bytes, so i read 4 bytes here */
         .pMsg = pReadBuf
     };
    /* Store the stop command */
    
    I2C_Stop_t StopValue = iqs5xx.Stop;

    /* Switch off Stop - Repeat Start */
    
    iqs5xx.Stop = I2C_Repeat_Start;	// set stop bit


    IQS5XX_CleanResetPin();
    /* the CT210 dongles do reset for around 21ms, so 30ms here should be enough */
    Delay_ms(rst_duration);
    IQS5XX_SetResetPin();
    
    start_ms= getSysTime();
    /* after reset, wait for a short period of time to let the touch IC boots into bootloader */
    Delay_us(1000);
    //timeout exit
    while(getSysTime() - start_ms < timeout)
    {          
        i2cMsg.devAddr= iqs5xx.pBlI2cDrv->pConfig->devAddress; //0x68

        /* do polling by reading the bootloader version */
        i2c_res= I2CDrv_MasterRead(iqs5xx.pBlI2cDrv, &i2cMsg);

        if(i2c_res==TP_SUCCESS) {
            ATOUCH_PRINTF("IQS572 DFU: Good, enter bootlaoder\r\n");
            pollSuccess = 1;
            break;
        }
    }

    /* restore the original i2c stop settings */
    iqs5xx.Stop = StopValue;	// set stop bit

    if(!pollSuccess)
    {
        /* as now we fail to enter bootloader by polling,
         * reset again to make sure we go back to 572 application code */

        //Set 572 reset pin to low
        IQS5XX_CleanResetPin();
        /* the CT210 dongles do reset for around 21ms, so 30ms here should be enough */
        Delay_ms(rst_duration);
        //Set 572 reset pin to high
        IQS5XX_SetResetPin();
        ATOUCH_PRINTF("IQS572 DFU: Fail to enter bootloader by polling\r\n");
        return ERR_CAN_NOT_ENTER_BL;
    }
    else
    {
        return RETURN_OK;
    }
}

void IQS5XX_SetResetPin()
{
    //PC8
    GpioDrv_SetBit(iqs5xx.gpioObj, GPIO_OUT_TCH_572_RST);
}

void IQS5XX_CleanResetPin()
{
    //PC8
    GpioDrv_ClearBit(iqs5xx.gpioObj, GPIO_OUT_TCH_572_RST);  
}

