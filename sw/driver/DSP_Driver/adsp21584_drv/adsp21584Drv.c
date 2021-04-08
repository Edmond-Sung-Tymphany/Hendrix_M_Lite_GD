/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  dsp ADSP-21584 driver
                  -------------------------

                  SW Module Document




@file        dsp_adsp21584_driver.c
@brief       This file implements the drivers for adsp-21584 
@author      Ones Yang
@date        2016-06-22
@copyright (c) Tymphany Ltd. All rights reserved. 
-------------------------------------------------------------------------------
*/
#include <stdio.h>
#include "trace.h"
#include "cplus.h"
#include "commonTypes.h"
#include "SpiDrv.h"
#include "GpioDrv.h"
#include "DspDrv21584.h"
#include "attachedDevices.h"



/***********************************************************/
/********************* Definition **************************/
/***********************************************************/
#ifndef NULL
#define NULL          (0)
#endif



/***********************************************************/
/****************** PUBLIC FUNCTION ************************/
/***********************************************************/
/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv21584_Ctor(cDSPDrv21584* me, cSpiDrv *pSpiObj)
{
    me->spiObj        = pSpiObj;
    me->isCreated     = TRUE;
    me->AdspIntOccur  = 0x00;
    me->isAdspInitDone= FALSE;
    me->spiEnable     = TRUE;
    DSPDrv21584_InitSpi(me);
    DSPDrv21584_InitGpio(me);
}

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv21584_Xtor(cDSPDrv21584* me)
{  
    ASSERT(me && me->isCreated);  
    SpiDrv_Xtor(me->spiObj);
    GpioDrv_Xtor(me->gpioObj);
    me->isCreated     = FALSE;

}


/**
 * Mutes the DSP DAC output
 *
 * @param      void
 * @return     void
 */
uint8 tempTest[4] = {0,0,0,0};      //just for temporary testing
void DSPDrv21584_Mute(cDSPDrv21584 *me)
{
    tSpiTxBuf SpiTxBuf;
    SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;
    SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
    SpiTxBuf.dataBits = MUTE_OUTPUT;
    //tempTest[0] = 0x04;
    //tempTest[1] = 0x00;
    //tempTest[2] = 0x00;
    //tempTest[3] = 0x20;
    //DSPDrv21584_SpiWrite(me,4,tempTest);
    DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
}

/**
 * Unmutes the DSP DAC output.
 *
 * @param      void
 * @return     void
 */
void DSPDrv21584_UnMute(cDSPDrv21584 *me)
{
    tSpiTxBuf SpiTxBuf;
    SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;
    SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
    SpiTxBuf.dataBits = MUTE_OUTPUT;
    DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
}

/**
 *  Start play
 *
 * @param      void
 * @return     void
 */   
void DSPDrv21584_StartPlay(cDSPDrv21584 *me)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;
   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = START_PLAY;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
}

/**
 *  Stop play
 *
 * @param      void
 * @return     void
 */
void DSPDrv21584_StopPlay(cDSPDrv21584 *me)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;
   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = STOP_PLAY;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
}

/**
 *  SPORT initial
 *
 * @param      void
 * @return     void
 */
void DSPDrv21584_SportInit(cDSPDrv21584 *me)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;
   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = SPORT_INIT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
}

/**
 *  General Decoding Options
 *
 * @param
 * @return     void
 */
void DSPDrv21584_GenDecoOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = param[0];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_PARAM_2;
   SpiTxBuf.dataBits = param[1];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send second parameter

   SpiTxBuf.cmdBufAddr = DSP_PARAM_3;
   SpiTxBuf.dataBits = param[2];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send third parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = GENERAL_DECODE_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send general decoding option command
}

/**
 *  TrueHD Option
 *
 * @param
 * @return     void
 */
void DSPDrv21584_TrueHDOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = *param;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send tureHD parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = TRUEHD_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send trueHD option command
}

/**
 *  DDPlus Option
 *
 * @param
 * @return     void
 */
void DSPDrv21584_DDPlusOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = param[0];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_PARAM_2;
   SpiTxBuf.dataBits = param[1];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = DDPLUS_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send DDPlus option command
}

/**
 *  Atmos PCM Option
 *
 * @param
 * @return     void
 */
void DSPDrv21584_AtmosMatPcmOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = *param;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = ATMOS_MAT_PCM_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send Atmos Mat PCM option command
}

/**
 *  Dolby Audio Processing set up
 *
 * @param
 * @return     void
 */
void DSPDrv21584_DolbyAudioProSetup(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = param[0];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_PARAM_2;
   SpiTxBuf.dataBits = param[1];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send second parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = DOLBY_AUDIO_PROCESSING;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send Dolby Audio Processing Set up command
}

/**
 * Object Audio Renderer Options
 *
 * @param
 * @return     void
 */
void DSPDrv21584_ObjectAudioRendererOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = param[0];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_PARAM_2;
   SpiTxBuf.dataBits = param[1];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send second parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = OAR_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send Object Audio Renderer option command
}

/**
 * Customer Post Processing set up
 *
 * @param
 * @return     void
 */
void DSPDrv21584_CustomerPostProcessingOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = param[0];
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = CUSTOMER_PP;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send CUSTOMER_PP command
}

/**
 * MCPCM Downmix Options
 *
 * @param
 * @return     void
 */
void DSPDrv21584_MCPCMDownmixOpt(cDSPDrv21584 *me,uint16 *param)
{
   tSpiTxBuf SpiTxBuf;
   SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_WRITE;

   SpiTxBuf.cmdBufAddr = DSP_PARAM_1;
   SpiTxBuf.dataBits = *param;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf); 			//send first parameter

   SpiTxBuf.cmdBufAddr = DSP_COMMAND_REG;
   SpiTxBuf.dataBits = MCPCM_DOWNMIX_OPT;
   DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);			//send Atmos Mat PCM option command
}

/**
 * Reading all Status Information
 *
 * @param
 * @return     void
 */
void DSPDrv21584_GetAllStatusInfo(cDSPDrv21584 *me,uint16 *statusArray,uint8 StatusQty)
{
    tSpiTxBuf SpiTxBuf;
    tSpiRxBuf SpiRxBuf;
    SpiTxBuf.readWrite = HOST_TO_ADSP_COM_READ;
    SpiTxBuf.dataBits = 0x00;                   //Databits set to 0

    SpiTxBuf.cmdBufAddr = DSP_STATUS_1;
    DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);           //Send first status address, so this spi read have no status return

	for(uint32_t i = 1;i < StatusQty;i++)
    {
        SpiTxBuf.cmdBufAddr = DSP_STATUS_1 + i;
        DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);
        statusArray[i - 1] = SpiRxBuf.databits;
    }

    SpiTxBuf.cmdBufAddr = 0x00;                                                 //Get last status information, so no more status address need to be sent
    DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);
    statusArray[StatusQty - 1] = SpiRxBuf.databits;
}

/**
 * Reading Status Information
 *
 * @param
 * @return     void
 */
void DSPDrv21584_GetStatusInfo(cDSPDrv21584 *me,uint16 *status,uint16 statusAddr)
{
    tSpiTxBuf SpiTxBuf;
    tSpiRxBuf SpiRxBuf;
    SpiTxBuf.readWrite = HOST_TO_ADSP_COM_READ;
    SpiTxBuf.dataBits = 0x00;                   //Databits set to 0

    SpiTxBuf.cmdBufAddr = statusAddr;
    DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);

    SpiTxBuf.cmdBufAddr = 0x00;
    DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);
    *status = SpiRxBuf.databits;
}

void DSPDrv21584_ClearIntStatus(cDSPDrv21584 *me)         //Clear GPIO interrupt status
{
    me->AdspIntOccur = 0x00;
}

void DSPDrv21584_IRQHandler(cDSPDrv21584 *me)
{
    DSPDrv21584_InterDescRead(me);
}

void DSPDrv21584_InterDescRead(cDSPDrv21584 *me)
{
    if(me->AdspIntOccur == ADSP21584_GPIO_INT_RISING)
    {
        tSpiTxBuf SpiTxBuf;
        SpiTxBuf.readWrite =  HOST_TO_ADSP_COM_READ;

        SpiTxBuf.cmdBufAddr = DSP_INT_DESC;
        SpiTxBuf.dataBits = HOST_TO_ADSP_DUMMY_ZEROS;                   //This is just a read behavior, so dataBits set to dummy zeros
        DSPDrv21584_SpiWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiTxBuf);
    }
    else if(me->AdspIntOccur == ASDP21584_GPIO_INT_FALLING)
    {
        tSpiTxBuf SpiTxBuf;
        tSpiRxBuf SpiRxBuf;
        SpiTxBuf.readWrite = HOST_TO_ADSP_COM_READ;
        SpiTxBuf.cmdBufAddr = HOST_TO_ADSP_DUMMY_ZEROS;
        SpiTxBuf.dataBits = HOST_TO_ADSP_DUMMY_ZEROS;                   //This is just a read behavior, so dataBits set to dummy zeros
        DSPDrv21584_SpiReadWrite(me,sizeof(SpiTxBuf),(uint8 *)&SpiRxBuf,(uint8 *)&SpiTxBuf);
        me->interDescbits = SpiRxBuf.databits;
    }
    else
        TP_PRINTF("ADSP21584 GPIO interrupt status error\r\n");
    DSPDrv21584_ClearIntStatus(me);
}


void DSPDrv21584_UpdateStatus(cDSPDrv21584 *me)
{
    if(me->AdspIntOccur)
    {
        DSPDrv21584_IRQHandler(me);
    }
}

void DSPDrv21584_SetIntStatus(cDSPDrv21584 *me)         //Set GPIO interrupt status
{
    if(GpioDrv_ReadBit(me->gpioObj, GPIO_ADSP21584_INT))
    {
        me->AdspIntOccur = ADSP21584_GPIO_INT_RISING;
    }
    else
    {
        me->AdspIntOccur = ASDP21584_GPIO_INT_FALLING;
    }
}

/**
 * Select DSP input source according to channel
 * @param  -    channel
 */
void DSPDrv21584_SetInputChannel(cDSPDrv21584 *me, eAudioChannel input)
{
    TP_PRINTF("DSPDrv21584_SetInputChannel: input=%d\r\n", input);
    //DSPDrv21584_SpiWrite();     //need to be revised

}

/**
 * Sets the DSP volume, by writing the L/R channels PGA Gain registers. Please
 * reference on the volume table to check the real gain.
 *
 * @param      vol             The volume step that will be set
 * @return     void
 */
void DSPDrv21584_SetVol(cDSPDrv21584 *me, uint8 vol)
{
    TP_PRINTF("DSPDrv21584_SetVol: vol=%d \r\n", vol);
    //DSPDrv21584_SpiWrite();         //need to be revised
}

/**
 * Detect whether there is a input present
 *
 * @param
 * @return     bool    If the music is present either source depending on the DSP flow design
 */
bool DSPDrv21584_HasMusicStream(cDSPDrv21584 *me)
{
    bool    ret = FALSE;

    //DSPDrv21584_SpiRead();          //need to be revised
    return ret;
}

/**
 * Enter/exit low power sleep (hibernation) mode
 *
 * @param      void
 * @return     void
 */
void DSPDrv21584_SleepModeEnable(cDSPDrv21584 *me, bool enable)
{
    TP_PRINTF("DSPDrv21584_SleepModeEnable: enable=%d \r\n", enable);
    
    //DSPDrv21584_SpiWrite();         //need to be revised 
}


/**
 * Read DSP Version
 *
 * @param      N/A
 * @return     float float version
 */
float DSPDrv21584_GetDspVer(cDSPDrv21584 *me)
{
    /* Note DSP header file also include versoon:
     */
    ASSERT(me && me->isCreated);
      
    float ver= 0.0;    
    
      
    TP_PRINTF("\r\nDSP Version: %02.1f\r\n\r\n", ver);
    return ver;
}

static void DSPDrv21584_InitSpi(void *p)
{
    TP_PRINTF("DSPDrv21584_InitSpi\r\n");

    //Initialize SPI
    cDSPDrv21584* me = (cDSPDrv21584*)p;
    
    const tDevice * pDevice = NULL;
    pDevice = getDevicebyIdAndType(SPI2_DEV_ID, SPI_DEV_TYPE, NULL);
    ASSERT(pDevice);
    SpiDrv_Ctor(me->spiObj,(tSpiDevice *)pDevice);

    TP_PRINTF("DSPDrv21584_InitSpi finish\r\n");
}

static void DSPDrv21584_InitGpio(void *p)
{
    TP_PRINTF("DSPDrv21584_InitGpio\r\n");

    cDSPDrv21584* me = (cDSPDrv21584*)p;
    const tDevice * pDevice = NULL;
    pDevice = getDevicebyIdAndType(DSP_DECODER_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pDevice);
    GpioDrv_Ctor(me->gpioObj,(tGPIODevice *)pDevice);
    GpioDrv_EnableExtInterrupt(me->gpioObj, GPIO_ADSP21584_INT, ExtiIntTri_Rising_Falling);

    TP_PRINTF("DSPDrv21584_InitGpio finish\r\n");

}

/**
 * Write data to DSP by SPI.
 *
 * @param      
 *
 * @param      uint8           data byte number
 *
 * @param      const uint8*    pointer to data array
 *
 * @return     void
 */
static void DSPDrv21584_SpiWrite(cDSPDrv21584 *me, uint16 bytes, const uint8 *data)
{
    if(!me->spiEnable) {
        return;
    }
    bool ret= SpiDrv_WriteArray(me->spiObj,(uint8_t *)data ,bytes);
    ASSERT(ret==TP_SUCCESS);
}

static void DSPDrv21584_SpiRead(cDSPDrv21584 *me, uint16 bytes, const uint8 *data)
{
    if(!me->spiEnable) {
        return;
    }
    bool ret= SpiDrv_ReadArray(me->spiObj,(uint8_t *)data,bytes);
    ASSERT(ret==TP_SUCCESS);
}

static void DSPDrv21584_SpiReadWrite(cDSPDrv21584 *me, uint16 bytes, const uint8 *rxBuf, const uint8 *txBuf)
{
    if(!me->spiEnable) {
        return;
    }
    bool ret= SpiDrv_ReadWriteArray(me->spiObj,(uint8_t *)rxBuf,(uint8_t *)txBuf,bytes);
    ASSERT(ret==TP_SUCCESS);
}
