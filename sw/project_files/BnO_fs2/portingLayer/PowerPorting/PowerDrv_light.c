/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for B&O FS2)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in B&O FS2
@author      Wesley Lee
@date        2015-03-15
@copyright (c) Tymphany Ltd. All rights reserved.

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "trace.h"
#include "PowerSrv.h"
#include "PowerDrv_v2.h"
#include "SettingSrv.h"
#include "controller.h"


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
static eDcInSta      dcInStatus   = DC_IN_STA_MAX;
static const char *  hwVersion    = NULL;



/***********************************************
 * Array
 ***********************************************/
/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
};



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
            {
                ASSERT(0);
                return;
            }
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
}


void PowerDrv_EnterSleepMode()
{
    //before sleep
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    PowerDrv_DisableSystemTimerInt();
    PowerDrv_DisableEXTI_Config();
    PowerDrv_EnablelWakeUpSources();
    GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_TCH_360_RST);
    //sleep
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    
    //after wakeup
    GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_TCH_360_RST);
    GpioDrv_EnableExtInterrupt(&powerGpioDrv, GPIO_IN_TCH_572_RDY, ExtiIntTri_Rising_Falling);
    GpioDrv_EnableExtInterrupt(&powerGpioDrv, GPIO_IN_TCH_360_RDY, ExtiIntTri_Rising_Falling);
    PowerDrv_EnableSystemTimerInt();
    
    /* It seems WFI set system clock to default,
     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
     */
    BSP_init_clock();
}


void PowerDrv_SetOverheat(cPowerDrv *me, bool overheat)
{
    /* FS2 power driver do not handle over-heat
     */
}


void PowerDrv_Update(cPowerDrv *me)
{
    #define PERIOD_5Hz_MS 200
    static uint32 refresh5HzTimer = PERIOD_5Hz_MS;

    /*--------------------------------------------------------------------
     *  per tick 100ms (POWER_SRV_TIMEOUT_IN_MS)
     *-------------------------------------------------------------------- */ 
    /* update DC in status */
    eDcInSta dcInStatus_now = (eDcInSta)IS_DC_IN(powerGpioDrv);
    if(dcInStatus_now!=dcInStatus) {
        TP_PRINTF("PowerDrv: report change, dc=%d\r\n", dcInStatus_now); 
        dcInStatus= dcInStatus_now;
        Setting_Set(SETID_IS_DC_PLUG_IN, &dcInStatus); // DC Plug in
    }

    /*--------------------------------------------------------------------
     *  per 500 ms (PERIOD_5Hz_MS)
     *-------------------------------------------------------------------- */ 
    refresh5HzTimer += me->timetick;
    if(refresh5HzTimer > PERIOD_5Hz_MS)
    {
        refresh5HzTimer = 0;
    }

    /*--------------------------------------------------------------------
     *  per 30sec (POWER_DRIVER_UPDATE_INTERVAL_MS)
     *-------------------------------------------------------------------- */ 
    me->timer += me->timetick;
    if(me->timer < me->period)
    {
        return;
    }
    me->timer = 0;
    PowerDrv_UpdateAdcValues();
}


void PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable)
{
    switch (setId)
    {
        default:
            break;
    }
}


void PowerDrv_Get(cPowerDrv *me, ePowerGetId getId)
{
}


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me)
{
    /* Initialize DSP gpios, to disable dynamic boost on very begining stage
     * Note DSP driver will initialize again
     */
    tGPIODevice* pDspGPIOConf = NULL;
    pDspGPIOConf= (tGPIODevice*)getDevicebyIdAndType(DSP_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pDspGPIOConf);
    cGpioDrv gpioDsp;
    GpioDrv_Ctor(&gpioDsp,pDspGPIOConf);
}


static void PowerDrv_DisableEXTI_Config()
{
  EXTI_InitTypeDef   EXTI_InitStructure;

  EXTI_InitStructure.EXTI_Line = EXTI_Line1  | EXTI_Line2  | EXTI_Line3  | EXTI_Line4  |
                                 EXTI_Line5  | EXTI_Line6  | EXTI_Line7  | EXTI_Line8  |
                                 EXTI_Line9  | EXTI_Line10 | EXTI_Line11 | EXTI_Line12 |
                                 EXTI_Line13 | EXTI_Line14 | EXTI_Line15 | EXTI_Line16 |
                                 EXTI_Line19 | EXTI_Line21 | EXTI_Line22 | EXTI_Line23 |
                                 EXTI_Line27 ;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
}

static void PowerDrv_SetExtInterrupt(uint8 port, uint8 pin, uint32 extiLine,
                                                  EXTITrigger_TypeDef type, IRQn_Type irq, FunctionalState state)
{
    /* Enable the interrupts */
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Connect EXTI Line to gpio pin */
    SYSCFG_EXTILineConfig(port, pin);
    /* Configure EXTI line */
    EXTI_InitStructure.EXTI_Line = extiLine;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = type;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = irq;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
}

static void PowerDrv_EnablelWakeUpSources()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Initial GPIO for AC interrupt as it's used for ADC in running time*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);  // for PC12, PC14

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    //PC12 GPIO_IN_TCH_572_RDY,  PC14, GPIO_IN_POWER_KEY
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
    GPIO_Init(GPIOC, &GPIO_InitStructure);    

    //PC12 GPIO_IN_TCH_572_RDY
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOC, EXTI_PinSource12,
                   EXTI_Line12, EXTI_Trigger_Rising_Falling, EXTI4_15_IRQn, ENABLE);
    
    //PC14 GPIO_IN_POWER_KEY
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOC, EXTI_PinSource14,
                   EXTI_Line14, EXTI_Trigger_Rising_Falling, EXTI4_15_IRQn, ENABLE);
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

static void PowerDrv_UpdateAdcValues()
{
    int32 rawResult = 0;
    uint16 ii;
    ASSERT(adcDrv.isCreated);
    
    //HW Version
    if( !Setting_IsReady(SETID_HW_VER) )
    {
        rawResult = ADCDrv_GetData(&adcDrv, ADC_HW_VER);
        if (ADC_DATA_IS_NOT_READY != rawResult)
        {
            for (ii = 0; ii < ArraySize(hwVersionArray); ii++)
            {
                if (hwVersionArray[ii].minVol <= rawResult && rawResult <= hwVersionArray[ii].maxVol)
                {
                    hwVersion = hwVersionArray[ii].hwVersionStr;
                    break;
                }
            }
            ASSERT(ii<ArraySize(hwVersionArray));
            Setting_Set(SETID_HW_VER, hwVersion);
            TP_PRINTF("\r\n\r\nHW_VERSION: %s \r\n\r\n\r\n", hwVersion);
        }
    }
        
    
    //Continus ADC scanning
    ADCDrv_StartScanning(&adcDrv);
}





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
    
    //debug
    if(tempNew<=levels[TL_SUBNORMAL].upper)
    {
        ASSERT(TL_SUBNORMAL == tempLevelNew);
    }
    if(tempNew>=levels[TL_CRITICAL].lower)
    {
        ASSERT(TL_CRITICAL == tempLevelNew);
    }
    
    //Store result
    *pTemp= tempNew;
    *pLevel= tempLevelNew;
}

