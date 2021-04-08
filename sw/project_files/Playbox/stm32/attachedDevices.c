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
    {GPIO_0, IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // VOL_UP key, PC13
};

static const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  ArraySize(gpioPinsForKey),
  .pGPIOPinSet = gpioPinsForKey
};

static const tGPIOPin gpioPinsForBT[] =
{
     {GPIO_1, IO_PORT_B,  IO_BIT_0,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB0
     {GPIO_2, IO_PORT_B,  IO_BIT_1,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB1

     {GPIO_3, IO_PORT_A,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PA12
     {GPIO_4, IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PA15
     {GPIO_5, IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB3
     {GPIO_10, IO_PORT_A, IO_BIT_11,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //BT_PWR_EN  PA11
};

static const tGPIODevice GPIOConfigForBT =
{
  .deviceInfo.deviceID = BT_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForBT),
  .pGPIOPinSet = gpioPinsForBT
};


static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_6, IO_PORT_A, IO_BIT_4, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},   //VDC_DET_IN, PA4
    {GPIO_7, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //BUCK_ON, PA6
    {GPIO_8, IO_PORT_A, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //USB_ON, PA7
    {GPIO_9, IO_PORT_F, IO_BIT_0, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE},   //TPS2546_STATUS PF0 TODO: check if GPIO_ACTIVE_HIGH
    {GPIO_11,  IO_PORT_A,  IO_BIT_8,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},  //Reset BT& DSP PA8
    {GPIO_12,  IO_PORT_F,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //P_ON System Power
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
    {GPIO_19,  IO_PORT_A,  IO_BIT_0,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MUTE  PA0
    {GPIO_20,  IO_PORT_A,  IO_BIT_1, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SDZ,  PA1
};

static const tGPIODevice gpioConfigForAudio =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
  .pGPIOPinSet = gpioPinsForAudio
};

static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_21,  IO_PORT_B,  IO_BIT_4,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //LEDR
    {GPIO_22,  IO_PORT_B,  IO_BIT_5, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //LEDB
};

static const tGPIODevice gpioConfigForLed =
{
  .deviceInfo.deviceID    = LED_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForLed),
  .pGPIOPinSet = gpioPinsForLed
};



static const tAdcPinIoAttr attrADCPinsForKey[] =
{
    {ADC_PIN3, ADC_CHANNEL3,IO_PORT_A, IO_BIT_3}, // PA3
};
static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
};

static const tAdcPinIoAttr attrADCPinsForPower[] =
{
    {ADC_PIN5, ADC_CHANNEL5,IO_PORT_A, IO_BIT_5},  // VBAT_DET  PA5
    {ADC_PIN4, ADC_CHANNEL4,IO_PORT_A, IO_BIT_4},  // VDC_DET  PA4

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
#define ADC_THRESHOLD_LEVEL_1   265
#define ADC_THRESHOLD_LEVEL_2   875
#define ADC_THRESHOLD_LEVEL_3   1501
#define ADC_THRESHOLD_LEVEL_4   2060
#define ADC_THRESHOLD_RANGE_1   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_2   ADC_THRESHOLD_LEVEL_2,ADC_THRESHOLD_LEVEL_3
#define ADC_THRESHOLD_RANGE_3   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4


static const tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {BT_KEY,              ADC_PIN3,   ADC_THRESHOLD_RANGE_1},
    {VOLUME_UP_KEY,     ADC_PIN3,   ADC_THRESHOLD_RANGE_2},
    {VOLUME_DOWN_KEY,       ADC_PIN3,   ADC_THRESHOLD_RANGE_3},
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
    {VOLUME_UP_KEY,         GPIO_0},
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
//    (tDevice*)&adcKeyboardConfig,
    (tDevice*)&gpioKeyboardConfig,
//    (tDevice*)&dsp_MGT_Config,
//    (tDevice*)&GPIOConfigForBT,
//    (tDevice*)&ADCConfigForPower,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&gpioConfigForAudio,
//    (tDevice*)&gpioConfigForLed,
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


