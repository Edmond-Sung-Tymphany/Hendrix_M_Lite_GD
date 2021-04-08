/**
*  @file      dsp_TLV320AIC3254_driver_config.inc
*  @brief     This file stores all the configuration value for speicfic project.
*             different project had different configuration
*  @author    Edmond Sung
*  @date      01-2014
*  @copyright Tymphany Ltd.
*/


/* Page Selection Table */

/* Init commands */
/* CMD to reset chip */

/* CMD to set up power */

/* CMD to set other parts */
const tDspData dsp_init_cmd_sec2[/*DSP_INIT_CMD_SEC2_LEN*/] =
{
    {  0,0x00},
    { 60,0x00},        //            # reg[  0][ 60] = 0x00    ; DAC prog Mode: miniDSP_A and miniDSP_D NOT powered up together, miniDSP_A used for signal processing
    { 61,0x00},        //            # reg[  0][ 61] = 0x00    ; Use miniDSP_A for signal processing
    { 17,0x08},        //            # reg[  0][ 17] = 0x08    ; 8x Interpolation
    { 23,0x04},        //            # reg[  0][ 23] = 0x04    ; 4x Decimation
    { 15,0x03},
    { 16,0x88},
    { 21,0x03},
    { 22,0x88},
    {  0,0x08},            //            Select Page 8
    {  1,0x04},            //            # reg[  8][  1] = 0x04    ; adaptive mode for ADC
    {  0,0x2C},
    {  1,0x04},            //            # reg[ 44][  1] = 0x04    ; adaptive mode for DAC
    {  0,0x00},
/**************************************************************/
    /* PLL and clock source settings */
    {  5,0x92},//            # reg[  0][  5] = 0x91    ; P=1, R=2, J=32
    {  6,0x20},//            # reg[  0][  6] = 0x08    ; P=1, R=2, J=32
    {  7,0x00},//            # reg[  0][  7] = 0x00    ; D=0000 (MSB)
    {  8,0x00},//            # reg[  0][  8] = 0x00    ; D=0000 (LSB)
    {  4,0x07},//            # reg[  0][  4] = 0x03    ; PLL_clkin = BLCK, codec_clkin = PLL_CLK, PLL on
    { 12,0x88},//            # reg[  0][ 12] = 0x88    ; MDAC = 8, divider powered on
    { 13,0x00},//            # reg[  0][ 13] = 0x00    ; DOSR = 128 (MSB)
    { 14,0x80},//            # reg[  0][ 14] = 0x80    ; DOSR = 128 (LSB)
    { 18,0x82},//            # reg[  0][ 18] = 0x02    ; NADC = 2, divider powered on
    { 19,0x88},//            # reg[  0][ 19] = 0x88    ; MADC = 8, divider powered on
    { 20,0x80},//            # reg[  0][ 20] = 0x80    ; AOSR = 128
    { 11,0x82},//            # reg[  0][ 11] = 0x82    ; NDAC = 2, divider powered on
    {27, 0x00},//            # reg[  0][ 27] = 0x00    ; I2S, Data word length = 16bits, BCLK as input, WCLK as input
    {28, 0x00},//            # reg[  0][ 28] = 0x00    ; data offset = 0
    {29, 0x00},
    {30, 0xA0},//            # reg[  0][ 30] = 0xA0    ; set a right value N as a divider for the BCLK
    {31, 0x15},//            # reg[  0][ 31] = 0x15    ; !!TODO
    {33, 0x44},//            #                          !!TODO
/**************************************************************/
    {  0,0x01},//            # reg[  1][ 51] = 0x40    ; Mic Bias enabled, Source = Avdd, 1.25V
    { 51,0x40},
    { 52,0x40},//            # reg[  1][ 52] = 0x40    ; Route IN2L to LEFT_P with 10K input impedance; Route CM1L to LEFT_M with 10K input impedance; Route IN2R to RIGHT_P with 10K input impedance; Route IN1L to LEFT_P with 10K input impedance
    { 54,0x40},//            # reg[  1][ 54] = 0x40    ; Route CM1L to LEFT_M with 10K input impedance
    { 55,0x40},//            # reg[  1][ 55] = 0x40    ; Route IN1R to RIGHT_P with 10K input impedanc
    { 57,0x40},//            # reg[  1][ 57] = 0x40    ; Route CM1R to RIGHT_M with 10K input impedance
    { 59,0x00},//            # reg[  1][ 59] = 0x00    ; Enable MicPGA_L Gain Control, 0dB
    { 60,0x00},//            # reg[  1][ 60] = 0x00    ; Enable MicPGA_R Gain Control, 0dB
    {  0,0x00},
    { 81,0xC0},//            # reg[  0][ 81] = 0xc0    ; Power up LADC/RADC
    { 82,0x00},//            # reg[  0][ 82] = 0x00    ; Unmute LADC/RADC
    {  0,0x01},
    { 20,0x25},//            # reg[  1][ 20] = 0x25    ; De-pop: 5 time constants, 6k resistance
    { 12,0x08},//            # reg[  1][ 12] = 0x08    ; Route LDAC to HPL
    { 13,0x08},//            # reg[  1][ 13] = 0x08    ; Route RDAC to HPR
    { 14,0x08},//            # reg[  1][ 14] = 0x08    ; Route LDAC to LOL
    { 15,0x08},//            # reg[  1][ 15] = 0x08    ; Route LDAC to LOR
    {  0,0x00},
    { 63,0xD4},//            # reg[  0][ 63] = 0xd4    ; Power up LDAC/RDAC w/ soft stepping
    {  0,0x01},
    { 16,0x00},//            # reg[  1][ 16] = 0x00    ; Unmute HPL driver, 0dB Gain
    { 17,0x00},//            # reg[  1][ 17] = 0x00    ; Unmute HPR driver, 0dB Gain
    { 18,0x00},//            # reg[  1][ 18] = 0x00    ; Unmute LOL driver, 0dB Gain
    { 19,0x00},//            # reg[  1][ 19] = 0x00    ; Unmute LOR driver, 0dB Gain
    {  9,0x3C},//            # reg[  1][  9] = 0x3c    ; Power up HPL/HPR and LOL/LOR drivers
    {  0,0x00},
    { 64,0x00},//            # reg[  0][ 64] = 0x00    ; Unmute LDAC/RDAC
    { 56,0x04},//            # initialize MFP3 as general purpose input pin
    { 55,0x14},//            # initialize MFP4 as word clock output for secondary audio interface
    { 52,0x20},//            # initialize MFP5 as bit clock output for secondary audio interface
    { 82,0x00},//            # reg[0][82] = 0
    { 83,8},   //            # reg[0][83] = 0
    { 84,8},   //            # reg[0][84] = 0
    { 86,0x20},//            # reg[0][86] = 32
    { 87,0xFE},//            # reg[0][87] = 254
    { 88,0x00},//            # reg[0][88] = 0
    { 89,0x68},//            # reg[0][89] = 104
    { 90,0xA8},//            # reg[0][90] = 168
    { 91,0x06},//            # reg[0][91] = 6
    { 92,0x00},//            # reg[0][92] = 0
    { 94,0x20},//            # reg[0][94] = 32
    { 95,0xFE},//            # reg[0][95] = 254
    { 96,0x00},//            # reg[0][96] = 0
    { 97,0x68},//            # reg[0][97] = 104
    { 98,0xA8},//            # reg[0][98] = 168
    { 99,0x06},//            # reg[0][99] = 6
    {100,0x00},//            # reg[0][100] = 0


};

const uint8 TLV320AC_LINE_BT_MIX_52[]         ={52,0x50};
const uint8 TLV320AC_LINE_BT_MIX_54[]         ={54,0x50};
const uint8 TLV320AC_LINE_BT_MIX_55[]         ={55,0x44};
const uint8 TLV320AC_LINE_BT_MIX_57[]         ={57,0x44};

const uint8 TLV320AC_IN1R_52[]                     ={52,0x40};
const uint8 TLV320AC_IN1L_54[]                       ={54,0x40};
const uint8 TLV320AC_IN1R_55[]                       ={55,0x40};
const uint8 TLV320AC_IN1L_57[]                       ={57,0x40};

const uint8 TLV320AC_IN2R_52[]                     ={52,0x10};
const uint8 TLV320AC_IN2L_54[]                     ={54,0x10};
const uint8 TLV320AC_IN2R_55[]                     ={52,0x10};
const uint8 TLV320AC_IN2L_57[]                     ={54,0x10};

const uint8 TLV320AC_IN3R_52[]                     ={0x34,0x04};
const uint8 TLV320AC_IN3L_54[]                     ={0x36,0x40};
const uint8 TLV320AC_IN3R_55[]                     ={0x37,0x04};
const uint8 TLV320AC_IN3L_57[]                     ={0x39,0x40};

const uint8 TLV320AC_IN1L_IN2L_52[]             ={0x34,0x10};
const uint8 TLV320AC_IN1L_IN2L_54[]            ={0x36,0x10};
const uint8 TLV320AC_IN1R_IN2R_57[]            ={0x39,0x10};
const uint8 TLV320AC_IN1R_IN2R_55[]            ={0x37,0x40};



