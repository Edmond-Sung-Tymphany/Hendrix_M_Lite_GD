/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for B&O CA16)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in B&O CA16
@author      Daniel Qin
@date        2016-03-17
@copyright (c) Tymphany Ltd. All rights reserved.

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "trace.h"

/*GPIO object*/
cGpioDrv powerGpioDrv;
static const char *  hwVersion    = NULL;

/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
    {&PowerDrv_PowerUpStage2, GET_TICKS_IN_MS(POWER_STAGE2_DELAY)},
    {&PowerDrv_PowerUpStage3, GET_TICKS_IN_MS(POWER_STAGE3_DELAY)},
    {&PowerDrv_PowerUpStage4, GET_TICKS_IN_MS(POWER_STAGE4_DELAY)},
    {&PowerDrv_PowerUpStage5, GET_TICKS_IN_MS(POWER_STAGE5_DELAY)},
};

static QActive* pIntEvtRequestor;
static bool isIntEvtRequest;

cADCDrv adcDrv;

hwVersionTag hwVersionArray[] =
{
    {"MP2",     0,    400 },
    {"PVT",     400,  750 },
    {"DVT2",    750,  1100},
    {"DVT1",    1100, 1450},
    {"EVT2",    1450, 1750},
    {"EVT1",    1750, 2200},
    {"ES3",     2200, 2700},
    {"ES2",     2700, 3150},
    {"ES1",     3150, 3500},
    {"PRE-ES",  3500, 3900},
    {"MP",      3900, 4095},
};
/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/

void PowerDrv_Ctor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
    
    /* ADC for reading battery and AC input status*/
    uint16 attached_device_index = 0;
    tDevice *pPowerDev = NULL;

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
          ASSERT(0);
          return;
        }
        attached_device_index++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
    }

}

void PowerDrv_Xtor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
    ADCDrv_Xtor(&adcDrv);
    PowerDrv_DeinitialPower(me);

}

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
    { /* if reset is completed, return 0 */
        delayTime = 0;
        me->step = 0;
    }
    return delayTime;
}

void PowerDrv_DeinitialPower(cPowerDrv *me)
{
    PowerDrv_EnablelWakeUpSources();

    /*turn off system pwr*/
    //SYS_PWR_DISABLE(powerGpioDrv);
}

void PowerDrv_SetPowerForStandby(bool switchToStandby)
{
    if (switchToStandby)
    {
#ifdef AMP_PDN_N
        AMP_PDN_N_OFF(powerGpioDrv);
#endif
        SYS_PWR_DISABLE(powerGpioDrv);
    }
    else
    {
        SYS_PWR_ENABLE(powerGpioDrv);
#ifdef AMP_PDN_N
        AMP_PDN_N_ON(powerGpioDrv);
#endif

    }
}

void  PowerDrv_RegisterIntEvent(QActive* pRequestor)
{
    pIntEvtRequestor = pRequestor;
    isIntEvtRequest = TRUE;
}
void  PowerDrv_UnRegisterIntEvent()
{
    pIntEvtRequestor = NULL;
    isIntEvtRequest = FALSE;
}

void PowerDrv_EnterSleepMode()
{
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    PowerDrv_DisableSystemTimerInt();
    PowerDrv_EnablelWakeUpSources();
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    PowerDrv_EnableSystemTimerInt();
}

bool PowerDrv_IsHwVerCorrect(const char * hwVerString)
{
    if (strcmp(hwVerString, HARDWARE_VERSION_STRING) == 0)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me)
{
    // HW version
    PowerDrv_GetHwVersion(&hwVersion);
    TP_PRINTF("[PowerDrv_PowerUpStage1] hwVersion: %s \n\r", hwVersion);
    Setting_Set(SETID_HW_VER, hwVersion);
    if (PowerDrv_IsHwVerCorrect(hwVersion) == FALSE)
    {
        TP_PRINTF("[PowerDrv_PowerUpStage1] hwVersion is NOT correct, it should be %s \n\r", HARDWARE_VERSION_STRING);
    }
}

static void PowerDrv_PowerUpStage2(cPowerDrv *me)
{
}

static void PowerDrv_PowerUpStage3(cPowerDrv *me)
{
}


static void PowerDrv_PowerUpStage4(cPowerDrv *me)
{
}

static void PowerDrv_PowerUpStage5(cPowerDrv *me)
{
}

static void PowerDrv_EnablelWakeUpSources()
{
    SetEnableWakeupSources();
}

static void PowerDrv_DisableSystemTimerInt()
{
    uint32 tmp = SysTick->CTRL;
    tmp = tmp & ~SysTick_CTRL_TICKINT_Msk;
    tmp = tmp & ~SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL  = tmp;
}

static void PowerDrv_EnableSystemTimerInt()
{
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}

static bool PowerDrv_GetHwVersion(const char ** hwVersion)
{
    bool ret        = FALSE;
    int16 rawResult = 0;
    uint8 ii;

    if (adcDrv.isCreated)
    {
        rawResult = (int16)ADCDrv_GetData(&adcDrv, adcDrv.ADCConfig->pAdcPinIoAttr[0].adcPin);
        if (ADC_DATA_IS_NOT_READY != rawResult)
        {
            for (ii = 0; ii < ArraySize(hwVersionArray); ii++)
            {
                if (hwVersionArray[ii].minVol <= rawResult && rawResult <= hwVersionArray[ii].maxVol)
                {
                    *hwVersion = hwVersionArray[ii].hwVersionStr;
                    ret = TRUE;
                    break;
                }
            }
            ASSERT(ii<ArraySize(hwVersionArray));
        }
        ADCDrv_StartScanning(&adcDrv);
    }
    return ret;
}

