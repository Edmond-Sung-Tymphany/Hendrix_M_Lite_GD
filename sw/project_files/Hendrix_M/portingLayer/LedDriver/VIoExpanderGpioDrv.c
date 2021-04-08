
/**
 * @file        VIoExpanderGpioDrv.c
 * @brief       This file implements the virtul GPIO driver for IO expander, use to split different GPIO
 *              from One physic IO expander, let it can serve Different function driver.
 * @author      Colin Chen
 * @date        2018-01-25
 * @copyright   Tymphany Ltd.
 */

#include "VIoExpanderGpioDrv_priv.h"
#include "IoExpanderDrv.h"
#include "IoExpanderLedDrv.h"
#include "VIoExpanderGpioDrv.h"

static cIoExpanderDrv IoexpanderObj[MAX_SUPPORT_IOE_OBJ];
void VIoExpanderGpioDrv_Ctor(cVIoExpanderGpioDrv *me, tVIoeGpioDevice *pVIoeGpioDevice)
{
    me->pIoExpander = &IoexpanderObj[pVIoeGpioDevice->chipID];
    me->pIoExpanderLedDevice = pVIoeGpioDevice->pIoeDevice;
    IoExpanderDrv_Ctor_aw9523(me->pIoExpander, me->pIoExpanderLedDevice);
}

void VIoExpanderGpioDrv_Xtor(cVIoExpanderGpioDrv *me)
{
    IoExpanderDrv_Xtor_aw9523(me->pIoExpander);
}


/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
/* @brief       Set the corresponding Port-Pin to High
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 */
void VIoExpanderGpioDrv_SetGpio(cVIoExpanderGpioDrv *me, uint8 index)
{
    uint8 j = 0;
    for ( ; j < me->pIoExpanderLedDevice->ledNum; j++)
    {
        if(me->pIoExpanderLedDevice->pIoExpanderLedMap[j].ledID == index)
        {
            uint8 port = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].port;
            uint8 pin = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].pin;
            IoExpanderDrv_SetGpio_aw9523(me->pIoExpander, port, pin);
        }
    }
}

/* @brief       Clear the corresponding Port-Pin to Ground
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 */
void VIoExpanderGpioDrv_ClearGpio(cVIoExpanderGpioDrv *me, uint8 index)
{
    uint8 j = 0;
    for ( ; j < me->pIoExpanderLedDevice->ledNum; j++)
    {
        if(me->pIoExpanderLedDevice->pIoExpanderLedMap[j].ledID == index)
        {
            uint8 port = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].port;
            uint8 pin = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].pin;
            IoExpanderDrv_ClearGpio_aw9523(me->pIoExpander, port, pin);
        }
    }
}


/* @brief       get the corresponding Port-Pin to Ground
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 */
uint8 VIoExpanderGpioDrv_ReadGpio(cVIoExpanderGpioDrv *me, uint8 index)
{
    uint8 Result;
    uint8 j = 0;
    for ( ; j < me->pIoExpanderLedDevice->ledNum; j++)
    {
        if(me->pIoExpanderLedDevice->pIoExpanderLedMap[j].ledID == index)
        {
            uint8 port = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].port;
            uint8 pin = me->pIoExpanderLedDevice->pIoExpanderLedMap[j].pin;
            Result = IoExpanderDrv_ReadGpio_aw9523(me->pIoExpander, port, pin);
        }
    }
    return Result;
}



