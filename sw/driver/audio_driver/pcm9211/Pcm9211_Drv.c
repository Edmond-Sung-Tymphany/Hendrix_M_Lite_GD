/**
 *  @file      Pcm9211_Drv.c
 *  @brief     This file contains the PCM9211 codec driver implementation.
 *  @author    Viking Wang
 *  @date      16-May-2016
 *  @copyright Tymphany Ltd.
 */

#include "stm32f0xx.h"

#ifdef HAS_PCM9211_CODEC

#include "commonTypes.h"
#include "i2cDrv.h"
#include "trace.h"
#include "assert.h"
#include "pcm9211_drv.h"

/* reset timer, the spec says >= 1us ????? */
#define PCM9211_RESET_TIMEOUT_MS    2

#ifdef PCM9211DRV_DEBUG
    #define PCM9211_printf  TP_PRINTF
#else
    #define PCM9211_printf(...)
#endif

extern uint32_t getSysTime(void);

/* driver instance */
static stPcm9211Drv_t   pcm9211Drv;
static stPcm9211Drv_t   *p_pcm9211Drv=NULL;

static uint8_t pcm9211_reg_dir_source=DIR_SOURCE_RX2;
static uint8_t pcm9211_reg_main_output=0x00;    // the default value is 0x00 after reset.
static uint8_t pcm9211_reg_adc_gain=ADC_GAIN_0dB;

/* setup the reset GPIO to high/low */
__weak void Pcm9211_ResetSetup(int high)
{
    (void)high;
}

/* config the reset GPIO pin */
__weak void Pcm9211_ResetGPIOInit(void)
{
}

/**
 * Construct the Pcm9211 driver instance.
 * @return : none
 */
void Pcm9211Drv_Ctor(void)
{
    stPcm9211Device_t *p_pcm9211Device;

    p_pcm9211Drv = &pcm9211Drv;

    ASSERT(p_pcm9211Drv && (! p_pcm9211Drv->isReady) );

    p_pcm9211Drv->deviceID = PCM9211_DEV_ID;
    p_pcm9211Drv->deviceType = I2C_DEV_TYPE;
    p_pcm9211Device = (stPcm9211Device_t *) getDevicebyIdAndType(p_pcm9211Drv->deviceID, p_pcm9211Drv->deviceType, NULL);

    I2CDrv_Ctor((&p_pcm9211Drv->i2cDrv), (tI2CDevice *)(p_pcm9211Device->p_I2cDevice));
    p_pcm9211Drv->isReady = TRUE;
    p_pcm9211Drv->i2cEnable = TRUE;

    Pcm9211_Init();
}

/**
 * Exit & clean up the driver.
 */
void Pcm9211Drv_Xtor(void)
{
    ASSERT(p_pcm9211Drv && p_pcm9211Drv->isReady);

    Pcm9211_ResetSetup(0);  // hold the reset pin

    I2CDrv_Xtor( &p_pcm9211Drv->i2cDrv );
    p_pcm9211Drv->isReady = FALSE;
    p_pcm9211Drv->i2cEnable = FALSE;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void Pcm9211Drv_I2cEnable(bool enable)
{
    p_pcm9211Drv->i2cEnable = enable;
}

static bool Pcm9211_WriteByte(uint8_t addr, uint8_t data)
{
    uint8_t reg_data[2];
    
    ASSERT( p_pcm9211Drv->isReady );

    if( ! p_pcm9211Drv->i2cEnable )
        return TP_ACCESS_ERROR;

    reg_data[0] = addr;
    reg_data[1] = data;
    
    tI2CMsg i2cMsg=
    {
        .devAddr = p_pcm9211Drv->i2cDrv.pConfig->devAddress,
        .regAddr = NULL,
        .length = 2,
        .pMsg = (uint8_t *)reg_data,
    };
    bool ret;
    ret = (bool)I2CDrv_MasterWrite(&p_pcm9211Drv->i2cDrv, &i2cMsg);
    ASSERT(ret==TP_SUCCESS);

    return ret;
}

static bool Pcm9211_ReadByte(uint8_t addr, uint8_t *p_byte)
{
    ASSERT( p_pcm9211Drv->isReady );

    if( ! p_pcm9211Drv->i2cEnable )
        return TP_ACCESS_ERROR;

    tI2CMsg i2cMsg=
    {
        .devAddr = p_pcm9211Drv->i2cDrv.pConfig->devAddress,
        .regAddr = addr,
        .length = 1,
        .pMsg = p_byte,
    };
    bool ret;
    ret = I2CDrv_MasterRead(&p_pcm9211Drv->i2cDrv, &i2cMsg);
//    ASSERT(ret==TP_SUCCESS);

    return ret;
}

static bool Pcm9211_ReadArray(uint8_t addr, uint16_t len, uint8_t *p_buf)
{
    ASSERT( p_pcm9211Drv->isReady );

    if( ! p_pcm9211Drv->i2cEnable )
        return TP_ACCESS_ERROR;

    tI2CMsg i2cMsg=
    {
        .devAddr = p_pcm9211Drv->i2cDrv.pConfig->devAddress,
        .regAddr = addr,
        .length = len,
        .pMsg = p_buf,
    };
    bool ret;
    ret = I2CDrv_MasterRead(&p_pcm9211Drv->i2cDrv, &i2cMsg);
//    ASSERT(ret==TP_SUCCESS);

    return ret;
}

/**
 * brief : setup the ADC gain, range : mute/-100dB ~ +20dB
 * @param - gain : adc gain setting
 * @return : none
 */
void Pcm9211_AdcGain(int8_t gain)
{
    uint8_t reg_gain;

    ASSERT( gain < 21 );
    
    if( gain < -100 )   // mute
        reg_gain = 0x00;
    else
    {
        reg_gain = ADC_GAIN_0dB;
        if( gain >=0 )
            reg_gain += gain * 2;
        else
        {
            gain = -gain;
            reg_gain -= gain * 2;
        }
    }

    pcm9211_reg_adc_gain = reg_gain;

    Pcm9211_WriteByte(PCM9211_REG_ADC_L_GAIN, reg_gain);  // left channel 
    Pcm9211_WriteByte(PCM9211_REG_ADC_R_GAIN, reg_gain);  // right channel 
}

/**
 * brief : select the DIR0..11 source
 * @param - DIRx : RX port
 * @return : none
 */
void Pcm9211_DirSourceSelect(uint8_t DIRx)
{
    ASSERT( DIRx < 0x10 );

    pcm9211_reg_dir_source &= 0xf0;
    pcm9211_reg_dir_source |= DIRx;
    Pcm9211_WriteByte(PCM9211_REG_DIR_SOURCE, pcm9211_reg_dir_source);
}

/**
 * brief : get the sample rate of input audio
 * @param - none
 * @return : sample rate value:SAMPLE_RATE_xxxxx
 */
uint8_t Pcm9211_GetSampleRate(void)
{
    uint8_t reg_value;

    if( TP_SUCCESS != Pcm9211_ReadByte(PCM9211_REG_SAMPLE_RATE, &reg_value) )
    {   // i2c bus error
        reg_value = 0x80;   // set to NOT_READY flag
    }

    // check SFSST status is 0?
    if( reg_value & 0x80 )
        reg_value = SAMPLE_RATE_NOT_READY;
    else
        reg_value &= 0x0f;

    return reg_value;
}

/**
 * brief : main output select, DIR, ADC, RX4~7/AUXIN0, MPIO_C:AUXIN1, MPIO_B:AUXIN2
 * @param - setting -> refer to pcm9211_drv.h
 * @return : none
 */
void Pcm9211_MainOutputSetting(uint8_t setting)
{
    pcm9211_reg_main_output = setting;
    Pcm9211_WriteByte(PCM9211_REG_MAIN_OUTPUT, pcm9211_reg_main_output);
}

/**
 * brief : Pcm9211 initial, write the necessary config register data to the chip.
 * this is for ATMOS board setting.
 * @return : none
 */
void Pcm9211_Init(void)
{
    uint32_t tickEnd;
    
    Pcm9211_ResetGPIOInit();
    Pcm9211_ResetSetup(0);
    tickEnd = getSysTime() + PCM9211_RESET_TIMEOUT_MS;
//    tickEnd = getSysTime() + 50;
    while( getSysTime() < tickEnd );
    Pcm9211_ResetSetup(1);

    // delay for a while to wait the PCM9211 ready??????
    tickEnd = getSysTime() + 3;
    while( getSysTime() < tickEnd );

    /* initial the register */
    pcm9211_reg_dir_source = RX0_AMP_ENABLE | RX1_AMP_DISABLE | DIR_SOURCE_RX2; // atmos board used RX0 for coaxial.
    Pcm9211_WriteByte(PCM9211_REG_DIR_SOURCE, pcm9211_reg_dir_source);
//  Pcm9211_AdcGain(0);     // the default is 0dB, so needn't to enable, if you want change the gain, enable it
//    Pcm9211_MainOutputSetting(MAIN_OUTPUT_SOURCE_AUTO); // the default value is "AUTO"
    /* config the MPIO_A/B/C, ATMOS board, MPIO_A&MPIO_B is unused, MPIO_C used for MTK8507 I2S in */
    Pcm9211_WriteByte(0x37, 0x05);  // target port setting to Main output
    Pcm9211_WriteByte(0x6d, 0xf0);  // MPIO_B -> Hi-Z
    Pcm9211_WriteByte(0x6e, 0x0f);  // MPIO_A -> Hi-Z, MPIO_C -> output
    Pcm9211_WriteByte(0x6f, 0x00);  // MPIO_A:RX4~7, MPIO_B:AUXIN2, MPIO_C:AUXIN1
    Pcm9211_WriteByte(0x78, 0x00);  // MPO_0:Hi-Z, MPO_1:Hi-Z
}

#endif  // HAS_PCM9211_CODEC

