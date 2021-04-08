#ifndef _ADAU1451_CONFIG_
#define _ADAU1451_CONFIG_

#include "math.h"


/************************************************************************
**********  INCLUDE                                            **********
************************************************************************/
typedef const uint8 ADI_REG_TYPE; //must before include file
#include "ADAU1451_IC_1_PARAM.h"
#include "ADAU1451_IC_1.h"


/************************************************************************
**********  MACRO                                              **********
************************************************************************/
#define  DSP_RESET_PIN_HIGH() GpioDrv_SetBit(&gpioDsp,GPIO_OUT_DSP_RST_N) //** Active LOW **
#define  DSP_RESET_PIN_LOW()  GpioDrv_ClearBit(&gpioDsp,GPIO_OUT_DSP_RST_N)
#define  IS_DSP_TUNE()        GpioDrv_ReadBit(&gpioDsp,GPIO_IN_DSP_TUNE)


#define  BOOST_CAP_ENABLE(x)  GpioDrv_SetBit(&(x),GPIO_OUT_BOOST_CAP) //Active HIGH
#define  BOOST_CAP_DISABLE(x) GpioDrv_ClearBit(&(x),GPIO_OUT_BOOST_CAP)

extern cGpioDrv gpioDsp;


/************************************************************************
**********  PORTING LAYDER                                     **********
************************************************************************/
#define SIGMA_WRITE_REGISTER_BLOCK(dev_address, reg_addr, data_len, pdata) \
    DSPDrv1451_Writer_Register(me, dev_address, reg_addr, data_len, (uint8*)pdata);



/************************************************************************
********** Messages related to the "DSP Core Control Register" **********
************************************************************************/
/* Common */
#define ADAU1451_REGISTER_LEN       (4)
#define PROGRAM_RAM_PAGE_SIZE       (4)
#define PARA_RAM_PAGE_SIZE          (4)
#define DM1_PAGE_SIZE               (4)

/* DSP Version */
#define DSP_VER_REGISTER              (MOD_DC1_DCINPALG145X1VALUE_ADDR)
#define DSP_VER_REGISTER_LEN          (4)
#define DSP_VER_FLOAT_VALUE           (MOD_DC1_DCINPALG145X1VALUE_VALUE)

/* Volume Setting */
#define ADAU1451_UPDATE_VOLUME_LEN          (6)
#define ADAU1451_VOLUME_ADDR                MOD_VOLUMECONTROL_SETVOLUME_DCINPALG145X8VALUE_ADDR

/* Source selection 
 * the register for source selection is changed when the DSP layout is changed
 * so the complier switch is here
 */
#define SOURCE_SELECT_REGISTER_HIGH_BYTE    (MOD_SOURCESWITCH_NX2_SRCSWITCH_ALG0_STEREOMUXSIGMA3001VOL00_ADDR >> 8)
#define SOURCE_SELECT_REGISTER_LOW_BYTE     (MOD_SOURCESWITCH_NX2_SRCSWITCH_ALG0_STEREOMUXSIGMA3001VOL00_ADDR & 0xff)
#define SOURCE_SELECT_REGISTER_LEN          (10)


/* Source selection 
 * the register for source selection is changed when the DSP layout is changed
 * so the complier switch is here
 */
#define CHANNEL_SELECT_REGISTER_HIGH_BYTE    (MOD_SOURCESWITCH_NX2_SWITCH_CHANNEL_ALG0_STEREOMUXSIGMA3004VOL00_ADDR >> 8)
#define CHANNEL_SELECT_REGISTER_LOW_BYTE     (MOD_SOURCESWITCH_NX2_SWITCH_CHANNEL_ALG0_STEREOMUXSIGMA3004VOL00_ADDR & 0xff)
#define CHANNEL_SELECT_REGISTER_LEN          (14)


//Normal (Left + Right)
const uint8 ADAU1451_CHANNEL_NORMAL_SELECT[CHANNEL_SELECT_REGISTER_LEN]  = {CHANNEL_SELECT_REGISTER_HIGH_BYTE, CHANNEL_SELECT_REGISTER_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//Left Only (L to L/R)
const uint8 ADAU1451_CHANNEL_LEFT_SELECT[CHANNEL_SELECT_REGISTER_LEN]  = {CHANNEL_SELECT_REGISTER_HIGH_BYTE, CHANNEL_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//Right Only (R to L/R)
const uint8 ADAU1451_CHANNEL_RIGHT_SELECT[CHANNEL_SELECT_REGISTER_LEN]  = {CHANNEL_SELECT_REGISTER_HIGH_BYTE, CHANNEL_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00};

//I2S from ASE-TK
const uint8 ADAU1451_DIGITAL_INPUT_1_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//I2S from AUX-IN
const uint8 ADAU1451_DIGITAL_INPUT_2_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00};


//Volume output 
#define VOL_INPUT_REGISTER           MOD_SIGNALDETECTION_READBACK_SIG_INPUT_READBACKALGSIGMA30026_ADDR
#define VOL_INPUT2_REGISTER          MOD_SIGNALDETECTION_1XRTA1_ALG0_SINGLEBANDLEVELLITE3003_ADDR
#define VOL_AUXIN_INPUT_REGISTER     MOD_SIGNALDETECTION_AUXIN_READBACK_AUXINDET_DB_READBACKALGSIGMA30010_ADDR
#define VOL_AUXIN_INPUT2_REGISTER    MOD_SIGNALDETECTION_AUXIN_RTA1_AFTERFILTER_ALG0_SINGLEBANDLEVELLITE3001_ADDR


#define VOL_OUTPUT_WOOFER_REGISTER   MOD_TEST_AND_GAIN_FEATURES_READBACK_VOLOUT_WOOFER_READBACKALGSIGMA30021_ADDR
#define VOL_OUTPUT_MIDDLE_A_REGISTER MOD_TEST_AND_GAIN_FEATURES_READBACK_VOLOUT_MIDDLE_A_READBACKALGSIGMA30028_ADDR
#define VOL_OUTPUT_MIDDLE_B_REGISTER MOD_TEST_AND_GAIN_FEATURES_READBACK_VOLOUT_MIDDLE_B_READBACKALGSIGMA30030_ADDR
#define VOL_OUTPUT_TWEETER_REGISTER  MOD_TEST_AND_GAIN_FEATURES_READBACK_VOLOUT_TWEETER_READBACKALGSIGMA30032_ADDR
#define VOL_REGISTER_LEN  (4)





/* DSP Parameters  START ----------------------------------------------- */

/* Parameter: Bass */
#define ADAU1451_PARAM_BASS_ADDR    (MOD_VOLUMECONTROL_BASS_DCINPALG145X10VALUE_ADDR)
//#define ADAU1451_PARAM_BASS_HIGH_BYTE    (MOD_VOLUMECONTROL_BASS_DCINPALG145X11VALUE_ADDR >> 8)
//#define ADAU1451_PARAM_BASS_LOW_BYTE     (MOD_VOLUMECONTROL_BASS_DCINPALG145X11VALUE_ADDR & 0xff)



/* Parameter: Treble */
#define ADAU1451_PARAM_TREBLE_ADDR    (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X24VALUE_ADDR)
//#define ADAU1451_PARAM_TREBLE_HIGH_BYTE    (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X12VALUE_ADDR >> 8)
//#define ADAU1451_PARAM_TREBLE_LOW_BYTE     (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X12VALUE_ADDR & 0xff)



/* Parameter: Loudness */
#define ADAU1451_PARAM_LOUDNESS_HIGH_BYTE    (MOD_VOLUMECONTROL_LOUDNESS_ON_OFF_ALG0_MONOMUXS300SLEW1VOL00_ADDR >> 8)
#define ADAU1451_PARAM_LOUDNESS_LOW_BYTE     (MOD_VOLUMECONTROL_LOUDNESS_ON_OFF_ALG0_MONOMUXS300SLEW1VOL00_ADDR & 0xff)
#define ADAU1451_PARAM_LOUDNESS_REGISTER_LEN  10

//Loudness: ON
const uint8 ADAU1451_PARAM_LOUDNESS_ON_SELECT[ADAU1451_PARAM_LOUDNESS_REGISTER_LEN]  = {ADAU1451_PARAM_LOUDNESS_HIGH_BYTE, ADAU1451_PARAM_LOUDNESS_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//Loudness: OFF
const uint8 ADAU1451_PARAM_LOUDNESS_OFF_SELECT[ADAU1451_PARAM_LOUDNESS_REGISTER_LEN]  = {ADAU1451_PARAM_LOUDNESS_HIGH_BYTE, ADAU1451_PARAM_LOUDNESS_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00};



/* Parameter: Position */
#define ADAU1451_PARAM_POSITION_HIGH_BYTE    (MOD_POSSWITCH_POSSW_ALG0_STEREOMUXSIGMA3005VOL00_ADDR >> 8)
#define ADAU1451_PARAM_POSITION_LOW_BYTE     (MOD_POSSWITCH_POSSW_ALG0_STEREOMUXSIGMA3005VOL00_ADDR & 0xff)
#define ADAU1451_PARAM_POSITION_REGISTER_LEN  14

//Loudness: Free
const uint8 ADAU1451_PARAM_POSITION_FREE_SELECT[ADAU1451_PARAM_POSITION_REGISTER_LEN]  = {ADAU1451_PARAM_POSITION_HIGH_BYTE, ADAU1451_PARAM_POSITION_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//Loudness: Wall
const uint8 ADAU1451_PARAM_POSITION_WALL_SELECT[ADAU1451_PARAM_POSITION_REGISTER_LEN]  = {ADAU1451_PARAM_POSITION_HIGH_BYTE, ADAU1451_PARAM_POSITION_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//Loudness: Corner
const uint8 ADAU1451_PARAM_POSITION_CORNER_SELECT[ADAU1451_PARAM_POSITION_REGISTER_LEN]  = {ADAU1451_PARAM_POSITION_HIGH_BYTE, ADAU1451_PARAM_POSITION_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00};

/* DSP Parameters  END ----------------------------------------------- */



/*
* Line-in Mute
*/
#define LINE_IN_TO_ASETK_MUTE_REGISTER_HIGH_BYTE    (MOD_MUTE_AUXINTO_ASETK_MUTENOSLEWADAU145XALG1MUTE_ADDR >> 8)
#define LINE_IN_TO_ASETK_MUTE_REGISTER_LOW_BYTE     (MOD_MUTE_AUXINTO_ASETK_MUTENOSLEWADAU145XALG1MUTE_ADDR & 0xff)
#define LINE_IN_TO_ASETK_MUTE_REGISTER_LEN          (6)

const uint8 ADAU1451_LINE_IN_TO_ASETK_MUTE_ENABLE[LINE_IN_TO_ASETK_MUTE_REGISTER_LEN]  = {LINE_IN_TO_ASETK_MUTE_REGISTER_HIGH_BYTE, LINE_IN_TO_ASETK_MUTE_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_LINE_IN_TO_ASETK_MUTE_DISABLE[LINE_IN_TO_ASETK_MUTE_REGISTER_LEN] = {LINE_IN_TO_ASETK_MUTE_REGISTER_HIGH_BYTE, LINE_IN_TO_ASETK_MUTE_REGISTER_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00};

/* Calibration gain adjustment
 */
#define CAL_GAIN1_REGISTER         MOD_TEST_AND_GAIN_FEATURES_GAIN_WF_GAINALGNS145X8GAIN_ADDR       //woofer
#define CAL_GAIN2_REGISTER         MOD_TEST_AND_GAIN_FEATURES_GAIN_MIDDLE_A_GAINALGNS145X9GAIN_ADDR //middle-A
#define CAL_GAIN3_REGISTER         MOD_TEST_AND_GAIN_FEATURES_GAIN_MIDDLE_B_GAINALGNS145X10GAIN_ADDR //middle-B
#define CAL_GAIN4_REGISTER         MOD_TEST_AND_GAIN_FEATURES_GAIN_TWEETER_GAINALGNS145X11GAIN_ADDR  //twetter
#define CAL_GAIN_REGISTER_LEN     (6)


/* NTC tempearture update */
#define DC_REGISTER_LEN     (6)


/* Pass through enable */
#define PASS_THROUGH_REGISTER_HIGH_BYTE    (MOD_TEST_AND_GAIN_FEATURES_NX4_BYPASS_STEREOMUXSIGMA300NS41INDEX_ADDR >> 8)
#define PASS_THROUGH_REGISTER_LOW_BYTE     (MOD_TEST_AND_GAIN_FEATURES_NX4_BYPASS_STEREOMUXSIGMA300NS41INDEX_ADDR & 0xff)
#define PASS_THROUGH_REGISTER_LEN          (6)

const uint8 ADAU1451_PASS_THROUGH_ENABLE[PASS_THROUGH_REGISTER_LEN]  = {PASS_THROUGH_REGISTER_HIGH_BYTE, PASS_THROUGH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x06};
const uint8 ADAU1451_PASS_THROUGH_DISABLE[PASS_THROUGH_REGISTER_LEN] = {PASS_THROUGH_REGISTER_HIGH_BYTE, PASS_THROUGH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};


/* Mute Woofer */
#define MUTE_WOOFER_REGISTER_HIGH_BYTE    (MOD_TEST_AND_GAIN_FEATURES_MUTEWOOFER_MUTENOSLEWADAU145XALG9MUTE_ADDR >> 8)
#define MUTE_WOOFER_REGISTER_LOW_BYTE     (MOD_TEST_AND_GAIN_FEATURES_MUTEWOOFER_MUTENOSLEWADAU145XALG9MUTE_ADDR & 0xff)
#define MUTE_WOOFER_REGISTER_LEN          (6)

const uint8 ADAU1451_MUTE_WOOFER_ENABLE[MUTE_WOOFER_REGISTER_LEN]  = {MUTE_WOOFER_REGISTER_HIGH_BYTE, MUTE_WOOFER_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_MUTE_WOOFER_DISABLE[MUTE_WOOFER_REGISTER_LEN] = {MUTE_WOOFER_REGISTER_HIGH_BYTE, MUTE_WOOFER_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};

/* Mute MiddleA */
#define MUTE_MIDDLEA_REGISTER_HIGH_BYTE    (MOD_TEST_AND_GAIN_FEATURES_MUTEMIDDLEA_MUTENOSLEWADAU145XALG8MUTE_ADDR >> 8)
#define MUTE_MIDDLEA_REGISTER_LOW_BYTE     (MOD_TEST_AND_GAIN_FEATURES_MUTEMIDDLEA_MUTENOSLEWADAU145XALG8MUTE_ADDR & 0xff)
#define MUTE_MIDDLEA_REGISTER_LEN          (6)

const uint8 ADAU1451_MUTE_MIDDLEA_ENABLE[MUTE_MIDDLEA_REGISTER_LEN]  = {MUTE_MIDDLEA_REGISTER_HIGH_BYTE, MUTE_MIDDLEA_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_MUTE_MIDDLEA_DISABLE[MUTE_MIDDLEA_REGISTER_LEN] = {MUTE_MIDDLEA_REGISTER_HIGH_BYTE, MUTE_MIDDLEA_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};

/* Mute MiddleB */
#define MUTE_MIDDLEB_REGISTER_HIGH_BYTE    (MOD_TEST_AND_GAIN_FEATURES_MUTEMIDDLEB_MUTENOSLEWADAU145XALG6MUTE_ADDR >> 8)
#define MUTE_MIDDLEB_REGISTER_LOW_BYTE     (MOD_TEST_AND_GAIN_FEATURES_MUTEMIDDLEB_MUTENOSLEWADAU145XALG6MUTE_ADDR & 0xff)
#define MUTE_MIDDLEB_REGISTER_LEN          (6)

const uint8 ADAU1451_MUTE_MIDDLEB_ENABLE[MUTE_MIDDLEB_REGISTER_LEN]  = {MUTE_MIDDLEB_REGISTER_HIGH_BYTE, MUTE_MIDDLEB_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_MUTE_MIDDLEB_DISABLE[MUTE_MIDDLEB_REGISTER_LEN] = {MUTE_MIDDLEB_REGISTER_HIGH_BYTE, MUTE_MIDDLEB_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};

/* Mute Tweeter */
#define MUTE_TWEETER_REGISTER_HIGH_BYTE    (MOD_TEST_AND_GAIN_FEATURES_MUTETWEETER_MUTENOSLEWADAU145XALG7MUTE_ADDR >> 8)
#define MUTE_TWEETER_REGISTER_LOW_BYTE     (MOD_TEST_AND_GAIN_FEATURES_MUTETWEETER_MUTENOSLEWADAU145XALG7MUTE_ADDR & 0xff)
#define MUTE_TWEETER_REGISTER_LEN          (6)

const uint8 ADAU1451_MUTE_TWEETER_ENABLE[MUTE_TWEETER_REGISTER_LEN]  = {MUTE_TWEETER_REGISTER_HIGH_BYTE, MUTE_TWEETER_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_MUTE_TWEETER_DISABLE[MUTE_TWEETER_REGISTER_LEN] = {MUTE_TWEETER_REGISTER_HIGH_BYTE, MUTE_TWEETER_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};


/* DSP Version */
#define DSP_VER_REGISTER              (MOD_DC1_DCINPALG145X1VALUE_ADDR)
#define DSP_VER_REGISTER_LEN          (4)

/* ASRC */
#define DSP_ASRC_FROM_ASETK  DSP_ASRC0



/***********************************************************/
/****************** Global Variable ************************/
/***********************************************************/
static tDspInitSection DspInitSection[] =
{
    {DSPDrv1451_Reset1, 50},
    {DSPDrv1451_Reset2, 50},
    {DSPDrv1451_Reset3, 50},
    {&DSPDrv1451_InitI2c, 10},
    {&DSPDrv1451_InitSection1, 10},
    {&DSPDrv1451_InitSection2, 10},
    {&DSPDrv1451_InitSection3, 10},
    {&DSPDrv1451_InitSection4, 10},
    {&DSPDrv1451_InitSection5, 50},
    {&DSPDrv1451_InitSection6, 50},
    {&DSPDrv1451_InitSection7, 50},
    {&DSPDrv1451_InitSection8, 10},
    {&DSPDrv1451_InitSection9, 10},
    {&DSPDrv1451_InitSection10, 200},
    {&DSPDrv1451_InitSection11, 200},
    {&DSPDrv1451_InitSection12, 200}
};



/************************************************************************
 **********  INIT FUNCTION                                     **********
 ************************************************************************/
/* When export files from ADAU1451.dspproj, remember to porting 
 * initialize function from ADAU1451_IC_1.h
 */
static void DSPDrv1451_InitSection1(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection1\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R0_SOFT_RESET_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOFT_RESET_IC_1_ADDR, REG_SOFT_RESET_IC_1_BYTE, R1_SOFT_RESET_IC_1_Default );
    //SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R2_RESET_DELAY_IC_1_SIZE, R2_RESET_DELAY_IC_1_Default );

    TP_PRINTF("DSPDrv1451_InitSection1 finish\r\n");
}

static void DSPDrv1451_InitSection2(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection2\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R3_HIBERNATE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R4_HIBERNATE_IC_1_Default );
    //SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R5_HIBERNATE_DELAY_IC_1_SIZE, R5_HIBERNATE_DELAY_IC_1_Default );

    TP_PRINTF("DSPDrv1451_InitSection2 finish\r\n");
}

static void DSPDrv1451_InitSection3(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection3\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;    
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R6_KILL_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R7_KILL_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R8_PLL_ENABLE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLL_CTRL1_IC_1_ADDR, REG_PLL_CTRL1_IC_1_BYTE, R9_PLL_CTRL1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLL_CLK_SRC_IC_1_ADDR, REG_PLL_CLK_SRC_IC_1_BYTE, R10_PLL_CLK_SRC_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_MCLK_OUT_IC_1_ADDR, REG_MCLK_OUT_IC_1_BYTE, R11_MCLK_OUT_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PLL_ENABLE_IC_1_ADDR, REG_PLL_ENABLE_IC_1_BYTE, R12_PLL_ENABLE_IC_1_Default );
    //SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R13_PLL_LOCK_DELAY_IC_1_SIZE, R13_PLL_LOCK_DELAY_IC_1_Default );

    TP_PRINTF("DSPDrv1451_InitSection3 finish\r\n");
}

static void DSPDrv1451_InitSection4(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection4\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;    
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_POWER_ENABLE0_IC_1_ADDR, REG_POWER_ENABLE0_IC_1_BYTE, R14_POWER_ENABLE0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_POWER_ENABLE1_IC_1_ADDR, REG_POWER_ENABLE1_IC_1_BYTE, R15_POWER_ENABLE1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_MUTE_IC_1_ADDR, REG_ASRC_MUTE_IC_1_BYTE, R16_ASRC_MUTE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT1_IC_1_ADDR, REG_ASRC_INPUT1_IC_1_BYTE, R17_ASRC_INPUT1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT2_IC_1_ADDR, REG_ASRC_INPUT2_IC_1_BYTE, R18_ASRC_INPUT2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT3_IC_1_ADDR, REG_ASRC_INPUT3_IC_1_BYTE, R19_ASRC_INPUT3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE0_IC_1_ADDR, REG_ASRC_OUT_RATE0_IC_1_BYTE, R20_ASRC_OUT_RATE0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE1_IC_1_ADDR, REG_ASRC_OUT_RATE1_IC_1_BYTE, R21_ASRC_OUT_RATE1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE2_IC_1_ADDR, REG_ASRC_OUT_RATE2_IC_1_BYTE, R22_ASRC_OUT_RATE2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE3_IC_1_ADDR, REG_ASRC_OUT_RATE3_IC_1_BYTE, R23_ASRC_OUT_RATE3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE0_IC_1_ADDR, REG_SOUT_SOURCE0_IC_1_BYTE, R24_SOUT_SOURCE0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE1_IC_1_ADDR, REG_SOUT_SOURCE1_IC_1_BYTE, R25_SOUT_SOURCE1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE2_IC_1_ADDR, REG_SOUT_SOURCE2_IC_1_BYTE, R26_SOUT_SOURCE2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE3_IC_1_ADDR, REG_SOUT_SOURCE3_IC_1_BYTE, R27_SOUT_SOURCE3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE4_IC_1_ADDR, REG_SOUT_SOURCE4_IC_1_BYTE, R28_SOUT_SOURCE4_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE5_IC_1_ADDR, REG_SOUT_SOURCE5_IC_1_BYTE, R29_SOUT_SOURCE5_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE6_IC_1_ADDR, REG_SOUT_SOURCE6_IC_1_BYTE, R30_SOUT_SOURCE6_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE7_IC_1_ADDR, REG_SOUT_SOURCE7_IC_1_BYTE, R31_SOUT_SOURCE7_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE8_IC_1_ADDR, REG_SOUT_SOURCE8_IC_1_BYTE, R32_SOUT_SOURCE8_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE9_IC_1_ADDR, REG_SOUT_SOURCE9_IC_1_BYTE, R33_SOUT_SOURCE9_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE10_IC_1_ADDR, REG_SOUT_SOURCE10_IC_1_BYTE, R34_SOUT_SOURCE10_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE11_IC_1_ADDR, REG_SOUT_SOURCE11_IC_1_BYTE, R35_SOUT_SOURCE11_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE12_IC_1_ADDR, REG_SOUT_SOURCE12_IC_1_BYTE, R36_SOUT_SOURCE12_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE13_IC_1_ADDR, REG_SOUT_SOURCE13_IC_1_BYTE, R37_SOUT_SOURCE13_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE14_IC_1_ADDR, REG_SOUT_SOURCE14_IC_1_BYTE, R38_SOUT_SOURCE14_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE15_IC_1_ADDR, REG_SOUT_SOURCE15_IC_1_BYTE, R39_SOUT_SOURCE15_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE16_IC_1_ADDR, REG_SOUT_SOURCE16_IC_1_BYTE, R40_SOUT_SOURCE16_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE17_IC_1_ADDR, REG_SOUT_SOURCE17_IC_1_BYTE, R41_SOUT_SOURCE17_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE18_IC_1_ADDR, REG_SOUT_SOURCE18_IC_1_BYTE, R42_SOUT_SOURCE18_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE19_IC_1_ADDR, REG_SOUT_SOURCE19_IC_1_BYTE, R43_SOUT_SOURCE19_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE20_IC_1_ADDR, REG_SOUT_SOURCE20_IC_1_BYTE, R44_SOUT_SOURCE20_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE21_IC_1_ADDR, REG_SOUT_SOURCE21_IC_1_BYTE, R45_SOUT_SOURCE21_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE22_IC_1_ADDR, REG_SOUT_SOURCE22_IC_1_BYTE, R46_SOUT_SOURCE22_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE23_IC_1_ADDR, REG_SOUT_SOURCE23_IC_1_BYTE, R47_SOUT_SOURCE23_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_1_0_IC_1_ADDR, REG_SERIAL_BYTE_1_0_IC_1_BYTE, R48_SERIAL_BYTE_1_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_4_0_IC_1_ADDR, REG_SERIAL_BYTE_4_0_IC_1_BYTE, R49_SERIAL_BYTE_4_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_5_0_IC_1_ADDR, REG_SERIAL_BYTE_5_0_IC_1_BYTE, R50_SERIAL_BYTE_5_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_6_0_IC_1_ADDR, REG_SERIAL_BYTE_6_0_IC_1_BYTE, R51_SERIAL_BYTE_6_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_7_0_IC_1_ADDR, REG_SERIAL_BYTE_7_0_IC_1_BYTE, R52_SERIAL_BYTE_7_0_IC_1_Default );

    TP_PRINTF("DSPDrv1451_InitSection4 finish\r\n");
}

static void DSPDrv1451_InitSection5(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection5\r\n");

    //SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, PROGRAM_ADDR_IC_1, PROGRAM_SIZE_IC_1, Program_Data_IC_1 );
    cDSPDrv1451* me = (cDSPDrv1451*)p;
    uint8 buffer[PROGRAM_RAM_PAGE_SIZE+2]; 
    uint16 RegAddr;
    const uint8* bufferPtr;
    bufferPtr=Program_Data_IC_1;
    RegAddr = PROGRAM_ADDR_IC_1;
    for(RegAddr=PROGRAM_ADDR_IC_1;RegAddr<(PROGRAM_ADDR_IC_1 + (PROGRAM_SIZE_IC_1/PROGRAM_RAM_PAGE_SIZE));RegAddr++)
    {
        buffer[0]=(RegAddr>>8); 
        buffer[1]=(RegAddr&0xff);
        memcpy(&buffer[2], bufferPtr, PROGRAM_RAM_PAGE_SIZE);
        DSPDrv1451_I2cWrite(me, sizeof(buffer), (uint8*)buffer);
        bufferPtr+=PROGRAM_RAM_PAGE_SIZE;
    }
    
    TP_PRINTF("DSPDrv1451_InitSection5 finish\r\n");
}


static void DSPDrv1451_InitSection6(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection6\r\n");
    //SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, PARAM_ADDR_IC_1, PARAM_SIZE_IC_1, Param_Data_IC_1 );

    cDSPDrv1451* me = (cDSPDrv1451*)p;

    uint8 buffer[PARA_RAM_PAGE_SIZE+2]; 
    uint16 RegAddr;
    const uint8* bufferPtr;
    bufferPtr=Param_Data_IC_1;
    RegAddr = PARAM_ADDR_IC_1;
    for(RegAddr=PARAM_ADDR_IC_1;RegAddr<(PARAM_ADDR_IC_1 + (PARAM_SIZE_IC_1/PARA_RAM_PAGE_SIZE));RegAddr++)
    {
        buffer[0]=(RegAddr>>8); 
        buffer[1]=(RegAddr&0xff);
        memcpy(&buffer[2], bufferPtr, PARA_RAM_PAGE_SIZE);
        DSPDrv1451_I2cWrite(me, sizeof(buffer), (uint8*)buffer);
        bufferPtr+=PARA_RAM_PAGE_SIZE;
    }
    
    TP_PRINTF("DSPDrv1451_InitSection6 finish\r\n");
}


static void DSPDrv1451_InitSection7(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection7\r\n");
    //SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, DM1_DATA_ADDR_IC_1, DM1_DATA_SIZE_IC_1, DM1_DATA_Data_IC_1 );
    cDSPDrv1451* me = (cDSPDrv1451*)p;

    uint8 buffer[DM1_PAGE_SIZE+2]; 
    uint16 RegAddr;
    const uint8* bufferPtr;
    bufferPtr=DM1_DATA_Data_IC_1;
    RegAddr = DM1_DATA_ADDR_IC_1;
    for(RegAddr=DM1_DATA_ADDR_IC_1;RegAddr<(DM1_DATA_ADDR_IC_1 + (DM1_DATA_SIZE_IC_1/DM1_PAGE_SIZE));RegAddr++)
    {
        buffer[0]=(RegAddr>>8); 
        buffer[1]=(RegAddr&0xff);
        memcpy(&buffer[2], bufferPtr, PARA_RAM_PAGE_SIZE);
        DSPDrv1451_I2cWrite(me, sizeof(buffer), (uint8*)buffer);
        bufferPtr+=DM1_PAGE_SIZE;
    }

    TP_PRINTF("DSPDrv1451_InitSection7 finish\r\n");
}

static void DSPDrv1451_InitSection8(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection8\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R56_KILL_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_ADDRESS_IC_1_ADDR, REG_START_ADDRESS_IC_1_BYTE, R57_START_ADDRESS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_PULSE_IC_1_ADDR, REG_START_PULSE_IC_1_BYTE, R58_START_PULSE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R59_START_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R60_START_CORE_IC_1_Default );
    //SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R56_START_DELAY_IC_1_SIZE, R56_START_DELAY_IC_1_Default );
                               
    TP_PRINTF("DSPDrv1451_InitSection8 finish\r\n");
}

static void DSPDrv1451_InitSection9(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection9\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R62_HIBERNATE_IC_1_Default );
    TP_PRINTF("DSPDrv1451_InitSection9 finish\r\n");
}

static void DSPDrv1451_InitSection10(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection10\r\n");
    BOOST_CAP_ENABLE(gpioDsp);
}


static void DSPDrv1451_InitSection11(void *p)
{
}


static void DSPDrv1451_InitSection12(void *p)
{
}

static void DSPDrv1451_Xtor_Cust(cDSPDrv1451* me)
{
    BOOST_CAP_DISABLE(gpioDsp);
}





/************************************************************************
 **********  CUST FUNCTION                                     **********
 ************************************************************************/






/**
 * Sets the DSP volume, by writing the L/R channels PGA Gain registers. Please
 * reference on the volume table to check the real gain.
 *
 * @param      fGainDb        The gain on linear format
 * @return     void
 */
#ifdef HAS_DSP_CALIBRATION_GAIN
void DSPDrv1451_SetCalibrateGain(cDSPDrv1451 *me, eAudioSettId audioSettId, float fGainDb)
{
    float fGainLinear= pow(10.0, fGainDb/20.0);
    uint32 data_gain= DSPDrv1451_FloatTo8_24Data(me, fGainLinear);
    TP_PRINTF("DSPDrv1451_SetCalibrateGain: audioSettId=%d, gain:%.3fdB (linear:%.3f)\r\n", audioSettId, fGainDb, fGainLinear);
    
    uint8 data_iic[CAL_GAIN_REGISTER_LEN] = {0}; /* write DSP register */
    uint32 reg_addr= 0;
    
    switch(audioSettId)
    {
    case DSP_CAL_GAIN1_ID:
        reg_addr= CAL_GAIN1_REGISTER;
        break;
    case DSP_CAL_GAIN2_ID:
        reg_addr= CAL_GAIN2_REGISTER;
        break;
         
    case DSP_CAL_GAIN3_ID:
        reg_addr= CAL_GAIN3_REGISTER;
        break;

    case DSP_CAL_GAIN4_ID:
        reg_addr= CAL_GAIN4_REGISTER;
        break;
        
    default:
        ASSERT(0);
        return;
    }
    
    data_iic[0] = reg_addr >> 8;
    data_iic[1] = reg_addr & 0x00FF;
    data_iic[2] = UINT32_GET_BIT_RANGE(data_gain, 31, 24);
    data_iic[3] = UINT32_GET_BIT_RANGE(data_gain, 23, 16);
    data_iic[4] = UINT32_GET_BIT_RANGE(data_gain, 15,  8);
    data_iic[5] = UINT32_GET_BIT_RANGE(data_gain,  7,  0);

    DSPDrv1451_I2cWrite(me, ADAU1451_UPDATE_VOLUME_LEN, (uint8*)&data_iic);
}
#endif /* HAS_DSP_CALIBRATION_GAIN */



/**
 * Write Treble value, 0 ~ 20
 */
void DSPDrv1451_SetTreble(cDSPDrv1451 *me, uint32 value)
{
    TP_PRINTF("DSPDrv1451_SetTreble: value=%d \r\n", value);
    ASSERT(value>=0 && value<=20);
    DSPDrv1451_WriteDcValue32_0(me, ADAU1451_PARAM_TREBLE_ADDR, value);
}


/**
 * Write Bass value, range: 0 ~ 20
 */
void DSPDrv1451_SetBase(cDSPDrv1451 *me, uint32 value)
{
    TP_PRINTF("DSPDrv1451_SetBase: value=%d \r\n", value);
    ASSERT(value>=0 && value<=20);
    DSPDrv1451_WriteDcValue32_0(me, ADAU1451_PARAM_BASS_ADDR, value);
}


/**
 * Write Loudness value
 */
void DSPDrv1451_SetLoudness(cDSPDrv1451 *me, bool enable)
{
    TP_PRINTF("DSPDrv1451_SetLoudness: enable=%d \r\n", enable);
    if(enable)
    {
        DSPDrv1451_I2cWrite(me, ADAU1451_PARAM_LOUDNESS_REGISTER_LEN, ADAU1451_PARAM_LOUDNESS_ON_SELECT);
    }
    else
    {
        DSPDrv1451_I2cWrite(me, ADAU1451_PARAM_LOUDNESS_REGISTER_LEN, ADAU1451_PARAM_LOUDNESS_OFF_SELECT);
    }    
}


void DSPDrv1451_SetChannel_L_R(cDSPDrv1451 *me, tDspChannel channel)
{
    TP_PRINTF("\r\n\r\n\r\nDSPDrv1451_SetChannel_L_R: channel=%d \r\n\r\n\r\n", channel);
    
    const uint8 * ctrData = NULL;
    switch (channel)
    {
        case DSP_CH_NORMAL:
        {
           ctrData = ADAU1451_CHANNEL_NORMAL_SELECT;
           break;
       }
       case DSP_CH_LEFT_ONLY:
       {
           ctrData = ADAU1451_CHANNEL_LEFT_SELECT;
           break;
       }
       case DSP_CH_RIGHT_ONLY:
       {
           ctrData = ADAU1451_CHANNEL_RIGHT_SELECT;
           break;
       }
       default:
       {
           ASSERT(0);
           return;
       }
    }

    DSPDrv1451_I2cWrite(me, CHANNEL_SELECT_REGISTER_LEN, (uint8*)ctrData);    
}



#endif /*_ADAU1451_CONFIG_*/
