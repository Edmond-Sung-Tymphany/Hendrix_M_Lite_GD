/*********************************************************************
 *
 *                  PIC32 Boot Loader
 *
 *********************************************************************
 * FileName:        NVMem.c
 * Dependencies:
 * Processor:       PIC32
 *
 * Compiler:        MPLAB C32
 *                  MPLAB IDE
 * Company:         Microchip Technology, Inc.
 *
 * Software License Agreement
 *
 * The software supplied herewith by Microchip Technology Incorporated
 * (the �Company? for its PIC32 Microcontroller is intended
 * and supplied to you, the Company�s customer, for use solely and
 * exclusively on Microchip PIC32 Microcontroller products.
 * The software is owned by the Company and/or its supplier, and is
 * protected under applicable copyright laws. All rights are reserved.
 * Any use in violation of the foregoing restrictions may subject the
 * user to criminal sanctions under applicable laws, as well as to
 * civil liability for the breach of the terms and conditions of this
 * license.
 *
 * THIS SOFTWARE IS PROVIDED IN AN �AS IS?CONDITION. NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 * TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 * IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *
 * $Id:  $
 * $Name: $
 *
 **********************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include <plib.h>
#include <GenericTypeDefs.h>
#include "HardwareProfile_flash.h"
#include "TimerUtil.h"
#include "NVMem.h"
#include "assert.h"
#include "HardwareProfile.h"
#include "NvmDrv.h"
#include "dbgprint.h"


// Erase Application
// Output: '0' if operation completed successfully.
uint32 NVMemEraseApplication()
{
    uint32 ret= 0, i;
    uint32 upgrade_times   = NvmDrv_ReadWord(NVM_STORAGE_ADDR_UPGRADE_TIMES);
    uint32 wdog_reset_times= NvmDrv_ReadWord(NVM_STORAGE_ADDR_WDOG_RESET_TIMES);    
    DBG_PRINT("[%s] Erase Flash 0x%08x~0x%08x (page size is %dbytes)\r\n", __func__, APP_FLASH_BASE_ADDRESS, APP_FLASH_END_ADDRESS, FLASH_PAGE_SIZE);

    //If the previous erase is interrupted by pressing REBOOT key, we should claer error flag
    if( NVMIsError() ) {
        NVMClearError();
        DBG_PRINT("*** WARNING: Flash operation had error before ***\r\n", __func__);
    }

    //This loop spent 200ms to earse 50 pages
    CHECK_PROGRAM_FLASH_ADDR(); //Chekc if PROGRAM_FLASH_SIZE is metch this chip
    for( i=0; i < ((APP_FLASH_END_ADDRESS - APP_FLASH_BASE_ADDRESS + 1)/FLASH_PAGE_SIZE); i++ )
    {
        void *addr= (void*)APP_FLASH_BASE_ADDRESS + (i*FLASH_PAGE_SIZE);
        //DBG_PRINT("[%s] Erase address 0x%08x~0x%08x (page %d)\r\n", __func__, addr, addr+FLASH_PAGE_SIZE-1, i);
        ret= NVMErasePage( addr ); //spent 3~5ms per page, average 4ms
        if(ret!=0) {
            NVMClearError();
            DBG_PRINT("*** ERROR: Erase Fail ***\r\n", __func__);
            assert(0);
            break;
        }
    }
    NvmDrv_WriteWord(NVM_STORAGE_ADDR_UPGRADE_TIMES,    upgrade_times);
    NvmDrv_WriteWord(NVM_STORAGE_ADDR_WDOG_RESET_TIMES, wdog_reset_times);
    DBG_PRINT("[%s] Erase Finish\r\n", __func__);
    return ret;
}



/***********************End of File*************************************************************/
