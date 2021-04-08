/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  System Driver Edition
                 -------------------------
                  SW Module Document
@file        SystemDrv_BTL.c
@brief       It's the simple system driver for bootloader.
@author      Viking Wang
@date        2016-12-06
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "commontypes.h"
#include "attachedDevices.h"
#include "GpioDrv.h"
#include "SystemDrv_BTL.h"

/*GPIO object*/
static cGpioDrv powerGpioDrv;

void BTL_SystemDrv_ShutDownAmp(bool enable)
{
    if( enable )
    {
        BTL_AMP_SHUTDOWN_ENABLE(powerGpioDrv);
    }
    else
    {
        BTL_AMP_SHUTDOWN_DISABLE(powerGpioDrv);
    }
}

void BTL_SystemDrv_PowerOnA2B(bool enable)
{
    if( enable )
    {
        BTL_A2B_PWR_ENABLE(powerGpioDrv);
    }
    else
    {
        BTL_A2B_PWR_DISABLE(powerGpioDrv);
    }
}

/*
 * power off the external power supply
 */
void BTL_SystemDrv_PowerOff(void)
{
    BTL_SYS_PWR_DISABLE(powerGpioDrv);
}

/*
 * power on the external power supply
 */
void BTL_SystemDrv_PowerOn(void)
{
    BTL_SYS_PWR_ENABLE(powerGpioDrv);
}


void BTL_SystemDrv_Init(void)
{
    tGPIODevice *pPowerGPIOConf;

    pPowerGPIOConf = (tGPIODevice*) getDevicebyIdAndType(POWER_DEV_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&powerGpioDrv, pPowerGPIOConf);

    BTL_SystemDrv_ShutDownAmp(TRUE);   // shut down amplifier
}

