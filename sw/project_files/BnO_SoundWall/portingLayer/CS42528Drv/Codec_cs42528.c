/**
 *  @file      Codec_cs42528.c
 *  @brief     This file contains the CS42528 codec driver implementation.
 *  @author    Viking Wang
 *  @date      11-Nov-2016
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"

#ifdef HAS_CS42528_CODEC

#include "commonTypes.h"
#include "bsp.h"
#include "GpioDrv.h"
#include "i2cDrv.h"
#include "trace.h"
#include "assert.h"
#include "attacheddevices.h"
#include "SystemDrv.h"
#include "Codec_cs42528.h"

#define OMCK_SINGLE_SPEED       0x00
#define OMCK_DOUBLE_SPEED       0x20
#define OMCK_SPEED_USED     OMCK_SINGLE_SPEED 

// master/slave configuration
#define CODEC_SAI_MASTER        0x01
#define CODEC_SAI_SLAVE         0x00
#define CODEC_CX_MASTER         0x02
#define CODEC_CX_SLAVE          0x00
#define CODEC_CX_M_S_MODE       CODEC_CX_MASTER
#define CODEC_M_S_NORAL         (CODEC_SAI_MASTER|CODEC_CX_M_S_MODE)
#define CODEC_M_S_A2B           (CODEC_SAI_SLAVE|CODEC_CX_M_S_MODE)

// I2S format configuration
#define CODEC_I2S_FORMAT_I2S    0x40    // i2s
#define CODEC_I2S_FORMAT_LJ     0x00    // left-justify
#define CODEC_I2S_FORMAT        CODEC_I2S_FORMAT_I2S

/* driver instance */
static CS42528Drv_t     cs42528_drv;
static CS42528Drv_t     *p_cs42528_drv=NULL;
static cGpioDrv         codecGpioDrv;

static bool CS42528_WriteByte(uint8_t addr, uint8_t data)
{
    uint8_t reg_data[2];
    
    ASSERT( p_cs42528_drv->isReady );

    if( ! p_cs42528_drv->i2cEnable )
        return TP_ACCESS_ERROR;

    reg_data[0] = addr;
    reg_data[1] = data;
    
    tI2CMsg i2cMsg=
    {
        .devAddr = p_cs42528_drv->i2cDrv.pConfig->devAddress,
        .regAddr = NULL,
        .length = 2,
        .pMsg = (uint8_t *)reg_data,
    };
    bool ret;
    ret = (bool)I2CDrv_MasterWrite(&p_cs42528_drv->i2cDrv, &i2cMsg);
    if( ret != TP_SUCCESS )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_CODEC);
#endif
        p_cs42528_drv->i2cEnable = FALSE;
    }

    return ret;
}

/*
static bool CS42528_ReadByte(uint8_t addr, uint8_t *p_byte)
{
    ASSERT( p_cs42528_drv->isReady );

    if( ! p_cs42528_drv->i2cEnable )
        return TP_ACCESS_ERROR;

    tI2CMsg i2cMsg=
    {
        .devAddr = p_cs42528_drv->i2cDrv.pConfig->devAddress,
        .regAddr = addr,
        .length = 1,
        .pMsg = p_byte,
    };
    bool ret;
    ret = (bool)I2CDrv_MasterRead(&p_cs42528_drv->i2cDrv, &i2cMsg);
    if( ret != TP_SUCCESS )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_CODEC);
#endif
        p_cs42528_drv->i2cEnable = FALSE;
    }

    return ret;
}
*/

/*
static bool CS42528_ReadArray(uint8_t addr, uint16_t len, uint8_t *p_buf)
{
    ASSERT( p_cs42528_drv->isReady );

    if( ! p_cs42528_drv->i2cEnable )
        return TP_ACCESS_ERROR;

    tI2CMsg i2cMsg=
    {
        .devAddr = p_cs42528_drv->i2cDrv.pConfig->devAddress,
        .regAddr = addr,
        .length = len,
        .pMsg = p_buf,
    };
    bool ret;
    ret = I2CDrv_MasterRead(&p_cs42528_drv->i2cDrv, &i2cMsg);
//    ASSERT(ret==TP_SUCCESS);

    return ret;
}
*/

static void CS42528_PowerDown(bool enable)
{
    uint8_t reg_value;
    if( enable )
        reg_value = 0x81;
    else
        reg_value = 0x00;
    CS42528_WriteByte(0x02, reg_value);
}

/**
 * Construct the CS42528 driver instance.
 * @return : none
 */
void CS42528Drv_Ctor(void)
{
    tI2CDevice *p_i2c_device;
    tGPIODevice *p_codec_gpio;

    p_cs42528_drv = &cs42528_drv;

    ASSERT(p_cs42528_drv && (! p_cs42528_drv->isReady) );

    // codec GPIO used
    p_codec_gpio = (tGPIODevice*)getDevicebyIdAndType(AUDIO_CODEC_ID, GPIO_DEV_TYPE, NULL);
    GpioDrv_Ctor(&codecGpioDrv, p_codec_gpio);

    // i2c setup
    p_i2c_device = (tI2CDevice *) getDevicebyIdAndType(AUDIO_CODEC_ID, I2C_DEV_TYPE, NULL);
    I2CDrv_Ctor((&p_cs42528_drv->i2cDrv), p_i2c_device);
    p_cs42528_drv->isReady = TRUE;
    p_cs42528_drv->i2cEnable = TRUE;

#if defined(TUNING_ON_ST_EVK_BOARD) // || defined(BRINGUP_DEBUG)
    p_cs42528_drv->i2cEnable = FALSE;  // for debug on ST EVK board
#endif
}


/**
 * Exit & clean up the driver.
 */
void CS42528Drv_Xtor(void)
{
    ASSERT(p_cs42528_drv && p_cs42528_drv->isReady);

    CS42528_PowerDown(TRUE);

    I2CDrv_Xtor( &p_cs42528_drv->i2cDrv );
    p_cs42528_drv->isReady = FALSE;
    p_cs42528_drv->i2cEnable = FALSE;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void CS42528Drv_I2cEnable(bool enable)
{
    p_cs42528_drv->i2cEnable = enable;
}

/**
 * brief : setup the ADC gain, range : -15dB ~ +15dB
 * @param - gain : adc gain setting
 * @return : none
 */
void CS42528_AdcGain(int8_t gain)
{
    uint8_t reg_value;

    ASSERT( (-16 < gain) && (gain < 16) );

    if( gain >= 0 )
    {
        reg_value = gain;
    }
    else
    {
        gain = - gain;
        reg_value = (16 - gain) | 0x30;
    }
    
    CS42528_WriteByte(0x1c, reg_value); // adc left channel gain
    CS42528_WriteByte(0x1d, reg_value); // adc right channel gain    
}

/**
 * brief : SAI output from SPDIF or ADC
 * @param - SAI source 
 * @return : none
 */
void CS42528Drv_SAIOutputSelect(SAISource_t source)
{
    ASSERT( source < SAI_SOURCE_MAX );

    switch( source )
    {
    case SAI_SOURCE_ADC:
        CS42528_WriteByte(0x05, CODEC_M_S_NORAL);  // setup codec to slave, SAI to master, RMCK->Hiz
        CS42528_WriteByte(0x03, 0x08);  // output map to SAI_SDOUT
        CS42528_WriteByte(0x06, (OMCK_SPEED_USED|0x02));  // disable auto switch
        CS42528_WriteByte(0x1f, 0x77);  // spdif TX from RX7(NC in the board), RX from RX7(NC)
        break;
    case SAI_SOURCE_RX0:
        CS42528_WriteByte(0x05, CODEC_M_S_NORAL);  // setup codec to slave, SAI to master, RMCK->Hiz
        CS42528_WriteByte(0x03, 0x04);  
        CS42528_WriteByte(0x06, (OMCK_SPEED_USED|0x06));  // enable auto switch
        CS42528_WriteByte(0x1f, 0x70);  // spdif TX from RX7(NC in the board), RX from RX0
        break;
    case SAI_SOURCE_RX2:
        CS42528_WriteByte(0x05, CODEC_M_S_NORAL);  // setup codec to slave, SAI to master, RMCK->Hiz
        CS42528_WriteByte(0x03, 0x04);  
        CS42528_WriteByte(0x06, (OMCK_SPEED_USED|0x00));  // fix to PLL output
        CS42528_WriteByte(0x1f, 0x72);  // spdif TX from RX7(NC in the board), RX from RX2
        break;
    case SAI_SOURCE_A2B:
        CS42528_WriteByte(0x05, CODEC_M_S_A2B);  // setup codec to slave, SAI to slave, RMCK->Hiz
        CS42528_WriteByte(0x03, 0x08);  // output map to SAI_SDOUT
        CS42528_WriteByte(0x06, (OMCK_SPEED_USED|0x02));  // disable auto switch
        CS42528_WriteByte(0x1f, 0x77);  // spdif TX from RX7(NC in the board), RX from RX7(NC)
        break;
    default :   // unknown source
        ASSERT(0);
        break;
    }
}

/**
 * brief : CS42528 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void CS42528Drv_Init(void)
{
    // fix me later
    CS42528_WriteByte(0x04, CODEC_I2S_FORMAT);  // I2S format setup
    CS42528_WriteByte(0x05, CODEC_M_S_NORAL);  // codec master/slave setup
    CS42528_WriteByte(0x0d, 0x38);  // soft ramp on zero crossing + auto mute on
    // setup the adc gain if necessary
//    CS42528_AdcGain(0);
    CS42528_WriteByte(0x1e, 0x04);  // interrupt : active low.
    CS42528_WriteByte(0x21, 0x80);  // enable the lock/unlock interrupt
    CS42528_WriteByte(0x24, 0x80);  // external filter configuration 3
    CS42528_WriteByte(0x27, 0x10);  // unmask the unlock
    CS42528_WriteByte(0x28, 0x1f);  // channel mute don't map to MUTEC

    CS42528Drv_SAIOutputSelect(SAI_SOURCE_ADC);

    CS42528Drv_DacMute(TRUE);

    CS42528_PowerDown(FALSE);   // power on the codec

#if 0
uint8_t power, power1;
//    CS42528_WriteByte(0x04, 0x40);  // I2S-24bits
//CS42528_WriteByte(0x02, 0x50);
CS42528_ReadByte(0x06, &power);
CS42528_ReadByte(0x1f, &power1);
BRINGUP_printf("\n\rp=0x%x,0x%x.\n\r", power,power1);
/*CS42528_ReadByte(0x04, &power);
CS42528_ReadByte(0x05, &power1);
BRINGUP_printf("\n\rp5=0x%x,0x%x.\n\r", power,power1);
CS42528_ReadByte(0x03, &power);
CS42528_ReadByte(0x06, &power1);
BRINGUP_printf("\n\rp7=0x%x,0x%x.\n\r", power,power1);
*/
#endif
}

/**
 * brief : CS42528 mute/unmute the DAC output
 * @param enable / disable
 * @return : none
 */
void CS42528Drv_DacMute(bool enable)
{
    uint8_t reg_value;

    if( enable )
        reg_value = 0xff;
    else
        reg_value = 0x00;

    CS42528_WriteByte(0x0e, reg_value);
}

#endif  // HAS_CS42528_CODEC

