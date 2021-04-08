
/// The max volume when system power is low
#define LOW_POWER_MAX_VOLUME 15

/// The default volume when system is boot up

#define TP_AUDIO_V2

#define AUDIO_ADC_AFTER_DSP

#define EXTERNAL_HIGH_SPEED_OSC_FREQ        (8000000)

#define HAS_PERIODIC_SERVER_TASK

//#define BSP_TICKS_PER_SEC
#define HAS_AUDIO_CONTROL
#define HAS_KEYS
#define HAS_LINEAR_ADC_KNOB_KEY
#define HAS_GPIO_KEY
#define KEY_SRV_HAS_DEBUG
#define HAS_COMB_KEY

#define HAS_DEBUG

#define HAS_SETTING

#define HAS_HW_VERSION_TAG

///define the number of UART system is using
#define NUM_OF_UART     1

#define NUM_OF_SMALL_EVENTS         10
#define SIZE_OF_SMALL_EVENTS        8

#define NUM_OF_MEDIUM_EVENTS        16
#define SIZE_OF_MEDIUM_EVENTS       16

#define NUM_OF_LARGE_EVENTS         16
#define SIZE_OF_LARGE_EVENTS        32

#define AUDIO_SRV_EVENT_Q_SIZE      10
#define SETT_SRV_EVENT_Q_SIZE       6
#define DBG_SRV_EVENT_Q_SIZE        10
#define KEY_SRV_EVENT_Q_SIZE        10

#define BRINGUP_printf		printf
#define ALWAYS_printf		printf

#define HAS_GPIO_LED

#define HAS_ADAU1761_DSP

