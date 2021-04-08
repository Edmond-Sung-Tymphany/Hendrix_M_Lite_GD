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

/******************************************************************************
  ******************************struct*****************************************
  *****************************************************************************/
typedef void (*powerUpFunc)(cPowerDrv *me);

typedef struct tpowerUpSequence
{
    powerUpFunc powerUpFunction;
    int32 delaytime;
} tpowerUpSequence;

typedef enum
{
    HW_VERSION_UNKNOWN,
    HW_VERSION_SM_ES,
    HW_VERSION_L_ES,
    HW_VERSION_S_EVT,
    HW_VERSION_M_EVT,
    HW_VERSION_L_EVT,
    HW_VERSION_S_DVT,
    HW_VERSION_M_DVT,
    HW_VERSION_L_DVT,
    HW_VERSION_S_PVT,
    HW_VERSION_M_PVT,
    HW_VERSION_L_PVT,
    HW_VERSION_S_MP,
    HW_VERSION_M_MP,
    HW_VERSION_L_MP,
    HW_VERSION_MAX
} HwVersion_t;

typedef enum
{
    JOPLIN_MODEL_S = 0,
    JOPLIN_MODEL_M,
    JOPLIN_MODEL_L,
    JOPLIN_MODEL_MAX,
} eJoplinModelId;

typedef enum
{
    ES = 0,
    EVT,
    DVT,
    PVT,
    MP,
    MAX_STAGE,
} eHwStage;

typedef struct hwVersionTag
{
    HwVersion_t     hw_ver;
    eJoplinModelId  modelId;
    eHwStage        hwStage;
    char            hwVersionStr[HW_VERSION_LENGTH];
    uint16          volThreshold;
} hwVersionTag;

/******************************************************************************
 ********************* private functions / data *******************************
 *****************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me);
static void PowerDrv_PowerDownStage(cPowerDrv *me);
static void PowerDrv_DisableSystemTimerInt();
static void PowerDrv_EnableSystemTimerInt();
static void PowerDrv_UpdateAdcValues();
static void PowerDrv_InitVariables(cPowerDrv *me);
static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me);
static void PowerDrv_ReportPowerState(cPowerDrv *me);
static HwVersion_t PowerDrv_GetHWversion(void);
static void PowerDrv_SetHWversion(void);

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
