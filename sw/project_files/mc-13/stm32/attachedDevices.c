/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Christopher Alexander, Wesley Lee
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

#include "stm32f0xx_usart.h"

#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"

#include "product.config"

/* @warning
 * This struct cannot be defined in deviceTypes.h as the enum eLed cannot be forward declared
 * enum eLed should be defined in attachedDevices.h as it can be different among projects
 * the workaround is forward declared tIoExpanderLed in deviceTypes.h
 */

static const tGPIOPin gpioPinsForKey[] =
{
    {GPIO_0,  IO_PORT_A,  IO_BIT_1,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, //POWER KEY
};

static const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  ArraySize(gpioPinsForKey),
  .pGPIOPinSet = gpioPinsForKey
};
#if defined(HW_VERSION_ES2)
static const tGPIOPin gpioPinsForBT[] =
{
     {GPIO_1,  IO_PORT_A,  IO_BIT_12,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
     {GPIO_2,  IO_PORT_A,  IO_BIT_11,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},

     {GPIO_3,  IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_4,  IO_PORT_A,  IO_BIT_9,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_5,  IO_PORT_A,  IO_BIT_10,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_10, IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //BT_POWER
};
#else if defined(HW_VERSION_EVT)
static const tGPIOPin gpioPinsForBT[] =
{
     {GPIO_1,  IO_PORT_A,  IO_BIT_12,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
     {GPIO_2,  IO_PORT_A,  IO_BIT_11,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},

     {GPIO_3,  IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_4,  IO_PORT_A,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_5,  IO_PORT_A,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
     {GPIO_10, IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //BT_POWER
};
#endif

static const tGPIODevice GPIOConfigForBT =
{
  .deviceInfo.deviceID = BT_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForBT),
  .pGPIOPinSet = gpioPinsForBT
};

static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_12,  IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},  //SYS_POWER
    {GPIO_11,  IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //BT_RESET
    {GPIO_10,  IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},  //BT_POWER
    {GPIO_26,  IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},    //BAT CHARGE STATUS
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
  .pGPIOPinSet = gpioPinsForPower
};

static const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_8,   IO_PORT_B,  IO_BIT_6, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},
    {GPIO_13,  IO_PORT_B,  IO_BIT_7, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},
    {GPIO_20,  IO_PORT_F,  IO_BIT_0,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SDZ
    {GPIO_19,  IO_PORT_F,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MUTE
};

static const tGPIODevice gpioConfigForAudio =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
  .pGPIOPinSet = gpioPinsForAudio
};

#if defined(HW_VERSION_ES2)
static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_14,  IO_PORT_A,  IO_BIT_6, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //RED
    {GPIO_16,  IO_PORT_A,  IO_BIT_7, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //GREEN
    {GPIO_17,  IO_PORT_A,  IO_BIT_5, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //BLUE
    {GPIO_18,  IO_PORT_A,  IO_BIT_4, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //WHITE
};
#else if defined(HW_VERSION_EVT)
static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_14,  IO_PORT_A,  IO_BIT_6, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //RED
    {GPIO_16,  IO_PORT_A,  IO_BIT_7, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //GREEN
    {GPIO_17,  IO_PORT_B,  IO_BIT_5, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //BLUE
    {GPIO_18,  IO_PORT_B,  IO_BIT_0, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //WHITE
};
#endif

static const tGPIODevice gpioConfigForLed =
{
  .deviceInfo.deviceID    = LED_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForLed),
  .pGPIOPinSet = gpioPinsForLed
};

static const tAdcPinIoAttr attrADCPinsForKey[] =
{
    {ADC_PIN1, ADC_CHANNEL2,IO_PORT_A, IO_BIT_2}, // ADC KEYS
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
};

static const tAdcPinIoAttr attrADCPinsForPower[] =
{
    {ADC_PIN2, ADC_CHANNEL3,IO_PORT_A, IO_BIT_3}, //BAT VOLTAGE
    {ADC_PIN3, ADC_CHANNEL9,IO_PORT_B, IO_BIT_1}, //5V
};

static const tADCDevice ADCConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrADCPinsForPower),
    .pAdcPinIoAttr = attrADCPinsForPower
};

/**
 * keyboard object setting
 * The ADC threshold defind in our document is not correct(theoretical value),
 * It has too large difference with the real value. Below is the theoretical
 * adc threshold table and the real threshold  table
 * Theoretical threshold table (defined in our project specification):
        key_id       key_ADC_channel   adc_max_value  adc_max_value
        {VOLUME_DOWN_KEY,   ADC_CHANNEL0,   {(512-15),  (512+15)}},
        {VOLUME_UP_KEY,     ADC_CHANNEL0,   {(410-13),  (410+13)}},
        {VOL_UP_DOWN_KEY,   ADC_CHANNEL0,   {(640-18),  (640+18)}},
        {NEXT_PREV_KEY,     ADC_CHANNEL1,   {(600-17),  (600+17)}},
        {NEXT_KEY,          ADC_CHANNEL1,   {(300-10),  (300+10)}},
        {PLAY_PAUSE_KEY,    ADC_CHANNEL1,   {(410-13),  (408+13)}},
        {PREV_KEY,          ADC_CHANNEL1,   {(512-15),  (512+15)}}
 * The real threshold table is based on Ruben's idea, the adc range is enlarged
 * to handle specail case such as hardware bugs. Aparrently, the range is
 * continuous, this enables the software to, at least, "do something" if there
 * is hardware defeat which lead to an uncommon adc value. It is difficult
 * to detect the bug if the software just igonore this uncommon adc value.
 */
#define ADC_THRESHOLD_LEVEL_1   0
#define ADC_THRESHOLD_LEVEL_2   372//300
#define ADC_THRESHOLD_LEVEL_3   1117//900
#define ADC_THRESHOLD_LEVEL_4   1738//1400
#define ADC_THRESHOLD_LEVEL_5   2669//2150
#define ADC_THRESHOLD_LEVEL_6   3351//2700

#define ADC_THRESHOLD_RANGE_1   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_2   ADC_THRESHOLD_LEVEL_2,ADC_THRESHOLD_LEVEL_3
#define ADC_THRESHOLD_RANGE_3   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4
#define ADC_THRESHOLD_RANGE_4   ADC_THRESHOLD_LEVEL_4,ADC_THRESHOLD_LEVEL_5
#define ADC_THRESHOLD_RANGE_5   ADC_THRESHOLD_LEVEL_5,ADC_THRESHOLD_LEVEL_6

static const tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {BT_KEY,          ADC_PIN1,   ADC_THRESHOLD_RANGE_1},
    {INPUT_KEY,       ADC_PIN1,   ADC_THRESHOLD_RANGE_2},
    {PLAY_PAUSE_KEY,  ADC_PIN1,   ADC_THRESHOLD_RANGE_3},
    {VOLUME_DOWN_KEY, ADC_PIN1,   ADC_THRESHOLD_RANGE_4},
    {VOLUME_UP_KEY,   ADC_PIN1,   ADC_THRESHOLD_RANGE_5},
};
static tAdcKeyboardDevice adcKeyboardConfig = // const
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = NUM_OF_ADC_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pAdcKeySet = adcKeys
};

static const tGpioKey gpioKeys[] =
{   /* key_id           */
    {POWER_KEY,         GPIO_0},
};
static tGpioKeyboardDevice gpioKeyboardConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY,
    .attachedDeviceObjConfig = &GPIOConfigForKey,
    .pGpioKeySet = gpioKeys
};
/* end of keyboard attached devices setting */

static const tI2CDevice dsp_MC13_Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .address = 0x30, /* device address */
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400 //KHz
};

#ifdef HAS_DEBUG
static const tUARTDevice UartDebugConfig = {
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};
#endif

const tDevice * const devices[] =
{
    (tDevice*)&adcKeyboardConfig,
    (tDevice*)&gpioKeyboardConfig,
    (tDevice*)&dsp_MC13_Config,
    (tDevice*)&GPIOConfigForBT,
    (tDevice*)&ADCConfigForPower,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&gpioConfigForAudio,
    (tDevice*)&gpioConfigForLed,
#ifdef HAS_DEBUG
    (tDevice*)&UartDebugConfig,
#endif
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

void UartDrv_Init(eTpUartDevice id)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    switch(id)
    {
#if defined(HW_VERSION_ES2)
        case TP_UART_DEV_1:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
            /* Configure the HSI as USART clock */
            RCC_USARTCLKConfig(RCC_USART1CLK_HSI);

            GPIO_PinAFConfig(GPIOA, GPIO_PinSource4, GPIO_AF_1);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_1);

            /* Configure USART1 pins:  Rx and Tx ----------------------------*/
            GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4 | GPIO_Pin_5;
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            break;
#else if defined(HW_VERSION_EVT)
        case TP_UART_DEV_1:
            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
            RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
            /* Configure the HSI as USART clock */
            RCC_USARTCLKConfig(RCC_USART1CLK_HSI);

            GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

            /* Configure USART1 pins:  Rx and Tx ----------------------------*/
            GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10;
            GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
            GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
            GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
            GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
            GPIO_Init(GPIOA, &GPIO_InitStructure);
            break;
#endif
        default:
        ASSERT(0);
        break;
    }

}

void UartDrv_Deinit(eTpUartDevice id)
{
  // TODO: to be implemented
}

void sEE_LowLevel_Init(void)
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
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
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


