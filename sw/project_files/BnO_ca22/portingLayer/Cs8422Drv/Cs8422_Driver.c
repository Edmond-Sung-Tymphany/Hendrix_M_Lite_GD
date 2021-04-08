/**
 *  @file      Cs8422Drv.c
 *  @brief     This file contains the CS8422 spdif receiver driver implementation.
 *  @author    Viking Wang
 *  @date      22-Sep-2017
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"

#ifdef HAS_CS8422

#include "commonTypes.h"
#include "bsp.h"
#include "GpioDrv.h"
#include "i2cDrv.h"
#include "trace.h"
#include "assert.h"
#include "attacheddevices.h"
#include "SWi2c_Drv.h"
#include "Cs8422_Driver.h"
#include "Cs8422_priv.h"

/* SPDIF channel */
#define CS8422_SOURCE_SPDIF     (SDOUT1_SOURCE_AES3 | SDOUT2_SOURCE_AES3 | SDOUT1_UNMUTE | SDOUT2_MUTED | SRC_SOURCE_AES3)

/* EXT I2S channel */
#define CS8422_SOURCE_EXT_I2S   (SDOUT1_SOURCE_SDIN | SDOUT2_SOURCE_SDIN | SDOUT1_UNMUTE | SDOUT2_MUTED | SRC_SOURCE_SDIN)

static uint8_t cs8422_data_routing=CS8422_SOURCE_SPDIF;

#define MCLK_AUTO_SWITCH_ON     0x20
#define MCLK_AUTO_SWITCH_OFF    0xdf
static uint8_t cs8422_clock_control=0x80;   // default value

static uint8_t cs8422_input_control=0x08;   // default value

typedef struct tagCs8422Drv
{
    bool        isReady;
    bool        i2cEnable;
    cSWi2cDrv_t     swi2cDrv;
}CS8422Drv_t;

/* driver instance */
static CS8422Drv_t     cs8422_drv;
static CS8422Drv_t     *p_cs8422_drv=NULL;

static bool CS8422_WriteByte(uint8_t addr, uint8_t data)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_cs8422_drv->swi2cDrv);
    if( (! p_swi2cDrv->devReady) || (! p_cs8422_drv->i2cEnable) )
        return TP_ACCESS_ERROR;

    ret = SWi2cDrv_WriteByte(p_swi2cDrv, addr, data);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}

static bool CS8422_ReadByte(uint8_t addr, uint8_t *p_byte)
{
    cSWi2cDrv_t *p_swi2cDrv;
    eTpRet ret;

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_cs8422_drv->swi2cDrv);
    if( (! p_swi2cDrv->devReady) || (! p_cs8422_drv->i2cEnable) )
    {
        *p_byte = 0xff;
        return TP_ACCESS_ERROR;
    }

    ret = SWi2cDrv_ReadByte(p_swi2cDrv, addr, p_byte);
//  ASSERT(ret == TP_SUCCESS)

    return ret;
}


/*
static bool CS8422_ReadArray(uint8_t addr, uint16_t len, uint8_t *p_buf)
{
}
*/

static void CS8422_PowerDown(bool enable)
{
//    CS8422_ReadByte(0x02, &cs8422_clock_control);
    if( enable )
        cs8422_clock_control |= 0x80;
    else
        cs8422_clock_control &= 0x7f;

    CS8422_WriteByte(0x02, cs8422_clock_control);
}

/**
 * Construct the CS8422 driver instance.
 * @return : none
 */
void CS8422Drv_Ctor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;

    p_cs8422_drv = &cs8422_drv;

#ifndef BRINGUP_DEBUG
    ASSERT(p_cs8422_drv && (! p_cs8422_drv->isReady) );
#endif
    
    p_swi2cDrv = (cSWi2cDrv_t *)(& p_cs8422_drv->swi2cDrv);

    p_swi2cDrv->pConfig = (stSWi2cDevice_t *) getDevicebyIdAndType(AUDIO_CODEC_ID, SWI2C_DEV_TYPE, NULL);
    ASSERT(p_swi2cDrv->pConfig);
    SWi2cDrv_Ctor(p_swi2cDrv, p_swi2cDrv->pConfig);
    if( ! SWi2cDrv_DeviceAvailable(p_swi2cDrv, 3) )
    {
        TP_PRINTF("\n\rCS8422 chip is not ready.");
    }
    else
    {
        p_cs8422_drv->isReady = TRUE;
        p_cs8422_drv->i2cEnable = TRUE;
    }
}

/**
 * Exit & clean up the driver.
 */
void CS8422Drv_Xtor(void)
{
    cSWi2cDrv_t *p_swi2cDrv;

    ASSERT(p_cs8422_drv && p_cs8422_drv->isReady);

    CS8422_PowerDown(TRUE);

    p_swi2cDrv = (cSWi2cDrv_t *)(& p_cs8422_drv->swi2cDrv);
    SWi2cDrv_Xtor(p_swi2cDrv);
    
    p_cs8422_drv->isReady = FALSE;
    p_cs8422_drv->i2cEnable = FALSE;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void CS8422Drv_I2cEnable(bool enable)
{
    p_cs8422_drv->i2cEnable = enable;
}

/**
 * brief : CS8422 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void CS8422Drv_Init(void)
{
    cs8422_input_control = 0x88;    // single-end, RX0
    CS8422_WriteByte(0x03, cs8422_input_control);
    CS8422_WriteByte(0x05, 0xf0);       // GPIO0 -> copy XTI
    CS8422_WriteByte(0x07, 0x60);       // ILRCK = MCLK / 512
    CS8422_WriteByte(0x08, 0xa0);       // OLRCK = MCLK / 512
    CS8422_WriteByte(0x0a, cs8422_data_routing);
    CS8422_WriteByte(0x0b, (SAIMS_SLAVE|SAIF_I2S));
    CS8422_WriteByte(0x0c, (SAOMS_MASTER|SAORES_24BITS|SAOF_I2S));

    CS8422Drv_SourceSelect(CS8422_SOURCE_RX0);
//    CS8422Drv_SourceSelect(CS8422_SOURCE_I2S_IN);

    // power control
    cs8422_clock_control = 0x90;    // RMCK->HiZ
    CS8422_PowerDown(FALSE);
}

/**
 * brief : CS8422 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void CS8422Drv_SourceSelect(CS8422Source_t source)
{
    ASSERT(source < CS8422_SOURCE_MAX);

    if( source == CS8422_SOURCE_I2S_IN )
    {
        cs8422_input_control = 0xe8;    // rx=3,tx=1
        cs8422_data_routing = CS8422_SOURCE_EXT_I2S;
    }
    else
    {
        cs8422_data_routing = CS8422_SOURCE_SPDIF;
        switch( source )
        {
        case CS8422_SOURCE_RX0:
            cs8422_input_control = 0x88;
            break;
        case CS8422_SOURCE_RX1:
            cs8422_input_control = 0xa8;
            break;
        case CS8422_SOURCE_RX2:
            cs8422_input_control = 0xc8;
            break;
        case CS8422_SOURCE_RX3:
            cs8422_input_control = 0xe8;
            break;
        default :
            cs8422_input_control = 0x88;
            ALWAYS_printf("\n\rERROR : unknown source.");
            break;
        }
    }

    CS8422_WriteByte(0x03, cs8422_input_control);
    CS8422_WriteByte(0x0a, cs8422_data_routing);
}

//#define CS8422_READBACK_ENABLE
#ifdef CS8422_READBACK_ENABLE
void CS8422Drv_ReadAll()
{
    uint8_t reg;
    uint32_t i;

    for(i=2; i<0x0f; i++)
    {
        CS8422_ReadByte(i, &reg);
        BRINGUP_printf("\n\rReg[%d] = 0x%x", i, reg);
        BSP_BlockingDelayMs(10);
    }
}
#endif

#endif  // HAS_CS8422


