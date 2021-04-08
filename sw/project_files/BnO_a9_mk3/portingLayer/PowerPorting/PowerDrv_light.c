/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for B&O CA17)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in B&O CA17
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

/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
    {&PowerDrv_PowerUpStage2, GET_TICKS_IN_MS(POWER_STAGE2_DELAY)},
};

static QActive* pIntEvtRequestor;
static bool isIntEvtRequest;

cADCDrv adcDrv;

hwVersionTag hwVersionArray[HW_TYPE_NUM] =
{
    {"MP",      0,    573},  //HW_TYPE_MP
    {"PVT",     573,  756 }, //HW_TYPE_PVT
    {"DVT2",    756,  1085}, //HW_TYPE_DVT2
    {"DVT1",    1085, 1456}, //HW_TYPE_DVT1
    {"EVT2",    1459, 1750}, //HW_TYPE_EVT2
    {"EVT1",    1750, 2150}, //HW_TYPE_EVT1
    {"ES3",     2150, 2725}, //HW_TYPE_ES3
    {"ES2",     2725, 3185}, //HW_TYPE_ES2
    {"ES1",     3185, 3541}, //HW_TYPE_ES1
    {"PRE-ES",  3541, 3096}, //HW_TYPE_PRE_ES
};

/* Please remove a particular HW version if it is not supported by current SW.
*/
const char * supportedHwVersionArray[] =
{
    "MP2",
    "PVT",
    "DVT2",
    "DVT1",
    "EVT2",
    "EVT1",
    "ES3",
    "ES2",
    "ES1",
    "PRE-ES",
    "MP",
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
}



void PowerDrv_EnterSleepMode()
{
    TP_PRINTF("sleep\r\n");
    BSP_ExpBlockingDelayMs(5); //wait print finish, for debug
    
    //before sleep
    IWDG_ReloadCounter();
    ADCDrv_ResetAdcConversionStatus();
    PowerDrv_DisableSystemTimerInt();
    PowerDrv_DisableEXTI_Config();
    PowerDrv_EnablelWakeUpSources();
    
    //sleep
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    
    //wakeup
    PowerDrv_EnableSystemTimerInt();  

    /* It seems WFI set system clock to default,
     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
     */
    BSP_init_clock();
	 
    TP_PRINTF("wakeup\r\n");
}

//bool PowerDrv_IsHwVerCorrect(const char * hwVerString)
//{
//    if (strcmp(hwVerString, HARDWARE_VERSION_STRING) == 0)
//    {
//        return TRUE;
//    }
//    else
//    {
//        return FALSE;
//    }
//}

bool PowerDrv_IsHwSupported(const char * hwVerString)
{
    uint8 i;
    bool ret = FALSE;

    for (i = 0; i < ArraySize(supportedHwVersionArray); i++)
    {
        if (strcmp(hwVerString, supportedHwVersionArray[i]) == 0)
        {
            ret = TRUE;
        }
    }

    return ret;
}


void PowerDrv_Update(cPowerDrv *me)
{
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
    // HW version
    uint32 hwVerIndex= (uint32)PowerDrv_GetHwVersion();

    //TODO: check Hardware version
    //ASSERT(hwVerIndex>=0 && hwVerIndex<HW_TYPE_NUM);

    char const *hwVersion= hwVersionArray[hwVerIndex].hwVersionStr;
    TP_PRINTF("[PowerDrv_PowerUpStage1] hwVersion: %s \n\r", hwVersion);
    Setting_Set(SETID_HW_VER, hwVersion);
    Setting_Set(SETID_HW_VER_INDEX, &hwVerIndex);

//    if (PowerDrv_IsHwVerCorrect(hwVersion) == FALSE)
//    {
//        TP_PRINTF("[PowerDrv_PowerUpStage1] hwVersion is NOT correct, it should be %s \r\n\r\n", HARDWARE_VERSION_STRING);
//    }
}

static void PowerDrv_PowerUpStage2(cPowerDrv *me)
{
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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);  // for PC5

    //PC13 GPIO_IN_POWER_KEY
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOC, EXTI_PinSource5,
               EXTI_Line5, EXTI_Trigger_Falling, EXTI4_15_IRQn, ENABLE);
	

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

static eHwVer PowerDrv_GetHwVersion()
{
    eHwVer hwVerIndex= HW_TYPE_NUM;
    int16 rawResult = 0;
    uint8 ii;
    ASSERT( ArraySize(hwVersionArray) == HW_TYPE_NUM );

    if (adcDrv.isCreated)
    {
        rawResult = (int16)ADCDrv_GetData(&adcDrv, adcDrv.ADCConfig->pAdcPinIoAttr[0].adcPin);
        if (ADC_DATA_IS_NOT_READY != rawResult)
        {
            for (ii = 0; ii < ArraySize(hwVersionArray); ii++)
            {
                if (hwVersionArray[ii].minVol <= rawResult && rawResult <= hwVersionArray[ii].maxVol)
                {
                    hwVerIndex = ii;
                    break;
                }
            }
            
            //TODO: check Hardware version
            //ASSERT(ii<ArraySize(hwVersionArray));
        }
        ADCDrv_StartScanning(&adcDrv);
    }
    return hwVerIndex;
}

