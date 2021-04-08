/**
 * @file        SettingSrv_light_priv.h
 * @brief       This implement the server for all settings store and retrieve
 * @author      Edmond Sung
 * @date        2015-02-06
 * @copyright   Tymphany Ltd.
 */

#ifndef SETTINGSRV_LIGHT_PRIV_H
#define SETTINGSRV_LIGHT_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "SettingSrv.h"

/* @brief the structure of the array element
 *          for the fast lookup setting entity with only ID provided
 */
typedef struct tSettingDatabase
{
    void * const    p;      /**< pointer to the object*/
    uint16          size;   /**< size of the object*/
    uint16          attr;   /**< attribute*/
}tSettingDatabase;


#define SETTING_ATTR_NVM        0x0001      /**< the entity need to be stored in Non-Volatile Memory */
#define SETTING_ATTR_EEPROM     0x0002      /**< the entity need to be stored in EEPROM */
#define SETTING_ATTR_SET        0x2000      /**< the entity is being set already */
#define SETTING_ATTR_VALID      0x4000      /**< the entity is valid for the product */
#define SETTING_ATTR_UPD        0x8000      /*setting update mask if set,- data entry was updated in RAM */

static uint32 getRomAddr(eSettingId id);

/* State function definitions */

#ifdef __cplusplus
}
#endif

#endif /* SETTINGSRV_LIGHT_PRIV_H */




