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
#define  DSP_RESET_PIN_HIGH()    GpioDrv_SetBit(&gpioDsp,GPIO_OUT_DSP_RST_N) //** Active LOW **
#define  DSP_RESET_PIN_LOW()     GpioDrv_ClearBit(&gpioDsp,GPIO_OUT_DSP_RST_N)
#define  IS_DSP_TUNE()           GpioDrv_ReadBit(&gpioDsp,GPIO_IN_DSP_TUNE)

#define  BOOST_ENABLE(x)         GpioDrv_SetBit(&(x),GPIO_OUT_BOOST_EN) //Active HIGH
#define  BOOST_DISABLE(x)        GpioDrv_ClearBit(&(x),GPIO_OUT_BOOST_EN)

#define  BOOST_CAP1_ENABLE(x)    GpioDrv_SetBit(&(x),GPIO_OUT_BOOST_CAP1) //Active HIGH
#define  BOOST_CAP1_DISABLE(x)   GpioDrv_ClearBit(&(x),GPIO_OUT_BOOST_CAP1)

#define  BOOST_CAP2_ENABLE(x)    GpioDrv_SetBit(&(x),GPIO_OUT_BOOST_CAP2) //Active HIGH
#define  BOOST_CAP2_DISABLE(x)   GpioDrv_ClearBit(&(x),GPIO_OUT_BOOST_CAP2)

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

/* Signal Detection */
#define ADAU1451_SIGNAL_DETECTION_LEN       (4)
#define ADAU1451_SIGNAL_DETECTION_ADDR      MOD_SIGNALDETECTION_READBACK_SIGDET_READBACKALGSIGMA3004_ADDR
    
/* Fix-Me: change correct address according to DSP flow. */
#define ADAU1451_SPDIF_IN_SIGNAL_DETECTION_ADDR      MOD_SIGNALDETECTION_READBACK_SIGDET_READBACKALGSIGMA3004_ADDR

/* Volume Setting */
#define ADAU1451_UPDATE_VOLUME_LEN          (6)
#define ADAU1451_VOLUME_ADDR                MOD_VOLUMECONTROL_DC1_2_DCINPALG145X2VALUE_ADDR

/* Source selection 
 * the register for source selection is changed when the DSP layout is changed
 * so the complier switch is here
 */
#define SOURCE_SELECT_REGISTER_HIGH_BYTE    (MOD_SOURCESWITCH_NX2_1_ALG0_STEREOMUXSIGMA3001VOL00_ADDR >> 8)
#define SOURCE_SELECT_REGISTER_LOW_BYTE     (MOD_SOURCESWITCH_NX2_1_ALG0_STEREOMUXSIGMA3001VOL00_ADDR & 0xff)
#define SOURCE_SELECT_REGISTER_LEN          (10)

//I2S from SPDIF
const uint8 ADAU1451_DIGITAL_INPUT_SPDIF_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x01, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};

//I2S from ASE-TK
const uint8 ADAU1451_DIGITAL_INPUT_1_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x01, 0x00, 0x00, 0x00};



//Volume output
#define VOL_INPUT_REGISTER    MOD_SIGNALDETECTION_VOLINPUT_ALG0_SINGLEBANDLEVELLITE3001_ADDR
#define VOL_OUTPUT_WOOFER_REGISTER   MOD_LEVELMETERWOOFER_ALG0_SINGLEBANDLEVELLITE3002_ADDR
#define VOL_OUTPUT_TWEETER_REGISTER  MOD_LEVELMETERTWEETER_ALG0_SINGLEBANDLEVELLITE3003_ADDR
#define VOL_REGISTER_LEN  (4)






/* DSP Parameters  START ----------------------------------------------- */

/* Parameter: Bass */
#define ADAU1451_PARAM_BASS_ADDR    (MOD_VOLUMECONTROL_BASS_DCINPALG145X11VALUE_ADDR)
//#define ADAU1451_PARAM_BASS_HIGH_BYTE    (MOD_VOLUMECONTROL_BASS_DCINPALG145X11VALUE_ADDR >> 8)
//#define ADAU1451_PARAM_BASS_LOW_BYTE     (MOD_VOLUMECONTROL_BASS_DCINPALG145X11VALUE_ADDR & 0xff)



/* Parameter: Treble */
#define ADAU1451_PARAM_TREBLE_ADDR    (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X12VALUE_ADDR)
//#define ADAU1451_PARAM_TREBLE_HIGH_BYTE    (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X12VALUE_ADDR >> 8)
//#define ADAU1451_PARAM_TREBLE_LOW_BYTE     (MOD_VOLUMECONTROL_TREBLE_DCINPALG145X12VALUE_ADDR & 0xff)



/* Parameter: Loudness */
#define ADAU1451_PARAM_LOUDNESS_HIGH_BYTE    (MOD_VOLUMECONTROL_LOUDNESS_ON_OFF_ALG0_MONOMUXS300SLEW2VOL00_ADDR >> 8)
#define ADAU1451_PARAM_LOUDNESS_LOW_BYTE     (MOD_VOLUMECONTROL_LOUDNESS_ON_OFF_ALG0_MONOMUXS300SLEW2VOL00_ADDR & 0xff)
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
#define ADAU1451_PARAM_POSITION_HIGH_BYTE    (MOD_POSSWITCH_POSSW_ALG0_MONOMUXS300SLEW1VOL00_ADDR >> 8)
#define ADAU1451_PARAM_POSITION_LOW_BYTE     (MOD_POSSWITCH_POSSW_ALG0_MONOMUXS300SLEW1VOL00_ADDR & 0xff)
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




 
/* Dynamic Boost Level Readback */
#define ADAU1451_DBOOST_LEVEL_LEN       (4)
#define ADAU1451_DBOOST_LEVEL_REGISTER  MOD__DYNABOOST_READBACK_DYNAMIC_BOOST_LEVEL_READBACKALGSIGMA3003_ADDR
    

//Invalid selection
#define ADAU1451_DIGITAL_INPUT_2_SELECT ADAU1451_DIGITAL_INPUT_1_SELECT


/* Calibration gain adjustment
 */
#define CAL_GAIN1_REGISTER         MOD_CAL_GAIN_GAIN_WOOFER_GAINALGNS145X2GAIN_ADDR //WOOFER
#define CAL_GAIN2_REGISTER         MOD_CAL_GAIN_GAIN_TWETTER_GAINALGNS145X3GAIN_ADDR //TWETTER
#define CAL_GAIN3_REGISTER         MOD_CAL_GAIN_GAIN_TWETTER_GAINALGNS145X3GAIN_ADDR //fake definition, TWETTER
#define CAL_GAIN_REGISTER_LEN     (6)


/* NTC tempearture update */
#define DC_REGISTER_LEN     (6)

/* Power mode selection for dynamic boost
 *
 * The previuos way for dynamic boost, is use Nx-DcStatus to switch between battery and DC. But it cause problem:
 * when remove DC and output volume is high, boost level raise from min to max directly. To resolve, we change "mute1"
 * checkbox for DC/battery mode.
 *
 * New Method:
 * - Battery mode: allow input for boost decision, thus boost level will dynamic adjust
 * - DC mode:  mute input for boost decision
 * - Insert DC (battery to DC mode): boost level decrease from max to min step by step
 * - Remove DC (DC to battery mode): boost level increase from min to max step by step
 */
#define DYNA_BOOST_DC_STATUS_REGISTER_HIGH_BYTE    (MOD_BOOST_NX_DCSTATUS_MONOMUXSIGMA300NS1INDEX_ADDR >> 8)
#define DYNA_BOOST_DC_STATUS_REGISTER_LOW_BYTE     (MOD_BOOST_NX_DCSTATUS_MONOMUXSIGMA300NS1INDEX_ADDR & 0xff)
#define DYNA_BOOST_DC_STATUS_REGISTER_LEN   (6)

const uint8 ADAU1451_DYNA_BOOST_DC_MODE_SELECT[DYNA_BOOST_DC_STATUS_REGISTER_LEN]  = {DYNA_BOOST_DC_STATUS_REGISTER_HIGH_BYTE, DYNA_BOOST_DC_STATUS_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};

const uint8 ADAU1451_DYNA_BOOST_BATT_MODE_SELECT[DYNA_BOOST_DC_STATUS_REGISTER_LEN]  = {DYNA_BOOST_DC_STATUS_REGISTER_HIGH_BYTE, DYNA_BOOST_DC_STATUS_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};

/* Old method for dynamic boost (battery/dc mode)
 * We should always choose battery mode to enable Dynamic Boost
 */
//#define DYNA_BOOST_OLD_DC_STATUS_REGISTER_HIGH_BYTE    (MOD__DYNABOOST_NX_DCSTATUS_MONOMUXSIGMA300NS2INDEX_ADDR >> 8)
//#define DYNA_BOOST_OLD_DC_STATUS_REGISTER_LOW_BYTE     (MOD__DYNABOOST_NX_DCSTATUS_MONOMUXSIGMA300NS2INDEX_ADDR & 0xff)
//const uint8 ADAU1451_DYNA_BOOST_OLD_DC_MODE_SELECT[DYNA_BOOST_DC_STATUS_REGISTER_LEN]  = {DYNA_BOOST_OLD_DC_STATUS_REGISTER_HIGH_BYTE, DYNA_BOOST_OLD_DC_STATUS_REGISTER_LOW_BYTE,
//                                                  0x00, 0x00, 0x00, 0x01};
//const uint8 ADAU1451_DYNA_BOOST_OLD_BATT_MODE_SELECT[DYNA_BOOST_DC_STATUS_REGISTER_LEN]  = {DYNA_BOOST_OLD_DC_STATUS_REGISTER_HIGH_BYTE, DYNA_BOOST_OLD_DC_STATUS_REGISTER_LOW_BYTE,
//                                                  0x00, 0x00, 0x00, 0x00};


/* Power mode selection for dynamic boost
 */
//#define GAIN_DC_STATUS_REGISTER_HIGH_BYTE    (MOD_SE_TUNING_BATTERYPROTECTCOMPRESSOR_MONOMUXSIGMA300NS5INDEX_ADDR >> 8)
//#define GAIN_DC_STATUS_REGISTER_LOW_BYTE     (MOD_SE_TUNING_BATTERYPROTECTCOMPRESSOR_MONOMUXSIGMA300NS5INDEX_ADDR & 0xff)
//#define GAIN_DC_STATUS_REGISTER_LEN   (6)
//
//const uint8 ADAU1451_GAIN_DC_MODE_SELECT[GAIN_DC_STATUS_REGISTER_LEN]  = {GAIN_DC_STATUS_REGISTER_HIGH_BYTE, GAIN_DC_STATUS_REGISTER_LOW_BYTE,
//                                                  0x00, 0x00, 0x00, 0x00};
//
//const uint8 ADAU1451_GAIN_BATT_MODE_SELECT[GAIN_DC_STATUS_REGISTER_LEN]  = {GAIN_DC_STATUS_REGISTER_HIGH_BYTE, GAIN_DC_STATUS_REGISTER_LOW_BYTE,
//                                                  0x00, 0x00, 0x00, 0x01};


/* Pass through enable */
#define PASS_THROUGH_REGISTER_HIGH_BYTE    (MOD_BYPASS_NX2_3_STEREOMUXSIGMA300NS1INDEX_ADDR >> 8)
#define PASS_THROUGH_REGISTER_LOW_BYTE     (MOD_BYPASS_NX2_3_STEREOMUXSIGMA300NS1INDEX_ADDR & 0xff)
#define PASS_THROUGH_REGISTER_LEN          (6)

const uint8 ADAU1451_PASS_THROUGH_ENABLE[PASS_THROUGH_REGISTER_LEN]  = {PASS_THROUGH_REGISTER_HIGH_BYTE, PASS_THROUGH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x02};
const uint8 ADAU1451_PASS_THROUGH_DISABLE[PASS_THROUGH_REGISTER_LEN] = {PASS_THROUGH_REGISTER_HIGH_BYTE, PASS_THROUGH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};


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
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_PANIC_PARITY_MASK_IC_1_ADDR, REG_PANIC_PARITY_MASK_IC_1_BYTE, R17_PANIC_PARITY_MASK_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_MP8_MODE_IC_1_ADDR, REG_MP8_MODE_IC_1_BYTE, R18_MP8_MODE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_MP8_WRITE_IC_1_ADDR, REG_MP8_WRITE_IC_1_BYTE, R19_MP8_WRITE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SDATA_OUT3_PIN_IC_1_ADDR, REG_SDATA_OUT3_PIN_IC_1_BYTE, R20_SDATA_OUT3_PIN_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT0_IC_1_ADDR, REG_ASRC_INPUT0_IC_1_BYTE, R21_ASRC_INPUT0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT1_IC_1_ADDR, REG_ASRC_INPUT1_IC_1_BYTE, R22_ASRC_INPUT1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT2_IC_1_ADDR, REG_ASRC_INPUT2_IC_1_BYTE, R23_ASRC_INPUT2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_INPUT3_IC_1_ADDR, REG_ASRC_INPUT3_IC_1_BYTE, R24_ASRC_INPUT3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE0_IC_1_ADDR, REG_ASRC_OUT_RATE0_IC_1_BYTE, R25_ASRC_OUT_RATE0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE1_IC_1_ADDR, REG_ASRC_OUT_RATE1_IC_1_BYTE, R26_ASRC_OUT_RATE1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE2_IC_1_ADDR, REG_ASRC_OUT_RATE2_IC_1_BYTE, R27_ASRC_OUT_RATE2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_ASRC_OUT_RATE3_IC_1_ADDR, REG_ASRC_OUT_RATE3_IC_1_BYTE, R28_ASRC_OUT_RATE3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE0_IC_1_ADDR, REG_SOUT_SOURCE0_IC_1_BYTE, R29_SOUT_SOURCE0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE1_IC_1_ADDR, REG_SOUT_SOURCE1_IC_1_BYTE, R30_SOUT_SOURCE1_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE2_IC_1_ADDR, REG_SOUT_SOURCE2_IC_1_BYTE, R31_SOUT_SOURCE2_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE3_IC_1_ADDR, REG_SOUT_SOURCE3_IC_1_BYTE, R32_SOUT_SOURCE3_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE4_IC_1_ADDR, REG_SOUT_SOURCE4_IC_1_BYTE, R33_SOUT_SOURCE4_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE5_IC_1_ADDR, REG_SOUT_SOURCE5_IC_1_BYTE, R34_SOUT_SOURCE5_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE6_IC_1_ADDR, REG_SOUT_SOURCE6_IC_1_BYTE, R35_SOUT_SOURCE6_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE7_IC_1_ADDR, REG_SOUT_SOURCE7_IC_1_BYTE, R36_SOUT_SOURCE7_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE8_IC_1_ADDR, REG_SOUT_SOURCE8_IC_1_BYTE, R37_SOUT_SOURCE8_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE9_IC_1_ADDR, REG_SOUT_SOURCE9_IC_1_BYTE, R38_SOUT_SOURCE9_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE10_IC_1_ADDR, REG_SOUT_SOURCE10_IC_1_BYTE, R39_SOUT_SOURCE10_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE11_IC_1_ADDR, REG_SOUT_SOURCE11_IC_1_BYTE, R40_SOUT_SOURCE11_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE12_IC_1_ADDR, REG_SOUT_SOURCE12_IC_1_BYTE, R41_SOUT_SOURCE12_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE13_IC_1_ADDR, REG_SOUT_SOURCE13_IC_1_BYTE, R42_SOUT_SOURCE13_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE14_IC_1_ADDR, REG_SOUT_SOURCE14_IC_1_BYTE, R43_SOUT_SOURCE14_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE15_IC_1_ADDR, REG_SOUT_SOURCE15_IC_1_BYTE, R44_SOUT_SOURCE15_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE16_IC_1_ADDR, REG_SOUT_SOURCE16_IC_1_BYTE, R45_SOUT_SOURCE16_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE17_IC_1_ADDR, REG_SOUT_SOURCE17_IC_1_BYTE, R46_SOUT_SOURCE17_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE18_IC_1_ADDR, REG_SOUT_SOURCE18_IC_1_BYTE, R47_SOUT_SOURCE18_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE19_IC_1_ADDR, REG_SOUT_SOURCE19_IC_1_BYTE, R48_SOUT_SOURCE19_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE20_IC_1_ADDR, REG_SOUT_SOURCE20_IC_1_BYTE, R49_SOUT_SOURCE20_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE21_IC_1_ADDR, REG_SOUT_SOURCE21_IC_1_BYTE, R50_SOUT_SOURCE21_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE22_IC_1_ADDR, REG_SOUT_SOURCE22_IC_1_BYTE, R51_SOUT_SOURCE22_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SOUT_SOURCE23_IC_1_ADDR, REG_SOUT_SOURCE23_IC_1_BYTE, R52_SOUT_SOURCE23_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_4_0_IC_1_ADDR, REG_SERIAL_BYTE_4_0_IC_1_BYTE, R53_SERIAL_BYTE_4_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_5_0_IC_1_ADDR, REG_SERIAL_BYTE_5_0_IC_1_BYTE, R54_SERIAL_BYTE_5_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_6_0_IC_1_ADDR, REG_SERIAL_BYTE_6_0_IC_1_BYTE, R55_SERIAL_BYTE_6_0_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_SERIAL_BYTE_7_0_IC_1_ADDR, REG_SERIAL_BYTE_7_0_IC_1_BYTE, R56_SERIAL_BYTE_7_0_IC_1_Default );

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
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_KILL_CORE_IC_1_ADDR, REG_KILL_CORE_IC_1_BYTE, R61_KILL_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_ADDRESS_IC_1_ADDR, REG_START_ADDRESS_IC_1_BYTE, R62_START_ADDRESS_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_PULSE_IC_1_ADDR, REG_START_PULSE_IC_1_BYTE, R63_START_PULSE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R64_START_CORE_IC_1_Default );
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_START_CORE_IC_1_ADDR, REG_START_CORE_IC_1_BYTE, R65_START_CORE_IC_1_Default );
	SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R66_START_DELAY_IC_1_SIZE, R66_START_DELAY_IC_1_Default );
    //SIGMA_WRITE_DELAY( DEVICE_ADDR_IC_1, R58_START_DELAY_IC_1_SIZE, R58_START_DELAY_IC_1_Default );
                               
    TP_PRINTF("DSPDrv1451_InitSection8 finish\r\n");
}

static void DSPDrv1451_InitSection9(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection9\r\n");

    cDSPDrv1451* me = (cDSPDrv1451*)p;
	SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_HIBERNATE_IC_1_ADDR, REG_HIBERNATE_IC_1_BYTE, R67_HIBERNATE_IC_1_Default );

    TP_PRINTF("DSPDrv1451_InitSection9 finish\r\n");
}


static void DSPDrv1451_InitSection10(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection10\r\n");
    BOOST_CAP1_ENABLE(gpioDsp);
    TP_PRINTF("DSPDrv1451_InitSection10 finish\r\n");
}


static void DSPDrv1451_InitSection11(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection11\r\n");
    BOOST_CAP2_ENABLE(gpioDsp);
    TP_PRINTF("DSPDrv1451_InitSection11 finish\r\n");
}


static void DSPDrv1451_InitSection12(void *p)
{
    TP_PRINTF("DSPDrv1451_InitSection12\r\n");

    /* Note we must enable BOOST_CAP1/2 then BOOST_ENABLE,
     * or main power will drop on battery mode (No AC power)
     */
    BOOST_ENABLE(gpioDsp);
    
    TP_PRINTF("DSPDrv1451_InitSection12 finish\r\n");
}

static void DSPDrv1451_Xtor_Cust(cDSPDrv1451* me)
{
    BOOST_DISABLE(gpioDsp);
    BOOST_CAP1_DISABLE(gpioDsp);
    BOOST_CAP2_DISABLE(gpioDsp);    
}





/************************************************************************
 **********  CUST FUNCTION                                     **********
 ************************************************************************/

/**
 * Enable pass-through or not
 * @param  -    passEnable
 */
#ifdef HAS_DSP_DYNA_BOOST
void DSPDrv1451_SetDcStatus(cDSPDrv1451 *me, bool dc_on)
{
    //TP_PRINTF("DSPDrv1451_SetDcStatus: dc=%d \r\n", dc_on);    
    const uint8 * ctrDataBoost = NULL;
    //const uint8 * ctrDataGain = NULL;
    
    /* For old battery/dc swtich, we always choose battery mode, to enable dynamic boost
     */
    //const uint8 * ctrDataBoostOld= ADAU1451_DYNA_BOOST_OLD_BATT_MODE_SELECT;
    
    if(dc_on) {
        ctrDataBoost= ADAU1451_DYNA_BOOST_DC_MODE_SELECT;
        //ctrDataGain= ADAU1451_GAIN_DC_MODE_SELECT;
    }
    else {
        ctrDataBoost= ADAU1451_DYNA_BOOST_BATT_MODE_SELECT;    
        //ctrDataGain= ADAU1451_GAIN_BATT_MODE_SELECT;
    }
    //DSPDrv1451_I2cWrite(me, DYNA_BOOST_DC_STATUS_REGISTER_LEN, (uint8*)ctrDataBoostOld);
    DSPDrv1451_I2cWrite(me, DYNA_BOOST_DC_STATUS_REGISTER_LEN, (uint8*)ctrDataBoost);
    //DSPDrv1451_I2cWrite(me, GAIN_DC_STATUS_REGISTER_LEN, (uint8*)ctrDataGain);
}


//uint32 DSPDrv1451_GetDynaBoostLevel(cDSPDrv1451 *me)
//{
//    uint8   volData[ADAU1451_DBOOST_LEVEL_LEN] = {0};
//    uint32  level= 0;
//
//    ASSERT(me->isCreated);
//    DSPDrv1451_I2cRead(me, ADAU1451_DBOOST_LEVEL_REGISTER, sizeof(volData), volData);
//    level= (volData[0]<<24) | (volData[1]<<16) | (volData[2]<<8) | volData[3];
//    //TP_PRINTF("DBoost-Level= %02X %02X %02X %02X = %d\r\n", volData[0], volData[1], volData[2], volData[3], level);
//      
//    return level;
//}
#endif /* HAS_DSP_DYNA_BOOST */



#ifdef HAS_AUDIO_MUTE_CHANNEL
/**
 * Mute Woofer or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteWoofer(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteWoofer: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_WOOFER_ENABLE;
    else
        ctrData= ADAU1451_MUTE_WOOFER_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_WOOFER_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute MiddleA or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleA(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteMiddleA: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_MIDDLEA_ENABLE;
    else
        ctrData= ADAU1451_MUTE_MIDDLEA_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_MIDDLEA_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute MiddleB or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteMiddleB(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteMiddleB: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_MIDDLEB_ENABLE;
    else
        ctrData= ADAU1451_MUTE_MIDDLEB_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_MIDDLEB_REGISTER_LEN, (uint8*)ctrData);
}

/**
 * Mute Tweeter or not
 * @param  -    bIsMute
 */
void DSPDrv1451_MuteTweeter(cDSPDrv1451 *me, bool bIsMute)
{
    TP_PRINTF("DSPDrv1451_MuteTweeter: bIsMute=%d \r\n", bIsMute);

    const uint8 * ctrData = NULL;
    if(bIsMute)
        ctrData= ADAU1451_MUTE_TWEETER_ENABLE;
    else
        ctrData= ADAU1451_MUTE_TWEETER_DISABLE;

    DSPDrv1451_I2cWrite(me, MUTE_TWEETER_REGISTER_LEN, (uint8*)ctrData);
}
#endif




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



#endif /*_ADAU1451_CONFIG_*/
