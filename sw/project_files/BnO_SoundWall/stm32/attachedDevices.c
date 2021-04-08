/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Viking Wang
* @date 02-Nov-2016
* @copyright Tymphany Ltd.
*/

#include "product.config"

#include "stm32f0xx.h"

#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"


/********************************* power control ********************************/
static const tGPIOPin powerGpioPins[] =
{
    // power control
    {GPIO_OUT_MAIN_POWER,  IO_PORT_A,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
    {GPIO_OUT_DSP_POWER,   IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW, ODC_ENABLE},
    {GPIO_OUT_A2B_POWER,   IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
    {GPIO_OUT_5V_POWER,    IO_PORT_C,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
    {GPIO_IN_POWER_LOST,   IO_PORT_A,  IO_BIT_15, GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    // add amp shutdown and some power up one shot reset pin here
    {GPIO_OUT_OP_MUTE,   IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW, ODC_DISABLE},
    {GPIO_OUT_AMP_RESET,  IO_PORT_C,  IO_BIT_10,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
    {GPIO_IN_AMP_CLP,   IO_PORT_A,  IO_BIT_5,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    {GPIO_IN_AMP_FAULT,  IO_PORT_A,  IO_BIT_6,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    // power up reset chip
    {GPIO_OUT_DSP_RESET,  IO_PORT_C,  IO_BIT_9,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_ENABLE},
    {GPIO_OUT_CODEC_RESET,  IO_PORT_C,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_ENABLE},
#ifdef MCU_COMM_VIA_GPIO
    {GPIO_OUT_A2B_OUTPUT_1,  IO_PORT_C,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_ENABLE},
    {GPIO_OUT_A2B_OUTPUT_2,  IO_PORT_C,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_ENABLE},
    {GPIO_IN_A2B_INPUT_1,   IO_PORT_A,  IO_BIT_2, GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    {GPIO_IN_A2B_INPUT_2,   IO_PORT_A,  IO_BIT_3, GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
#endif
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(powerGpioPins),
    .pGPIOPinSet            = powerGpioPins
};
/********************************* power control ********************************/


/********************************* DSP control ********************************/
static const tGPIOPin dspGpioPins[] =
{
    {GPIO_IN_DSP_SELFBOOT,  IO_PORT_C,  IO_BIT_12,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForDsp =
{
    .deviceInfo.deviceID    = DSP_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(dspGpioPins),
    .pGPIOPinSet            = dspGpioPins
};

static const tI2CDevice adau1452Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 400,//KHz
    .devAddress = 0x70,
};
/********************************* AD2410 control ********************************/

static const tI2CDevice ad24xxConfig =
{
    .deviceInfo.deviceID = A2B_MASTER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 100,//KHz
    .devAddress = 0xd0,
};


/********************************* Codec control ********************************/
static const tGPIOPin codecGpioPins[] =
{
    {GPIO_IN_CODEC_INT,  IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForCodec =
{
    .deviceInfo.deviceID    = AUDIO_CODEC_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(codecGpioPins),
    .pGPIOPinSet            = codecGpioPins
};

static const tI2CDevice codec42528Config =
{
    .deviceInfo.deviceID = AUDIO_CODEC_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400,//KHz
    .devAddress = 0x98,
};
/********************************* Codec control ********************************/

/********************************* for audio driver ********************************/
static const tGPIOPin audioGpioPins[] =
{
    {GPIO_IN_PL_INSERT,  IO_PORT_B,  IO_BIT_8,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},
    {GPIO_IN_DET_SPDIF,  IO_PORT_C,  IO_BIT_2,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    {GPIO_IN_DET_AUX,  IO_PORT_B,  IO_BIT_2,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
    {GPIO_IN_DSP_TUNING,  IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForAudioDrv =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(audioGpioPins),
    .pGPIOPinSet            = audioGpioPins
};
/********************************* for audio driver ********************************/

/********************************* temperature sense ********************************/
static const tAdcPinIoAttr attrADCPinsForXX[] =
{
    {ADC_PIN0, ADC_CHANNEL0, IO_PORT_A, IO_BIT_0},
    {ADC_PIN1, ADC_CHANNEL1, IO_PORT_A, IO_BIT_1},
    {ADC_PIN4, ADC_CHANNEL4, IO_PORT_A, IO_BIT_4},
#ifdef HAS_HW_VERSION_TAG
    {ADC_PIN8, ADC_CHANNEL8, IO_PORT_B, IO_BIT_0}, // HW version
#endif  
};

static const tADCDevice ADCConfigForXX =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrADCPinsForXX),
    .pAdcPinIoAttr = attrADCPinsForXX
};
/********************************* temperature sense ********************************/

/********************************* ioexpender LED ********************************/
static const tGPIOPin ledGpioPins[] =
{
    {GPIO_OUT_LED_RESET,  IO_PORT_D,  IO_BIT_2,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},
};

static const tGPIODevice GPIOConfigForLed =
{
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(ledGpioPins),
    .pGPIOPinSet            = ledGpioPins
};

#ifdef IOE_LED_VIA_HW_I2C
static const tI2CDevice ledIoeConfig =
{
    .deviceInfo.deviceID = LED_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400,//KHz
    .devAddress = 0xb6,
};
#endif

/****************************************
 * Device: software i2c                 *
 ****************************************/
#ifdef IOE_LED_VIA_SW_I2C
#ifdef EXTERNAL_HIGH_SPEED_CLOCK
// when cpu clock = 32MHz, delay=6, i2c speed ~= 100KHz
// when cpu clock = 32MHz, delay=12, i2c speed ~= 66KHz
// when cpu clock = 32MHz, delay=19, i2c speed ~= 50KHz
void vSWi2c_SclDelay_Normal(void)
{
    uint32_t delay = 6;
    while(delay--)
    {
        asm("nop");asm("nop");
        asm("nop");asm("nop");
    }
}

void vSWi2c_SclDelay_Slow(void)
{
    uint32_t delay = 19;
    while(delay--)
    {
        asm("nop");asm("nop");
        asm("nop");asm("nop");
    }
}

// when cpuclk = 32MHz, 3 nop ~= 200KHz
void vSWi2c_SclDelay_Fast(void)
{
    asm("nop");
    asm("nop");
    asm("nop");
}
#else
// cpu=8MHz, use the short delay
void vSWi2c_SclDelay_Normal(void)
{
    asm("nop");asm("nop");
}
void vSWi2c_SclDelay_Fast(void)
{
    asm("nop");
}
#endif

// SoundWall board : PC7:SCL, PC6:SDA
void SWi2c1_LowLevel_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef TUNING_ON_ST_EVK_BOARD
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#endif
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void vSWi2c1_SCLSet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_7);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
}

static void vSWi2c1_SDASet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_6);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_6);
}

static void vSWi2c1_SDADirSetup(int dir)
{
    uint32_t pinpos;
    
    // only change the MODER
    pinpos = GPIO_PinSource6;
    GPIOC->MODER  &= ~(GPIO_MODER_MODER0 << (pinpos * 2));
    if( dir )
        GPIOC->MODER |= (GPIO_Mode_OUT << (pinpos * 2));
    else
        GPIOC->MODER |= (GPIO_Mode_IN << (pinpos * 2));
}

static int iSWi2c1_SDASense(void)
{
    int ret_value;

    ret_value = (int)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);

    return ret_value;
}

static const stSWi2cDevice_t swi2c_IoeLed_Config = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0xb6,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Fast,
};
#endif  // IOE_LED_VIA_SW_I2C

/********************************* ioexpender LED ********************************/


/********************************* I2C slave ********************************/
static const tI2CSlaveDevice i2cSlaveConfig =
{
    .deviceInfo.deviceID = COMM_DEV_ID,
    .deviceInfo.deviceType = I2C_SLAVE_DEV_TYPE,
    .channel = (uint8)I2C_CHANNEL_ONE,
    .speed = (uint8)I2C_SPEED_400K,
#ifdef HAS_I2C_SLAVE
    .dev_addr = I2C_SLAVE_DEVICE_ADDR,
#else
    .dev_addr = 0,
#endif    
    .reg_len = (uint8)REG_LEN_8BITS,
};
/********************************* I2C slave ********************************/

/********************************* Uart config ********************************/
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

#ifdef MCU_COMM_VIA_UART
static const tUARTDevice UartA2BInputConfig = {
    .deviceInfo.deviceID    = COMM_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 19200,
    .uartId     = TP_UART_DEV_2,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

static const tUARTDevice UartA2BOutputConfig = {
    .deviceInfo.deviceID    = COMM_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 19200,
    .uartId     = TP_UART_DEV_3,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};
#endif
/********************************* Uart config ********************************/

/********************************* NVM memory ********************************/
static const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};
/********************************* NVM memory ********************************/

const tDevice * const devices[] =
{
    // power control
    (tDevice*)&GPIOConfigForPower,
    // dsp
    (tDevice*)&GPIOConfigForDsp,
    (tDevice*)&adau1452Config,
    // codec
    (tDevice*)&GPIOConfigForCodec,
    (tDevice*)&codec42528Config,
    // MUSIC/DSP/USBi detect
    (tDevice*)&GPIOConfigForAudioDrv,
    // adc
    (tDevice*)&ADCConfigForXX,
    // ioexpender led
    (tDevice*)&GPIOConfigForLed,
#ifdef IOE_LED_VIA_HW_I2C
    (tDevice*)&ledIoeConfig,
#endif    
#ifdef IOE_LED_VIA_SW_I2C
    (tDevice*)&swi2c_IoeLed_Config,
#endif
    // uart
    (tDevice*)&UartDebugConfig,
#ifdef MCU_COMM_VIA_UART
    (tDevice*)&UartA2BInputConfig,
    (tDevice*)&UartA2BOutputConfig,
#endif
    // nvm
    (tDevice*)&nvmConfig,
    // i2c salve
    (tDevice*)&i2cSlaveConfig,
    // a2b
    (tDevice*)&ad24xxConfig,
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

        /* Configure the HSI or PCKL as USART clock */
#if defined(UART_CLK_SOURCE_PCLK)
        RCC_USARTCLKConfig(RCC_USART1CLK_PCLK);
#elif defined(UART_CLK_SOURCE_HSI)
        RCC_USARTCLKConfig(RCC_USART1CLK_HSI);
#endif

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_1);

        /* Configure USART1(Debug) pins:  PA10:Rx and PA9:Tx */
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

        /* Configure the HSI or PCKL as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART2SW;
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

        /* Configure USART2 pins:  PA3:Rx and PA2:Tx */
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
      }
        break;
    case TP_UART_DEV_3:
#ifdef HAS_UART_3
      {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART3EN,ENABLE);

        /* Configure the HSI or PCKL as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_1);

        /* Configure USART3 pins:  PC5:Rx and PC4:Tx */
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
      }
#endif
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
    case TP_UART_DEV_1: 
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_9 | GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    case TP_UART_DEV_2: 
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
    case TP_UART_DEV_3:
#ifdef HAS_UART_3
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif
        break;
    default:
        ASSERT(0);
        break;
    }
}


void I2C1_LowLevel_Init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

#ifdef SYSCLK_RUN_AT_32M
    /* Configure the I2C clock source. The clock is derived from the SYSCLK */
    RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
#else
    /* Configure the I2C clock source. The clock is derived from the HSI */
    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
#endif

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
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#ifdef TUNING_ON_ST_EVK_BOARD
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#endif
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C2_LowLevel_Init(void)
{
#ifdef HAS_I2C2
    GPIO_InitTypeDef  GPIO_InitStructure;

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
#if defined(TUNING_ON_ST_EVK_BOARD)
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
#else
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
#endif
    GPIO_Init(GPIOB, &GPIO_InitStructure);
#endif
}

void I2C2_GPIO_Deinit(void)
{
    GPIO_InitTypeDef	GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C2_GPIO_ReInit(void)
{
    GPIO_InitTypeDef	GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

#ifdef HAS_I2C_SLAVE
void I2C1_Slave_LowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

#ifdef SYSCLK_RUN_AT_32M
    /* Configure the I2C clock source. The clock is derived from the SYSCLK */
    RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);
#else
    /* Configure the I2C clock source. The clock is derived from the HSI */
    RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);
#endif

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
}

void I2C2_Slave_LowLevel_Init(void)
{
}
#endif


