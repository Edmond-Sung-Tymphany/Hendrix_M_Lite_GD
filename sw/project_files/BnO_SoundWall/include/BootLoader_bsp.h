/**
 *  @file      BootLoader_bsp.h
 *  @brief     Implemented BSP API for bootloader
 *  @author    Viking Wang
 *  @date      02-Nov-2016
 *  @copyright Tymphany Ltd.
 */

#ifndef __BOOTLOADER_BSP_H__
#define __BOOTLOADER_BSP_H__


void Bl_BlockingDelayMs(uint32_t ms);
void BSP_BlockingDelayMs(uint32_t ms);
void bl_BSP_init(void);

#endif  // __BOOTLOADER_BSP_H__

