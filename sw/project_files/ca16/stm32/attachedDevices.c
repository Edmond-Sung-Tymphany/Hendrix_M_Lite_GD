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


/****************************************
 * Device: Keys                         *
 ****************************************/
static const tGPIOPin gpioKeyPins[] =
{
    {GPIO_IN_STB_SW,        IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Power key, PC13
    {GPIO_IN_CONF_SW,       IO_PORT_C,  IO_BIT_14,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Config key, PC14
    {GPIO_IN_CONN_SW,       IO_PORT_B,  IO_BIT_4,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Connect key, PB4
    {GPIO_IN_VOL_UP_SW,     IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Connect key, PB3
    {GPIO_IN_VOL_DOWN_SW,   IO_PORT_D,  IO_BIT_2,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Connect key, PD2
};
static const tGPIODevice GPIOKeyConfig =
{
  .usedGPIOPinNum =  ArraySize(gpioKeyPins),
  .pGPIOPinSet = gpioKeyPins
};

#ifdef PT_I2C_DEBUG
static const tI2CDevice i2cDbgConfig1 =
{
    .deviceInfo.deviceID = DBG_I2C1_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C2
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400, //KHz
};
static const tI2CDevice i2cDbgConfig2 =
{
    .deviceInfo.deviceID = DBG_I2C2_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400, //KHz
};
#endif
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
 */
static const tGpioKey gpioKeysSet[] =
{   /* key_id           */
    {STANDBY_KEY,          GPIO_IN_STB_SW},
    {CONFIG_KEY,           GPIO_IN_CONF_SW},
    {CONNECT_KEY,          GPIO_IN_CONN_SW},
    {VOLUME_UP_KEY,        GPIO_IN_VOL_UP_SW},
    {VOLUME_DOWN_KEY,      GPIO_IN_VOL_DOWN_SW},
};

static tGpioKeyboardDevice gpioKeyConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY, // == ArraySize(gpioKeysSet)
    .attachedDeviceObjConfig = &GPIOKeyConfig,
    .pGpioKeySet = gpioKeysSet
};



/****************************************
 * Device: Power                        *
 ****************************************/
static const tAdcPinIoAttr attrAdcAsetkPins[] =
{
    {ADC_HW_VER,  ADC_CHANNEL14,   IO_PORT_C, IO_BIT_4},  // FEP Hardware version PC4
};

static const tADCDevice adcPowerConfig =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrAdcAsetkPins),
    .pAdcPinIoAttr = attrAdcAsetkPins
};


/****************************************
 * Device: ioexpander reset ctrl         *
 ****************************************/
static const tGPIOPin ioexpanderRstPin[] =
{
    {GPIO_OUT_IOEXP_RST,  IO_PORT_B, IO_BIT_12, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //IO_EXPANDER_RST  PB12
};

static const tGPIODevice ioexpanderRstConfig =
{
  .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(ioexpanderRstPin),
  .pGPIOPinSet = ioexpanderRstPin
};

/****************************************
 * Device: DSP                          *
 ****************************************/
static const tGPIOPin gpioDspPins[] =
{
    {GPIO_IN_DSP_TUNE,     IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,       ODC_NOT_APPLICABLE}, //PB0
    {GPIO_OUT_DSP_RST_N,   IO_PORT_B, IO_BIT_2,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},  //PB2    
};
static const tGPIODevice gpioDspConfig =
{
  .deviceInfo.deviceID = DSP_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioDspPins),
  .pGPIOPinSet = gpioDspPins
};


static const tI2CDevice i2cDspConfig =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400, //KHz
    .devAddress = 0x70,
};



/****************************************
 * Device: AMP TAS5760                  *
 ****************************************/
static const tI2CDevice i2cAmpMidConfig =
{
    .deviceInfo.deviceID = AMP_MIDDLE_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400, //KHz
    .devAddress = 0xD8,
};

static const tI2CDevice i2cAmpWooferConfig =
{
    .deviceInfo.deviceID = AMP_WOOFER_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400, //KHz
    .devAddress = 0xDA,
};




/****************************************
 * Device: Audio                        *
 ****************************************/
static const tGPIOPin gpioAudioPins[] =
{
    {GPIO_IN_AUDIO_JACK_DET1,   IO_PORT_A,  IO_BIT_0,   GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,         ODC_NOT_APPLICABLE}, //AUDIO_JACK_DET1,  PA0
    {GPIO_IN_AUDIO_JACK_DET2,   IO_PORT_B,  IO_BIT_5,   GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,         ODC_NOT_APPLICABLE}, //AUDIO_JACK_DET2,  PB5
    {GPIO_OUT_SYSPWR_ON,        IO_PORT_B,  IO_BIT_0,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},   //PB0
    {GPIO_IN_AMP_FAULTZ,        IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,         ODC_NOT_APPLICABLE},  //PB14. SPK_FAULT is open drain, MCU need to week pull low
    {GPIO_OUT_AMP_SDZ,          IO_PORT_B,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE}, //PB13
};
static const tGPIODevice gpioAudioConfig =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioAudioPins),
  .pGPIOPinSet = gpioAudioPins
};

static const tAdcPinIoAttr attrAdcAudioPins[] =
{
    {ADC_AMP1_NTC,  ADC_CHANNEL5,   IO_PORT_A, IO_BIT_5},  // Amplifer 1 NTC  PA4
    {ADC_AMP2_NTC,  ADC_CHANNEL4,   IO_PORT_A, IO_BIT_4},  // Amplifer 2 NTC  PA5
    {ADC_WF_NTC,    ADC_CHANNEL12,  IO_PORT_C, IO_BIT_2},  // Woofer NTC  PC2
    {ADC_TW_NTC,    ADC_CHANNEL13,  IO_PORT_C, IO_BIT_3},  // Twerter NTC  PC3
};

static const tADCDevice adcAudioConfig =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrAdcAudioPins),
    .pAdcPinIoAttr = attrAdcAudioPins
};


/****************************************
 * Device: ASE-TK                       *
 ****************************************/
static const tGPIOPin gpioAsetkPins[] =
{
    /* GPIO_OUT_BOOT_STATUS_SW should defaultly pull low, because
     *   Normal condition: pull low
     *   Before jump to ST-BL: pull high
     */
    {GPIO_OUT_BOOT_STATUS_SW,   IO_PORT_B,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB8
    {GPIO_OUT_ASE_RST_N,        IO_PORT_A,  IO_BIT_11, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //PA11
    {GPIO_OUT_ASE_SYS_EN,       IO_PORT_A,  IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //PA12
    {GPIO_IN_ASE_REQ_PDN,       IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,          ODC_NOT_APPLICABLE},  //PA8 
};
static const tGPIODevice gpioAsetkConfig =
{
  .deviceInfo.deviceID    = ASETK_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioAsetkPins),
  .pGPIOPinSet = gpioAsetkPins
};


static const tUARTDevice uartAsetkConfig = {
    .deviceInfo.deviceID    = ASETK_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1, //PA9, PA10
    .byteSize   = TP_UART_BYTE_SIZE_9_BITS,
    .parity     = TP_UART_PARITY_EVEN,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};



/****************************************
 * Device: LED                         *
 ****************************************/
static const tIoExpanderLedMap ioeLedMap[] =
{
    {LED0_RED,      COLOR_COMPONENT_RED,    0,   0},
    {LED1_GREEN,    COLOR_COMPONENT_GREEN,  0,   1},
    {LED2_BLUE,     COLOR_COMPONENT_BLUE,   0,   2},
    {LED3_RED,      COLOR_COMPONENT_RED,    0,   3},
    {LED4_GREEN,    COLOR_COMPONENT_GREEN,  0,   4},
    {LED5_BLUE,     COLOR_COMPONENT_BLUE,   0,   5},
};

static const tI2CDevice  Ledi2cDeviceConf =
{
    .deviceInfo.deviceID= LED_DEV_ID,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,  // I2C1
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = 400,      //KHz
    .devAddress         = 0xB6,
};

static const tIoeLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = ioeLedMap,
    .i2cDeviceConf = &Ledi2cDeviceConf,
};




/****************************************
 * Device: ADC: PCM1862                 *
 ****************************************/
static const tI2CDevice i2cAdcConfig =
{
    .deviceInfo.deviceID = AUDIO_ADC_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C1
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400, //KHz
    .devAddress = 0x94, //MD1 is hardware pull low, thus I2C address is 0x94
};



/****************************************
 * Device: Debug                        *
 ****************************************/
#ifdef HAS_DEBUG
static const tUARTDevice uartDebugConfig = {
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_2, //PA2, PA3
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};
#endif




/****************************************
 * Device: NVM                          *
 ****************************************/
const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};




/****************************************
 * Device list                          *
 ****************************************/
const tDevice * const devices[] =
{
    //ASE-TK
    (tDevice*)&uartAsetkConfig,
    (tDevice*)&gpioAsetkConfig,

    //KEY
    (tDevice*)&gpioKeyConfig,

    //POWER
    (tDevice*)&adcPowerConfig,
    
    //Audio
    (tDevice*)&gpioAudioConfig,
    (tDevice*)&adcAudioConfig,
    (tDevice*)&gpioDspConfig,
    (tDevice*)&i2cDspConfig,
    (tDevice*)&i2cAmpMidConfig,
    (tDevice*)&i2cAmpWooferConfig,
    (tDevice*)&i2cAdcConfig,

    //LED
    (tDevice*)&ledConfig,
    
    //Other
    (tDevice*)&nvmConfig,
    (tDevice*)&ioexpanderRstConfig,
#ifdef PT_I2C_DEBUG
    (tDevice*)&i2cDbgConfig1,
    (tDevice*)&i2cDbgConfig2,
#endif
    
    //UART
#ifdef HAS_DEBUG
    (tDevice*)&uartDebugConfig,
#endif
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

USART_TypeDef *gUartMap[] =
{
    USART1,
    USART2,
    USART3,
};

const uint8_t gUartNvicIrq[] =
{
    USART1_IRQn,
    USART2_IRQn,
    USART3_8_IRQn,
};


/****************************************
 * Low level API                        *
 ****************************************/
void UartDrv_Init(eTpUartDevice id)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    switch(id)
    {
    case TP_UART_DEV_1: //UART1 for ASE-TK
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
        /* Configure the HSI as USART clock */
#if defined(UART_CLK_SOURCE_PCLK)
        RCC_USARTCLKConfig(RCC_USART1CLK_PCLK);
#elif defined(UART_CLK_SOURCE_HSI)
        RCC_USARTCLKConfig(RCC_USART1CLK_HSI);
#endif

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1); //PA9
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1); //PA10

        /* Configure USART1 pins:  Rx and Tx ----------------------------*/
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    case TP_UART_DEV_2: //UART2 for Debug
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
        /* Configure the HSI as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART2SW;
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1); //PA2, TX
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1); //PA3, RX

        /* Configure USART2 pins:  Rx and Tx ----------------------------*/
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    case TP_UART_DEV_3:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART3EN,ENABLE);

        /* Configure the HSI as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_1);

        /* Configure USART3 pins:  Rx and Tx ----------------------------*/
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_10 | GPIO_Pin_11;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        break;
    default:
        ASSERT(0);
        break;
    }

}

void UartDrv_Deinit(eTpUartDevice id)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    switch(id)
    {
    case TP_UART_DEV_1: //UART1 for ASE-TK
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    case TP_UART_DEV_2: //UART2 for Debug
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    default:
        ASSERT(0);
        break;
    }
}

void I2C1_LowLevel_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure the I2C clock source. The clock is derived from the HSI */
  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

  /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  /*!< sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_1); //PB6

  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_1); //PB7
  
  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure sEE_I2C pins: SDA */
  //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  //GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//I2C2 for DSP
void I2C2_LowLevel_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure the I2C clock source. The clock is derived from the HSI */
  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

  /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

  /*!< sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_1); //PB10

  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_1); //PB11
  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

