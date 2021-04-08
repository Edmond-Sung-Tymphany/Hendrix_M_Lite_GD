#include "trace.h"
#include "attachedDevices.h"
#include "gpioDrv.h"
#include "AdcDrv.h"
#include "AudioDrv.h"
#include "audioDrv.config"


#ifndef AUDIO_DRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif


/* GPIO object */
static cGpioDrv gpioDrv;
#ifdef HAS_SPDIF_SIGNAL_SENSE
static cGpioDrv gpioAudio;
#endif 
/* ADC object */
#ifdef HAS_ANALOG_SIGNAL_SENSE
static cADCDrv adcAnalogSignal;
#endif

/* DC USB input Select, high: 9v, low: USB*/
#define     MUTE_PIN                     GPIO_19
#define     AMP_MUTE_HIGH(x)         GpioDrv_SetBit(&(x),MUTE_PIN)
#define     AMP_MUTE_LOW(x)          GpioDrv_ClearBit(&(x),MUTE_PIN)

#define     SDZ_PIN                     GPIO_20
#define     SDZ_PIN_ON(x)            GpioDrv_SetBit(&(x),SDZ_PIN)
#define     SDZ_PIN_OFF(x)           GpioDrv_ClearBit(&(x),SDZ_PIN)


#define     SPDIF1_PIN(x)                GpioDrv_ReadBit(&(x), GPIO_22)
#define     SPDIF0_PIN(x)                GpioDrv_ReadBit(&(x), GPIO_21)




void AudioDrv_Ctor()
{
    uint16 dev_index=0;
// The sequence of these three devices in "AttachedDevices.c" needed to be in order.
#ifdef HAS_AMPLIFY_MUTE
    tGPIODevice *pAudioGPIOConf;
    pAudioGPIOConf = (tGPIODevice*)getDevicebyId(AUDIO_DEV_ID,&dev_index);
    GpioDrv_Ctor(&gpioDrv,pAudioGPIOConf);
    dev_index++;
#endif
#ifdef HAS_SPDIF_SIGNAL_SENSE
    tGPIODevice* pAudioGPIOConf;
    pAudioGPIOConf = (tGPIODevice*)getDevicebyId(AUDIO_DEV_ID,&dev_index);
    GpioDrv_Ctor(&gpioAudio,pAudioGPIOConf);
    for (uint8 i=0;i<NUM_OF_SPDIF_CHANNEL;i++)
    {
        SpdifObj[i].ringBuf.pAvgBuff= &SpdifObj[i].ringBuf.AvgBuff[0];
        SpdifObj[i].gpio = &gpioAudio;
        for(uint8 j=0; j<BUFF_SIZE; j++)
        {
            SpdifObj[i].ringBuf.AvgBuff[j] = SPDIF_SENSE_INVALID_LOWER;
        }
    }
    dev_index++;
#endif
#ifdef HAS_ANALOG_SIGNAL_SENSE
    tADCDevice* pAudioAdcConf;
    pAudioAdcConf = (tADCDevice*)getDevicebyId(AUDIO_DEV_ID,&dev_index);
    ADCDrv_Ctor(&adcAnalogSignal, pAudioAdcConf);
    for (uint8 i=0;i<NUM_OF_ANALOG_CHANNEL;i++)
    {
       AnalogObj[i].ringBuf.pAvgBuff = &AnalogObj[i].ringBuf.AvgBuff[0];
       AnalogObj[i].adc = &adcAnalogSignal;
    }
#endif
}

void AudioDrv_Mute(bool mute)
{
    if (mute)
    {
        SDZ_PIN_OFF(gpioDrv);
        AMP_MUTE_HIGH(gpioDrv);
    }
    else
    {
        SDZ_PIN_ON(gpioDrv);
        AMP_MUTE_LOW(gpioDrv);
    }
}

#ifdef HAS_AUDIO_IO_MONITOR
bool AudioDrv_IsLineInJackPluggedIn(void)
{
    return (IsLineInPluggedIn());  // low active
}

bool AudioDrv_IsSpdifInJackPluggedIn(void)
{
    return (IsSpdif0InPluggedIn());
}

bool AudioDrv_IsSpdif1InJackPluggedIn(void)
{
    return (IsSpdif1InPluggedIn());
}

bool AudioDrv_IsLineInMusicDetected(void)
{
    return (IsLineInMusicDetected() && IsLineInPluggedIn());
}

bool AudioDrv_IsSpdif0InMusicDetected(void)
{
    return (IsSpdif0MusicDetected());
}

bool AudioDrv_IsSpdif1MusicDetected(void)
{
    return (IsSpdif1MusicDetected() && IsSpdif1InPluggedIn());
}


#endif

#ifdef HAS_RJ45_INPUT
bool AudioDrv_IsRJ45InJackPluggedIn(void)
{
    return (IsRJ45InPluggedIn());
}

bool AudioDrv_IsRj45_MusicDetected(void)
{
    return (IsRJ45MusicDetected() && IsRJ45InPluggedIn());
}
#endif



#if defined(HAS_SPDIF_SIGNAL_SENSE)||defined(HAS_ANALOG_SIGNAL_SENSE)

static uint16 AudioDrv_TakeTotal(tRingBuf* RingBufObj, uint16 pushData, uint8 BuffSize)
{
    uint16 sum=0;
    *(RingBufObj->pAvgBuff) = pushData;
    RingBufObj->pAvgBuff++;
    if (RingBufObj->pAvgBuff>(&(RingBufObj->AvgBuff[BuffSize-1])))
    {
        RingBufObj->pAvgBuff = &(RingBufObj->AvgBuff[0]);
    }
    for (uint8 i=0; i<BuffSize; i++)
    {
        sum+=RingBufObj->AvgBuff[i];
    }
    return (sum);
}


/** \brief
*   push a new data to AvgBuff[] of SpdifObj and take a average value
*   return the average value
static uint16 AudioDrv_TakeAverage(tRingBuf* RingBufObj, uint16 pushData, uint8 BuffSize)
{
    uint16 sum=AudioDrv_TakeTotal(RingBufObj, pushData, BuffSize);
    return (sum/BuffSize);
    
}

*/


/** \brief
*   check if the valueToCheck is within a range.
*   return bool
*/
static bool WithinRange(uint16 valueToCheck, uint16 lowerRange, uint16 UpperRange)
{
    return (valueToCheck>lowerRange && valueToCheck<UpperRange);
}
#endif

#ifdef HAS_ANALOG_SIGNAL_SENSE
/** \brief
*   check if the valueToCheck is within a range.
*   return bool
*/

static uint32 AudioDrv_CaptureAnalogData(tAnalogObj* AnaObj)
{
    uint16 delta, total_delta;
    static uint16 AvgDataNew=0;

    if (ADCDrv_IsConversionDone(&adcAnalogSignal))
    {
        AvgDataNew=ADCDrv_GetData(&adcAnalogSignal, AnaObj->pin);
        //TP_PRINTF("AvgDataNew=%d\r\n",AvgDataNew);
        ADCDrv_StartScanning(&adcAnalogSignal);
    }
    delta = (AvgDataNew>AnaObj->rawData)?(AvgDataNew - AnaObj->rawData):(AnaObj->rawData - AvgDataNew);
    total_delta = AudioDrv_TakeTotal(&(AnaObj->ringBuf), delta, BUFF_SIZE)*10;
    if (AnaObj->sample_count++>=SPDIF_IN_SAMPLES_BUFFER_SIZE/10)
    {
        //TP_PRINTF("analog(%d) = %d\r\n", AnaObj->pin,total_delta);
        AnaObj->sample_count = 0;
        //if (((bool)AnaObj->isHasMusic) != (total_delta>ANALOG_SENSE_VALID))
        //{
            //TP_PRINTF("AnalogObj->isHasMusic(%d) = %d\r\n", AnaObj->pin, (total_delta>ANALOG_SENSE_VALID));
        //}
        AnaObj->isHasMusic = (total_delta>ANALOG_SENSE_VALID);
    }
    AnaObj->rawData= AvgDataNew;
    return (AvgDataNew);
}


uint16 AudioDrv_AnalogReading(void)
{
    static uint8 i=0;
    uint32 retVal = AudioDrv_CaptureAnalogData(&AnalogObj[i]);;
    i++;
    if (i>=ArraySize(AnalogObj))
    {
        i=0;
    }
    return retVal;
}
#endif


#ifdef HAS_SPDIF_SIGNAL_SENSE
/** \brief Capture the SPDIF signal to determine if it has a music content
 *      This function should be called every 2ms other SPDIF_SENSE_VALID_COUNT_MIN and SPDIF_SENSE_VALID_COUNT_MAX need to be adjusted.
 *      return a captured averaged SPDIF pulse change count.
*/
static uint32 AudioDrv_CaptureOpticalData(tSpdifObj* SpdifObj)
{
        
    int16 spdifLevel1 = GpioDrv_ReadBit(SpdifObj->gpio, SpdifObj->pin);
    MICRO_DELAY
    asm("nop");asm("nop");asm("nop");asm("nop");//asm("nop");
    int16 spdifLevel2 = GpioDrv_ReadBit(SpdifObj->gpio, SpdifObj->pin);
	SpdifObj->digital_data += (spdifLevel1!=spdifLevel2);

    if (SpdifObj->sample_count++>=SPDIF_IN_SAMPLES_BUFFER_SIZE)
    {
        SpdifObj->sample_count=0;
        //TP_PRINTF("(%d) = %d\r\n", SpdifObj->pin,SpdifObj->digital_data);
        SpdifObj->isHasMusic = (WithinRange(SpdifObj->digital_data, SPDIF_SENSE_VALID_COUNT_MIN, SPDIF_SENSE_VALID_COUNT_MAX)||WithinRange(SpdifObj->digital_data, SPDIF_SENSE_DOLBY_VALID_COUNT_MIN, SPDIF_SENSE_DOLBY_VALID_COUNT_MAX));
        //TP_PRINTF("SpdifObj->isHasMusic(%d) = %d\r\n", SpdifObj->pin,SpdifObj->isHasMusic);
        SpdifObj->isCableConnected = (SpdifObj->digital_data>SPDIF_HAVE_SIGNAL_COUNT);
        SpdifObj->digital_data = 0;	
        return (SpdifObj->digital_data);    
    }   
    return (SpdifObj->digital_data);    
}    


uint16 AudioDrv_OpticalReading(void)
{
  uint32 retVal;
    for (uint8 i=0; i<ArraySize(SpdifObj);i++)
    {
      retVal= AudioDrv_CaptureOpticalData(&SpdifObj[i]);
    }
    return (retVal);
}

#endif
