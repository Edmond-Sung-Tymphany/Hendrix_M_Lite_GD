#ifndef _PIU_COMMON_H
#define _PIU_COMMON_H

#include "IoExpanderDrv.h"

/* we use this value to enter either DFU mode or Application mode */
#define ADDR_BOOT_MODE                  (0x0801BE30)

/* we call also this address as ISP address or ROM bootloader address */
#define ADDR_SYSTEM_MEMORY              (0x1FFFC800)

/* the application program address*/
#define ADDR_APP_MEMORY                 (0x08003000)

/* the SRAM address*/
#define ADDR_SRAM_MEMORY                (0x20000000)

/* the bootloader program address */
#define ADDR_BOOTLOADER_MEMORY          (0x08000000)

enum
{
    BOOT_MODE_BOOTLOADER,
    BOOT_MODE_APP,
};
void jump2Address(uint32_t address);

void remappingVectorTable(uint32_t address);

void amplifier_Initial(void);

void ioExpander_Initial(void);

void ioExpander_SetLeds(ledMask leds, bool on);

#endif
