#ifndef __SYSTEM_DRV_DEFAULT_H__
#define __SYSTEM_DRV_DEFAULT_H__

typedef enum 
{
    SYSTEM_STATUS_AC_OFF=0,
    SYSTEM_STATUS_STANDBY,
    SYSTEM_STATUS_POWERING_UP,
    SYSTEM_STATUS_WORKING,
    SYSTEM_STATUS_AUTO_STANDBY,
    SYSTEM_STATUS_MAX
}SystemStatus_t;

typedef enum 
{
    POWER_STAGE_POWER_OFF=0,
    POWER_STAGE_POWER_READY,
    POWER_STAGE_DSP_READY,
    POWER_STAGE_BT_READY,
    POWER_STAGE_MAX
}PowerStage_t;

typedef enum
{
    POWER_SWITCH_OFF=0,
    POWER_SWITCH_ON=1,
    POWER_SWITCH_MAX
}PowerSwitch_t;

typedef enum
{
    BATTERY_CHARGE_STATUS_LOW = 0,
    BATTERY_CHARGE_STATUS_HIGH = 1,
}BatteryChargeStatus_t;

#ifdef HAS_DC_IN
typedef enum
{
    DC_IN_UNKNOWN = 0,
    DC_IN_NOT_DETECTED,
    DC_IN_DETECTED
} DCInStatus_t;
#endif

#ifdef HAS_BATTERY_DETECT
typedef struct
{
    uint8 sample_count;
    uint8 average_count;
    uint16 sample_peak;
    uint16 average_result;
    uint32 average_sum;
    uint16 StateOfCharge;
    uint16 instant_critical_counter;
} battery_t;
#endif

#ifdef HAS_BATTERY_NTC
typedef struct
{
    uint32_t value;
    uint8 sample_count;
    uint8 average_count;
    uint16 sample_peak;
    uint16 average_result;
    uint32 average_sum;
    bool TurnOffSystem;
    bool TurnOffCharging;
    bool Alert;
#ifdef UNIT_TEST_ENABLE
    uint16 level;
#endif
} NTC_t;
#endif

#ifdef HAS_I2C_BUS_DETECT
#define I2C_ERROR_NONE      ((uint16_t)0x00)
#define I2C_ERROR_DSP       ((uint16_t)0x01)
#define I2C_ERROR_AMP       ((uint16_t)0x02)
#endif

#ifdef HAS_BAT_CHARGE_STATUS
#define CHECK_BATTERY_CHARGE_STATUS_TIME_ONE_SECOND     (20)        /* 20 * 50 = 1000ms */
#define BATTERY_CHARGE_STATUS_TOGGLE_TIMES                          (7)         /* means 3 sec (HIGH-LOW-HIGH) or (LOW_HIGH_LOW) */
#endif

#ifdef HAS_BAT_CHG_CURRENT_SELECT
#define CHG_CUR_LV_HIGH     1
#define CHG_CUR_LV_LOW      0
#endif


typedef struct tagSystemPowerStatus
{
    uint8_t  sys_status;    // system status
    uint8_t  pow_stage;     // power stage
#ifdef HAS_AUTO_STANDBY
    bool     is_auto_standby;   // auto standby flag
#endif    
#ifdef HAS_POWER_SWITCH_KEY
    uint8_t  power_switch_on;
#endif
#ifdef HAS_DC_IN
    uint8_t dc_in_status;       // DC in status
#endif
#ifdef HAS_BATTERY_DETECT
    battery_t  battery;
#endif
#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
    int16_t restart_timeout;    // unit : ms;
#endif
#ifdef HAS_I2C_BUS_DETECT
    uint16_t i2c_error_status;
#endif
#ifdef QUICKLY_POWER_DOWN
    bool amp_mute_enable;
#endif
#ifdef HAS_BAT_CHARGE_STATUS
    uint8_t loopCount;
    uint8_t batteryChargeStatus;
    uint8_t batteryErrorCount;
    bool isBatteryError;
    bool isBatteryFullyCharged;
#endif
#ifdef HAS_BATTERY_NTC
    NTC_t   NTC;
    bool NTCrequestTurnOffSystem;
    bool NTCrequestTurnOffCharging;
#endif
#ifdef HAS_BAT_CHG_CURRENT_SELECT
    bool chargerCurrentLevel;
#endif

#ifdef UNIT_TEST_ENABLE
    uint16 unit_test_level;
#endif
}SystemPowerStatus_t;

#ifdef HAS_DC_IN
#define DC_IN_ADC_PIN         ADC_PIN8
#define DC_IN_ADC_THRESHOLD     (2172)          /* 1.7V, DC=11V */
#endif

#ifdef HAS_BAT_CHARGE_STATUS
#define CHECK_BATTERY_CHARGE_STATUS_TIME_ONE_SECOND     (20)        /* 20 * 50 = 1000ms */
#define BATTERY_CHARGE_STATUS_TOGGLE_TIMES                          (7)         /* means 3 sec (HIGH-LOW-HIGH) or (LOW_HIGH_LOW) */
#endif
void SystemDrv_SetSystemStatus(SystemStatus_t sys_status);
uint8_t SystemDrv_GetSystemStatus(void);
void SystemDrv_NextPowerStage(void);
void SystemDrv_SetPowerStage(PowerStage_t pow_stage);
uint8_t SystemDrv_GetPowerStage(void);
#ifdef HAS_AUTO_STANDBY
void SystemDrv_SetAutoStandby(bool on);
bool SystemDrv_IsAutoStandby(void);
#endif
#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
void SystemDrv_SystemRestart(uint16_t timeout);
#endif
#ifdef HAS_I2C_BUS_DETECT
void SystemDrv_SetI2cBusStatus(uint16_t error_status);
uint16_t SystemDrv_GetI2cBusStatus(void);
#endif

void SystemDrv_ShutDownAmp(bool enable);
void SystemDrv_SetIdHandler(int32_t set_id, bool enable, uint32_t param);

#ifdef HAS_POWER_SWITCH_KEY
void SystemDrv_PowerSwitchUpdate(void);
uint8_t SystemDrv_IsPowerSwitchOn(void);
#endif
#ifdef HAS_DC_IN
void SystemDrv_PowerDCInDetect(void);
uint8_t SystemDrv_IsDCInDetected(void);
#endif
#ifdef HAS_HW_VERSION_TAG
typedef enum {
    HW_VERSION_UNKNOWN,
    HW_VERSION_ES1,
    HW_VERSION_ES2,
    HW_VERSION_ES3,
    HW_VERSION_EVT1,
    HW_VERSION_EVT2,
    HW_VERSION_DVT1,
    HW_VERSION_DVT2,
    HW_VERSION_PVT,
    HW_VERSION_MP1,
    HW_VERSION_MP2,
    HW_VERSION_MAX
} HwVersion_t;
HwVersion_t SystemDrv_GetHWversion(void);
#endif

void SystemDrv_ShutDownAmp(bool enable);
void SystemDrv_SetIdHandler(int32_t set_id, bool enable, uint32_t param);

#ifdef HAS_POWER_SWITCH_KEY
void SystemDrv_PowerSwitchUpdate(void);
uint8_t SystemDrv_IsPowerSwitchOn(void);
#endif
#ifdef QUICKLY_POWER_DOWN
void SystemDrv_SetAmpMuteEnable(bool enable);
bool SystemDrv_GetAmpMuteEnable(void);
#endif
#ifdef HAS_DC_IN
void SystemDrv_PowerDCInDetect(void);
uint8_t SystemDrv_IsDCInDetected(void);
#endif
#ifdef HAS_BATTERY_NTC
void SystemDrv_InitNTCStatus(void);
void SystemDrv_BatteryNTCValue(void);
uint8_t SystemDrv_IsNTCTurnOffSystem(void);
uint8_t SystemDrv_isNTCTurnOffCharging(void);
uint8_t SystemDrv_isNTCAlert(void);
uint32_t SystemDrv_GetNTCValue(void);
#endif
#ifdef HAS_BATTERY_DETECT
void SystemDrv_PowerBatteryDetect(void);
uint16_t SystemDrv_BatteryADCValue(void);
uint32_t SystemDrv_getBatteryStateOfCharge(void);
#endif
#ifdef HAS_EXT_CHARGE_CTRL
void SystemDrv_ExternalChargeEnable(bool enable);
#endif
#ifdef HAS_BOOST_ENABLE
void SystemDrv_BoostEnable(bool enable);
#endif
#ifdef HAS_BAT_CHARGE_STATUS
void SystemDrv_InitBatteryChargeStatus(void);
void SystemDrv_CheckBatteryChargeStatus();
uint8_t SystemDrv_IsBatteryFullyCharged(void);
uint8_t SystemDrv_isBatteryError(void);
#endif
void SystemDrv_PowerEnable(bool enable);
#ifdef HAS_DSP_EN
void SystemDrv_DSPEnable(bool enable);
#endif
#ifdef HAS_BAT_CHARGE
void SystemDrv_BatteryChargeEnable(bool enable);
#endif
#ifdef HAS_MCU_SLEEP
void SystemDrv_PowerStopMode(void);
#endif
#ifdef UNIT_TEST_ENABLE
void SystemDrv_SetUnitTest(uint16 level);
#endif
#ifdef HAS_BAT_CHG_CURRENT_SELECT

void SystemDrv_BatteryChargeCurrentSelect(bool enable);
bool SystemDrv_GetBatteryChargeCurrentLevel(void);

#endif


typedef void (*SystemUpFunc_t)(void);

typedef struct tagSystemUpSequence{
    SystemUpFunc_t SystemUpStepFunc;
    int16_t delay_time;
} SystemUpSeq_t;


typedef struct tagSystemDriver
{
    uint8_t step;
} cSystemDrv;

/**
 * Construct the system driver instance.
 * initial the GPIO/ADC/... according to the attacheddevices.c
 * power off all the external power supply
 * @param me - instance of the driver
 * @return : none
 */
void    SystemDrv_Ctor(cSystemDrv *me);

/**
 * Exit & clean up the driver.
 * power off all the external power supply
 * @param me - instance of the driver
 */
void    SystemDrv_Xtor(cSystemDrv *me);

/*
 * update some status on normal mode
 */
void    SystemDrv_Update(cSystemDrv *me);

/*
 * update some status on standby mode
 */
void    SystemDrv_StandbyUpdate(cSystemDrv *me);

/*
 * implement the power up sequence
 * turn on power supply, reset the chip. etc...
 */
int16_t  SystemDrv_InitPower(cSystemDrv *me);

/*
 * power off the external power supply
 */
void    SystemDrv_PowerOff(cSystemDrv *me);

/* On exception handler, interrupt is disabled, we can not use BSP_BlockingDelayMs()
 * But this delay is not accurary
 */
void BSP_ExpBlockingDelayMs(uint32 ms);
#endif  // __SYSTEM_DRV_DEFAULT_H__
