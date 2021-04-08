#ifndef DSPDRV_V2_H
#define DSPDRV_V2_H
#include "cplus.h"
#include "commonTypes.h"
#include "deviceTypes.h"
#include "SpiDrv.h"
#include "GpioDrv.h"



typedef enum
{
	START_PLAY              = 0x02,
	STOP_PLAY               = 0x03,
	MUTE_OUTPUT             = 0x04,
	RESUME_OUTPUT           = 0x05,
	SPORT_INIT              = 0x06,
	GENERAL_DECODE_OPT      = 0x10,
	DDPLUS_OPT              = 0x30,
	TRUEHD_OPT              = 0x32,
	ATMOS_MAT_PCM_OPT       = 0x33,
	MCPCM_DOWNMIX_OPT       = 0x35,
	OAR_OPT                 = 0x41,    //OAR means objet audio renderer
	DOLBY_AUDIO_PROCESSING  = 0x42,
	CUSTOMER_PP             = 0x43,
} eCmdMicroToDsp;

typedef enum
{
	DSP_COMMAND_REG            = 0X00,
	DSP_PARAM_1,
	DSP_PARAM_2,
	DSP_PARAM_3,
	DSP_PARAM_4,
	DSP_PARAM_5,
	DSP_PARAM_6,
	DSP_PARAM_7,
	DSP_PARAM_8,
	DSP_PARAM_9,
	DSP_PARAM_10,
	DSP_PARAM_11,
	DSP_PARAM_12,
	DSP_PARAM_13,
	DSP_PARAM_14,
	DSP_PARAM_15,
	DSP_PARAM_16,
	DSP_PARAM_17,
	DSP_PARAM_18,
	DSP_PARAM_19,
	DSP_PARAM_20,
	DSP_PARAM_21,
	DSP_PARAM_22,
	DSP_PARAM_23,
	DSP_PARAM_24,
	DSP_PARAM_25,
	DSP_PARAM_26,
	DSP_PARAM_27,
	DSP_PARAM_28,
	DSP_PARAM_29,
	DSP_PARAM_30,
	DSP_PARAM_31,
	DSP_PARAM_32,
	DSP_STATUS_1,
	DSP_STATUS_2,
	DSP_STATUS_3,
	DSP_STATUS_4,
	DSP_STATUS_5,
	DSP_STATUS_6,
	DSP_STATUS_7,
	DSP_STATUS_8,
	DSP_STATUS_9,
	DSP_STATUS_10,
	DSP_STATUS_11,
	DSP_STATUS_12,
	DSP_STATUS_13,
	DSP_STATUS_14,
	DSP_STATUS_15,
	DSP_STATUS_16,

	DSP_INT_DESC            = 0x3f,
} eCmdBufAddr;

typedef enum
{
    DEFAULT_CONTENT    = 0x00,
	INITIAL_DONE,
} eInitStatus;

typedef enum
{
    CMD_EXE_BUSY    = 0x01,
	CMD_EXE_SUCCE,
	CMD_EXE_ERRO,
} eCmdExeStatus;

typedef enum
{
    PATH_CODE_DOWNLOAD_NOT_ALLOWED = 0x00,
    PATH_CODE_DOWNLOAD_ALLOWED,
} ePathCode;

typedef enum
{
    STATUS_CHANGE_WITHIN_SAME_STREAM  = 0x01,
    STATUS_STREAM_BOTH_CHANGE = 0x03,
} eStatusStreamChange;


#define CheckInitStatus(x)            (x && 0x01)
#define CheckCmdExeStatus(x)          ((x && 0x03) >> 1)
#define CheckPathCodeStatus(x)        ((x && 0x08) >> 3)
#define CheckStatusStreamChange(x)    (x && 0x30)  >> 4)



#define ADSP21584_STATUS_WORD_COUNT     23
#define HOST_TO_ADSP_COM_READ           0x00
#define HOST_TO_ADSP_COM_WRITE          0x01
#define HOST_TO_ADSP_COM_READWRITE      0x02
#define HOST_TO_ADSP_DUMMY_ZEROS        0x00

#define ADSP21584_GPIO_INT_RISING       0x01
#define ASDP21584_GPIO_INT_FALLING      0x02

typedef struct tSpiTxBuf
{
    uint16   dataBits;
    uint8    cmdBufAddr;        //command buffer location
    uint8    readWrite;         //set to 0 means this command is a read behavior, otherwise is a arite behavior
} tSpiTxBuf;

typedef struct tSpiRxBuf
{
    uint16    databits;
    uint16    dummyBits;
} tSpiRxBuf;

CLASS(cDSPDrv21584)
    bool            isCreated:1;
    bool            isAdspInitDone:1;
    uint8           AdspIntOccur;                     //receive interrupt from adsp21584, zero means not receive int, 1 means receive a gpio rising int, 2 means receive a gpio falling int.
    uint16          interDescbits;                    //Use to hold interrupt description bits status
    tSpiTxBuf       SpiTxBuf;
    tSpiRxBuf       SpiRxBuf;
    cSpiDrv         *spiObj;  /* A spi obj should be ctored before ctor dspDrv*/
    cGpioDrv        *gpioObj;
    
    bool            spiEnable;
METHODS

#define AUTO_DETE_MODE_1                0x00
#define AUTO_DETE_MODE_2                0x01
#define AUTO_DETE_MODE_3                0x09
#define AUTO_DETE_MODE_FORCE_PCM_1      0x02
#define AUTO_DETE_MODE_FORCE_PCM_3      0x04

#define LPCM_SAMPLE_DELAY_2048          0x01
#define LPCM_SAMPLE_DELAY_4096          0x02
#define LPCM_SAMPLE_DELAY_16384         0x04

#define HIDR_0_0        0x00        //HIDR means high dynamic range scale factor
#define HIDR_0_1        0x01
#define HIDR_0_2        0x02
#define HIDR_0_3        0x03
#define HIDR_0_4        0x04
#define HIDR_0_5        0x05
#define HIDR_0_6        0x06
#define HIDR_0_7        0x07
#define HIDR_0_8        0x08
#define HIDR_0_9        0x09
#define HIDR_1_0        0x0A

#define LODR_0_0        0x00    //LODR means Low dynamic range scale factor
#define LODR_0_1        0x01
#define LODR_0_2        0x02
#define LODR_0_3        0x03
#define LODR_0_4        0x04
#define LODR_0_5        0x05
#define LODR_0_6        0x06
#define LODR_0_7        0x07
#define LODR_0_8        0x08
#define LODR_0_9        0x09
#define LODR_1_0        0x0A

#define INPUT_SAM_FREQ_32KHZ    0x00
#define INPUT_SAM_FREQ_44KHZ    0x01
#define INPUT_SAM_FREQ_48KHZ    0x02
#define INPUT_SAM_FREQ_64KHZ    0x04
#define INPUT_SAM_FREQ_88KHZ    0x05
#define INPUT_SAM_FREQ_96KHZ    0x06
#define INPUT_SAM_FREQ_128KHZ   0x08
#define INPUT_SAM_FREQ_176KHZ   0x09
#define INPUT_SAM_FREQ_192KHZ   0x0A

#define DOWN_SAM_MODE_FULL      0x00        //Down sampling mode
#define DOWN_SAM_MODE_HALF      0x01
#define DOWN_SAM_MODE_QUARTER   0x03

/**
 * Construct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv21584_Ctor(cDSPDrv21584* me, cSpiDrv *pSpiObj);

/**
 * Destruct the DSP driver instance.
 * @param      me - instance of the driver
 */
void DSPDrv21584_Xtor(cDSPDrv21584* me);



/**
 * Un-Mutes the Lineout
 *
 * @param      void
 * @return     void
 */
void DSPDrv21584_Mute(cDSPDrv21584 *me);

void DSPDrv21584_UnMute(cDSPDrv21584 *me);

void DSPDrv21584_StartPlay(cDSPDrv21584 *me);

void DSPDrv21584_StopPlay(cDSPDrv21584 *me);

void DSPDrv21584_SportInit(cDSPDrv21584 *me);

void DSPDrv21584_GenDecoOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_TrueHDOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_DDPlusOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_AtmosMatPcmOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_DolbyAudioProSetup(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_ObjectAudioRendererOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_CustomerPostProcessingOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_MCPCMDownmixOpt(cDSPDrv21584 *me,uint16 *param);

void DSPDrv21584_GetAllStatusInfo(cDSPDrv21584 *me,uint16 *statusArray,uint8 StatusQty);

void DSPDrv21584_GetStatusInfo(cDSPDrv21584 *me,uint16 *status,uint16 statusAddr);

void DSPDrv21584_ClearIntStatus(cDSPDrv21584 *me);

void DSPDrv21584_IRQHandler(cDSPDrv21584 *me);

void DSPDrv21584_UpdateStatus(cDSPDrv21584 *me);

void DSPDrv21584_SetIntStatus(cDSPDrv21584 *me);

void DSPDrv21584_InterDescRead(cDSPDrv21584 *me);





/**
 * Gets the status of the music streaming status
 *
 * @param      void
 * @return     bool         if there is music or not
 */
BOOL DSPDrv21584_HasMusicStream(cDSPDrv21584 *me);

/**
 * Select channel whether I2S or Analog
 * @param  -    channel
 */
void DSPDrv21584_SetInputChannel(cDSPDrv21584 *me, eAudioChannel inputChannel);

/**
 * Set volume
 * @param  -    vol
 */
void DSPDrv21584_SetVol(cDSPDrv21584 *me, uint8 vol);

bool DSPDrv21584_HasMusicStream(cDSPDrv21584 *me);

void DSPDrv21584_SleepModeEnable(cDSPDrv21584 *me, bool enable);

float DSPDrv21584_GetDspVer(cDSPDrv21584 *me);

static void DSPDrv21584_InitSpi(void *p);

static void DSPDrv21584_InitGpio(void *p);

static void DSPDrv21584_SpiWrite(cDSPDrv21584 *me, uint16 bytes, const uint8 *data);

static void DSPDrv21584_SpiRead(cDSPDrv21584 *me, uint16 bytes, const uint8 *data);

static void DSPDrv21584_SpiReadWrite(cDSPDrv21584 *me, uint16 bytes, const uint8 *rxBuf, const uint8 *txBuf);


END_CLASS

#endif /* DSPDRV__V2_H */
