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



static const tAdcPinIoAttr attrADCPinsForKey[] =
{
    {ADC_PIN3, ADC_CHANNEL3,IO_PORT_A, IO_BIT_3}, // PA3 - volume
    {ADC_PIN4, ADC_CHANNEL4,IO_PORT_A, IO_BIT_4}, // PA4 - bass
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
};

#define ADC_THRESHOLD_RANGE    0, 2000
static const tLinearKnobKey linearKnobKeys[]=
{   /* key_id         key_ADC_channel  settingID, resolution  deltaBuffer adc threshold */
    {VOLUME_KNOB_BASE_KEY_ID, ADC_PIN3, SETID_VOLUME_ADC_MIN, SETID_VOLUME_ADC_MAX, 30,    30,  ADC_THRESHOLD_RANGE},
    {BASS_KNOB_BASE_KEY_ID,   ADC_PIN4, SETID_BASS_ADC_MIN,   SETID_BASS_ADC_MAX,    9,    30,  ADC_THRESHOLD_RANGE},
    {TREBLE_KNOB_BASE_KEY_ID, ADC_PIN5, SETID_TREBLE_ADC_MIN, SETID_TREBLE_ADC_MAX,  9,    30,  ADC_THRESHOLD_RANGE},
};
static tLinearKnobKeyDevice linearKnobKeyConfig = // const
{
    .linearKnobKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .linearKnobKeyboard.deviceInfo.deviceType = LINEAR_KNOB_DEV_TYPE,
    .linearKnobKeyboard.keyNum = NUM_OF_LINEAR_KNOB_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pLinearKnobKeySet = linearKnobKeys
};

/* end of keyboard attached devices setting */

static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_12, IO_PORT_F, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP TAS5754 RESET, PF6
    {GPIO_13, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP PDN, PF7
    {GPIO_14, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},    // EN_+3V3 - PA6; probably set to  GPIO_ACTIV

    {GPIO_16, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be

    {GPIO_17, IO_PORT_B, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //  ADC_3.3V_EN
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
  .pGPIOPinSet = gpioPinsForPower
};

/********************************GPIO key configuration**************************/
static const tGPIOPin gpioKeyPins[] =
{
    {GPIO_15,  IO_PORT_A,  IO_BIT_5,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, //power key
};
static const tGPIODevice GPIOKeyConfig =
{
  .usedGPIOPinNum =  ArraySize(gpioKeyPins),
  .pGPIOPinSet = gpioKeyPins
};

static tGpioKey gpioKeysSet[] = 
{
  {POWER_KEY, GPIO_15},
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

static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_21,  IO_PORT_B,  IO_BIT_0,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //LED AUX2 - RCA
    {GPIO_22,  IO_PORT_B,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //LED POWER ON GREEN
};

static const tGPIODevice gpioConfigForLed =
{
  .deviceInfo.deviceID    = LED_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForLed),
  .pGPIOPinSet = gpioPinsForLed
};

static const tI2CDevice dspI2cConfigTweeter =
{
    .deviceInfo.deviceID = AMP_TWEETER_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .devAddress = 0x98,
    .baudRate = 400 //KHz
};

static const tI2CDevice dspI2cConfigWoofer =
{
    .deviceInfo.deviceID = AMP_WOOFER_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .devAddress = 0x9a,
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
    (tDevice*)&dspI2cConfigTweeter,
    (tDevice*)&dspI2cConfigWoofer,
    (tDevice*)&adcI2cConfig,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&nvmConfig,
    (tDevice*)&linearKnobKeyConfig,
    (tDevice*)&gpioKeyConfig,
    (tDevice*)&gpioConfigForLed
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


