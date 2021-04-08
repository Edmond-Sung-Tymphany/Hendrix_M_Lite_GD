#ifndef _ADAU1761_CONFIG_
#define _ADAU1761_CONFIG_

/************************************************************************
 **********  INCLUDE                                           **********
 ************************************************************************/
typedef const uint8 ADI_REG_TYPE; //must before include file
#include "ADAU1761_IC_1_PARAM.h"
#include "ADAU1761_IC_1.h"
#include "attachedDevices.h"



/************************************************************************
 **********  PIN CONTROL                                       **********
 ************************************************************************/
#define  DSP_VCC_ON(x)       GpioDrv_SetBit(&(x),GPIO_OUT_DSP_VCC_ON) //Active HIGH
#define  DSP_VCC_OFF(x)      GpioDrv_ClearBit(&(x),GPIO_OUT_DSP_VCC_ON)

#define  DSP_I2S_ENABLE(x)   { \
        GpioDrv_ClearBit(&(x),GPIO_OUT_DSP_I2S_ENABLE_N); \
        GpioDrv_ClearBit(&(x),GPIO_OUT_DSP_I2S_ENABLE_EVT2_REWORK_N); } //Active LOW

#define  DSP_I2S_DISABLE(x)  { \
        GpioDrv_SetBit(&(x),GPIO_OUT_DSP_I2S_ENABLE_N); \
        GpioDrv_SetBit(&(x),GPIO_OUT_DSP_I2S_ENABLE_EVT2_REWORK_N); }




/************************************************************************
 **********  CONFIGURATION                                     **********
 ************************************************************************/
#define ADAU1761_MAX_TX_BYTES   80
#define DC_REGISTER_LEN     (6)


#define SIGMA_WRITE_REGISTER_BLOCK(dev_address, reg_addr, data_len, pdata) \
    DSPDrv1761_Writer_Register(me, reg_addr, data_len, (uint8*)pdata);



#define ADAU1761_I2S_CONTROL_ADDR 0x4015

/* Source selection
 * the register for source selection is changed when the DSP layout is changed
 * so the complier switch is here
 */
#define SOURCE_SELECT_REGISTER_HIGH_BYTE    (MOD_SOURCESWITCH_SWSOURCESWITCH_ALG0_STAGE0_STEREOSWITCHNOSLEW_ADDR >> 8)
#define SOURCE_SELECT_REGISTER_LOW_BYTE     (MOD_SOURCESWITCH_SWSOURCESWITCH_ALG0_STAGE0_STEREOSWITCHNOSLEW_ADDR & 0xff)
#define SOURCE_SELECT_REGISTER_LEN          (10)

//I2S from ASE-NG
const uint8 ADAU1451_DIGITAL_INPUT_1_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x80, 0x00, 0x00};

//I2S from AUX-IN
const uint8 ADAU1451_DIGITAL_INPUT_2_SELECT[SOURCE_SELECT_REGISTER_LEN]  = {SOURCE_SELECT_REGISTER_HIGH_BYTE, SOURCE_SELECT_REGISTER_LOW_BYTE,
                                                  0x00, 0x80, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};


/* Speaker Position Setting */
#define SPEAKER_POSITION_SWITCH_REGISTER_HIGH_BYTE    (MOD_POSSWITCH_POSSW_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR >> 8)
#define SPEAKER_POSITION_SWITCH_REGISTER_LOW_BYTE     (MOD_POSSWITCH_POSSW_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR & 0xff)
#define SPEAKER_POSITION_SWITCH_REGISTER_LEN          (14)

const uint8 ADAU1451_SPEAKER_POSITION_FREE[SPEAKER_POSITION_SWITCH_REGISTER_LEN]  = {SPEAKER_POSITION_SWITCH_REGISTER_HIGH_BYTE, SPEAKER_POSITION_SWITCH_REGISTER_LOW_BYTE,
                                                  0x00, 0x80, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_SPEAKER_POSITION_WALL[SPEAKER_POSITION_SWITCH_REGISTER_LEN]  = {SPEAKER_POSITION_SWITCH_REGISTER_HIGH_BYTE, SPEAKER_POSITION_SWITCH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x80, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00};
const uint8 ADAU1451_SPEAKER_POSITION_CORNER[SPEAKER_POSITION_SWITCH_REGISTER_LEN]  = {SPEAKER_POSITION_SWITCH_REGISTER_HIGH_BYTE, SPEAKER_POSITION_SWITCH_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00,
                                                  0x00, 0x80, 0x00, 0x00};


/* Auxin Mute Control
 */
#define SOURCE_LINEIN_MUTE_CONTROL_REGISTER_HIGH_BYTE    (MOD_LINEINSIGNALDETECTION_MUTELINEIN_MUTENOSLEWALG2MUTE_ADDR >> 8)
#define SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LOW_BYTE     (MOD_LINEINSIGNALDETECTION_MUTELINEIN_MUTENOSLEWALG2MUTE_ADDR & 0xff)
#define SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LEN          (6)

const uint8 ADAU1451_DIGITAL_LINEIN_UNMUTE_SELECT[SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LEN]  = {SOURCE_LINEIN_MUTE_CONTROL_REGISTER_HIGH_BYTE, SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LOW_BYTE,
                                                  0x00, 0x80, 0x00, 0x00};

const uint8 ADAU1451_DIGITAL_LINEIN_MUTE_SELECT[SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LEN]  = {SOURCE_LINEIN_MUTE_CONTROL_REGISTER_HIGH_BYTE, SOURCE_LINEIN_MUTE_CONTROL_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};



/* Tone Touch
 */
#define TONE_TOUCH_ENABLE_CONTROL_REGISTER_HIGH_BYTE    (MOD_VOLUMECONTROL_TT_BYPASS_MONOSWSLEW_ADDR >> 8)
#define TONE_TOUCH_ENABLE_CONTROL_REGISTER_LOW_BYTE     (MOD_VOLUMECONTROL_TT_BYPASS_MONOSWSLEW_ADDR & 0xff)
#define TONE_TOUCH_ENABLE_CONTROL_REGISTER_LEN          (6)

const uint8 TONE_TOUCH_BYPASS_SELECT[TONE_TOUCH_ENABLE_CONTROL_REGISTER_LEN]  = {TONE_TOUCH_ENABLE_CONTROL_REGISTER_HIGH_BYTE, TONE_TOUCH_ENABLE_CONTROL_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x00};

const uint8 TONE_TOUCH_ENABLE_SELECT[TONE_TOUCH_ENABLE_CONTROL_REGISTER_LEN]  = {TONE_TOUCH_ENABLE_CONTROL_REGISTER_HIGH_BYTE, TONE_TOUCH_ENABLE_CONTROL_REGISTER_LOW_BYTE,
                                                  0x00, 0x00, 0x00, 0x01};




/* I2S slave/master mode
 */
#define I2S_MODE_SELECT_REGISTER_HIGH_BYTE    (0x4015 >> 8)
#define I2S_MODE_SELECT_REGISTER_LOW_BYTE     (0x4015 & 0xff)
#define I2S_MODE_SELECT_REGISTER_LEN          (3)

//I2S master
const uint8 ADAU1451_I2S_MODE_SELECT_MASTER[SOURCE_SELECT_REGISTER_LEN]  = {I2S_MODE_SELECT_REGISTER_HIGH_BYTE, I2S_MODE_SELECT_REGISTER_LOW_BYTE,
                                                  0x09};

//I2S slave
const uint8 ADAU1451_I2S_MODE_SELECT_SLAVE[SOURCE_SELECT_REGISTER_LEN]  = {I2S_MODE_SELECT_REGISTER_HIGH_BYTE, I2S_MODE_SELECT_REGISTER_LOW_BYTE,
                                                  0x08};




//#define ADAU1761_AUDIO_DETECT_ADDR                (MOD_LINEINSIGNALDETECTION_SWSIGNALDETECTION_READBACKALGSIGMA2001_ADDR)

#define DSP_VER_REGISTER_LEN                      (4)
#define DSP_VER_REGISTER                          (MOD_SWVERSION_DCINPALG1_ADDR)


#define ADAU1761_AUXIN_SIG_DETECT_ADDR                (MOD_LINEINSIGNALDETECTION_SWSIGNALDETECTION_2_READBACKALGSIGMA2002_ADDR)


/* Pass through enable */
#define PASS_THROUGH_SW1_1_REGISTER_HIGH_BYTE    (MOD_BYPASS_SWBYPASS1_ALG0_STEREODEMUX1940NS10_ADDR >> 8)
#define PASS_THROUGH_SW1_1_REGISTER_LOW_BYTE     (MOD_BYPASS_SWBYPASS1_ALG0_STEREODEMUX1940NS10_ADDR & 0xff)
#define PASS_THROUGH_REGISTER_LEN          (6)


/*Volume control*/
#define ADAU1761_VOLUME_CONTROL_ADDR             MOD_VOLUMECONTROL_VOLUME_DCINPALG7_ADDR//MOD_VOLUMECONTROL_SWVOLUME_ALG0_GAINS200ALG1GAINTARGET_ADDR
#define ADAU1761_UPDATE_VOLUME_LEN               6



#endif /*_ADAU1761_CONFIG_*/
