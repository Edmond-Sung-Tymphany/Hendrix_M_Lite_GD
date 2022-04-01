/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for Hendrix M)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in Hendrix M
@author      Daniel Qin
@date        2015-01-23
@copyright (c) Tymphany Ltd. All rights reserved.
@modify history:
V0.2      by Colin Chen   2017-11-20    Modify for Hendrix M

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "trace.h"
#include "PowerSrv.h"
#include "PowerDrv_v2.h"
#include "SettingSrv.h"
#include "controller.h"
#include "I2CDrv.h"
#include "Timer.h"
#include "projBsp.h"
#ifdef HAS_PWR_IO_EXPANDER
#include "IoExpanderDrv.h"
#include "IoExpanderLedDrv.h"
#include "VIoExpanderGpioDrv.h"
#endif

/***********************************************
 * Definition
 ***********************************************/


#define BATT_ACTUAL_MAX_RSOC_PERCENT (93)
#define BATT_USER_FULL_RSOC_PERCENT (90)
#define HUNDRED_PERCENT (100)

#define FULL_CHARGE_VOLTAGE_MV (8250)

#define BATT_CAPACITY_LEVEL_DEBOUNCE_MS (60000)



/***********************************************
 * Global Variable
 ***********************************************/
/*GPIO object*/
cGpioDrv powerGpioDrv;

/*ADC object*/
cADCDrv adcDrv;

/*IO expander object */
#ifdef HAS_PWR_IO_EXPANDER
static cVIoExpanderGpioDrv PwrIoeDrv;
#endif

#ifdef HAS_PWR_IO_EXPANDER
static cIoExpanderDrv PwrIoeMbForSleepDrv;
static cIoExpanderDrv PwrIoeLedForSleepDrv;
#endif


/* Variables */
static eDcInSta      dcInStatus        = DC_IN_STA_MAX;
static const char *  hwVersion         = NULL;
#ifdef HAS_BATTERY_NTC
static eTempLevel   BattChgTempLevel = TL_NORMAL;
static eTempLevel   BattDischgTempLevel = TL_NORMAL;
#endif
static int32   dc_debouncer_start_ms         = 0;
#ifdef HAS_POWER_SWITCH_KEY
static ePwrSwSta  pwrSwStatus        = PWR_SW_MAX;
static int32   pwr_sw_key_debouncer_ms = 0;
#endif

#ifdef HAS_BATTERY
static tBatteryFilter battFilter =
{
    .count = 0,
    .isReady = FALSE,
    .intBatt.isRemovable = FALSE,
};
static eChargerState battChargeState_bk = CHARGER_STA_MAX;

static uint8    batt_CapacityResult = 255;

static uint8    batt_DCchanged;
static int16    batt_chgtimecounter = INVALID_VALUE;
static uint8    batt_chgtimerenable;

#endif /*end of HAS_BATTERY*/

static uint8 is_mute = 1;

/***********************************************
 * Array
 ***********************************************/
/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
    // To Do: need more stage
};

static uint8 PowerDrv_GetBattRsocUserFromAdcValues( uint16 adc_value);
// To Do:  need to added powerDownSequence

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void PowerDrv_Ctor(cPowerDrv *me)
{
    me->step = INITIAL_STEP;

    /* ADC for reading battery and AC input status*/
    uint16 attached_device_index = 0;
    tDevice *pPowerDev = NULL;

    PowerDrv_InitVariables(me);

    pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
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
                ADCDrv_StartScanning(&adcDrv);
                break;
            }
#ifdef HAS_PWR_IO_EXPANDER
            case IO_EXPANDER_DEV_TYPE:
            {
                tVIoeGpioDevice *pPwrIoeGpioDevice = (tVIoeGpioDevice*)pPowerDev;
                VIoExpanderGpioDrv_Ctor(&PwrIoeDrv,pPwrIoeGpioDevice);
                break;
            }
#endif

            default:
            {
                ASSERT(0);
                return;
            }
        }
        attached_device_index++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
    }

    PowerDrv_Set(me, POWER_SET_ID_CHARGER_ON, TRUE);
#ifdef HAS_HW_VERSION_TAG
    PowerDrv_SetHWversion();
#endif

}

void PowerDrv_Xtor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
    ADCDrv_Xtor(&adcDrv);
#ifdef HAS_PWR_IO_EXPANDER
    VIoExpanderGpioDrv_Xtor(&PwrIoeDrv);
#endif
    PowerDrv_DeinitialPower(me);
}


#ifdef HAS_PWR_IO_EXPANDER
void PowerDrv_IoeSetByID(cVIoExpanderGpioDrv* drv, uint8 index)
{
    VIoExpanderGpioDrv_SetGpio(drv, index);
}
void PowerDrv_IoeClearByID(cVIoExpanderGpioDrv* drv, uint8 index)
{
    VIoExpanderGpioDrv_ClearGpio(drv, index);
}
#endif

/****************************************************************************************************************
*****************************************************************************************************************/
#ifdef HAS_DSP_EN
void PowerDrv_DSPEnable(bool enable)
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


/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
uint16  PowerDrv_InitialPower(cPowerDrv *me)
{
    /*need to change uint16 to int32, but keep it for now as it's used in other project*/
    uint16 delayTime;
    if(me->step < Q_DIM(powerUpSequence))
    {
        /* call the power up function according to step*/
        powerUpSequence[me->step].powerUpFunction(me);
        /* get delay time*/
        delayTime = powerUpSequence[me->step].delaytime;
        me->step++;
    }
    else
    {
        /* if reset is completed, return 0 */
        delayTime = 0;
        me->step = 0;
    }
    return delayTime;
}


void PowerDrv_DeinitialPower(cPowerDrv *me)
{
    PowerDrv_PowerDownStage(me);
}

#ifdef HAS_PWR_IO_EXPANDER
static void PowerDrv_WaitWakeupStable(void)
{
    BSP_BlockingDelayMs(WAKE_UP_STABLE_TIME_MS);
}

static void PowerDrv_SetIoExpanderForSleep(void)
{
    VIoExpanderGpioDrv_Xtor(&PwrIoeDrv);
    // set IO Expander state for sleep
    IoExpanderDrv_Ctor_aw9523(&PwrIoeMbForSleepDrv, &ioeGpioMbSleepConfig);
    IoExpanderDrv_Ctor_aw9523(&PwrIoeLedForSleepDrv, &ledSleepConfig);

}

static void PowerDrv_SetIoExpanderForWakeup(void)
{
    PowerDrv_WaitWakeupStable();
    IoExpanderDrv_Xtor_aw9523(&PwrIoeMbForSleepDrv);
    IoExpanderDrv_Xtor_aw9523(&PwrIoeLedForSleepDrv);
    VIoExpanderGpioDrv_Ctor(&PwrIoeDrv,(tVIoeGpioDevice*)getDevicebyIdAndType(POWER_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL));
}
#endif

static void PowerDrv_OperationForSleep(cPowerDrv *me)
{
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
#ifdef HAS_PWR_IO_EXPANDER
    PowerDrv_SetIoExpanderForSleep();
#endif

    PowerDrv_DisableSystemTimerInt();
    SetEnableWakeupSources();
    PowerDrv_InitVariablesBeforeSleep(me);
    SetGpioStateForSleep();
}

static void PowerDrv_OperationForWakeUp(cPowerDrv *me)
{
#ifdef SOFT_REBOOT_WHEN_WAKE_UP_FROM_STANDBY
	//when wake up from standby just reset it
    BSP_SoftReboot();		//edmond_20210715
#else
	//after wakeup
    SetDisableWakeupSources();
    SetGpioStateForWakeup();
    PowerDrv_EnableSystemTimerInt();
    // re-detect pwrSwStatus
    pwrSwStatus = PWR_SW_MAX;

    /* It seems WFI set system clock to default,
     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
     */
    BSP_init_clock();
#ifdef HAS_PWR_IO_EXPANDER
    PowerDrv_SetIoExpanderForWakeup();
#endif
#endif



}


void PowerDrv_EnterSleepMode(cPowerDrv *me)
{
#ifdef HAS_IWDG
    IWDG_ReloadCounter();
    if(TRUE == IsEXTIWakeUp())
#endif
    {
        PowerDrv_OperationForSleep(me);
    }
#ifdef HAS_IWDG
    ResetEXTIWakeUpFlag();
#endif
    //sleep
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

#ifdef HAS_IWDG
    if(TRUE == IsEXTIWakeUp())
#endif
    {
#ifdef SOFT_REBOOT_WHEN_WAKE_UP_FROM_STANDBY
        BSP_init_clock();
    	printf("IsEXTIWakeUp(0x%x)\r\n", IsEXTIWakeUp());
        printf("PowerDrv_OperationForWakeUp\r\n");
#endif    

        PowerDrv_OperationForWakeUp(me);
    }
}


bool PowerDrv_power_switch(void)
{
    return ((bool)(pwrSwStatus));
}




#ifdef HAS_POWER_SWITCH_KEY
void PowerDrv_PwrSwitchUpdate(cPowerDrv *me)
{
    ePwrSwSta pwrSwStatus_now;
    if(IS_PWR_SW_KEY_ON(powerGpioDrv) == PWR_SW_ON_LEVEL)
    {
        
        pwrSwStatus_now = PWR_SW_ON;
    }
    else
    {
        pwrSwStatus_now = PWR_SW_OFF;
    }
    if(pwrSwStatus_now != pwrSwStatus)
    {
        if (0 == pwr_sw_key_debouncer_ms)
        {
            pwr_sw_key_debouncer_ms = getSysTime();
        }
        if ((getSysTime() - pwr_sw_key_debouncer_ms) >= PWR_SW_DEBOUNCE_TIME_MS)
        {
            TYMQP_LOG(NULL,"pwr_sw=%d", pwrSwStatus_now);
            pwrSwStatus = pwrSwStatus_now;
            Setting_Set(SETID_IS_PWR_SWITH_ON, &pwrSwStatus); // PWR SW change
            PowerDrv_ReportPowerState(me);
            pwr_sw_key_debouncer_ms = 0;
        }
    }
    else
    {
        pwr_sw_key_debouncer_ms = 0;
    }
}
#endif

static void PowerDrv_UpdateChargerState(cPowerDrv *me)
{
    uint8 chargerState = PowerDrv_GetChargerStatus();
    if(battChargeState_bk != chargerState)
    {
        battChargeState_bk = chargerState;
        PowerDrv_ReportBatteryStateChange(me);
    }
}



static void PowerDrv_UpdateChgTimeDC(uint8 dcstate, uint8 capacity)
{
    if(dcstate == TRUE &&
       capacity < DC_CHG_HIGEST_CAP_LIMIT)
    {
        batt_chgtimerenable = TRUE;
        batt_chgtimecounter = 0;
        TYMQP_LOG(NULL,"ChgBatt_Start");
    }
    else
    {
        batt_chgtimerenable = FALSE;
        batt_chgtimecounter = INVALID_VALUE;
        TYMQP_LOG(NULL,"ChgBatt_Stop");
    }
    batt_DCchanged = FALSE;
}

static void PowerDrv_UpdateChgCounter(void)
{
    if(batt_chgtimerenable)
    {
        batt_chgtimecounter++;
    }
    if(batt_chgtimecounter > DC_CHG_MAX_TIME_CNT)
    {
        batt_chgtimerenable = FALSE;
        batt_chgtimecounter = INVALID_VALUE;
    }
}

static uint8 PowerDrv_UpdateChgBattCapacity(uint8 adcCapacity)
{
    if(battChargeState_bk == CHARGER_STA_CHARGING_DONE)
    {
        batt_chgtimerenable = FALSE;
        batt_chgtimecounter = INVALID_VALUE;
        return DC_CHG_MAX_CAP;
    }

    uint8 capacity = batt_CapacityResult;
    uint16 battChgCounter;
    (capacity <= DC_CHG_LOW_LV_QUICK_CHG ) ? (battChgCounter = DC_CHG_LOW_LV_QCHG_TIME_CNT): \
    (battChgCounter = DC_CHG_PERCENT_TIME_CNT);

    if (batt_chgtimecounter > battChgCounter)
    {
        batt_chgtimecounter -= battChgCounter;
        capacity = DC_CHG_PERCENTAGE + *((uint8*)Setting_Get(SETID_DISPLAY_CAPACITY));
        TYMQP_LOG(NULL,"ChgBatt=%d,adc=%d", capacity,adcCapacity);
        if(capacity > adcCapacity)
        {
            capacity = adcCapacity;
            if(capacity == DC_CHG_MAX_CAP)
            {
                batt_chgtimerenable = FALSE;
                batt_chgtimecounter = INVALID_VALUE;
            }
        }
    }
    return capacity;
}

#ifdef HAS_DC_IN
eDcInSta PowerDrv_DCInDetect(void)
{
    int32 dcAdc;
    dcAdc = ADCDrv_GetData(&adcDrv, DC_IN_ADC_PIN);
    if (dcAdc > DC_IN_ADC_THRESHOLD)
    {
        return DC_IN_STA_ON;
    }
    else
    {
        return DC_IN_STA_OFF;
    }
}

void PowerDrv_UpdateDCIn(cPowerDrv *me)
{
    eDcInSta dcInStatus_now = (eDcInSta)PowerDrv_DCInDetect();
    if(dcInStatus_now != dcInStatus)
    {
        if (0 == dc_debouncer_start_ms)
        {
            dc_debouncer_start_ms = getSysTime();
        }

        if ((getSysTime() - dc_debouncer_start_ms) >= DC_DEBOUNCE_TIME_MS)
        {
            TYMQP_LOG(NULL,"dc=%d", dcInStatus_now);

            dcInStatus = dcInStatus_now;            
            Setting_Set(SETID_IS_DC_PLUG_IN, &dcInStatus); // DC Plug in

            batt_DCchanged = TRUE;

#ifdef HAS_AUTO_BOOST_CONTROL
            if(dcInStatus == DC_IN_STA_OFF)
            {
                BOOST_ENABLE(powerGpioDrv);
            }
            else
            {
                BOOST_DISABLE(powerGpioDrv);
            }
#endif
            PowerDrv_ReportBatteryStateChange(me);
        }
    }
    else
    {
        dc_debouncer_start_ms = 0;
    }

}
#endif


void PowerDrv_UpdateOne(cPowerDrv *me, bool sleepUpdate)
{
    /*--------------------------------------------------------------------
     *  per tick 50ms (POWER_SRV_TIMEOUT_IN_MS)
     *--------------------------------------------------------------------
     */
    /* update DC in status */
#ifdef HAS_DC_IN
    PowerDrv_UpdateDCIn(me);
#endif

    /* Power switch update */
#ifdef HAS_POWER_SWITCH_KEY
    PowerDrv_PwrSwitchUpdate(me);
#endif
    /* update charger status */
    PowerDrv_UpdateChargerState(me);

#ifdef HAS_BATTERY_NTC
    PowerDrv_UpdateTempADC(me);
#endif

    PowerDrv_UpdateVoltageValues(me);

    if(batt_DCchanged)
        PowerDrv_UpdateChgTimeDC(dcInStatus,*((uint8*)Setting_Get(SETID_DISPLAY_CAPACITY)));

    static uint32 timeStamp = 0;
    uint16 adcUpdatePeriod = POWER_ADC_UPDATE_PERIOD;
    if((getSysTime() - timeStamp) >= adcUpdatePeriod)
    {
        timeStamp = getSysTime();
        PowerDrv_UpdateBattADC(me);
    }

    PowerDrv_UpdateChgCounter();

}


void PowerDrv_Update(cPowerDrv *me)
{
    PowerDrv_UpdateOne(me, TRUE);
}


void PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable)
{
    switch (setId)
    {
        case POWER_SET_ID_FUEL_GAUGE_SHUT_DOWN:
        {
            // To Do: should be run to deep sleep here? shut down will disable MCU's power.

            /* If can not shutdown battery after 1 hour, reboot or assert */
            ASSERT(0);
            BSP_SoftReboot();

            break;
        }
        case POWER_SET_ID_OVERHEAT:
        {
            break;
        }
        case POWER_SET_ID_CHARGER_ON:
        {
            if (enable)
            {
                BAT_CHG_ENABLE(powerGpioDrv);
            }
            else
            {
                BAT_CHG_DISABLE(powerGpioDrv);
            }
            break;
        }
        case POWER_SET_ID_SLOW_CHARGER:
        {
            TYMQP_LOG(NULL,"charger speed %s", (enable?"SLOW":"FAST") );
            if (enable)
            {
                BAT_CHG_CUR_LOW(PwrIoeDrv); // Charger slow
            }
            else
            {
                BAT_CHG_CUR_HIGH(PwrIoeDrv);    // Charger fast
            }
            break;
        }
        case POWER_SET_ID_SHUT_DOWN:
        {
            TYMQP_LOG(NULL,"Set POWEREN %s", (enable?"OFF":"ON"));
            if(enable == TRUE)
            {
#ifdef SOFT_REBOOT_AFTER_CUT_OFF_POWER
                //printf("Cut power\n");
                SYS_PWR_DISABLE(powerGpioDrv);
                //while(1);
                //BSP_SoftReboot();		//edmond_20210715
#else
                
                SYS_PWR_DISABLE(powerGpioDrv);
                //BSP_BlockingDelayMs(1000);
                BSP_SoftReboot();		//edmond_20210715
#endif
            }
            else
            {
                SYS_PWR_ENABLE(powerGpioDrv);
            }
            break;
        }
        case POWER_SET_ID_AMP_SHUTDOWN:
        {
            if(enable == TRUE)
            {
                AMP_DISABLE(powerGpioDrv);
            }
            else
            {
                AMP_ENABLE(powerGpioDrv);
            }
            break;
        }
        case POWER_SET_ID_BOOST_ENABLE:
        {
#ifdef HAS_BOOST_ENABLE
            TYMQP_LOG(NULL,"Set BOOST %s", (enable?"ON":"OFF"));
            if(enable == TRUE)
            {
                BOOST_ENABLE(powerGpioDrv);
            }
            else
            {
                BOOST_DISABLE(powerGpioDrv);
            }
#endif
            break;
        }
        case POWER_SET_ID_RESET_CAPACITY:
        {
            uint32 adc_sample = ADCDrv_GetData(&adcDrv, BATTERY_ADC_PIN);
            uint8 capacity = PowerDrv_GetBattRsocUserFromAdcValues(adc_sample);
            TYMQP_LOG(NULL,"ResetCapacity:cap:%d,adc:%d",capacity,adc_sample);
            batt_CapacityResult = capacity;
            Setting_Set(SETID_BATTERY_CAPACITY,&batt_CapacityResult);
            Setting_Set(SETID_DISPLAY_CAPACITY,&batt_CapacityResult);
            Setting_Set(SETID_STORE_CAPACITY,&batt_CapacityResult);
            PowerDrv_ReportBatteryStateChange(me);
            break;
        }
        default:
            break;
    }
}


void PowerDrv_Get(cPowerDrv *me, ePowerGetId getId)
{
    switch(getId)
    {
        case POWER_GET_ID_CHARGER_STAT:
            PowerDrv_ReportBatteryStateChange(me);
            break;
        case POWER_GET_ID_PWR_SWITCH_STAT:
            break;

        default:
            break;
    }
}


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

/***************************** Power up process ***************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me)
{
    // To Do:  made the process at a right sequence .
    SYS_PWR_ENABLE(powerGpioDrv);

    //LS & POS don't have battery
    uint32 shopword = *(uint32*)Setting_Get(SETID_SHOP_MODE_WORD);
    if((shopword != SHOP_MODE_VALUE) &&
       (shopword != LS_SAMPLE_VALUE))
    {
#ifdef HAS_AUTO_BOOST_CONTROL
        if(dcInStatus == DC_IN_STA_ON)
        {
            BOOST_DISABLE(powerGpioDrv);
        }
        else
#endif
        {
            BOOST_ENABLE(powerGpioDrv);
        }
    }
    else
    {
        BOOST_DISABLE(powerGpioDrv);
    }

    DSP_PWR_ENABLE(powerGpioDrv);

    AMP_ENABLE(powerGpioDrv);
    
    /*Avoid pop noise when starting up*/
#ifdef HAS_TAS5760_AMP //Nick++ the define for old Amp
    if(is_mute)
    {
      AudioDrv_Mute( AUDIO_AMP_SOFT_MUTE, TRUE);
      is_mute= 0;
    }
#endif
}

/***************************** Power down process ***************************/

static void PowerDrv_PowerDownStage(cPowerDrv *me)
{
    // To Do:  made the process at a right sequence .

    BOOST_DISABLE(powerGpioDrv);

    DSP_PWR_DISABLE(powerGpioDrv);

    AMP_DISABLE(powerGpioDrv);


}

static void PowerDrv_DisableSystemTimerInt()
{
    uint32 tmp = SysTick->CTRL;
    tmp = tmp & ~SysTick_CTRL_TICKINT_Msk;
    tmp = tmp & ~SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL  = tmp;
}


static void PowerDrv_InitVariables(cPowerDrv *me)
{
    dc_debouncer_start_ms = 0;
    dcInStatus          = DC_IN_STA_MAX;
    Setting_Reset(SETID_IS_DC_PLUG_IN); // DC Plug in
#ifdef HAS_POWER_SWITCH_KEY
    Setting_Reset(SETID_IS_PWR_SWITH_ON); // power swith on
#endif
    PowerDrv_InitVariablesBeforeSleep(me);
}

static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me)
{
    dc_debouncer_start_ms = 0;
#ifdef HAS_POWER_SWITCH_KEY
    pwr_sw_key_debouncer_ms = 0;
#endif
    // To Do: init charging state
    PowerDrv_InitBattVariables(me);
}

/* Init battery variable when
 * 1) Bootup
 * 2) Battery I2C communicate fail
 * 3) After sleep
 */
static void PowerDrv_InitBattVariables(cPowerDrv *me)
{
    /* When wakeup or remove/re-insert battery, read again for every fileds
     */
    //After wakeup, battery/charger status need to update

#ifdef HAS_BATTERY_NTC
    BattChgTempLevel = TL_NORMAL;
    BattDischgTempLevel = TL_NORMAL;
#endif
}

static void PowerDrv_EnableSystemTimerInt()
{
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}


static eChargerState PowerDrv_GetChargerStatus()
{
    // To Do: chValue must have charger state renew
    uint8 chValue =  0;
    eChargerState chargeState  = CHARGER_STA_ERROR;
    /*if(IS_CHG_READY(powerGpioDrv) != CHG_READY)
    {
        return chargeState;
    }*/
    chValue = GET_CHARGER_STAT2(powerGpioDrv)<<1;
    chValue |= GET_CHARGER_STAT1(powerGpioDrv);
    switch(chValue)
    {
        case CHARGE_IN_PROCESS:
            chargeState = CHARGER_STA_CHARGING;
            break;
        case CHARGE_COMPLETE:
            chargeState = CHARGER_STA_CHARGING_DONE;
            break;
        default:
            /* charger error */
            break;
    }
    return chargeState;
}


static void PowerDrv_BattHandler(cPowerDrv *me, eBatteryInfoId currId)
{
    // To Do: Need to rework for Hendrix project
}

#ifdef HAS_BATTERY
static int16 PowerDrv_FindMaxValue(uint16* pArray, int16 size)
{
    int16 maxValue = 0;
    while(size)
    {
        if(maxValue< (*pArray))
        {
            maxValue = (*pArray);
        }
        size--;
        pArray ++;
    }
    return maxValue;
}

/**
* @brief: it's the battery filter which can filter out the voltage jitter whne playing strong music
* @Author: Johnny Fan
* @Note: below is the steps how it works:
*  1. when start the firmware for the first time , get the battery voltage quickly for battery indication
*  2. if the voltage sample is within oldvoltage +/- SAMPLE_RANGE_mV,
        then consider it as a valid sample and put it in the array. Otherwise discard it
*  3. if (OUT_OF_RANGE_ACCEPT_NUMBER) samples are out of range continuously, then consider it as valid
*  4. when we get BATT_FILTER_LEN number valid samples, find the max value and fill in intBatVoltageMaxValue[]
*  5. repeat step1~4, and when we get BATT_FILTER_LEN number max values in intBatVoltageMaxValue[],
        calculate its avarage value and consider it as a valid battery voltage, and publish it out
*  6. For external battery, there's insert/eject action in which the battery voltage will change quickly,
       so if we find it's inserted/ejected for (extBattActionDebounceCount) times, set the voltage as fininal battery voltage
*/
static bool PowerDrv_StartHigestFilter(tBattFilterCount* pBattFilterCount,
                                       tBattFilterValue* pBattFilterValue, uint16 sample)
{
    bool ret = FALSE;
    if (pBattFilterCount->isFirstStart)
    {
        pBattFilterCount->isFirstStart = FALSE;
        pBattFilterValue->filterResult = sample;
        ret = TRUE;
    }
    else
    {
        if(pBattFilterValue->isRemovable)
        {
            if (((sample < BATT_EJECT_mVOLT) && (pBattFilterCount->isInserted)) ||
                ((sample >= BATT_EJECT_mVOLT) && (!pBattFilterCount->isInserted)))
            {
                /* if we found there's extBattActionDebounceCount number eject/insert action */
                pBattFilterCount->actionDebounceCount++;
                if(pBattFilterCount->actionDebounceCount>OUT_OF_RANGE_ACCEPT_NUMBER)
                {
                    /* directly set it as battery voltage, and reset the filter*/
                    pBattFilterCount->actionDebounceCount = 0;
                    pBattFilterCount->isInserted = !pBattFilterCount->isInserted;
                    pBattFilterCount->sampleIndex = 0;
                    pBattFilterCount->maxValueIndex = 0;
                    pBattFilterCount->maxValueSum = 0;
                    pBattFilterValue->filterResult = sample;
                    ret  = TRUE;
                }
                else
                {
                    ret = FALSE;
                }
                return ret;
            }
        }
        pBattFilterCount->actionDebounceCount = 0;
        /* filter the internal battery*/
        if((sample <= (pBattFilterValue->filterResult + SAMPLE_RANGE_mV))&&
           (sample >= (pBattFilterValue->filterResult - SAMPLE_RANGE_mV)))
        {
            /* only get the sample within SAMPLE_RANGE_mV range*/

            pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
            pBattFilterCount->sampleIndex ++;
            pBattFilterCount->exceedRangeCount = 0;
        }
        else
        {
            /* if get intBattExceedRangeCount number out-of-range samples, then consider it as valid*/
            pBattFilterCount->exceedRangeCount++;
            if(pBattFilterCount->exceedRangeCount>=OUT_OF_RANGE_ACCEPT_NUMBER)
            {
                pBattFilterValue->sample[pBattFilterCount->sampleIndex] = sample;
                pBattFilterCount->sampleIndex ++;
                pBattFilterCount->exceedRangeCount = 0;
            }
        }
        if(pBattFilterCount->sampleIndex == BATT_FILTER_LEN)
        {
            /* if already get BATT_FILTER_LEN sample, then pick the max value to intBatVoltageMaxValue[]*/
            pBattFilterCount->sampleIndex = 0;
            pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex] =
                PowerDrv_FindMaxValue(pBattFilterValue->sample, BATT_FILTER_LEN);
            pBattFilterCount->maxValueSum += pBattFilterValue->maxValue[pBattFilterCount->maxValueIndex];
            pBattFilterCount->maxValueIndex++;
            if(pBattFilterCount->maxValueIndex == BATT_FILTER_LEN)
            {
                /* get the average of all the max value and consider it as battery voltage*/
                pBattFilterCount->maxValueIndex = 0;
                pBattFilterValue->filterResult = (pBattFilterCount->maxValueSum / BATT_FILTER_LEN);
                pBattFilterCount->maxValueSum = 0;
                ret  = TRUE;
            }
        }
    }
    return ret;
}

static bool PowerDrv_UpdateBattVoltage(uint16 intBatteryVol)
{
    bool ret = FALSE;
    static tBattFilterCount intBattFilterCount =
    {
        .isFirstStart = TRUE,
        .maxValueIndex = 0,
        .sampleIndex= 0,
        .exceedRangeCount = 0,
        .maxValueSum = 0,
    };
    ret = PowerDrv_StartHigestFilter(&intBattFilterCount, &battFilter.intBatt, intBatteryVol);

    return ret;
}

static uint16 PowerDrv_BattSmoothVoltage(uint16 reading)
{
    static bool firstStart=true;
    static int16 trend=0;
    static uint16 result;
    if(firstStart == TRUE)
    {
        result = reading;
        trend = 0;
        firstStart = FALSE;
    }
    else
    {
        int16 old = (int16)result;
        if(reading > old)
        {
            result = ((SMOOTH_OLD_RISE_SAMPLE_COUNT * old)+ (SMOOTH_CURR_RISE_SAMPLE_COUNT * reading) + trend + 3) / SMOOTH_RISE_ALL_COUNT;
        }
        else
        {
            result = ((SMOOTH_OLD_DROP_SAMPLE_COUNT * old)+ (SMOOTH_CURR_DROP_SAMPLE_COUNT * reading) + trend + 3) / SMOOTH_DROP_ALL_COUNT;
        }
        trend = (trend + (int16)(result - old)) / 2;
    }
    if(result > (0xFFFF)/2)
    {
        result = reading;
        trend = 0;
    }
    return result;
}

#endif /* end of HAS_BATTERY*/

static uint8 PowerDrv_GetBattRsocUserFromAdcValues( uint16 adc_value)
{
    uint8 ret = 0;
    uint8 i;
    uint16 battCapVol = ADC_TO_mVOLT(adc_value);
    // To Do: change value to RsocUser base on battery cavicity table.
    if(battCapVol >= BATT_VOLTAGE_LEVEL_FULL)
    {
        ret = 100;
    }
    else
    {
        for(i=0; i<battCapacityTableMaxNum; i++)
        {
            if(battCapVol >= battCapacityTable[i])
            {
                ret = (uint8)(((uint16)battCapacityTableMaxNum - i)*100/battCapacityTableMaxNum);
                break;
            }

        }

    }

    return ret;
}

static void PowerDrv_UpdateBattADC(cPowerDrv *me)
{
    uint32 adc_sample = ADCDrv_GetData(&adcDrv, BATTERY_ADC_PIN);
    uint8 currBattCapacity;
    uint8 chgBattCapacity;
    uint8 currBattStatus;
    uint8 currBTBattSta;
    bool fReport = FALSE;

#ifdef HAS_BATTERY
    if(PowerDrv_UpdateBattVoltage((uint16)adc_sample) == TRUE)
    {
        uint16 filterResult = PowerDrv_BattSmoothVoltage(battFilter.intBatt.filterResult);
        currBattCapacity = PowerDrv_GetBattRsocUserFromAdcValues(filterResult);
        //TYMQP_LOG(NULL,"BattADC: %d cap: %d Capbk:%d :volt :%d",filterResult,currBattCapacity,batt_CapacityResult,ADC_TO_mVOLT(adc_sample));

        //save the actual data for debug
        if(currBattCapacity != batt_CapacityResult)
        {
            Setting_Set(SETID_BATTERY_CAPACITY, &currBattCapacity);
            //first time report battery info
            if(!Setting_IsReady(SETID_DISPLAY_CAPACITY))
            {
                uint8 store_capacity = *((uint8*)Setting_Get(SETID_STORE_CAPACITY));
                (store_capacity > DC_CHG_MAX_CAP)?(batt_CapacityResult = currBattCapacity):(batt_CapacityResult = store_capacity);
                // if current battery lv is ok, but restored battery lv very low,then we let restored battery as 10pa, so system can startup
                if( currBattCapacity > DC_CHG_LOW_INIT_LV && store_capacity < DC_CHG_LOW_INIT_LV )
                {
                    batt_CapacityResult = DC_CHG_LOW_INIT_LV;
                }
                TYMQP_LOG(NULL,"RestoreBattCap %d",batt_CapacityResult);
                fReport = TRUE;
            }
        }
        if(currBattCapacity < batt_CapacityResult)
        {
            batt_CapacityResult = currBattCapacity;
            fReport = TRUE;
        }

        chgBattCapacity = PowerDrv_UpdateChgBattCapacity(currBattCapacity);
        if(chgBattCapacity != batt_CapacityResult)
        {
            batt_CapacityResult = chgBattCapacity;
            fReport = TRUE;
        }

    }
#endif

    if(fReport)
    {
        Setting_Set(SETID_DISPLAY_CAPACITY, &batt_CapacityResult);
        currBattStatus = PowerDrv_GetBatteryStatus(batt_CapacityResult);
        Setting_Set(SETID_BATTERY_STATUS, &currBattStatus);
        currBTBattSta = PowerDrv_GetBTBattStatus(batt_CapacityResult);
        Setting_Set(SETID_BT_BATT_STATUS, &currBTBattSta);
        TYMQP_LOG(NULL,"BATT STA:%d,BT STA:%d",currBattStatus,currBTBattSta);
        PowerDrv_ReportBatteryStateChange(me);
    }

}

static BatteryStatus PowerDrv_GetBTBattStatus(uint16 battCapacity)
{
    BatteryStatus battLevel= BTBatteryStatus_Lv_0;
#ifdef HAS_BT_BATT_CMD
    if (battCapacity <= batt_bt_lv_1)
    {
        battLevel = BTBatteryStatus_Lv_1;
    }
    else if (battCapacity <= batt_bt_lv_2)
    {
        battLevel = BTBatteryStatus_Lv_2;
    }
    else if (battCapacity <= batt_bt_lv_3)
    {
        battLevel = BTBatteryStatus_Lv_3;
    }
    else if (battCapacity <= batt_bt_lv_4)
    {
        battLevel = BTBatteryStatus_Lv_4;
    }
    else if (battCapacity <= batt_bt_lv_5)
    {
        battLevel = BTBatteryStatus_Lv_5;
    }
    else if (battCapacity <= batt_bt_lv_6)
    {
        battLevel = BTBatteryStatus_Lv_6;
    }
    else
    {
        ASSERT(0);
    }
#endif
    return battLevel;
}


static BatteryStatus PowerDrv_GetBatteryStatus(uint16 battCapacity)
{
    BatteryStatus battLevel= BatteryStatus_NO_BATTERY;

    if (battCapacity <= batt_capacity_critical)
    {
        battLevel = BatteryStatus_LEVEL_CRITICAL;
    }
    else if (battCapacity <= batt_capacity_low)
    {
        battLevel = BatteryStatus_LEVEL_LOW;
    }
    else if (battCapacity <= batt_capacity_mid)
    {
        battLevel = BatteryStatus_LEVEL_MID;
    }
    else if (battCapacity <= batt_capacity_extra)
    {
        battLevel = BatteryStatus_LEVEL_EXTRA;
    }
    else if (battCapacity <= batt_capacity_high)
    {
        battLevel = BatteryStatus_LEVEL_HIGH;
    }
    else
    {
        ASSERT(0);
    }

    return battLevel;
}


static void PowerDrv_ReportBatteryStateChange(cPowerDrv *me)
{
    PowerSrvInfoEvt *evt = Q_NEW(PowerSrvInfoEvt, POWER_BATT_STATE_SIG);
    ASSERT(evt);
// To Do: need to rework it.
    evt->dcInStatus= dcInStatus;
    evt->batteryInfo.chargerState = PowerDrv_GetChargerStatus();
    Setting_Set(SETID_CHARGING_STATUS, &evt->batteryInfo.chargerState);
    TYMQP_LOG(NULL,"chg sta %d", evt->batteryInfo.chargerState);
#ifdef HAS_BATTERY_NTC
    Setting_Set(SETID_BATTERY_CHARGE_TEMP_LEVEL, &BattChgTempLevel);
    Setting_Set(SETID_BATTERY_DISCHARGE_TEMP_LEVEL, &BattDischgTempLevel);
#endif

    if( Setting_IsReady(SETID_BATTERY_STATUS) )
    {
        evt->batteryInfo.battStatus= (BatteryStatus)*(uint8*)Setting_Get(SETID_BATTERY_STATUS);
    }
    else
    {
        evt->batteryInfo.battStatus = BatteryStatus_NO_BATTERY;
    }
    QF_PUBLISH(&evt->super, NULL);
}

static void PowerDrv_ReportPowerState(cPowerDrv *me)
{
    PowerSrvInfoEvt *evt = Q_NEW(PowerSrvInfoEvt, POWER_STATE_SIG);
    ASSERT(evt);
    QF_PUBLISH(&evt->super, NULL);
}

static void PowerDrv_UpdateVoltageValues(cPowerDrv *me)
{

    uint32 adc_sample;
    adc_sample = ADCDrv_GetData(&adcDrv, BATTERY_ADC_PIN);

    bool value;
    (adc_sample > ZERO_BATTERY_THERSHOLD)?(value = FALSE):(value = TRUE);
    if(*(bool *)Setting_Get(SETID_IS_BATT_VOL_ZERO) != value)
        Setting_Set(SETID_IS_BATT_VOL_ZERO, &value);


    static bool isAllowChgPowerUp_bk = FALSE;
    static bool isAllowPowerUp_bk    = FALSE;

    //transfer adc to voltage
    adc_sample = ADC_TO_mVOLT(adc_sample);
    if( TRUE ==  *(bool*)Setting_Get(SETID_IS_DC_PLUG_IN))
    {
        if(!isAllowChgPowerUp_bk &&
           (adc_sample >= ENABLE_CHGPOWERINGUP_FROM_CRITICAL))
        {
            isAllowChgPowerUp_bk = TRUE;
            Setting_Set(SETID_ALLOW_CHG_PWRUP,&isAllowChgPowerUp_bk);
            TYMQP_LOG(NULL,"SETID_ALLOW_CHG_PWRUP=%d,%d", isAllowChgPowerUp_bk,adc_sample);
        }
    }
    else
    {
        if(isAllowChgPowerUp_bk)
        {
            isAllowChgPowerUp_bk = FALSE;
            Setting_Set(SETID_ALLOW_CHG_PWRUP,&isAllowChgPowerUp_bk);
        }
        if(!isAllowPowerUp_bk &&
           adc_sample >= BATT_VOLTAGE_LEVEL_0 )
        {
            isAllowPowerUp_bk = TRUE;
            Setting_Set(SETID_ALLOW_POWERUP,&isAllowPowerUp_bk);
            TYMQP_LOG(NULL,"SETID_ALLOW_POWERUP=%d,%d", isAllowPowerUp_bk,adc_sample);
        }
        else if(isAllowPowerUp_bk &&
                adc_sample < ENABLE_POWERINGUP_FROM_CRITICAL)
        {
            isAllowPowerUp_bk = FALSE;
            Setting_Set(SETID_ALLOW_POWERUP,&isAllowPowerUp_bk);
            TYMQP_LOG(NULL,"SETID_ALLOW_POWERUP=%d,%d", isAllowPowerUp_bk,adc_sample);
        }
    }

}

static uint16 PowerDrv_MapSetidToNTCValue(uint8 id)
{
#define NTC_TEST_NORNAL_VALUE        1600
#define NTC_TEST_STEP                50
    static uint16 value;
    switch(id)
    {
        case NTC_TEST_DECREASE:
            value -= NTC_TEST_STEP;
            break;
        case NTC_TEST_INCREASE:
            value += NTC_TEST_STEP;
            break;
        case NTC_TEST_NORMAL:
            value = NTC_TEST_NORNAL_VALUE;
            break;
        default:
            break;
    }
    uint8 reset = NTC_TEST_WAITING;
    Setting_Set(SETID_NTC_TEST_VALUE, &reset);
    return value;
}


#ifdef HAS_BATTERY_NTC
static void PowerDrv_UpdateTempADC(cPowerDrv *me)
{
    uint16 tempAdc = ADCDrv_GetData(&adcDrv, BATTERY_NTC_PIN);

    if(*(uint8*)Setting_Get(SETID_NTC_TEST_VALUE) != (uint8)INVALID_VALUE)
    {
       tempAdc =  PowerDrv_MapSetidToNTCValue(*(uint8*)Setting_Get(SETID_NTC_TEST_VALUE));
    }

    bool value;
    (tempAdc < NO_BATTERY_NTC_THERSHOLD)?(value = FALSE):(value = TRUE);
    Setting_Set(SETID_IS_BATT_NTC_REMOVED, &value);

    uint16 mVolt = ADC_TO_mVOLT(tempAdc);
    static uint8 count;
    if(count > NTC_PRINT_COUNT)
    {
        count = 0;
        //TYMQP_LOG(NULL,"NTCvolt:%d", mVolt);
    }
    else
        count++;


    if(PowerDrv_GetNtcTempLevel(mVolt))
    {
        PowerDrv_ReportBatteryStateChange(me);
    }
}

static bool PowerDrv_IsNtcTempLevelRise(uint16 currValue, uint16 threshold, uint8* sampleCount)
{
    /* NTC Voltage fall when Temp rise */
    bool res = FALSE;
    if(currValue <= threshold)
    {
        (*sampleCount)++;
        if(*sampleCount >= PWR_NTC_SAMPLE_NUM)
        {
            *sampleCount = 0;
            res = TRUE;
        }
    }
    else
    {
        *sampleCount = 0;
    }
    return res;
}

static bool PowerDrv_IsNtcTempLevelFall(uint16 currValue, uint16 threshold, uint8* sampleCount)
{
    /* NTC Voltage rise when Temp fall */
    bool res = FALSE;
    if(currValue >= threshold)
    {
        (*sampleCount)++;
        if(*sampleCount >= PWR_NTC_SAMPLE_NUM)
        {
            *sampleCount = 0;
            res = TRUE;
        }
    }
    else
    {
        *sampleCount = 0;
    }
    return res;
}

static bool PowerDrv_GetNtcTempLevel(uint16 ntcValue)
{
    // To Do: convert ntc to temp level here
    bool res = FALSE;
    /* Charging temp level detect  */
    static uint8 chgSampleCount1 = 0;
    static uint8 chgSampleCount2 = 0;
    switch(BattChgTempLevel)
    {
        case TL_SERIOUS:
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_CHARGING_OVERHEAT_RETURN_THRESHOLD,&chgSampleCount1))
            {
                BattChgTempLevel = TL_NORMAL;
                res = TRUE;
            }
            break;
        case TL_NORMAL:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_CHARGING_OVERHEAT_THRESHOLD,&chgSampleCount1))
            {
                BattChgTempLevel = TL_SERIOUS;
                res = TRUE;
            }
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_CHARGING_SUPERCOOL_THRESHOLD,&chgSampleCount2))
            {
                BattChgTempLevel = TL_CRITICAL_COLD;
                res = TRUE;
            }
            break;
        case TL_CRITICAL_COLD:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_CHARGING_SUPERCOOL_RETURN_THRESHOLD,&chgSampleCount1))
            {
                BattChgTempLevel = TL_NORMAL;
                res = TRUE;
            }
            break;
        default:
            BattChgTempLevel = TL_NORMAL;
            break;
    }


    /* Discharging temp level detect */
    static uint8 dischgSampleCount1 = 0;
    static uint8 dischgSampleCount2 = 0;
    switch(BattDischgTempLevel)
    {
        case TL_CRITICAL:
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_OVERHEAT_RETURN_NORMAL_THRESHOLD,&dischgSampleCount1))
            {
                BattDischgTempLevel = TL_SERIOUS;
                res = TRUE;
            }
            break;
        case TL_SERIOUS:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_OVERHEAT_THRESHOLD,&dischgSampleCount1))
            {
                BattDischgTempLevel = TL_CRITICAL;
                res = TRUE;
            }
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_SERIOUS_HIGH_RETURN_THRESHOLD,&dischgSampleCount2))
            {
                BattDischgTempLevel = TL_NORMAL;
                res = TRUE;
            }
            break;
        case TL_NORMAL:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_SERIOUS_HIGH_THRESHOLD,&dischgSampleCount1))
            {
                BattDischgTempLevel = TL_SERIOUS;
                res = TRUE;
            }
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_SUBNORMAL_THERSHOLD,&dischgSampleCount2))
            {
                BattDischgTempLevel = TL_SUBNORMAL;
                res = TRUE;
            }
            break;
        case TL_SUBNORMAL:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_SUBNORMAL_RETURN_THERSHOLD,&dischgSampleCount1))
            {
                BattDischgTempLevel = TL_NORMAL;
                res = TRUE;
            }
            if(PowerDrv_IsNtcTempLevelFall(ntcValue,NTC_SUPERCOOL_THRESHOLD,&dischgSampleCount2))
            {
                BattDischgTempLevel = TL_CRITICAL_COLD;
                res = TRUE;
            }
            break;
        case TL_CRITICAL_COLD:
            if(PowerDrv_IsNtcTempLevelRise(ntcValue,NTC_SUPERCOOL_RETURN_THRESHOLD,&dischgSampleCount1))
            {
                BattDischgTempLevel = TL_SUBNORMAL;
                res = TRUE;
            }
            break;
        default:
            BattDischgTempLevel = TL_NORMAL;
            break;
    }

    return res;
}

#endif /*end of HAS_BATTERY_NTC*/

void PowerDrv_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel)
{
    eTempLevel tempLevelNew;
    int16 i;

    //config check
    ASSERT( numLevel==TL_NUM );

    if( *pTemp < tempNew  ) //raise curve
    {
        //Check if new curve cross over "lower bound"
        for (i = 0; i < numLevel; i++)
        {
            if ( *pTemp<=levels[i].lower && levels[i].lower<=tempNew  )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i >= numLevel)
        {
            tempLevelNew= *pLevel;
        }
    }
    else //down curve (include equal)
    {
        //Check if new curve cross over "lower bound"
        for (i = numLevel-1; i >=0; i--)
        {
            if ( *pTemp>=levels[i].upper && levels[i].upper >= tempNew )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i < 0)
        {
            tempLevelNew= *pLevel;
        }
    }

    ASSERT(tempLevelNew<TL_NUM);

    //Store result
    *pTemp= tempNew;
    *pLevel= tempLevelNew;
}

#ifdef HAS_HW_VERSION_TAG
static HwVersion_t PowerDrv_GetHWversion(void)
{
    HwVersion_t hw_ver=HW_VERSION_UNKNOWN;
    int32 adc_sample;
    adc_sample = ADCDrv_GetData(&adcDrv, HW_VERSION_ADC_PIN);

    uint16 mVolt;
    mVolt = ADC_TO_mVOLT(adc_sample);

    int8 i;
    for(i=0; i<HW_VERSION_MAX; i++)
    {
        if(mVolt >= hwVerMap[i].volThreshold)
        {
            hw_ver =  hwVerMap[i].hw_ver;
            break;
        }
    }
    return hw_ver;
}

static void PowerDrv_SetHWversion(void)
{
    HwVersion_t hw_ver;
    hw_ver = PowerDrv_GetHWversion();
    Setting_Set(SETID_HW_VER, &hwVerMap[hw_ver].hwVersionStr[0]);
}

void PowerDrv_WFMute(void)
{
  WF_MUTE_Enable(powerGpioDrv);
}

void PowerDrv_WFUnMute(void)
{
  WF_MUTE_Disable(powerGpioDrv);
}

void PowerDrv_TWUnMute(void)
{
  TW_PWDNN_UnMute(powerGpioDrv);
}

void PowerDrv_TWMute(void)
{
  TW_PWDNN_Mute(powerGpioDrv);
}

uint8 TW_READ_FAULT(void)
{
  uint8 Result;
  Result = GET_TW_FAULT_STATE(powerGpioDrv);
  return Result;
}

uint8 WF_READ_FAULT(void)
{
  uint8 Result;
  Result = GET_WF_FAULT_STATE(powerGpioDrv);
  return Result;
}
#endif


