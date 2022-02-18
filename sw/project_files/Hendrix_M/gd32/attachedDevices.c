/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Christopher Alexander, Wesley Lee
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

#include "stm32f0xx_usart.h"
#include "Setting_id.h"
#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"

#include "product.config"

/******************************** GPIO key configuration **************************/
static const tGPIOPin gpioKeyPins[] =
{
    {GPIO_IN_BT_KEY,  IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // only one key as BT key
};

static const tGPIODevice GPIOKeyConfig =
{
    .usedGPIOPinNum =  ArraySize(gpioKeyPins),
    .pGPIOPinSet = gpioKeyPins
};

static tGpioKey gpioKeysSet[] =
{
    {BT_KEY,         GPIO_IN_BT_KEY},
};

static tGpioKeyboardDevice gpioKeyConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY, // == ArraySize(gpioKeysSet)
    .attachedDeviceObjConfig = &GPIOKeyConfig,
    .pGpioKeySet = gpioKeysSet
};
/******************************** GPIO key configuration **************************/


/******************************** ADC key configuration **************************/
static const tAdcPinIoAttr attrADCPinsForKey[] =
{
    {ADC_PIN3, ADC_CHANNEL3,    IO_PORT_A, IO_BIT_3}, // PA3 - volume
    {ADC_PIN4, ADC_CHANNEL4,    IO_PORT_A, IO_BIT_4}, // PA4 - bass
    {ADC_PIN5, ADC_CHANNEL5,    IO_PORT_A, IO_BIT_5}, // PA5 - treble
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
};

#define ADC_THRESHOLD_RANGE    0, 4095
static const tLinearKnobKey linearKnobKeys[]=
{
    /* key_id         key_ADC_channel  settingID, resolution  deltaBuffer adc threshold */
    {VOLUME_KNOB_BASE_KEY_ID, ADC_PIN3, SETID_MAX, SETID_MAX, MAX_VOLUME_STEPS,    30,  ADC_THRESHOLD_RANGE},
    {BASS_KNOB_BASE_KEY_ID , ADC_PIN4, SETID_MAX, SETID_MAX, MAX_BASS_STEPS,  30,  ADC_THRESHOLD_RANGE},
    {TREBLE_KNOB_BASE_KEY_ID, ADC_PIN5, SETID_MAX, SETID_MAX, MAX_TREBLE_STEPS,  30,  ADC_THRESHOLD_RANGE},
};

static tLinearKnobKeyDevice linearKnobKeyConfig = // const
{
    .linearKnobKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .linearKnobKeyboard.deviceInfo.deviceType = LINEAR_KNOB_DEV_TYPE,
    .linearKnobKeyboard.keyNum = NUM_OF_LINEAR_KNOB_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pLinearKnobKeySet = linearKnobKeys
};
/******************************** ADC key configuration **************************/

/******************************** Main Board IO expander configuration **************************/
static const tIoExpanderLedMap ioeMbMap[] =
{
    //  port(0:A, 1:B)    pin
    // Power Io Expander port define
    {IOE_OUT_ICHG_SEL, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_A, IOE_BIT_4},  // P0_4
    {IOE_OUT_BAT_CHG_EN, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_B, IOE_BIT_2}, // P1_2
    {IOE_OUT_AMP_ON, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_A, IOE_BIT_1},     // P0_1
    {IOE_OUT_BOOST_EN, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_A, IOE_BIT_0},   // P0_0

    // Audio Io Expander port define
    {IOE_IN_WF_FAULTZ, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_B, IOE_BIT_7},
    {IOE_IN_TW_FAULT, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_B, IOE_BIT_6},
    {IOE_OUT_TW_MUTE, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_A, IOE_BIT_3},
    {IOE_OUT_WF_MUTE, COLOR_COMPONENT_BRIGHTNESS, IOE_PORT_A, IOE_BIT_2},

};
static /*const */tI2CDevice  ioeMbI2cDeviceConf =
{
    .deviceInfo.deviceID= IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_TWO,  // I2C2
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = I2C2_CLK_KHZ,
    .devAddress         = 0xB6,//0xB4,
};

//port A bit: 0:boost 1:AMPon 2:WFMUTE 3:TWMUTE 4:CHGcurrent 567:x
//
//port B bit: 01x 2:BatChg 3:x 4:Ctl3 5:Ctl2 6:TWFau 7:WFFau


static tIoExpanderConfig ioeMbConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x08,
    .ledModePortB = 0x00,
    .outPutPortA  = 0x04,
    .outPutPortB  = 0xD0,
    .ioDirectionA  = 0xFF,//bit:0-7 output
    .ioDirectionB  = 0x3F,//bit:6-7 input
    .controlValue = (0x10 | 0x03),

};

static const tGPIOPin ioeMbRstPin[] =
{
    {GPIO_OUT_IOEXP_RST1,  IO_PORT_B, IO_BIT_31, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //IO-NRST-1  PB6
};

static tGPIODevice ioeMbRstConfig =
{
    .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(ioeMbRstPin),
    .pGPIOPinSet = ioeMbRstPin
};

static tIoeLedDevice ioeGpioMbConfig =
{
    .deviceInfo.deviceID    = IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeMbMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeMbMap,
    .i2cDeviceConf          = &ioeMbI2cDeviceConf,
    .pIoExpanderConfig      = &ioeMbConfig,
    .pResetGpioDevice       = &ioeMbRstConfig,
};

// for sleep special setting just to initial IO Expander
static tIoExpanderConfig ioeMbSleepConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0xFF,
    .ledModePortB = 0xFF,
    .outPutPortA  = 0x00,
    .outPutPortB  = 0x00,
    .ioDirectionA  = 0xFF,//bit:0-7 output
    .ioDirectionB  = 0xFF,//bit:6-7 input
    .controlValue = (0x10 | 0x03),

};

tIoeLedDevice ioeGpioMbSleepConfig =
{
    .deviceInfo.deviceID    = IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeMbMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeMbMap,
    .i2cDeviceConf          = &ioeMbI2cDeviceConf,
    .pIoExpanderConfig      = &ioeMbSleepConfig,
    .pResetGpioDevice       = &ioeMbRstConfig,
};

/******************************** Main Board IO expander configuration **************************/




/******************************** ADC Power configuration **************************/
static const tAdcPinIoAttr attrADCPinsForPower[] =
{
#ifdef HAS_BATTERY_DETECT
    {BATTERY_ADC_PIN , ADC_CHANNEL6, IO_PORT_A, IO_BIT_6}, // BAT-DET
#endif
#ifdef HAS_BATTERY_NTC
    {BATTERY_NTC_PIN, ADC_CHANNEL7, IO_PORT_A, IO_BIT_7}, // NTC
#endif
#ifdef HAS_DC_IN
    {DC_IN_ADC_PIN, ADC_CHANNEL8, IO_PORT_B, IO_BIT_0}, // DC-IN-DET
#endif
#ifdef HAS_HW_VERSION_TAG
    {HW_VERSION_ADC_PIN, ADC_CHANNEL9,IO_PORT_B, IO_BIT_1}, // HW version
#endif
};

static const tADCDevice ADCConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrADCPinsForPower),
    .pAdcPinIoAttr = attrADCPinsForPower
};

/******************************** ADC Power configuration **************************/


/******************************** power control **************************/
static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_OUT_PWR_EN, IO_PORT_A, IO_BIT_0, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},        // PA0

    {GPIO_IN_DC_IN_DET, IO_PORT_B, IO_BIT_0, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},  // PB0
    {GPIO_IN_POWER_KEY, IO_PORT_A,  IO_BIT_2, GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be
    {GPIO_OUT_DSP_3V3, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, //  DSP 3.3V : PF7
    {GPIO_OUT_AMP_ON, IO_PORT_A, IO_BIT_8, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  //AMP ON :PA8 //Nick: Controls the AMP_PDN# -> PWDNN
    {GPIO_OUT_WF_MUTE, IO_PORT_B, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  //WF MUTE :PB7
    {GPIO_OUT_TW_PWDNN, IO_PORT_B, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  //TF PWDNN :PB6 //Nick: Controls TW-MUTE
    {GPIO_OUT_TW_FAULT, IO_PORT_F, IO_BIT_0, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},    //TF FAULT :PF0
    {GPIO_OUT_WF_FAULT, IO_PORT_C, IO_BIT_13, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},   //WF FAULT :PC13
    {GPIO_OUT_BOOST_EN, IO_PORT_F, IO_BIT_1, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  //BOOST EN :PF1
    //{GPIO_OUT_EX_CHG_CTL, IO_PORT_C, IO_BIT_14, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  // at IO expandar
    //{GPIO_OUT_BAT_CHG_EN, IO_PORT_C, IO_BIT_15, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, // at IO expandar
    {GPIO_IN_BAT_CHG_STATUS1, IO_PORT_C, IO_BIT_14, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
    {GPIO_IN_BAT_CHG_STATUS2, IO_PORT_C, IO_BIT_15, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
    {GPIO_OUT_BAT_CHG_EN, IO_PORT_A, IO_BIT_1, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},  //CHG_BAT_ENABLE :PA1
    //{GPIO_IN_BAT_PG, IO_PORT_B, IO_BIT_7, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},  //  PB7

#ifndef HAS_MCO
    //{GPIO_RESRVE_1, IO_PORT_F, IO_BIT_0, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
    //{GPIO_RESRVE_3, IO_PORT_A, IO_BIT_8, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
#endif
    //{GPIO_OUT_BOOST_EN, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, //boost_en at the IO expandar
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID = POWER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
    .pGPIOPinSet = gpioPinsForPower
};


static const tVIoExpanderGpioMap vIoePwrMap[] =
{
    //  port(0:A, 1:B)    pin
    {IOE_OUT_ICHG_SEL},  // P0_4
    {IOE_OUT_BAT_CHG_EN}, // P1_2
    {IOE_OUT_AMP_ON},     // P0_1
    {IOE_OUT_BOOST_EN},   // P0_0
};

static const tVIoeGpioDevice vIoeGpioPwrConfig =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .chipID = IoeChip0,
    .gpioNum = ArraySize(vIoePwrMap),
    .pVIoExpanderGpioMap = (tVIoExpanderGpioMap*)vIoePwrMap,
    .pIoeDevice = &ioeGpioMbConfig,
};
/******************************** power control **************************/


/****************************************
 * Device: LED                         *
 ****************************************/
static const tIoExpanderLedMap ioeLedMap[] =
{
    //  port(0:A, 1:B)    pin
    {LED_PWR_R,    COLOR_COMPONENT_RED,      1,      5}, // P1_5
    {LED_PWR_G,    COLOR_COMPONENT_GREEN,    1,      7}, // P1_7
    {LED_PWR_B,    COLOR_COMPONENT_BLUE,     1,      6}, // P1_6
#ifdef REVERSE_BATT_LED
    {LED_BAT_1,     COLOR_COMPONENT_RED,      1,      1}, // P1_4
    {LED_BAT_2,     COLOR_COMPONENT_RED,      1,      2}, // P0_7
    {LED_BAT_3,     COLOR_COMPONENT_RED,      1,      3}, // P0_6
    {LED_BAT_4,     COLOR_COMPONENT_RED,      0,      0}, // P0_5
    {LED_BAT_5,     COLOR_COMPONENT_RED,      0,      1}, // P0_4
    {LED_BAT_6,     COLOR_COMPONENT_RED,      0,      4}, // P0_1
    {LED_BAT_7,     COLOR_COMPONENT_RED,      0,      5}, // P0_0
    {LED_BAT_8,     COLOR_COMPONENT_RED,      0,      6}, // P1_3
    {LED_BAT_9,     COLOR_COMPONENT_RED,      0,      7}, // P1_2
    {LED_BAT_10,     COLOR_COMPONENT_RED,     1,      4}, // P1_1
#else
    {LED_BAT_1,     COLOR_COMPONENT_RED,      1,      4}, // P1_4
    {LED_BAT_2,     COLOR_COMPONENT_RED,      0,      7}, // P0_7
    {LED_BAT_3,     COLOR_COMPONENT_RED,      0,      6}, // P0_6
    {LED_BAT_4,     COLOR_COMPONENT_RED,      0,      5}, // P0_5
    {LED_BAT_5,     COLOR_COMPONENT_RED,      0,      4}, // P0_4
    {LED_BAT_6,     COLOR_COMPONENT_RED,      0,      1}, // P0_1
    {LED_BAT_7,     COLOR_COMPONENT_RED,      0,      0}, // P0_0
    {LED_BAT_8,     COLOR_COMPONENT_RED,      1,      3}, // P1_3
    {LED_BAT_9,     COLOR_COMPONENT_RED,      1,      2}, // P1_2
    {LED_BAT_10,     COLOR_COMPONENT_RED,     1,      1}, // P1_1
#endif
};

static /*const */tI2CDevice  ioeI2cDeviceConf =
{
    .deviceInfo.deviceID= IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_TWO,  // I2C2
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = I2C2_CLK_KHZ,
    .devAddress         = 0xB6,
};

#ifdef HENDRIX_Lite
static tIoExpanderConfig ioeLedConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x08,
    .ledModePortB = 0x00,
    .outPutPortA  = 0x08,
    .outPutPortB  = 0x00,
    .controlValue = (0x10 | 0x03),
};
#endif

#ifdef HENDRIX_L
static tIoExpanderConfig ioeLedConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x08,
    .ledModePortB = 0x00,
    .outPutPortA  = 0x08,
    .outPutPortB  = 0x00,
    .controlValue = (0x10 | 0x03),
};
#endif
#ifdef HENDRIX_M
static tIoExpanderConfig ioeLedConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x00,
    .ledModePortB = 0x00,
    .outPutPortA  = 0x00,
    .outPutPortB  = 0x00,
    .controlValue = (0x10 | 0x03),
};
#endif
static const tGPIOPin ioeRstPin[] =
{
    {GPIO_OUT_IOEXP_RST2,  IO_PORT_B, IO_BIT_2, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //IO_NRST-2  PB2
};

static tGPIODevice ioeRstConfig =
{
    .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(ioeRstPin),
    .pGPIOPinSet = ioeRstPin
};

static const tIoeLedDevice ledConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeLedMap,
    .i2cDeviceConf          = &ioeI2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedConfig,
    .pResetGpioDevice       = &ioeRstConfig,
};

// for sleep setting for LED IO expander
static tIoExpanderConfig ioeLedSleepConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x0C,
    .ledModePortB = 0x01,
    .outPutPortA  = 0x00,
    .outPutPortB  = 0x00,
    .ioDirectionA  = 0xFF,//bit:0-7 output
    .ioDirectionB  = 0xFF,//bit:6-7 input
    .controlValue = (0x10 | 0x03),
};

tIoeLedDevice ledSleepConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeLedMap,
    .i2cDeviceConf          = &ioeI2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedSleepConfig,
    .pResetGpioDevice       = &ioeRstConfig,
};



/*************************Device: ioexpandar GPIO  ********************/

/******************************** audio control **************************/
static const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_IN_DSP_TUNING, IO_PORT_B,  IO_BIT_13,  GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // PB13 dsp tuning detect
};

static const tGPIODevice GPIOConfigForAudio =
{
    .deviceInfo.deviceID = AUDIO_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
    .pGPIOPinSet = gpioPinsForAudio
};

static const tVIoExpanderGpioMap vIoeAudioMap[] =
{
    //  port(0:A, 1:B)    pin
    {IOE_IN_WF_FAULTZ},
    {IOE_IN_TW_FAULT},
    {IOE_OUT_TW_MUTE},
    {IOE_OUT_WF_MUTE},
};

static const tVIoeGpioDevice vIoeGpioAudioConfig =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .chipID = IoeChip0,
    .gpioNum = ArraySize(vIoeAudioMap),
    .pVIoExpanderGpioMap = (tVIoExpanderGpioMap*)vIoeAudioMap,
    .pIoeDevice = &ioeGpioMbConfig,
};

/******************************** audio control **************************/

/******************************** CSR bt module pin **************************/
static const tGPIOPin gpioPinsForBT[] =
{
    {BT_INPUT0, IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB14    LED0  BT-GOIO0
    {BT_INPUT1, IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB15    LED1  BT-GOIO1
    {BT_INPUT2, IO_PORT_A,  IO_BIT_11,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PA11   LED2  BT-GOIO2
    {BT_OUTPUT1, IO_PORT_F,  IO_BIT_6,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},  // PF6  VREG_ENABLE    BT_PWR_EN
    {BT_OUTPUT2, IO_PORT_A, IO_BIT_15,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, // PA15  PIO1  BT_UART_RX
    {BT_OUTPUT3, IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB5   PIO8   B2M-GPIO3
    {BT_OUTPUT4, IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB4   PIO21  B2M-GPIO4
    {BT_OUTPUT5, IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB3   PIO18  B2M-GPIO5
    //{BT_RESET_PIN, IO_PORT_A, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, // BT RESET, PA12
    {BT_RESET_PIN, IO_PORT_A, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW, ODC_DISABLE}, // BT RESET, PA12 //Nick modify to initial low to follow Joplin's setting
};

static const tGPIODevice GPIOConfigForBT =
{
    .deviceInfo.deviceID = BT_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForBT),
    .pGPIOPinSet = gpioPinsForBT,
};
/******************************** CSR bt module pin **************************/

/******************************** DSP Adau1761 related **************************/
static const tI2CDevice adau1761Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = I2C1_CLK_KHZ,//KHz
    .devAddress = 0x70,
};
/******************************** DSP Adau1761 related **************************/

/******************************** Amplifier related **************************/

#ifdef HAS_TAS5760_AMP
static const tI2CDevice i2cTAS5760Config =
{
    .deviceInfo.deviceID = AMP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C1_CLK_KHZ, //KHz
    .devAddress = 0xD8,
};
#endif

#ifdef HAS_RT9120S_AMP //Nick++
static const tI2CDevice i2cRT9120SConfig =
{
    .deviceInfo.deviceID = AMP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C1_CLK_KHZ, //KHz
    .devAddress = 0x32, //0011001 0
};
#endif
/******************************** Amplifier related **************************/

static const tUARTDevice UartDebugConfig =
{
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 38400,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = QF_AWARE_ISR_CMSIS_PRI + 1,     /* priority can be 0..3 from highest to lowest */
    //.interrupt.subPriority = QF_AWARE_ISR_CMSIS_PRI + 1,     // not-used, for compatibility with PIC32
};

static const tStorageDevice nvmConfig =
{
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

const tDevice * const devices[] =
{
    (tDevice*)&gpioKeyConfig,
    (tDevice*)&linearKnobKeyConfig,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&GPIOConfigForAudio,
    (tDevice*)&adau1761Config,
    (tDevice*)&ADCConfigForPower,
    (tDevice*)&GPIOConfigForBT,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&nvmConfig,
    (tDevice*)&ledConfig,
    (tDevice*)&ioeGpioMbConfig,
#ifdef HAS_PWR_IO_EXPANDER
    (tDevice*)&vIoeGpioPwrConfig,
#endif

#ifdef HAS_AUDIO_IO_EXPANDER
    (tDevice*)&vIoeGpioAudioConfig,
#endif

    // amplifier used
#ifdef HAS_SSM3582_AMP_HWI2C
    (tDevice*)&i2cSSM3582Config,
#endif

#ifdef HAS_TAS5760_AMP
    (tDevice*)&i2cTAS5760Config,
#endif

#ifdef HAS_RT9120S_AMP //Nick++
    (tDevice*)&i2cRT9120SConfig,
#endif
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

void UartDrv_Init(eTpUartDevice id)
{
    switch(id)
    {
        case TP_UART_DEV_1:
        {
            GPIO_InitTypeDef GPIO_InitStructure;

            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);

            GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

            /* Configure USART1(Debug) pins:  Rx and Tx */
            GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10;
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
        }
        break;
        case TP_UART_DEV_2:
        {
            GPIO_InitTypeDef GPIO_InitStructure;

            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

            GPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_1);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_1);

            /* Configure USART2(BLE) pins:  Rx and Tx */
            GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_14 | GPIO_Pin_15;
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
        }
        break;
        default:
            ASSERT(0);
            break;
    }
}

void UartDrv_Deinit(eTpUartDevice id)
{
    // TODO: to be implemented
}

void I2C1_LowLevel_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Configure the I2C clock source. The clock is derived from the HSI */
    //RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);        //no need for GD32F130?

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_1);

    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_1);

    /*!< GPIO configuration */
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);*/
}

void I2C2_LowLevel_Init(void)
{
#ifdef HAS_I2C2
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Configure the I2C clock source. The clock is derived from the HSI */
    //RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);  //no need for GD32F130?

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1);

    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1);
    /*!< GPIO configuration */
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);*/
#endif
}

void I2C1_GPIO_Deinit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  /*!< sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);

 // I2C_DeInit(I2C1);
  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}

void I2C1_GPIO_ReInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  /*!< sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, DISABLE);

  //I2C_DeInit(I2C1);
  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

}

//#ifdef HAS_MCO
void MCO_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_0);

    /* Configure PA8 as MCO output */
    GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //RCC->CFGR &= (uint32_t)(~RCC_CFGR_MCO);
    //RCC->CFGR |= (uint32_t)(/*RCC_CFGR_PLLNODIV |*/ RCC_CFGR_MCO_1 | RCC_CFGR_MCO_HSE);
    RCC->CFGR &= ~((0x07 << 24) | (0x07 << 28) | (0x01 << 31));
    RCC->CFGR |= (0x04 << 24) | (0x0 << 28);
}

//#endif

