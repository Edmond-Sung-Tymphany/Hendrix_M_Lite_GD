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

#include "stm32f10x.h"
#include "qp_port.h"
#include "GpioDrv.h"
#include "PowerDrv.h"
#include "bsp.h"
#include "AdcDrv.h"  //for getting the battery level
#include "signals.h"
#include "PowerDrv_light.config"

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

static void PowerDrv_EnableUSBandBuck();
static void PowerDrv_DisableUSBandBuck();
#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
