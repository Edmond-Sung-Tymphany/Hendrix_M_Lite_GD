/**
* @file AudioSRCDrv.c
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 26-Mar-2015
* @copyright Tymphany Ltd.
*/
#include "product.config"
#include "I2CDrv.h"
#include "cs8422Drv.h"
#include "cs8422InitTab.h"
//#include "bsp.h"

#define CS8422_INPUT_SEL_LEN       (2)

static void Cs8422AudioSRCDrv_Init(cAudioSRCDrv * me);

void Cs8422AudioSRCDrv_I2cWrite(cAudioSRCDrv * me, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(me->pAudioSRCI2c, &i2cMsg);
}

void Cs8422AudioSRCDrv_Ctor(cAudioSRCDrv * me, cI2CDrv *pI2cObj)
{
    me->isCreated  = TRUE;
    me->pAudioSRCI2c        = pI2cObj;
    me->deviceI2cAddr = pI2cObj->pConfig->devAddress;
    I2CDrv_Ctor(me->pAudioSRCI2c, me->pI2CConfig);
    Cs8422AudioSRCDrv_Init(me);
}

void Cs8422AudioSRCDrv_Xtor(cAudioSRCDrv * me)
{
    I2CDrv_Xtor(me->pAudioSRCI2c);
    me->pI2CConfig = NULL;
    me->isCreated  = FALSE;
}

void Cs8422AudioSRCDrv_Init(cAudioSRCDrv * me)
{
    int i = 0;
    cfg_reg *r = cs8422InitTab;
    int n = sizeof(cs8422InitTab)/sizeof(cs8422InitTab[0]);
    while (i < n) {
        Cs8422AudioSRCDrv_I2cWrite(me, 2, (unsigned char *)&r[i]);
        i++;
    }
}
