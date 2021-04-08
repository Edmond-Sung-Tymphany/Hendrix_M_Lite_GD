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



#define ADC_THRESHOLD_LEVEL_1 2000
#define ADC_THRESHOLD_LEVEL_2 2300

static const tAdcPinIoAttr adcPowerDetectPins[] =
{
    {ADC_NTC_POWER_DETECT,    ADC_CHANNEL9,  IO_PORT_B,  IO_BIT_1},  // adc 24v power detect
};

static const tADCDevice adcPowerDetectConfig =
{
    .deviceInfo.deviceID    = POWER_DETECT_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(adcPowerDetectPins),
    .pAdcPinIoAttr = adcPowerDetectPins
};

/* end of keyboard attached devices setting */

static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_12, IO_PORT_F, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP TAS5754 RESET, PF6
    {GPIO_13, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP PDN, PF7
    {GPIO_14, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},    // EN_+3V3 - PA6; probably set to  GPIO_ACTIVE_FLOATING
    {GPIO_15, IO_PORT_A, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},    // AUDIO_DET PA7
    {GPIO_17, IO_PORT_B, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //  ADC_3.3V_EN
    {GPIO_AMP_MUTE,  IO_PORT_A,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //Amplify mute
    {GPIO_IN_AUDIO_JACK_DET1,  IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},  //AUX In
  //  {GPIO_29, IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},// power key
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID = POWER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
    .pGPIOPinSet = gpioPinsForPower
};

static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_21,  IO_PORT_A,  IO_BIT_3,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //LED AUX2 - RCA
};

static const tGPIODevice gpioConfigForLed =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForLed),
    .pGPIOPinSet = gpioPinsForLed
};

static const tI2CDevice dspI2cConfig =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .devAddress = 0x98,
    .baudRate = 400 //KHz
};

static const tI2CDevice adcI2cConfig =
{
    .deviceInfo.deviceID = AUDIO_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .devAddress = 0x94,
    .baudRate = 400 //KHz
};

static const tStorageDevice nvmConfig =
{
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

static const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_IN_AUDIO_JACK_DET1,  IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},  //AUX In
    {GPIO_AMP_MUTE,  IO_PORT_A,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //Amplify mute
    {GPIO_IN_AUDIO_DETECT,  IO_PORT_A,  IO_BIT_4,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForAudio =
{
    .deviceInfo.deviceID    = AUDIO_ADC_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
    .pGPIOPinSet = gpioPinsForAudio
};
/********************************GPIO key configuration**************************/
static const tGPIOPin gpioKeyPins[] =
{
    {GPIO_15,  IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_29, IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},// power key

};
static const tGPIODevice GPIOKeyConfig =
{
    .usedGPIOPinNum =  ArraySize(gpioKeyPins),
    .pGPIOPinSet = gpioKeyPins
};

static tGpioKey gpioKeysSet[] =
{
    {CONFIG_KEY, GPIO_15},
    {POWER_KEY, GPIO_29},
};

static tGpioKeyboardDevice gpioKeyConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY, // == ArraySize(gpioKeysSet)
    .attachedDeviceObjConfig = &GPIOKeyConfig,
    .pGpioKeySet = gpioKeysSet
};
/********************************End of GPIO key configuration**************************/

/********************************Encoder key start**************************************/
static const tGPIOPin gpioEncoderKeyPins[] =
{

    {GPIO_22,  IO_PORT_B,  IO_BIT_4,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_23,  IO_PORT_B,  IO_BIT_5,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_19,  IO_PORT_A,  IO_BIT_8,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_20,  IO_PORT_A,  IO_BIT_9,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}


};
static const tGPIODevice GPIOEncoderKeyConfig =
{
    .usedGPIOPinNum =  ArraySize(gpioEncoderKeyPins),
    .pGPIOPinSet = gpioEncoderKeyPins
};

static tGpioEncoderKeyDevice encoderKeyConfig =
{
    .encoderKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .encoderKeyboard.deviceInfo.deviceType = GPIO_ENCODER_DEV_TYPE,
    .encoderKeyboard.keyNum = 1,
    .attachedDeviceObjConfig = &GPIOEncoderKeyConfig,
};
/********************************end of Encoder key************************************/

const tDevice * const devices[] =
{
    (tDevice*)&dspI2cConfig,
    (tDevice*)&adcI2cConfig,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&nvmConfig,
    (tDevice*)&gpioConfigForLed,
    (tDevice*)&encoderKeyConfig,
    (tDevice*)&gpioKeyConfig,
    (tDevice*)&GPIOConfigForAudio,
    (tDevice*)&adcPowerDetectConfig
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

void I2C1_LowLevel_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Configure the I2C clock source. The clock is derived from the HSI */
    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

    /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

    /*!< sEE_I2C Periph clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    /* Connect PXx to I2C_SCL*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1);

    /* Connect PXx to I2C_SDA*/
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1);

    /*!< GPIO configuration */
    /*!< Configure sEE_I2C pins: SCL */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C2_LowLevel_Init(void)
{
#ifdef HAS_I2C2
    GPIO_InitTypeDef  GPIO_InitStructure;

    /* Configure the I2C clock source. The clock is derived from the HSI */
    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

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
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
}


