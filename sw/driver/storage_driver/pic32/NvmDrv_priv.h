/**
 * @file        NvmDrv_priv.h
 * @brief       It's the driver to read/write Non-Volatile Memory (NVM) of Microchip
 * @author      Johnny Fan 
 * @date        2014-03-17
 * @copyright   Tymphany Ltd.
 */
#ifndef NVMDRV_PRIV_H
#define NVMDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

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

/**
* Write some 32bit datas into flash storage
*/
static bool NvmDrv_WriteWords(cStorageDrv *me, uint32 addr, uint8* data, uint32 sizeInBytes);
/**
* Read some 32bit datas from flash storage
*/
static bool NvmDrv_ReadWords(cStorageDrv *me, uint32 addr, uint8* data, uint32 sizeInBytes);
    
static bool NvmDrv_CheckError(cNvmDrv *me);

#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_PRIV_H */
