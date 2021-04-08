/**
 *  @file      PwmLedDrv.c
 *  @brief     This file defines the pwm-led driver to control LEDs on STM32
 *  @author    Edmond Sung
 *  @date      10-Jan-2014
 *  @copyright Tymphany Ltd.
 */
#include "stm32f10x_rcc.h"
#include "PwmLedDrv_priv.h"

#include "trace.h"

/* Private functions / variables. Declare and drivers here */

extern uint32_t SystemCoreClock;

uint32_t TimerPeriod = 0;



#define LED_OFF_LEVEL   0
//#define LED_ON_LEVEL    (100)

#define PWMPINS (me->pPwmLedConfig->pwmPin)


/* store the number of object that is created*/
static uint8 PwmLedObjectCount = 0;

static const tLedFunc pwmLedDrvFunc = 
{
    .LedOn          = PwmLedDrv_On,
    .LedOff         = PwmLedDrv_Off,
    .LedSetColor    = PwmLedDrv_SetColor,
    .LedXtor        = PwmLedDrv_Xtor,
};

static void PwmLedDrv_pinConfig(const tPwmLedMap* pPwmLedMap)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_TypeDef *GPIOX;
    uint32 RCC_AHBPeriph_x;
    uint16 GPIO_Pin_x;
    switch(pPwmLedMap->pwmPin.gpioPort)
    {
        case IO_PORT_A:
            RCC_AHBPeriph_x = RCC_APB2Periph_GPIOA;
            GPIOX = GPIOA;
            break;
        case IO_PORT_B:
            RCC_AHBPeriph_x = RCC_APB2Periph_GPIOB;
            GPIOX = GPIOB;
            break;
        case IO_PORT_C:
            RCC_AHBPeriph_x = RCC_APB2Periph_GPIOC;
            GPIOX = GPIOC;
            break;
        default:
            ASSERT(0);
            break;
    }
    

    /* GPIOA, GPIOB and GPIOE Clocks enable */
    RCC_APB2PeriphClockCmd( RCC_AHBPeriph_x, ENABLE);
    GPIO_Pin_x = GPIO_Pin_0<<(pPwmLedMap->pwmPin.gpioBit);
    
    /* GPIOA Configuration: Channel 1, 2, 3, 4 and Channel 1N as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_x;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOX, &GPIO_InitStructure);

    // Check referece manual page 186
    GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE); 
}

static void PwmLedDrv_TimerConfig(const tPwmLedMap* pPwmLedMap)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    TimerPeriod = (SystemCoreClock / 17570 ) - 1;
    //uint16 ChannelPulse = (uint16_t) (((uint32_t) 5 * (TimerPeriod - 1)) / 255);

    /* TIM1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1 , ENABLE);

    /* Time Base configuration */
    TIM_TimeBaseStructure.TIM_Prescaler = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* Channel 1, 2,3 and 4 Configuration in PWM mode */
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    /* TIM1 counter enable */
    TIM_Cmd(TIM1, ENABLE);

    /* TIM1 Main Output Enable */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
/* @brief       Pwm Led driver constructor
 * @param[in]   me          pointer to Pwm-Led driver object
 * @param[in]   pConfig     pointer to Led config
 * @param[in]   index       The index of where the current object locate in the pConfig
 */
void PwmLedDrv_Ctor(cPwmLedDrv* me, const tDevice* pConfig, uint8 index)
{
    ASSERT(me);
    ASSERT(pConfig);

    tPwmLedDevice *pPwmLedConfig = (tPwmLedDevice *)pConfig;

    me->super_.pLedFunc = &pwmLedDrvFunc;
    me->pPwmLedConfig = &pPwmLedConfig->pPwmLedMap[index];

    /* Initial GPIO and PWM */
    PwmLedDrv_pinConfig(&pPwmLedConfig->pPwmLedMap[index]);
    PwmLedDrv_TimerConfig(&pPwmLedConfig->pPwmLedMap[index]);

    ++PwmLedObjectCount;
}

/* @brief       Pwm Led driver destructor
 * @param[in]   me          pointer to Pwm-Led driver object
 */
void PwmLedDrv_Xtor(cLedDrv* me)
{
    ASSERT(me);
    PwmLedDrv_Off(me);

    --PwmLedObjectCount;
    if(PwmLedObjectCount ==0)
    { /* if there's no object left, turn off the timer*/
        //T3CONbits.ON = 0;              
    }
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
/* @brief       Turn on the Pwm Led with the driver pointed to
 * @param[in]   me          pointer to the super class of Pwm-Led driver object
 */
void PwmLedDrv_On(cLedDrv* me)
{
    ASSERT(me);
    cPwmLedDrv* pDrv = (cPwmLedDrv*)me;

    PwmLedDrv_PwmStop(pDrv);
    //PORTSetBits(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));
}

/* @brief       Turn off the Pwm Led with the driver pointed to
 * @param[in]   me          pointer to the super class of Pwm-Led driver object
 */
void PwmLedDrv_Off(cLedDrv* me)
{
    ASSERT(me);
    cPwmLedDrv* pDrv = (cPwmLedDrv*)me;

    PwmLedDrv_PwmStop(pDrv);
    //PORTClearBits(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));
}

/* @brief       Set the brightness to the Pwm Led with the driver pointed to
 * @param[in]   me          pointer to the super class of Pwm-Led driver object
 * @param[in]   color       the color to be set, only the corresponding component will be set
 */
void PwmLedDrv_SetColor(cLedDrv* me, Color color)
{
    ASSERT(me);
    cPwmLedDrv* pDrv = (cPwmLedDrv*)me;
    uint8 brightness;

#ifdef LED_HAS_RGB
    switch (pDrv->pPwmLedConfig->colorComp)
    {
        case COLOR_COMPONENT_RED:
            brightness = GET_RED(color);
            break;
        case COLOR_COMPONENT_GREEN:
            brightness = GET_GREEN(color);
            break;
        case COLOR_COMPONENT_BLUE:
            brightness = GET_BLUE(color);
            break;
        case COLOR_COMPONENT_BRIGHTNESS:
            brightness = GET_BRIGHTNESS(color);
            break;
        default:
            ASSERT(0);
            break;
    }
#else
    brightness = GET_BRIGHTNESS(color);
#endif

//    if (brightness >= LED_ON_LEVEL)
//    {
//        PwmLedDrv_On(me);
//    }
//    else
//    {
        PwmLedDrv_PwmStart(pDrv, brightness);
//    }
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
/* @brief       Start PWM: run timer3, and start Output Compare (OC)
 * @param[in]   me          pointer to Pwm-Led driver object
 */
static void PwmLedDrv_PwmStart(cPwmLedDrv* me, uint8 brightness)
{
    TIM_OCInitTypeDef  TIM_OCInitStructure;
    uint16 ChannelPulse = (uint16_t) (((uint32_t) brightness * (TimerPeriod - 1)) / 255);
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse = ChannelPulse;
    switch (PWMPINS.pOCRegister)
    {
        case PWM_CH1:
        case PWM_CH1N:
            TIM_OC1Init(TIM1, &TIM_OCInitStructure);
            break;
        case PWM_CH2:
        case PWM_CH2N:
            TIM_OC2Init(TIM1, &TIM_OCInitStructure);
            break;
        case PWM_CH3:
        case PWM_CH3N:
            TIM_OC3Init(TIM1, &TIM_OCInitStructure);
            break;
        case PWM_CH4:
        case PWM_CH4N:
            TIM_OC4Init(TIM1, &TIM_OCInitStructure);    // RED
            break;
        default:
            ASSERT(0);
            break;
    }


    /* TIM1 Main Output Enable */
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/* @brief       Stop PWM: stop Output Compare (OC)
 * @param[in]   me          pointer to Pwm-Led driver object
 */
static void PwmLedDrv_PwmStop(cPwmLedDrv* me)
{


}

