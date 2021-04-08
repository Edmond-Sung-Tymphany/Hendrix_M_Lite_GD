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
    {GPIO_IN_STB_SW,        IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // Power key, PC13
};
static const tGPIODevice GPIOKeyConfig =
{
  .usedGPIOPinNum =  ArraySize(gpioKeyPins),
  .pGPIOPinSet = gpioKeyPins
};

/**
 * keyboard object setting
 */
static const tGpioKey gpioKeysSet[] =
{   /* key_id           */
    {STANDBY_KEY,          GPIO_IN_STB_SW},
};

static tGpioKeyboardDevice gpioKeyConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = NUM_OF_GPIO_KEY, // == ArraySize(gpioKeysSet)
    .attachedDeviceObjConfig = &GPIOKeyConfig,
    .pGpioKeySet = gpioKeysSet
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

/* #6 board volume up key = 2777 */
#define ADC_THRESHOLD_LEVEL_3   2500
#define ADC_THRESHOLD_LEVEL_4   3000

/* #6 board BT key = 2143 */
#define ADC_THRESHOLD_LEVEL_5   2000
#define ADC_THRESHOLD_LEVEL_6   2300

#define ADC_THRESHOLD_RANGE_3_4   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4
#define ADC_THRESHOLD_RANGE_5_6   ADC_THRESHOLD_LEVEL_5,ADC_THRESHOLD_LEVEL_6

static const tAdcPinIoAttr adcKeyPins[] =
{
    {ADC_KEY_BUTTON0,    ADC_CHANNEL10,  IO_PORT_C, IO_BIT_0},  // adc key 0
    {ADC_KEY_BUTTON1,    ADC_CHANNEL11,  IO_PORT_C, IO_BIT_1},  // adc key 1
};

static const tADCDevice adcKeyConfig =
{
//    .deviceInfo.deviceID    = KEYBOARD_DEV_ID,
//    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(adcKeyPins),
    .pAdcPinIoAttr = adcKeyPins
};

static const tAdcKey adcKeys[]=
{   /* key_id           key_ADC_channel   adc threshold */
    {SOURCE_SWITCH_IR_KEY,  ADC_PIN11,   ADC_THRESHOLD_RANGE_5_6},    
    {BT_KEY,            ADC_PIN10,   ADC_THRESHOLD_RANGE_5_6},
    {VOLUME_UP_KEY,     ADC_PIN10,   ADC_THRESHOLD_RANGE_3_4},
    {VOLUME_DOWN_KEY,   ADC_PIN11,   ADC_THRESHOLD_RANGE_3_4},
};

static tAdcKeyboardDevice adcKeyboardConfig =
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = ArraySize(adcKeys),
    .attachedDeviceObjConfig = &adcKeyConfig,
    .pAdcKeySet = adcKeys
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

/****************************************
 * Device: ioexpander reset ctrl         *
 ****************************************/
static const tGPIOPin ioexpanderRstPin[] =
{
    {GPIO_OUT_IOEXP_RST,  IO_PORT_C, IO_BIT_2, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},  // PC2
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

/****************************************
 * Device: Audio                        *
 ****************************************/
static const tGPIOPin gpioAudioPins[] =
{
    {GPIO_OUT_PCM9211_POWER,    IO_PORT_A,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_PCM3V3_POWER,     IO_PORT_C,  IO_BIT_6,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_AMP_POWER,        IO_PORT_A,  IO_BIT_6,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_1V1_POWER,        IO_PORT_A,  IO_BIT_8,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
};

static const tGPIODevice gpioAudioConfig =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioAudioPins),
  .pGPIOPinSet = gpioAudioPins
};

/****************************************
 * Device: LED                         *
 ****************************************/
//Mark
#if 0
static const tIoExpanderLedMap ioeLedMap[] =
{
    {LED0_RED,      COLOR_COMPONENT_RED,    1,   6},
    {LED1_GREEN,    COLOR_COMPONENT_GREEN,  0,   7},
    {LED2_BLUE,     COLOR_COMPONENT_BLUE,   1,   2},
    {LED3_RED,      COLOR_COMPONENT_RED,    1,   3},
    {LED4_GREEN,    COLOR_COMPONENT_GREEN,  1,   4},
    {LED5_BLUE,     COLOR_COMPONENT_BLUE,   1,   5},
};
#else
static const tIoExpanderLedMap ioeLedMap[] =
{
    {LED_RED,          COLOR_COMPONENT_RED,           0,   2},
    {LED_GREEN,        COLOR_COMPONENT_GREEN,         0,   1},
    {LED_BLUE,         COLOR_COMPONENT_BLUE,          0,   0},
    {LED1_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   7},
    {LED2_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   6},
    {LED3_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   5},
    {LED4_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   4},
    {LED5_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    0,   7},
    {LED6_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   3},
    {LED7_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   2},
    {LED8_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   1},
    {LED9_WHITE,       COLOR_COMPONENT_BRIGHTNESS,    1,   0},
};
#endif

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
 * Device: CODEC PCM9211                *
 ****************************************/
#ifdef HAS_PCM9211_CODEC
// PB4
#define PCM_RESET_GPIO_CLOCK        RCC_AHBPeriph_GPIOB
#define PCM_RESET_GPIO_GROUP        GPIOB
#define PCM_RESET_GPIO_PIN          GPIO_Pin_4
void Pcm9211_ResetSetup(int high)
{
    if( high )
        GPIO_SetBits(PCM_RESET_GPIO_GROUP, PCM_RESET_GPIO_PIN);
    else
        GPIO_ResetBits(PCM_RESET_GPIO_GROUP, PCM_RESET_GPIO_PIN);
}

/* config the reset GPIO pin */
void Pcm9211_ResetGPIOInit(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(PCM_RESET_GPIO_CLOCK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = PCM_RESET_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(PCM_RESET_GPIO_GROUP, &GPIO_InitStructure);
}

static const tI2CDevice  pcm9211I2cDevice =
{
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,
    .pvSlaveCallback    = NULL,
    .regAddrLen      = REG_LEN_8BITS,
    .baudRate           = 400,      //KHz
    .devAddress         = 0x84,
};

static const stPcm9211Device_t pcm9211Config =
{
    .deviceInfo.deviceID    = PCM9211_DEV_ID,
    .deviceInfo.deviceType  = I2C_DEV_TYPE,
    .p_I2cDevice = &pcm9211I2cDevice,
};
#endif  // HAS_PCM9211_CODEC

/****************************************
 * Device: NVM                          *
 ****************************************/
const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

/****************************************
 * Device: SPI                          *
 ****************************************/
#ifdef HAS_HW_SPI_DEVICE
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
    .spiConfig.SPI_FirstBit = SPI_FirstBit_LSB,     //on dsp side, spi configure to LSB,so here need to same as dsp
    .spiConfig.SPI_CRCPolynomial = 7,
};
#endif	// HAS_HW_SPI_DEVICE

/****************************************
 * Device: software i2c                 *
 ****************************************/
#ifdef HAS_SWi2c_DEVICE

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
#endif

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
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
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

// ATMOS board : PC15:SCL, PC14:SDA
void SWi2c2_LowLevel_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
//    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void vSWi2c2_SCLSet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_15);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_15);
}

static void vSWi2c2_SDASet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_14);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_14);
}

static void vSWi2c2_SDADirSetup(int dir)
{
    uint32_t pinpos;
    
    // only change the MODER
    pinpos = GPIO_PinSource14;
    GPIOC->MODER  &= ~(GPIO_MODER_MODER0 << (pinpos * 2));
    if( dir )
        GPIOC->MODER |= (GPIO_Mode_OUT << (pinpos * 2));
    else
        GPIOC->MODER |= (GPIO_Mode_IN << (pinpos * 2));
}

static int iSWi2c2_SDASense(void)
{
    int ret_value;

    ret_value = (int)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);

    return ret_value;
}

#ifdef HAS_SSM3582_AMP
static const stSWi2cDevice_t swi2c_SSM3582_W_L_Config = {
    .deviceInfo.deviceID    = SSM3582_W_L_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x2a,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};

static const stSWi2cDevice_t swi2c_SSM3582_T_L_LU_Config = {
    .deviceInfo.deviceID    = SSM3582_T_L_LU_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x20,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};

static const stSWi2cDevice_t swi2c_SSM3582_T_C_Config = {
    .deviceInfo.deviceID    = SSM3582_T_C_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x28,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};

static const stSWi2cDevice_t swi2c_SSM3582_W_C_Config = {
    .deviceInfo.deviceID    = SSM3582_W_C_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x2c,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};

static const stSWi2cDevice_t swi2c_SSM3582_T_R_RU_Config = {
    .deviceInfo.deviceID    = SSM3582_T_R_RU_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x22,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};

static const stSWi2cDevice_t swi2c_SSM3582_W_R_Config = {
    .deviceInfo.deviceID    = SSM3582_W_R_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_NORMAL,
    .devAddress = 0x2e,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c1_SCLSet,
    .vSWi2cSDA_Set = vSWi2c1_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c1_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c1_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Normal,
};
#endif  // HAS_SSM3582_AMP


#ifdef HAS_HDMI_EP91A6S     //[HDMI]
static const tGPIOPin gpioHdmiPins[] =
{
    {GPIO_IN_HDMI_AMUTE,  IO_PORT_B,  IO_BIT_0,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,   ODC_NOT_APPLICABLE},    //HMCU_AMUTE
    {GPIO_OUT_HDMI_RSTB,  IO_PORT_B,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},           //HMCU_RSTB
    {GPIO_IN_HDMI_INTB,   IO_PORT_B,  IO_BIT_2,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,   ODC_NOT_APPLICABLE},    //HMCU_INTB
};
static const tGPIODevice gpioHdmiConfig =
{
  .deviceInfo.deviceID    = EP_HDMI_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioHdmiPins),
  .pGPIOPinSet = gpioHdmiPins
};


static const stSWi2cDevice_t swi2c_Hdmi_EP91A6S_Config = {
    .deviceInfo.deviceID    = HDMI_DEV_I2C_GPIO_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID2,
    .swi2cSpeed = eSWi2c_SPEED_SLOW,//eSWi2c_SPEED_NORMAL,
    .devAddress = 0xc8,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c2_SCLSet,
    .vSWi2cSDA_Set = vSWi2c2_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c2_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c2_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Slow,//vSWi2c_SclDelay_Normal,
};
   
#endif // HAS_HDMI_EP91A6S

#endif  // HAS_SWi2c_DEVICE

#ifdef HAS_COMM
/****************************************
 * Device: Comm                         *
 ****************************************/
static const tGPIOPin gpioCommPins[] =
{
    {GPIO_OUT_SYSPWR_ON,        IO_PORT_A,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_MTK_USB_UART_SEL, IO_PORT_C,  IO_BIT_8,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_1V5_POWER,        IO_PORT_C,  IO_BIT_7,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_1V2_POWER,        IO_PORT_C,  IO_BIT_9,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
    {GPIO_OUT_WIFI_POWER,       IO_PORT_A,  IO_BIT_11,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  
};

static const tGPIODevice gpioCommConfig =
{
  .deviceInfo.deviceID    = COMM_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioCommPins),
  .pGPIOPinSet = gpioCommPins
};

static const tUARTDevice commConfig = {
    .deviceInfo.deviceID    = COMM_DEV_ID,
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
#endif

/****************************************
 * Device list                          *
 ****************************************/
const tDevice * const devices[] =
{
    //KEY
    (tDevice*)&gpioKeyConfig,
    (tDevice*)&adcKeyboardConfig,

    //Audio
    (tDevice*)&gpioAudioConfig,

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

	// SPI
#ifdef HAS_HW_SPI_DEVICE
    (tDevice*)&spi2DspConfig,
#endif

#ifdef HAS_SSM3582_AMP
    (tDevice *)&swi2c_SSM3582_W_L_Config,
    (tDevice *)&swi2c_SSM3582_T_L_LU_Config,
    (tDevice *)&swi2c_SSM3582_T_C_Config,
    (tDevice *)&swi2c_SSM3582_W_C_Config,
    (tDevice *)&swi2c_SSM3582_T_R_RU_Config,
    (tDevice *)&swi2c_SSM3582_W_R_Config,
#endif

#ifdef HAS_PCM9211_CODEC
    (tDevice *)&pcm9211Config,
#endif

#ifdef HAS_COMM
    (tDevice*)&gpioCommConfig,
    (tDevice*)&commConfig,
#endif
    
#ifdef HAS_HDMI_EP91A6S     //[HDMI]
    (tDevice *)&swi2c_Hdmi_EP91A6S_Config,
    (tDevice *)&gpioHdmiConfig,
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
    case TP_UART_DEV_1: //UART1 for wifi module
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
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
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

