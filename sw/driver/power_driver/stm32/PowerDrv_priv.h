/**
 * @file        PowerDrv_priv.h
 * @brief       It's the power driver for STM32F0xx, used in iHome
 * @author      Johnny Fan 
 * @date        2014-06-06
 * @copyright   Tymphany Ltd.
 */
#ifndef POWERDRV_PRIV_H
#define POWERDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx.h"
#include "qp_port.h"
#include "GpioDrv.h"
#include "PowerDrv.h"
#include "bsp.h"
#include "AdcDrv.h"  //for getting the battery level

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/

//#define ENABLE_SIGNAL_REVERSE /*Enable for PP, Disable for DVR*/

/* input source select*/
/* DC USB input Select, high: 9v, low: USB*/
#define     DC_USB_SEL               GPIO_12  //DC9V/Mic5V_CTL, PB13
#define     SEL_DC_INPUT(x)          GpioDrv_SetBit(&(x),DC_USB_SEL)
#define     SEL_USB_INPUT(x)         GpioDrv_ClearBit(&(x),DC_USB_SEL)
/* USB input control, high: OFF, low: ON*/
#define     MINI_5V_EN_N             GPIO_15  //Mini_5V_CTL, PA12
#define     USB_5V_ENABLE(x)         GpioDrv_ClearBit(&(x),MINI_5V_EN_N)
#define     USB_5V_DISABLE(x)        GpioDrv_SetBit(&(x),MINI_5V_EN_N)
/* Ext bat enable, high: enable*/
#define     EXT_BAT_EN               GPIO_16  //EXT_BAT_O/F, PF6
#define     EXT_BAT_ENABLE(x)        GpioDrv_SetBit(&(x),EXT_BAT_EN)
#define     EXT_BAT_DISABLE(x)       GpioDrv_ClearBit(&(x),EXT_BAT_EN)
/* Int bat enable, low: enable*/
#define     INT_BAT_EN_N             GPIO_17 //INT_BAT_O/F, PF7
#ifdef ENABLE_SIGNAL_REVERSE
#define     INT_BAT_ENABLE(x)        GpioDrv_SetBit(&(x),INT_BAT_EN_N)
#define     INT_BAT_DISABLE(x)       GpioDrv_ClearBit(&(x),INT_BAT_EN_N)
#else
#define     INT_BAT_ENABLE(x)        GpioDrv_ClearBit(&(x),INT_BAT_EN_N)
#define     INT_BAT_DISABLE(x)       GpioDrv_SetBit(&(x),INT_BAT_EN_N)
#endif

/* power enables pins*/

/* BT,DSP,IO expander power enable*/
#define     SYS_PWR_ON               GPIO_4  //P_ON, PC14
#define     SYS_PWR_ENABLE(x)        GpioDrv_SetBit(&(x),SYS_PWR_ON)
#define     SYS_PWR_DISABLE(x)       GpioDrv_ClearBit(&(x),SYS_PWR_ON)

/* Soleniod power enable*/
#define     SOL_5V_EN                GPIO_11 //SOL_5V_EN ,PB12
#define     SOL_5V_ENABLE(x)         GpioDrv_SetBit(&(x),SOL_5V_EN)
#define     SOL_5V_DISABLE(x)        GpioDrv_ClearBit(&(x),SOL_5V_EN)
/* amplifier 9V power enable*/
#define     AMP_9V_EN                GPIO_13 //AMP_9V_EN, PA8
#define     AMP_9V_ENABLE(x)         GpioDrv_SetBit(&(x),AMP_9V_EN)
#define     AMP_9V_DISABLE(x)        GpioDrv_ClearBit(&(x),AMP_9V_EN)
/* ext power bank enable*/
#define     EXT_5V_EN                GPIO_14 //EXT_5V_EN ,PA11
#define     EXT_5V_ENABLE(x)         GpioDrv_SetBit(&(x),EXT_5V_EN)
#define     EXT_5V_DISABLE(x)        GpioDrv_ClearBit(&(x),EXT_5V_EN)
/* Soleniod on/off control*/
#define     SOL_ON_PIN               GPIO_3  //SOL_ON ,PC13
#define     SOL_ON(x)                GpioDrv_SetBit(&(x),SOL_ON_PIN)
#define     SOL_OFF(x)               GpioDrv_ClearBit(&(x),SOL_ON_PIN)
/* reset*/
#define     BT_DSP_EXPANDER_RST_N      GPIO_6 //RESET ,PF0
#define     BT_DSP_EXPANDER_RST_ON(x)  GpioDrv_ClearBit(&(x),BT_DSP_EXPANDER_RST_N)
#define     BT_DSP_EXPANDER_RST_OFF(x) GpioDrv_SetBit(&(x),BT_DSP_EXPANDER_RST_N)
/*A_5V_CTL*/
#define     A_5V_CTL_PIN            GPIO_7  //A_5V_CTL ,PA1
#define     SET_A_5V_CTL_PIN(x)        GpioDrv_SetBit(&(x),A_5V_CTL_PIN)
#define     CLR_A_5V_CTL_PIN(x)        GpioDrv_ClearBit(&(x),A_5V_CTL_PIN)


/*charger status*/
#define     CHAR_STAT                GPIO_5 //CH_STATUS ,PC15
#define     IS_CHAR_DONE(x)          GpioDrv_ReadBit(&(x),CHAR_STAT)

/* power input detect*/
#define     DC_DET_IN                GPIO_8 //VDC_DET, PA6
#define     IS_DC_IN(x)              GpioDrv_ReadBit(&(x),DC_DET_IN)

#define     MINI_U_DET_IN            GPIO_18 //Mini_U_DET ,PA15
#define     IS_MINI_U_IN(x)          GpioDrv_ReadBit(&(x),MINI_U_DET_IN)

/*EXT_BAT_INT*/
#define     EXT_BAT_INT_PIN          GPIO_29 //EXT_BAT_INT, PB1
#define     IS_EXT_BAT_IN(x)         GpioDrv_ReadBit(&(x),EXT_BAT_INT_PIN)

#define     INITIAL_STEP             0 

#define     EJECT_BATTERY_STEP1_TIME_MS    500
#define     EJECT_BATTERY_STEP2_TIME_MS    1000

/***************************************************************************
* ADC config for power
***************************************************************************/
#define ADC_REFERENCE_mVOLT  3000
#define ADC_PRECISION        4096
/***************************************************************************
 * DC input Config
 ***************************************************************************/
#define CHARGER_MAX_mVOLT    10350 //9V  +15%
#define CHARGER_MIN_mVOLT    7650  //9V  -15%

#define DC_RESISTOR1         470
#define DC_RESISTOR2         1000
#define DC_ADC_TO_mVOLT(adc)  ((int16) (adc* ADC_REFERENCE_mVOLT/ADC_PRECISION \
                                *(DC_RESISTOR2+DC_RESISTOR1)/DC_RESISTOR1))

/***************************************************************************
 * BATTERY detect Config
 ***************************************************************************/
#define BATT_RESISTOR1         470
#define BATT_RESISTOR2         470
#define BATT_ADC_TO_mVOLT(adc)  ((int16) (adc* ADC_REFERENCE_mVOLT/ADC_PRECISION \
                                  *(BATT_RESISTOR2+BATT_RESISTOR1)/BATT_RESISTOR1))

/******************************************************************************
  ******************************struct*****************************************
  *****************************************************************************/
typedef void (*powerUpFunc)(cPowerDrv *me);

typedef struct tpowerUpSequence{
    powerUpFunc powerUpFunction;
    int32 delaytime;
} tpowerUpSequence;

/******************************************************************************
 ********************* private functions / data *******************************
 *****************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me);
static void PowerDrv_PowerUpStage2(cPowerDrv *me);
static void PowerDrv_PowerUpStage3(cPowerDrv *me);
static void PowerDrv_PowerUpStage4(cPowerDrv *me);
static void PowerDrv_PushIntEvtToServer(eSignal signal);
static void PowerDrv_EnablelWakeUpSources();
static void PowerDrv_DisableEXTI_Config();
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();
#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
