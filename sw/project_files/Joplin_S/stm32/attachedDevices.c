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
    {GPIO_IN_MULTI_KEY, IO_PORT_B,  IO_BIT_12, GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Multiple function key
    {GPIO_IN_INPUT_KEY, IO_PORT_F,  IO_BIT_0,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // input source key
    {GPIO_IN_POWER_KEY, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be
};

static const tGPIODevice GPIOKeyConfig =
{
    .usedGPIOPinNum =  ArraySize(gpioKeyPins),
    .pGPIOPinSet = gpioKeyPins
};

static tGpioKey gpioKeysSet[] =
{
    {PLAY_PAUSE_KEY,    GPIO_IN_MULTI_KEY},
    {INPUT_KEY,         GPIO_IN_INPUT_KEY},
    {POWER_KEY,         GPIO_IN_POWER_KEY},
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


/******************************** Rotater key configuration **************************/
static const tGPIOPin gpioRotaterVolPins[] =
{
    {GPIO_IN_ROTARY_VOL_A, IO_PORT_A, IO_BIT_3, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
    {GPIO_IN_ROTARY_VOL_B, IO_PORT_A, IO_BIT_4, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
};

static const tGPIODevice gpioRotaterVolConfig =
{
    .deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioRotaterVolPins),
    .pGPIOPinSet = gpioRotaterVolPins
};

static tRotaterKeyboardDevice rotaterVolKeyConfig =
{
    .kbDev.deviceInfo.deviceID   = KEYBOARD_DEV_ID,
    .kbDev.deviceInfo.deviceType = ROTATER_DEV_TYPE,
    .kbDev.keyNum = 1,  // only : CW and CCW
    .gpioConfig = &gpioRotaterVolConfig,
    .keyCw      = VOLUME_DOWN_KEY,
    .keyCcw     = VOLUME_UP_KEY,
};

static const tGPIOPin gpioRotaterBasPins[] =
{
    {GPIO_IN_ROTARY_VOL_A, IO_PORT_A, IO_BIT_5, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
    {GPIO_IN_ROTARY_VOL_B, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
};

static const tGPIODevice gpioRotaterBasConfig =
{
    .deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioRotaterBasPins),
    .pGPIOPinSet = gpioRotaterBasPins
};

static tRotaterKeyboardDevice rotaterBasKeyConfig =
{
    .kbDev.deviceInfo.deviceID   = KEYBOARD_DEV_ID,
    .kbDev.deviceInfo.deviceType = ROTATER_DEV_TYPE,
    .kbDev.keyNum = 1,  // only : CW and CCW
    .gpioConfig = &gpioRotaterBasConfig,
    .keyCw      = BASS_DOWN_KEY,
    .keyCcw     = BASS_UP_KEY,
};

static const tGPIOPin gpioRotaterTrePins[] =
{
    {GPIO_IN_ROTARY_VOL_A, IO_PORT_C, IO_BIT_14, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
    {GPIO_IN_ROTARY_VOL_B, IO_PORT_C, IO_BIT_15, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_LOW, ODC_NOT_APPLICABLE},
};

static const tGPIODevice gpioRotaterTreConfig =
{
    .deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioRotaterTrePins),
    .pGPIOPinSet = gpioRotaterTrePins
};

static tRotaterKeyboardDevice rotaterTreKeyConfig =
{
    .kbDev.deviceInfo.deviceID   = KEYBOARD_DEV_ID,
    .kbDev.deviceInfo.deviceType = ROTATER_DEV_TYPE,
    .kbDev.keyNum = 1,  // only : CW and CCW
    .gpioConfig = &gpioRotaterTreConfig,
    .keyCw      = TREBLE_DOWN_KEY,
    .keyCcw     = TREBLE_UP_KEY,
};

/******************************** Rotater key configuration **************************/

/******************************** ADC Power configuration **************************/
static const tAdcPinIoAttr attrADCPinsForPower[] =
{
    {ADC_PIN9, ADC_CHANNEL9,IO_PORT_B, IO_BIT_1}, // HW version
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
    {GPIO_IN_POWER_KEY, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID = POWER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
    .pGPIOPinSet = gpioPinsForPower
};

/******************************** power control **************************/


/****************************************
 * Device: LED                         *
 ****************************************/
static const tIoExpanderLedMap ioeTreLedMap[] =
{
    /*                                        port(0:A, 1:B)  pin  */
    {LED_PWR,       RED,      1,      0}, // P1_0
    {LED_TRE_0,     RED,      1,      3}, // P1_3
    {LED_TRE_1,     RED,      1,      2}, // P1_2
    {LED_TRE_2,     RED,      1,      1}, // P1_1
    {LED_TRE_3,     RED,      0,      0}, // P0_0
    {LED_TRE_4,     RED,      0,      1}, // P0_1
    {LED_TRE_5,     RED,      0,      2}, // P0_2
    {LED_TRE_6,     RED,      0,      3}, // P0_3
    {LED_TRE_7,     RED,      0,      4}, // P0_4
    {LED_TRE_8,     RED,      1,      7}, // P1_7
    {LED_TRE_9,     RED,      1,      6}, // P1_6
    {LED_TRE_10,    RED,      1,      5}, // P1_5
};

static tI2CDevice  ioeTreI2cDeviceConf =
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

static tIoExpanderConfig ioeLedConfig =
{
    .swResetValue = 0x00,
    .ledModePortA = 0x00,
    .ledModePortB = 0x00,
    .outPutPortA  = 0x00,
    .outPutPortB  = 0x00,
    .controlValue = (0x10 | 0x03),
};

static const tGPIOPin ioeRstPin[] =
{
    {GPIO_OUT_IOEXP_NRST,  IO_PORT_B, IO_BIT_2, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //IO_NRST-2  PB2
};

static tGPIODevice ioeRstConfig =
{
    .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(ioeRstPin),
    .pGPIOPinSet = ioeRstPin
};

static const tIoeLedDevice ledTreConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeTreLedMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeTreLedMap,
    .i2cDeviceConf          = &ioeTreI2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedConfig,
    .pResetGpioDevice       = NULL,
};

static const tIoExpanderLedMap ioeVolLedMap[] =
{
    /*                                        port(0:A, 1:B)  pin  */
    {LED_BT,        RED,      1,      0}, // P1_0
    {LED_AUX,       RED,      1,      1}, // P1_1
    {LED_RCA,       RED,      1,      2}, // P1_2
    {LED_VOL_0,     RED,      1,      5}, // P1_5
    {LED_VOL_1,     RED,      1,      4}, // P1_4
    {LED_VOL_2,     RED,      0,      7}, // P0_7
    {LED_VOL_3,     RED,      0,      6}, // P0_6
    {LED_VOL_4,     RED,      0,      5}, // P0_5
    {LED_VOL_5,     RED,      0,      4}, // P0_4
    {LED_VOL_6,     RED,      0,      3}, // P0_3
    {LED_VOL_7,     RED,      0,      2}, // P0_2
    {LED_VOL_8,     RED,      0,      1}, // P0_1
    {LED_VOL_9,     RED,      0,      0}, // P0_0
    {LED_VOL_10,    RED,      1,      3}, // P1_3
};

static tI2CDevice  ioeVolI2cDeviceConf =
{
    .deviceInfo.deviceID= IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_TWO,  // I2C2
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = I2C2_CLK_KHZ,
    .devAddress         = 0xB2,
};

static const tIoeLedDevice ledVolConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeVolLedMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeVolLedMap,
    .i2cDeviceConf          = &ioeVolI2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedConfig,
    .pResetGpioDevice       = NULL,
};

static const tIoExpanderLedMap ioeBasLedMap[] =
{
    /*                                        port(0:A, 1:B)  pin  */
    {LED_BAS_0,     RED,      0,      0}, // P0_0
    {LED_BAS_1,     RED,      0,      1}, // P0_1
    {LED_BAS_2,     RED,      0,      2}, // P0_2
    {LED_BAS_3,     RED,      1,      7}, // P1_7
    {LED_BAS_4,     RED,      1,      6}, // P1_6
    {LED_BAS_5,     RED,      1,      5}, // P1_5
    {LED_BAS_6,     RED,      1,      4}, // P1_4
    {LED_BAS_7,     RED,      0,      7}, // P0_7
    {LED_BAS_8,     RED,      0,      6}, // P0_6
    {LED_BAS_9,     RED,      0,      5}, // P0_5
    {LED_BAS_10,    RED,      0,      4}, // P0_4
};

static tI2CDevice  ioeBasI2cDeviceConf =
{
    .deviceInfo.deviceID= IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_TWO,  // I2C2
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = I2C2_CLK_KHZ,
    .devAddress         = 0xB0,
};

static const tIoeLedDevice ledBasConfig =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9523,
    .ledNum                 = ArraySize(ioeBasLedMap),
    .pIoExpanderLedMap      = (tIoExpanderLedMap*)ioeBasLedMap,
    .i2cDeviceConf          = &ioeBasI2cDeviceConf,
    .pIoExpanderConfig      = &ioeLedConfig,
    .pResetGpioDevice       = &ioeRstConfig,
};

/*************************Device: ioexpandar GPIO  ********************/

/******************************** audio control **************************/
static const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_OUT_DSP_3V3,   IO_PORT_B,  IO_BIT_5,   GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_FLOATING,    ODC_DISABLE}, //  DSP 3.3V : PB5
    {GPIO_IN_DSP_TUNING, IO_PORT_B,  IO_BIT_13,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,             ODC_NOT_APPLICABLE}, // PB13 dsp tuning detect
    {GPIO_OUT_AMP_ON,    IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,         ODC_DISABLE},
    {GPIO_OUT_PVDD_EN,   IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,         ODC_DISABLE},
    {GPIO_OUT_WF_MUTE,   IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH,        ODC_DISABLE},
    {GPIO_OUT_TW_MUTE,   IO_PORT_F,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH,        ODC_DISABLE},
    {GPIO_IN_WF_FAULT,   IO_PORT_A,  IO_BIT_8,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,              ODC_NOT_APPLICABLE},
    {GPIO_IN_TW_FAULT,   IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,              ODC_NOT_APPLICABLE},
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
    {BT_INPUT0,     IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,      ODC_NOT_APPLICABLE},
    {BT_OUTPUT1,    IO_PORT_A,  IO_BIT_15, GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
    {BT_PWR_EN,     IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
    {BT_RESET_PIN,  IO_PORT_A,  IO_BIT_12, GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
    {BT_3V3_EN,     IO_PORT_B,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},
};

static const tGPIODevice GPIOConfigForBT =
{
    .deviceInfo.deviceID = BT_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioPinsForBT),
    .pGPIOPinSet = gpioPinsForBT
};

static const tUARTDevice uartBtConfig =
{
    .deviceInfo.deviceID    = BT_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1, //PA9, PA10
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
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

static const tI2CDevice i2cTAS5760Config =
{
    .deviceInfo.deviceID = AMP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C1_CLK_KHZ, //KHz
    .devAddress = 0xDA,
};
/******************************** Amplifier related **************************/

static const tUARTDevice UartDebugConfig =
{
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_4,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

static const tStorageDevice nvmConfig =
{
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

const tDevice * const devices[] =
{
    (tDevice*)&gpioKeyConfig,
    (tDevice*)&rotaterVolKeyConfig,
    (tDevice*)&rotaterBasKeyConfig,
    (tDevice*)&rotaterTreKeyConfig,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&GPIOConfigForAudio,
    (tDevice*)&adau1761Config,
    (tDevice*)&ADCConfigForPower,
    (tDevice*)&GPIOConfigForBT,
    (tDevice*)&uartBtConfig,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&nvmConfig,

    (tDevice*)&ledBasConfig,
    (tDevice*)&ledVolConfig,
    (tDevice*)&ledTreConfig,

    // amplifier used
    (tDevice*)&i2cTAS5760Config,
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

/* TODO: Let UartDrv support flexible UART selection. Ex.
 *       gUartMap= {USART1, USART8}
 */
USART_TypeDef *gUartMap[NUM_OF_UART] =
{
    USART1,
    USART2,
    USART3,
    USART4,
};

const uint8_t gUartNvicIrq[NUM_OF_UART] =
{
    USART1_IRQn,
    USART2_IRQn,
    USART3_8_IRQn,
    USART3_8_IRQn,
};

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
        case TP_UART_DEV_4:
        {
            GPIO_InitTypeDef GPIO_InitStructure;

            RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
            RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART4,ENABLE);

            GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_4);
            GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_4);

            /* Configure USART2(BLE) pins:  Rx and Tx */
            GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_0 | GPIO_Pin_1;
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
    GPIO_InitTypeDef    GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C1_GPIO_ReInit(void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

#ifdef HAS_MCO
void MCO_Init(void)
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
    RCC->CFGR |= (uint32_t)(RCC_CFGR_MCO_1 | RCC_CFGR_MCO_HSE);
}
#endif


