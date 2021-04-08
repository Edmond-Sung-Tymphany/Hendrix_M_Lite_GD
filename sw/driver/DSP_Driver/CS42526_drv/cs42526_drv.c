/**
*  @file      cs42526_drv.c
*  @brief     This file implements the driver for the DAC chip CS42526.
*  @modified  Edmond Sung
*  @date      01-2015
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "trace.h"
#include "DspDrv.h"
#include "cs42526_drv_priv.h"
#include "cs42526_drv_config.inc"

#include "I2CDrv.h"
#include "signals.h"
#include "bsp.h"

#ifndef DSP_DRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif


#ifndef NULL
#define NULL                             (0)
#endif

#ifdef DEBUG_DSP
#define DSP_DEBUG(x) DEBUG(x)
#else
#define DSP_DEBUG(x)
#endif

/* GPIO check times */
#define PIO_CHK_TIMES                                      (10)
#define PIO_CHK_CFM_TIMES                                  (3)



/* Private Variables */

static cI2CDrv dspI2c;

/* Private Functions */
static void DSPDrv_InitTask(void *p);
static void DSPDrv_InitSection(void *p);


void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(&dspI2c, &i2cMsg);
}

void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    I2CDrv_MasterRead(&dspI2c, &i2cMsg);
}




/**
 * This set of process are necessary for the DSP to bootup and need to be handle by the DSP/ Audio server.
 * Important Note that the delay time should never be zero
 */

static const tDspInitSection DspInitSection[]=
{
    {&DSPDrv_InitTask, 100},
    {&DSPDrv_InitTask, 1},
    {&DSPDrv_InitSection, 1}
};

static const tCtrIdEQIdMap ctrIdEQIdMap[] =
{
    /* DSP setting ID  index of setting db*/
    {DSP_VOLUME_SETT_ID,  SETID_VOLUME},
};


void DSPDrv_Ctor(cDSPDrv* me)
{
    tI2CDevice * conf = (tI2CDevice *) getDevicebyId(DSP_DEV_ID, NULL);
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    ASSERT(conf);
    me->pInitTable = DspInitSection;
    me->sectionSize = ArraySize(DspInitSection);
    me->initPhase = 0;
    me->max_vol = MAX_VOLUME;
    me->default_vol = DEFAULT_VOLUME;
    me->pI2CConfig = conf;
    me->isCreated = TRUE;
}

void DSPDrv_Xtor(cDSPDrv* me)
{
    DSPDrv_setPower(me, 0);
    me->pInitTable = NULL;
    me->sectionSize = 0;
    me->initPhase = 0;
    me->pI2CConfig = NULL;
    I2CDrv_Xtor(&dspI2c);
}


static void DSPDrv_InitTask(void *p)
{
    cDSPDrv *me = (cDSPDrv*)p;
    ASSERT(me->pI2CConfig);
    /* If the device type is I2C, create the DSP I2C object */
    if (me->pI2CConfig->deviceInfo.deviceType==I2C_DEV_TYPE)
        I2CDrv_Ctor(&dspI2c, me->pI2CConfig);

}


static void DSPDrv_InitSection(void *p)
{
    uint8 i;
    cDSPDrv *me = (cDSPDrv*)p;

    //DSPDrv_I2cRead(&tmp, me->pI2CConfig->address, 0x01, 1);

    for(i=0; i<ArraySize(dsp_init_cmd_sec); i++)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, sizeof(tDspData), (uint8*)(&(dsp_init_cmd_sec[i])));
    }
}

static void DSPDrv_setPower(cDSPDrv *me, bool sw)
{
    uint8 Data[2];
    Data[0] = REG_POWER_CONTROL;
    Data[1] = sw?0x00:0xff;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}

void DSPDrv_MuteDACOut(cDSPDrv *me)
{
    uint8 Data[2];
    Data[0] = REG_CHANNEL_MUTE;
    Data[1] = 0x3f;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}

void DSPDrv_UnMuteDACOut(cDSPDrv *me)
{
    uint8 Data[2];
    Data[0] = REG_CHANNEL_MUTE;
    Data[1] = 0x00;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}


void DSPDrv_ActivateAmp(cDSPDrv *me)
{
}

void DSPDrv_DeActivateAmp(cDSPDrv *me)
{
}

eLineoutStatus DSPDrv_GetLineoutStatus(cDSPDrv *me)
{
    return LINEOUT_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////
//! \brief This function sets up the system to ADC mode.
//!
//! \param[in]  pCfg - Pointer to a structure which contains ADC mode 
//! configuration parameters.
//!
//! \retval TP_SUCCESS   
//! \retval ERROR_CMP_BSP_INVALID_INPUT
//!
//! \note Use one "\note" command for each note.
////////////////////////////////////////////////////////////////////////////////
static ret_t DSPDrv_CfgAdcMode(cDSPDrv* me, adc_mode_cfg_t *pCfg)
{
	cs42528_cfg_t Cs42528Cfg = {0, 0, 0, 0, 0, 0, 0};
	//
	// Configure CS42528 for ADC Input


	//Analog Stereo Input Configuration 
	Cs42528Cfg.r1f.B.Rmux 	= 2;
	Cs42528Cfg.r1f.B.Tmux 	= 7;    // output from RX7
	//Cs42528Cfg.r1f.B.Tmux 	= pCfg->eSpdifTx;

	Cs42528Cfg.r02.B.PdnAdc = 0;

	Cs42528Cfg.r03.B.CodecFm	= pCfg->eDacMode;

	Cs42528Cfg.r03.B.SaiFm	= pCfg->eAdcMode;
		
	Cs42528Cfg.r03.B.AdcSpSel	= 2;


	if(pCfg->eDigFormat == BOARDCFG_DIGFORMAT_ONELINE_MODE1)
    {
		Cs42528Cfg.r04.B.AdcOl	= 1;
		Cs42528Cfg.r04.B.Dif	= 1; //unused

		Cs42528Cfg.r05.B.ExtAdcSclk	= 1;	
		Cs42528Cfg.r05.B.CodecSpMS	= 1;
	}
	else
	{
		Cs42528Cfg.r04.B.AdcOl	= 0;

		Cs42528Cfg.r04.B.Dif = pCfg->eDacFormat;
	
	   	Cs42528Cfg.r05.B.ExtAdcSclk	= 0;	
	}	

	Cs42528Cfg.r0e_DacMute = 0;
		
	Cs42528Cfg.r06.B.FrcPllLk	= 0;
	Cs42528Cfg.r06.B.SwCtrl		= 1;

	DSPDrv_cs42528_cfg(me, &Cs42528Cfg);	//Config Codec and SPDIF

	return SUCCESS_CMP_BSP_VALID_INPUT;
}
////////////////////////////////////////////////////////////////////////////////
//! \brief This function sets up the system to SPDIF mode.
//!
//! \param[in]  pCfg - Pointer to a structure which contains SPDIF mode 
//! configuration parameters.
//!
//! \retval TP_SUCCESS   
//! \retval ERROR_CMP_BSP_INVALID_INPUT
//!
//! \note Use one "\note" command for each note.
////////////////////////////////////////////////////////////////////////////////
static ret_t DSPDrv_CfgSpdifMode(cDSPDrv* me, spdif_mode_cfg_t *pCfg, BOARDCFG_SPDIF_SRC_t Rmux)
{
	cs42528_cfg_t Cs42528Cfg = {0, 0, 0, 0, 0, 0, 0};
	//
	// Configure CS42528 for SPDIF
	//

	if(pCfg->eMclk == BOARDCFG_MCLK_256FS)
		Cs42528Cfg.r06.B.RmckDiv = 0;			//Divided by 1
	else if(pCfg->eMclk == BOARDCFG_MCLK_128FS)
		Cs42528Cfg.r06.B.RmckDiv = 1;			//Divided by 2
		

	Cs42528Cfg.r1f.B.Rmux = Rmux;
	Cs42528Cfg.r1f.B.Tmux 	= 7;    // output from RX7

	Cs42528Cfg.r02.B.PdnAdc = 1;


	Cs42528Cfg.r03.B.SaiFm	=
	Cs42528Cfg.r03.B.CodecFm= pCfg->eDacMode;

	Cs42528Cfg.r03.B.AdcSpSel	= 1;

    Cs42528Cfg.r04.B.Dif	= pCfg->eDacFormat;

	Cs42528Cfg.r04.B.AdcOl		= 0;

	Cs42528Cfg.r05.B.ExtAdcSclk	= 0;	

	Cs42528Cfg.r0e_DacMute 	= 0x3f;     //mute
		
	Cs42528Cfg.r06.B.FrcPllLk	= 0;
	Cs42528Cfg.r06.B.SwCtrl		= 3;

	DSPDrv_cs42528_cfg(me, &Cs42528Cfg);

	return SUCCESS_CMP_BSP_VALID_INPUT;
}




static void DSPDrv_cs42528_cfg(cDSPDrv* me, cs42528_cfg_t *pCs42528Cfg)	
{
    uint8 Data[2]={REG_RECEIVER_MODE_CONTROL_2, 0x00};

    //Cfg Dac Mute
    //Data[0] = REG_CHANNEL_MUTE;
    //Data[1] = 0xff;             // MUTE
    //DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
    
    //Cfg Function Mode
    Data[0] = REG_FUNCTION_MODE;
    Data[1] = 0x00| pCs42528Cfg->r03.U; ;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    //Cfg Interface Format Register
    Data[0] = REG_INTERF_FORMAT;
    Data[1] = 0x00| pCs42528Cfg->r04.U; 
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    Data[0] = REG_MISC_CONTROL;
    Data[1] = 0x01| pCs42528Cfg->r05.U;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    //Cfg MCLK from PLL for SPDIF/HDMI mode 
    Data[0] = REG_CLOCK_CONTROL;
    Data[1] = 0x00| pCs42528Cfg->r06.U; 
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    //Cfg Receiver Mode Control-2 Register
    Data[0] = REG_RECEIVER_MODE_CONTROL_2;
    Data[1] = 0x00| pCs42528Cfg->r1f.U; 
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    //Cfg Dac Mute
    //Data[0] = REG_CHANNEL_MUTE;
    //Data[1] = 0x00| pCs42528Cfg->r0e_DacMute;
    //DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}


uint16 DSPDrv_Init(cDSPDrv* me)
{
    //static uint16 phase=0;
    uint16 delaytime;
    ASSERT(me);
    //if we are at max init steps we probably need to start init again 
    ASSERT(me->pInitTable);
    me->pInitTable[me->initPhase].initSectionFunc(me);
    delaytime=me->pInitTable[me->initPhase].delaytime;
    me->initPhase++;
    if (me->initPhase == me->sectionSize)
    {
        me->initPhase = 0;
        delaytime = 0;
    }   
    return delaytime;
}


#ifdef AUDIO_MULTI_SOURCE
/*
static void CS42526_SPDIF_channel_select(cDSPDrv *me, cs42528_cfg_t *pCs42528Cfg, BOARDCFG_SPDIFTX_t spdif_rx_channel, BOARDCFG_SPDIFTX_t spdif_tx_channel)
{
    uint8 Data[2]={REG_RECEIVER_MODE_CONTROL_2, 0x00};
    pCs42528Cfg->r1f.B.Rmux = spdif_rx_channel;
    pCs42528Cfg->r1f.B.Tmux = spdif_tx_channel;
    Data[1]=pCs42528Cfg->r1f.U;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}
*/
//TODO: this function is not generic enough and need to be re-write in generic form so it will be easy to re-config.
static void DSPDrv_InputChannelSelect(cDSPDrv *me, eCS42526InputChannel audio_input)
{
    switch (audio_input)
    {
        case CS42526_SPDIF0:
            DSPDrv_CfgSpdifMode(me, (spdif_mode_cfg_t*)&spdif_mode_cfg, BOARDCFG_SPDIF_SRC_RX0);
            break;
        case CS42526_SPDIF1:
            DSPDrv_CfgSpdifMode(me, (spdif_mode_cfg_t*)&spdif_mode_cfg, BOARDCFG_SPDIF_SRC_RX1);
            break;
        case CS42526_ANALOG1:
            DSPDrv_CfgAdcMode(me, (adc_mode_cfg_t*)&adc_mode_cfg);
            break;
    }
}

#endif // AUDIO_MULTI_SOURCE

void DSPDrv_set_Input(cDSPDrv *me, eAudioCtrlDriverInput input)
{
    ASSERT(input < AUDIOCTRL_DRIVER_INPUT_MAX);
    //DSPDrv_MuteDACOut(me);
    switch(input)
    {

        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_2:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_3:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1_2:
            DSPDrv_InputChannelSelect(me, CS42526_ANALOG1);
            break;
        case AUDIOCTRL_DRIVER_INPUT_SPDIF_0:
            DSPDrv_InputChannelSelect(me, CS42526_SPDIF0);
            break;
        case AUDIOCTRL_DRIVER_INPUT_SPDIF_1:
            DSPDrv_InputChannelSelect(me, CS42526_SPDIF1);
            break;
         default:
            break;

    }
    //DSPDrv_UnMuteDACOut(me);
}

static void DSPDrv_SetVol_dB(cDSPDrv *me, uint8 minusDB)
{
    uint8 Data[2];
    Data[0] = REG_VOL_CONTROL_0;
    Data[1] = minusDB*2;
    for(uint8 i=0;i<6;i++)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
        Data[0]++;
    }
}


void DSPDrv_SetVol(cDSPDrv *me, uint8 vol)
{
    TP_PRINTF("DSPDrv_SetVol(%d)\r\n", vol);
    DSPDrv_SetVol_dB(me, MASTER_VOL[vol]);
}



int DSPDrv_setStereoMux(cDSPDrv *me, eDspMuxChannel ch)
{
    return 0;
}

bool DSPDrv_HasMusicStream(cDSPDrv *me)
{
    return TRUE;
}

bool DSPDrv_IsPCM(cDSPDrv *me)
{
    uint8 tmp[2];
    DSPDrv_I2cRead(&tmp[0], me->pI2CConfig->address, 0x25, 2);
    tmp[0]&=0x04;
    return (!tmp[0]);
}


bool DSPDrv_IsAuxin(cDSPDrv *me)
{
    return TRUE;
}



/*
 * Below are the public functions which are not used in this project but have to
 * be declared to supress compiling errors
 */
void DSPDrv_SetTunning(uint16 freq)
{}
void DSPDrv_SetBGC(int16 boost)
{}
void DSPDrv_SetPolarity(uint8 polarity)
{}


void DSPDrv_SetAudio(cDSPDrv *me, eDspSettId dspSettId, BOOL enable)
{
    uint8 i;
    for(i = 0; i < ArraySize(ctrIdEQIdMap); i++)
    {
        if(dspSettId == ctrIdEQIdMap[i].dspSettid)
        {
            break;
        }
    }
    ASSERT(dspSettId == DSP_VOLUME_SETT_ID);
    if(enable)
    {
        int8 volumeLevel = 0;
        volumeLevel =  *(int8*)Setting_Get(ctrIdEQIdMap[i].settingId);
        if(volumeLevel <= MAX_VOLUME && volumeLevel >= 0)
        {
            DSPDrv_SetVol(me,volumeLevel);
        }
    }
    else
    {
        DSPDrv_SetVol(me,0);
    }
}

