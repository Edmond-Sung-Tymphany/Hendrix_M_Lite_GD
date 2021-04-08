/**
 * @file        EepromDrv_priv.h
 * @brief       It's the driver for read/write external EEPROM
 * @author      Johnny Fan 
 * @date        2014-03-31
 * @copyright   Tymphany Ltd.
 */
#ifndef EEPROMDRV_PRIV_H
#define EEPROMDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "EepromDrv.h"

 /**
* Write some 32bit datas into EEPROM
*/
static bool EepromDrv_WriteWords(cStorageDrv *me, uint32 addr, uint32* data, uint32 sizeInBytes);

/**
* Read some 32bit datas from EEPROM
*/
static bool EepromDrv_ReadWords(cStorageDrv *me, uint32 addr, uint32* data, uint32 sizeInBytes);

/* private functions / data */

#ifdef __cplusplus
}
#endif

#endif /* EEPROMDRV_PRIV_H */