/**
*  @file      dsp_TLV320AIC3254_driver_config.inc
*  @brief     This file stores all the configuration value for speicfic project.
*             different project had different configuration
*  @author    Edmond Sung
*  @date      01-2014
*  @copyright Tymphany Ltd.
*/


/* Page Selection Table */
typedef struct {
            uint8  control_page;           //coefficient page location
            uint8 control_base;           //coefficient base address within page
            uint8 control_mute_flag;      //non-zero means muting required
            uint8 control_string_index;   //string table index
} control; 
//static control MUX_controls[] = {
//};
static control VOLUME_controls[] = {
{8,52,0,0}
};

/* Init commands */
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
        //MCLOC 12.288MHz

    {  5,0x91},//            # reg[  0][  5] = 0x91    ; P=1, R=1, J=7
    {  6,0x07},//            # reg[  0][  6] = 0x07    ; P=1, R=1, J=7
    {  7,0x02},//            # reg[  0][  7] = 0x02    ; D=0000 (MSB)
    {  8,0x30},//            # reg[  0][  8] = 0x30    ; D=0000 (LSB)
    {  4,0x03},//            # reg[  0][  4] = 0x03    ; PLL_clkin = MCLK, codec_clkin = PLL_CLK, PLL on
    { 12,0x88},//            # reg[  0][ 12] = 0x88    ; MDAC = 8, divider powered on
    { 13,0x00},//            # reg[  0][ 13] = 0x00    ; DOSR = 120 (MSB)
    { 14,0x78},//            # reg[  0][ 14] = 0x78    ; DOSR = 120 (LSB)
    { 18,0x02},//            # reg[  0][ 18] = 0x02    ; NADC = 2, divider powered off
    { 19,0x88},//            # reg[  0][ 19] = 0x88    ; MADC = 8, divider powered on
    { 20,0x78},//            # reg[  0][ 20] = 0x78    ; AOSR = 120
    { 11,0x82},//            # reg[  0][ 11] = 0x82    ; NDAC = 2, divider powered on

    {27, 0x0c},
    {30, 0x90},
/**************************************************************/
    {  0,0x01},//            # reg[  1][ 51] = 0x40    ; Mic Bias enabled, Source = Avdd, 1.25V
    { 51,0x40},
    

/*
const uint8 TLV320AC_IN1L_IN2L_52[]             ={0x34,0x10};
const uint8 TLV320AC_IN1L_IN2L_54[]            ={0x36,0x10};
const uint8 TLV320AC_IN1R_IN2R_57[]            ={0x39,0x10};
const uint8 TLV320AC_IN1R_IN2R_55[]            ={0x37,0x40};

*/
/* set channel to BT*/
    { 52,0x10},//            
    { 54,0x10},//            
    { 55,0x40},
    { 57,0x10},//           
/* set channel to AuxIn*/
    //{ 52,0x04},//            # reg[  1][ 52] = 0x40    ; Route IN2L to LEFT_P with 10K input impedance; Route CM1L to LEFT_M with 10K input impedance; Route IN2R to RIGHT_P with 10K input impedance; Route IN1L to LEFT_P with 10K input impedance
    //{ 54,0x40},//            # reg[  1][ 54] = 0x40    ; Route CM1L to LEFT_M with 10K input impedance
    //{ 55,0x04},//            # reg[  1][ 55] = 0x40    ; Route IN1R to RIGHT_P with 10K input impedanc
    //{ 57,0x40},//            # reg[  1][ 57] = 0x40    ; Route CM1R to RIGHT_M with 10K input impedance

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
    {  9,0x3F},//            # reg[  1][  9] = 0x3c    ; Power up HPL/HPR and LOL/LOR drivers
    {  0,0x00},
    { 64,0x00},//            # reg[  0][ 64] = 0x00    ; Unmute LDAC/RDAC

    { 56,0x04},      // # initialize MFP3 as general purpose input pin

    { 82,0x00},//            # reg[0][82] = 0

    { 83,8},        //  ADC gain +xxdB//            # reg[0][83] = 0
    { 84,8},        //  ADC gain +xxdB//            # reg[0][84] = 0

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

const uint8 TLV320AC_IN3R_52[]                ={0x34,0x04};
const uint8 TLV320AC_IN3L_54[]                ={0x36,0x40};
const uint8 TLV320AC_IN3R_55[]                ={0x37,0x04};
const uint8 TLV320AC_IN3L_57[]                ={0x39,0x40};

const uint8 TLV320AC_IN1L_IN2L_52[]             ={0x34,0x10};
const uint8 TLV320AC_IN1L_IN2L_54[]            ={0x36,0x10};
const uint8 TLV320AC_IN1R_IN2R_57[]            ={0x39,0x10};
const uint8 TLV320AC_IN1R_IN2R_55[]            ={0x37,0x40};


#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL

#define GAIN_PAGE0  0x2C
#define GAIN_PAGE1  0x3E

static uint8 gain_coefs[4][4] = {
    {0x4C, 0x4C, 0xCC, 0xCD},// adaptor
    {0x4C, 0x41, 0x47, 0xAE},// full battery
    {0x4C, 0x3A, 0xE1, 0x48},// high battery
    {0x4C, 0x34, 0x7A, 0xE1} // low battery
};

#endif

/*******************************************/
/***********20 vol step table********************/
/*******************************************/
static const uint8 MASTER_VOL[MAX_VOLUME+1][3] =
{
    {0x40, 0x00, 0x00}, // step 19       0      dB   0x400000
    {0x39, 0x0A, 0x41}, // step 18      -1     dB   0x390A41
    {0x2F, 0xFE, 0x44}, // step 17      -2.5  dB   0x2FFE44
    {0x28, 0x61, 0x9A}, // step 16      -4     dB   0x28619A
    {0x21, 0xFA, 0x02}, // step 15      -5.5 dB    0x21FA02
    {0x1C, 0x96, 0x76}, // step 14      -7    dB    0x1C9676
    {0x16, 0xB5, 0x43}, // step 13      -9    dB    0x16B543
    {0x12, 0x09, 0xA3}, // step 12      -11  dB     0x1209A3
    {0x0E, 0x53, 0xEB}, // step 11      -13  dB     0x0E53EB
    {0x0B, 0x61, 0x88}, // step 10      -15  dB    0x0B6188
    {0x08, 0x0E, 0x9F}, // step 9       -18  dB     0x080E9F
    {0x05, 0x15, 0x6D}, // step 8         -22  dB    0x05156D
    {0x03, 0x35, 0x25}, // step 7         -26  dB    0x033525
    {0x02, 0x06, 0x1B}, // step 6        -30  dB    0x02061B
    {0x01, 0x23, 0x5A}, // step 5        -35  dB    0x01235A
    {0x00, 0xA3, 0xD7}, // step 4        -40  dB    0x00A3D7
    {0x00, 0x33, 0xCF}, // step 3        -50  dB    0x0033CF
    {0x00, 0x10, 0x62}, // step 2        -60  dB    0x001062
    {0x00, 0x02, 0xE9}, // step 1        -75  dB    0x0002E9
    {0x00, 0x00, 0x0D}, // step 0        -110 dB   0x00000D
};



#ifdef AUDIO_LIMITER_FOR_LOW_POWER

#define DRC_PAGE0   0x08    // 8
#define DRC_PAGE1   0x1A    // 26
#define DRC_PAGE2   0x2E    // 46
#define DRC_PAGE3   0x40    // 64

#ifdef DRC_SETT_ADAPTOR_MODE_3_38  // drc sett for adaptor mode from 3.38
const uint8 drc_coefs_adaptor_mode_p0[3] = {0x41, 0x32, 0x0D}; // reg 65 - 66 :page0

const uint8 drc_coefs_adaptor_mode_p1[3] = {0x41, 0x32, 0x0D}; // reg 65 - 66 :page1

const uint8 drc_coefs_adaptor_mode_p2_1[4] = {0x24, 0xF2, 0xC2, 0x35}; // reg 36 - 38 :page2
const uint8 drc_coefs_adaptor_mode_p2_2[2] = {0x30, 0x15};             // reg 48      :page2

const uint8 drc_coefs_adaptor_mode_p3_1[4] = {0x24, 0xF2, 0xC2, 0x35}; // reg 33 - 38 :page3
const uint8 drc_coefs_adaptor_mode_p3_2[2] = {0x30, 0x15};             // reg 48      :page3

#else // drc sett for adaptor mode from 3.42

const uint8 drc_coefs_adaptor_mode_p0[3] = {0x41, 0x2A, 0x37}; // reg 65 - 66 :page0

const uint8 drc_coefs_adaptor_mode_p1[3] = {0x41, 0x2A, 0x37}; // reg 65 - 66 :page1

const uint8 drc_coefs_adaptor_mode_p2_1[4] = {0x24, 0xF4, 0x5D, 0x17}; // reg 36 - 38 :page2
const uint8 drc_coefs_adaptor_mode_p2_2[2] = {0x30, 0x13};             // reg 48      :page2

const uint8 drc_coefs_adaptor_mode_p3_1[4] = {0x24, 0xF4, 0x5D, 0x17}; // reg 33 - 38 :page3
const uint8 drc_coefs_adaptor_mode_p3_2[2] = {0x30, 0x13};             // reg 48      :page3

#endif


const uint8 drc_coefs_batt_mode_p0[3] = {0x41, 0x28, 0x41}; // reg 65 - 66 :page0

const uint8 drc_coefs_batt_mode_p1[3] = {0x41, 0x28, 0x41}; // reg 65 - 66 :page1

const uint8 drc_coefs_batt_mode_p2_1[4] = {0x24, 0xF0, 0xA3, 0xD7}; // reg 36 - 38 :page2
const uint8 drc_coefs_batt_mode_p2_2[2] = {0x30, 0x17};             // reg 48      :page2

const uint8 drc_coefs_batt_mode_p3_1[4] = {0x24, 0xF0, 0xA3, 0xD7}; // reg 33 - 38 :page3
const uint8 drc_coefs_batt_mode_p3_2[2] = {0x30, 0x17};             // reg 48      :page3

#endif


