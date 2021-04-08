/**
 * @file        SettingSrv.h
 * @brief       This implement the server for all settings store and retrieve
 * @author      Wesley Lee
 * @date        2014-06-09
 * @copyright   Tymphany Ltd.
 */


#ifndef SETTINGSRV_H
#define SETTINGSRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "server.h"
#include "PowerSrv.h"
#ifdef HAS_ALLPLAY
    #include "AllplaySrv.h"
#endif
#include "AudioSrv.h"
#ifdef HAS_POWER_DELEGATE
#include "PowerDlg.h"
#endif
#ifdef HAS_BLUETOOTH
#include "BluetoothSrv.h"
#endif
#ifdef HAS_BT_DELEGATE
#include "BluetoothDlg.h"
#endif
#include "StorageDrv.h"
#include "attachedDevices.h"
#include "setting_id.h"  

#include "PowerSrv.h"

typedef struct tSettingRomMap
{
    eSettingId  id;
    uint32      addr;
}tSettingRomMap;

/* the flash type */
typedef enum
{
    PIC32_NVM_FLASH = 0,
    EEPROM_FLASH,
    MAX_FLASH_TYPE,
}eFlashType;

typedef enum
{
    SETTING_DIRECTION_DOWNLOAD = 0, /**< data from PC/BLE to DUT */
    SETTING_DIRECTION_UPLOAD,       /**< data from DUT to PC/BLE */
}eSettingDirection;

#define SETTING_CHUNK_SIZE   (SIZE_OF_LARGE_EVENTS - 8)
REQ_EVT(SettingStartReqEvt)
    eSettingId          id;
    eSettingDirection   direction;
    uint16              size;
END_REQ_EVT(SettingStartReqEvt)
RESP_EVT(SettingStartRespEvt)
END_RESP_EVT(SettingStartRespEvt)

REQ_EVT(SettingDataReqEvt)
    uint8 data[SETTING_CHUNK_SIZE];
END_REQ_EVT(SettingDataReqEvt)
RESP_EVT(SettingDataRespEvt)
END_RESP_EVT(SettingDataRespEvt)

REQ_EVT(SettingEndReqEvt)
    uint16 rest_size;
    uint8 data[SETTING_CHUNK_SIZE - sizeof(uint16)];
END_REQ_EVT(SettingEndReqEvt)
RESP_EVT(SettingEndRespEvt)
END_RESP_EVT(SettingEndRespEvt)

IND_EVT(SettingUpdateEvt)       /* SETTING_UPDATE_SIG payload event */
    eSettingId       setting_id;     /* which setting have been updated */
    uint16           server_id;/*server id who made update through SETTING_xxx_SIG or LAST_SRV_ID if it was made by Setting_Set() */
END_IND_EVT(SettingUpdateEvt)

REQ_EVT(SettingWriteOffsetReqEvt)
    eSettingId       setting_id;     /* which setting have been updated */
    uint16           offset;
    uint16           size;
    uint8 data[SETTING_CHUNK_SIZE - sizeof(eSettingId) - sizeof(uint16) - sizeof(uint16)];
END_REQ_EVT(SettingWriteOffsetReqEvt)


REQ_EVT(SettingReadOffsetReqEvt)
    eSettingId       setting_id;     /* which setting will be read */
    uint16           offset;
    uint16           size;
END_REQ_EVT(SettingReadOffsetReqEvt)

RESP_EVT(SettingReadOffsetRespEvt)
    eSettingId       setting_id;     /* a response with the data */
    uint16           offset;
    uint16           size;
    uint8 data[SETTING_CHUNK_SIZE - sizeof(eSettingId) - sizeof(uint16) - sizeof(uint16)];
END_RESP_EVT(SettingReadOffsetRespEvt)

REQ_EVT(SettingFlashReqEvt)
    bool             bIsSave;     /* if TRUE - force to save into flash; if FALSE force to read from flash */
END_REQ_EVT(SettingFlashReqEvt)

SUBCLASS(cSettingSrv, cServer)
    /* private data */
    QActive         *pRequestor;
    cStorageDrv     *pStorageDrv;
    eSettingId      processingId;   /**< last setting id have been updated */
    uint16          size;           /**< the number of bytes of the interested data */
    uint16          processed;      /**< number of bytes processed */
    uint8           resendCnt;      /**< counter for the number of resend triggered */
METHODS
    /* public functions */
END_CLASS

/* Implement these so the controller can launch the server */

#ifdef SETTING_HAS_ROM_DATA
void SettingSrv_StartUp(cPersistantObj *me);
void SettingSrv_ShutDown(cPersistantObj *me);
#endif
void        SettingSrv_InitDB(void);
bool        Setting_IsReady(eSettingId id);
const void* Setting_Get(eSettingId id);
const void* Setting_GetEx(eSettingId id, const void* pDefault);
const void* Setting_GetAddr(eSettingId id);
uint32      Setting_GetSize(eSettingId id);
void        Setting_Set(eSettingId id, const void* pValue);
bool        Setting_IsIdValid(eSettingId id);
void        Setting_Reset(eSettingId id);
void        SettingSrv_BookkeepingEx();
void        SettingSrv_FlashReq(QActive* sender, bool bIsSave);

#ifdef __cplusplus
}
#endif

#endif /* SETTINGSRV_H */

