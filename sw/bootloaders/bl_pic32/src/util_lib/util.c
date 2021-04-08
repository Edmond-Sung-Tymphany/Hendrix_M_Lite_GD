/**
 * @file      util.c
 * @brief     The util tools for Polk CamdenSquare bootloader
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "hwsetup.h"
#include "NvmDrv.h"
#include "dbgprint.h" //DBG_PRINT






/*****************************************************************************
 * Function Implement                                                        *
 *****************************************************************************/
void reboot(void)
{
    /* When MCU receive reboot commend, SAM is not ready to reboot accually.
     * Reboot directly will cause SAM v1.4.46 fall into "wifi unconfigure mode".
     * Thus we need some delay here. 
     * The 4s delay will reboot SAM while SAM just complet ROM bootloader and start the uboot
     */
    int delay_sec= 4;
    DBG_PRINT("delay %d seconds before reboot\r\n", delay_sec); //must print before bsp_destroy()
    TimerUtil_delay_us(delay_sec*1000*1000); //2sec
    DBG_PRINT("reboot\r\n\r\n\r\n\r\n", delay_sec); //must print before bsp_destroy()

    sam_destroy(); //destroy SAM if it was initialized
    bsp_destroy();
    SoftReset();
}


/********************************************************************
* Function:     JumpToApp()
*
* Precondition:
*
* Input:         None.
*
* Output:
*
* Side Effects:  No return from here.
*
* Overview:      Jumps to application.
*
* Note:           None.
********************************************************************/
void JumpToApp(void)
{
    sam_destroy(); //destroy SAM if it was initialized
    bsp_destroy();
    
    void (*fptr)(void);
    fptr = (void (*)(void))USER_APP_RESET_ADDRESS;
    fptr();
}


/********************************************************************
* Function:     ValidAppWrong()
*
* Precondition:
*
* Input:         None.
*
* Output:        TRUE: If application is valid.
*
* Side Effects:  None.
*
* Overview:      Logic: Check application vector has
                 some value other than "0xFFFFFFFF"
*
* Note:           None.
********************************************************************/
BOOL ValidAppWrong(void)
{
    DWORD *AppPtr= (DWORD *)USER_APP_RESET_ADDRESS;
    uint32 flag_flash_writing= NvmDrv_ReadWord(NVM_STORAGE_ADDR_FLASH_WRITING);
    BOOL ret= TRUE;

    if( flag_flash_writing==1 ) {
        DBG_PRINT("App Flash is wrong (flag_flash_writing:0x%x)\r\n", flag_flash_writing);
    }
    else if(*AppPtr==0xFFFFFFFF) {
        DBG_PRINT("App Flash is wrong (First byte of App:0x%08x)\r\n", (*AppPtr));        
    }
    else {
        //DBG_PRINT("App Flash is OK\r\n");
        ret= FALSE;
    }
    return ret;
}

/*********************End of File************************************/



