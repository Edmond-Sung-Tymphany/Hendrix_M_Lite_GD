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
/* The mapping of LED ID to the corresponding Port-Pin in IO Expander */
static const tIoExpanderLedMap ioeLedMap[] =
{
    {LED_INT_BAT1,      0,  4},
    {LED_INT_BAT2,      0,  5},
    {LED_INT_BAT3,      0,  6},
    {LED_INT_BAT4,      0,  7},
    {LED_EXT_BAT1,      0,  3},
    {LED_EXT_BAT2,      0,  2},
    {LED_EXT_BAT3,      0,  1},
    {LED_EXT_BAT4,      0,  0},
    {LED_POWER_WHITE,   1,  7},
    {LED_POWER_RED,     1,  6},
    {LED_BT,            1,  5},
    {LED_EJECT,         1,  4},
    {LED_VOL_DOWN,      1,  3},
    {LED_VOL_UP,        1,  2},
    {LED_PLAY_WHITE,    1,  1},
    {LED_PLAY_GREEN,    1,  0},
};

static const tIoeLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .ledNum                 = NUM_OF_IOEXPANDER_LED,
    .i2cDevice              = DSP_DEV_ID,
    .pIoExpanderLedMap      = ioeLedMap,
};

static const tGPIOPin gpioPinsForKey[] =
{
    {GPIO_0, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_19, IO_PORT_A, IO_BIT_3,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    {GPIO_31, IO_PORT_A, IO_BIT_7,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  NUM_OF_GPIO_KEY_PIN,
  .pGPIOPinSet = gpioPinsForKey
};

static const tGPIOPin gpioPinsForBT[] =
{
#if SW_MAJOR_VERSION == 1
    {GPIO_1,  IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},    // CSR Codec Status
    {GPIO_21, IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_22, IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_23, IO_PORT_B,  IO_BIT_6,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_24, IO_PORT_B,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},   
#else
    {GPIO_1,  IO_PORT_B,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},    
    {GPIO_21, IO_PORT_B,  IO_BIT_6,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_22, IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_23, IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},       
    {GPIO_24, IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},    // // CSR Codec Status      
#endif        
    {GPIO_10, IO_PORT_B,  IO_BIT_11,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //BT_PWR_EN
    
    {GPIO_2,  IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},     
    {GPIO_25, IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},  
    {GPIO_30, IO_PORT_A,  IO_BIT_7,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},  

};

static const tGPIODevice GPIOConfigForBT =
{
  .deviceInfo.deviceID    = BT_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  NUM_OF_GPIO_BT_PIN,
  .pGPIOPinSet = gpioPinsForBT
};


static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_3,  IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SOL_ON_PIN 
    {GPIO_4,  IO_PORT_C,  IO_BIT_14,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SYS_PWR_ON
    {GPIO_5,  IO_PORT_C,  IO_BIT_15,  GPIO_DIGITAL_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //CHARGER STAT    
    {GPIO_6,  IO_PORT_F,  IO_BIT_0,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //BT_DSP_EXPANDER_RST_N
    {GPIO_8,  IO_PORT_A,  IO_BIT_6,   GPIO_DIGITAL_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //VDD_DET_IN  
    {GPIO_11, IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SOL_5V_EN
    {GPIO_12, IO_PORT_B,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //DC_USB_SEL    
    {GPIO_13, IO_PORT_A,  IO_BIT_8,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //AMP_9V_EN
    {GPIO_14, IO_PORT_A,  IO_BIT_11,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //EXT_5V_EN
    {GPIO_15, IO_PORT_A,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MINI_5V_EN_N
    {GPIO_16, IO_PORT_F,  IO_BIT_6,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //EXT_BAT_EN
    {GPIO_17, IO_PORT_F,  IO_BIT_7,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //INT_BAT_EN_N
    {GPIO_18, IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //MINI_U_DET_IN
    {GPIO_7,  IO_PORT_A,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //A_5V_CTL
    {GPIO_29, IO_PORT_B,  IO_BIT_1,  GPIO_DIGITAL_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //EXT_BAT_INT
    /* un-used pins below, set them to analog input to save power*/
    {GPIO_26, IO_PORT_F,  IO_BIT_1,   GPIO_ANALOG_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, 
    {GPIO_27, IO_PORT_A,  IO_BIT_0,   GPIO_ANALOG_INPUT,    GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, 
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID    = POWER_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  NUM_OF_GPIO_POWER_PIN,
  .pGPIOPinSet = gpioPinsForPower
};

static const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_19,  IO_PORT_B,  IO_BIT_2,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MUTE     
    {GPIO_20,  IO_PORT_B,  IO_BIT_10,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SDZ 
};

static const tGPIODevice gpioConfigForAudio =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  NUM_OF_GPIO_AUDIO_PIN,
  .pGPIOPinSet = gpioPinsForAudio
};

static const tAdcPinIoAttr attrADCPinsForKey[] =
{
  {ADC_PIN0, ADC_CHANNEL4,IO_PORT_A, IO_BIT_4},
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = NUM_OF_ADC_KEY_PIN,
    .pAdcPinIoAttr = attrADCPinsForKey
};

static const tAdcPinIoAttr attrADCPinsForPower[] = 
{
  {ADC_PIN2, ADC_CHANNEL5,IO_PORT_A, IO_BIT_5},
  {ADC_PIN3, ADC_CHANNEL6,IO_PORT_A, IO_BIT_6},
  {ADC_PIN4, ADC_CHANNEL8,IO_PORT_A, IO_BIT_0},
};

static const tADCDevice ADCConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = NUM_OF_ADC_POWER_PIN,
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
#define ADC_THRESHOLD_LEVEL_5   2647
#define ADC_THRESHOLD_RANGE_1   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_2   ADC_THRESHOLD_LEVEL_2,ADC_THRESHOLD_LEVEL_3
#define ADC_THRESHOLD_RANGE_3   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4
#define ADC_THRESHOLD_RANGE_4   ADC_THRESHOLD_LEVEL_4,ADC_THRESHOLD_LEVEL_5


static const tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {BT_KEY,              ADC_PIN0,   ADC_THRESHOLD_RANGE_1},
    {VOLUME_DOWN_KEY,     ADC_PIN0,   ADC_THRESHOLD_RANGE_2},
    {VOLUME_UP_KEY,       ADC_PIN0,   ADC_THRESHOLD_RANGE_3},
    {PLAY_PAUSE_KEY,      ADC_PIN0,   ADC_THRESHOLD_RANGE_4},    
};

/* 
 * adcKeyboardConfig should not be const since the element 
 * attachedDeviceObj will be assigned with a pointer when system is only the fly
*/
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
    {EJECT_KEY,         GPIO_19},
    {NFC_TAG_KEY,       GPIO_31},
};
/* 
 * gpioKeyboardConfig should not be const since the element 
 * attachedDeviceObj will be assigned with a pointer when system is only the fly
*/
static tGpioKeyboardDevice gpioKeyboardConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY,
    .attachedDeviceObjConfig = &GPIOConfigForKey,
    .pGpioKeySet = gpioKeys
};
/* end of keyboard attached devices setting */

static const tI2CDevice dsp_iHome_iBT150_Config =
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

static const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

const tDevice * const devices[] =
{
    (tDevice*)&ledConfig,
    (tDevice*)&adcKeyboardConfig,
    (tDevice*)&gpioKeyboardConfig,
    (tDevice*)&dsp_iHome_iBT150_Config,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&nvmConfig,
    (tDevice*)&ADCConfigForPower,
    (tDevice*)&gpioConfigForAudio,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&GPIOConfigForBT,
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

void sEE_LowLevel_Init()
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

