/**
 * @file        MainApp_priv.h
 * @brief       Main application for MGT
 * @author      Dmitry Abdulov
 * @date        2014-12-12
 * @copyright   Tymphany Ltd.
 */


#ifndef MAIN_APP_PRIVATE_H
#define MAIN_APP_PRIVATE_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "application.h"
#include "product.config"
#include "keySrv.h"
#include "controller.h"
#include "modes.h"
#include "audioSrv.h"
#include "BluetoothSrv.h"
#include "DebugSSrv.h"
#include "SettingSrv.h"

#include "bsp.h"
#include "MainApp.h"

#define POWERUP_DELAY_MS_FOR_BT_Q    5000
/*************** batt filter *********************/
#define CHECK_CHARGER_PIN_DELAY_MS   1200

#define BATT_HIGHEST_FILTER
///Filter length for battery voltage
#define BATT_FILTER_LEN  5

#define MIN_VOLUME       0

///the range that considered as valid, measured in mV
#define SAMPLE_RANGE_mV  50
///the number of samples that continuously out of range, then consider it as valid
#define OUT_OF_RANGE_ACCEPT_NUMBER 10

typedef struct
{
    bool isRemovable;
    uint16 sample[BATT_FILTER_LEN];
#ifdef BATT_HIGHEST_FILTER
    uint16 maxValue[BATT_FILTER_LEN];
#endif
    uint16 filterResult;
}tBattFilterValue;

typedef struct
{
    bool isReady;
    uint8 count;
    tBattFilterValue intBatt;
#ifdef EXTERNAL_BATTERY
    tBattFilterValue extBatt;
#endif
}tBatteryFilter;


typedef struct
{
    bool isFirstStart;
    uint8 sampleIndex;
    uint8 maxValueIndex;
    uint8 exceedRangeCount;
    bool  isInserted;
    uint8 actionDebounceCount;
    uint32 maxValueSum;
}tBattFilterCount;

/*________________________________________________________________*/
/***************************************************************************
 * Battery State Config
 ***************************************************************************/
///use high/low thresholds to filter the battery state (like schmitt trigger), make battery state more stable
///define the number of battery state that power server will detect
#define NUMBER_OF_STATE   7
#define NUMBER_OF_BOUNDS  (NUMBER_OF_STATE-1)

///Define the order that you put your state,
/// INCREASING_ORDER_STATE means from lower state to higher state
//#define INCREASING_ORDER_STATE
/// DECREASING_ORDER_STATE means from higher state to lower state
#define DECREASING_ORDER_STATE

#if defined(DECREASING_ORDER_STATE) && defined(INCREASING_ORDER_STATE)
    #error "can NOT define the DECREASING_ORDER_STATE and INCREASING_ORDER_STATE at the same time"
#endif

/***************************************************************************
 * Internal Battery Config
 ***************************************************************************/
/**
* @name Internal Battery Config
* The config data for internal battery
*/
//@{

///@note: choose SCHMITT_TRIGGER filter,
///  change the value below for the trigger thresholds
///           if not, need to consider the SMITH_THREADSHOLD_mV for high/low threshold
/// Config the battery voltage for each states, measured in "mV"

#define MAX_BATT_mVOLT              12600  //max. voltage
#define MIN_BATT_mVOLT              9900

#define BATT_95_PERCENTAGE_mVOLT_HIGH_BOUND    12190  //95% threshold, high bound
#define BATT_95_PERCENTAGE_mVOLT_LOW_BOUND     12190  //95% threshold, low bound

#define BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND    11640  //75% threshold, high bound
#define BATT_75_PERCENTAGE_mVOLT_LOW_BOUND     11640  //75% threshold, low bound

#define BATT_50_PERCENTAGE_mVOLT_HIGH_BOUND    10990  //50%, high bound of low voltage
#define BATT_50_PERCENTAGE_mVOLT_LOW_BOUND     10990  //50%, low  bound of low voltage

#define BATT_25_PERCENTAGE_mVOLT_HIGH_BOUND    10640  //25% high bound of critical voltage
#define BATT_25_PERCENTAGE_mVOLT_LOW_BOUND     10640  //25% low  bound of critical voltage

#define BATT_15_PERCENTAGE_mVOLT_HIGH_BOUND    10490  //16%, high bound of shutdown voltage
#define BATT_15_PERCENTAGE_mVOLT_LOW_BOUND     10490  //16%, low bound of shutdown voltage

#define BATT_5_PERCENTAGE_mVOLT_HIGH_BOUND     10270  //5%, high bound of real system shutdown voltage
#define BATT_5_PERCENTAGE_mVOLT_LOW_BOUND      10270  //5%, low bound of real system shutdown voltage

/// The threshold to detemine if battery is eject, measured in "mV"
#define BATT_EJECT_mVOLT         2800


/**
*  @brief This const array is corresponding with eBatteryState.
*  @see eBatteryState
*  @note: please put them in order
*/
const int16 intBattAdcHighBound[NUMBER_OF_BOUNDS]=
{
    BATT_95_PERCENTAGE_mVOLT_HIGH_BOUND,
    BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND,
    BATT_50_PERCENTAGE_mVOLT_HIGH_BOUND,
    BATT_25_PERCENTAGE_mVOLT_HIGH_BOUND,
    BATT_15_PERCENTAGE_mVOLT_HIGH_BOUND,
    BATT_5_PERCENTAGE_mVOLT_HIGH_BOUND,
};
const int16 intBattAdcLowBound[NUMBER_OF_BOUNDS]=
{
    BATT_95_PERCENTAGE_mVOLT_LOW_BOUND,
    BATT_75_PERCENTAGE_mVOLT_LOW_BOUND,
    BATT_50_PERCENTAGE_mVOLT_LOW_BOUND,
    BATT_25_PERCENTAGE_mVOLT_LOW_BOUND,
    BATT_15_PERCENTAGE_mVOLT_LOW_BOUND,
    BATT_5_PERCENTAGE_mVOLT_LOW_BOUND,
};
//@} //endof internal Battery Config
///Filter length for input source
#define INPUT_SROUCE_FILTER_LEN 10




/* the time (ms) per timeout signal */
#define MAIN_APP_TIMEOUT_IN_MS  20

#define VOLUME_STEP     1
/*inactive timeout*/
#define INACTIVE_TIMEOUT_IN_MS  900000   // 15 min
#define RECONNECT_TIMEOUT_IN_MS 5000     // 5 sec

#define MAINAPP_TIMER_IS_NOT_SETUP (-1)

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
};


typedef enum
{
    LED_FLASHING_NONE,
    LED_FLASHING_BT_PAIRING,
    LED_FLASHING_BT_RECONNECT,
    LED_FLASHING_BT_VOL_MAXMIN,
    LED_FLASHING_AUX_VOL_MAXMIN,
    LED_FLASHING_CRITICAL_BATT_LEVEL,
    LED_FLASHING_MAX,
}eMainAppLedFlashingState;



/* private state functions */
QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);
QState MainApp_Active(cMainApp * const me, QEvt const * const e);
QState MainApp_BtDisconnect(cMainApp * const me, QEvt const * const e);
QState MainApp_PoweringDown(cMainApp * const me, QEvt const * const e);

QState MainApp_Standby(cMainApp * const me, QEvt const * const e);


/* private  functions */
static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e);
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e);


static void MainApp_VolumeDown(cMainApp * const me, uint8 step);
static void MainApp_VolumeUp(cMainApp * const me, uint8 step);

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId);
static void MainApp_InitialVariablesForPowerUp(cMainApp * const me);
static void MainApp_InitialVariablesForFactoryReset(cMainApp * const me);
static void MainApp_PublishSystemStatus(cMainApp * const me, eSystemStatus systemStatus);

static void MainApp_SetWakeUp(cMainApp * const me);
static void MainApp_Update(cMainApp * const me);
static void MainApp_UpdateInputSource(cMainApp * const me);
static bool MainApp_StartHigestFilter(tBattFilterCount* pBattFilterCount,
                                            tBattFilterValue* pBattFilterValue, uint16 sample);
static void MainApp_UpdateBatt(cMainApp * const me);
static bool MainApp_UpdateBattVoltage(uint16 intBatteryVol);
static void MainApp_UpdateBattState(cMainApp * const me);

static void MainApp_SaveInputSource(cMainApp * const me, tInputSourceState* inputSourceState);
static void MainApp_UpdateCharger(cMainApp * const me);
static bool MainApp_IsInputSourceStable(bool* pCurInputState, bool* pPreInputState, uint8* pCount);
static void MainApp_InitialBattVariable(cMainApp * const me);
static int16 MainApp_FindMaxValue(uint16* pArray, int16 size);
static uint8 MainApp_GetBattState(uint8 preState,
            int16 batteryVol, const int16* highBound,  const int16* lowBound);
static void MainApp_ParseBTEvent(cMainApp * const me, QEvt const * const e);
static void MainApp_UpdateLedFlashing();
static void MainApp_StartVolMaxMinLedFlashing();
static void MainApp_StartPairingLedFlashing();
static void MainApp_StartReconnectLedFlashing();
static void MainApp_SetNoneLedFlashing();
static void MainApp_KeySPressVolUpDownEvtAction(cMainApp * const me, int16 count);
static bool MainApp_IsSystemIdle(cMainApp * const me);
static void MainApp_RestoreVolume(cMainApp * const me, bool bIsStartupRestore);
static QStateHandler* MainApp_IsBtConnected(cMainApp * const me);
#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
static eAudioPowerGainLevel MainApp_GetGainLevel(cMainApp * const me);
#endif

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
static eAudioPowerDrcLevel MainApp_GetDrcLevel(cMainApp * const me);
#endif

static void MainApp_SettingReset(cMainApp * const me);

static void MainApp_StartCriticalBattLevelLedFlashing();
static void MainApp_SwitchSource(cMainApp * const me, eAudioChannel channel);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_APP_PRIVATE_H */
