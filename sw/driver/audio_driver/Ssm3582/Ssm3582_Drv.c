/**
 *  @file      Ssm3582_Drv.c
 *  @brief     This file contains the ssm3582 amplifier driver implementation.
 *  @author    Viking Wang
 *  @date      10-May-2016
 *  @copyright Tymphany Ltd.
 */


#include "stm32f0xx.h"

#ifdef HAS_SSM3582_AMP

#include "commonTypes.h"
#include "SWi2c_Drv.h"
#include "trace.h"
#include "assert.h"
#include "ssm3582_drv.h"

#ifdef SSMDRV_DEBUG
    #define SSM3582_printf  TP_PRINTF
#else
    #define SSM3582_printf(...)
#endif

/**
 * Construct the Ssm3582 driver instance.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Ctor(stSsm3582Drv_t * me)
{
    cSWi2cDrv_t *p_swi2cDrv;

    ASSERT(me && (! me->isReady) );

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);

    p_swi2cDrv->pConfig = (stSWi2cDevice_t *) getDevicebyIdAndType(me->deviceID, me->deviceType, NULL);
    ASSERT(p_swi2cDrv->pConfig);
    SWi2cDrv_Ctor(p_swi2cDrv, p_swi2cDrv->pConfig);
    if( ! SWi2cDrv_DeviceAvailable(p_swi2cDrv, 3) )
    {
        TP_PRINTF("SSM3582_Drv - deviceID:%d is not available.\n", me->deviceID);
    }

    me->isReady = TRUE;

    Ssm3582Drv_Init(me);
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void Ssm3582Drv_Xtor(stSsm3582Drv_t * me)
{
    cSWi2cDrv_t *p_swi2cDrv;
    
    ASSERT(me && me->isReady);

    Ssm3582Drv_PowerControl(me, 0); // power down 

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    SWi2cDrv_Xtor(p_swi2cDrv);

    me->isReady = FALSE;    
}

/**
 * brief : soft reset the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:reset, 0: normal operation
 * @return : none
 */
void Ssm3582Drv_SoftReset(stSsm3582Drv_t * me, bool reset)
{
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    reg_addr = SSM3582_REG_SOFT_RESET;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    if( reset )
        *p_reg = 0x01;
    else
        *p_reg = 0x00;
        
    SWi2cDrv_WriteByte(p_swi2cDrv, reg_addr, *p_reg);
}

/**
 * brief : power on/off the SSM3582.
 * @param me - instance of the driver
 * @param reset - 1:power on, 0:power off
 * @return : none
 */
void Ssm3582Drv_PowerControl(stSsm3582Drv_t * me, bool on)
{
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
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
        
    SWi2cDrv_WriteByte(p_swi2cDrv, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 mute on/off.
 * @param me - instance of the driver
 * @param mute_on - 1:mute on, 0:mute off
 * @return : none
 */
void Ssm3582Drv_MuteControl(stSsm3582Drv_t * me, bool mute_on)
{
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
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
        
    SWi2cDrv_WriteByte(p_swi2cDrv, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 sample rate setup.
 * @param me - instance of the driver
 * @param sample_rate - SSM3582_DAC_FS_xxxxxxxx
 * @return : none
 */
void Ssm3582Drv_SampleRateControl(stSsm3582Drv_t * me, uint8_t sample_rate)
{
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    reg_addr = SSM3582_REG_DAC_CTRL;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    *p_reg &= (uint8_t)SSM3582_DAC_FS_MASK;
    sample_rate &= (uint8_t)SSM3582_DAC_FS_MASK;
    *p_reg |= sample_rate;
        
    SWi2cDrv_WriteByte(p_swi2cDrv, reg_addr, *p_reg);
}

/**
 * brief : SSM3582 initial, write the all config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_Init(stSsm3582Drv_t * me)
{
    uint32_t len;
    bool ret;
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    reg_addr = SSM3582_REG_CONFIG_START;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    for(len=SSM3582_REG_CONFIG_START; len<SSM3582_REG_CONFIG_END; len++)
    {
        ret = SWi2cDrv_WriteByte(p_swi2cDrv, reg_addr, *p_reg);
        ASSERT(ret == TP_SUCCESS);
        reg_addr ++;
        p_reg ++;
    }

/* [NOTE] : SSM3582 I2C DO NOT support the standard i2c array write command, 
   so we use the above single write byte.
   it has the special array write, refer to page29 of the datasheet
 */
/*
    len = (uint32_t)(SSM3582_REG_CONFIG_END - SSM3582_REG_CONFIG_START);
    ret = SWi2cDrv_WriteArray(p_swi2cDrv, reg_addr, p_reg, len);
    ASSERT(ret == TP_SUCCESS);
*/
}

/**
 * brief : read all the SSM3582 register
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegDump(stSsm3582Drv_t * me)
{
    uint32_t len;
    bool ret;
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    reg_addr = 0;
    p_reg = me->p_regMap;

    len = (uint32_t)(SSM3582_REG_END);

    ret = SWi2cDrv_ReadArray(p_swi2cDrv, reg_addr, p_reg, len);
    ASSERT(ret == TP_SUCCESS);
}

/**
 * brief : read the SSM3582 status register only
 * @param me - instance of the driver
 * @return : none
 */
void Ssm3582Drv_RegGetStatus(stSsm3582Drv_t * me)
{
    uint32_t len;
    uint8_t reg_addr, *p_reg;
    cSWi2cDrv_t *p_swi2cDrv;

    p_swi2cDrv = (cSWi2cDrv_t *)(& me->swi2cDrv);
    reg_addr = SSM3582_REG_STATUS_START;
    p_reg = me->p_regMap;
    p_reg += reg_addr;

    len = (uint32_t)(SSM3582_REG_STATUS_END - SSM3582_REG_STATUS_START);

    SWi2cDrv_ReadArray(p_swi2cDrv, reg_addr, p_reg, len);
}

#endif  // HAS_SSM3582_AMP

