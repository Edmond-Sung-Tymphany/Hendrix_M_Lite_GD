/**
 * @file        EepromDrv.h
 * @brief       It's the driver for read/write external EEPROM
 * @author      Johnny Fan 
 * @date        2014-03-31
 * @copyright   Tymphany Ltd.
 */

#ifndef EEPROMDRV_H
#define EEPROMDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "storageDrv.h"

SUBCLASS(cEepromDrv,cStorageDrv)
    /* private data */
METHODS
    /* public functions */
void EepromDrv_Ctor(cEepromDrv *me);
void EepromDrv_Xtor(cEepromDrv *me);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* EEPROMDRV_H */

