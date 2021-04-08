#include "trace.h"
#include "AudioDrv.h"
//#include "./AmpDrv_priv.h"
#include "attachedDevices.h"
#include "AudioDrv.config"
#include "gpioDrv.h"

/*GPIO object*/
static cGpioDrv gpioDrv;
/* will remove the extern with getDeviceId function after the GPIO struct is changed*/
extern const tGPIODevice GPIOConfigForAudio;

/* DC USB input Select, high: 9v, low: USB*/
#define     MUTE_PIN                     GPIO_19
#define     AMP_MUTE_HIGH(x)         GpioDrv_SetBit(&(x),MUTE_PIN)
#define     AMP_MUTE_LOW(x)          GpioDrv_ClearBit(&(x),MUTE_PIN)

#define     SDZ_PIN                     GPIO_20
#define     SDZ_PIN_ON(x)            GpioDrv_SetBit(&(x),SDZ_PIN)
#define     SDZ_PIN_OFF(x)           GpioDrv_ClearBit(&(x),SDZ_PIN)

void AudioDrv_Ctor()
{
    GpioDrv_Ctor(&gpioDrv,&GPIOConfigForAudio);
    AudioDrv_Mute(TRUE);
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

bool AudioDrv_IsLineInJackPluggedIn(void)
{
    return (0);
}


