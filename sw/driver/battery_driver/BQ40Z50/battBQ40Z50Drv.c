/**
*  @file      battBQ40Z50Drv.c
*  @brief     Driver for BQ40Z50
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/8/28
*  @copyright Tymphany Ltd.
*/
#include "./BatteryBQ40Z50Drv_priv.h"
#include "BatteryBQ40Z50Drv.h"
#include "I2CDrv.h"
#include "trace.h"

/******************************************************************************
 *
 * Public functions
 *
 ******************************************************************************/

/**
  * @brief  Battery driver constructor
  * @param  me: pointer to battery driver object.
  * @param  cI2CDrv: I2c obj pointer (BQ40Z50 used as a I2C device).
  * @retval None
  */
void BatteryDrv_Ctor_BQ40Z50(cBatteryDrv_BQ40Z50 * me, cI2CDrv *pI2cObj)
{
    me->pI2cObj = pI2cObj;
    me->deviceI2cAddr = pI2cObj->pConfig->devAddress;
    me->isCreated = TRUE;

    ASSERT(pI2cObj->pConfig->regAddrLen == REG_LEN_8BITS);
    ASSERT(me->pI2cObj);
    I2CDrv_Ctor(me->pI2cObj, me->pI2cObj->pConfig);
    ASSERT(me && me->isCreated);
//#ifndef NDEBUG
//    uint8 getName[11] = {0};
//
//    BatteryDrv_I2cRead_BQ40Z50(me, BQ40Z50_DEVICENAME_CMD, sizeof(getName), getName);
//
//    //should get "\nbq40z50-R2"
//    if(strcmp(getName, "\nbq40z50-R2"))
//    {
////        ASSERT(0);//Get wrong chip name
//        TP_PRINTF("Batt: Wrong chip name\r\n");
//    }
//    else
//    {
//        TP_PRINTF("Batt: detect battery: %s\r\n",getName);
//    }
//
//#endif
}

/**
  * @brief  Battery driver destructor
  * @param  me: pointer to battery driver object.
  * @retval None
  */
void BatteryDrv_Xtor_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    BatteryDrv_ShutDown_BQ40Z50(me);
    I2CDrv_Xtor(me->pI2cObj);
    me->pI2cObj = NULL;
    me->isCreated  = FALSE;
}

/**
  * @brief  Shut down battery
  * @param  me: pointer to battery driver object.
  * @retval None
  */
void BatteryDrv_ShutDown_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    ASSERT(me && me->isCreated);

    //send in little endian
    uint8 data[3] = {BQ40Z50_MAC_CMD, BQ40Z50_MAC_SHUTDOWN & 0xff, BQ40Z50_MAC_SHUTDOWN >> 8};
    //send cmd twice to shut down device immediately
    ASSERT(!BatteryDrv_I2cWrite_BQ40Z50(me, sizeof(data), data));
//    ASSERT(!BatteryDrv_I2cWrite_BQ40Z50(me, sizeof(data), data));
}

/**
  * @brief  Get Battery Temperature.
  * @param  me: pointer to battery driver object.
  * @retval Temperature. Unit:0.1 °„C
  */
int16 BatteryDrv_GetTemperature_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    uint16 ret = 0;
    ret = BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_TEMP_CMD);
    return BQ40Z50_KELVINS_2_CELSIUS(ret);
}

/**
  * @brief  Get Battery Voltage.
  * @param  me: pointer to battery driver object.
  * @retval Voltage. Unit:mV
  */
uint16 BatteryDrv_GetVoltage_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_VOLTAGE_CMD);
}

/**
  * @brief  Get Battery Current.
  * @param  me: pointer to battery driver object.
  * @retval Current. Unit:mA
  */
uint16 BatteryDrv_GetCurrent_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_CURRENT_CMD);
}

/**
  * @brief  Get Average Current.
  * @param  me: pointer to battery driver object.
  * @retval Average Current. Unit:mA
  */
uint16 BatteryDrv_GetAverageCurrent_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_AVRCURRENT_CMD);
}

/**
  * @brief  Get Relative State Of Charge.
  * @param  me: pointer to battery driver object.
  * @retval Relative State Of Charge. Unit:%
  */
uint16 BatteryDrv_GetRelativeStateOfCharge_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_RSCHARGE_CMD);
}

/**
  * @brief  Get Absolute State Of Charge.
  * @param  me: pointer to battery driver object.
  * @retval Absolute State Of Charge. Unit:%
  */
uint16 BatteryDrv_GetAbsoluteStateOfCharge_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_ASCHARGE_CMD);
}

/**
  * @brief  Get Remaining Capacity.
  * @param  me: pointer to battery driver object.
  * @retval Remaining Capacity. Unit:mAh
  */
uint16 BatteryDrv_GetRemainingCapacity_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_REMAINCAPACITY_CMD);
}

/**
  * @brief  Get Full Charge Capacity.
  * @param  me: pointer to battery driver object.
  * @retval Full Charge Capacity. Unit:mAh
  */
uint16 BatteryDrv_GetFullChargeCapacity_BQ40Z50(cBatteryDrv_BQ40Z50 * me)
{
    return BatteryDrv_ReadWordCmd_BQ40Z50(me, BQ40Z50_FULLCHARGECAPACITY_CMD);
}

/**
  * @brief  Use specified CMD to read a word value from BQ40Z50.
  * @param  me: pointer to battery driver object.
  * @retval Full Charge Capacity. Unit:mAh
  */
uint16 BatteryDrv_ReadWordCmd_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint8 cmd)
{
    uint8 data[2] = {0};
    uint16 ret = 0;
    ASSERT(!BatteryDrv_I2cRead_BQ40Z50(me, cmd, sizeof(data), data));
    ret = (((uint16)data[1]) << 8) | (uint16)data[0];
    return ret;
}

/**
  * @brief  This function used to suit power driver, and only support word value like old one.
  * @param  me: pointer to battery driver object.
  * @param  me: pointer to battery driver object.
  * @retval Full Charge Capacity. Unit:mAh
  */
eTpRet BatteryDrv_readRegValue_BQ40Z50(cBatteryDrv_BQ40Z50 * me, tBattRegData_BQ40Z50 *pBattData)
{
    uint16 regValue = 0; //set invalid value to an absoult value
    uint8 tempData[7] = {0};//BQ40Z50_DASTATUS_CMD will return 17 bytes, but we only care about first 7 bytes, BQ40Z50_SAFETYSTATUS_CMD PF_CMD will return 4 bytes
    eTpRet ret = TP_FAIL;
    ASSERT(me && me->isCreated);
    ASSERT(pBattData->valueLength == 2);

    if(pBattData->regAddr != BQ40Z50_DASTATUS_CMD 
       && pBattData->regAddr != BQ40Z50_SAFETYSTATUS_CMD 
       && pBattData->regAddr != BQ40Z50_PFSTATUS_CMD)
    {
        ret = BatteryDrv_I2cRead_BQ40Z50(me, pBattData->regAddr, pBattData->valueLength, tempData);//only support word value
        regValue = (((uint16)tempData[1]) << 8) | (uint16)tempData[0];
    }

    else if(pBattData->regAddr == BQ40Z50_DASTATUS_CMD)//can not get value by read word value
    {
        ret = BatteryDrv_I2cRead_BQ40Z50(me, pBattData->regAddr, sizeof(tempData), tempData);

        //decode
        if(pBattData->settingId == SETID_BATTERY_TEMP_CELL1)
        {
            regValue = BQ40Z50_KELVINS_2_CELSIUS((((uint16)tempData[4]) << 8) | (uint16)tempData[3]);// TS1 Temperature
        }

        else if(pBattData->settingId == SETID_BATTERY_TEMP_CELL2)
        {
            regValue = BQ40Z50_KELVINS_2_CELSIUS((((uint16)tempData[6]) << 8) | (uint16)tempData[5]);// TS2 Temperature
        }

        else
        {
            ASSERT(0);
        }

    }
    else if(pBattData->regAddr == BQ40Z50_SAFETYSTATUS_CMD || pBattData->regAddr == BQ40Z50_PFSTATUS_CMD)
    {
        ret = BatteryDrv_I2cRead_BQ40Z50(me, pBattData->regAddr, sizeof(tempData), tempData);
        //decode
        if(pBattData->settingId == SETID_BATTERY_SAFETY_STATUS_HIGH || pBattData->settingId == SETID_BATTERY_PF_STATUS_HIGH)
        {
            regValue = (((uint16)tempData[3]) << 8) | (uint16)tempData[4];// SETID_BATTERY_SAFETY_STATUS_HIGH || SETID_BATTERY_PF_STATUS_HIGH
        }

        else if(pBattData->settingId == SETID_BATTERY_SAFETY_STATUS_LOW || SETID_BATTERY_PF_STATUS_LOW)
        {
            regValue = (((uint16)tempData[1]) << 8) | (uint16)tempData[2];// SETID_BATTERY_SAFETY_STATUS_LOW || SETID_BATTERY_PF_STATUS_LOW
        }

        else
        {
            ASSERT(0);
        }
    }

    if(ret == TP_SUCCESS)
    {
        pBattData->valid = TRUE;
        pBattData->regValue = regValue;

        if(pBattData->settingId < SETID_MAX)
        {
            //setting value must be 2 bytes (uint16)
            Setting_Set(pBattData->settingId, &regValue);
        }
    }

    else
    {
        if(pBattData->settingId < SETID_MAX)
        {
            Setting_Reset(pBattData->settingId);
        }

        pBattData->valid = FALSE;
        pBattData->regValue = -1;
    }

    return ret;
}


/******************************************************************************
 *
 * Private functions
 *
 ******************************************************************************/

static eTpRet BatteryDrv_I2cWrite_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint8 bytes, uint8 *pData)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = NULL,
        .length  = bytes,
        .pMsg    = pData
    };
    return I2CDrv_MasterWrite(me->pI2cObj, &i2cMsg);
}

/**
  * @brief  Read data from BQ40Z50.Watch out! we will get data in little endian
  * @param  me: pointer to battery driver object.
  * @param  regAddr: read regAddr/cmd.
  * @param  bytes: bytes to be read.
  * @param  pData: pointer to data array.
  * @retval eTpRet type Reslut.
  */
static eTpRet BatteryDrv_I2cRead_BQ40Z50(cBatteryDrv_BQ40Z50 * me, uint32 regAddr, uint16 bytes, uint8 *pData)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = me->deviceI2cAddr,
        .regAddr = regAddr,
        .length  = bytes,
        .pMsg    = pData
    };
    return I2CDrv_MasterRead(me->pI2cObj, &i2cMsg);
}
