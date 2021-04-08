/**
 * @file        DebugSSrv.h
 * @brief       this\ is\ debug\ s \ server
 * @author      Dmitry.Abdulov
 * @date        2014-10-08
 * @copyright   Tymphany Ltd.
 */


#ifndef DEBUGSETTSRV_H
#define	DEBUGSETTSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "product.config"
#include "server.h"

#include "UartDrv.h"

#include "SettingSrv.h"
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

/*______________________________________________________________________________________________________________*/
/*
 *  START SIGNATURE  | SIG   | SRV ID | SIZE_LSB | SIZE_MSB | DATA0 | ... | CRC16_LSB | CRC16_MSB |
 *                   |       |        |          |          |       |     |           |           |
 * ?	START SIGNATURE: (1 byte), package signature "0xAA"
 * ?	SIG(1 byte):  signal ID (one of from ?typedef enum {?}eSignal;?  please reference to ?tymphany_platform\sw\include\signals.h?)
 * ?	SRV_ID(1 byte): Target AO(active object)id(one of from ?typedef enum {?}ePersistantObjID;?  please reference to?tymphany_platform\sw\include\object_ids.h? )
 * ?	SIZE(2 bytes):  message size(size of byte stream)
 * ?	DATA(?):  encapsulated specific (according SIG)data structure
 * ?	CRC16: (2 bytes) uses CRC-CCITT (used in Xmodem, Bluetooth etc.)
 * http://en.wikipedia.org/wiki/Cyclic_redundancy_check.
 * Check your CRC here:
 * http://depa.usst.edu.cn/chenjq/www2/SDesign/JavaScript/CRCcalculation.htm
 * ex: CRC(0x01,0x02,0x03) -> 0x6131
 */
typedef enum
{
    START_SIGN_IDX,
    SIG_IDX,
    SRVID_IDX,
    SIZELSB_IDX,
    SIZEMSB_IDX,
    DATASTART_IDX,
}eDbgSnkyMsgIndx;

#define DMSG_MIN_SIZE   0x07

#define START_SIGN      0xAA  // package signature
#define READ_TIMEOUT_MS      3000  // reading timeout in ms

enum DebugSrvSignals
{
    //TIMEOUT_SIG = MAX_SIG,
    CMD_READY_CALLBACK_SIG = MAX_SIG + 1,
    CMD_WAKEUP_SIG,
    CMD_START_READ_TIMEOUT_SIG,
    DEBUG_TIMEOUT_SIG,
};



typedef enum
{
    DEBUG_PRINT_EVT,
    DEBUG_SIM_DONE_EVT, /* not used now */
} eDebugEvt;

typedef enum
{
    REQ_EVT_SIG,
    RESP_EVT_SIG,
    IND_EVT_SIG,
    INVALID_EVT_SIG,
} eSignalType;

typedef struct
{
    uint8	        appPrivSig;
    eSignalType	    sigType;
}tAppSigMapTable;


IND_EVT(DebugCmdEvt)
END_IND_EVT(DebugCmdEvt)

RESP_EVT(DebugSnkyResp)
END_RESP_EVT(DebugSnkyResp)

#ifdef PROJECT_BnO_SOUNDWALL
// special case on soundwall, hardcode the size here.
#define MAX_PRINTSTR_SIZE   24
#else
#define MAX_PRINTSTR_SIZE   (SIZE_OF_LARGE_EVENTS - sizeof(QEvt) - sizeof(QActive*) - sizeof(uint8))
#endif
REQ_EVT(DebugPrintEvt)
    uint8 size;
    char msg[MAX_PRINTSTR_SIZE];
END_REQ_EVT(DebugPrintEvt)

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

#ifdef HAS_GPIO_JACK_IN_DETECT
#define SETT_JACK_IN_STATUS_SIZE sizeof(bool)
#else
#define SETT_JACK_IN_STATUS_SIZE 0
#endif

#ifdef HAS_MUSIC_STATUS_INFO
#define SETT_MUSIC_STATUS_SIZE sizeof(bool)
#else
#define SETT_MUSIC_STATUS_SIZE 0
#endif

#ifdef HAS_AUDIO_SOURCE_INFO
#define SETT_AUDIO_SOURCE_SIZE sizeof(uint8)
#else
#define SETT_AUDIO_SOURCE_SIZE 0
#endif

#define VERSTR_SIZE ((SETTING_CHUNK_SIZE - SETT_BATT_INFO_SIZE - BLUETOOTH_STATUS_SIZE - SETT_JACK_IN_STATUS_SIZE - SETT_MUSIC_STATUS_SIZE - SETT_AUDIO_SOURCE_SIZE - 1)/2)

RESP_EVT(SettingRespEvt)
#ifdef HAS_AUDIO_SOURCE_INFO
    uint8          audioSource;
#endif
#ifdef HAS_GPIO_JACK_IN_DETECT
    bool           jackInStatus;
#endif
#ifdef HAS_MUSIC_STATUS_INFO
    bool           isMusicPlaying;
#endif
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
typedef struct tDebugSettConfig
{
    ePersistantObjID       srv_id;
    eDeviceID              source_device_id;
    /* only two options are avaiable: subscribe or not
     will change into bit mask or config mask about subscribed sig in future*/
    bool            bIsSubscribe;
    QEvt const      **ppEvtQue;
} tDebugSettConfig;

typedef struct tDbgSnkyMsgEvt
{
    uint8 seq;
    uint16 msg_size;
    eSignal signal;
    ePersistantObjID target_srv_id;
} tDbgSnkyMsgEvt;


#ifdef HW_GPIO_DEBUG
typedef enum
{
    IO_PORT_READ,
    IO_PORT_WRITE,
}eIoPortReadWriteAction;

REQ_EVT(HwGpioDebugReqEvt)
    eIoPort                     port;
    eIoBit                      bit;
    eGPIOInitAttr               action; //Indicate GPIO_ACTIVE_xxx for input pin, GPIO_DEFAULT_OUT_xxx for output pin
    eIoPortReadWriteAction      mode;
    eGPIODrection               direction;
END_REQ_EVT(HwGpioDebugReqEvt)


RESP_EVT(HwGpioDebugRespEvt)
    uint8                       value;
END_RESP_EVT(HwGpioDebugRespEvt)

#endif

#ifdef PT_ADC_DEBUG
REQ_EVT(DbgAdcReqEvt)
    eIoPort                     port;
    eIoBit                      bit;
END_REQ_EVT(DbgAdcReqEvt)

RESP_EVT(DbgAdcRespEvt)
    eIoPort                     port;
    eIoBit                      bit;
    uint16                      value;
END_RESP_EVT(DbgAdcRespEvt)

#endif

#ifdef PT_I2C_DEBUG

typedef enum
{
    I2C_READ_MODE,
    I2C_WRITE_MODE
}eITwoCReadWrite;

typedef enum
{
    I2C_READ,
    I2C_WRITE
}eI2CReadWrite;

#define I2C_DATA_MAX_SIZE       (SETTING_CHUNK_SIZE -  sizeof(eI2CReadWrite)  - sizeof(eI2C_Channel)- sizeof(uint32)-sizeof(eI2CRegAddLen) - sizeof(uint16) - sizeof(uint8) - sizeof(eEvtReturn))  // 73
REQ_EVT(DbgI2CPTReqEvt)
    eI2CReadWrite               mode;
    eI2C_Channel                channel;
    uint32                      regAddr;
    eI2CRegAddLen               regAddrLen;
    uint16                      length;
    uint8                       devAddr;
    uint8                       data[I2C_DATA_MAX_SIZE];
END_REQ_EVT(DbgI2CPTReqEvt)

RESP_EVT(DbgI2CPTRespEvt)
    eI2CReadWrite               mode;
    eI2C_Channel                channel;
    uint32                      regAddr;
    eI2CRegAddLen               regAddrLen;
    uint16                      length;
    uint8                       devAddr;
    uint8                       data[I2C_DATA_MAX_SIZE];
END_RESP_EVT(DbgI2CPTRespEvt)
#endif

#define CLI_INPUT_BUFFER_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)
#define CLI_OUTPUT_BUFFER_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)
#define DBG_UART_TX_BUF_SIZE ((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)* DBG_SRV_TX_BUF_Q_SIZE) + 1
#define DBG_UART_RX_BUF_SIZE (SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE) + 1

SUBCLASS(cDebugSSrv, cServer)
/*______________________________*/
    tDebugSettConfig    config;
/*------------------------------*/
    cUartDrv  p_debug_dev;
    cRingBuf txBuf;
    cRingBuf rxBuf;
    tDbgSnkyMsgEvt dmsg;
    uint8 in_buf_bs_idx;
    uint8 out_buf_byte_stream[CLI_OUTPUT_BUFFER_SIZE];
    uint8 uartTxBuf[DBG_UART_TX_BUF_SIZE];
    uint8 uartRxBuf[DBG_UART_RX_BUF_SIZE];
/*______________________________*/
METHODS
    /* public functions */
QState DebugSSrv_Initial(cDebugSSrv * const me, QEvt const * const e);
QState DebugSSrv_Ready(cDebugSSrv * const me, QEvt const * const e);
QState DebugSSrv_DeActive(cDebugSSrv * const me, QEvt const * const e);

/* Implement these so the controller can launch the server */
void DebugSrv_StartUp(cPersistantObj *me);// pre-starup function for debug server instance
#ifdef HAS_BLE_CTRL
void BLESrv_StartUp(cPersistantObj *me); // pre-starup function for ble server instance
#endif
void DebugSSrv_StartUp(cPersistantObj *me); // base start up function
void DebugSSrv_ShutDown(cPersistantObj *me); // base shutdown function

END_CLASS

/* to print out pMsg through debug pc tool */
void DebugSSrv_PrintStr(char* pMsg);
void DebugSSrv_Printf(char* pMsg);
void DebugSSrv_AssertPrintf(char* pMsg);
/*__________________________________________*/

#ifdef	__cplusplus
}
#endif

#endif	/* DEBUGSETTSRV_H */

