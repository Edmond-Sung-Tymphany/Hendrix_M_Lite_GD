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

#ifdef HAS_MENU
#include "MenuDlg.h"
#endif

#define PHASE_DEGREE                0x1B /*ascii 0x1B used for phase degree symbol*/
#define ENABLED                     1
#define DISABLED                    0
#define PRESET_EMPTY                0
#define TEN_MS                      10
#define MINUTES_IN_MS(x)            ((x)*60*1000)
#define SECONDS_IN_MS(x)            ((x)*1000)
#define TWO_SECONDS                 2
#define FLOAT_TYPE(x)               x<10
#define LOADED_TEXT                 displayText[50]
#define SAVED_TEXT                  displayText[51]
#define EMPTY_TEXT                  displayText[52]
#define CLEAN_SCREEN                displayText[60]
#define VOLUME_TEXT                 displayText[3]
#define LOGO_TEXT                   displayText[55]
#define OFF_TEXT                    displayText[54]
#define AUTO_TEXT                   displayText[57]
#define TRIGGER_TEXT                displayText[58]
#define MANUAL_TEXT                 displayText[59]
#define RESET_DONE_TEXT             displayText[77]
#define STANDBY_TIME_THRESHOLD_MIN  20 /* 2O minites */
#define NUM_OF_PRESET               3

#define RGC_FREQ_25                 250
#define RGC_FREQ_31                 310
#define RGC_FREQ_40                 400

#define DISPLAY_VOL                 0
#define DISPLAY_OFF                 1
#define DISPLAY_LOGO                2

#define INDEX_OF_LP_FRE_IN_DB       0

#define AUTO_STANDBY                0
#define TRIGGER_STANDBY             1
#define MANUAL_STANDBY              2
#define isPresetLoad                (pageSettId == PAGE_SETTING_PRE1_LOAD || \
                                    pageSettId == PAGE_SETTING_PRE2_LOAD || \
                                    pageSettId == PAGE_SETTING_PRE3_LOAD)
/* Supress warnings and disable unnecessary the optimization */
#define ROM_DADA(x) (void)(x)

#define NO_OFFSET                   0
#define ONE_ITEM                    1
#define DEFAULT_TIMEOUT_SEC         10

typedef struct tMenuDataAttr{
    ePageSettingId  settingId;
    int16           minVal;
    int16           maxVal;
    int16           defaultVal;
    int16           valPerStep;
    eDspSettId      dspSettId;
    const uchar     *pUnitString;
}tMenuDataAttr;

typedef struct tPageIdDspSettIdMap
{
    ePageSettingId  settingId;
    eDspSettId      dspSettId;
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

/* private state functions */
static QState MainApp_Initial(cMainApp * const me, QEvt const *const e);
static QState MainApp_PreActive(QActive * const me, QEvt const * const e);
static QState MainApp_Active(QActive * const me, QEvt const * const e);
static QState MainApp_DeActive(QActive * const me, QEvt const * const e);
static QState MainApp_PreDeActive(QActive * const me, QEvt const * const e);
static QState MainApp_SysReset(QActive * const me, QEvt const * const e);


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
static void MainApp_SetPageSetting(QActive * const me,const tPageNode * pPageNode, uint8 index);
static void MainApp_SyncSettFromBle();
static void MainApp_SendDisplayString(uint8 index);

static void MainApp_ParseKeyEvent(cMainApp * const me, KeyStateEvt const * const evt);
static void MainApp_VolumeCtr(QActive * const me, KeyStateEvt const * const e, BOOL volUp);
static void MainApp_PresetKeyHandler(QActive * const me,KeyStateEvt const * const e);
static void MainApp_UpdateDatabase(MenuDataUpdate * pMenuDataEvt, uint8 index);
static uint8 MainApp_SearchSettIndex(ePageSettingId pageSettId);
static void MainApp_HandleOnOffCase(QActive * const me, ePageSettingId pageSettId);
static void MainApp_HandlePresetCase(QActive * const me,ePageSettingId pageSettId);
static void MainApp_MuteKeyHandler(QActive * const me, KeyStateEvt const * const e);
static BOOL MainApp_HandeSysSett(QActive * const me, MenuDataUpdate * pMenuDataEvt);
static void MainApp_SendStandbyString(uint8 index);
static void MainApp_FactoryReset(QActive * const me);
static void MainApp_NextRGCValue(uint8 index);
static void MainApp_PreRGCValue(uint8 index);
static void MainApp_SendBtCmd(eBtCmd btCmd);
static void MainApp_TrigerDataTxToBle(uint16 numberOfSettItem, uint16 offset);
#ifdef __cplusplus
}
#endif
 
#endif /* MAIN_APP_PRIVATE_H */
