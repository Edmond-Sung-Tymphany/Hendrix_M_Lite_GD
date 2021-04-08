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

//#include "stm32f0xx.h"
#include "qp_port.h"
#include "GpioDrv.h"
#include "PowerDrv.h"
#include "bsp.h"
#include "AdcDrv.h"  //for getting the battery level

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/

/* pin definition */
#define    AC_DETECT_PIN       PORTBbits.RB3
#define    CH_STATUS_PIN       PORTGbits.RG7
#define    SET_CH_STATUS_PIN_DIGITAL  ANSELGbits.ANSG7 = 0;
#define    SET_CH_STATUS_PIN_PULL_UP  CNPUGbits.CNPUG7 = 1;
#define    SET_CH_STATUS_PIN_INPUT    TRISGbits.TRISG7 = 1;

#define     INITIAL_STEP             0 

#define     EJECT_BATTERY_STEP1_TIME_MS    500
#define     EJECT_BATTERY_STEP2_TIME_MS    1000

/***************************************************************************
* ADC config for power
***************************************************************************/
#define ADC_REFERENCE_mVOLT  3300
#define ADC_PRECISION        1024
/***************************************************************************
 * DC input Config
 ***************************************************************************/
#define CHARGER_MAX_mVOLT    13800 //9V  +15%
#define CHARGER_MIN_mVOLT    10200  //9V  -15%

#define DC_RESISTOR1         180
#define DC_RESISTOR2         470
#define DC_ADC_TO_mVOLT(adc)  ((int16) (adc* ADC_REFERENCE_mVOLT/ADC_PRECISION \
                                *(DC_RESISTOR2+DC_RESISTOR1)/DC_RESISTOR1))

/***************************************************************************
 * BATTERY detect Config
 ***************************************************************************/
#define BATT_RESISTOR1         470
#define BATT_RESISTOR2         1500
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
static void PowerDrv_SetPower(bool state);
static void PowerDrv_SetReset(uint8 state);

static void PowerDrv_PushIntEvtToServer(eSignal signal);
static void PowerDrv_EnablelWakeUpSources();
static void PowerDrv_DisableEXTI_Config();
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();

static void PowerDrv_InitialUnusedPins();
static void PowerDrv_EnableAcDetInt();
static void PowerDrv_DisableAcDetInt();
/* use the change notice interrupt for AC plugged in detect*/
static void PowerDrv_InitialCNIntOnAcDet();
static void PowerDrv_DisableCNIntOnAcDet();
static void PowerDrv_InitChargeStatusPin();

static void PowerDrv_SaveRegisters();
/* resume the register setting*/
static void PowerDrv_ResumeRegisters();
/* set specific pins as tri-state (input) to save power*/
static void PowerDrv_SetPinsSavingPower();

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
