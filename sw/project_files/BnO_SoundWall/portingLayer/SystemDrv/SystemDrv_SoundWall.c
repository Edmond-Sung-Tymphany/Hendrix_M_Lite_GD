/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  System Driver Edition
                 -------------------------
                  SW Module Document
@file        SystemDrv.c
@brief       It's the system driver.
             the purpose is for the power up sequence, temperature sense handler.
@author      Viking Wang
@date        2015-11-10
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "commontypes.h"
#include "trace.h"
#include "bsp.h"
#include "attachedDevices.h"
#include "SettingSrv.h"
#include "I2cSrv.h"
#include "DebugSSrv.h"
#include "GpioDrv.h"
#include "AdcDrv.h"
#include "SystemSrv.h"
#include "SystemDrv.h"
#include "AudioDrv.h"
#ifdef HAS_IOE_LED
#include "IoeLedDrv.h"
#endif

#include "./SystemDrv_priv.h"

/*GPIO object*/
static cGpioDrv powerGpioDrv;

/* adc pin define */
#define ADC_IN_NTC_DSP_SENSE            ADC_PIN0
#define ADC_IN_NTC_AMP_SENSE            ADC_PIN1
#define ADC_IN_NTC_PSU_SENSE            ADC_PIN4
#define ADC_IN_HW_VERSION               ADC_PIN8
static cADCDrv adcDrv;

#ifdef HAS_TEMPERATURE_MONITOR
static SystemNtcStatus_t system_ntc_status=SYSTEM_NTC_SAFE_LEVEL;
static SystemNtcStatus_t system_ntc_prev_status=SYSTEM_NTC_SAFE_LEVEL;
#define NTC_DEGREE_STEP    ((float)12.5)
// NTC value instored.
static NTCInfo_t ntc_adc_value[NTC_INDEX_MAX];
NTCInfo_t * SystemDrv_GetNtcValue(void)
{
    return (NTCInfo_t *)ntc_adc_value;
}

static void SystemDrv_NtcDataAnalyse()
{
    int32_t i;
    NTCInfo_t *p_ntc;
    float ff;
    uint32_t max_degree=0;
    uint32_t combine_degree=0;
//    BRINGUP_printf("\n\r");
    p_ntc = ntc_adc_value;
    for(i=0; i<(int32_t)NTC_INDEX_MAX; i++)
    {
        ff = p_ntc->adc_value / NTC_DEGREE_STEP;
        ff += 0.5;
        p_ntc->degree = (uint16_t)ff;
        if( p_ntc->degree > 100 )
            p_ntc->degree = 100;
//        BRINGUP_printf("d%d=%d,", i, p_ntc->degree);
        if( p_ntc->degree > max_degree )
            max_degree = p_ntc->degree;
        combine_degree |= p_ntc->degree;
        combine_degree <<= 8;
        p_ntc ++;
    }

    // store all degree to setting server for A2B master read
    Setting_Set(SETID_NTC_INFO, (void *)&combine_degree);

    if( max_degree > SYSTEM_NTC_ERROR_DEGREE )
    {
        system_ntc_status = SYSTEM_NTC_ERROR_LEVEL;
    }
    else if( max_degree > SYSTEM_NTC_WARNING1_DEGREE )
    {
        system_ntc_status = SYSTEM_NTC_WARNING_LEVEL_2;
    }
    else if( max_degree > SYSTEM_NTC_WARNING2_DEGREE )
    {
        system_ntc_status = SYSTEM_NTC_WARNING_LEVEL_1;
    }
    else
    {
        system_ntc_status = SYSTEM_NTC_SAFE_LEVEL;
    }

    if( system_ntc_prev_status == SYSTEM_NTC_ERROR_DEGREE )
    {   // system ERROR, waiting until to WARNING_LEVEL_1
        if( system_ntc_status <= SYSTEM_NTC_WARNING_LEVEL_1 )
        {   // resume the power and unmute
            MAIN_POWER_ENABLE(powerGpioDrv);
            OP_SHUTDOWN_DISABLE(powerGpioDrv);
            AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
            AudioDrv_Mute(AUDIO_DSP_DACOUT_MUTE, TRUE);
            if( system_ntc_status == SYSTEM_NTC_WARNING_LEVEL_1 )
            {
                SystemDrv_ErrorLed(SYSTEM_ERROR_OVERHEAT_WARNING1);
            }
            else
            {
                SystemDrv_ErrorLed(SYSTEM_ERROR_STATUS_NONE);
            }
            system_ntc_prev_status = system_ntc_status;
        }
    }
    else
    {
        if( system_ntc_status != system_ntc_prev_status )
        {
            switch( system_ntc_status )
            {
            case SYSTEM_NTC_SAFE_LEVEL:
                SystemDrv_ErrorLed(SYSTEM_ERROR_STATUS_NONE);
                if( system_ntc_prev_status > SYSTEM_NTC_WARNING_LEVEL_2 )
                {   // resume amplifier
                    AudioDrv_Mute(AUDIO_AMP_MUTE, FALSE);
                    AudioDrv_Mute(AUDIO_DSP_DACOUT_MUTE, FALSE);
                }
                break;
            case SYSTEM_NTC_WARNING_LEVEL_1:
                SystemDrv_ErrorLed(SYSTEM_ERROR_OVERHEAT_WARNING1);
                if( system_ntc_prev_status > SYSTEM_NTC_WARNING_LEVEL_1 )
                {   // resume amplifier
                    AudioDrv_Mute(AUDIO_AMP_MUTE, FALSE);
                    AudioDrv_Mute(AUDIO_DSP_DACOUT_MUTE, FALSE);
                }
                break;
            case SYSTEM_NTC_WARNING_LEVEL_2:
                SystemDrv_ErrorLed(SYSTEM_ERROR_OVERHEAT_WARNING2);
                // mute amplifier
                AudioDrv_Mute(AUDIO_DSP_DACOUT_MUTE, TRUE);
                AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
                break;
            case SYSTEM_NTC_ERROR_LEVEL:
                SystemDrv_ErrorLed(SYSTEM_ERROR_OVERHEAT_ERROR);
                // critical ERROR
                // mute amplifier
                AudioDrv_Mute(AUDIO_DSP_DACOUT_MUTE, TRUE);
                AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
                // shut down 34V and OP
                OP_SHUTDOWN_ENABLE(powerGpioDrv);
                MAIN_POWER_DISABLE(powerGpioDrv);
                break;
            default:
                ASSERT(0);  // should not reach here
                break;
            }
        }
        system_ntc_prev_status = system_ntc_status;
    }        
}
#endif

#ifdef HAS_DC_IN_DETECT
static bool power_is_lost=FALSE;
static uint8_t power_lost_cnt=0;
#endif

static uint16_t system_update_cnt=0;
static uint16_t system_update_1s_cnt=0;

//
#define SYSTEM_WAKEUP_DEBOUNCE  5  // unit : 20ms
static uint16_t system_wakeup_cnt=0;

/* Private functions / variables. Declare and drivers here */
const static SystemUpSeq_t power_up_seq[] =
{
    { &SystemDrv_PowerOnStage,      100 },
    { &SystemDrv_ResetFinishStage,  60 },
    { &SystemDrv_PowerReadyStage,   20 },
};
static SystemPowerStatus_t system_power_status;

static bool system_amp_otw=FALSE;    // overheat warning
static bool system_amp_ote=FALSE;    // overheat/overcurrent ERROR
static void SystemDrv_AmpClipFaultHandler(void)
{
    // check ote first
    if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_AMP_FAULT) )
    {   // no error, then check warning
        if( system_amp_ote )
        {
            system_amp_ote = FALSE;
            SystemDrv_ShutDownAmp(FALSE);
            SystemDrv_ErrorLed(SYSTEM_ERROR_STATUS_NONE);
        }
/*
        else
        {
            if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_AMP_CLP) )
            {   // resume to normal 
                if( system_amp_otw )
                {
                    system_amp_otw = FALSE;
                    SystemDrv_ErrorLed(SYSTEM_ERROR_STATUS_NONE);
                    printf("\n\r Resume OTW.");
                }
            }
            else
            {
                if( ! system_amp_otw )
                {
                    system_amp_otw = TRUE;
                    SystemDrv_ErrorLed(SYSTEM_ERROR_STATUS_NONE);
                    printf("\n\r Enter OTW.");
                }
            }
        }
*/
    }
    else
    {   // amp error
        if( ! system_amp_ote )
        {
            system_amp_ote = TRUE;
            SystemDrv_ShutDownAmp(TRUE);
            SystemDrv_ErrorLed(SYSTEM_ERROR_AMP_FAULT);
        }
    }
}

void SystemDrv_SetSystemStatus(SystemStatus_t sys_status)
{
    ASSERT( sys_status < SYSTEM_STATUS_MAX );
    system_power_status.sys_status = sys_status;

#ifdef MCU_COMM_VIA_GPIO
    if( (sys_status == SYSTEM_STATUS_POWERING_UP) || (sys_status == SYSTEM_STATUS_WORKING) )
    {   // inform the next slave tile to wake up
        GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_A2B_OUTPUT_1);
        GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_A2B_OUTPUT_2);
    }
    else
    {
        GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_A2B_OUTPUT_1);
        GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_A2B_OUTPUT_2);
    }
#endif
}

uint8_t SystemDrv_GetSystemStatus(void)
{
    return system_power_status.sys_status;
}

void SystemDrv_NextPowerStage(void)
{
    system_power_status.pow_stage ++;
}

void SystemDrv_SetPowerStage(PowerStage_t pow_stage)
{
    ASSERT( pow_stage < POWER_STAGE_MAX );
    system_power_status.pow_stage = pow_stage;
}

uint8_t SystemDrv_GetPowerStage(void)
{
    return system_power_status.pow_stage;
}

#ifdef HAS_I2C_BUS_DETECT
void SystemDrv_SetI2cBusStatus(uint16_t error_status)
{
    system_power_status.i2c_error_status |= error_status;
}

uint16_t SystemDrv_GetI2cBusStatus(void)
{
    return (system_power_status.i2c_error_status);
}
#endif

#ifdef HAS_HW_VERSION_TAG
HwVersion_t SystemDrv_GetHWversion(void)
{
    HwVersion_t hw_ver=HW_VERSION_UNKNOWN;
    int32_t adc_sample;

    adc_sample = ADCDrv_GetData(&adcDrv, HW_VERSION_PIN);

//    printf("HW adc = %d\n\r", adc_sample);

    if( (adc_sample < HW_ES1_HIGH_THRESHOLD) && (adc_sample > HW_ES1_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_ES1;
    }
/*    else if( (adc_sample < HW_ES2_HIGH_THRESHOLD) && (adc_sample > HW_ES2_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_ES2;
    }
    else if( (adc_sample < HW_ES3_HIGH_THRESHOLD) && (adc_sample > HW_ES3_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_ES3;
    }
*/
    else if( (adc_sample < HW_EVT1_HIGH_THRESHOLD) && (adc_sample > HW_EVT1_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_EVT1;
    }
    else if( (adc_sample < HW_EVT2_HIGH_THRESHOLD) && (adc_sample > HW_EVT2_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_EVT2;
    }
    else if( (adc_sample < HW_DVT1_HIGH_THRESHOLD) && (adc_sample > HW_DVT1_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_DVT1;
    }
    else if( (adc_sample < HW_DVT2_HIGH_THRESHOLD) && (adc_sample > HW_DVT2_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_DVT2;
    }
    else if( (adc_sample < HW_PVT_HIGH_THRESHOLD) && (adc_sample > HW_PVT_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_PVT;
    }
    else if( (adc_sample < HW_MP1_HIGH_THRESHOLD) && (adc_sample > HW_MP1_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_MP1;
    }
    else if( (adc_sample < HW_MP2_HIGH_THRESHOLD) && (adc_sample > HW_MP2_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_MP2;
    }
    else
    {
        hw_ver = HW_VERSION_UNKNOWN;
    }

    return hw_ver;
}
#endif

void SystemDrv_SetDspVersion(uint32_t ver)
{
    system_power_status.dsp_version = ver;
}

uint32_t SystemDrv_GetDspVersion(void)
{
    return system_power_status.dsp_version;
}

void SystemDrv_SetA2BMode(uint32_t mode)
{
#ifdef ENABLE_ONLY_STANDALONE_MODE
    mode = A2B_MODE_STANDALONE;
#endif

    system_power_status.a2b_mode = mode;
}

uint32_t SystemDrv_GetA2BMode(void)
{
#ifdef ENABLE_ONLY_STANDALONE_MODE
    system_power_status.a2b_mode = A2B_MODE_STANDALONE;
#endif

    return system_power_status.a2b_mode;
}

bool SystemDrv_A2BModeIsMaster(void)
{
    return (system_power_status.a2b_mode == A2B_MODE_MASTER);
}

bool SystemDrv_A2BModeIsStandalone(void)
{
    return (system_power_status.a2b_mode == A2B_MODE_STANDALONE);
}

bool SystemDrv_A2BModeIsSlave(void)
{
    return (system_power_status.a2b_mode == A2B_MODE_SLAVE);
}

void SystemDrv_SetTotalNodes(uint32_t nodes)
{
    system_power_status.total_nodes = nodes;
}

uint32_t SystemDrv_GetTotalNodes(void)
{
    uint32_t total_nodes;
    
    if( SystemDrv_A2BModeIsMaster() )
        total_nodes = system_power_status.total_nodes;
    else
        total_nodes = 1;

    return total_nodes;
}

void SystemDrv_ShutDownAmp(bool enable)
{
    if( enable )
    {
        AMP_SHUTDOWN_ENABLE(powerGpioDrv);
    }
    else
    {
        AMP_SHUTDOWN_DISABLE(powerGpioDrv);
    }
}

/**
 * Construct the system driver instance.
 * initial the GPIO/ADC/... according to the attacheddevices.c
 * power off all the external power supply
 * @param me - instance of the driver
 * @return : none
 */
void SystemDrv_Ctor(cSystemDrv *me)
{
    /* Fill me in! */
    me->step = SYSTEM_INIT_STEP;

    /* ADC for reading battery and AC input status*/
    uint16 attached_device_index = 0;
    tDevice *pPowerDev = NULL;

    pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID, &attached_device_index);
    while(pPowerDev != NULL)
    {
        switch(pPowerDev->deviceType)
        {
            case GPIO_DEV_TYPE:
            {
                tGPIODevice *pPowerGPIOConf = (tGPIODevice*)pPowerDev;
                GpioDrv_Ctor(&powerGpioDrv,pPowerGPIOConf);
                break;
            }
            case ADC_DEV_TYPE:
            {
                tADCDevice *pPowerAdcConf = (tADCDevice*)pPowerDev;
                ADCDrv_Ctor(&adcDrv, pPowerAdcConf);
                ADCDrv_StartScanning(&adcDrv);    //  get the temprature and HW version here
                break;
            }
            default:
                break;
        }
        attached_device_index ++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID, &attached_device_index);
    }

    SystemDrv_PowerOff(me);   // power down the peripheral first

    system_power_status.a2b_mode = *(uint32_t *)Setting_Get(SETID_A2B_MODE);
    if( system_power_status.a2b_mode >= A2B_MODE_MAX )
    {
        system_power_status.a2b_mode = A2B_MODE_DEFAULT;
        Setting_Set(SETID_A2B_MODE, (void *)(&system_power_status.a2b_mode));
    }
    if( SystemDrv_A2BModeIsSlave() )
    {
        I2cSrv_Enable(TRUE);
    }
}

/**
 * Exit & clean up the driver.
 * power off all the external power supply
 * @param me - instance of the driver
 */
void SystemDrv_Xtor(cSystemDrv *me)
{
    me->step = SYSTEM_INIT_STEP;
    ADCDrv_Xtor(&adcDrv);
    SystemDrv_PowerOff(me);   // power down the peripheral
}

/*
 * update some status
 */
void SystemDrv_Update(cSystemDrv *me)
{
    (void)me;
#ifdef HAS_DC_IN_DETECT
    if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_POWER_LOST) ) // power on
    {
        power_lost_cnt = 0;
        if( power_is_lost )
        {
            AudioDrv_Mute(AUDIO_AMP_MUTE, FALSE);  // resume the amplifier
            power_is_lost = FALSE;
        }
    }
    else
    {
        power_lost_cnt ++;
        if( (power_lost_cnt & 0x07) == 0 )
        {   // mute the amplifier
            AudioDrv_Mute(AUDIO_AMP_MUTE, TRUE);
            power_is_lost = TRUE;
        }
    }
#endif

    system_update_cnt ++;
    if( system_update_cnt < 5 )
    {
        return ;
    }
    system_update_cnt = 0;

    // here is 100ms task
    
    system_update_1s_cnt ++;
    SystemDrv_AmpClipFaultHandler();
    if( system_update_1s_cnt < 9 )
    {
        return ;
    }
    system_update_1s_cnt = 0;

    // here is 1000ms task
#ifdef HAS_I2C_BUS_DETECT
    if( system_power_status.i2c_error_status )
    {
        extern void UsbSrv_SendString(char *msg);
        if( system_power_status.i2c_error_status & I2C_ERROR_DSP )
        {
            system_power_status.i2c_error_status |= (uint16_t)(~I2C_ERROR_DSP);
            ALWAYS_printf("\n\rDSP I2C Error.\n\r");
//            UsbSrv_SendString("\n\r[DSP I2C Error.]");
        }
        if( system_power_status.i2c_error_status & I2C_ERROR_CODEC )
        {
            system_power_status.i2c_error_status |= (uint16_t)(~I2C_ERROR_CODEC);
            ALWAYS_printf("\n\rCodec I2C Error.\n\r");
//            UsbSrv_SendString("\n\r[Codec I2C Error.]");
        }
        if( system_power_status.i2c_error_status & I2C_ERROR_LED )
        {
            system_power_status.i2c_error_status |= (uint16_t)(~I2C_ERROR_LED);
            ALWAYS_printf("\n\rLED I2C Error.\n\r");
//            UsbSrv_SendString("\n\r[LED I2C Error.]");
        }
        SystemDrv_ErrorLed(SYSTEM_ERROR_HARDWARE_ERROR);
    }
#endif

#ifdef HAS_TEMPERATURE_MONITOR
    ntc_adc_value[NTC_INDEX_AMP].adc_value = (uint16_t)ADCDrv_GetData(&adcDrv, ADC_IN_NTC_AMP_SENSE);
    ntc_adc_value[NTC_INDEX_DSP].adc_value = (uint16_t)ADCDrv_GetData(&adcDrv, ADC_IN_NTC_DSP_SENSE);
    ntc_adc_value[NTC_INDEX_PSU].adc_value = (uint16_t)ADCDrv_GetData(&adcDrv, ADC_IN_NTC_PSU_SENSE);
    ADCDrv_StartScanning(&adcDrv);    //  get the temprature and HW version here
    SystemDrv_NtcDataAnalyse();
//    BRINGUP_printf("\n\rNTC->[MAIN:%d][AMP:%d][PSU:%d]", \
//        ntc_adc_value[NTC_INDEX_DSP].degree, ntc_adc_value[NTC_INDEX_AMP].degree, ntc_adc_value[NTC_INDEX_PSU].degree);
#endif  // HAS_TEMPERATURE_MONITOR
}

/*
 * update some status on standby mode
 */
void SystemDrv_StandbyUpdate(cSystemDrv *me)
{
    (void)me;

#ifdef MCU_COMM_VIA_GPIO
    // check the wake up GPIO status
    if( SystemDrv_A2BModeIsSlave() )
    {
        if( (GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_A2B_INPUT_1) == 0) && \
            (GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_A2B_INPUT_2) == 0) )
        {
            system_wakeup_cnt ++;
            if( system_wakeup_cnt == SYSTEM_WAKEUP_DEBOUNCE )
            {
                SystemDrv_SetSystemStatus(SYSTEM_STATUS_POWERING_UP);
            }
        }
        else
        {
            system_wakeup_cnt = 0;
        }
    }
#endif
}

/*
 * implement the power up sequence
 * turn on power supply, reset the chip. etc...
 */
int16_t SystemDrv_InitPower(cSystemDrv *me)
{
    int16_t delay_time, total_steps;

    total_steps = sizeof(power_up_seq) / sizeof(power_up_seq[0]);
    if(me->step < total_steps)
    {
        /* call the power up function according to step*/
        power_up_seq[me->step].SystemUpStepFunc();
        /* get delay time*/
        delay_time = power_up_seq[me->step].delay_time;
        me->step ++;
    }
    else
    {
        // initial done, tell the System server.
        delay_time = 0;
    }

    return delay_time;
}

/*
 * power off the external power supply
 */
void SystemDrv_PowerOff(cSystemDrv *me)
{
    me->step = SYSTEM_INIT_STEP;    // since the xxx_Xtor is never called, reset the step here.
    SystemDrv_SetPowerStage(POWER_STAGE_POWER_OFF);

    SystemDrv_ShutDownAmp(TRUE);
    OP_SHUTDOWN_DISABLE(powerGpioDrv);
    SYS_PWR_DISABLE(powerGpioDrv);
    A2B_PWR_DISABLE(powerGpioDrv);
#ifdef HAS_I2C_BUS_DETECT
    system_power_status.i2c_error_status = I2C_ERROR_NONE;
#endif
}

void SystemDrv_DeActiveInit(void)
{
    system_update_cnt = 0;
    system_update_1s_cnt = 0;
    system_amp_otw = FALSE;
    system_amp_ote = FALSE;
}

void SystemDrv_ActiveInit(void)
{
    system_wakeup_cnt = 0;
    system_update_1s_cnt = 0;
    system_amp_otw = FALSE;
    system_amp_ote = FALSE;
}

static void SystemDrv_PowerOnStage(void)
{
    SystemDrv_ShutDownAmp(TRUE);
    OP_SHUTDOWN_DISABLE(powerGpioDrv);
    SYS_PWR_ENABLE(powerGpioDrv);
    if( ! SystemDrv_A2BModeIsStandalone() )
    {
        A2B_PWR_ENABLE(powerGpioDrv);
    }
    POWER_UP_RESET_CHIP_ENABLE(powerGpioDrv);
}

static void SystemDrv_ResetFinishStage(void)
{
    POWER_UP_RESET_CHIP_DISABLE(powerGpioDrv);
}

static void SystemDrv_PowerReadyStage(void)
{
    // delay for a while to wait the chip/module ready.
}

void SystemDrv_SetIdHandler(int32_t set_id, bool enable, uint32_t param)
{
    switch( set_id )
    {
        case SYSTEM_SET_ID_AMP_SD:
            SystemDrv_ShutDownAmp(enable);
            break;
        default:
            // something error
            ASSERT(0);
            break;
    }
}

/* 
 * ERROR/WARNIGN LED status
 */
void SystemDrv_ErrorLed(SystemErrorStatus_t err_status)
{
#ifdef HAS_IOE_LED
    switch( err_status )
    {
        case SYSTEM_ERROR_STATUS_NONE:
            IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_GREEN, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_BLUE, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);            
            break;
        case SYSTEM_ERROR_OVERHEAT_WARNING1: // 
            IoeLed_SetupMode(LED_ID_BLUE, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_BLINK_MODE_SLOW, 0x00);
            IoeLed_SetupMode(LED_ID_GREEN, IOE_LED_MODE_BLINK, LED_BLINK_MODE_APPEND, 0x00);
            break;
        case SYSTEM_ERROR_OVERHEAT_WARNING2:
            IoeLed_SetupMode(LED_ID_BLUE, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_FLASH_MODE_MEDIUM, 0x00);
            IoeLed_SetupMode(LED_ID_GREEN, IOE_LED_MODE_BLINK, LED_BLINK_MODE_APPEND, 0x00);
            break;
        case SYSTEM_ERROR_OVERHEAT_ERROR:
        case SYSTEM_ERROR_AMP_FAULT:
        case SYSTEM_ERROR_HARDWARE_ERROR:
            IoeLed_SetupMode(LED_ID_BLUE, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_GREEN, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
            IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_BLINK, LED_FLASH_MODE_FAST, 0x00);
            break;
        default :
            break;
    }
#endif
}

