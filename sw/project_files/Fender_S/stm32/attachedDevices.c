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
    {GPIO_IN_TALK_KEY,  IO_PORT_A,  IO_BIT_1,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // source
    {GPIO_IN_BT_KEY, IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // bt
};

static const tGPIODevice GPIOKeyConfig =
{
  .usedGPIOPinNum =  ArraySize(gpioKeyPins),
  .pGPIOPinSet = gpioKeyPins
};

static tGpioKey gpioKeysSet[] = 
{
    {TALK_KEY,      GPIO_IN_TALK_KEY},
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
    {ADC_PIN3, ADC_CHANNEL3,    IO_PORT_A, IO_BIT_3}, // PA3 - treble
    {ADC_PIN4, ADC_CHANNEL4,    IO_PORT_A, IO_BIT_4}, // PA4 - volume
    {ADC_PIN5, ADC_CHANNEL5,    IO_PORT_A, IO_BIT_5}, // PA5 - bass
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
};

#define ADC_THRESHOLD_RANGE    0, 4095
static const tLinearKnobKey linearKnobKeys[]=
{   /* key_id         key_ADC_channel  settingID, resolution  deltaBuffer adc threshold */
    {BASS_KNOB_BASE_KEY_ID,   ADC_PIN3, SETID_MAX, SETID_MAX, MAX_BASS_STEPS,    30,  ADC_THRESHOLD_RANGE},
    {TREBLE_KNOB_BASE_KEY_ID, ADC_PIN4, SETID_MAX, SETID_MAX, MAX_TREBLE_STEPS,  30,  ADC_THRESHOLD_RANGE},
    {VOLUME_KNOB_BASE_KEY_ID, ADC_PIN5, SETID_MAX, SETID_MAX, MAX_VOLUME_STEPS,  30,  ADC_THRESHOLD_RANGE},
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

/******************************** ADC Power configuration **************************/
static const tAdcPinIoAttr attrADCPinsForPower[] =
{
    {ADC_PIN6, ADC_CHANNEL6, IO_PORT_A, IO_BIT_6}, // BAT-DET
#ifdef HAS_BATTERY_NTC
    {ADC_PIN7, ADC_CHANNEL7, IO_PORT_A, IO_BIT_7}, // NTC
#endif
    {ADC_PIN8, ADC_CHANNEL8, IO_PORT_B, IO_BIT_0}, // DC-IN-DET
#ifdef HAS_HW_VERSION_TAG    
    {ADC_PIN9, ADC_CHANNEL9,IO_PORT_B, IO_BIT_1}, // HW version
#endif    
};

static const tADCDevice ADCConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = NUM_OF_ADC_POWER_PIN, //ArraySize(attrADCPinsForPower),
    .pAdcPinIoAttr = attrADCPinsForPower
};

/******************************** ADC Power configuration **************************/


/******************************** power control **************************/
static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_OUT_PWR_EN, IO_PORT_B, IO_BIT_10, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},
    {GPIO_IN_DC_IN_DET, IO_PORT_B, IO_BIT_0, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},  // PB0
    {GPIO_IN_POWER_KEY, IO_PORT_A,  IO_BIT_2, GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be
    {GPIO_OUT_DSP_3V3, IO_PORT_C, IO_BIT_13, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, //  DSP 3.3V : PC13
    {GPIO_OUT_EX_CHG_CTL, IO_PORT_C, IO_BIT_14, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, 
    {GPIO_OUT_BAT_CHG_EN, IO_PORT_C, IO_BIT_15, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE}, 
    {GPIO_IN_BAT_CHG_STATUS, IO_PORT_F, IO_BIT_6, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE}, 
    {GPIO_OUT_BOOST_EN, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING, ODC_DISABLE},
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
  .pGPIOPinSet = gpioPinsForPower
};
/******************************** power control **************************/
/******************************** audio control **************************/
static const tGPIOPin gpioPinsForAudio[] =
{
#ifdef HW_ES2
    {GPIO_IN_AUDIO_JACK_DET1, IO_PORT_B, IO_BIT_2, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
#else
    {GPIO_IN_AUDIO_JACK_DET1, IO_PORT_A, IO_BIT_7, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
#endif
    {GPIO_IN_DSP_TUNING, IO_PORT_B,  IO_BIT_11,  GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // PB11 dsp tuning detect
};

static const tGPIODevice GPIOConfigForAudio =
{
  .deviceInfo.deviceID = AUDIO_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
  .pGPIOPinSet = gpioPinsForAudio
};
/******************************** audio control **************************/

/******************************** CSR bt module pin **************************/
static const tGPIOPin gpioPinsForBT[] =
{
    {BT_INPUT0, IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB14   LED0
    {BT_INPUT1, IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB15   LED1
    {BT_OUTPUT1, IO_PORT_B,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB7
    {BT_OUTPUT2, IO_PORT_B,  IO_BIT_6,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB6
    {BT_OUTPUT3, IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB5
    {BT_OUTPUT4, IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB4
    {BT_OUTPUT5, IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB3
    {BT_PWR_EN, IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //BT_PWR_EN  PA15
    {BT_RESET_PIN, IO_PORT_A, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, // BT RESET, PA12
};

static const tGPIODevice GPIOConfigForBT =
{
  .deviceInfo.deviceID = BT_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForBT),
  .pGPIOPinSet = gpioPinsForBT
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
    .baudRate = 400,//KHz
    .devAddress = 0x70,
};
/******************************** DSP Adau1761 related **************************/

/******************************** Amplifier related **************************/
#ifdef HAS_SSM3582_AMP_HWI2C
static const tI2CDevice i2cSSM3582Config =
{
    .deviceInfo.deviceID = AMP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400, //KHz
    .devAddress = 0x20,
};
#endif
/******************************** Amplifier related **************************/

static const tUARTDevice UartDebugConfig = {
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

static const tStorageDevice nvmConfig = {
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

    // amplifier used 
#ifdef HAS_SSM3582_AMP_HWI2C
    (tDevice*)&i2cSSM3582Config,
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

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

        /* Configure USART2(BLE) pins:  Rx and Tx */
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
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
  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

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
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
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

void I2C1_GPIO_Deinit(void)
{
    GPIO_InitTypeDef	GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C1_GPIO_ReInit(void)
{
    GPIO_InitTypeDef	GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

