/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  System Driver Edition
                 -------------------------
                  SW Module Document
@file        SystemDrv.c
@brief       It's the system driver.
             the purpose is for the power up sequence and battery charging control
@author      Viking Wang
@date        2015-07-25
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "commontypes.h"
#include "trace.h"
#include "bsp.h"
#include "attachedDevices.h"
#include "SettingSrv.h"
#include "DebugSSrv.h"
#include "GpioDrv.h"
#include "AdcDrv.h"
#include "SystemSrv.h"
#include "SystemSrv.config"
#include "SystemDrv.h"
#include "AudioDrv.h"

#include "./SystemDrv_priv.h"

/*GPIO object*/
static cGpioDrv powerGpioDrv;

/* The ADC config is not in devices[], so can't use getDeviceId for now, use extern for quick test*/
static cADCDrv adcDrv;

#ifdef HAS_DC_IN_DETECT
static bool power_is_lost=FALSE;
static uint8_t power_lost_cnt=0;
#endif

/* Private functions / variables. Declare and drivers here */
const static SystemUpSeq_t power_up_seq[] =
{
    { &SystemDrv_ResetStage,        60 },   // power on the external power supply and pull the "RESET pin" to low(reset on) if necessary.
    { &SystemDrv_PoweringUpStage,   60 },   // pull the "RESET pin" to high(reset off)
    { &SystemDrv_PowerReadyStage,   0 },  // tell the other server that the power and reset is ready for initial.
};

static SystemPowerStatus_t system_power_status;

void SystemDrv_SetSystemStatus(SystemStatus_t sys_status)
{
    ASSERT( sys_status < SYSTEM_STATUS_MAX );
    system_power_status.sys_status = (uint8_t)sys_status;
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
    system_power_status.pow_stage = (uint8_t)pow_stage;
}

uint8_t SystemDrv_GetPowerStage(void)
{
    return system_power_status.pow_stage;
}

#ifdef HAS_AUTO_STANDBY
void SystemDrv_SetAutoStandby(bool on)
{
    system_power_status.is_auto_standby = on;
}

bool SystemDrv_IsAutoStandby(void)
{
    return system_power_status.is_auto_standby;
}
#endif

#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
void SystemDrv_SystemRestart(uint16_t timeout)
{
    system_power_status.restart_timeout = timeout;
}
#endif

#ifdef HAS_I2C_BUS_DETECT
void SystemDrv_SetI2cBusStatus(uint16_t error_status)
{
    system_power_status.i2c_error_status = error_status;
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
    else if( (adc_sample < HW_ES2_HIGH_THRESHOLD) && (adc_sample > HW_ES2_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_ES2;
    }
    else if( (adc_sample < HW_ES3_HIGH_THRESHOLD) && (adc_sample > HW_ES3_LOW_THRESHOLD) )
    {
        hw_ver = HW_VERSION_ES3;
    }
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

#ifdef HAS_POWER_SWITCH_KEY
void SystemDrv_PowerSwitchUpdate(void)
{
    if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_POWER_KEY) )
        system_power_status.power_switch_on = POWER_SWITCH_ON;
    else
        system_power_status.power_switch_on = POWER_SWITCH_OFF;
}

uint8_t SystemDrv_IsPowerSwitchOn(void)
{
    return system_power_status.power_switch_on;
}
#endif

#ifdef QUICKLY_POWER_DOWN
void SystemDrv_SetAmpMuteEnable(bool enable)
{
    system_power_status.amp_mute_enable = enable;
}

bool SystemDrv_GetAmpMuteEnable(void)
{
    return system_power_status.amp_mute_enable;
}
#endif

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
//            ADCDrv_StartScanning(&adcDrv);    // let the other xxx_Ctor(e.g adc key) to start scanning, don't scan on powering up stage
                break;
            }
            default:
                break;
        }
        attached_device_index ++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID, &attached_device_index);
    }

    SystemDrv_PowerOff(me);   // power down the peripheral first
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
    if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_DC_IN_DET) ) // power on
    {
        power_lost_cnt = 0;
        if( power_is_lost )
        {
            AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, FALSE);  // resume the amplifier
            power_is_lost = FALSE;
        }
    }
    else
    {
        power_lost_cnt ++;
        if( (power_lost_cnt & 0x07) == 0 )
        {   // mute the amplifier
            AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);
            power_is_lost = TRUE;
        }
    }
#endif
#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
    if( system_power_status.restart_timeout > 0 )
    {
        system_power_status.restart_timeout -= SYSTEM_SRV_TIMEOUT_IN_MS;
        if( system_power_status.restart_timeout <= 0 )
        {
            BSP_SoftReboot();
        }
    }
#endif
#ifdef HAS_I2C_BUS_DETECT
    if( system_power_status.i2c_error_status )
    {
        if( system_power_status.i2c_error_status & I2C_ERROR_DSP )
        {
            ALWAYS_printf("\n\rDSP I2C Error.\n\r");
            DebugSSrv_AssertPrintf("[DSP I2C Error.]");
        }
        if( system_power_status.i2c_error_status & I2C_ERROR_AMP )
        {
            ALWAYS_printf("\n\rAMP I2C Error.\n\r");
            DebugSSrv_AssertPrintf("[AMP I2C Error.]");
        }
    }
#endif
}

/*
 * update some status on standby mode
 */
void SystemDrv_StandbyUpdate(cSystemDrv *me)
{
    (void)me;
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
    SYS_PWR_DISABLE(powerGpioDrv);
#ifdef HAS_I2C_BUS_DETECT
    system_power_status.i2c_error_status = I2C_ERROR_NONE;
#endif
}

static void SystemDrv_ResetStage(void)
{
    SystemDrv_ShutDownAmp(TRUE);
    SYS_PWR_ENABLE(powerGpioDrv);
}

static void SystemDrv_PoweringUpStage(void)
{
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

