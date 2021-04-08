/**
 * @file        DebugSettSrv.h
 * @brief       this\ is\ debug\ setting \ server
 * @author      Dmitry.Abdulov
 * @date        2014-08-13
 * @copyright   Tymphany Ltd.
 */


#ifndef DEBUGSETTSRV_H
#define	DEBUGSETTSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "product.config"
#include "server.h"
#include "PowerSrv.h"
#ifdef HAS_POWER_DELEGATE
#include "PowerDlg.h"
#endif
#ifdef HAS_BLUETOOTH
#include "BluetoothSrv.h"
#endif
#ifdef HAS_BT_DELEGATE
#include "BluetoothDlg.h"
#endif

#include "AudioSrv.h"



#define CR 0x0D
#define END_OF_CMD_SYM  0x0D  /* caret return */

#define START_SIGN      0xAA  // package signature
#define READ_TIMEOUT_MS      3000  // reading timeout in ms

enum DebugSrvSignals
{
    //TIMEOUT_SIG = MAX_SIG,
    CMD_READY_CALLBACK_SIG = MAX_SIG + 1,
    CMD_WAKEUP_SIG,
    CMD_START_READ_TIMEOUT_SIG,
    DEBUG_TIMEOUT_SIG,
    CLI_ADC_MMI_REQ_SIG,
    CLI_AUX_REQ_SIG,
};



typedef enum
{
    DEBUG_PRINT_EVT,
    DEBUG_SIM_DONE_EVT, /* not used now */
} eDebugEvt;


IND_EVT(DebugCmdEvt)
END_IND_EVT(DebugCmdEvt)

REQ_EVT(DebugEvt)
    eDebugEvt evt;
    uint8 length;
    uint8* pMsg;
END_REQ_EVT(DebugEvt)

RESP_EVT(DebugRespEvt)
    uint32 resp;   /* return value from UartDrv_Write() */
END_RESP_EVT(DebugRespEvt)

RESP_EVT(DebugSnkyResp)
END_RESP_EVT(DebugSnkyResp)


#define MAX_PRINTSTR_SIZE   (SIZE_OF_LARGE_EVENTS - sizeof(QEvt) - sizeof(QActive*))
REQ_EVT(DebugPrintEvt)
    uint8 size;
    char msg[MAX_PRINTSTR_SIZE - sizeof(uint8)];
END_REQ_EVT(DebugPrintEvt)
/*_______ setting data______________*/
typedef enum
{
    SETID_BATT_INFO = 0,
    SETID_VOLUME,        // 1
    SETID_AC_STATUS,     // 2
    SETID_ALLPLAY_INFO,  // 3
    SETID_DSP_INIT_DATA,
    SETID_DSP_TUNABLE_PART,
    SETID_CHANNEL,
    SETID_IS_AUXIN_PLUG_IN,
    SETID_MAX_VOLUME,
    SETID_POWER_MODE,
    SETID_BT_STATUS,
    SETID_CALLING_STATUS,
    SETID_MUSIC_DET,
    SETID_MAX
}eSettingId;

typedef struct tSettingEntry
{
#ifdef HAS_BATTERY
    tBatteryInfo    bi;             /**< SETID_BATT_INFO    */
#endif
    uint8   vol_level;              /**< SETID_VOLUME       */
    bool    isAcPlugIn;             /**< SETID_AC_STATUS    */
#ifdef HAS_ALLPLAY
    AllPlaySrvInfoEvt allplayInfo;  /**< SETID_ALLPLAY_INFO */
#endif // HAS_ALLPLAY
#ifdef AUDIO_MULTI_SOURCE
    eAudioChannel channel;
    bool isAuxInPlugIn;
#endif
    uint8 maxVolume;
#ifdef HAS_POWER_DELEGATE
    ePowerMode powerMode;
#endif
#ifdef HAS_BLUETOOTH
    eBtStatus btStatus;
#endif
#ifdef HAS_BT_DELEGATE
    eCallState callState;
#endif
    bool isMusicPlaying;            /***< SETID_MUSIC_DET */
}tSettingEntry;

/* the flash type */
typedef enum
{
    PIC32_NVM_FLASH = 0,
    EEPROM_FLASH,
    MAX_FLASH_TYPE,
}eFlashType;

typedef enum
{
    SETTING_DIRECTION_DOWNLOAD = 0, /**< data from PC to DUT */
    SETTING_DIRECTION_UPLOAD,       /**< data from DUT to PC */
}eSettingDirection;

#define SETTING_CHUNK_SIZE   (SIZE_OF_LARGE_EVENTS - 8)
REQ_EVT(SettingStartReqEvt)
    eSettingId          settingId;
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
/*______________________________________*/
REQ_EVT(SettingReqEvt)
END_REQ_EVT(SettingReqEvt)

#ifdef DEBUG_BATT_INFO
typedef struct
{
    int16	        intBatteryVol;
    int16	        extBatteryVol;
    int16           dcPlugInVoltage;
    eChargerState   chargerState;
}tBattSettInfo;
#endif

#ifdef DEBUG_BATT_INFO
#define SETT_BATT_INFO_SIZE (sizeof(tBattSettInfo))
#else
#define SETT_BATT_INFO_SIZE 0
#endif

#ifdef HAS_BLUETOOTH
#define BLUETOOTH_STATUS_SIZE (sizeof(eBtStatus))
#else
#define BLUETOOTH_STATUS_SIZE 0
#endif

#define VERSTR_SIZE (SETTING_CHUNK_SIZE - SETT_BATT_INFO_SIZE - BLUETOOTH_STATUS_SIZE - 1)/2

RESP_EVT(SettingRespEvt)
#ifdef DEBUG_BATT_INFO
    tBattSettInfo batt_info;
#endif
#ifdef HAS_BLUETOOTH
    eBtStatus       btStatus;
#endif
#ifdef PRODUCT_VERSION_MCU
    char            mcuVer[VERSTR_SIZE];
#endif
#ifdef PRODUCT_VERSION_DSP
    char            dspVer[VERSTR_SIZE];
#endif
END_RESP_EVT(SettingRespEvt)
/***************************************/

IND_EVT(SettingUpdateEvt)       /* SETTING_UPDATE_SIG payload event */
    eSettingId          setting_id;     /* which setting have been updated */
END_IND_EVT(SettingUpdateEvt)


SUBCLASS(cDebugSettSrv, cServer)
/*______________________________*/
    QActive         *pRequestor;
    uint16          size;           /**< the number of bytes of the interested data */
    uint16          processed;      /**< number of bytes processed */
    uint8           currBuf;        /**< buffer ID currently used */
    uint8           resendCnt;      /**< counter for the number of resend triggered */
/*______________________________*/
    eSettingId      settingId;      /* last setting id have been updated */
METHODS
    /* public functions */
/* Implement these so the controller can launch the server */
void DebugSettSrv_StartUp(cPersistantObj *me);
void DebugSettSrv_ShutDown(cPersistantObj *me);

END_CLASS

/* to print out pMsg through debug pc tool */
void DebugSettSrv_PrintStr(char* pMsg);
/*__________________________________________*/
void        SettingSrv_InitDB(void);
bool        Setting_IsReady(eSettingId id);
const void* Setting_Get(eSettingId id);
void        Setting_Set(eSettingId id, const void* pValue);

#ifdef	__cplusplus
}
#endif

#endif	/* DEBUGSETTSRV_H */

