/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  dsp adau1761 driver
                  -------------------------
@file        Adau1761_Drv.c
@brief       This file implements the drivers for adau1761
@author      Viking WANG
@date        2016-07-12
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "DspDrv.Config"
#include "commontypes.h"
#include "stm32f0xx.h"
#include "trace.h"
#include "I2CDrv.h"

#include "Adau1761_Drv.h"
#include "./Adau1761_Drv_priv.h"
#include "./Adau1761_Volume.h"
#ifdef DSP_TREBLE_BASS_TUNING
#include "./Adau1761_TrebleBass.h"
#endif

// dsp files
#include "./ADAU1761_IC_1.h"
#include "./ADAU1761_IC_1_REG.h"
#include "./ADAU1761_IC_1_PARAM.h"


#define ADAUDSP_SPECIAL_I2C

#define BYTES_PER_PROGRAM_ADDR      5
#define BYTES_PER_PARAM_ADDR        4

#ifdef ADAUDSP_SPECIAL_I2C
#define ADAUDSP_MAX_WRITE_BYTES     200
#define ADAUDSP_PROGRAM_ADDR_STEP   (ADAUDSP_MAX_WRITE_BYTES / BYTES_PER_PROGRAM_ADDR)   // program data : 5 bytes per register address
#define ADAUDSP_PARAM_ADDR_STEP     (ADAUDSP_MAX_WRITE_BYTES / BYTES_PER_PARAM_ADDR)   // parameter data : 4 bytes per register address
#endif

static uint8_t adau1761_dev_addr;
static uint8_t adau1761_inti_stage=0;

static Adau1761Drv_t adau1761_drv;
static Adau1761Drv_t *p_adau1761_drv=NULL;

/* init stage */
const static DspInitStage_t dsp_init_stages[] =
{
    {&Adau1761Drv_InitStage0, 50},
    {&Adau1761Drv_InitStage1, 0},
};

/* for sigma studio compiler */
static void SIGMA_WRITE_REGISTER_BLOCK(uint8_t device_add, uint16_t reg_add, uint16_t bytes, const uint8_t* data)
{
    eTpRet accessOK;

#ifdef ADAUDSP_SPECIAL_I2C
    tI2CMsg i2cMsg;

    ASSERT(p_adau1761_drv->isReady);

    if( ! p_adau1761_drv->i2cEnable )
        return ;

    i2cMsg.devAddr = device_add;
    i2cMsg.regAddr = reg_add;
    i2cMsg.length = bytes;
    i2cMsg.pMsg = (uint8_t *)data;

    accessOK = I2CDrv_MasterWriteWith2ByteRegAddress(&p_adau1761_drv->i2cDrv, &i2cMsg);
#else
    ASSERT(bytes <= ADAU1761_MAX_TX_BYTES);
    ASSERT(p_adau1761_drv->isReady);

    if( ! p_adau1761_drv->i2cEnable )
        return ;

    uint8_t buffer[ADAU1761_MAX_TX_BYTES+4];
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = NULL,
        .length = bytes+2
    };
    buffer[0] = (uint8_t)((reg_add&0xff00)>>8);
    buffer[1] = (uint8_t)(reg_add&0x00ff);
    memcpy(&buffer[2], data, bytes);
    i2cMsg.pMsg = &buffer[0];

    accessOK = I2CDrv_MasterWrite(&p_adau1761_drv->i2cDrv, &i2cMsg);
#endif

    ASSERT( accessOK == TP_SUCCESS );
}

static void Adau1761Drv_I2CWrite(uint16_t reg_add, uint16_t bytes, const uint8_t* data)
{
    eTpRet accessOK;

#ifdef ADAUDSP_SPECIAL_I2C
    tI2CMsg i2cMsg;

    ASSERT(p_adau1761_drv->isReady);

    if( ! p_adau1761_drv->i2cEnable )
        return ;

    i2cMsg.devAddr = adau1761_dev_addr;
    i2cMsg.regAddr = reg_add;
    i2cMsg.length = bytes;
    i2cMsg.pMsg = (uint8_t *)data;

    accessOK = I2CDrv_MasterWriteWith2ByteRegAddress(&p_adau1761_drv->i2cDrv, &i2cMsg);
#else
    ASSERT(bytes <= ADAU1761_MAX_TX_BYTES);
    ASSERT(p_adau1761_drv->isReady);

    if( ! p_adau1761_drv->i2cEnable )
        return ;

    uint8_t buffer[ADAU1761_MAX_TX_BYTES+4];
    tI2CMsg i2cMsg=
    {
        .devAddr = adau1761_dev_addr,
        .regAddr = NULL,
        .length = bytes+2
    };
    buffer[0] = (uint8_t)((reg_add&0xff00)>>8);
    buffer[1] = (uint8_t)(reg_add&0x00ff);
    memcpy(&buffer[2], data, bytes);
    i2cMsg.pMsg = &buffer[0];

    accessOK = I2CDrv_MasterWrite(&p_adau1761_drv->i2cDrv, &i2cMsg);
#endif

    ASSERT( accessOK == TP_SUCCESS );
}

static void Adau1761Drv_I2CRead(uint16_t reg_add, uint16_t bytes, uint8_t* data)
{
    ASSERT(bytes < 0x100);
    ASSERT(p_adau1761_drv->isReady);

    if( ! p_adau1761_drv->i2cEnable )
        return ;

    tI2CMsg i2cMsg =
    {
        .devAddr = adau1761_dev_addr,
        .regAddr = reg_add,
        .length  = bytes,
        .pMsg    = data
    };
    I2CDrv_MasterRead(&p_adau1761_drv->i2cDrv, &i2cMsg);
}

/**
 * Safeload process
 *
 * @param      num   : data length in word(4 bytes)
 * @param      array : data array
 * @param      addr  : target address
 * @return     none.
 */
static void Adau1761Drv_SafeLoadData(uint8_t num, const uint8_t *array, uint16_t addr)
{
    uint8_t  reg_data[4] = {0};
    ASSERT((num <= SAFE_LOAD_DATA_SIZE_MAX) && (num > 0));

    /* step1: set modulo size */
    reg_data[0] = 0;
    reg_data[1] = 0;
    reg_data[2] = 0;
    reg_data[3] = num;
    Adau1761Drv_I2CWrite(SAFE_LOAD_MODULO_SIZE_ADD, 4, reg_data);

    /* step2: write safeload data */
    Adau1761Drv_I2CWrite(SAFE_LOAD_DATA_START_ADD, (num<<2), array);

    /* step3: write target addr */
    -- addr;
    reg_data[0] = 0;
    reg_data[1] = 0;
    reg_data[2] = (uint8_t)((addr&0xff00) >> 8);
    reg_data[3] = (uint8_t)(addr&0x00ff);
    Adau1761Drv_I2CWrite(SAFE_LOAD_ADD_FOR_TARGET_ADD, 4, reg_data);

    /* step4: trigger safeload */
    reg_data[0] = 0;
    reg_data[1] = 0;
    reg_data[2] = 0;
    reg_data[3] = num;
    Adau1761Drv_I2CWrite(SAFE_LOAD_SET_DATA_SIZE_ADD, 4, reg_data);
}

static void Adau1761Drv_InitProgramArea(void)
{
    uint8_t *p_data;
    uint16_t reg_addr;
    uint32_t i, loop;

#ifdef ADAUDSP_SPECIAL_I2C
    loop = PROGRAM_SIZE_IC_1 / ADAUDSP_MAX_WRITE_BYTES;
    reg_addr = PROGRAM_ADDR_IC_1;
    p_data = (uint8_t *)(Program_Data_IC_1);

    for(i=0; i<loop; i++)
    {
        Adau1761Drv_I2CWrite(reg_addr, ADAUDSP_MAX_WRITE_BYTES, p_data);
        reg_addr += ADAUDSP_PROGRAM_ADDR_STEP;
        p_data += ADAUDSP_MAX_WRITE_BYTES;
    }
    loop = PROGRAM_SIZE_IC_1 % ADAUDSP_MAX_WRITE_BYTES;
    if( loop )
        Adau1761Drv_I2CWrite(reg_addr, loop, p_data);
#else
    loop = PROGRAM_SIZE_IC_1 / BYTES_PER_PROGRAM_ADDR;
    reg_addr = PROGRAM_ADDR_IC_1;
    p_data = (uint8_t *)(Program_Data_IC_1);

    for(i=0; i<loop; i++)
    {
        Adau1761Drv_I2CWrite(reg_addr, BYTES_PER_PROGRAM_ADDR, p_data);
        reg_addr ++;
        p_data += BYTES_PER_PROGRAM_ADDR;
    }
#endif
}

static void Adau1761Drv_InitParamArea(void)
{
    uint8_t *p_data;
    uint16_t reg_addr;
    uint32_t i, loop;

#ifdef ADAUDSP_SPECIAL_I2C
    loop = PARAM_SIZE_IC_1 / ADAUDSP_MAX_WRITE_BYTES;
    reg_addr = PARAM_ADDR_IC_1;
    p_data = (uint8_t *)(Param_Data_IC_1);

    for(i=0; i<loop; i++)
    {
        Adau1761Drv_I2CWrite(reg_addr, ADAUDSP_MAX_WRITE_BYTES, p_data);
        reg_addr += ADAUDSP_PARAM_ADDR_STEP;
        p_data += ADAUDSP_MAX_WRITE_BYTES;
    }
    loop = PARAM_SIZE_IC_1 % ADAUDSP_MAX_WRITE_BYTES;
    if( loop )
        Adau1761Drv_I2CWrite(reg_addr, loop, p_data);
#else
    loop = PARAM_SIZE_IC_1 / BYTES_PER_PARAM_ADDR;
    reg_addr = PARAM_ADDR_IC_1;
    p_data = (uint8_t *)(Param_Data_IC_1);

    for(i=0; i<loop; i++)
    {
        Adau1761Drv_I2CWrite(reg_addr, BYTES_PER_PARAM_ADDR, p_data);
        reg_addr ++;
        p_data += BYTES_PER_PARAM_ADDR;
    }
#endif
}

static void Adau1761Drv_InitStage0(void)
{
    /* initial the system clock, PLL clock etc... */
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SAMPLE_RATE_SETTING_IC_1_ADDR, REG_SAMPLE_RATE_SETTING_IC_1_BYTE, R0_SAMPLE_RATE_SETTING_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DSP_RUN_REGISTER_IC_1_ADDR, REG_DSP_RUN_REGISTER_IC_1_BYTE, R1_DSP_RUN_REGISTER_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_CLKCTRLREGISTER_IC_1_ADDR, REG_CLKCTRLREGISTER_IC_1_BYTE, R2_CLKCTRLREGISTER_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLLCRLREGISTER_IC_1_ADDR, REG_PLLCRLREGISTER_IC_1_BYTE, R3_PLLCRLREGISTER_IC_1_Default );
    /* delay for a while to wait the system ready */
}

static void Adau1761Drv_InitStage1(void)
{
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_PORT_CONTROL_0_IC_1_ADDR , R5_SERIAL_PORT_CONTROL_REGISTERS_IC_1_SIZE, R5_SERIAL_PORT_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ALC_CONTROL_0_IC_1_ADDR , R6_ALC_CONTROL_REGISTERS_IC_1_SIZE, R6_ALC_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_MICCTRLREGISTER_IC_1_ADDR, REG_MICCTRLREGISTER_IC_1_BYTE, R7_MICCTRLREGISTER_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_RECORD_PWR_MANAGEMENT_IC_1_ADDR , R8_RECORD_INPUT_SIGNAL_PATH_REGISTERS_IC_1_SIZE, R8_RECORD_INPUT_SIGNAL_PATH_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ADC_CONTROL_0_IC_1_ADDR , R9_ADC_CONTROL_REGISTERS_IC_1_SIZE, R9_ADC_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLAYBACK_MIXER_LEFT_CONTROL_0_IC_1_ADDR , R10_PLAYBACK_OUTPUT_SIGNAL_PATH_REGISTERS_IC_1_SIZE, R10_PLAYBACK_OUTPUT_SIGNAL_PATH_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_CONVERTER_CTRL_0_IC_1_ADDR , R11_CONVERTER_CONTROL_REGISTERS_IC_1_SIZE, R11_CONVERTER_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DAC_CONTROL_0_IC_1_ADDR , R12_DAC_CONTROL_REGISTERS_IC_1_SIZE, R12_DAC_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_PORT_PAD_CONTROL_0_IC_1_ADDR , R13_SERIAL_PORT_PAD_CONTROL_REGISTERS_IC_1_SIZE, R13_SERIAL_PORT_PAD_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_COMM_PORT_PAD_CTRL_0_IC_1_ADDR , R14_COMMUNICATION_PORT_PAD_CONTROL_REGISTERS_IC_1_SIZE, R14_COMMUNICATION_PORT_PAD_CONTROL_REGISTERS_IC_1_Default );
    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_JACKREGISTER_IC_1_ADDR, REG_JACKREGISTER_IC_1_BYTE, R15_JACKREGISTER_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DSP_ENABLE_REGISTER_IC_1_ADDR, REG_DSP_ENABLE_REGISTER_IC_1_BYTE, R21_DSP_ENABLE_REGISTER_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_CRC_IDEAL_1_IC_1_ADDR , R22_CRC_REGISTERS_IC_1_SIZE, R22_CRC_REGISTERS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_GPIO_0_CONTROL_IC_1_ADDR , R23_GPIO_REGISTERS_IC_1_SIZE, R23_GPIO_REGISTERS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_NON_MODULO_RAM_1_IC_1_ADDR , R24_NON_MODULO_REGISTERS_IC_1_SIZE, R24_NON_MODULO_REGISTERS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_WATCHDOG_ENABLE_IC_1_ADDR , R25_WATCHDOG_REGISTERS_IC_1_SIZE, R25_WATCHDOG_REGISTERS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SAMPLE_RATE_SETTING_IC_1_ADDR, REG_SAMPLE_RATE_SETTING_IC_1_BYTE, R26_SAMPLE_RATE_SETTING_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ROUTING_MATRIX_INPUTS_IC_1_ADDR, REG_ROUTING_MATRIX_INPUTS_IC_1_BYTE, R27_ROUTING_MATRIX_INPUTS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ROUTING_MATRIX_OUTPUTS_IC_1_ADDR, REG_ROUTING_MATRIX_OUTPUTS_IC_1_BYTE, R28_ROUTING_MATRIX_OUTPUTS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_DATAGPIO_PIN_CONFIG_IC_1_ADDR, REG_SERIAL_DATAGPIO_PIN_CONFIG_IC_1_BYTE, R29_SERIAL_DATAGPIO_PIN_CONFIG_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DSP_SLEW_MODES_IC_1_ADDR, REG_DSP_SLEW_MODES_IC_1_BYTE, R30_DSP_SLEW_MODES_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_PORT_SAMPLE_RATE_SETTING_IC_1_ADDR, REG_SERIAL_PORT_SAMPLE_RATE_SETTING_IC_1_BYTE, R31_SERIAL_PORT_SAMPLE_RATE_SETTING_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_CLOCK_ENABLE_REG_0_IC_1_ADDR , R32_CLOCK_ENABLE_REGISTERS_IC_1_SIZE, R32_CLOCK_ENABLE_REGISTERS_IC_1_Default );

    // write the program data & parameter data.
    Adau1761Drv_InitProgramArea();
    Adau1761Drv_InitParamArea();

    SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, NON_MODULO_RAM_ADDR_IC_1, NON_MODULO_RAM_SIZE_IC_1, NON_MODULO_RAM_Data_IC_1 );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SAMPLE_RATE_SETTING_IC_1_ADDR, REG_SAMPLE_RATE_SETTING_IC_1_BYTE, R36_SAMPLE_RATE_SETTING_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DSP_RUN_REGISTER_IC_1_ADDR, REG_DSP_RUN_REGISTER_IC_1_BYTE, R37_DSP_RUN_REGISTER_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DEJITTER_REGISTER_CONTROL_IC_1_ADDR, REG_DEJITTER_REGISTER_CONTROL_IC_1_BYTE, R38_DEJITTER_REGISTER_CONTROL_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_DEJITTER_REGISTER_CONTROL_IC_1_ADDR, REG_DEJITTER_REGISTER_CONTROL_IC_1_BYTE, R39_DEJITTER_REGISTER_CONTROL_IC_1_Default );
}

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Adau1761Drv_Ctor(void)
{
    tI2CDevice *p_i2c_device;

    p_adau1761_drv = &adau1761_drv;

    ASSERT( ! p_adau1761_drv->isReady );

    p_i2c_device = (tI2CDevice *) getDevicebyIdAndType(DSP_DEV_ID, I2C_DEV_TYPE, NULL);
    ASSERT(p_i2c_device);

    adau1761_dev_addr = p_i2c_device->devAddress;
    I2CDrv_Ctor((&p_adau1761_drv->i2cDrv), p_i2c_device);
    p_adau1761_drv->isReady = TRUE;
    p_adau1761_drv->i2cEnable = TRUE;
    adau1761_inti_stage = 0;
}

/**
 * Exit & clean up the driver.
 */
void Adau1761Drv_Xtor(void)
{
    ASSERT(p_adau1761_drv && p_adau1761_drv->isReady);

    I2CDrv_Xtor( &p_adau1761_drv->i2cDrv );
    p_adau1761_drv->isReady = FALSE;
    p_adau1761_drv->i2cEnable = FALSE;
    adau1761_inti_stage = 0;
}

/**
 * Enable/Disalbe the I2C bus access
 */
void Adau1761Drv_I2cEnable(bool enable)
{
    p_adau1761_drv->i2cEnable = enable;
}

/**
 * Write the dsp program/parameter data.
 */
uint16_t Adau1761Drv_Init(void)
{
    uint8_t total_stages;
    uint16_t delay_time;

    ASSERT(p_adau1761_drv->isReady);

    total_stages = ArraySize(dsp_init_stages);
    ASSERT(adau1761_inti_stage < total_stages);

    dsp_init_stages[adau1761_inti_stage].initSectionFunc();
    delay_time = dsp_init_stages[adau1761_inti_stage].delay_time;

    adau1761_inti_stage ++;
    if( adau1761_inti_stage == total_stages )
        delay_time = 0;

    return delay_time;
}

/*
 * mute the LOUTP/N & ROUTP/N
 */
void Adau1761Drv_DacMute(bool mute_enable)
{
    uint8_t left, right;
    Adau1761Drv_I2CRead(REG_PLAYBACK_LINE_OUT_LEFT_IC_1_ADDR, REG_PLAYBACK_LINE_OUT_LEFT_IC_1_BYTE, (uint8_t *)(&left));
    Adau1761Drv_I2CRead(REG_PLAYBACK_LINE_OUT_RIGHT_IC_1_ADDR, REG_PLAYBACK_LINE_OUT_RIGHT_IC_1_BYTE, (uint8_t *)(&right));

    if( mute_enable )
    {
        left &= 0xfd;
        right &= 0xfd;
    }
    else
    {
        left |= 0x02;
        right |= 0x02;
    }

    Adau1761Drv_I2CWrite(REG_PLAYBACK_LINE_OUT_LEFT_IC_1_ADDR, REG_PLAYBACK_LINE_OUT_LEFT_IC_1_BYTE, (uint8_t *)(&left));
    Adau1761Drv_I2CWrite(REG_PLAYBACK_LINE_OUT_RIGHT_IC_1_ADDR, REG_PLAYBACK_LINE_OUT_RIGHT_IC_1_BYTE, (uint8_t *)(&right));
}

/*
 * setup the DSP to AUX-in or LINE-in
 */
void Adau1761Drv_AnalogInSource(DspAnalogIn_t analog_in)
{
// dsp v0.1 only support analog in or i2s in.

    uint16_t reg_addr;
    uint8_t reg_data[4] = {0};

    if (analog_in == DSP_ANALOG_LINE_IN)
    {
        reg_addr = REG_RECORD_VOLUME_CTRL_LEFT_IC_1_ADDR;
        reg_data[0] = 0x23;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_MIXER_LEFT_CTRL_1_IC_1_ADDR;
        reg_data[0] = 0x08;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_VOLUME_CTRL_RIGHT_IC_1_ADDR;
        reg_data[0] = 0x23;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_MIXER_RIGHT_CTRL_1_IC_1_ADDR;
        reg_data[0] = 0x08;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));
    }
    else if (analog_in == DSP_ANALOG_AUX_IN)
    {
        reg_addr = REG_RECORD_VOLUME_CTRL_LEFT_IC_1_ADDR;
        reg_data[0] = 0x22;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_MIXER_LEFT_CTRL_1_IC_1_ADDR;
        reg_data[0] = 0x05;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_VOLUME_CTRL_RIGHT_IC_1_ADDR;
        reg_data[0] = 0x22;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));

        reg_addr = REG_RECORD_MIXER_RIGHT_CTRL_1_IC_1_ADDR;
        reg_data[0] = 0x05;
        Adau1761Drv_I2CWrite(reg_addr, 1, (uint8_t *)(&reg_data));
    }

    reg_addr = DSP_SOURCE_SWITCH_ADDR;
    reg_data[3] = AUDIO_SOURCE_AUX;
    Adau1761Drv_I2CWrite(reg_addr, 4, (uint8_t *)(&reg_data));
}

/*
 * select the I2S source, ADC-in or I2S in
 */
void Adau1761Drv_I2SInSource(DspI2SChannel_t i2s_source)
{
    // dsp v0.1 only support analog in or i2s in.

    uint16_t reg_addr;
    uint8_t reg_data[4] = {0};

    ASSERT(i2s_source < DSP_I2S_MAX);

    reg_addr = DSP_SOURCE_SWITCH_ADDR;

    reg_data[3] = AUDIO_SOURCE_I2S;  //0x01 is for i2s   0x0 is for adc

    Adau1761Drv_I2CWrite(reg_addr, 4, (uint8_t *)(&reg_data));
}

/*
 * mute the I2S input in Sigma studio data flow
 * dsp project need the 'mute' module to support
 */
void Adau1761Drv_I2SMute(bool mute_enable)
{
    // not support from current dsp v0.1

}

/*
 * setup the volume, range from 0 ~ (MAX_VOLUME_STEPS-1)
 * NOTE : the volume control module must be "shared slider" mode.
 */
void Adau1761Drv_SetVolume(uint32_t vol_value)
{
    uint16_t reg_addr;
    uint8_t reg_data[4];
    uint32_t reg_value;

    ASSERT(vol_value < MAX_VOLUME_STEPS);

    reg_addr = DSP_SW_VOL_CONTROL_ADDR1;

    reg_value = DSP_Gain_Signed_Format_5_23_Table[vol_value];
    reg_data[0] = (uint8_t)((reg_value >> 24) & 0x00ff);
    reg_data[1] = (uint8_t)((reg_value >> 16) & 0x00ff);
    reg_data[2] = (uint8_t)((reg_value >> 8) & 0x00ff);
    reg_data[3] = (uint8_t)(reg_value & 0x00ff);

    Adau1761Drv_I2CWrite(reg_addr, 4, reg_data);

    reg_addr = DSP_SW_VOL_CONTROL_ADDR2;

    Adau1761Drv_I2CWrite(reg_addr, 4, reg_data);
}

/*
 * setup the bass, range from 0 ~ (MAX_BASS_STEPS-1)
 * NOTE : dsp flow related
 */
void Adau1761Drv_SetBass(uint32_t bass_level)
{
    ASSERT(bass_level < MAX_BASS_STEPS);
#ifdef DSP_TREBLE_BASS_TUNING
    uint16_t reg_addr;
    uint8_t *p_reg_data;

    reg_addr = MOD_USERSPECIALFEATURE_TREBLEBASS_ALG0_STAGE0_B0_ADDR;
    p_reg_data = (uint8_t *)Adau1761_BassTable;
    p_reg_data += bass_level * TREBLE_BASS_PARAM_TOTAL_SIZES;

    Adau1761Drv_SafeLoadData(TREBLE_BASS_PARAM_LENGHT, p_reg_data, reg_addr);
#endif
}

/*
 * setup the bass, range from 0 ~ (MAX_TREBLE_STEPS-1)
 * NOTE : dsp flow related
 */
void Adau1761Drv_SetTreble(uint32_t treble_level)
{
    ASSERT(treble_level < MAX_TREBLE_STEPS);
#ifdef DSP_TREBLE_BASS_TUNING
    uint16_t reg_addr;
    uint8_t *p_reg_data;

    reg_addr = MOD_USERSPECIALFEATURE_TREBLEBASS_ALG0_STAGE1_B0_ADDR;
    p_reg_data = (uint8_t *)Adau1761_TrebleTable;
    p_reg_data += treble_level * TREBLE_BASS_PARAM_TOTAL_SIZES;

    Adau1761Drv_SafeLoadData(TREBLE_BASS_PARAM_LENGHT, p_reg_data, reg_addr);
#endif
}

/*
 * bypass enable/disable
 * NOTE : dsp flow related
 */
void Adau1761Drv_BypassEnable(bool enable)
{
    uint16_t reg_addr_a,reg_addr_b;
    uint8_t reg_data[4]= {0x00, 0x00, 0x00, 0x00};

    reg_addr_a = DSP_EQ_BYPASS_SWITCH_ADDR1;
    reg_addr_b = DSP_EQ_BYPASS_SWITCH_ADDR1;
    if( enable )
    {
        Adau1761Drv_I2CWrite(reg_addr_a, 4, reg_data);
        reg_data[1] = 0x80;
        Adau1761Drv_I2CWrite(reg_addr_b, 4, reg_data);
    }
    else
    {
        Adau1761Drv_I2CWrite(reg_addr_b, 4, reg_data);
        reg_data[1] = 0x80;
        Adau1761Drv_I2CWrite(reg_addr_a, 4, reg_data);
    }
}


/*
 * music detect on current source
 * NOTE : dsp flow related
 */
bool Adau1761Drv_MusicDetected(void)
{
    return TRUE;// to be support

}
