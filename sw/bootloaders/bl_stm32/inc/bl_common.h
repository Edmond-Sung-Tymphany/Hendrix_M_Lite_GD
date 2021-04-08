/**
 *  @file      bl_common.h
 *  @brief     Implemented the shared API for bootloader
 *  @author    Wesley Lee
 *  @date      30-Nov-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _BL_COMMON_H_
#define _BL_COMMON_H_

#include "commonTypes.h"
#include "fep_addr.h"
#include "UartDrv.h"

typedef struct cDbg
{
    cUartDrv    uartDrv;
    cRingBuf    txBuf;
}cDbg;

uint32 getSysTime();
void bl_BSP_init(void);
void bl_jumpAddr(uint32 address);
uint32 bl_calcChecksum(uint32* pStart, uint32* pEnd);
void bl_jumpAddr(uint32 address);
void bl_setStblStatus(uint32 stbl_status);
eFepStblStatus bl_getStblStatus(void);
void bl_setFirmwareStatus(uint32 fw_status);
eFepFirmwareStatus bl_getFirmwareStatus(void);
void bl_writeVersion(void *addr, uint8 ver1, uint8 ver2, uint8 ver3, uint8 ver4);
char* bl_readVersion(void *addr);


#endif

