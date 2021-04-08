/**
 * @file      HardwareProfile_flash.h
 * @brief     Define the flash configuration parameter
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef _HARDWARE_PROFILE_FLASH_H_
#define _HARDWARE_PROFILE_FLASH_H_


//Memory configuration
#define PROGRAM_FLASH_SIZE       0x40000  //256KB, should be the same as register BMXPFMSZ
#define CHECK_PROGRAM_FLASH_ADDR() ( assert(BMXPFMSZ==PROGRAM_FLASH_SIZE) )  //check if flash size have correct defination
//#define PROGRAM_FLASH_END_ADRESS (0x9D000000+BMXPFMSZ-1)  //0x9D03FFFF
#define PROGRAM_FLASH_END_ADRESS (0x9D000000+PROGRAM_FLASH_SIZE-1)  //0x9D03FFFF


/* APP_FLASH_BASE_ADDRESS and APP_FLASH_END_ADDRESS reserves program Flash for the application
 * Rule:
 * 1)The memory regions kseg0_program_mem, kseg0_boot_mem, exception_mem and
 *   kseg1_boot_mem of the application linker script must fall with in APP_FLASH_BASE_ADDRESS
 *   and APP_FLASH_END_ADDRESS
 *
 * 2)The base address and end address must align on  4K address boundary */
#define BOOT_FLASH_BASE_ADDRESS     0x9FC00000
#define APP_FLASH_BASE_ADDRESS      0x9D00E000
#define APP_FLASH_END_ADDRESS       PROGRAM_FLASH_END_ADRESS
#define APP_FLASH_PHY_BASE_ADDRESS  KVA_TO_PA(APP_FLASH_BASE_ADDRESS) //convert to pysical address (0x9D000000 to 0x1D000000)
#define APP_FLASH_PHY_END_ADDRESS   KVA_TO_PA(APP_FLASH_END_ADDRESS)  //convert to pysical address (0x9D000000 to 0x1D000000)

#define VALID_APP_FALSH_ADDR(addr) ((void*)(addr)>=(void*)APP_FLASH_BASE_ADDRESS && (void*)(addr)<=(void*)APP_FLASH_END_ADDRESS)

/* Address of  the Flash from where the application starts executing */
/* Rule: Set APP_FLASH_BASE_ADDRESS to _RESET_ADDR value of application linker script*/
// For PIC32MX1xx and PIC32MX2xx Controllers only
#define USER_APP_RESET_ADDRESS     (APP_FLASH_BASE_ADDRESS + 0x1000)


//Flash parameter
// PIC32MX3xx to PIC32MX7xx devices
#define FLASH_PAGE_SIZE             4096
#define DEV_CONFIG_REG_BASE_ADDRESS 0x9FC02FF0
#define DEV_CONFIG_REG_END_ADDRESS  0x9FC02FFF

#endif /* #define _HARDWARE_PROFILE_FLASH_H_ */
