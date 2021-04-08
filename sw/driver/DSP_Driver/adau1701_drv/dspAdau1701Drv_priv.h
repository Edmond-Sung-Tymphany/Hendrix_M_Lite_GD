/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  dsp 1701 driver
                  -------------------------

                  SW Module Document




@file        dsp_adau1701_driver.h
@brief       This file declares the structures and macros for adau1701
@author      Edmond Sung
@date        2015-10-27
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#ifndef __ADAU1701_DRV_H__
#define __ADAU1701_DRV_H__
#include "DspDrv1701.h"

#define DB_INDEX_OF_LP     0
#define DB_INDEX_OF_RGC    12
#define DB_INDEX_OF_PEQ1   3
#define DB_INDEX_OF_PEQ2   6
#define DB_INDEX_OF_PEQ3   9
#define DB_INDEX_OF_PHASE  2
#define DB_INDEX_OF_VOL    19

#define ADDR_OFFSET_0      0
#define ADDR_OFFSET_1      1
#define ADDR_OFFSET_2      2
#define ADDR_OFFSET_3      3
#define ADDR_OFFSET_4      4

#define CTR_ADDR_GROUP_1   1
#define CTR_ADDR_GROUP_2   2
#define CTR_ADDR_GROUP_3   3
#define CTR_ADDR_GROUP_4   4

#define EQ_CTRL_RAM_SLOT_NUM    (5)
#define PARAM_RAM_WORD_LEN      (4)
#define EQ_CTRL_RAM_BYTE_NUM    (PARAM_RAM_WORD_LEN*EQ_CTRL_RAM_SLOT_NUM)

#define PI                      (3.1415926536)
#define NUM_OF_FILTER_ADD       (4)
#define NUM_OF_RGC_SET_ADD      (2)
#define PARAM_EQ_GAIN           (0)

#define NO_PHASE_SHIFT          (1)
#define MAX_DELAY_SAMPLES       480

#define PROGRAM_RAM_ADDRESS_LOW_BOUNDARY        (0x0400)
#define PROGRAM_RAM_ADDRESS_HIGH_BOUNDARY       (0x07FF)
#define PARAM_RAM_ADDRESS_LOW_BOUNDARY          (0x0000)
#define PARAM_RAM_ADDRESS_HIGH_BOUNDARY         (0x03FF)
#define REG_ADDRESS_LEN                         (2)
#define PROGRAM_RAM_DATA_LEN                    (5)
#define PARAM_RAM_PAGE_SIZE                     (32)
#define PARAM_RAM_DATA_LEN                      (4)
#define SHIFT_EIGHT_BIT                         (8)
#define MASK_LOW_BYTE                           (0xFF)
//#define AUDIO_DETECT_REGISTER                   0x0801

#define POS_DATA_BYTE_LEN                       (4)

typedef struct tCtrIdEQIdMap
{
    eAudioSettId dspSettid;
    uint8        dbIndex;  /* setting database index */
}tCtrIdEQIdMap;

typedef struct tCtrlInputIdMap
{
    eAudioChannel InputId; /* the input id requedted by audio server. */
    uint8                 controllerId; /*in case there can be more than 1 controller address in one array, we need to find the correct address*/
    uint8                 sourceIdx; /* the index of the source on the controller*/
}tCtrlInputIdMap;

typedef struct
{
    const uint8* head;     /*pointer to the tabe head*/
    uint8        type;     /*controller type*/
    uint8          id;     /*in case there can be more than 1 controller address in one array, we need to find the correct address*/
    uint16       addr;
} tTunableInfo;

typedef enum
{
    INIT_DATA_SEGMENT_1,
    INIT_DATA_SEGMENT_2,
    INIT_DATA_SEGMENT_3,
    INIT_DATA_SEGMENT_4,
    INIT_DATA_SEGMENT_5
}eInitDataSegment;


typedef struct tInitTableTXCtrl
{
    const uint8*        pOffset;
    uint32              NumOfDataInRow;
    uint16              tableRowNum;
    eInitDataSegment    dataSegment;
}tInitTableTXCtrl;

/*
 * The filter types we can support
 */
typedef enum
{
    FILTER_TYPE_PEAKING,
    FILTER_TYPE_TONE_HIGH_SHELF,
    FILTER_TYPE_TONE_LOW_SHELF,
    FILTER_TYPE_GENERAL_HIGH_PASS,
    FILTER_TYPE_GENERAL_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_HIGH_PASS,
    FILTER_TYPE_BESSEL_LOW_PASS,
    FILTER_TYPE_BESSEL_HIGH_PASS,
    FILTER_TYPE_FIRST_ORDER_HIGH_PASS,
    FILTER_TYPE_FIRST_ORDER_LOW_PASS,
} eDspFiltTyp;

/* necessary constants for each EQ part */
/* THE FOLLOWING ENUM IS PRODUCT-RELATED, USER CAN DEFINE THE AUDIO CFG CHOICES HERE */
/* Tihs is just an example, user can modify it according the HW design */
typedef enum
{
    PROD_AUD_IN_OPTION0,
    PROD_AUD_IN_OPTION1,
    PROD_AUD_IN_OPTION2
} eDspAudInOption;

typedef enum
{
    PROD_AUD_OUT_OPTION0
} eDspAudOutOption;

/* RGC */
typedef enum
{
    USER_RGC_SLOPE_6DB  =  6,
    USER_RGC_SLOPE_12DB = 12,
} eUserRGCSlope;
#define RGC_GAIN                        (0)
#define RGC_QFACTOR                     (1)
#define RGC_BOOST                       (0)

const uint8 ADAU1701_CLEAR_INT_REGS[]   = {
                                           0x08, 0x1c,  // DSP CORE CONTROL REGISTER
                                           0x00, 0x18}; // Mute ADCs & DACs. Clear Internal Registers to 0.
const uint8 ADAU1701_RESUME_SIGNAL[]    = {
                                           0x08, 0x1c,  // DSP CORE CONTROL REGISTER
                                           0x00, 0x1c}; // Mute ADCs & DACs. Resume audio after clearing Internal Registers to 0.
const uint8 ADAU1701_UNLOCK_REGISTERS[] = {
                                           0x08, 0x1c,  // DSP CORE CONTROL REGISTER
                                           0x00, 0x5c}; // Mute ADCs & DACs. Clear Int Regs to 0. Write directly to the interface registers (2048 to 2055)
const uint8 ADAU1701_LOCK_REGISTERS[]   = {
                                           0x08, 0x1c, // DSP CORE CONTROL REGISTER
                                           0x00, 0x1c}; // Mute ADCs & DACs. Stop direct write to the interface registers

const uint8 ADAU1701_SYSTEM_MUTE[]      = {0x00, 0x27, // DSP mute block register
                                           0x00, 0x00, 0x00, 0x00}; // value of the register
const uint8 ADAU1701_SYSTEM_UNMUTE[]    = {0x00, 0x27,// DSP mute block register
                                           0x00, 0x80, 0x00, 0x00}; // value of the register
/* consts for User LP */
typedef enum
{
    USER_LP_SLOPE_6DB  =  6,
    USER_LP_SLOPE_12DB = 12,
    USER_LP_SLOPE_18DB = 18,
    USER_LP_SLOPE_24DB = 24,
} eUserLpSlope;

#define USER_LP_QFACTOR           (1.41)
#define USER_LP_BOOST             (0)
#define USER_LP_GAIN              (0)

/* consts for User HP */
typedef enum
{
    USER_HP_SLOPE_6DB  =  6,
    USER_HP_SLOPE_12DB = 12,
    USER_HP_SLOPE_18DB = 18,
    USER_HP_SLOPE_24DB = 24,
} eUserHpSlope;

#define USER_HP_QFACTOR           (1.41)
#define USER_HP_BOOST             (0)
#define USER_HP_GAIN              (0)

/* consts for TUNING */
typedef enum
{
    TUNING_RANGE_20HZ   = 20,
    TUNING_RANGE_16HZ   = 16,
    TUNING_RANGE_SEALED = 12,
} eTuningRange;

#define TUNING_FREQ               (50)
#define TUNING_QFACTOR            (1)
#define TUNING_BOOST0             (-12)
#define TUNING_BOOST1             (-6)
#define TUNING_GAIN               (0)

const uint8 eqBypassData[] =
{
    0x00, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
};

/* Parametric EQ */
typedef enum
{
    PARAMETRIC_EQ_0,
    PARAMETRIC_EQ_1,
    PARAMETRIC_EQ_2,
    PARAMETRIC_EQ_MAX,
} eParametricEQ;

#define ADAU1701_DSP_CORE_CTRL_REG_LEN        4
#define ADAU1701_AUDIO_DETECT_LEN             4
#define ADAU1701_INPUT_SEL_LEN                6
#define ADAU1701_UPDATE_VOLUME_LEN            6
#define ADAU1701_SET_MUTE_LEN                 6
#define AUDIO_DETECTED                        (0x00)
#define AUDIO_NOT_DETECTED                    (0x80)

/* DSP chip address */
#define SAFE_LOAD_MODULO_SIZE_ADD             (0x0000)
#define SAFE_LOAD_DATA_START_ADD              (0x0001)
#define SAFE_LOAD_ADD_FOR_TARGET_ADD          (0x0006)
#define SAFE_LOAD_SET_DATA_SIZE_ADD           (0x0007)
#define SAFE_LOAD_DATA_SIZE_MAX               (5)

/* GPIO pin control mirror register while controlled by MCU */
#define GPIO0_SET_REG                         (0x0620)
#define GPIO1_SET_REG                         (0x0621)
#define GPIO2_SET_REG                         (0x0622)
#define GPIO3_SET_REG                         (0x0623)

/* ClkCtrlRegister  - Registers (IC 1) */
#define CLKCTRLREGISTER_ADDR                  (0x4000)

/* RegPowCtrlRegister  - Registers (IC 1) */
#define REGPOWCTRLREGISTER_ADDR               (0x4001)

/* PLLCrlRegister  - Registers (IC 1) */
#define PLLCTRLREGISTER_ADDR                  (0x4002)

/* MicCtrlRegister  - Registers (IC 1) */
#define MICCTRLREGISTER_ADDR                  (0x4008)

/* Record Pwr Management  - Registers (IC 1) */
#define RECORD_PWR_MANAGEMENT_ADDR            (0x4009)

/* Record Mixer Left Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_0_ADDR         (0x400A)

/* Record Mixer Left Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_1_ADDR         (0x400B)

/* Record Mixer Right Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_0_ADDR        (0x400C)

/* Record Mixer Right Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_1_ADDR        (0x400D)

/* Record Volume Ctrl Left  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_LEFT_ADDR          (0x400E)

/* Record Volume Ctrl Right  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_RIGHT_ADDR         (0x400F)

/* Record Mic Bias Control  - Registers (IC 1) */
#define RECORD_MIC_BIAS_CONTROL_ADDR          (0x4010)

/* ALC Control 0  - Registers (IC 1) */
#define ALC_CONTROL_0_ADDR                    (0x4011)

/* ALC Control 1  - Registers (IC 1) */
#define ALC_CONTROL_1_ADDR                    (0x4012)

/* ALC Control 2  - Registers (IC 1) */
#define ALC_CONTROL_2_ADDR                    (0x4013)

/* ALC Control 3  - Registers (IC 1) */
#define ALC_CONTROL_3_ADDR                    (0x4014)

/* Serial Port Control 0  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_0_ADDR            (0x4015)

/* Serail Port Control 1  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_1_ADDR            (0x4016)

/* Converter Ctrl 0  - Registers (IC 1) */
#define CONVERTER_CTRL_0_ADDR                 (0x4017)

/* Converter Ctrl 1  - Registers (IC 1) */
#define CONVERTER_CTRL_1_ADDR                 (0x4018)

/* ADC Control 0  - Registers (IC 1) */
#define ADC_CONTROL_0_ADDR                    (0x4019)

/* ADC Control 1  - Registers (IC 1) */
#define ADC_CONTROL_1_ADDR                    (0x401A)

/* ADC Control 2  - Registers (IC 1) */
#define ADC_CONTROL_2_ADDR                    (0x401B)

/* Playback Mixer Left Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_0_ADDR    (0x401C)

/* Plaback Mixer Left Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_1_ADDR    (0x401D)

/* Plaback Mixer Right Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_0_ADDR   (0x401E)

/* Playback Mixer Right Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_1_ADDR   (0x401F)

/* Playback LR Left  - Registers (IC 1) */
#define PLAYBACK_LR_LEFT_ADDR                 (0x4020)

/* Playback LR Right  - Registers (IC 1) */
#define PLAYBACK_LR_RIGHT_ADDR                (0x4021)

/* Playback LR Mono Ctrl  - Registers (IC 1) */
#define PLAYBACK_LR_MONO_CTRL_ADDR            (0x4022)

/* Playback Headphone Left  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_LEFT_ADDR          (0x4023)

/* Playback Headphone Right  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_RIGHT_ADDR         (0x4024)

/* Playback Line Out Left  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_LEFT_ADDR           (0x4025)

/* Playback Line Out Right  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_RIGHT_ADDR          (0x4026)

/* Playback Line Out Mono  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_MONO_ADDR           (0x4027)

/* Playback Control  - Registers (IC 1) */
#define PLAYBACK_CONTROL_ADDR                 (0x4028)

/* Playback Power Management  - Registers (IC 1) */
#define PLAYBACK_POWER_MANAGEMENT_ADDR        (0x4029)

/* DAC Control 0  - Registers (IC 1) */
#define DAC_CONTROL_0_ADDR                    (0x402A)

/* DAC Control 1  - Registers (IC 1) */
#define DAC_CONTROL_1_ADDR                    (0x402B)

/* DAC Control 2  - Registers (IC 1) */
#define DAC_CONTROL_2_ADDR                    (0x402C)

/* Serial Port Pad Control 0  - Registers (IC 1) */
#define SERIAL_PORT_PAD_CONTROL_0_ADDR        (0x402D)

/* Comm Port Pad Ctrl 0  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_0_ADDR             (0x402F)

/* Comm Port Pad Ctrl 1  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_1_ADDR             (0x4030)

/* JackRegister  - Registers (IC 1) */
#define JACKREGISTER_ADDR                     (0x4031)

/* Dejitter Register Control  - Registers (IC 1) */
#define DEJITTER_REGISTER_CONTROL_ADDR        (0x4036)

/* CRC Ideal_1  - Registers (IC 1) */
#define CRC_IDEAL_1_ADDR                      (0x40C0)

/* CRC Ideal_2  - Registers (IC 1) */
#define CRC_IDEAL_2_ADDR                      (0x40C1)

/* CRC Ideal_3  - Registers (IC 1) */
#define CRC_IDEAL_3_ADDR                      (0x40C2)

/* CRC Ideal_4  - Registers (IC 1) */
#define CRC_IDEAL_4_ADDR                      (0x40C3)

/* CRC Enable  - Registers (IC 1) */
#define CRC_ENABLE_ADDR                       (0x40C4)

/* GPIO 0 Control  - Registers (IC 1) */
#define GPIO_0_CONTROL_ADDR                   (0x40C6)

/* GPIO 1 Control  - Registers (IC 1) */
#define GPIO_1_CONTROL_ADDR                   (0x40C7)

/* GPIO 2 Control  - Registers (IC 1) */
#define GPIO_2_CONTROL_ADDR                   (0x40C8)

/* GPIO 3 Control  - Registers (IC 1) */
#define GPIO_3_CONTROL_ADDR                   (0x40C9)

/* Watchdog_Enable  - Registers (IC 1) */
#define WATCHDOG_ENABLE_ADDR                  (0x40D0)

/* Watchdog Register Value 1  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_1_ADDR        (0x40D1)

/* Watchdog Register Value 2  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_2_ADDR        (0x40D2)

/* Watchdog Register Value 3  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_3_ADDR        (0x40D3)

/* Watchdog Error  - Registers (IC 1) */
#define WATCHDOG_ERROR_ADDR                   (0x40D4)

/* Non Modulo RAM 1  - Registers (IC 1) */
#define NON_MODULO_RAM_1_ADDR                 (0x40E9)

/* Non Modulo RAM 2  - Registers (IC 1) */
#define NON_MODULO_RAM_2_ADDR                 (0x40EA)

/* Sample Rate Setting  - Registers (IC 1) */
#define SAMPLE_RATE_SETTING_ADDR              (0x40EB)

/* Routing Matrix Inputs  - Registers (IC 1) */
#define ROUTING_MATRIX_INPUTS_ADDR            (0x40F2)

/* Routing Matrix Outputs  - Registers (IC 1) */
#define ROUTING_MATRIX_OUTPUTS_ADDR           (0x40F3)

/* Serial Data/GPIO Pin Config  - Registers (IC 1) */
#define SERIAL_DATAGPIO_PIN_CONFIG_ADDR       (0x40F4)

/* DSP Enable Register  - Registers (IC 1) */
#define DSP_ENABLE_REGISTER_ADDR              (0x40F5)

/* DSP Run Register  - Registers (IC 1) */
#define DSP_RUN_REGISTER_ADDR                 (0x40F6)

/* DSP Slew Modes  - Registers (IC 1) */
#define DSP_SLEW_MODES_ADDR                   (0x40F7)

/* Serial Port Sample Rate Setting  - Registers (IC 1) */
#define SERIAL_PORT_SAMPLE_RATE_SETTING_ADDR  (0x40F8)

/* Clock Enable Reg 0  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_0_ADDR               (0x40F9)

/* Clock Enable Reg 1  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_1_ADDR               (0x40FA)

static void DSPDrv1701_Reset1(void *p);
static void DSPDrv1701_Reset2(void *p);
static void DSPDrv1701_Reset3(void *p);
static void DSPDrv1701_InitSection0(void *p);
static void DSPDrv1701_InitSection1(void *p);
static void DSPDrv1701_InitSection2(void *p);
static void DSPDrv1701_InitSection3(void *p);
static void DSPDrv1701_InitSection4(void *p);
static void DSPDrv1701_InitSection5(void *p);
static void DSPDrv1701_InitSection6(void *p);
static void DSPDrv1701_InitSection7(void *p);

static void DSPDrv1701_InitSection8(void *p);
static void DSPDrv1701_Delay(uint16 count);
static void DSPDrv1701_SetPower(bool state);
static void DSPDrv1701_I2cWrite(cDSPDrv1701 *me, uint16 bytes, const uint8 *data);
static void DSPDrv1701_I2cRead(cDSPDrv1701 *me, uint32 regAddr, uint16 bytes, const uint8 *data);
static void DSPDrv1701_GetCtrlAddr(tTunableInfo *p);








#endif
