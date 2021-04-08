/**
 * @file        PowerDrv_priv.h
 * @brief       It's the power driver for STM32F0xx, (light edition)used in MGT
 * @author      Dmitry Abdulov
 * @date        2015-01-23
 * @copyright   Tymphany Ltd.
 */
#ifndef POWERDRV_PRIV_H
#define POWERDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx.h"
#include "qp_port.h"
#include "GpioDrv.h"
#include "PowerDrv.h"
#include "PowerSrv.h"
#include "bsp.h"
#include "AdcDrv.h"  //for getting the battery level
#include "signals.h"
#include "Setting_id.h"
#include "PowerDrv_light.config"
#include "deviceTypes.h"
#include "controller.h"

/******************************************************************************
  ******************************define*****************************************
  *****************************************************************************/
#define TYMQP_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)


//After DC insert, DC input in need 1 seconds to change DC pin
#define DC_DEBOUNCE_TIME_MS (80)

#define MAX_5V_mVOLT (5000)
#define MAX_5V_ADC   (3103)
#define _5V_TO_mVOLT(adc)  (int16)((adc * MAX_5V_mVOLT) / MAX_5V_ADC)

#define CHARGER_ERROR_PERIOD_COUNT                    (7) // <-- Enlarge debouncing period by increaing this number
#define CHARGER_ERROR_PERIOD_LENGTH_MS                (1000) // From spec
#define CHARGER_ERROR_PERIOD_LENGTH_WITH_TOLERANCE_MS (CHARGER_ERROR_PERIOD_LENGTH_MS * 1.5)
#define CHARGER_ERROR_DEBOUNCING_LENGTH_MS            (CHARGER_ERROR_PERIOD_LENGTH_MS * (CHARGER_ERROR_PERIOD_COUNT + 0.5))

#define TIME_IS_UP                                    (0)
#define POWER_DRIVER_BATTERY_NO_RESPONSE_CRITICAL_MS  (6000)
#define BATTERY_HAVING_REST_PERIOD_SHORT_MS             (100)//If charging
#define BATTERY_HAVING_REST_PERIOD_NORMAL_MS            (3000)//If in active mode and not charging
#define BATTERY_HAVING_REST_PERIOD_LONG_MS              (10000)//If in off-charging mode but not charging


#define INSTANT_CRITICAL_COUNTER_THRESHOLD      (1)

#define CHECK_CHARGER_PIN_DELAY_MS   1200




/******************************************************************************
  ******************************struct*****************************************
  *****************************************************************************/
typedef void (*powerUpFunc)(cPowerDrv *me);

typedef struct tpowerUpSequence
{
    powerUpFunc powerUpFunction;
    int32 delaytime;
} tpowerUpSequence;


#ifdef HAS_BATTERY
typedef struct
{
    bool isRemovable;
    uint16 sample[BATT_FILTER_LEN];
    uint16 maxValue[BATT_FILTER_LEN];
    uint16 filterResult;
} tBattFilterValue;

typedef struct
{
    bool isReady;
    uint8 count;
    tBattFilterValue intBatt;
} tBatteryFilter;


typedef struct
{
    bool isFirstStart;
    uint8 sampleIndex;
    uint8 maxValueIndex;
    uint8 exceedRangeCount;
    bool  isInserted;
    uint8 actionDebounceCount;
    uint32 maxValueSum;
} tBattFilterCount;
#endif /*end of HAS_BATTERY*/

typedef enum eChargerDebouncingState
{
    CHARGER_STATE_STABLE,
    CHARGER_STATE_DEBOUNCING,
    CHARGER_STATE_ERROR,
} eChargerDebouncingState;

typedef enum {
    NTC_TEST_NORMAL,
    NTC_TEST_DECREASE,
    NTC_TEST_INCREASE,
    NTC_TEST_WAITING,
}NTC_TEST_CMD;

#ifdef HAS_HW_VERSION_TAG
typedef enum
{
    HW_VERSION_UNKNOWN,
    HW_VERSION_ES,
    HW_VERSION_EVT,
    HW_VERSION_DVT,
    HW_VERSION_MV,
    HW_VERSION_MAX
} HwVersion_t;

typedef struct hwVersionTag
{
    HwVersion_t hw_ver;
    char hwVersionStr[HW_VERSION_LENGTH];
    uint16 volThreshold;
} hwVersionTag;

const hwVersionTag hwVerMap[]=
{
    {HW_VERSION_UNKNOWN,     "Unknown",   (HW_ES_THRESHOLD+HW_THRESHOLD_OFFSET) },
    {HW_VERSION_ES,          "ES",        (HW_ES_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_EVT,         "EVT",       (HW_EVT_THRESHOLD-HW_THRESHOLD_OFFSET)},
    {HW_VERSION_DVT,         "DVT",       (HW_DVT_THRESHOLD-HW_THRESHOLD_OFFSET)},
    {HW_VERSION_MV,          "MV",        (HW_MV_THRESHOLD-HW_THRESHOLD_OFFSET) },
};
#endif /*end of HAS_HW_VERSION_TAG*/
/******************************************************************************
********************* private const data *******************************
*****************************************************************************/

const uint8 batt_capacity_critical = 5;
const uint8 batt_capacity_low      = 9;
const uint8 batt_capacity_mid      = 25;
const uint8 batt_capacity_extra    = 94;
const uint8 batt_capacity_high     = 100;

const uint8 batt_bt_lv_1      = 10;
const uint8 batt_bt_lv_2      = 30;
const uint8 batt_bt_lv_3      = 50;
const uint8 batt_bt_lv_4      = 70;
const uint8 batt_bt_lv_5      = 90;
const uint8 batt_bt_lv_6      = 100; 



const uint16 battCapacityTable[]=
{
    BATT_VOLTAGE_LEVEL_FULL,
    BATT_VOLTAGE_LEVEL_95,
    BATT_VOLTAGE_LEVEL_90,
    BATT_VOLTAGE_LEVEL_85,
    BATT_VOLTAGE_LEVEL_80,
    BATT_VOLTAGE_LEVEL_75,
    BATT_VOLTAGE_LEVEL_70,
    BATT_VOLTAGE_LEVEL_65,
    BATT_VOLTAGE_LEVEL_60,
    BATT_VOLTAGE_LEVEL_55,
    BATT_VOLTAGE_LEVEL_50,
    BATT_VOLTAGE_LEVEL_45,
    BATT_VOLTAGE_LEVEL_40,
    BATT_VOLTAGE_LEVEL_35,
    BATT_VOLTAGE_LEVEL_30,
    BATT_VOLTAGE_LEVEL_25,
    BATT_VOLTAGE_LEVEL_20,
    BATT_VOLTAGE_LEVEL_15,
    BATT_VOLTAGE_LEVEL_10,
    BATT_VOLTAGE_LEVEL_5,
    BATT_VOLTAGE_LEVEL_0,
};
const uint8 battCapacityTableMaxNum = ArraySize(battCapacityTable);



/******************************************************************************
 ********************* private functions / data *******************************
 *****************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me);
static void PowerDrv_PowerDownStage(cPowerDrv *me);
static void PowerDrv_PushIntEvtToServer(eSignal signal);
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();
static void PowerDrv_UpdateBattADC(cPowerDrv *me);
static void PowerDrv_UpdateVoltageValues(cPowerDrv *me);
static void PowerDrv_InitVariables(cPowerDrv *me);
static void PowerDrv_InitBattVariables(cPowerDrv *me);
static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me);
static void PowerDrv_UpdateChargerState(cPowerDrv *me);
static eChargerState PowerDrv_GetChargerStatus();
static void PowerDrv_ReportBatteryStateChange(cPowerDrv *me);
static void PowerDrv_ReportPowerState(cPowerDrv *me);
static void PowerDrv_BattHandler(cPowerDrv *me, eBatteryInfoId currId);
static void PowerDrv_CheckTempLevel(cPowerDrv *me, eTempLevel tempLevelNew, eTempLevel tempLevelPrev);
#ifdef HAS_BATTERY_NTC
static void PowerDrv_UpdateTempADC(cPowerDrv *me);
static bool PowerDrv_IsNtcTempLevelRise(uint16 currValue, uint16 threshold, uint8* sampleCount);
static bool PowerDrv_IsNtcTempLevelFall(uint16 currValue, uint16 threshold, uint8* sampleCount);
static bool PowerDrv_GetNtcTempLevel(uint16 ntcValue);
#endif
static BatteryStatus PowerDrv_GetBatteryStatus(uint16 battCapacity);
static BatteryStatus PowerDrv_GetBTBattStatus(uint16 battCapacity);

#ifdef HAS_HW_VERSION_TAG
static HwVersion_t PowerDrv_GetHWversion(void);
static void PowerDrv_SetHWversion(void);
#endif
#ifdef HAS_PWR_IO_EXPANDER
static void PowerDrv_WaitWakeupStable(void);
#endif


#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
