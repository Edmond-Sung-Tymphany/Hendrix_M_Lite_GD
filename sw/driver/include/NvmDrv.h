/**
 * @file        NvmDrv.h
 * @brief       It's the driver to read/write Non-Volatile Memory (NVM) of Microchip
 * @author      Johnny Fan 
 * @date        2014-03-17
 * @copyright   Tymphany Ltd.
 */

#ifndef NVMDRV_H
#define NVMDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "storageDrv.h"
#include "attachedDevices.h"
#include "NvmDrv_common.h" //NVM_STORE_BYTE_SIZE


SUBCLASS(cNvmDrv,cStorageDrv)
    /* private data */
    uint32  currAddr;
METHODS
    /* public functions */
void NvmDrv_Ctor(cNvmDrv *me);
void NvmDrv_Xtor(cNvmDrv *me);

END_CLASS

/***************************************************************************
 * Single word read/write, just keep them for bootloader usage for now
 **************************************************************************/
bool NvmDrv_WriteWord(uint32 address, uint32 wData);
bool NvmDrv_ReadWord(uint32 address, uint32* pReadData);

/**
* Erase all the nvm flash storage, whose size is defined by NVM_STORE_BYTE_SIZE
*/
BOOL NvmDrv_EraseAll(cNvmDrv *me);
#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_H */

