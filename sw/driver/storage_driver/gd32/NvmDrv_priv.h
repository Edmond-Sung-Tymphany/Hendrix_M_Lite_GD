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

#ifdef HAS_CBUFFER
#define CBUF_PAGE_SIZE      (1024)

#define CBUF_INVALID_SLOT   (-1)
#define CBUF_MAGIC_WORD     (0xBAFFBA5E)
#define CBUF_MAGIC_WORD_LEN (4)
#define CBUF_STAT_SLOT_LEN  (4)
#define CBUF_CRC16_SLOT_LEN (4)

typedef struct
{
    uint32 pageAddress;
    int32  currentSlot;
    uint16 dataLength;
    uint16 slotNumber;
    uint16 headerLength;
} tCyclicBufferPrivConfig;

typedef struct
{
    uint32 pageAddress;
    uint16 dataLength;
} tCyclicBufferPubConfig;

/* private functions / data */
static bool NvmDrv_WriteBuffer(cStorageDrv *me, uint8 cbufId, uint8* pBuf);
static bool NvmDrv_ReadBuffer(cStorageDrv *me, uint8 cbufId, uint8* pBuf);
#endif
static bool NvmDrv_WriteWords(cStorageDrv *me, uint32 addr, uint8 * pBuf, uint32 sizeInBytes);
static bool NvmDrv_ReadWords(cStorageDrv *me, uint32 addr, uint8 * pBuf, uint32 sizeInBytes);
static bool NvmDrv_ErasePage(cStorageDrv *me, uint32 addr);
static bool NvmDrv_IsError(void);

#ifdef __cplusplus
}
#endif

#endif /* NVMDRV_PRIV_H */
