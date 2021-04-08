/**
 *  @file      BootLoader_common.h
 *  @brief     Implemented the shared API for bootloader
 *  @author    Viking Wang
 *  @date      02-Nov-2016
 *  @copyright Tymphany Ltd.
 */

#ifndef __BOOTLOADER_COMMON_H__
#define __BOOTLOADER_COMMON_H__

#define ERR_MSG_CHECKSUM        0x01
#define ERR_MSG_SYSTEM_FAULT    0x02
#define ERR_MSG_ENTER_DFU       0x03

void bl_jumpAddr(uint32 address);
uint32 bl_calcChecksum(uint32* pStart, uint32* pEnd);
void BL_error_msg(uint32_t err_msg_id);

#endif  // __BOOTLOADER_COMMON_H__

