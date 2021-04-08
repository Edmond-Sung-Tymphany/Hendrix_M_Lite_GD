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

#include "stm32f0xx_pwr.h"
#include "product.config"
#include "commontypes.h"
#include "trace.h"
#include "attachedDevices.h"
#include "SettingSrv.h"
#include "GpioDrv.h"
#include "AdcDrv.h"
#include "SystemSrv.config"
#include "SystemDrv.h"
#include "projbsp.h"
#include "bsp.h"

#include "./SystemDrv_priv.h"

/*GPIO object*/
static cGpioDrv powerGpioDrv;

/* The ADC config is not in devices[], so can't use getDeviceId for now, use extern for quick test*/
static cADCDrv adcDrv;

/* Private functions / variables. Declare and drivers here */
const static SystemUpSeq_t power_up_seq[] =
{
#ifdef HAS_BT_RESET_PIN
    { &SystemDrv_ResetStage,        60},   // power on the external power supply and pull the "RESET pin" to low(reset on) if necessary.
#else
    { &SystemDrv_ResetStage,        60 },   // power on the external power supply and pull the "RESET pin" to low(reset on) if necessary.
#endif    
    { &SystemDrv_PoweringUpStage, 20 },   // pull the "RESET pin" to high(reset off)
};

static SystemPowerStatus_t system_power_status;
static void SystemDrv_CheckTemperatureStatus(void);

#ifdef UNIT_TEST_ENABLE
static uint16 SystemDrv_GetUnitTestLevel(void);
#endif

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

#ifdef HAS_POWER_SWITCH_KEY
void SystemDrv_PowerSwitchUpdate(void)
{
    if( GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_POWER_KEY) )
        system_power_status.power_switch_on = POWER_SWITCH_OFF;
    else
        system_power_status.power_switch_on = POWER_SWITCH_ON;
}

uint8_t SystemDrv_IsPowerSwitchOn(void)
{
    return system_power_status.power_switch_on;
}
#endif

#ifdef HAS_DC_IN
void SystemDrv_PowerDCInDetect(void)
{
    int32 dcAdc;
    dcAdc = ADCDrv_GetData(&adcDrv, DC_IN_ADC_PIN);
    if (dcAdc > DC_IN_ADC_THRESHOLD)
    {
        system_power_status.dc_in_status = DC_IN_DETECTED;
    }
    else
    {
        system_power_status.dc_in_status = DC_IN_NOT_DETECTED;
    }
}

uint8_t SystemDrv_IsDCInDetected(void)
{
    return system_power_status.dc_in_status;
}
#endif

#ifdef HAS_BAT_CHARGE_STATUS
void SystemDrv_InitBatteryChargeStatus(void)
{
    system_power_status.batteryErrorCount = 0;
    system_power_status.loopCount = 0;
    system_power_status.isBatteryError =FALSE;
    system_power_status.isBatteryFullyCharged = FALSE;
}

uint8_t SystemDrv_IsBatteryFullyCharged(void)
{
    return (system_power_status.isBatteryFullyCharged);
}

uint8_t SystemDrv_isBatteryError(void)
{
    return (system_power_status.isBatteryError);
}

void SystemDrv_CheckBatteryChargeStatus(void)
{
    system_power_status.loopCount++;
    if (system_power_status.loopCount > CHECK_BATTERY_CHARGE_STATUS_TIME_ONE_SECOND)
    {
        system_power_status.loopCount = 0;
        if (GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_BAT_CHG_STATUS))
        {
            if (BATTERY_CHARGE_STATUS_HIGH == system_power_status.batteryChargeStatus)
            {
                system_power_status.batteryErrorCount = 0;                  /* reset */
                system_power_status.isBatteryFullyCharged = TRUE;
                system_power_status.isBatteryError = FALSE;
            }
            else
            {
                system_power_status.batteryErrorCount++;            /* to see if this pin toggled every 1 sec */
            }
            system_power_status.batteryChargeStatus = BATTERY_CHARGE_STATUS_HIGH;
        }
        else
        {
            if (BATTERY_CHARGE_STATUS_LOW == system_power_status.batteryChargeStatus)
            {
                system_power_status.batteryErrorCount = 0;                  /* reset */
                system_power_status.isBatteryFullyCharged = FALSE;
                system_power_status.isBatteryError = FALSE;
            }
            else
            {
                system_power_status.batteryErrorCount++;            /* to see if this pin toggled every 1 sec */
             }
             system_power_status.batteryChargeStatus = BATTERY_CHARGE_STATUS_LOW;
        }
    }

    if (system_power_status.batteryErrorCount >= BATTERY_CHARGE_STATUS_TOGGLE_TIMES)
    {
        system_power_status.batteryErrorCount  = BATTERY_CHARGE_STATUS_TOGGLE_TIMES;
        system_power_status.isBatteryError = TRUE;
    }

#ifdef UNIT_TEST_ENABLE
    if (SystemDrv_GetUnitTestLevel() == 5)
    {
        system_power_status.isBatteryError = TRUE;
    }
    else if (SystemDrv_GetUnitTestLevel() == 6)
    {
        system_power_status.isBatteryError = FALSE;
    }
#endif
}
#endif


#ifdef HAS_BATTERY_NTC
void SystemDrv_InitNTCStatus(void)
{
    system_power_status.NTC.value = NTC_ROOM_TEMPERATURE;
    system_power_status.NTC.TurnOffSystem = FALSE;
    system_power_status.NTC.TurnOffCharging = FALSE;
    system_power_status.NTC.Alert = FALSE;

    system_power_status.NTC.sample_peak = 0;
    system_power_status.NTC.sample_count = 0;
    system_power_status.NTC.average_sum = 0;
    system_power_status.NTC.average_count = 0;
    system_power_status.NTC.average_result = NTC_ROOM_TEMPERATURE;

#ifdef UNIT_TEST_ENABLE
    system_power_status.unit_test_level = 0;
#endif
    
}
/**
 *  SystemDrv_BatteryNTCValue, 30 * 50ms = 1500ms to update NTC
 *  Algorithm Description:
 *
 * @return void
 */
void SystemDrv_BatteryNTCValue(void)
{

    uint32 adc_sample;
    static uint32 counter = 0;

    counter ++;
    if (counter > 30)
    {
        counter = 0;
        adc_sample = ADCDrv_GetData(&adcDrv, BATTERY_NTC_PIN);
/*        ALWAYS_printf("NTC=%d\n", adc_sample); */
        system_power_status.NTC.value = adc_sample;
        SystemDrv_CheckTemperatureStatus();
    }
}

uint32_t SystemDrv_GetNTCValue(void)
{
    return (system_power_status.NTC.value);
}

uint8_t SystemDrv_IsNTCTurnOffSystem(void)
{
    return (system_power_status.NTC.TurnOffSystem);
}

uint8_t SystemDrv_isNTCTurnOffCharging(void)
{
    return (system_power_status.NTC.TurnOffCharging);
}

uint8_t SystemDrv_isNTCAlert(void)
{
    return (system_power_status.NTC.Alert);
}

#ifdef UNIT_TEST_ENABLE
void SystemDrv_SetUnitTest(uint16 level)
{
    system_power_status.unit_test_level = level;
}

static uint16 SystemDrv_GetUnitTestLevel(void)
{
    return (system_power_status.unit_test_level);
}
#endif


static void SystemDrv_CheckTemperatureStatus(void)
{

    /* Record the peak value in a sampling period */
    system_power_status.NTC.sample_peak = system_power_status.NTC.value;/*MAX(system_power_status.NTC.sample_peak, system_power_status.NTC.value);*/
    system_power_status.NTC.sample_count++;

    /* End of a sampling period  */
    if(system_power_status.NTC.sample_count == SYS_NTC_SAMPLE_NUM)
    {
        /* Record sampling period result in average period */
        system_power_status.NTC.average_sum += system_power_status.NTC.sample_peak;
        system_power_status.NTC.average_count++;

        /* Clean sampling period data */
        system_power_status.NTC.sample_peak = 0;
        system_power_status.NTC.sample_count = 0;
    }

    /* End of average period */
    if(system_power_status.NTC.average_count == SYS_NTC_AVERAGE_NUM)
    {
        /* Calculate average of peak values */
        system_power_status.NTC.average_result = system_power_status.NTC.average_sum / SYS_NTC_AVERAGE_NUM;

        /* Clean average period data */
        system_power_status.NTC.average_sum = 0;
        system_power_status.NTC.average_count = 0;
    }

#ifdef UNIT_TEST_ENABLE
    if (SystemDrv_GetUnitTestLevel() == 3)
    {
        system_power_status.NTC.average_result = NTC_CHARGING_OVERHEAT_THRESHOLD;
    }
    else if (SystemDrv_GetUnitTestLevel() == 4)
    {
        system_power_status.NTC.average_result = NTC_OVERHEAT_THRESHOLD;
    }
#endif
/*    ALWAYS_printf("average NTC=%d\n", system_power_status.NTC.average_result); */

    /* determine the NTC temperature range */
    if ( (NTC_SUPERCOOL_THRESHOLD <= system_power_status.NTC.average_result) ||
           (NTC_OVERHEAT_THRESHOLD >= system_power_status.NTC.average_result))
    {
            system_power_status.NTC.TurnOffSystem = TRUE;
            system_power_status.NTC.Alert = TRUE;
    }
    else if ((NTC_OVERHEAT_WARNING >= system_power_status.NTC.average_result) &&
            (DC_IN_NOT_DETECTED == system_power_status.dc_in_status))
    {
            system_power_status.NTC.Alert = TRUE;
    }
    else if ((((NTC_CHARGING_SUPERCOOL_THRESHOLD <= system_power_status.NTC.average_result)||
                (NTC_CHARGING_OVERHEAT_THRESHOLD >= system_power_status.NTC.average_result))) &&
                (DC_IN_DETECTED == system_power_status.dc_in_status))
    {
            system_power_status.NTC.TurnOffSystem = FALSE;
            system_power_status.NTC.TurnOffCharging = TRUE;
            system_power_status.NTC.Alert = TRUE;
    }

    /* return to normal */
    if ((NTC_NORMAL_LOWER_THRESHOLD >= system_power_status.NTC.average_result) &&
         (NTC_NORMAL_HIGHER_THRESHOLD <= system_power_status.NTC.average_result))
    {
        system_power_status.NTC.TurnOffSystem = FALSE;
        system_power_status.NTC.TurnOffCharging = FALSE;
        system_power_status.NTC.Alert = FALSE;
    }
    else if ((NTC_OVERHEAT_RETURN_NORMAL <= system_power_status.NTC.average_result) &&
            (DC_IN_NOT_DETECTED == system_power_status.dc_in_status))
    {
            system_power_status.NTC.Alert = FALSE;
    }

    /* less than 58C, do not allow to turn off */
    if (NTC_OVERHEAT_WARNING <= system_power_status.NTC.average_result) 
    {
        system_power_status.NTC.TurnOffSystem = FALSE;
    }
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

#ifdef HAS_BATTERY_DETECT
static void SystemDrv_InitPowerBattery(void)
{
    system_power_status.battery.StateOfCharge = ADC_CAPACITY_50;
    system_power_status.battery.sample_peak = 0;
    system_power_status.battery.sample_count = 0;
    system_power_status.battery.average_sum = 0;
    system_power_status.battery.average_count = 0;
    system_power_status.battery.average_result = ADC_CAPACITY_50;
    system_power_status.battery.instant_critical_counter = 0;
}

/**
 *  SystemDrv_PowerBatteryDetect
 *  Algorithm Description:
 *  Get peak vaule of a sampling preiod. Calculate average of a average period
 *  Average Period = Sampling Period * SYS_BATTERY_AVERAGE_NUM
 *
 * @return void
 */
void SystemDrv_PowerBatteryDetect(void)
{
    uint32 adc_sample;

    adc_sample = ADCDrv_GetData(&adcDrv, BATTERY_ADC_PIN);

    /* test */
    /*adc_sample = ADC_CAPACITY_5 -10;*/
    /* test */
    

    /* Record the peak value in a sampling period */
    system_power_status.battery.sample_peak = MAX(system_power_status.battery.sample_peak, adc_sample);
    system_power_status.battery.sample_count++;

    /* End of a sampling period  */
    if(system_power_status.battery.sample_count == SYS_BATTERY_SAMPLE_NUM)
    {
        /* Record sampling period result in average period */
        system_power_status.battery.average_sum += system_power_status.battery.sample_peak;
        system_power_status.battery.average_count++;

        /* Clean sampling period data */
        system_power_status.battery.sample_peak = 0;
        system_power_status.battery.sample_count = 0;
    }

    /* End of average period */
    if(system_power_status.battery.average_count == SYS_BATTERY_AVERAGE_NUM)
    {
        /* Calculate average of peak values */
        system_power_status.battery.average_result = system_power_status.battery.average_sum / SYS_BATTERY_AVERAGE_NUM;

        /* Clean average period data */
        system_power_status.battery.average_sum = 0;
        system_power_status.battery.average_count = 0;
    }

    /* update StateOfCharge */
    system_power_status.battery.StateOfCharge = system_power_status.battery.average_result;

    /* This is for emergency that the battery drop below than 6V */
    if (adc_sample <= ADC_CAPACITY_CRITICAL)
    {
        system_power_status.battery.instant_critical_counter++;
        if (system_power_status.battery.instant_critical_counter >= INSTANT_CRITICAL_COUNTER_THRESHOLD)
        {
            system_power_status.battery.StateOfCharge = CAPACITY_4;
            system_power_status.battery.instant_critical_counter = 0;
        }
    }
    else
    {
        if (system_power_status.battery.instant_critical_counter != 0)
        {
            system_power_status.battery.instant_critical_counter--;
        }
    }
}

uint16_t SystemDrv_BatteryADCValue(void)
{
    return system_power_status.battery.average_result;
}

uint32_t SystemDrv_getBatteryStateOfCharge(void)
{
    uint32_t reading;
    if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_5)
    {
        reading = CAPACITY_4;
    }
    else if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_10)
    {
        reading = CAPACITY_5+((system_power_status.battery.StateOfCharge-ADC_CAPACITY_5)*(CAPACITY_10-CAPACITY_5))/(ADC_CAPACITY_10-ADC_CAPACITY_5);
    }
    else if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_20)
    {
        reading = CAPACITY_10+((system_power_status.battery.StateOfCharge-ADC_CAPACITY_10)*(CAPACITY_20-CAPACITY_10))/(ADC_CAPACITY_20-ADC_CAPACITY_10);
    }
    else if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_80)
    {
        reading = CAPACITY_20+((system_power_status.battery.StateOfCharge-ADC_CAPACITY_20)*(CAPACITY_80-CAPACITY_20))/(ADC_CAPACITY_80-ADC_CAPACITY_20);
    }
    else if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_90)
    {
        reading = CAPACITY_80+((system_power_status.battery.StateOfCharge-ADC_CAPACITY_80)*(CAPACITY_90-CAPACITY_80))/(ADC_CAPACITY_90-ADC_CAPACITY_80);
    }
    else if (system_power_status.battery.StateOfCharge <= ADC_CAPACITY_100)
    {
        reading = CAPACITY_90+((system_power_status.battery.StateOfCharge-ADC_CAPACITY_90)*(CAPACITY_100-CAPACITY_90))/(ADC_CAPACITY_100-ADC_CAPACITY_90);
    }
    else
    {
        reading = CAPACITY_100;
    }

    Setting_Set(SETID_BATTERY_CAPACITY, &reading);
    return reading;
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
    SystemDrv_InitPowerBattery();
    SystemDrv_InitBatteryChargeStatus();
    SystemDrv_InitNTCStatus();
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

void SystemDrv_PowerEnable(bool enable)
{
    if (enable)
    {
        SYS_PWR_ENABLE(powerGpioDrv);
    }
    else
    {
        SYS_PWR_DISABLE(powerGpioDrv);
    }
}

#ifdef HAS_BOOST_ENABLE
void SystemDrv_BoostEnable(bool enable)
{
    if (enable)
    {
        BOOST_ENABLE(powerGpioDrv);
    }
    else
    {
        BOOST_DISABLE(powerGpioDrv);
    }
}
#endif

#ifdef HAS_DSP_EN
void SystemDrv_DSPEnable(bool enable)
{
    if (enable)
    {
        DSP_PWR_ENABLE(powerGpioDrv);
    }
    else
    {
        DSP_PWR_DISABLE(powerGpioDrv);
    }
}
#endif

#ifdef HAS_BAT_CHARGE
void SystemDrv_BatteryChargeEnable(bool enable)
{
    if (enable)
    {
        BAT_CHG_ENABLE(powerGpioDrv);
    }
    else
    {
        BAT_CHG_DISABLE(powerGpioDrv);
    }
}
#endif

#ifdef HAS_EXT_CHARGE_CTRL
void SystemDrv_ExternalChargeEnable(bool enable)
{
    if (enable)
    {
        EX_CHG_CTRL_ENABLE(powerGpioDrv);
    }
    else
    {
        EX_CHG_CTRL_DISABLE(powerGpioDrv);
    }
}
#endif

/*
 * power off the external power supply
 */
void SystemDrv_PowerOff(cSystemDrv *me)
{
    me->step = SYSTEM_INIT_STEP;    // since the xxx_Xtor is never called, reset the step here.
    SystemDrv_SetPowerStage(POWER_STAGE_POWER_OFF);

#ifdef HAS_BOOST_ENABLE
    BOOST_DISABLE(powerGpioDrv);
#endif

#ifdef  HAS_DSP_EN
    DSP_PWR_DISABLE(powerGpioDrv);
#endif
}

static void SystemDrv_ResetStage(void)
{
    SYS_PWR_ENABLE(powerGpioDrv);
#ifdef HAS_BOOST_ENABLE
    BOOST_ENABLE(powerGpioDrv);
#endif
#ifdef HAS_DSP_EN    
    DSP_PWR_ENABLE(powerGpioDrv);
#endif
}

static void SystemDrv_PoweringUpStage(void)
{
}

void SystemDrv_SetIdHandler(int32_t set_id, bool enable, uint32_t param)
{
}

/*
 * update some status on standby mode
 */
void SystemDrv_StandbyUpdate(cSystemDrv *me)
{
    (void)me;
}

#ifdef HAS_MCU_SLEEP
static void SystemDrv_EnablelWakeUpSources()
{
    SetEnableWakeupSources();
}

static void SystemDrv_EnableSystemTimerInt()
{
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}

static void SystemDrv_DisableSystemTimerInt()
{
    uint32 tmp = SysTick->CTRL;
    tmp = tmp & ~SysTick_CTRL_TICKINT_Msk;
    tmp = tmp & ~SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL  = tmp;
}

void SystemDrv_PowerStopMode(void)
{
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    SystemDrv_DisableSystemTimerInt();
    SystemDrv_EnablelWakeUpSources();
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    SystemDrv_EnableSystemTimerInt();
}
#endif

