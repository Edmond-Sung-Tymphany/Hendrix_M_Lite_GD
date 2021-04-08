/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for Marley Get Together)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in Marley Get Together
@author      Dmitry Abdulov
@date        2015-01-23
@copyright (c) Tymphany Ltd. All rights reserved.

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "trace.h"

/*GPIO object*/
static cGpioDrv gpioDrv;

/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
    {&PowerDrv_PowerUpStage2, GET_TICKS_IN_MS(POWER_STAGE2_DELAY)},
    {&PowerDrv_PowerUpStage3, GET_TICKS_IN_MS(POWER_STAGE3_DELAY)},
    {&PowerDrv_PowerUpStage4, GET_TICKS_IN_MS(POWER_STAGE4_DELAY)},
};

static QActive* pIntEvtRequestor;
static bool isIntEvtRequest;

/* The ADC config is not in devices[], so can't use getDeviceId for now, use extern for quick test*/
cADCDrv adcDrv;


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
            GpioDrv_Ctor(&gpioDrv,pPowerGPIOConf);
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
 * Public interrupt handle function
 *
 *****************************************************************************************************************/
 #ifdef HAS_BATTERY
void  PowerDrv_HandleDcDetInterrupt()
{ /* send the event to power server */
    PowerDrv_PushIntEvtToServer(AC_IN_INTERRUPT_SIG);
    /* disable the interrupt request, unless power server turn it on again*/
    PowerDrv_UnRegisterIntEvent();
    TP_PRINTF("interrupt happen in driver layer\r\n");
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
        me->step ++;

    }
    else
    { /* if reset is completed, return 0 */
        delayTime = 0;
        me->step = 0;
    }
    return delayTime;
}

void  PowerDrv_DeinitialPower(cPowerDrv *me)
{
    PowerDrv_EnablelWakeUpSources();
#ifdef ADC_3_3V_EN
    ADC_3_3V_EN_OFF(gpioDrv);
#endif

    /* Reset DSP and BT*/
    BT_DSP_EXPANDER_RST_ON(gpioDrv);
    /*turn off system pwr*/
    SYS_PWR_DISABLE(gpioDrv);
}

void  PowerDrv_SetExtraCommand(cPowerDrv *me, eExtraCommand extraCommand)
{
    switch(extraCommand)
    {
        case SET_A_5V_CTL_CMD:
        {
            PowerDrv_EnableUSBandBuck();
            break;
        }
        case CLR_A_5V_CTL_CMD:
        {
            PowerDrv_DisableUSBandBuck();
            break;
        }
        default:
        {
            ASSERT(0);
        }
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


#ifdef HAS_BATTERY

void  PowerDrv_GetInputSourceState(cPowerDrv *me, tInputSourceState* inputSourceState)
{
    if (adcDrv.isCreated)
    {
      int16 acRawValue = (int16)ADCDrv_GetData(&adcDrv, adcDrv.ADCConfig->pAdcPinIoAttr[1].adcPin);

        if(ADC_DATA_IS_NOT_READY!=acRawValue)
        {
            inputSourceState->dcPlugInVoltage = DC_ADC_TO_mVOLT(acRawValue);
            if( (inputSourceState->dcPlugInVoltage < CHARGER_MAX_mVOLT) &&
                (inputSourceState->dcPlugInVoltage > CHARGER_MIN_mVOLT))
            {
                inputSourceState->isDcPlugIn = TRUE;
            }
            else
            {
                inputSourceState->isDcPlugIn = FALSE;
            }
        }
        ADCDrv_StartScanning(&adcDrv);
    }
    inputSourceState->isDcPlugInDetectByGPIO = IS_DC_IN(gpioDrv);
    if (IS_TPS2546_CHARGING_DONE(gpioDrv))
    {
        inputSourceState->isUsbPlugIn = FALSE;
    }
    else
    {
        inputSourceState->isUsbPlugIn = TRUE;
    }
}

bool  PowerDrv_GetBatteryVol(cPowerDrv *me, tBatteryVol* batteryVol)
{
    bool ret = FALSE;
    int16 intBattAdcValue1 = 0;

    if (adcDrv.isCreated)
    {
        intBattAdcValue1 = (int16)ADCDrv_GetData(&adcDrv, adcDrv.ADCConfig->pAdcPinIoAttr[0].adcPin);

        if(ADC_DATA_IS_NOT_READY != intBattAdcValue1)
        {
            batteryVol->intBatteryVol = BATT_ADC_TO_mVOLT(intBattAdcValue1);           
            ret = TRUE;
        }
        ADCDrv_StartScanning(&adcDrv);
    }
    return ret;
}



#endif // HAS_BATTERY
void PowerDrv_PowerSaveSleep()
{
   /**
     * calls in standby\\off mode, in case power plug was plugged or unplugged
     * if power cable plugged - stanby mode >> allow charging from usb
     * if power cable unplugged - off mode >> not allow charging from usb
     */
#ifdef IS_DC_IN
    if(IS_DC_IN(gpioDrv))
    { // before entering sleep, if AC already in, asser for debug purpose
        PowerDrv_EnableUSBandBuck();
    }
    else
    {
        PowerDrv_DisableUSBandBuck();
    }
#endif
  /*________________________________________________________________________*/
    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    PowerDrv_DisableSystemTimerInt();
    PowerDrv_EnablelWakeUpSources();
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
    PowerDrv_EnableSystemTimerInt();
}

#ifdef IS_POWER_SWITCH_ON
bool PowerDrv_IsMainSwitchOn()
{
    return IS_POWER_SWITCH_ON(gpioDrv);
}
#endif


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me)
{
#ifdef BT_DSP_EXPANDER_RST_ON
    /*reset BT, DSP */
    BT_DSP_EXPANDER_RST_ON(gpioDrv);
#endif
#ifdef  AMP_RST_N_ON
    AMP_RST_N_ON(gpioDrv);
#endif
#ifdef AMP_PDN_N_OFF
    AMP_PDN_N_OFF(gpioDrv);
#endif
#ifdef ADC_3_3V_EN
    ADC_3_3V_EN_OFF(gpioDrv);
#endif
    SYS_PWR_DISABLE(gpioDrv);
}

 static void PowerDrv_PowerUpStage2(cPowerDrv *me)
{
    /*enable system power*/
    SYS_PWR_ENABLE(gpioDrv);
#ifdef AMP_PDN_N_ON
    AMP_PDN_N_ON(gpioDrv);
#endif
#ifdef ADC_3_3V_EN
    ADC_3_3V_EN_ON(gpioDrv);
#endif

}

static void PowerDrv_PowerUpStage3(cPowerDrv *me)
{
    PowerDrv_EnableUSBandBuck();

}

static void PowerDrv_PowerUpStage4(cPowerDrv *me)
{
#ifdef BT_DSP_EXPANDER_RST_OFF
    /*release the reset pin*/
    BT_DSP_EXPANDER_RST_OFF(gpioDrv);
#endif
#ifdef AMP_RST_N_OFF
    AMP_RST_N_OFF(gpioDrv);
#endif

}

void PowerDrv_BT_OFF()
{
    BT_DSP_EXPANDER_RST_ON(gpioDrv);
}
void PowerDrv_BT_ON()
{
    BT_DSP_EXPANDER_RST_OFF(gpioDrv);
}

static void PowerDrv_EnableUSBandBuck()
{
#ifdef BUCK_ON_ENABLE
    BUCK_ON_ENABLE(gpioDrv);
#endif
#ifdef USB_ON_ENABLE
    USB_ON_ENABLE(gpioDrv);
#endif
}
static void PowerDrv_DisableUSBandBuck()
{
#ifdef BUCK_ON_DISABLE
    BUCK_ON_DISABLE(gpioDrv);
#endif
#ifdef USB_ON_DISABLE
    USB_ON_DISABLE(gpioDrv);
#endif
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

#ifdef HAS_BATTERY
static void PowerDrv_PushIntEvtToServer(eSignal signal)
{
    if(isIntEvtRequest != TRUE)
    {
        return;
    }
    if(pIntEvtRequestor!= NULL)
    {
        QEvt* pEvt = Q_NEW(QEvt, signal);
        QACTIVE_POST(pIntEvtRequestor,(QEvt*)pEvt,0);
    }
}



static void PowerDrv_DisableEXTI_Config()
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  EXTI_InitStructure.EXTI_Line = EXTI_Line1 | EXTI_Line2 | EXTI_Line3 | EXTI_Line4 |
                                 EXTI_Line5 | EXTI_Line6 | EXTI_Line7 | EXTI_Line8 |
                                 EXTI_Line9 | EXTI_Line10 | EXTI_Line11 | EXTI_Line12 |
                                 EXTI_Line13 | EXTI_Line14 | EXTI_Line15 | EXTI_Line16 |
                                 EXTI_Line17 | EXTI_Line19;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
}

#endif //HAS_BATTERY



