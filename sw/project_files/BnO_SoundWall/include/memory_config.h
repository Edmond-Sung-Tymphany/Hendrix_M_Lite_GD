/**
 *  @file      memory_config.h
 *  @brief     The list of flash memory address.
 *  @author    Viking Wang  
 *  @date      02-Nov-2016
 *  @copyright Tymphany Ltd.
 */


#ifndef __MEMORY_CONFIG_H__
#define __MEMORY_CONFIG_H__

// ST default bootloader address
#define MEMORY_ADDR_ISP         (0x1FFFC800)
// user bootloader address
#define MEMORY_ADDR_BTL         (0x08000000)
// user application address
#define MEMORY_ADDR_APP         (0x08004000)
#define MEMORY_ADDR_APP_END     (0x0801F7fb)
#define MEMORY_ADDR_APP_CSM     (0x0801F7fc)

// setting server related
#define SETT_PAGE_ROM_ADDR      0x0801F800
// how many dwords should be write when erase page happens.
#define SETT_MAX_USED_DWORDS    8
// enter dfu checking address
#define ENTER_DFU_FLAG_OFFSET   0x0400

#define ENTER_DFU_MAGIC_NUMBER  0x12345678

#endif  // __MEMORY_CONFIG_H__
