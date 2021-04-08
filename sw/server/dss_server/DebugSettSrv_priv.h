/**
 * @file        DebugSettSrv_priv.h
 * @brief       this\ is\ debug\ setting\ server
 * @author      Dmitry.Abdulov
 * @date        2014-08-13
 * @copyright   Tymphany Ltd.
 */

#ifndef DEBUGSETT_SERVER_PRIVATE_H
#define DEBUGSETT_SERVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

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


typedef struct tDbgSnkyMsgEvt
{
    uint8 seq;
    eSignal signal;
    ePersistantObjID target_srv_id;
    uint16 msg_size;
} tDbgSnkyMsgEvt;

#ifdef __cplusplus
}
#endif

#endif /* DEBUGSETT_SERVER_PRIVATE_H */