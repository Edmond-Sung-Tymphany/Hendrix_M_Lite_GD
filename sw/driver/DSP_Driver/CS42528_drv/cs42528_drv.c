/**
*  @file      cs42528_drv.c
*  @brief     This file implements the driver for the DAC chip CS42526.
*  @modified  Edmond Sung
*  @date      10-2016
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "trace.h"
#include "DspDrv.h"
#include "cs42528_drv.h"
#include "cs42528_drv_priv.h"
#include "cs42528_drv_config.inc"

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


/* Private Functions */
static void CS42528Drv_InitTask(void *p);
static void CS42528Drv_InitSection(void *p);

/**
 * This set of process are necessary for the DSP to bootup and need to be handle by the DSP/ Audio server.
 * Important Note that the delay time should never be zero
 */

static const tDspInitSection Cs42528InitSection[]=
{
    {&CS42528Drv_InitTask, 100},
    {&CS42528Drv_InitTask, 1},
    {&CS42528Drv_InitSection, 1}
};

void CS42528Drv_I2cEnable(cCS42528Drv* me, bool i2cEnable)
{
    me->i2cEnable= i2cEnable;
}

void CS42528Drv_I2cWrite(cCS42528Drv* me, uint8 bytes, const uint8 *data)
{
    if (me->i2cEnable == 0)
    {
        return;
    }
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceAddr,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(me->i2cObj, &i2cMsg);
}

void CS42528Drv_I2cRead(cCS42528Drv* me, uint8 * bufptr,  uint8 reg_add, uint16 bytes)
{
    if (me->i2cEnable == 0)
    {
        return;
    }
    tI2CMsg i2cMsg=
    {
        .devAddr = me->deviceAddr,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    I2CDrv_MasterRead(me->i2cObj, &i2cMsg);
}






void CS42528Drv_Ctor(cCS42528Drv* me, cI2CDrv *pI2cObj)
{
    tI2CDevice * conf = (tI2CDevice *) getDevicebyId(AUDIO_CODEC_ID, NULL);
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    ASSERT(conf);
    me->pInitTable = Cs42528InitSection;
    me->sectionSize = ArraySize(Cs42528InitSection);
    me->initPhase = 0;
    me->i2cObj= pI2cObj;
    me->deviceAddr = pI2cObj->pConfig->devAddress;
    me->isCreated = TRUE;
    me->i2cEnable = TRUE;
}

void CS42528Drv_Xtor(cCS42528Drv* me)
{
    me->pInitTable = NULL;
    me->sectionSize = 0;
    me->initPhase = 0;
    me->i2cObj = NULL;
    I2CDrv_Xtor(me->i2cObj);
}


static void CS42528Drv_InitTask(void *p)
{
    cCS42528Drv *me = (cCS42528Drv*)p;
    const tDevice * pDevice = NULL;
    pDevice = getDevicebyIdAndType(AUDIO_CODEC_ID, I2C_DEV_TYPE, NULL);
    ASSERT(me->i2cObj);
    I2CDrv_Ctor(me->i2cObj, (tI2CDevice*)pDevice);

}


static void CS42528Drv_InitSection(void *p)
{
    uint8 i;
    cCS42528Drv *me = (cCS42528Drv*)p;

    //DSPDrv_I2cRead(&tmp, me->pI2CConfig->address, 0x01, 1);

    for(i=0; i<ArraySize(dsp_init_cmd_sec); i++)
    {
        CS42528Drv_I2cWrite(me, sizeof(tDspData), (uint8*)(&(dsp_init_cmd_sec[i])));
    }
}

static void CS42528Drv_setPower(cCS42528Drv *me, bool sw)
{
    uint8 Data[2];
    Data[0] = REG_POWER_CONTROL;
    Data[1] = sw?0x00:0xff;
    CS42528Drv_I2cWrite(me, 2, Data);
}

void CS42528Drv_MuteDACOut(cCS42528Drv *me)
{
    uint8 Data[2];
    Data[0] = REG_CHANNEL_MUTE;
    Data[1] = 0xff;
    CS42528Drv_I2cWrite(me, 2, Data);
}

void CS42528Drv_UnMuteDACOut(cCS42528Drv *me)
{
    uint8 Data[2];
    Data[0] = REG_CHANNEL_MUTE;
    Data[1] = 0x00;
    CS42528Drv_I2cWrite(me, 2, Data);
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
static ret_t CS42528Drv_CfgAdcMode(cCS42528Drv* me, adc_mode_cfg_t *pCfg)
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

    CS42528Drv_cfg(me, &Cs42528Cfg);	//Config Codec and SPDIF

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
static ret_t CS42528Drv_CfgSpdifMode(cCS42528Drv* me, spdif_mode_cfg_t *pCfg, BOARDCFG_SPDIF_SRC_t Rmux)
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

    CS42528Drv_cfg(me, &Cs42528Cfg);

    return SUCCESS_CMP_BSP_VALID_INPUT;
}




static void CS42528Drv_cfg(cCS42528Drv* me, cs42528_cfg_t *pCs42528Cfg)	
{
    uint8 Data[2]={REG_RECEIVER_MODE_CONTROL_2, 0x00};

    //Cfg Dac Mute
    //Data[0] = REG_CHANNEL_MUTE;
    //Data[1] = 0xff;             // MUTE
    //DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);

    //Cfg Function Mode
    Data[0] = REG_FUNCTION_MODE;
    Data[1] = 0x00| pCs42528Cfg->r03.U; ;
    CS42528Drv_I2cWrite(me, 2, Data);

    //Cfg Interface Format Register
    Data[0] = REG_INTERF_FORMAT;
    Data[1] = 0x00| pCs42528Cfg->r04.U; 
    CS42528Drv_I2cWrite(me, 2, Data);

    Data[0] = REG_MISC_CONTROL;
    Data[1] = 0x01| pCs42528Cfg->r05.U;
    CS42528Drv_I2cWrite(me, 2, Data);

    //Cfg MCLK from PLL for SPDIF/HDMI mode 
    Data[0] = REG_CLOCK_CONTROL;
    Data[1] = 0x00| pCs42528Cfg->r06.U; 
    CS42528Drv_I2cWrite(me, 2, Data);

    //Cfg Receiver Mode Control-2 Register
    Data[0] = REG_RECEIVER_MODE_CONTROL_2;
    Data[1] = 0x00| pCs42528Cfg->r1f.U; 
    CS42528Drv_I2cWrite(me, 2, Data);

    //Cfg Dac Mute
    //Data[0] = REG_CHANNEL_MUTE;
    //Data[1] = 0x00| pCs42528Cfg->r0e_DacMute;
    //DSPDrv_I2cWrite(me->pI2CConfig->address, 2, Data);
}


uint16 CS42528Drv_Init(cCS42528Drv* me)
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
static void CS42528Drv_InputChannelSelect(cCS42528Drv *me, eCS42526InputChannel audio_input)
{
    switch (audio_input)
    {
        case CS42526_SPDIF0:
            CS42528Drv_CfgSpdifMode(me, (spdif_mode_cfg_t*)&spdif_mode_cfg, BOARDCFG_SPDIF_SRC_RX0);
            break;
        case CS42526_SPDIF1:
            CS42528Drv_CfgSpdifMode(me, (spdif_mode_cfg_t*)&spdif_mode_cfg, BOARDCFG_SPDIF_SRC_RX1);
            break;
        case CS42526_ANALOG1:
            CS42528Drv_CfgAdcMode(me, (adc_mode_cfg_t*)&adc_mode_cfg);
            break;
    }
}

#endif // AUDIO_MULTI_SOURCE

/*
void CS42528Drv_set_Input(cCS42528Drv *me, eAudioCtrlDriverInput input)
{
    ASSERT(input < AUDIOCTRL_DRIVER_INPUT_MAX);
    //DSPDrv_MuteDACOut(me);
    switch(input)
    {

        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_2:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_3:
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1_2:
            CS42528Drv_InputChannelSelect(me, CS42526_ANALOG1);
            break;
        case AUDIOCTRL_DRIVER_INPUT_SPDIF_0:
            CS42528Drv_InputChannelSelect(me, CS42526_SPDIF0);
            break;
        case AUDIOCTRL_DRIVER_INPUT_SPDIF_1:
            CS42528Drv_InputChannelSelect(me, CS42526_SPDIF1);
            break;
         default:
            break;

    }
    //CS42528Drv_UnMuteDACOut(me);
}
*/
static void CS42528Drv_SetVol_dB(cCS42528Drv *me, uint8 minusDB)
{
    uint8 Data[2];
    ASSERT(minusDB<128);
    Data[0] = REG_VOL_CONTROL_0;
    Data[1] = minusDB*2;
    for(uint8 i=0;i<NUMBER_OF_DAC;i++)
    {
        CS42528Drv_I2cWrite(me, sizeof(Data), Data);
        Data[0]++;
    }
}




