/**
 * @file        NvmDrv_priv.h
 * @brief       
 * @author      
 * @date        
 * @copyright   Tymphany Ltd.
 */
#ifndef NVMDRV_PRIV_H
#define NVMDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "NvmDrv.h"

/* private functions / data */

static bool NvmDrv_WriteWords(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes);
static bool NvmDrv_ReadWords(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes);
static bool NvmDrv_ErasePage(cStorageDrv *me, uint32 addr);
static bool NvmDrv_IsError(void);

#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_PRIV_H */
