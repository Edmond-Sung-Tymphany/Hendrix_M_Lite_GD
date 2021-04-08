/**
 * @file      triggers.c
 * @brief     Implements bootloader mode trigger function
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include <stdlib.h>
#include <plib.h>
#include "Triggers.h"
#include "NvmDrv.h"
#include "assert.h"
#include "dbgprint.h"


/*****************************************************************************
 * Function Implemenation                                                    *
 *****************************************************************************/
/********************************************************************
 * Function:   check_triggers()
 *
 * Precondition:
 *
 * Input:      None.
 *
 * Output:     TRUE: If triggered
               FALSE: No trigger
 *
 * Side Effects: None.
 *
 * Overview:   Checks if there is a trigger to enter firmware upgrade mode.
 *             Then clear it
 *
 * Note:       None.
 ********************************************************************/
BOOL check_trigger(void)
{
#ifdef BL_FORCE_UPGRADE
    return TRUE
#else
    uint32 value= NvmDrv_ReadWord(NVM_STORAGE_ADDR_UPGRADE_MODE);
    if(value==1) {
        DBG_PRINT("NVM_STORAGE_ADDR_UPGRADE_MODE is set. Value=0x%08x\r\n", (int)value);
    }
    return (value==1);
#endif
}

void clear_trigger(void)
{
    NVM_STORAGE_VALUE_CLEAR(NVM_STORAGE_ADDR_UPGRADE_MODE);
}
