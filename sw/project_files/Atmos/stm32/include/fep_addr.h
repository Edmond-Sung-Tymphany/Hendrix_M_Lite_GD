/**
 *  @file      fep_addr.h
 *  @brief     The list of address for OTA
 *  @author    Wesley Lee
 *  @date      30-Nov-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _FEP_ADDR_H_
#define _FEP_ADDR_H_

#define FEP_ADDR_ISP                (0x1FFFD800)
#define FEP_ADDR_PIU                (0x08000000)
#define FEP_ADDR_UBL                (0x08004000)
#define FEP_ADDR_FIRMWARE           (0x0800A000)

#define FEP_ADDR_UBL_END            (0x08009FFF)
#define FEP_ADDR_FIRMWARE_END       (0x0803E7FF)

// For STM32F091RC with 256k flash
#define FEP_ADDR_STBL_STATUS        (0x0803F800)
#define FEP_ADDR_UBL_CHECKSUM       (0x0803F804)
#define FEP_ADDR_FIRMWARE_CHECKSUM  (0x0803F808)
#define FEP_ADDR_FIRMWARE_STATUS    (0x0803FC00)

#define FEP_ADDR_PIU_VER            (0x0803FBF0) //4bytes
#define FEP_ADDR_UBL_VER            (0x0803FBF4) //4bytes
#define FEP_ADDR_FIRMWARE_VER       (0x0803FBF8) //4bytes

typedef enum
{
    FEP_STBL_NORMAL = 0,
    FEP_STBL_NEW,                   // skip ASE-TK power sequence in PIU
    FEP_STBL_UPGRADE_UBL,           // upgrade Upgradable-Boot-Loader upon request
    FEP_STBL_UPGRADE_FIRMWARE,      // upgrade FEP-Firmware upon request
    FEP_STBL_ERROR_UBL,             // error detected for Upgradable-Boot-Loader
    FEP_STBL_ERROR_FIRMWARE,        // error detected for FEP-firmware
}eFepStblStatus;

typedef enum
{
    FEP_FIRMWARE_NORMAL = 0,
    FEP_FIRMWARE_POWERED_ASETK,         // powered ASE-TK in PIU
    FEP_FIRMWARE_NEW,                   // skip ASE-TK power sequence in Firmware
    FEP_FIRMWARE_UPGRADE_UBL,           // upgrade Upgradable-Boot-Loader upon request
    FEP_FIRMWARE_UPGRADE_FIRMWARE,      // upgrade FEP-Firmware upon request
    FEP_FIRMWARE_ERROR_UBL,             // error detected for Upgradable-Boot-Loader
    FEP_FIRMWARE_ERROR_FIRMWARE,        // error detected for FEP-firmware
}eFepFirmwareStatus;

#endif
