#ifndef __SYSTEM_DRV_SOUNDWALL_H__
#define __SYSTEM_DRV_SOUNDWALL_H__

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
    SYSTEM_ERROR_STATUS_NONE,
    SYSTEM_ERROR_OVERHEAT_WARNING1,
    SYSTEM_ERROR_OVERHEAT_WARNING2,
    SYSTEM_ERROR_OVERHEAT_ERROR,
    SYSTEM_ERROR_AMP_FAULT,
    SYSTEM_ERROR_HARDWARE_ERROR,
    SYSTEM_ERROR_STATUS_MAX
}SystemErrorStatus_t;

#ifdef HAS_I2C_BUS_DETECT
#define I2C_ERROR_NONE      ((uint16_t)0x00)
#define I2C_ERROR_DSP       ((uint16_t)0x01)
#define I2C_ERROR_CODEC     ((uint16_t)0x02)
#define I2C_ERROR_LED       ((uint16_t)0x04)
#endif

typedef struct tagSystemPowerStatus
{
    SystemStatus_t  sys_status;    // system status
    PowerStage_t  pow_stage;     // power stage
#ifdef HAS_I2C_BUS_DETECT
    uint32_t i2c_error_status;
#endif
    uint32_t a2b_mode;
    uint32_t dsp_version;
    uint32_t total_nodes;
}SystemPowerStatus_t;

typedef struct tagSourceAudioInfo
{
	uint32_t	audio_input_on;
	uint32_t	audio_linked_cnt;
	uint32_t	audio_lost_cnt;
}SourceAudioInfo_t;

typedef enum tagNTCIndex {
    NTC_INDEX_START=0,
    NTC_INDEX_DSP=NTC_INDEX_START,
    NTC_INDEX_AMP,
    NTC_INDEX_PSU,
    NTC_INDEX_MAX
}NTCIndex_t;

typedef struct tagNTCInfo {
    uint16_t    adc_value;
    uint16_t    degree;
}NTCInfo_t;
    
#ifdef HAS_TEMPERATURE_MONITOR
#define SYSTEM_NTC_WARNING1_DEGREE  75
#define SYSTEM_NTC_WARNING2_DEGREE  85
#define SYSTEM_NTC_ERROR_DEGREE     90
typedef enum tagSystemNtcStatus
{
    SYSTEM_NTC_SAFE_LEVEL,
    SYSTEM_NTC_WARNING_LEVEL_1,     // fade in/out led, keep amp working
    SYSTEM_NTC_WARNING_LEVEL_2,     // falsh led, shut down amp
    SYSTEM_NTC_ERROR_LEVEL,         // shut down system, quick flash RED led
    SYSTEM_NTC_STATUS_MAX
}SystemNtcStatus_t;

NTCInfo_t * SystemDrv_GetNtcValue(void);
#endif

void SystemDrv_SetSystemStatus(SystemStatus_t sys_status);
uint8_t SystemDrv_GetSystemStatus(void);
void SystemDrv_NextPowerStage(void);
void SystemDrv_SetPowerStage(PowerStage_t pow_stage);
uint8_t SystemDrv_GetPowerStage(void);
#ifdef HAS_I2C_BUS_DETECT
void SystemDrv_SetI2cBusStatus(uint16_t error_status);
uint16_t SystemDrv_GetI2cBusStatus(void);
#endif
#ifdef HAS_HW_VERSION_TAG
#define HW_VERSION_PIN      ADC_PIN8
// ES1 : Voltage = 3.0V
#define HW_ES1_HIGH_THRESHOLD       3823
#define HW_ES1_LOW_THRESHOLD        3623
// ES2 : NO THIS STAGE
#define HW_ES2_HIGH_THRESHOLD       0
#define HW_ES2_LOW_THRESHOLD        0
// ES3 : NO THIS STAGE
#define HW_ES3_HIGH_THRESHOLD       0
#define HW_ES3_LOW_THRESHOLD        0
// EVT1 : pull-up:22K, pull-down:100k, Voltage=2.7V, dac-value=3356
#define HW_EVT1_HIGH_THRESHOLD      3436
#define HW_EVT1_LOW_THRESHOLD       3276
// EVT2 : pull-up:150K, pull-down:100k, Voltage=1.8V, dac-value=2233
#define HW_EVT2_HIGH_THRESHOLD      2303
#define HW_EVT2_LOW_THRESHOLD       2163
// DVT1 : pull-up:220K, pull-down:100k, Voltage=1.5V, dac-value=1861
#define HW_DVT1_HIGH_THRESHOLD      1920
#define HW_DVT1_LOW_THRESHOLD       1800
// DVT2 : pull-up:360K, pull-down:100k, Voltage=1.2V, dac-value=1490
#define HW_DVT2_HIGH_THRESHOLD      1550
#define HW_DVT2_LOW_THRESHOLD       1430
// PVT : pull-up:560K, pull-down:100k, Voltage=0.9V, dac-value=1117
#define HW_PVT_HIGH_THRESHOLD       1177
#define HW_PVT_LOW_THRESHOLD        1057
// MP1 : pull-up:0K, pull-down:NC, Voltage=0.6V, dac-value=745
#define HW_MP1_HIGH_THRESHOLD      795
#define HW_MP1_LOW_THRESHOLD       695
// MP2 : pull-up:NC, pull-down:0K, Voltage=0.3V, dac-value=372
#define HW_MP2_HIGH_THRESHOLD      400
#define HW_MP2_LOW_THRESHOLD       340

typedef enum tagHwVersion{
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
}HwVersion_t;
HwVersion_t SystemDrv_GetHWversion(void);
#endif
void SystemDrv_SetDspVersion(uint32_t ver);
uint32_t SystemDrv_GetDspVersion(void);
void SystemDrv_SetA2BMode(uint32_t mode);
uint32_t SystemDrv_GetA2BMode(void);
bool SystemDrv_A2BModeIsMaster(void);
bool SystemDrv_A2BModeIsStandalone(void);
bool SystemDrv_A2BModeIsSlave(void);
void SystemDrv_SetTotalNodes(uint32_t nodes);
uint32_t SystemDrv_GetTotalNodes(void);

void SystemDrv_ShutDownAmp(bool enable);
void SystemDrv_SetIdHandler(int32_t set_id, bool enable, uint32_t param);

typedef void (*SystemUpFunc_t)(void);

typedef struct tagSystemUpSequence{
    SystemUpFunc_t SystemUpStepFunc;
    int16_t delay_time;
}SystemUpSeq_t;


typedef struct tagSystemDriver
{
    uint8_t step;
}cSystemDrv;

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

void    SystemDrv_DeActiveInit(void);
void    SystemDrv_ActiveInit(void);

/* 
 * ERROR/WARNIGN LED status
 */
void    SystemDrv_ErrorLed(SystemErrorStatus_t err_status);

#endif  // __SYSTEM_DRV_SOUNDWALL_H__

