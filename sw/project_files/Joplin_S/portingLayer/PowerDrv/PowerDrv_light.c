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
#endif

/***********************************************
 * Definition
 ***********************************************/
#define POWERDRV_DEBUG

#ifndef POWERDRV_DEBUG
// printf() silence if not debugging
#undef TP_PRINTF
#define TP_PRINTF
#endif

/***********************************************
 * Global Variable
 ***********************************************/
/*GPIO object*/
cGpioDrv powerGpioDrv;

/*ADC object*/
cADCDrv adcDrv;

/* Variables */
static const char *  hwVersion         = NULL;
#ifdef HAS_POWER_SWITCH_KEY
static ePwrSwSta  pwrSwStatus        = PWR_SW_MAX;
static int32   pwr_sw_key_debouncer_ms = 0;
#endif


/***********************************************
 * Array
 ***********************************************/
/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
    // To Do: need more stage
};

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
    PowerDrv_DeinitialPower(me);
}

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

void PowerDrv_EnterSleepMode(cPowerDrv *me)
{
    //before sleep
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    PowerDrv_DisableSystemTimerInt();
    SetEnableWakeupSources();
    PowerDrv_InitVariablesBeforeSleep(me);

    //sleep
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    //after wakeup
    SetDisableWakeupSources();
    PowerDrv_EnableSystemTimerInt();

    /* It seems WFI set system clock to default,
     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
     */
    BSP_init_clock();
}

#ifdef HAS_POWER_SWITCH_KEY
void PowerDrv_PwrSwitchUpdate(cPowerDrv *me)
{
    static ePwrSwSta lastPwrSwStates = PWR_SW_MAX;
    ePwrSwSta pwrSwStatus_now;
    if(IS_PWR_SW_KEY_ON(powerGpioDrv) == PWR_SW_ON_LEVEL)
    {
        pwrSwStatus_now = PWR_SW_ON;
    }
    else
    {
        pwrSwStatus_now = PWR_SW_OFF;
    }
    if(pwrSwStatus_now != lastPwrSwStates)
    {
        if (0 == pwr_sw_key_debouncer_ms)
        {
            pwr_sw_key_debouncer_ms = getSysTime();
        }

        if ((getSysTime() - pwr_sw_key_debouncer_ms) >= PWR_SW_DEBOUNCE_TIME_MS)
        {
            TP_PRINTF("PowerDrv: report change, pwr_sw=%d\r\n", pwrSwStatus_now);
            lastPwrSwStates = pwrSwStatus;

            pwrSwStatus = pwrSwStatus_now;

            Setting_Set(SETID_IS_PWR_SWITH_ON, &pwrSwStatus);

            PowerDrv_ReportPowerState(me);
        }
    }
    else
    {
        pwr_sw_key_debouncer_ms = 0;
    }
}
#endif

void PowerDrv_UpdateOne(cPowerDrv *me, bool sleepUpdate)
{
    /*--------------------------------------------------------------------
     *  per tick 50ms (POWER_SRV_TIMEOUT_IN_MS)
     *--------------------------------------------------------------------
     */

    /* Power switch update */
#ifdef HAS_POWER_SWITCH_KEY
    PowerDrv_PwrSwitchUpdate(me);
#endif

    if (TRUE == sleepUpdate)
    {
        return;
    }
}

void PowerDrv_Update(cPowerDrv *me)
{
    if (Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP))
    {
        PowerDrv_UpdateOne(me, TRUE);
    }
    else
    {
        PowerDrv_UpdateOne(me, FALSE);
    }
}

void PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable)
{
    switch (setId)
    {
        case POWER_SET_ID_SHUT_DOWN:
        {
            if(enable == TRUE)
            {
            }
            else
            {
            }
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
}

/***************************** Power down process ***************************/

static void PowerDrv_PowerDownStage(cPowerDrv *me)
{
    // To Do:  made the process at a right sequence .
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
#ifdef HAS_POWER_SWITCH_KEY
    Setting_Reset(SETID_IS_PWR_SWITH_ON); // power swith on
#endif
    PowerDrv_InitVariablesBeforeSleep(me);
}

static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me)
{
#ifdef HAS_POWER_SWITCH_KEY
    pwr_sw_key_debouncer_ms = 0;
#endif

}

static void PowerDrv_EnableSystemTimerInt()
{
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                     SysTick_CTRL_TICKINT_Msk   |
                     SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}

static void PowerDrv_UpdateAdcValues()
{

}

static void PowerDrv_ReportPowerState(cPowerDrv *me)
{
    PowerSrvInfoEvt *evt = Q_NEW(PowerSrvInfoEvt, POWER_STATE_SIG);
    ASSERT(evt);
    QF_PUBLISH(&evt->super, NULL);
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

#endif



