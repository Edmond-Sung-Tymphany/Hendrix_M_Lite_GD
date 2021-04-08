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
    {GPIO_IN_B1_SW, IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  ArraySize(gpioPinsForKey),
  .pGPIOPinSet = gpioPinsForKey
};

static const tGPIOPin gpioPinsForAseTk[] =
{
    {GPIO_OUT_BOOT_STATUS_SW,   IO_PORT_B,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},
    {GPIO_OUT_ASE_RST_N,        IO_PORT_A,  IO_BIT_11, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},
    {GPIO_OUT_ASE_SYS_EN,       IO_PORT_A,  IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},
    {GPIO_IN_ASE_REQ_PDN,       IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE},
};

static const tGPIODevice GPIOConfigForAseTk =
{
  .deviceInfo.deviceID      = ASETK_DEV_ID,
  .deviceInfo.deviceType    = GPIO_DEV_TYPE,
  .usedGPIOPinNum           = ArraySize(gpioPinsForAseTk),
  .pGPIOPinSet              = gpioPinsForAseTk
};

static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_6, IO_PORT_A, IO_BIT_4, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_FLOATING, ODC_NOT_APPLICABLE},   //VDC_DET_IN, PA4
    {GPIO_7, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //BUCK_ON, PA6
    {GPIO_8, IO_PORT_A, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //USB_ON, PA7
    {GPIO_9, IO_PORT_F, IO_BIT_0, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE},   //TPS2546_STATUS PF0 TODO: check if GPIO_ACTIVE_HIGH
    {GPIO_12,  IO_PORT_F,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //P_ON System Power
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
  .pGPIOPinSet = gpioPinsForPower
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

static const tI2CDevice dsp_MGT_Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400 //KHz
};

static const tUARTDevice UartAseTkConfig = {
    .deviceInfo.deviceID    = ASETK_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_9_BITS,
    .parity     = TP_UART_PARITY_EVEN,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

#ifdef HAS_DEBUG
static const tUARTDevice UartDebugConfig = {
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_2,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};
#endif

/****************************************
 * Device: LED                         *
 ****************************************/
static const tIoExpanderLedMap ioeLedMap[] =
{
    {LED1_R,  COLOR_COMPONENT_RED,      0,  1},
    {LED2_R,  COLOR_COMPONENT_RED,      1,  2},
};

static const tI2CDevice  Ledi2cDeviceConf =
{
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,  // I2C2
    .pvSlaveCallback    = NULL,
    .regAddrLen      = REG_LEN_8BITS,
    .baudRate           = 400,      //KHz
    .devAddress         = 0xB6,
};

static const tIoeLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = I2C_DEV_TYPE,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = ioeLedMap,
    .i2cDeviceConf          = &Ledi2cDeviceConf,
};

/****************************************
 * Device: NVM                          *
 ****************************************/
const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

#ifdef HAS_HW_SPI_DEVICE
/****************************************
 * Device: SPI                          *
 ****************************************/
static const tSpiDevice spi2DspConfig = {
    .deviceInfo.deviceID    = SPI2_DEV_ID,
    .deviceInfo.deviceType  = SPI_DEV_TYPE,
    .channel = TP_SPI_CH_2,
    .spiConfig.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .spiConfig.SPI_Mode = SPI_Mode_Master,
    .spiConfig.SPI_DataSize = SPI_DataSize_8b,
    .spiConfig.SPI_CPOL = SPI_CPOL_High,
    .spiConfig.SPI_CPHA = SPI_CPHA_1Edge,
    .spiConfig.SPI_NSS = SPI_NSS_Hard,
    .spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
    .spiConfig.SPI_FirstBit = SPI_FirstBit_MSB,
    .spiConfig.SPI_CRCPolynomial = 7,
};
#endif	// HAS_HW_SPI_DEVICE

#ifdef HAS_SWi2c_DEVICE
// ATMOS board : PA4:SCL, PA5:SDA
void SWi2c1_LowLevel_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

static void vSWi2c1_SCLSet(int high)
{
    if( high )
        GPIO_SetBits(GPIOA, GPIO_Pin_4);
    else
        GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}

static void vSWi2c1_SDASet(int high)
{
    if( high )
        GPIO_SetBits(GPIOA, GPIO_Pin_5);
    else
        GPIO_ResetBits(GPIOA, GPIO_Pin_5);
}

static void vSWi2c1_SDADirSetup(int dir)
{
    uint32_t pinpos;
    
    // only change the MODER
    pinpos = GPIO_PinSource5;
    GPIOA->MODER  &= ~(GPIO_MODER_MODER0 << (pinpos * 2));
    if( dir )
        GPIOA->MODER |= (GPIO_Mode_OUT << (pinpos * 2));
    else
        GPIOA->MODER |= (GPIO_Mode_IN << (pinpos * 2));
}

static int iSWi2c1_SDASense(void)
{
    int ret_value;

    ret_value = (int)GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);

    return ret_value;
}

#ifdef DEBUG_SW_I2C_API
static const stSWi2cDevice_t swi2cDebugConfig = {
    .deviceInfo.deviceID    = SW_I2C1_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x22,
    .delayMSAfterWrite = 3,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
};
#endif

#endif  // HAS_SWi2c_DEVICE

const tDevice * const devices[] =
{
//    (tDevice*)&adcKeyboardConfig,
    (tDevice*)&gpioKeyboardConfig,
//    (tDevice*)&dsp_MGT_Config,
//    (tDevice*)&ADCConfigForPower,
    (tDevice*)&GPIOConfigForPower,
//    (tDevice*)&GPIOConfigForAseTk,
//    (tDevice*)&UartAseTkConfig,
#ifdef HAS_DEBUG
    (tDevice*)&UartDebugConfig,
#endif
    (tDevice*)&ledConfig,
    (tDevice*)&nvmConfig,
    
	// SPI
#ifdef HAS_HW_SPI_DEVICE
    (tDevice*)&spi2DspConfig,
#endif

#ifdef DEBUG_SW_I2C_API
    (tDevice*)&swi2cDebugConfig,
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
    case TP_UART_DEV_2:
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
        /* Configure the HSI as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART2SW;
        RCC->CFGR3 |= RCC_CFGR3_USART2SW_HSI;

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

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
  // TODO: to be implemented
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
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
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

#ifdef HAS_HW_SPI_DEVICE
void Spi1_LowLevel_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // TODO : add GPIO config here base on your board.
}

// ATMOS board : PB12/13/14/15 used for SPI2
void Spi2_LowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    // enable periph. clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // AF select
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_0);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
#endif	// HAS_HW_SPI_DEVICE


