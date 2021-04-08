/**
 * @file        NvmDrv_priv.h
 * @brief       It's the driver to read/write Non-Volatile Memory (NVM) of Microchip
 * @author      Johnny Fan 
 * @date        2014-03-17
 * @copyright   Tymphany Ltd.
 */
#ifndef NVM_DRIVER_PRIVATE_H
#define NVM_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "BootLoader.h" //each file must include Bootloader.h
#include <p32xxxx.h>
#include <stdlib.h>
#include <plib.h>       //Include to use PIC32 peripheral libraries
#include "NvmDrv.h"

/* private functions / data */
 /* *******************************************************************************
  * allocate the memory, below is the meaning of attribute
  * aligned(BYTE_PAGE_SIZE): alighed to the indicated boundary so that we can erase a page
  * space(prog): indicate that it is located in flash
  * section(".nvm"): indicate the sention name
  ********************************************************************************/
#define NVM_ALLOCATE(name,bytes) volatile uint8 name[(bytes)] \
    __attribute__((aligned(BYTE_PAGE_SIZE),space(prog),section(".nvm"))) = \
    { [0 ... (bytes)-1] = 0xFF }

#define NVM_ALLOCATE_ADDR(addr,name,bytes) volatile uint8 name[(bytes)] \
    __attribute__((address(addr),aligned(BYTE_PAGE_SIZE),space(prog),section(".nvm"))) = \
    { [0 ... (bytes)-1] = 0xFF }
    
static BOOL NvmDrv_CheckError();

#ifdef __cplusplus
}
#endif

#endif /* NVM_DRIVER_PRIVATE_H */