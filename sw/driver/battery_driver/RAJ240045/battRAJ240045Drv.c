/**
* @file RAJ240045Drv.c
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 07-Jan-2016
* @copyright Tymphany Ltd.
*/
#include "product.config"
#include "attachedDevices.h"
#include "trace.h"
#include "GPIODrv.h"
#include "I2CDrv.h"
#include "BatteryDrvRAJ240045.h"
#include "bsp.h"


/****************************************************
 * Definition
 ****************************************************/
#ifdef BATTERY_DRV_DEBUG
    #define BATTERY_DRV_DEBUG_MSG TP_PRINTF
#else
    #define BATTERY_DRV_DEBUG_MSG(...)
#endif

uint8 RAJ240045_SHUT_DOWN_TABLE[RAJ240045_REG_ADDR_LEN+RAJ240045_REG_DATA_LEN] = {RAJ240045_SHUT_DOWN_ADDR, 0x00, 0x04};

/****************************************************
 * Function Prototype
 ****************************************************/
static void BatteryDrv_Init(cBatteryDrv * me);
static eTpRet BatteryDrv_I2cWrite(cBatteryDrv * me, uint8 bytes, const uint8 *pData);
static eTpRet BatteryDrv_I2cRead(cBatteryDrv * me, uint32 regAddr, uint16 bytes, const uint8 *pData);



/****************************************************
 * Function Implemenation
 ****************************************************/
void BatteryDrv_Ctor(cBatteryDrv * me, cI2CDrv *pI2cObj)
{
    me->pI2cObj = pI2cObj;
    me->deviceI2cAddr   = pI2cObj->pConfig->devAddress;
    me->isCreated       = TRUE;

    ASSERT(pI2cObj->pConfig->regAddrLen==REG_LEN_8BITS);    
    ASSERT(me->pI2cObj);
    I2CDrv_Ctor(me->pI2cObj, me->pI2cObj->pConfig);
    BatteryDrv_Init(me);
}

void BatteryDrv_Xtor(cBatteryDrv * me)
{
    BatteryDrv_shutDown(me);
    I2CDrv_Xtor(me->pI2cObj);
    me->pI2cObj = NULL;
    me->isCreated  = FALSE;
}

void BatteryDrv_shutDown(cBatteryDrv * me)
{
    ASSERT(me && me->isCreated);
    BatteryDrv_I2cWrite(me, RAJ240045_REG_ADDR_LEN+RAJ240045_REG_DATA_LEN, RAJ240045_SHUT_DOWN_TABLE);
}

eTpRet BatteryDrv_readRegValue(cBatteryDrv * me, tBattRegData *pBattData)
{
    uint16 regValue= 0; //set invalid value to an absoult value
    ASSERT(me && me->isCreated);
    eTpRet ret = BatteryDrv_I2cRead(me, pBattData->regAddr, pBattData->valueLength, (uint8 const*)&regValue);
    
    if(ret==TP_SUCCESS)
    {
        BATTERY_DRV_DEBUG_MSG("Battery Info reg: 0x%2.2x value: 0x%4.4x \n\r", pBattData->regAddr, regValue);
        pBattData->valid = TRUE;
        pBattData->regValue = regValue;

        if(pBattData->settingId < SETID_MAX ) 
        {
            //setting value must be 2 bytes (uint16)
            Setting_Set(pBattData->settingId, &regValue);
        }
    }
    else
    {
        if(pBattData->settingId < SETID_MAX ) 
        {
            Setting_Reset(pBattData->settingId);
        }
        pBattData->valid = FALSE;
        pBattData->regValue = -1;
    }
    
    return ret;
}

static void BatteryDrv_Init(cBatteryDrv * me)
{
    ASSERT(me && me->isCreated);
}

static eTpRet BatteryDrv_I2cWrite(cBatteryDrv * me, uint8 bytes, const uint8 *pData)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)pData
    };
    return I2CDrv_MasterWrite(me->pI2cObj, &i2cMsg);
}

static eTpRet BatteryDrv_I2cRead(cBatteryDrv * me, uint32 regAddr, uint16 bytes, const uint8 *pData)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = (uint8*)pData
    };
    return I2CDrv_MasterRead(me->pI2cObj, &i2cMsg);
}


