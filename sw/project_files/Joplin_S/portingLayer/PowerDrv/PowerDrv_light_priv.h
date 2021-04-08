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

#ifdef HAS_HW_VERSION_TAG
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

typedef struct hwVersionTag
{
    HwVersion_t hw_ver;
    char hwVersionStr[HW_VERSION_LENGTH];
    uint16 volThreshold;
} hwVersionTag;

const hwVersionTag hwVerMap[]=
{
    {HW_VERSION_UNKNOWN,        "Unknown",           (HW_SM_ES_MAX_THRESHOLD+HW_THRESHOLD_OFFSET) },
    {HW_VERSION_SM_ES,          "Joplin S\/M ES",    (HW_SM_ES_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_L_ES,           "Joplin L ES",       (HW_L_ES_MIN_THRESHOLD-HW_THRESHOLD_OFFSET)  },
    {HW_VERSION_S_EVT,          "Joplin S EVT",      (HW_S_EVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_M_EVT,          "Joplin M EVT",      (HW_M_EVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_L_EVT,          "Joplin L EVT",      (HW_L_EVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_S_DVT,          "Joplin S DVT",      (HW_S_DVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_M_DVT,          "Joplin M DVT",      (HW_M_DVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_L_DVT,          "Joplin L DVT",      (HW_L_DVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_S_PVT,          "Joplin S PVT",      (HW_S_PVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_M_PVT,          "Joplin M PVT",      (HW_M_PVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_L_PVT,          "Joplin L PVT",      (HW_L_PVT_MIN_THRESHOLD-HW_THRESHOLD_OFFSET) },
    {HW_VERSION_S_MP,           "Joplin S MP",       (HW_S_MP_MIN_THRESHOLD-HW_THRESHOLD_OFFSET)  },
    {HW_VERSION_M_MP,           "Joplin M MP",       (HW_M_MP_MIN_THRESHOLD-HW_THRESHOLD_OFFSET)  },
    {HW_VERSION_L_MP,           "Joplin L MP",       (HW_L_MP_MIN_THRESHOLD-HW_THRESHOLD_OFFSET)  },
};
#endif /*end of HAS_HW_VERSION_TAG*/


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

#ifdef HAS_HW_VERSION_TAG
static HwVersion_t PowerDrv_GetHWversion(void);
static void PowerDrv_SetHWversion(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* POWERDRV_PRIV_H */
