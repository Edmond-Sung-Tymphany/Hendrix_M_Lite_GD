/**
 *  @file      Pcm1690Drv.c
 *  @brief     This file contains the PCM1690 spdif receiver driver implementation.
 *  @author    Viking Wang
 *  @date      27-Sep-2017
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"

#ifdef HAS_PCM1690

#include "commonTypes.h"
#include "bsp.h"
#include "GpioDrv.h"
#include "i2cDrv.h"
#include "trace.h"
#include "assert.h"
#include "attacheddevices.h"
#include "SWi2c_Drv.h"
#include "Pcm1690_Driver.h"

typedef struct tagPcm1690Drv
{
    bool        isReady;
    bool        i2cEnable;
    cSWi2cDrv_t     swi2cDrv;
}PCM1690Drv_t;

/* driver instance */
static PCM1690Drv_t     pcm1690_drv;
static PCM1690Drv_t     *p_pcm1690_drv=NULL;

static bool PCM1690_WriteByte(uint8_t addr, uint8_t data)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_pcm1690_drv->swi2cDrv);
    if( (! p_swi2cDrv->devReady) || (! p_pcm1690_drv->i2cEnable) )
        return TP_ACCESS_ERROR;

    ret = SWi2cDrv_WriteByte(p_swi2cDrv, addr, data);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}

static bool PCM1690_ReadByte(uint8_t addr, uint8_t *p_byte)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_pcm1690_drv->swi2cDrv);
    if( (! p_swi2cDrv->devReady) || (! p_pcm1690_drv->i2cEnable) )
    {
        *p_byte = 0xff;
        return TP_ACCESS_ERROR;
    }

    ret = SWi2cDrv_ReadByte(p_swi2cDrv, addr, p_byte);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}


/*
static bool PCM1690_ReadArray(uint8_t addr, uint16_t len, uint8_t *p_buf)
{
}
*/

static void PCM1690_PowerDown(bool enable)
{
}

/**
 * Construct the PCM1690 driver instance.
 * @return : none
 */
void PCM1690Drv_Ctor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;

    p_pcm1690_drv = &pcm1690_drv;

#ifndef BRINGUP_DEBUG
    ASSERT(p_pcm1690_drv && (! p_pcm1690_drv->isReady) );
#endif
    
    p_swi2cDrv = (cSWi2cDrv_t *)(& p_pcm1690_drv->swi2cDrv);

    p_swi2cDrv->pConfig = (stSWi2cDevice_t *) getDevicebyIdAndType(AUDIO_DAC_DEV_ID, SWI2C_DEV_TYPE, NULL);
    ASSERT(p_swi2cDrv->pConfig);
    SWi2cDrv_Ctor(p_swi2cDrv, p_swi2cDrv->pConfig);
    if( ! SWi2cDrv_DeviceAvailable(p_swi2cDrv, 3) )
    {
        TP_PRINTF("\n\rPCM1690 chip is not ready.");
    }
    else
    {
        p_pcm1690_drv->isReady = TRUE;
        p_pcm1690_drv->i2cEnable = TRUE;
    }
}

/**
 * Exit & clean up the driver.
 */
void PCM1690Drv_Xtor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;

    ASSERT(p_pcm1690_drv && p_pcm1690_drv->isReady);

    PCM1690_PowerDown(TRUE);

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_pcm1690_drv->swi2cDrv);
    SWi2cDrv_Xtor(p_swi2cDrv);
    
    p_pcm1690_drv->isReady = FALSE;
    p_pcm1690_drv->i2cEnable = FALSE;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void PCM1690Drv_I2cEnable(bool enable)
{
    p_pcm1690_drv->i2cEnable = enable;
}

/**
 * brief : PCM1690 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void PCM1690Drv_Init(void)
{
#ifdef PCM1690_TDM_MODE
    PCM1690_WriteByte(0x41, 0x06);  // TDM8-I2S format
#else    
    PCM1690_WriteByte(0x41, 0x00);  // I2S format
#endif
}

#define PCM1690_READBACK_ENABLE
#ifdef PCM1690_READBACK_ENABLE
void PCM1690Drv_ReadAll()
{
    uint8_t reg;
    uint32_t i;

    for(i=0x40; i<0x4f; i++)
    {
        PCM1690_ReadByte(i, &reg);
        BRINGUP_printf("\n\rReg[%d] = 0x%x", i, reg);
        BSP_BlockingDelayMs(10);
    }
}
#endif

#endif  // HAS_PCM1690


