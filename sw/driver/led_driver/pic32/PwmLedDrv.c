/**
 *  @file      PwmLedDrv.c
 *  @brief     This file defines the pwm-led driver to control LEDs on PIC32
 *  @author    Johnny Fan
 *  @date      11-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#include "PwmLedDrv_priv.h"
#include "trace.h"

/* Private functions / variables. Declare and drivers here */

/* Some helper macros */
#define SHIFTBIT(val) ( 1 << val)
#define PWMPINS (pDrv->pPwmLedConfig->pwmPin)

/* I should probably add these to config generation */
#define OUT_COMPARE_1 1
#define OUT_COMPARE_2 2
#define OUT_COMPARE_3 3
#define OUT_COMPARE_4 4
#define OUT_COMPARE_5 5

#define LED_OFF_LEVEL   0
#define LED_ON_LEVEL    (100)

/* store the number of object that is created*/
static uint8 PwmLedObjectCount = 0;

static const tLedFunc pwmLedDrvFunc = 
{
    .LedOn          = PwmLedDrv_On,
    .LedOff         = PwmLedDrv_Off,
    .LedSetColor    = PwmLedDrv_SetColor,
    .LedXtor        = PwmLedDrv_Xtor,
};

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
    cPwmLedDrv* pDrv = (cPwmLedDrv*)me;     // for PWMPINS

    me->super_.pLedFunc = &pwmLedDrvFunc;
    me->pPwmLedConfig = &pPwmLedConfig->pPwmLedMap[index];

    /* Initial GPIO and PWM */
    PORTClearBits(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));
    PORTSetPinsDigitalOut(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));

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
        T3CONbits.ON = 0;              
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
    PORTSetBits(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));
}

/* @brief       Turn off the Pwm Led with the driver pointed to
 * @param[in]   me          pointer to the super class of Pwm-Led driver object
 */
void PwmLedDrv_Off(cLedDrv* me)
{
    ASSERT(me);
    cPwmLedDrv* pDrv = (cPwmLedDrv*)me;

    PwmLedDrv_PwmStop(pDrv);
    PORTClearBits(PWMPINS.gpioPort, SHIFTBIT(PWMPINS.gpioBit));
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

    if (brightness >= LED_ON_LEVEL)
    {
        PwmLedDrv_On(me);
    }
    else
    {
        PwmLedDrv_PwmStart(pDrv, brightness);
    }
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
    cPwmLedDrv* pDrv = me;
    T3CONbits.ON = 1;               //start the timer3
    *(getOCFlag(PWMPINS.pOCRegister)) = brightness * PR3 / MAX_BRT;
    *(getOCRegister(PWMPINS.pOCRegister)) |= OC_ON;
}

/* @brief       Stop PWM: stop Output Compare (OC)
 * @param[in]   me          pointer to Pwm-Led driver object
 */
static void PwmLedDrv_PwmStop(cPwmLedDrv* me)
{
    cPwmLedDrv* pDrv = me;
    *(getOCRegister(PWMPINS.pOCRegister)) &= ~OC_ON;
}

/* @brief       Get the Output Compare (OC) Flag register address
 * @param[in]   regID       register ID
 */
static volatile unsigned int * getOCFlag(uint8 regID)
{
    volatile unsigned int  * retval = NULL;
    switch (regID)
    {
        case OUT_COMPARE_1:
            retval = &OC1RS;
            break;
        case OUT_COMPARE_2:
            retval = &OC2RS;
            break;
        case OUT_COMPARE_3:
            retval = &OC3RS;
            break;
        case OUT_COMPARE_4:
             retval = &OC4RS;
             break;
        case OUT_COMPARE_5:
             retval = &OC5RS;
             break;
        default:
            ASSERT(0);
            break;
    }
    return retval;
}

/* @brief       Get the Output Compare (OC) register address
 * @param[in]   regID       register ID
 */
static uint32 * getOCRegister(uint8 regID)
{
    uint32  * retval = NULL;
    switch (regID)
    {
        case OUT_COMPARE_1:
            retval = (uint32*)&OC1CONbits;
            break;
        case OUT_COMPARE_2:
            retval = (uint32*)&OC2CONbits;
            break;
        case OUT_COMPARE_3:
            retval = (uint32*)&OC3CONbits;
            break;
        case OUT_COMPARE_4:
             retval = (uint32*)&OC4CONbits;
             break;
        case OUT_COMPARE_5:
             retval = (uint32*)&OC5CONbits;
             break;
        default:
            ASSERT(0);
            break;
    }
    return retval;
}


