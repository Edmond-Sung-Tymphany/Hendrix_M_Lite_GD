/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Christopher Alexander, Wesley Lee
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

#include "stm32f0xx_usart.h"
#include "stm32f0xx_crs.h"
#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"
#include "product.config"
#include "bsp.h"

/****************************************
 * Device: Keys                         *
 ****************************************/
static const tAdcPinIoAttr attrADCPinsForKey[] =
{
  {ADC_KEY, ADC_CHANNEL6,  IO_PORT_A, IO_BIT_6},
};

static const tADCDevice adcConfigForKey =
{
    .ADCEnabledPinNum = NUM_OF_ADC_KEY_PIN,
    .pAdcPinIoAttr = attrADCPinsForKey
};

static const tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {MINUS_KEY,          ADC_PIN6,   ADC_THRESHOLD_LEVEL_1,  ADC_THRESHOLD_LEVEL_2   },
    {PHASE_KEY,         ADC_PIN6,   ADC_THRESHOLD_LEVEL_3,  ADC_THRESHOLD_LEVEL_4   },
    {LPF_KEY,           ADC_PIN6,   ADC_THRESHOLD_LEVEL_5,  ADC_THRESHOLD_LEVEL_6   },
    {VOL_KEY,           ADC_PIN6,   ADC_THRESHOLD_LEVEL_7,  ADC_THRESHOLD_LEVEL_8   },
    {PLUS_KEY,         ADC_PIN6,   ADC_THRESHOLD_LEVEL_9,  ADC_THRESHOLD_LEVEL_10  },
    {STANDBY_KEY,       ADC_PIN6,   ADC_THRESHOLD_LEVEL_11, ADC_THRESHOLD_LEVEL_12  },
};

static tAdcKeyboardDevice adcKeyConfig =
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = NUM_OF_ADC_KEY,
    .attachedDeviceObjConfig = &adcConfigForKey,
    .pAdcKeySet = adcKeys
};


/****************************************
 * Device: Power                        *
 ****************************************/
static const tGPIOPin gpioPowerPins[] =
{
    {GPIO_OUT_POWER_CTRL,   IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},  //PA15
    {GPIO_IN_JACK_DET,      IO_PORT_A,  IO_BIT_1,   GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,        ODC_NOT_APPLICABLE},
    {GPIO_OUT_AMP_MUTE,     IO_PORT_B,  IO_BIT_2,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, // the same as gpioAudioPins
#ifdef USING_BD2242G_CTRL
    {GPIO_BD2242G_CTRL,     IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},  //PC13
#endif
};

static const tGPIODevice gpioPowerConfig =
{
  .deviceInfo.deviceID      = POWER_DEV_ID,
  .deviceInfo.deviceType    = GPIO_DEV_TYPE,
  .usedGPIOPinNum           =  ArraySize(gpioPowerPins),
  .pGPIOPinSet              = gpioPowerPins
};


static const tAdcPinIoAttr attrAdcPowerPins[] =
{
    {ADC_AC_SENSE,  ADC_CHANNEL7,   IO_PORT_A,  IO_BIT_7},  // 
    {ADC_JACK,      ADC_CHANNEL9,   IO_PORT_B,  IO_BIT_1},  //
};

static const tADCDevice adcPowerConfig =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum       = ArraySize(attrAdcPowerPins),
    .pAdcPinIoAttr          = attrAdcPowerPins
};

/****************************************
 * Device: GPIO LEDS control         *
 ****************************************/
static const tGPIOPin gpioLedPins[] =
{
    {GPIO_OUT_LED_LPF,      IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},
    {GPIO_OUT_LED_PHASE,    IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},
    {GPIO_OUT_LED_RESET,    IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH,   ODC_DISABLE},
};

static const tGPIODevice gpioLedConfig =
{
  .deviceInfo.deviceID      = LED_DEV_ID,
  .deviceInfo.deviceType    = GPIO_DEV_TYPE,
  .usedGPIOPinNum           =  ArraySize(gpioLedPins),
  .pGPIOPinSet              = gpioLedPins
};

/****************************************
 * Device: ioexpander reset ctrl         *
 ****************************************/
static const tGPIOPin ioexpanderRstPin[] =
{
    {GPIO_OUT_IOEXP_RST,  IO_PORT_B, IO_BIT_12, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
};

static tGPIODevice ioexpanderRstConfig =
{
  .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(ioexpanderRstPin),
  .pGPIOPinSet = ioexpanderRstPin
};


#ifdef PT_I2C_DEBUG
static const tI2CDevice i2cDbgConfig1 =
{
    .deviceInfo.deviceID = DBG_I2C1_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C2, PB10, PB11
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400, //KHz
    .devAddress = 0x70, 
};
static const tI2CDevice i2cDbgConfig2 =
{
    .deviceInfo.deviceID= DBG_I2C2_ID,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,  // I2C1
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = 400,      //KHz
    .devAddress         = 0xB6,

};
#endif

/****************************************
 * Device: DSP                          *
 ****************************************/
static const tI2CDevice i2cDspConfig =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C2, PB10, PB11
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400, //KHz
    .devAddress = 0x70,  
};


/****************************************
 * Device: Audio                        *
 ****************************************/
static const tGPIOPin gpioAudioPins[] =
{
    {GPIO_OUT_AMP_MUTE,     IO_PORT_B,  IO_BIT_2,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH,    ODC_DISABLE},
    {GPIO_OUT_AMP_STDBY,    IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},
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
    {ADC_AMP_OTP,   ADC_CHANNEL5,   IO_PORT_A, IO_BIT_5},  // AMP  NTC  PA5
};

static const tADCDevice adcAudioConfig =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrAdcAudioPins),
    .pAdcPinIoAttr = attrAdcAudioPins
};

/****************************************
 * Device: LED                         *
 ****************************************/
static /*const*/ tIoExpanderLedMap ioeLedMap[] =
{
    {LED_BAR_1,     COLOR_COMPONENT_BRIGHTNESS,    1,      0}, // IO-Expander OUT
    {LED_BAR_2,     COLOR_COMPONENT_BRIGHTNESS,    1,      1}, // IO-Expander OUT
    {LED_BAR_3,     COLOR_COMPONENT_BRIGHTNESS,    1,      2}, 
    {LED_BAR_4,     COLOR_COMPONENT_BRIGHTNESS,    0,      2},
    {LED_BAR_5,     COLOR_COMPONENT_BRIGHTNESS,    0,      3},
    {LED_BAR_6,     COLOR_COMPONENT_BRIGHTNESS,    0,      4},
    {LED_BAR_7,     COLOR_COMPONENT_BRIGHTNESS,    0,      5},
    {LED_BAR_8,     COLOR_COMPONENT_BRIGHTNESS,    0,      6},
    {LED_BAR_9,     COLOR_COMPONENT_BRIGHTNESS,    0,      7},
    {LED_BAR_10,    COLOR_COMPONENT_BRIGHTNESS,    1,      4},
    {LED_BAR_11,    COLOR_COMPONENT_BRIGHTNESS,    1,      5},
    {LED_PLUS,      COLOR_COMPONENT_BRIGHTNESS,    1,      3},
    {LED_MINUS,     COLOR_COMPONENT_BRIGHTNESS,    0,      0},
    {LED_VOL,       COLOR_COMPONENT_BRIGHTNESS,    0,      1},
    {LED_STBY_BLUE, COLOR_COMPONENT_BRIGHTNESS,    1,      7},
    {LED_STBY_AMBER,COLOR_COMPONENT_BRIGHTNESS,    1,      6},
};

static /*const*/ tI2CDevice  Ledi2cDeviceConf =
{
    .deviceInfo.deviceID= LED_DEV_ID,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,  // I2C1
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = 400,      //KHz
    .devAddress         = 0xB6,
};

static tIoExpanderConfig ioeLedConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x00,
    .ledModePortB = 0x00,
    .outPutPortA  = 0xff,
    .outPutPortB  = 0xff,
    .controlValue = (0x10 | 0x03),
};

static const tIoeLedDevice ledConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = ioeLedMap,
    .i2cDeviceConf          = &Ledi2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedConfig,
    .pResetGpioDevice       = &ioexpanderRstConfig,
};


/****************************************
 * Device: ADC: PCM1862                 *
 ****************************************/
//static const tI2CDevice i2cAdcConfig =
//{
//    .deviceInfo.deviceID = AUDIO_ADC_DEV_ID,
//    .deviceInfo.deviceType = I2C_DEV_TYPE,
//    .i2cMode = I2C_MASTER_MODE,
//    .channel = I2C_CHANNEL_ONE, //I2C1
//    .pvSlaveCallback = NULL,  //not-used parameter
//    .regAddrLen = REG_LEN_8BITS,
//    .baudRate = 400, //KHz
//    .devAddress = 0x94, //MD1 is hardware pull low, thus I2C address is 0x94
//};



/****************************************
 * Device: Debug                        *
 ****************************************/
static const tUARTDevice uartDebugConfig =
{
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

/****************************************
 * Device: NVM                          *
 ****************************************/
const tStorageDevice nvmConfig =
{
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};


/****************************************
 * Device: BLE                          *
 ****************************************/
static const tUARTDevice bleConfig =
{
    .deviceInfo.deviceID    = BLE_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_9_BITS,
    .parity     = TP_UART_PARITY_EVEN,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};


/****************************************
 * Device list                          *
 ****************************************/
const tDevice * const devices[] =
{
    //KEY
    (tDevice*)&adcKeyConfig,

    //POWER
    (tDevice*)&gpioPowerConfig,
    (tDevice*)&adcPowerConfig,

    //Audio
    (tDevice*)&gpioAudioConfig,
    (tDevice*)&adcAudioConfig,
    (tDevice*)&i2cDspConfig,

    //LED
    (tDevice*)&ioexpanderRstConfig,
    (tDevice*)&ledConfig,
    (tDevice*)&gpioLedConfig,

    //Storage
    (tDevice*)&nvmConfig,

    // BLE
    (tDevice*)&bleConfig,

    //UART
    (tDevice*)&uartDebugConfig,

#ifdef PT_I2C_DEBUG
    (tDevice*)&i2cDbgConfig1,
    (tDevice*)&i2cDbgConfig2,
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
    case TP_UART_DEV_1: // For BLE
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
        /* Configure the USART clock */
        RCC_USARTCLKConfig(RCC_USART1CLK_HSI);

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
        /* Configure the USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART2SW;
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_HSI;

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

        /* Configure the USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_HSI;

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

void UsbDrv_Init(void)
{
    /* Enable USB clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);

#ifdef USB_CLOCK_SOURCE_CRS
    /*For using CRS, you need to do the following:
    - Enable HSI48 (managed by the SystemInit() function at the application startup)
    - Select HSI48 as USB clock
    - Enable CRS clock
    - Set AUTOTRIMEN
    - Set CEN
    */
  
    /* Select HSI48 as USB clock */
    RCC_USBCLKConfig(RCC_USBCLK_HSI48);

    /* Configure the Clock Recovery System */
    /*Enable CRS Clock*/
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_CRS, ENABLE);

    /* Select USB SOF as synchronization source */
    CRS_SynchronizationSourceConfig(CRS_SYNCSource_USB);

    /*Enables the automatic hardware adjustment of TRIM bits: AUTOTRIMEN:*/
    CRS_AutomaticCalibrationCmd(ENABLE);

    /*Enables the oscillator clock for frequency error counter CEN*/
    CRS_FrequencyErrorCounterCmd(ENABLE);
#else 
    /* Configure PLL to be used as USB clock8 */
    /* Configure USBCLK from PLL clock */
    RCC_USBCLKConfig(RCC_USBCLK_PLLCLK); 
#endif /*USB_CLOCK_SOURCE_CRS */ 

#ifdef USB_DEVICE_LOW_PWR_MGMT_SUPPORT  

    EXTI_InitTypeDef EXTI_InitStructure;

    /* Enable the PWR clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    /* EXTI line 18 is connected to the USB Wakeup from suspend event   */
    EXTI_ClearITPendingBit(EXTI_Line18);
    EXTI_InitStructure.EXTI_Line = EXTI_Line18; 
    /*Must Configure the EXTI Line 18 to be sensitive to rising edge*/
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
#endif /*USB_DEVICE_LOW_PWR_MGMT_SUPPORT */

    /* Enable the USB interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USB_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void UsbDrv_DeInit(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, DISABLE);

    /* Enable the USB interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = USB_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    NVIC_Init(&NVIC_InitStructure);
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
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_1); //PB6

  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_1); //PB7

  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
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
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
}


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

    RCC->CFGR &= (uint32_t)(~RCC_CFGR_MCO);
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLNODIV | RCC_CFGR_MCO_1 | RCC_CFGR_MCO_HSE);
}

