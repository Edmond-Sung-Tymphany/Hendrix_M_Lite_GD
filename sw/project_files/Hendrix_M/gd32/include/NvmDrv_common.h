/**
 * @file        NvmDrv_common.h
 * @brief       The common parameter of NVM operation for application and bootloader
 * @author      Gavin Lee 
 * @date        2014-03-28
 * @copyright  Tymphany Ltd.
 */

#ifndef NVMDRV_COMMON_H
#define NVMDRV_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#define NVM_STORE_BYTE_SIZE	4096

//Must include NvmDrv.common.h before
// allocates "nvm_data", aligned on erasable page boundary
#define NVM_STORAGE_ADDR (PROGRAM_FLASH_END_ADRESS+1-NVM_STORE_BYTE_SIZE)

#define NVM_STORAGE_VALUE_CHECK(val) ((val)==0 || (val)==1 || (val)==0xFFFFFFFF)
#define NVM_STORAGE_VALUE_CLEAR(addr) NvmDrv_WriteWord(addr, 0x0)
#define NVM_STORAGE_VALUE_SET(addr) NvmDrv_WriteWord(addr, 0x1)

    /*
__inline uint32 NVM_STORAGE_VALUE_GET(uint32 addr) {
    uint32 value= 0;
    uint8 ret= NvmDrv_ReadWord(addr, &value);
    return (value && ret);
}*/

/* Index to store in NVM memory 
 * NOTE: Both bootloader and application have this file. Plese synchronize them
 *    Bootloader:  \Polk_AllPlay\PolkCamdenSquare_bootloaders\src\Driver\NvmDrv.h
 *    Application: \Polk_AllPlay\PolkCamdenSquare\src\nvm_lib\NvmDrv.h
 *
 */
typedef enum {
    /* Application will notify bootloader to enter upgrade firmware mode
     * when receive UPDATE commend from SAM
     */
    NVM_STORAGE_ADDR_UPGRADE_MODE  = 0,

   /*Indicate the bootloadr is wrirring the application region.
    * When you see this flag==1, the application firmware may not complete and can not boot
    */
    NVM_STORAGE_ADDR_FLASH_WRITING = 1*sizeof(uint32),

   /* While complete upgrade and reboot, application should not wait for power key
    */
    NVM_STORAGE_ADDR_IGNORE_PWR_KEY = 2*sizeof(uint32),

   /* The accumulated number of upgrade times
    * It will keep after MCU upgrade (bootloader will read first, erase flash, then write back)
    */
    NVM_STORAGE_ADDR_UPGRADE_TIMES = 3*sizeof(uint32),

   /* The code of system exception.
    * And the code of exception can be defined according to the request of particular project.
    */
    NVM_STORAGE_ADDR_EXCEPTION_CODE = 4*sizeof(uint32)
}NVM_STORAGE_ADDR_TYPE;

#define Nvm_StoreExceptionCode(x) {if(x) NvmDrv_WriteWord(NVM_STORAGE_ADDR_EXCEPTION_CODE, x);}


#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_COMMON_H */

