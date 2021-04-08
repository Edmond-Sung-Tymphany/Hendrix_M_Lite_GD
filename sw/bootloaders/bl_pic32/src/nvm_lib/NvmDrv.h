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

#include "Bootloader.h"
#include "commonTypes.h"
#include "assert.h"
#include "NvmDrv_common.h"

/**
* Write a 32bit data into flash storage
* @param[in]    address   the address where the data is written at
* @param[in]    wData     the data to be written
* @param[out]   BOOL      reture if the operation is successful
*/
BOOL NvmDrv_WriteWord(uint32 address, uint32 wData);

/**
* Write some 32bit datas into flash storage
* @param[in]    address         the starting address where the data is written at
* @param[in]    pWriteData      the pointer to the data to be written
* @param[in]    dataSizeInByte  the size of data to be written, in byes
* @param[out]   BOOL            reture if the operation is successful
*/
BOOL NvmDrv_WriteWords(uint32 address, uint32* pWriteData, uint32 dataSizeInByte);

/**
* Read a 32bit data from flash storage
* @param[in]    address         the address where the data is read
* @param[in]    pReadData       the pointer to the data that store the reading data
* @param[out]   BOOL            reture if the operation is successful
*/
uint32 NvmDrv_ReadWord(uint32 address);

/**
* Read some 32bit datas from flash storage
* @param[in]    address         the starting address where the data is read
* @param[in]    pReadData       he pointer to the data that store the reading data
* @param[in]    dataSizeInByte  the size of data to be read, in byes
* @param[out]   BOOL            reture if the operation is successful
*/
BOOL NvmDrv_ReadWords(uint32 address, uint32* pReadData, uint32 dataSizeInByte);

/**
* Erase all the nvm flash storage, whose size is defined by NVM_STORE_BYTE_SIZE
*/
BOOL NvmDrv_EraseAll();


#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_H */

