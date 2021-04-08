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
#include "bsp.h"

GPIO_TypeDef* ioPortMap[] = {GPIOA, GPIOB, GPIOC, GPIOD};

#ifdef HAS_SWI2C1
static void SWI2C1_SCL_Set(int high);
static void SWI2C1_SDA_Set(int high);
static void SWI2C1_SDA_DirSetup(int dir);
static int SWI2C1_SDA_Sense(void);
static void SWI2C1_SCL_Delay(void);
#endif

#ifdef HAS_SWI2C2
static void SWI2C2_SCL_Set(int high);
static void SWI2C2_SDA_Set(int high);
static void SWI2C2_SDA_DirSetup(int dir);
static int SWI2C2_SDA_Sense(void);
static void SWI2C2_SCL_Delay(void);
#endif

#ifdef HAS_SWI23
static void SWI2C3_SCL_Set(int high);
static void SWI2C3_SDA_Set(int high);
static void SWI2C3_SDA_DirSetup(int dir);
static int SWI2C3_SDA_Sense(void);
static void SWI2C3_SCL_Delay(void);
#endif



/****************************************
 * Device: Keys                         *
 ****************************************/
static const tGPIOPin gpioKeyPins[] =
{
    {GPIO_IN_STB_SW,      IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Power key, PA8
    {GPIO_IN_RESET_SW,    IO_PORT_C,  IO_BIT_15,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // Reset key, PC15
    //{GPIO_IN_VOL_DOWN_SW, IO_PORT_B,  IO_BIT_4,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE}, // Volume Down key, PB4
    //{GPIO_IN_VOL_UP_SW,   IO_PORT_B,  IO_BIT_3,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE}, // Volume Up key, PB3
    //{GPIO_IN_CONN_SW,     IO_PORT_D,  IO_BIT_2,   GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE}, // Connect key, PD2
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
    {RESET_KEY,            GPIO_IN_RESET_SW},
//    {CONNECT_KEY,          GPIO_IN_CONN_SW},
//    {VOLUME_UP_KEY,        GPIO_IN_VOL_UP_SW},
//    {VOLUME_DOWN_KEY,      GPIO_IN_VOL_DOWN_SW},
};

static tGpioKeyboardDevice gpioKeyConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = ArraySize(gpioKeysSet),
    .gpioKeyboard.isNeedInitSeq = FALSE,
    .gpioKeyboard.isInitSeqDone = TRUE,
    .attachedDeviceObjConfig = &GPIOKeyConfig,
    .pGpioKeySet = gpioKeysSet
};



/****************************************
 * Device: Power                        *
 ****************************************/
static const tAdcPinIoAttr attrAdcAsengPins[] =
{
    {ADC_HW_VER,  ADC_CHANNEL14,   IO_PORT_C, IO_BIT_4},  // FEP Hardware version PC4
};

static const tADCDevice adcPowerConfig =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrAdcAsengPins),
    .pAdcPinIoAttr = attrAdcAsengPins
};


/****************************************
 * Device: ioexpander reset ctrl         *
 ****************************************/
static const tGPIOPin ioexpanderRstPin[] =
{
    {GPIO_OUT_IOEXP_RST,  IO_PORT_C, IO_BIT_7, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //IO_EXPANDER_RST  PC7
};

static tGPIODevice ioexpanderRstConfig =
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
    {GPIO_IN_DSP_TUNE,           IO_PORT_A,  IO_BIT_15,   GPIO_DIGITAL_INPUT,    GPIO_ACTIVE_HIGH,       ODC_NOT_APPLICABLE}, //PA15
    {GPIO_OUT_ADC_I2S_ENABLE,    IO_PORT_A,  IO_BIT_6,    GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,       ODC_DISABLE}, // PA6,
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
    .channel = I2C_CHANNEL_TWO, //I2C2, PB10, PB11
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
    //{GPIO_IN_AUXIN_JACK_DET,    IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,         ODC_NOT_APPLICABLE}, //AUDIO_JACK_DET1,  PA0
    {GPIO_OUT_AMP_MUTE,         IO_PORT_C,  IO_BIT_9,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  //PC9
    {GPIO_OUT_AMP_SDZ,          IO_PORT_C,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE},  //PC8
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
    {ADC_AMP_NTC,  ADC_CHANNEL4,   IO_PORT_A, IO_BIT_4},  // AMP  NTC  PA4
    {ADC_WF_NTC,   ADC_CHANNEL5,   IO_PORT_A, IO_BIT_5},  // Woofer NTC  PA5
    //{ADC_DSP_NTC,  ADC_CHANNEL8,   IO_PORT_B, IO_BIT_0},  // DSP  NTC  PB0  (will support on EVT2)
};

static const tADCDevice adcAudioConfig =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrAdcAudioPins),
    .pAdcPinIoAttr = attrAdcAudioPins
};

/****************************************
 * Device: Audio reset                  * 
 ****************************************/
static const tGPIOPin gpioAudioResetPins[] =
{
    {GPIO_OUT_DSP_RESET,        IO_PORT_B,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},  //PB1
    {GPIO_OUT_CS8422_RESET,     IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},  //PB3
    {GPIO_OUT_DSP_VCC_ON,        IO_PORT_C,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,   ODC_DISABLE},  //PC3
};


static const tGPIODevice gpioAudioResetConfig =
{
  .deviceInfo.deviceID    = AUDIO_RESET_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioAudioResetPins),
  .pGPIOPinSet = gpioAudioResetPins
};


/****************************************
 * Device: ASE-TK                       *
 ****************************************/
static const tGPIOPin gpioAsetkPins[] =
{
    /* GPIO_OUT_BOOT_STATUS_SW should defaultly pull low, because
     *   Normal condition: pull low
     *   Before jump to ST-BL: pull high
     *
     * ASE datasheet require ASE pins to open drain, means
     *   MCU hope high  ->  MCU let it floating
     *   MCU hope low   ->  MCU pull low
     */
    {GPIO_OUT_BOOT_STATUS_SW,   IO_PORT_B,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB8
    {GPIO_OUT_ASE_RST_N,        IO_PORT_A,  IO_BIT_11, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_ENABLE},  //PA11
    {GPIO_OUT_ASE_SYS_EN,       IO_PORT_A,  IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_ENABLE},  //PA12
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

//    .byteSize   = TP_UART_BYTE_SIZE_8_BITS, //8BITS: 8bit data + 0bit parity 
//    .parity     = TP_UART_PARITY_NONE,
    .byteSize   = TP_UART_BYTE_SIZE_9_BITS, //9BITS: 8bit data + 1bit even parity
    .parity     = TP_UART_PARITY_EVEN,

    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};



/****************************************
 * Device: LED                         *
 ****************************************/
static /*const*/ tIoExpanderLedMap ioeLedMap[] =
 {                             //  port(0:A, 1:B)    pin
    {LED1_GREEN,   COLOR_COMPONENT_GREEN,    1,      0}, // IO-Expander OUT0, B0
    {LED2_BLUE,    COLOR_COMPONENT_BLUE,     1,      1}, // IO-Expander OUT1, B1
    {LED0_RED,     COLOR_COMPONENT_RED,      1,      2}, // IO-Expander OUT2, B2
    {LED4_GREEN,   COLOR_COMPONENT_GREEN,    1,      3}, // IO-Expander OUT3, B3
    {LED5_BLUE,    COLOR_COMPONENT_BLUE,     0,      0}, // IO-Expander OUT4, B4
    {LED3_RED,     COLOR_COMPONENT_RED,      0,      1}, // IO-Expander OUT5, B5
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
    .ledModePortA = 0x04,
    .ledModePortB = 0x00,
    .outPutPortA = 0xFf,
    .outPutPortB = 0xff,
    .controlValue = (0x10 | 0x03),
};

static const tIoeLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .deviceInfo.deviceSubType = AW9110B,
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
 * Device: S/W i2c                      *
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

// CA22 board : I2C3 for ADC/DAC/CS8422
//PC0:SCL, PC1:SDA
void SWi2c3_LowLevel_init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void vSWi2c3_SCLSet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_0);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_0);
}

static void vSWi2c3_SDASet(int high)
{
    if( high )
        GPIO_SetBits(GPIOC, GPIO_Pin_1);
    else
        GPIO_ResetBits(GPIOC, GPIO_Pin_1);
}

static void vSWi2c3_SDADirSetup(int dir)
{
    uint32_t pinpos;
    
    // only change the MODER
    pinpos = GPIO_PinSource1 << 1;
    GPIOC->MODER  &= ~(GPIO_MODER_MODER0 << pinpos);
    if( dir )
        GPIOC->MODER |= (GPIO_Mode_OUT << pinpos);
    else
        GPIOC->MODER |= (GPIO_Mode_IN << pinpos);
}

static int iSWi2c3_SDASense(void)
{
    int ret_value;

    ret_value = (int)GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_1);

    return ret_value;
}

#ifdef HAS_CS8422
static const stSWi2cDevice_t swi2c_CS8422_Config = {
    .deviceInfo.deviceID    = AUDIO_CODEC_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID3,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0x20,
//    .devAddress = 0x9c,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c3_SCLSet,
    .vSWi2cSDA_Set = vSWi2c3_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c3_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c3_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Fast,
};
#endif

#ifdef HAS_PCM1690
static const stSWi2cDevice_t swi2c_PCM1690_Config = {
    .deviceInfo.deviceID    = AUDIO_DAC_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID3,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0x9c,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c3_SCLSet,
    .vSWi2cSDA_Set = vSWi2c3_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c3_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c3_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Fast,
};
#endif

#endif

/****************************************
 * Device: HDMI                        *
 ****************************************/
static const tGPIOPin gpioHdmiPins[] =
{
    /* Do not below two pins position, it's relate with control-function */
    {GPIO_OUT_SWI2C1_SCL,    IO_PORT_C,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH,    ODC_DISABLE},  //PC12
    {GPIO_OUT_SWI2C1_SDA,    IO_PORT_D,  IO_BIT_2,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH,    ODC_DISABLE},  //PD2
    {GPIO_OUT_HDMI_RSTB,    IO_PORT_B,  IO_BIT_0,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, // PB0
    {GPIO_IN_HDMI_INTB,     IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_LOW,         ODC_NOT_APPLICABLE}, // PA0
};

static const tGPIODevice hdmiGpioConfig =
{
  .deviceInfo.deviceID    = EP_HDMI_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioHdmiPins),
  .pGPIOPinSet = gpioHdmiPins
};

#ifdef HAS_SWI2C1
static const stSWi2cDevice_t swi2c1_Config = {
    .deviceInfo.deviceID    = SW_I2C1_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID1,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0xC8,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = SWI2C1_SCL_Set,
    .vSWi2cSDA_Set = SWI2C1_SDA_Set,
    .vSWi2cSDA_DirSetup = SWI2C1_SDA_DirSetup,
    .iSWi2cSDA_Sense = SWI2C1_SDA_Sense,
    .vSWi2cSCL_Delay = SWI2C1_SCL_Delay,
};
#endif

static const stSWi2cDevice_t swi2c2_pcm1862_Config = {
    .deviceInfo.deviceID    = AUDIO_ADC_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID3,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0x94,
//    .devAddress = 0x9c,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = vSWi2c3_SCLSet,
    .vSWi2cSDA_Set = vSWi2c3_SDASet,
    .vSWi2cSDA_DirSetup = vSWi2c3_SDADirSetup,
    .iSWi2cSDA_Sense = iSWi2c3_SDASense,
    .vSWi2cSCL_Delay = vSWi2c_SclDelay_Fast,
};

#ifdef HAS_SWI2C3
static const stSWi2cDevice_t swi2c3_Config = {
    .deviceInfo.deviceID    = SW_I2C3_DEV_ID,
    .deviceInfo.deviceType  = SWI2C_DEV_TYPE,
    .busID = eSWi2cBus_ID3,
    .swi2cSpeed = eSWi2c_SPEED_FAST,
    .devAddress = 0xb8,
    .delayMSAfterWrite = 0,
    .vSWi2cSCL_Set = SWI2C3_SCL_Set,
    .vSWi2cSDA_Set = SWI2C3_SDA_Set,
    .vSWi2cSDA_DirSetup = SWI2C3_SDA_DirSetup,
    .iSWi2cSDA_Sense = SWI2C3_SDA_Sense,
    .vSWi2cSCL_Delay = SWI2C3_SCL_Delay,
};
#endif

#ifdef HAS_CS48L11_DSP
static const tGPIOPin gpioCS48L11Pins[] =
{
//    {GPIO_OUT_CS48L11_RESET,    IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE},
    {GPIO_OUT_CS48L11_SPI_CS,   IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,   ODC_DISABLE},
    {GPIO_IN_CS48L11_BUSY,      IO_PORT_C,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,   ODC_NOT_APPLICABLE},
    {GPIO_IN_CS48L11_INT,       IO_PORT_C,  IO_BIT_13,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_FLOATING,   ODC_NOT_APPLICABLE},
};


static const tGPIODevice gpioCS48L11Config =
{
  .deviceInfo.deviceID    = DSP_DECODER_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioCS48L11Pins),
  .pGPIOPinSet = gpioCS48L11Pins
};

static const tSpiDevice cs48L11_config = {
    .deviceInfo.deviceID    = DSP_DECODER_DEV_ID,
    .deviceInfo.deviceType  = SPI_DEV_TYPE,
    .channel = TP_SPI_CH_2,
    .spiConfig.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .spiConfig.SPI_Mode = SPI_Mode_Master,
    .spiConfig.SPI_DataSize = SPI_DataSize_8b,
    .spiConfig.SPI_CPOL = SPI_CPOL_Low,
    .spiConfig.SPI_CPHA = SPI_CPHA_1Edge,
    .spiConfig.SPI_NSS = SPI_NSS_Soft,
//    .spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
    .spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
    .spiConfig.SPI_FirstBit = SPI_FirstBit_MSB,
    .spiConfig.SPI_CRCPolynomial = 7,
};
#endif

#ifdef HAS_SPI_FLASH
static const tGPIOPin gpioSpiFlashPins[] =
{
    {GPIO_OUT_SPI_FLASH_CS,   IO_PORT_A,  IO_BIT_1,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,   ODC_DISABLE},
};


static const tGPIODevice gpioSpiFlashConfig =
{
  .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioSpiFlashPins),
  .pGPIOPinSet = gpioSpiFlashPins
};

static const tSpiDevice spiflash_config = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = SPI_DEV_TYPE,
    .channel = TP_SPI_CH_2,
    .spiConfig.SPI_Direction = SPI_Direction_2Lines_FullDuplex,
    .spiConfig.SPI_Mode = SPI_Mode_Master,
    .spiConfig.SPI_DataSize = SPI_DataSize_8b,
    .spiConfig.SPI_CPOL = SPI_CPOL_Low,
    .spiConfig.SPI_CPHA = SPI_CPHA_1Edge,
    .spiConfig.SPI_NSS = SPI_NSS_Soft,
//    .spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2,
    .spiConfig.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8,
    .spiConfig.SPI_FirstBit = SPI_FirstBit_MSB,
    .spiConfig.SPI_CRCPolynomial = 7,
};
#endif


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
    // codec/dsp reset
    (tDevice*)&gpioAudioResetConfig,

    //LED
    (tDevice*)&ledConfig,

    //Other
    (tDevice*)&nvmConfig,
    //(tDevice*)&ioexpanderRstConfig,
#ifdef PT_I2C_DEBUG
    (tDevice*)&i2cDbgConfig1,
    (tDevice*)&i2cDbgConfig2,
#endif

    //UART
#ifdef HAS_DEBUG
    (tDevice*)&uartDebugConfig,
#endif

#ifdef HAS_HDMI
    (tDevice*)&hdmiGpioConfig,
#endif

#ifdef HAS_SWI2C1
    (tDevice*)&swi2c1_Config,
#endif

    (tDevice*)&swi2c2_pcm1862_Config,

#ifdef HAS_SWI2C3
    (tDevice*)&swi2c3_Config,
#endif

#ifdef HAS_CS8422
    (tDevice*)&swi2c_CS8422_Config,
#endif
#ifdef HAS_PCM1690
    (tDevice*)&swi2c_PCM1690_Config,
#endif

#ifdef HAS_CS48L11_DSP
    (tDevice*)&cs48L11_config,
    (tDevice*)&gpioCS48L11Config,
#endif
    
#ifdef HAS_SPI_FLASH
    (tDevice*)&spiflash_config,
    (tDevice*)&gpioSpiFlashConfig,
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
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

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
  RCC_I2CCLKConfig(RCC_I2C1CLK_SYSCLK);

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

#ifdef HAS_HW_SPI_DEVICE
void HWspi1_LowLevel_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

    // TODO : add GPIO config here base on your board.
}

// 13/14/15 used for SPI2
void HWspi2_LowLevel_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // enable GPIO clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    // enable periph. clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

    // AF select
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_0);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_0);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}
#endif	// HAS_HW_SPI_DEVICE


static void SWI2C_Delay(uint32_t delay)
{
    while(delay--)
    {
        asm("nop");asm("nop");
        asm("nop");asm("nop");
    }
}

#ifdef HAS_SWI2C1
static void SWI2C1_SDA_DirSetup(int dir)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[1].gpioPort];
    uint32_t pinpos = gpioHdmiPins[1].gpioBit * 2;

   port->MODER &= ~(GPIO_MODER_MODER0 << pinpos);

    if (dir)
        port->MODER |= GPIO_Mode_OUT << pinpos;
    else
        port->MODER |= GPIO_Mode_IN << pinpos;
}

static int SWI2C1_SDA_Sense(void)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[1].gpioPort];
    uint32_t pinpos = (1 << gpioHdmiPins[1].gpioBit);

    return (int)GPIO_ReadInputDataBit(port, pinpos);
}

static void SWI2C1_SCL_Delay(void)
{
    SWI2C_Delay(5);
}

static void SWI2C1_SCL_Set(int high)
{
    #define SCL_SET_TIMEOUT    25

    uint32_t tickEnd;
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[0].gpioPort];
    uint16_t pinpos = 1 << gpioHdmiPins[0].gpioBit;

    if (high)
    {
        /* set High first*/
        GPIO_SetBits(port, pinpos);

        /* timeout checking */
        tickEnd = getSysTime() + SCL_SET_TIMEOUT;
        do
        {
            if (GPIO_ReadInputDataBit(port, pinpos))
                return ;

            SWI2C1_SCL_Delay();
        }while( getSysTime() < tickEnd );

        ASSERT(0);
    }
    else
    {
        GPIO_ResetBits(port, pinpos);
    }
}

static void SWI2C1_SDA_Set(int high)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[1].gpioPort];
    uint16_t pinpos = 1 << gpioHdmiPins[1].gpioBit;

    if (high)
        GPIO_SetBits(port, pinpos);
    else
        GPIO_ResetBits(port, pinpos);
}

#endif

#ifdef HAS_SWI2C2
static void SWI2C2_SCL_Set(int high)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[1].gpioPort];
    uint16_t pinpos = gpioHdmiPins[1].gpioBit;

    if (high)
        GPIO_SetBits(port, pinpos);
    else
        GPIO_ResetBits(port, pinpos);
}

static void SWI2C2_SDA_Set(int high)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint16_t pinpos = gpioHdmiPins[2].gpioBit;

    if (high)
        GPIO_SetBits(port, pinpos);
    else
        GPIO_ResetBits(port, pinpos);
}

static void SWI2C2_SDA_DirSetup(int dir)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint32_t pinpos = gpioHdmiPins[2].gpioBit << 1;

   port->MODER &= ~(GPIO_MODER_MODER0 << pinpos);

    if (dir)
        port->MODER |= GPIO_Mode_OUT << pinpos;
    else
        port->MODER |= GPIO_Mode_IN << pinpos;
}

static int SWI2C2_SDA_Sense(void)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint32_t pinpos = gpioHdmiPins[2].gpioBit << 1;

    return (int)GPIO_ReadInputDataBit(port, pinpos);
}

static void SWI2C2_SCL_Delay(void)
{
    uint32_t delay = 50;
    while(delay--)
    {
        asm("nop");asm("nop");
        asm("nop");asm("nop");
    }
}
#endif

#ifdef HAS_SWI2C3
static void SWI2C3_SCL_Set(int high)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[1].gpioPort];
    uint16_t pinpos = gpioHdmiPins[1].gpioBit;

    if (high)
        GPIO_SetBits(port, pinpos);
    else
        GPIO_ResetBits(port, pinpos);
}

static void SWI2C3_SDA_Set(int high)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint16_t pinpos = gpioHdmiPins[2].gpioBit;

    if (high)
        GPIO_SetBits(port, pinpos);
    else
        GPIO_ResetBits(port, pinpos);
}

static void SWI2C3_SDA_DirSetup(int dir)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint32_t pinpos = gpioHdmiPins[2].gpioBit << 1;

   port->MODER &= ~(GPIO_MODER_MODER0 << pinpos);

    if (dir)
        port->MODER |= GPIO_Mode_OUT << pinpos;
    else
        port->MODER |= GPIO_Mode_IN << pinpos;
}

static int SWI2C3_SDA_Sense(void)
{
    GPIO_TypeDef* port = ioPortMap[gpioHdmiPins[2].gpioPort];
    uint32_t pinpos = gpioHdmiPins[2].gpioBit << 1;

    return (int)GPIO_ReadInputDataBit(port, pinpos);
}

static void SWI2C3_SCL_Delay(void)
{
    asm("nop");
    asm("nop");
    asm("nop");
}
#endif


