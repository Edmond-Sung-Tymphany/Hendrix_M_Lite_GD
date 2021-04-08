/**
 *  @file      Ssm3582_HW_Drv.c
 *  @brief     This file contains the ssm3582 amplifier driver implementation.
 *             use hardware I2C to config.
 *  @author    Viking Wang
 *  @date      28-Jul-2016
 *  @copyright Tymphany Ltd.
 */


#include "stm32f0xx.h"

#ifdef HAS_SSM3582_AMP_HWI2C

#include "commonTypes.h"
#include "I2CDrv.h"
#include "trace.h"
#include "assert.h"
#include "ssm3582_HW_drv.h"

#ifdef SSMDRV_DEBUG
    #define SSM3582_printf  TP_PRINTF
#else
    #define SSM3582_printf(...)
#endif

/**
 * Write one byte data to ssm3582 chip
 * @return : TP_SUCCESS if write success, otherwise return TP_ACCESS_ERROR
 */
static eTpRet Ssm3582Drv_WriteByte(Ssm3582Drv_t *me, uint8_t addr, uint8_t data)
{
    eTpRet ret;
    cI2CDrv *p_i2cDrv;
    uint8_t reg_data[2];

    if( ! me->i2cEnable )
        return TP_FAIL;

    p_i2cDrv = (cI2CDrv *)(& me->i2c_obj);
    reg_data[0] = addr;
    reg_data[1] = data;

    tI2CMsg i2cMsg=
    {
        .devAddr = p_i2cDrv->pConfig->devAddress,
        .regAddr = NULL,
        .length = 2,
        .pMsg = (uint8_t *)reg_data,
    };
    ret = I2CDrv_MasterWrite(p_i2cDrv, &i2cMsg);
    ASSERT( ret==TP_SUCCESS );
    
    return ret;
}

static eTpRet Ssm3582Drv_ReadArray(Ssm3582Drv_t *me, uint8_t addr, uint8_t *pData, uint16_t bytes)
{
    eTpRet ret;
    cI2CDrv *p_i2cDrv;
    
    if( ! me->i2cEnable )
        return TP_FAIL;

    p_i2cDrv = (cI2CDrv *)(& me->i2c_obj);
    
    tI2CMsg i2cMsg =
    {
        .devAddr = p_i2cDrv->pConfig->devAddress,
        .regAddr = addr,
        .length  = bytes,
        .pMsg    = pData
    };
    ret = I2CDrv_MasterRead(p_i2cDrv, &i2cMsg);
    ASSERT( ret==TP_SUCCESS );
    
    return ret;
}

/**
 * Construct the Ssm3582 driver instance.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Ctor(Ssm3582Drv_t *me)
{
    cI2CDrv *p_i2cDrv;

    ASSERT(me && (! me->isReady) );

    p_i2cDrv = (cI2CDrv *)(& me->i2c_obj);

    p_i2cDrv->pConfig = (tI2CDevice *) getDevicebyIdAndType(me->deviceID, me->deviceType, NULL);
    ASSERT(p_i2cDrv->pConfig);
    I2CDrv_Ctor(p_i2cDrv, p_i2cDrv->pConfig);

    me->isReady = TRUE;
    me->i2cEnable = TRUE;
    me->reg_inited = FALSE;
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void Ssm3582Drv_Xtor(Ssm3582Drv_t *me)
{
    ASSERT(me && me->isReady);

    Ssm3582Drv_PowerControl(me, 0); // power down 

    I2CDrv_Xtor(& me->i2c_obj);
        
    me->isReady = FALSE;    
    me->i2cEnable = FALSE;
    me->reg_inited = FALSE;
}

/**
 * brief : soft reset the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:reset, 0: normal operation
 * @return : none
 */
void Ssm3582Drv_SoftReset(Ssm3582Drv_t *me, bool reset)
{
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_SOFT_RESET;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    if( reset )
        *p_reg = 0x01;
    else
        *p_reg = 0x00;
        
    Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
}

/**
 * brief : power on/off the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:power on, 0:power off
 * @return : none
 */
void Ssm3582Drv_PowerControl(Ssm3582Drv_t *me, bool on)
{
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_POWER_CTRL;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    *p_reg &= (uint8_t)SSM3582_POWER_ON_MASK;

    if( on )
    {
        *p_reg |= SSM3582_NORMAL_OPERATION;
    }
    else
    {
        *p_reg |= SSM3582_SW_POWER_DOWN;
    }
        
    Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 mute on/off.
 * @param me - instance of the driver
 * @param mute_on - 1:mute on, 0:mute off
 * @return : none
 */
void Ssm3582Drv_MuteControl(Ssm3582Drv_t *me, bool mute_on)
{
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_DAC_CTRL;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    *p_reg &= (uint8_t)SSM3582_DAC_MUTE_MASK;

    if( mute_on )
    {
        *p_reg |= SSM3582_DAC_MUTE;
    }
    else
    {
        *p_reg |= SSM3582_DAC_UNMUTE;
    }
        
    Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 sample rate setup.
 * @param me - instance of the driver
 * @param sample_rate - SSM3582_DAC_FS_xxxxxxxx
 * @return : none
 */
void Ssm3582Drv_SampleRateControl(Ssm3582Drv_t *me, uint8_t sample_rate)
{
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_DAC_CTRL;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    *p_reg &= (uint8_t)SSM3582_DAC_FS_MASK;
    sample_rate &= (uint8_t)SSM3582_DAC_FS_MASK;
    *p_reg |= sample_rate;
        
    Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 initial, write the all config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Init(Ssm3582Drv_t *me)
{
    uint32_t len;
    eTpRet ret;
    uint8_t reg_addr, *p_reg;

    if( me->reg_inited )
        return ;

    /* copy initial constant value to ram (p_reg) */
    for(len=SSM3582_REG_CONFIG_START; len<SSM3582_REG_CONFIG_END; len++)
    {
        me->p_regMap[len] = me->p_regMap_init[len];
    }

    reg_addr = SSM3582_REG_CONFIG_START;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    for(len=SSM3582_REG_CONFIG_START; len<SSM3582_REG_CONFIG_END; len++)
    {
        ret = Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
        ASSERT(ret != TP_ACCESS_ERROR);
        reg_addr ++;
        p_reg ++;
    }
/* [NOTE] : SSM3582 I2C DO NOT support the standard i2c array write command, 
   so we use the above single write byte.
   it has the special array write, refer to page29 of the datasheet
 */

    me->reg_inited = TRUE;
}

/**
 * brief : enable/disable the h/w i2c access
 * @param me - instance of the driver
 * @param enable - ...
 * @return : none
 */
void Ssm3582Drv_I2cEnable(Ssm3582Drv_t * me, bool enable)
{
    me->i2cEnable = enable;
}

/**
 * brief : read all the SSM3582 register
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegDump(Ssm3582Drv_t *me)
{
    uint32_t len;
    eTpRet ret;
    uint8_t reg_addr, *p_reg;

    reg_addr = 0;
    p_reg = me->p_regMap;

    len = (uint32_t)(SSM3582_REG_END);

    ret = Ssm3582Drv_ReadArray(me, reg_addr, p_reg, len);
    ASSERT(ret != TP_ACCESS_ERROR);
}

/**
 * brief : read the SSM3582 status register only
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegGetStatus(Ssm3582Drv_t *me)
{
    uint16_t len;
    eTpRet ret;
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_STATUS_START;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    len = (uint16_t)(SSM3582_REG_STATUS_END - SSM3582_REG_STATUS_START);

    ret = Ssm3582Drv_ReadArray(me, reg_addr, p_reg, len);
    ASSERT(ret != TP_ACCESS_ERROR);
}


/**
 * brief : output power the SSM3582.
 * @param me - instance of the driver
 * @param 
 * @return : none
 */
void Ssm3582Drv_OutputPower(Ssm3582Drv_t *me, uint8 level)
{
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_AMP_DAC_CTRL;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    *p_reg &= (uint8_t)SSM3582_ANA_GAIN_MASK;

    *p_reg |= level;

    Ssm3582Drv_WriteByte(me, reg_addr, *p_reg);
}


/**
 * brief : read the SSM3582 register temperature only
 * @param me - instance of the driver
 * @return : none
 */
uint16 Ssm3582Drv_RegGetTemperature(Ssm3582Drv_t *me)
{
    uint16_t len;
    uint8_t reg_addr, *p_reg;

    reg_addr = SSM3582_REG_TEMP;
    p_reg = me->p_regMap;

    Ssm3582Drv_ReadArray(me, reg_addr, p_reg, 1);

    return (*p_reg);
}



#endif  // HAS_SSM3582_AMP_HWI2C

