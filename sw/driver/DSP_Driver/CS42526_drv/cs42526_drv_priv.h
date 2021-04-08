/**
*  @file      dsp_TLV320AIC3254_driver.h
*  @brief     This file declares the driver for the Audio DSP TLV320AIC3254.
*  @author    Daniel Duan/Edmond
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/


#ifndef CS42526_DRV_PRIV_H
#define CS42526_DRV_PRIV_H

#include "SettingSrv.h"


#define REG_CHIP_REVISION           0x01
#define REG_POWER_CONTROL           0x02
#define REG_FUNCTION_MODE           0x03
#define REG_INTERF_FORMAT           0x04
#define REG_MISC_CONTROL            0x05
#define REG_CLOCK_CONTROL           0x06
#define REG_OMCK_PLL_CLK            0x07
#define REG_RVCR_STATUS             0x08
#define REG_VOL_TRANSITION_         0x0d
#define REG_CHANNEL_MUTE            0x0e
#define REG_VOL_CONTROL_0           0x0f
#define REG_VOL_CONTROL_1           0x10
#define REG_VOL_CONTROL_2           0x11
#define REG_VOL_CONTROL_3           0x12
#define REG_VOL_CONTROL_4           0x13
#define REG_VOL_CONTROL_6           0x14
#define REG_CHANNEL_INVERT          0x17
#define REG_MIX_CONTROL_PAIR_1        0x18
#define REG_MIX_CONTROL_PAIR_2        0x19
#define REG_MIX_CONTROL_PAIR_3        0x1a
#define REG_ADC_LEFT_GAIN               0x1c
#define REG_ADC_RIGHT_GAIN              0x1d
#define REG_RECEIVER_MODE_CONTROL_1     0x1e
#define REG_RECEIVER_MODE_CONTROL_2     0x1f
#define REG_INTERRUPT_STATUS            0x20
#define REG_INTERRUPT_MASK              0x21
#define REG_INTERRUPT_MODE_MSB          0x22
#define REG_INTERRUPT_MODE_LSB          0x23
#define REG_CHANNEL_STAT_BUF_CONTROL    0x24
#define REG_RECEIVER_CHANNEL_STATUS     0x25
#define REG_RECEIVER_ERROR              0x26
#define REG_RECEIVER_ERROR_MASK         0x27
#define REG_MUTEC_PIN_CONTROL           0x28
#define REG_RXP_GPIO_PIN_CONTROL_1        0x29
#define REG_RXP_GPIO_PIN_CONTROL_2        0x2a
#define REG_RXP_GPIO_PIN_CONTROL_3        0x2b
#define REG_RXP_GPIO_PIN_CONTROL_4        0x2c
#define REG_RXP_GPIO_PIN_CONTROL_5        0x2d
#define REG_RXP_GPIO_PIN_CONTROL_6        0x2e
#define REG_RXP_GPIO_PIN_CONTROL_7        0x2f
#define REG_QCHANNEL_SUBCODE_0          0x30            
#define REG_QCHANNEL_SUBCODE_1          0x31            
#define REG_QCHANNEL_SUBCODE_2          0x32            
#define REG_QCHANNEL_SUBCODE_3          0x33            
#define REG_QCHANNEL_SUBCODE_4          0x34            
#define REG_QCHANNEL_SUBCODE_5          0x35            
#define REG_QCHANNEL_SUBCODE_6          0x36            
#define REG_QCHANNEL_SUBCODE_7          0x37            
#define REG_QCHANNEL_SUBCODE_8          0x38            
#define REG_QCHANNEL_SUBCODE_9          0x39            

enum{
    RXP0,
    RXP1,
    RXP2,
    RXP3,
    RXP4,
    RXP5,
    RXP6,
    RXP7
};

typedef enum
{
    SUCCESS_CMP_BSP_VALID_INPUT,
    ERROR_CMP_BSP_INVALID_INPUT,
}ret_t;

/* DSP Status bit masks */
typedef enum
{
    DSP_INIT_NONE = 0,
    DSP_INIT_SEC0_COMPLETED = (1 << 0),
    DSP_INIT_SEC1_COMPLETED = (1 << 1),
    DSP_INIT_SEC2_COMPLETED = (1 << 2),
    DSP_INIT_SEC3_COMPLETED = (1 << 3)
} eDspStatus;

typedef enum
{
    CS42526_IN_NONE,
    CS42526_SPDIF0,
    CS42526_SPDIF1,
    CS42526_ANALOG1,
}eCS42526InputChannel;

typedef struct _cs42528_cfg_t
{
	union 
	{
		uint8 U;
		struct	//ADC Power down
		{
			uint8 Rsvd1: 	5;	//MUST BE ZERO
			uint8 PdnAdc:	1;	//Power down ADC
			uint8 Rsvd2:		2;	//MUST BE ZERO
		}B;
	}r02; //Power Control Register

	union 
	{
		uint8 U;
		struct	//DAC mode
		{
			uint8 Rsvd: 		2;	//MUST BE ZERO
			uint8 AdcSpSel: 	2;	//ADC Serial Port Select
			uint8 SaiFm:		2;	//Serial Audio Interface Mode: SSM, DSM, QSM
			uint8 CodecFm:	2;	//Codec Functional Mode: SSM, DSM, QSM
		}B;
	}r03; //Functional Mode Register

	union 
	{
		uint8 U;
		struct
		{
			uint8 Rsvd: 		4;	//MUST BE ZERO
			uint8 AdcOl:		2;	//ADC one_line Mode
			uint8 Dif:		2; 	//Digital interface format
	  	}B;
	}r04; //Interface Format Register

	union 
	{
		uint8 U;
		struct
		{
			uint8 Rsvd1:		1; 	//SAI SP is always Master mode. 
			uint8 CodecSpMS:	1; 	//CX SP Master/Slave mode
			uint8 Rsvd2:		5; 	//MUST BE ZERO
			uint8 ExtAdcSclk:1;	//External ADC SCLK Select
	  	}B;
	}r05; //Misc Control Register

	union 
	{
		uint8 U;
		struct
		{
			uint8 FrcPllLk: 1;	//Force PLL Lock
			uint8 SwCtrl:	2;	//Master Clock Source Selection
			uint8 Rsvd:		3; 	//MUST BE ZERO
			uint8 RmckDiv:	2; 	//RMCK_DIV
	  	}B;
	}r06; //Clock Control Register

	uint8 r0e_DacMute; //Channel Mute Register (0Eh)

	union 
	{
		uint8 U;
		struct 
		{
			uint8 Rmux:		3;	//SPDIF Rx0 or RX1 or HDMI-SPDIF
			uint8 Rsvd1:	1;	//MUST BE ZERO
			uint8 Tmux:		3;	//SPDIF Rx0 or RX1 or HDMI-SPDIF
			uint8 Rsvd2:	1;	//MUST BE ZERO
	  	}B;
	}r1f; //Receiver Mode Control-2 Register
	
}cs42528_cfg_t;

typedef enum _BOARDCFG_SPDIF_SRC_t
{
	BOARDCFG_SPDIF_SRC_RX0,
	BOARDCFG_SPDIF_SRC_RX1,
	BOARDCFG_SPDIF_SRC_RX2,
	BOARDCFG_SPDIF_SRC_RX3,
	BOARDCFG_SPDIF_SRC_RX4,
	BOARDCFG_SPDIF_SRC_RX5,
	BOARDCFG_SPDIF_SRC_RX6,
	BOARDCFG_SPDIF_SRC_RX7,
}BOARDCFG_SPDIF_SRC_t;

typedef enum _BOARDCFG_ADC_SRC_t
{
	BOARDCFG_ADC_SRC_STEREO,
	BOARDCFG_ADC_SRC_MIC,
	BOARDCFG_ADC_SRC_MULTI
}BOARDCFG_ADC_SRC_t;

typedef struct tCtrIdEQIdMap
{
    eDspSettId dspSettid;
    eSettingId settingId;  /* setting database index */
}tCtrIdEQIdMap;


//
// Define Parameters for DSP Audio Input Interface
//

typedef enum _BOARDCFG_INFS_t	//InFs (LRCLK to DSP DAI (HDMI ONLY)
{
	//1Fs
	BOARDCFG_INFS_48K = 0,
	BOARDCFG_INFS_44_1K,
	BOARDCFG_INFS_32K,

	//2Fs
	BOARDCFG_INFS_96K = 4,
	BOARDCFG_INFS_88_2K,
	BOARDCFG_INFS_64K,
	BOARDCFG_INFS_384K,

	//4Fs
	BOARDCFG_INFS_192K  = 8,
	BOARDCFG_INFS_176_4K,
	BOARDCFG_INFS_128K,
	BOARDCFG_INFS_768K,

	//unconfigured for read only
	BOARDCFG_INFS_UNCONFIGURED = 0xF
}BOARDCFG_INFS_t;

typedef enum _BOARDCFG_MCLK_t	//Input to DSP
{
	BOARDCFG_MCLK_TRISTATE=0,
	BOARDCFG_MCLK_128FS,	//HDMI only
	BOARDCFG_MCLK_256FS,	//Always use by SPDIF Rx0 and Rx1
	BOARDCFG_MCLK_512FS, 	//HDMI only
	BOARDCFG_MCLK_OMCK=0xF	//Not for HDMI
}BOARDCFG_MCLK_t;

typedef enum _BOARDCFG_SCLK_t	//Input to DSP
{
	BOARDCFG_SCLK_32FS=0,	//Not supported
	BOARDCFG_SCLK_64FS,		//Always
	BOARDCFG_SCLK_128FS		//Not supported
}BOARDCFG_SCLK_t;

typedef enum _BOARDCFG_DIGFORMAT_t	//Digital Interface Format to DSP
{
    BOARDCFG_DIGFORMAT_LJ,
    BOARDCFG_DIGFORMAT_I2S,
    BOARDCFG_DIGFORMAT_RSVD1,
    BOARDCFG_DIGFORMAT_RSVD2,
    BOARDCFG_DIGFORMAT_ONELINE_MODE1
}BOARDCFG_DIGFORMAT_t;

typedef enum _BOARDCFG_ADCMODE_t
{
	BOARDCFG_ADCMODE_SSM,
	BOARDCFG_ADCMODE_DSM,
	BOARDCFG_ADCMODE_QSM
}BOARDCFG_ADCMODE_t;


typedef enum _BOARDCFG_STREAM_TYPE_t
{
	BOARDCFG_STREAM_TYPE_PCM,
	BOARDCFG_STREAM_TYPE_COMPRESSED
}BOARDCFG_STREAM_TYPE_t;


//
// Define Parameters for DSP Audio Output Interface
//

typedef enum _BOARDCFG_DACMODE_t
{
	BOARDCFG_DACMOD_SSM,
	BOARDCFG_DACMOD_DSM,
	BOARDCFG_DACMOD_QSM
}BOARDCFG_DACMODE_t;

typedef enum _BOARDCFG_DACFORMAT_t
{
    BOARDCFG_DACFORMAT_LJ,
    BOARDCFG_DACFORMAT_I2S
}BOARDCFG_DACFORMAT_t;

typedef enum _BOARDCFG_SPDIFTX_t
{
    BOARDCFG_SPDIFTX_RXP0 = 0,
    BOARDCFG_SPDIFTX_RXP1,
    BOARDCFG_SPDIFTX_RXP2,
    BOARDCFG_SPDIFTX_RXP3,
    BOARDCFG_SPDIFTX_RXP4,
    BOARDCFG_SPDIFTX_RXP5,
    BOARDCFG_SPDIFTX_RXP6,
    BOARDCFG_SPDIFTX_RXP7
}BOARDCFG_SPDIFTX_t;


typedef struct _spdif_mode_cfg_t
{
	BOARDCFG_MCLK_t		eMclk:8;
	//BOARDCFG_INFS_t		eSclk:8;
	BOARDCFG_DIGFORMAT_t 	eDigFormat:8;
	BOARDCFG_DACMODE_t 	eDacMode:8;
	BOARDCFG_DACFORMAT_t 	eDacFormat:8;
}spdif_mode_cfg_t;

typedef struct _adc_mode_cfg_t
{
	BOARDCFG_INFS_t			eInFs:8;
	BOARDCFG_MCLK_t			eMclk:8;
	//BOARDCFG_INFS_t			eSclk:8;
	BOARDCFG_DIGFORMAT_t 	eDigFormat:8;
	BOARDCFG_ADCMODE_t		eAdcMode:8;
	BOARDCFG_DACFORMAT_t	eDacFormat:8;
	BOARDCFG_DACMODE_t 		eDacMode:8;
}adc_mode_cfg_t;


/* CMD structure */
typedef struct
{
    uint8 address;
    uint8 value;
} tDspData;


void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data);

void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes);

static void DSPDrv_cs42528_cfg(cDSPDrv* me, cs42528_cfg_t *pCs42528Cfg)	;
static ret_t DSPDrv_CfgAdcMode(cDSPDrv* me, adc_mode_cfg_t *pCfg);
static ret_t DSPDrv_CfgSpdifMode(cDSPDrv* me, spdif_mode_cfg_t *pCfg, BOARDCFG_SPDIF_SRC_t Rmux);
static void DSPDrv_setPower(cDSPDrv *me, bool sw);



#endif /* CS42526_DRV_PRIV_H */
