/**
 * @file        MainApp_priv.h
 * @brief       Main application for iBT150
 * @author      Christopher 
 * @date        2014-04-24
 * @copyright   Tymphany Ltd.
 */
 

#ifndef MAIN_APP_PRIVATE_H
#define MAIN_APP_PRIVATE_H
 
#ifdef __cplusplus
 extern "C" {
#endif

#include "product.config"
#include "application.h"

#include "keySrv.h"
#include "controller.h"
#include "modes.h"
#include "MainApp.h"
#include "audioSrv.h"
#include "BluetoothSrv.h"
#include "gpioDrv.h"
#include "SettingSrv.h"

#ifdef HAS_MENU
#include "MenuDlg.h"
#endif

#define BT_FACTORY_RESET            BT_RESET_PAIR_LIST_CMD
   
#define AMP_STANDBY_ENABLE          AMP_StandbyCtrl(FALSE)
#define AMP_STANDBY_DISABLE         AMP_StandbyCtrl(TRUE)

#define TRIGGER_IO                 (gpioDrv.gpioConfig->pGPIOPinSet[0].gpioId)
#define AMP_FAIL_IO                (gpioDrv.gpioConfig->pGPIOPinSet[1].gpioId)
#define CTRL_BOARD_AMP_POWER_IO    (gpioDrv.gpioConfig->pGPIOPinSet[2].gpioId)
#define AMP_STANDBY_IO             (gpioDrv.gpioConfig->pGPIOPinSet[3].gpioId)

#define AC_DETECT_PIN               ADC_PIN7

#define STRING_EQUAL                0
#define NUM_OF_PRESET               3
#define PHASE_DEGREE                0x1B /*ascii 0x1B used for phase degree symbol*/
#define ENABLED                     1
#define DISABLED                    0
#define EQ_ON                       10
#define EQ_OFF                      0
#define PRESET_EMPTY                0
#define PRESET_ENPTY_CHECK_INDEX    1
#define TEN_MS                      10
#define MINUTES_IN_MS(x)            ((x)*60*1000)
#define SECONDS_IN_MS(x)            ((x)*1000)
#define TWO_SECONDS                 2
#define ONE_SECONDS                 1
#define HOLDING_TIME_50MS           50
#define FLOAT_TYPE(x)               (x<10)
#define STANDBY_TIME_THRESHOLD_MIN  20 /* 20 minites */
#define STANDBY_MUSIC_TIMER_THRESHOLD 8 /* 8second */
#define NUM_OF_PRESET               3

#define RGC_FREQ_25                 250
#define RGC_FREQ_31                 310
#define RGC_FREQ_40                 400

#define PEQ_FREQ_20                 200
#define PEQ_FREQ_22                 220
#define PEQ_FREQ_25                 250
#define PEQ_FREQ_28                 280
#define PEQ_FREQ_30                 300

#define DISPLAY_VOL                 0
#define DISPLAY_OFF                 1
#define DISPLAY_LOGO                2

#define AUTO_STANDBY                0
#define TRIGGER_STANDBY             1
#define ALWAYS_ON_STANDBY           2
#define IS_PRESET_LOAD_ID           (pageSettId == PAGE_SETTING_PRE1_LOAD || \
                                     pageSettId == PAGE_SETTING_PRE2_LOAD || \
                                     pageSettId == PAGE_SETTING_PRE3_LOAD || \
                                     pageSettId == PAGE_SETTING_PRE4_LOAD)
#define IS_PEQ_FREQ                  (pMenuDataEvt->pPageNode->pageSettId == PAGE_SETTING_PEQ1_FRE || \
                                     pMenuDataEvt->pPageNode->pageSettId == PAGE_SETTING_PEQ2_FRE || \
                                     pMenuDataEvt->pPageNode->pageSettId == PAGE_SETTING_PEQ3_FRE)

#define IS_SETTING_SRV_READY        (Setting_IsReady(SETID_MENU_DATA) \
                                     &&Setting_IsReady(SETID_PRESET_1_NAME)&&Setting_IsReady(SETID_PRESET_2_NAME)\
                                     &&Setting_IsReady(SETID_PRESET_3_NAME))

#define IS_PRESET_KEY               (evt->keyId == PRESET_KEY_1_LOAD ||\
                                     evt->keyId == PRESET_KEY_2_LOAD ||\
                                     evt->keyId == PRESET_KEY_3_LOAD ||\
                                     evt->keyId == PRESET_KEY_1_SAVE ||\
                                     evt->keyId == PRESET_KEY_2_SAVE ||\
                                     evt->keyId == PRESET_KEY_3_SAVE)


#define NO_USER_INPUT_INFO         127 //It represents DEL, check ASCALL table
/* Supress warnings and disable unnecessary the optimization */
#define ROM_DADA(x) (void)(x)

#define NO_OFFSET                   0
#define ONE_ITEM                    1
#define DEFAULT_TIMEOUT_SEC         10

#define POLARITY_POSITIVE           0
#define POLARITY_NEGATIVE           10

#define PORT_TUNNING_STANDARD       200
#define PORT_TUNNING_EXTENDED       160
#define PORT_TUNNING_SEALED         120

#define EQ_SETTING_START_INDEX      4

#define   DISPLAY_OFFSET      0
#define   TIMEOUT_OFFSET      2
#define   STANDBY_OFFSET      4
#define   BRIGHTNESS_OFFSET   6
#define   LP_STATUS_OFFSET    8
#define   LP_FRE_OFFSET       10
#define   LP_SLO_OFFSET       12
#define   PEQ1_STATUS_OFFSET  14
#define   PEQ1_FRE_OFFSET     16
#define   PEQ1_BOOST_OFFSET   18
#define   PEQ1_Q_OFFSET       20
#define   PEQ2_STATUS_OFFSET  22
#define   PEQ2_FRE_OFFSET     24
#define   PEQ2_BOOST_OFFSET   26
#define   PEQ2_Q_OFFSET       28
#define   PEQ3_STATUS_OFFSET  30
#define   PEQ3_FRE_OFFSET     32
#define   PEQ3_BOOST_OFFSET   34
#define   PEQ3_Q_OFFSET       36
#define   RGC_STATUS_OFFSET   38
#define   RGC_FREQ _OFFSET    40
#define   RGC_SLOPE_OFFSET    42
#define   VOL_OFFSET          44
#define   PHASE_OFFSET        46
#define   POLARITY_OFFSET     48
#define   TUNNING_OFFSET      50

/* Below defines ascii boundary which could not be recognized by our system */
#define   LEFT_BRACE    0x7B  //"{"
#define   AT            0x40  //"@"
#define   COLON         0x3A  //":"
#define   FORWARD_SLASH 0x2F  //"/"
#define   SPACE         0x20  //" "
#define   SOH           0x01  //"SOH"
#define   IS_CHARACTER_LEGAL (pBleDataEvt->data[i] == 0 || pBleDataEvt->data[i] == SPACE \
          ||(pBleDataEvt->data[i] > FORWARD_SLASH && pBleDataEvt->data[i] < COLON) \
          ||(pBleDataEvt->data[i] > AT && pBleDataEvt->data[i] < LEFT_BRACE))
#define   PEQ_BOOST_DISPLAY_THRESHOLD   (-100)

#define MAX_DATA_SIZE   (SETTING_CHUNK_SIZE - sizeof(eSettingId) - sizeof(uint16) - sizeof(uint16))

#define    DISPLAY_ID           0x00
#define    SYSTEM_TIMEOUT_ID    0x01
#define    SYSTEM_STANDBY_ID    0x02
#define    LP_ID                0x03
#define    HP_ID                0x04
#define    PEQ1_ID              0x05
#define    PEQ2_ID              0x06
#define    PEQ3_ID              0x07
#define    RGC_ID               0x08
#define    PHASE_ID             0x09
#define    POLARITY_ID          0x0A
#define    TURNNING_ID          0x0B
#define    VOLUME_ID            0x0C
#define    PRESET_NAME_ID       0x0D
#define    BRIGHTNESS_ID        0x0E

typedef struct tMenuDataAttr{
    ePageSettingId  settingId;
    int16           minVal;
    int16           maxVal;
    int16           defaultVal;
    int16           valPerStep;
    eAudioSettId    dspSettId;
    const uchar     *pUnitString;
}tMenuDataAttr;

typedef struct tPageIdDspSettIdMap
{
    ePageSettingId  settingId;
    eAudioSettId    dspSettId;
}tPageIdDspSettIdMap;


typedef struct tSysSetting
{
    ePageSettingId  settingId;
    uint16          sysSetting;
}tSysSetting;

typedef enum
{
    DISP_INFO_STEP0,
    DISP_INFO_STEP1,
    DISP_INFO_BYE,
    DISP_INFO_STEP_MAX
}eDisplayInfoStep;

enum MainAppPriSignals /* main appp private signals */
{
    ACTIVATE_STATE_TIME_SIG      = MAX_SIG,
    DISPLAY_CTRL_TIME_SIG,
    BLE_WRITE_DATA_REQ_SIG       = 0xF0,//240
    BLE_READ_DATA_REQ_SIG        = 0xF1,//241
    BLE_READ_DATA_RESP_SIG       = 0xF2,//242
    BLE_RESET_ITEM_REQ_SIG       = 0xF3,//243
    BLE_MENU_FEATURE_REQ_SIG     = 0xF4,//244 what feature this product has
    BLE_MENU_FEATURE_RESP_SIG    = 0xF5,//245
    BLE_CONSUMING_REQ_SIG        = 0xF6,//246
    BLE_CONSUMING_RESP_SIG       = 0xF7,//247
    BLE_PRODUCTION_TEST_RQE_SIG  = 0xF8,//248 production test signal event only defined in sdf file
    BLE_PRODUCTION_TEST_RESP_SIG = 0xF9,//249
};

typedef struct tStandbyMusicCtrl
{
    uint32 musicStreamTimer;
    BOOL hasMusicBeforeStandby;
}tStandbyMusicCtrl;

/* Structures for state change */
typedef enum
{
    STANDBY_STATE,
    DEACTIVE_STATE,
    ACTIVE_STATE,
    STATE_MAX
}eTargetState;

typedef struct tStateChange
{
    eTargetState  targetState;
    int32         holdingTime; //in ms
}tStateChange;

REQ_EVT(BleWriteDataReq)
    eSettingId       setting_id;     /* which setting have been updated */
    uint16           offset;
    uint16           size;
    uint8 data[MAX_DATA_SIZE];
END_REQ_EVT(BleWriteDataReq)

REQ_EVT(BleReadDataReq)
    eSettingId       setting_id;     /* which setting will be read */
    uint16           offset;
    uint16           size;
END_REQ_EVT(BleReadDataReq)

#define SMALL_DATA_SIZE  4
RESP_EVT(BleReadDataSmallResp)
    eSettingId       setting_id;     /* a response which could carry the data with size of SMALL_DATA_SIZE */
    uint16           offset;
    uint16           size;
    int8             data[SMALL_DATA_SIZE];
END_RESP_EVT(BleReadDataSmallResp)

#define MEDIUM_DATA_SIZE  16
RESP_EVT(BleReadDataMediumResp)
    eSettingId       setting_id;     /* a response which could carry the data with size of MEDIUM_DATA_SIZE*/
    uint16           offset;
    uint16           size;
    int8             data[MEDIUM_DATA_SIZE];
END_RESP_EVT(BleReadDataMediumResp)

RESP_EVT(BleReadDataLargeResp)
    eSettingId       setting_id;     /* a response which could carry data with the maximum size */
    uint16           offset;
    uint16           size;
    int8             data[MAX_DATA_SIZE];
END_RESP_EVT(BleReadDataLargeResp)

REQ_EVT(BleResetItemReq)
    uint8    featureId;
END_REQ_EVT(BleResetItemReq)

/********* For sync the features **********/
REQ_EVT(BleGetFeatureReq)
END_REQ_EVT(BleGetFeatureReq)

RESP_EVT(BleFeatureResp)
    uint16           numberOfFeature;
    int8             data[MAX_DATA_SIZE];
END_RESP_EVT(BleFeatureResp)

RESP_EVT(BleUartTestResp)
END_RESP_EVT(BleUartTestResp)
/********End of for sync the features ********/

/* private state functions */
static QState MainApp_Initial(cMainApp * const me, QEvt const *const e);
static QState MainApp_PreActive(QActive * const me, QEvt const * const e);
static QState MainApp_Active(QActive * const me, QEvt const * const e);
static QState MainApp_DeActive(QActive * const me, QEvt const * const e);
static QState MainApp_PrepareStateChange(QActive * const me, QEvt const * const e);
static QState MainApp_SysReset(QActive * const me, QEvt const * const e);
static QState MainApp_ItemResetDisplay(QActive * const me, QEvt const * const e);


/* private  functions */
static void MainApp_SwitchMode(cMainApp * const me, uint16 modeId);
static void MainApp_SwitchAudioChannel(cMainApp * const me, eAudioChannel channel);

static void MainApp_TurnOnBt(cMainApp * const me);
static void MainApp_TurnOffBt(cMainApp * const me);

static void MainApp_VolumeDown(cMainApp * const me, uint8 step);
static void MainApp_VolumeUp(cMainApp * const me, uint8 step);
static void MainApp_RecoverPreSett(QActive * const me);
static void MainApp_RefreshSysSett();
static void MainApp_RecoverAduioSett(QActive * const me, const tPageIdDspSettIdMap * pRecover, uint8 num, BOOL isMandatory);
static void MainApp_DisplayPageInfo(const tPageNode * pPageNode);
static void MainApp_SetAudio(QActive * const me,const tPageNode * pPageNode, uint8 index);
static void MainApp_SyncSettFromBle(QActive * const me, uint8 settIndex);
static void MainApp_SendDisplayString(uint8 index);

static void MainApp_ParseKeyEvent(cMainApp * const me, KeyStateEvt const * const evt);
static void MainApp_VolumeCtr(QActive * const me, KeyStateEvt const * const e, BOOL volUp);
static void MainApp_PresetKeyHandler(QActive * const me,KeyStateEvt const * const e);
static void MainApp_UpdateDatabase(MenuDataUpdate * pMenuDataEvt, uint8 index);
static uint8 MainApp_SearchSettIndex(ePageSettingId pageSettId);
static void MainApp_HandleOnOffCase(QActive * const me, ePageSettingId pageSettId);
static void MainApp_HandlePresetCase(QActive * const me,ePageSettingId pageSettId);
static BOOL MainApp_HandleSysSett(QActive * const me, MenuDataUpdate * pMenuDataEvt);
static void MainApp_SendStandbyString(uint8 index);
static void MainApp_FactoryReset(QActive * const me);
static void MainApp_NextRGCValue(uint8 index);
static void MainApp_PreRGCValue(uint8 index);
static void MainApp_LogoCtrlReset();
static void MainApp_ProcessBleData(QActive * const me,BleWriteDataReq* pBLeDataEvt);
static BOOL MainApp_HandleEqSett(QActive * const me, MenuDataUpdate * pMenuDataEvt);
static BOOL MainApp_HandlePreset(QActive * const me, MenuDataUpdate * pMenuDataEvt);
static void MainApp_DisableMute(QActive * const me);
#ifdef HAS_SCREEN_DIM_CTRL
static void MainApp_RefreshDimTime();
#endif
static void MainApp_HandleBleOnOffCase(QActive * const me, ePageSettingId pageSettId);
static void MainApp_ExamDataFromBle();
static void MainApp_UpdateStandbyTimer();
static void MainApp_StartMenu(QActive * const me);
static void MainApp_InitPageStatus();
static void MainApp_UpdatePageStatus(tPageNode * pageNodeOff,tPageNode * pageNodeOn, uint8 i);
static void MainApp_ResetVolume();
static BOOL MainApp_DataVerification(BleWriteDataReq* pBleDataEvt);
static void MainApp_ResetItemData(uint8 optionId);
static BOOL MainApp_ResetData(ePageSettingId pageSettingId);
static void MainApp_SendReadDataResp(void * pDataToSend,eSettingId settId, uint16 size, uint16 offset);
static BOOL MainApp_IsEqOn(int16 statusValue);
static void MainApp_RefreshMenuNodes();
static void MainApp_RefreshDisCtrlTick(cMainApp * const me, const uint16 tickTime);
static void AMP_StandbyCtrl(BOOL standbyEnable);
static void MainApp_ValidateData(int16 * pData);
static void MainApp_ResetPresetName(eSettingId presetNameId, const uint8 * pString);
static void MainApp_UpdateString();
static void MainApp_ValidateCriticalInfo();
static void MainApp_ProcessPEQBootString(int8 settIndex);
static void MainApp_UpdateStandbyMusicStatus();
static void MainApp_SendBtCmd(eBtCmd btCmd);
static void MainApp_UpdateStateChangeInfo(eTargetState targetState, int32 holdTime);
static void MainApp_ResetStateChangeInfo();
static void MainApp_SetPolarityText(uint8 settIndex);
#ifdef SVS_16_ULTRA_PB
static void MainApp_SetTunningText(uint8 settIndex);
#endif
#ifdef __cplusplus
}
#endif
 
#endif /* MAIN_APP_PRIVATE_H */
