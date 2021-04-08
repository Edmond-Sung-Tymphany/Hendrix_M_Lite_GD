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

#define ASOC_CRITICAL                                 (batt_capacity_critical)

typedef enum eChargerDebouncingState
{
    CHARGER_STATE_STABLE,
    CHARGER_STATE_DEBOUNCING,
    CHARGER_STATE_ERROR,
} eChargerDebouncingState;
                                           //              Voltage  Compare   PowerDrv-level  ASE-TK-LEVEL  LED-Batt  LED-DC        ASE-TK
const uint32 batt_capacity_critical = 2;   //  0% ~   2%            (ASOC)     CRITICAL -----> CRITICAL     N/A       N/A           Shutdown
const uint32 batt_capacity_low      = 10;  //  3% ~  10%            (RSOC user)     LOW ----------> LOW          Dim-RED   White-Pulse   DFU Disable
const uint32 batt_capacity_mid      = 50;  // 11% ~  50%            (RSOC user)     MID ==========> LOW          N/A       White-Pulse   DFU Disable
const uint32 batt_capacity_extra    = 80;  // 51% ~  80%            (RSOC user)     EXTRA ========> HIGH         N/A       White-Pulse   DFU Enable
const uint32 batt_capacity_high     = 100; // 81% ~ 100%            (RSOC user)     HIGH ---------> HIGH         N/A       N/A           DFU Enable

/* Sometimes battery report >=100 health, means remain capacity > design capacity,
 * thus we set good threshold is also >= 100
 */
const uint32 healthLevelGood = 150; 
const uint32 healthLevelPoor = 50;  //suggest service center to replace battery
const uint32 healthLevelCritical = 40;  //power off system when DC=0

/******************************************************************************
  ******************************struct*****************************************
  *****************************************************************************/
typedef void (*powerUpFunc)(cPowerDrv *me);

typedef struct tpowerUpSequence{
    powerUpFunc powerUpFunction;
    int32 delaytime;
} tpowerUpSequence;

typedef struct hwVersionTag{
    const char * hwVersionStr;
    uint16 minVol;
    uint16 maxVol;
} hwVersionTag;



/******************************************************************************
 ********************* private functions / data *******************************
 *****************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me);
static void PowerDrv_PushIntEvtToServer(eSignal signal);
static void PowerDrv_EnablelWakeUpSources();
static void PowerDrv_DisableEXTI_Config();
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();
static void PowerDrv_UpdateAdcValues();
static void PowerDrv_InitVariables(cPowerDrv *me);
static void PowerDrv_InitBattVariables(cPowerDrv *me);
static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me);
static eChargerState PowerDrv_GetChargerStatus();
static void PowerDrv_ReportStateChange(cPowerDrv *me);
static void PowerDrv_PrintInfo(cPowerDrv *me);
static void PowerDrv_BattHandler(cPowerDrv *me, eBatteryInfoId currId);
static void PowerDrv_CheckTempLevel(cPowerDrv *me, eTempLevel tempLevelNew, eTempLevel tempLevelPrev);

static BatteryStatus PowerDrv_GetBatteryStatus(uint16 battCapacity);
static eBattHealth PowerDrv_GetHealthStatus();

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
