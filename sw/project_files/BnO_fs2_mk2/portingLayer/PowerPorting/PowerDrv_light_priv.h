/**
 * @file        PowerDrv_priv.h
 * @brief       It's the power driver for STM32F0xx, (light edition)used in MGT
 * @author      Dmitry Abdulov
 * @date        2015-01-23
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
#include "PowerSrv.h"
#include "bsp.h"
#include "AdcDrv.h"  //for getting the battery level
#include "signals.h"
#include "Setting_id.h"
#include "PowerDrv_light.config"
#include "deviceTypes.h"
#include "controller.h"

/******************************************************************************
  ******************************define*****************************************
  *****************************************************************************/
#define PVDD_MAX_mVOLT (24000)
#define PVDD_MAX_ADC   (2708)
#define PVDD_TO_mVOLT(adc)  (int16)((adc * PVDD_MAX_mVOLT) / PVDD_MAX_ADC)

#define MAX_5V_mVOLT (5000)
#define MAX_5V_ADC   (3103)
#define _5V_TO_mVOLT(adc)  (int16)((adc * MAX_5V_mVOLT) / MAX_5V_ADC)



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
static void PowerDrv_PushIntEvtToServer(eSignal signal);
static void PowerDrv_EnablelWakeUpSources();
static void PowerDrv_DisableEXTI_Config();
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
