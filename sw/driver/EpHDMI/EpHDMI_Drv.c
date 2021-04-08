/**
 *  @file      EpHDMI_Drv.c
 *  @brief     This file contains the EP9xAxx HDMI repeater driver implementation.
 *             the i2c bus is base on software i2c mode
 *  @author    Viking Wang
 *  @date      13-Jun-2016
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"

#ifdef HAS_EP9xAx_HDMI
#include "commonTypes.h"
#include "SWi2c_Drv.h"
#include "trace.h"
#include "assert.h"
#include "EpHDMI_drv.h"

typedef struct tagEpHDMIDrv {
    bool        isReady;
    eDeviceID   deviceID;
    eDeviceType deviceType;
    cSWi2cDrv_t     swi2cDrv;
}stEpHDMIDrv_t;

static stEpHDMIDrv_t   epHDMIDrv;
static stEpHDMIDrv_t   *p_epHDMIDrv=NULL;

static eTpRet EpHDMIDrv_WriteByte(uint8_t addr, uint8_t data)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_epHDMIDrv->swi2cDrv);
    if( ! p_swi2cDrv->devReady )
        return TP_ACCESS_ERROR;

    ret = SWi2cDrv_WriteByte(p_swi2cDrv, addr, data);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}

static eTpRet EpHDMIDrv_ReadByte(uint8_t addr, uint8_t *p_data)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_epHDMIDrv->swi2cDrv);
    if( ! p_swi2cDrv->devReady )
    {
        *p_data = 0xff;
        return TP_ACCESS_ERROR;
    }

    ret = SWi2cDrv_ReadByte(p_swi2cDrv, addr, p_data);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}

static void EpHDMIDrv_Init(void)
{
#ifdef BRINGUP_DEBUG
/*
    uint8_t reg_val;
    EpHDMIDrv_ReadByte(EpHDMI_REG_POWER, &reg_val);
    BRINGUP_printf("\n\rpower : 0x%x", reg_val);
    EpHDMIDrv_ReadByte(EpHDMI_REG_RX_CH, &reg_val);
    BRINGUP_printf("\n\rRX channel : 0x%x", reg_val);
    EpHDMIDrv_ReadByte(EpHDMI_REG_CEC_VOL, &reg_val);
    BRINGUP_printf("\n\rCEC volume : 0x%x", reg_val);
    */
#endif
}

/**
 * Construct the Ep9xAx HDMI driver instance.
 * @return : none
 */
void EpHDMIDrv_Ctor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;

    p_epHDMIDrv = &epHDMIDrv;

    ASSERT(p_epHDMIDrv && (! p_epHDMIDrv->isReady) );

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_epHDMIDrv->swi2cDrv);

    p_epHDMIDrv->deviceID = EP_HDMI_DEV_ID;
    p_epHDMIDrv->deviceType = SWI2C_DEV_TYPE;
    p_swi2cDrv->pConfig = (stSWi2cDevice_t *) getDevicebyIdAndType(p_epHDMIDrv->deviceID, p_epHDMIDrv->deviceType, NULL);
    ASSERT(p_swi2cDrv->pConfig);
    SWi2cDrv_Ctor(p_swi2cDrv, p_swi2cDrv->pConfig);
    if( ! SWi2cDrv_DeviceAvailable(p_swi2cDrv, 3) )
    {
        TP_PRINTF("\n\rEpHDMIDrv - hdmi chip is not ready\n\r");
    }

    p_epHDMIDrv->isReady = TRUE;

    EpHDMIDrv_Init();
}

/**
 * Exit & clean up the driver.
 */
void EpHDMIDrv_Xtor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;
    
    ASSERT(p_epHDMIDrv && p_epHDMIDrv->isReady);

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_epHDMIDrv->swi2cDrv);
    SWi2cDrv_Xtor(p_swi2cDrv);

    p_epHDMIDrv->isReady = FALSE;    
}

/**
 * HDMI in port select, port0/port1/port2
 * @return : none
 */
void EpHDMIDrv_Port(uint8_t port)
{
    ASSERT(port < 3);

    uint8_t reg_value;

    EpHDMIDrv_ReadByte(EpHDMI_REG_RX_CH, &reg_value);

    reg_value &= 0xf0;
    reg_value |= port;
    EpHDMIDrv_WriteByte(EpHDMI_REG_RX_CH, reg_value);
}

/**
 * CEC on/off control : 0->off, 1/others->on
 * @return : none
 */
void EpHDMIDrv_CEC_Enable(uint32_t enable)
{
    uint8_t reg_value;

    EpHDMIDrv_ReadByte(EpHDMI_REG_POWER, &reg_value);

    if( enable )
        reg_value &= 0xfb;
    else
        reg_value |= 0x04;

    EpHDMIDrv_WriteByte(EpHDMI_REG_POWER, reg_value);
}

/**
 * ARC on/off control : 0->off, 1/others->on
 * @return : none
 */
void EpHDMIDrv_ARC_Enable(uint32_t enable)
{
    uint8_t reg_value;

    EpHDMIDrv_ReadByte(EpHDMI_REG_POWER, &reg_value);

    if( enable )
        reg_value &= 0xfe;
    else
        reg_value |= 0x01;

    EpHDMIDrv_WriteByte(EpHDMI_REG_POWER, reg_value);
}

/**
 * Get the CEC volume value.
 * @return : CEC volume value. range 0~100, ERROR:0xff
 */
uint8_t EpHDMIDrv_GetCEC_Volume(void)
{
    uint8_t reg_value;

    EpHDMIDrv_ReadByte(EpHDMI_REG_CEC_VOL, &reg_value);

    return reg_value;
}

#endif  // HAS_EP9xAx_HDMI

