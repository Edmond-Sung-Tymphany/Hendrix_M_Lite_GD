/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Gavin Lee
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

#include "stm32f0xx_usart.h"
#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"
#include "product.config"

/****************************************
 * Device: Touch Keys                   *
 ****************************************/
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV
static const tGPIOPin gpioTouchPins[] =
{
    {GPIO_OUT_TCH_360_RST,      IO_PORT_C, IO_BIT_10,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PC10     
    {GPIO_IN_TCH_360_RDY,       IO_PORT_C, IO_BIT_11,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE}, //PC11
    {GPIO_IN_TCH_572_RDY,       IO_PORT_C, IO_BIT_12,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE}, //PC12
    {GPIO_OUT_TCH_572_RST,      IO_PORT_C, IO_BIT_8,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //PC8
 
    //help touch driver reset IO-EXP
	//NOTE: FS1-MK1 do not have this control (only FS2-MK1 have)       
    {GPIO_OUT_IOEXP_RST,   IO_PORT_B, IO_BIT_12,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE}, //PB12

    /* Both touch and IO-Expender use power of this pin, thus we re-define this pin on two array.
     * When modify, remember to modify another one.
     */
    {GPIO_OUT_TCH_POWER,        IO_PORT_C, IO_BIT_9,   GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //PC9
};

static const tGPIODevice gpioTouchConfig =
{
    .deviceInfo.deviceID = TOUCH_KEY_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioTouchPins),
    .pGPIOPinSet = gpioTouchPins
};

static const tI2CDevice i2cTouch360aConfig =
{
    .deviceInfo.deviceID = TOUCH_KEY_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C1
    .pvSlaveCallback = NULL, //not-used parameter
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0xC8, /* 8bit slave address */
};

static const tI2CDevice i2cTouch572Config =
{
    .deviceInfo.deviceID = TOUCH_KEY_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C1
    .pvSlaveCallback = NULL, //not-used parameter
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0xE8, /* 8bit slave address */
};

/* IQS572 bootloader i2c config 
 * Note i2cTouch572BlConfig can not "const", because Azoteq touch drvier
 * will modify (regAddrLen), to support bootloader command with various 
 * length of register address
 */
static /*const */tI2CDevice i2cTouch572BlConfig =
{
    .deviceInfo.deviceID = TOUCH_KEY_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C1
    .pvSlaveCallback = NULL, //not-used parameter
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0x68, /* 8bit slave address */
};

static const tAzIntegTouchKeyboardDevice touchKeyConfig =
{
    .touchKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .touchKeyboard.deviceInfo.deviceType = AZ_INTEG_TOUCH_KEY_DEV_TYPE,
    .touchKeyboard.keyNum = NUM_OF_AZ_INTEG_TOUCH_KEY,  // == ArraySize(azIntegTouchKey)
    .i2cConfig572= &i2cTouch572Config,
    .i2cConfig572Bl= &i2cTouch572BlConfig,
    .i2cConfig360a= &i2cTouch360aConfig,
    .gpioConfig= &gpioTouchConfig,
};
#endif /* #ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV */

#ifdef PT_I2C_DEBUG
static const tI2CDevice i2cDbgConfig1 =
{
    .deviceInfo.deviceID = DBG_I2C1_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C2
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = I2C_CLK_KHZ,
};
static const tI2CDevice i2cDbgConfig2 =
{
    .deviceInfo.deviceID = DBG_I2C2_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,  //not-used parameter
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = I2C_CLK_KHZ,
};
#endif /* #ifdef PT_I2C_DEBUG */

/****************************************
 * Device: ADC Keys                     *
 ****************************************/
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
#define ADC_THRESHOLD_LEVEL_2   400
#define ADC_THRESHOLD_LEVEL_3   1600
#define ADC_THRESHOLD_LEVEL_4   2400
#define ADC_THRESHOLD_RANGE_FACTORY_RESET_KEY   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_SOFT_AP_KEY         ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4

/* When add/remove items in attrADCPinsForKey[], 
 * remember to update (NUM_OF_ADC_KEY_PIN) on attachedDevices.h
 */
static const tAdcPinIoAttr attrADCPinsForKey[NUM_OF_ADC_KEY_PIN] =
{
    {ADC_FS_KEY , ADC_CHANNEL1, IO_PORT_A, IO_BIT_1}, //PA1, ADC_IN1
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = NUM_OF_ADC_KEY_PIN, // == ArraySize(attrADCPinsForKey)
    .pAdcPinIoAttr = attrADCPinsForKey
};

/* When add/remove items in adcKey[], 
 * remember to update (NUM_OF_ADC_KEY) on attachedDevices.h 
 */
static const tAdcKey adcKey[NUM_OF_ADC_KEY]=
{   /* key_id         key_ADC_pin        adc threshold */
    {FACTORY_RESET_KEY,  ADC_FS_KEY ,      ADC_THRESHOLD_RANGE_FACTORY_RESET_KEY}, //adc value: 70
    {SOFT_AP_KEY,        ADC_FS_KEY ,      ADC_THRESHOLD_RANGE_SOFT_AP_KEY},       //adc value: 2068
};

static tAdcKeyboardDevice adcKeyConfig = // const
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = NUM_OF_ADC_KEY,  // == ArraySize(adcKey)
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pAdcKeySet = adcKey
};

static const tGPIOPin gpioRotaterPins[] =
{
    {GPIO_IN_ROTARY_VOL_A, IO_PORT_B, IO_BIT_3, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE}, //PB3
    {GPIO_IN_ROTARY_VOL_B, IO_PORT_D, IO_BIT_2, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE}, //PD2
};

static const tGPIODevice gpioRotaterConfig =
{
    .deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .deviceInfo.deviceType = GPIO_DEV_TYPE,
    .usedGPIOPinNum =  ArraySize(gpioRotaterPins),
    .pGPIOPinSet = gpioRotaterPins
};

static tRotaterKeyboardDevice rotaterKeyConfig =
{
    .kbDev.deviceInfo.deviceID   = KEYBOARD_DEV_ID,
    .kbDev.deviceInfo.deviceType = ROTATER_DEV_TYPE,
    .kbDev.keyNum = 2,  // only : CW and CCW
    .gpioConfig = &gpioRotaterConfig,
    .keyCw      = VOLUME_UP_KEY,
    .keyCcw     = VOLUME_DOWN_KEY,
};


/****************************************
 * Device: ADC (not key)                *
 ****************************************/
/* When add/remove adc pins, 
 * remember to update (NUM_OF_ALL_ENABLED_ADC_PIN) on attachedDevices.h
 */
static const tAdcPinIoAttr attrAdcPowerPins[NUM_OF_ADC_POWER_PIN] =
{
    {ADC_5V_SEN,        ADC_CHANNEL6,   IO_PORT_A,  IO_BIT_6},    //PA6, ADC_IN6
    {ADC_PVDD_SEN,      ADC_CHANNEL7,   IO_PORT_A,  IO_BIT_7},    //PA7, ADC_IN7
    {ADC_HW_VER,        ADC_CHANNEL14,  IO_PORT_C,  IO_BIT_4},    //PC4, ADC_IN14
};

static const tADCDevice adcPowerConfig =
{
    .deviceInfo.deviceID = POWER_DEV_ID,
    .deviceInfo.deviceType = ADC_DEV_TYPE,
    .ADCEnabledPinNum = NUM_OF_ADC_POWER_PIN, // =ArraySize(attrAdcPowerPins)
    .pAdcPinIoAttr = attrAdcPowerPins
};


/* When add/remove adc pins, 
 * remember to update (NUM_OF_ALL_ENABLED_ADC_PIN) on attachedDevices.h
 */
static const tAdcPinIoAttr attrAdcAudioPins[NUM_OF_ADC_AUDIO_PIN] =
{
    {ADC_NTC_WF_AMP,    ADC_CHANNEL10,  IO_PORT_C,  IO_BIT_0},    //PC0, ADC_IN10
    {ADC_NTC_TW_SPK,    ADC_CHANNEL11,  IO_PORT_C,  IO_BIT_1},    //PC1, ADC_IN11
    {ADC_NTC_WF_SPK,    ADC_CHANNEL12,  IO_PORT_C,  IO_BIT_2},    //PC2, ADC_IN12
};

static const tADCDevice adcAudioConfig =
{
    .deviceInfo.deviceID = AUDIO_DEV_ID,
    .deviceInfo.deviceType = ADC_DEV_TYPE,
    .ADCEnabledPinNum = NUM_OF_ADC_AUDIO_PIN, // =ArraySize(attrAdcPowerPins)
    .pAdcPinIoAttr = attrAdcAudioPins
};



/****************************************
 * Device: GPIO Keys                     *
 ****************************************/
static const tGPIOPin gpioKeyPins[NUM_OF_GPIO_KEY_PIN] =
{
    {GPIO_IN_POWER_KEY, IO_PORT_C,  IO_BIT_14,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, //PC14    
};

static const tGPIODevice GPIOKeyConfig =
{
    .usedGPIOPinNum =  NUM_OF_GPIO_KEY_PIN,
    .pGPIOPinSet = gpioKeyPins
};


/* When add/remove items in gpioKeysSet[], 
 * remember to update (NUM_OF_GPIO_KEY) on attachedDevices.h 
 */
static const tGpioKey gpioKeysSet[NUM_OF_GPIO_KEY] =
{   /* key_id           */
    {POWER_KEY,    GPIO_IN_POWER_KEY}, //PC14
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
static const tGPIOPin gpioPowerPins[] =
{
    //Normal
    {GPIO_IN_DC_IN,        IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,        ODC_NOT_APPLICABLE},  //PB15  (FS1:PB15, FS2:PC6)
    {GPIO_IN_CH_STATE,     IO_PORT_B,  IO_BIT_5,   GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,        ODC_NOT_APPLICABLE},  //PB5
    {GPIO_OUT_CH_ON,       IO_PORT_B,  IO_BIT_9,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //PB9
    {GPIO_OUT_SLOW_CH,     IO_PORT_C,  IO_BIT_5,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PC5, low:1.9A, high:1.2A
    // low power mode for 360 when system go to standby mode
    {GPIO_OUT_TCH_360_RST,      IO_PORT_C, IO_BIT_10,  GPIO_DIGITAL_OUTPUT,    GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PC10
    //Note gpioTouchPins[] also have GPIO_IN_TCH_572_RDY.
    {GPIO_IN_TCH_360_RDY,       IO_PORT_C, IO_BIT_11,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE}, //PC11
    {GPIO_IN_TCH_572_RDY,       IO_PORT_C, IO_BIT_12,  GPIO_DIGITAL_INPUT,     GPIO_ACTIVE_FLOATING,  ODC_NOT_APPLICABLE}, //PC12
    
};

static const tGPIODevice gpioPowerConfig =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPowerPins),
  .pGPIOPinSet = gpioPowerPins
};


/****************************************
 * Device: ioexpander reset ctrl         *
 ****************************************/
static const tGPIOPin gpioIoexpanderPin[] =
{
    {GPIO_OUT_IOEXP_RST,   IO_PORT_B, IO_BIT_12,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,    ODC_DISABLE}, //PB12

    /* Both touch and IO-Expender use power of this pin, thus we re-define this pin on two array.
     * When modify, remember to modify another one.
     */
    {GPIO_OUT_TCH_POWER,   IO_PORT_C, IO_BIT_9,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //PC9
};

static const tGPIODevice gpioIoexpanderConfig =
{
  .deviceInfo.deviceID = IO_EXPANDER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioIoexpanderPin),
  .pGPIOPinSet = gpioIoexpanderPin
};

/****************************************
 * Device: DSP                          *
 ****************************************/
static const tGPIOPin gpioDspPins[] =
{
    {GPIO_OUT_DSP_RST_N,   IO_PORT_B,  IO_BIT_2,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PB2 
    {GPIO_OUT_BOOST_CAP1,  IO_PORT_B,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PB1
    {GPIO_OUT_BOOST_CAP2,  IO_PORT_C,  IO_BIT_7,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PC7
    {GPIO_OUT_BOOST_EN,    IO_PORT_B,  IO_BIT_4,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PB4 
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
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0x70,
};



/****************************************
 * Device: AMP TAS5760                  *
 ****************************************/
static const tI2CDevice i2cAmpWooferConfig = //Woofer Amplifier
{
    .deviceInfo.deviceID = AMP_WOOFER_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0xDA,
};

static const tI2CDevice i2cAmpTweeterConfig = //Tweeter Amplifier
{
    .deviceInfo.deviceID = AMP_TWEETER_1_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_TWO, //I2C2
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0xD8,
};




/****************************************
 * Device: Audio                        *
 ****************************************/
static const tGPIOPin gpioAudioPins[] =
{
    {GPIO_IN_DSP_TUNE,     IO_PORT_A,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,       ODC_NOT_APPLICABLE}, //PA15
    {GPIO_OUT_SYSPWR_ON,   IO_PORT_B,  IO_BIT_0,   GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PB0
    {GPIO_IN_AMP_FAULTZ,   IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,        ODC_NOT_APPLICABLE},  //PB14. SPK_FAULT is open drain, MCU need to week pull low
    {GPIO_OUT_AMP_SDZ,     IO_PORT_B,  IO_BIT_13,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,   ODC_DISABLE}, //PB13
};
static const tGPIODevice gpioAudioConfig =
{
  .deviceInfo.deviceID    = AUDIO_DEV_ID,
  .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioAudioPins),
  .pGPIOPinSet = gpioAudioPins
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
    {GPIO_IN_ASE_REQ_PDN,       IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,      ODC_NOT_APPLICABLE},  //PA8 
    
    //TODO: A9-MK3 can control ASE 5V, FSx-MK2 may support it on 
//    {GPIO_OUT_ASE_5V_EN,        IO_PORT_C,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},//PC7
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
static /*const */tIoExpanderLedMap ioeLedMap[] =
{ // ledID             pure_color    port   pin
    {LED_CONN_WHT,    WHITE,     0,    0}, // IO-Expander OUT0
    {LED_CONN_BLUE,   BLUE,      0,    1}, // IO-Expander OUT1
    {LED_CONN_ORG,    ORANGE,    0,    2}, // IO-Expander OUT2
    {LED_CONN_RED,    RED,       0,    3}, // IO-Expander OUT3
    {LED_PROD_RED,    RED,       0,    4}, // IO-Expander OUT4
    {LED_PROD_WHT,    WHITE,     0,    5}, // IO-Expander OUT5
};

static /*const */tI2CDevice  ioExpanderi2cDeviceConf =
{
    .deviceInfo.deviceID= IO_EXPANDER_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode            = I2C_MASTER_MODE,
    .channel            = I2C_CHANNEL_ONE,  // I2C1
    .pvSlaveCallback    = NULL,
    .regAddrLen         = REG_LEN_8BITS,
    .baudRate           = I2C_CLK_KHZ,
    .devAddress         = 0xB6,
};

static const tIoeLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = IO_EXPANDER_DEV_TYPE,
    .ledNum                 = ArraySize(ioeLedMap),
    .pIoExpanderLedMap      = ioeLedMap,
    .i2cDeviceConf = &ioExpanderi2cDeviceConf,
};




/****************************************
 * Device: Battery                      *
 ****************************************/
static const tI2CDevice i2cBattConfig =
{
    .deviceInfo.deviceID = BATT_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE, //I2C1
    .pvSlaveCallback = NULL,//not-used parameter
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = I2C_CLK_KHZ,
    .devAddress = 0x16, //8bit slave address
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
    //ASE
    (tDevice*)&uartAsetkConfig,
    (tDevice*)&gpioAsetkConfig, 

    //KEY
    (tDevice*)&adcKeyConfig,
    (tDevice*)&gpioKeyConfig,
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV    
    (tDevice*)&touchKeyConfig,    
#endif    
    (tDevice*)&rotaterKeyConfig,

    //POWER
    (tDevice*)&gpioPowerConfig,
    (tDevice*)&adcPowerConfig,
    
    //Audio
    (tDevice*)&gpioAudioConfig,
    (tDevice*)&gpioDspConfig,
    (tDevice*)&i2cDspConfig, 
    (tDevice*)&i2cAmpWooferConfig,
    (tDevice*)&i2cAmpTweeterConfig,
    (tDevice*)&adcAudioConfig,
    
//LED
#ifdef HAS_LEDS
    (tDevice*)&ledConfig,
#endif    
    
    //Other
    (tDevice*)&i2cBattConfig, 
    (tDevice*)&nvmConfig,
    (tDevice*)&gpioIoexpanderConfig,
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


/* TODO: Let UartDrv support flexible UART selection. Ex.
 *       gUartMap= {USART1, USART8}
 */
USART_TypeDef *gUartMap[NUM_OF_UART] =
{
    USART1,
    USART2,
    USART3,
    USART4,
    USART5,
    USART6,
    USART7,
    USART8
};

const uint8_t gUartNvicIrq[NUM_OF_UART] =
{
    USART1_IRQn,
    USART2_IRQn,
    USART3_8_IRQn,
    USART3_8_IRQn,
    USART3_8_IRQn,
    USART3_8_IRQn,
    USART3_8_IRQn,
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

    case TP_UART_DEV_3: //UART3 (not used)
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1ENR_USART3EN,ENABLE);

        /* Configure the HSI as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_1); //PC4
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_1); //PC5

        /* Configure USART3 pins:  Rx and Tx ----------------------------*/
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_UP;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        break;
        
    case TP_UART_DEV_8: //UART8 (not used)
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART8EN,ENABLE);
		
        /* Configure the HSI as USART clock */
        RCC->CFGR3 &= ~RCC_CFGR3_USART3SW;
		
#if defined(UART_CLK_SOURCE_PCLK)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_PCLK;
#elif defined(UART_CLK_SOURCE_HSI)
        RCC->CFGR3 |= RCC_CFGR3_USART3SW_HSI;
#endif

        GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_1); //PC8, TX
        GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_1); //PC9, RX

        /* Configure USART8 pins:  Rx and Tx ----------------------------*/
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8 | GPIO_Pin_9;
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
    case TP_UART_DEV_3: //UART3 (not used)
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_4 | GPIO_Pin_5;
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        break;
		
    case TP_UART_DEV_8: //UART8 (not used)
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_8 | GPIO_Pin_9; //PC8, PC9
        GPIO_InitStructure.GPIO_Speed   = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_Mode    = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType   = GPIO_OType_OD;
        GPIO_InitStructure.GPIO_PuPd    = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOC, &GPIO_InitStructure);
        break;
		
    default:
        ASSERT(0);
        break;
    }
}

//I2C1 for Battery & IO Expender, Touch(IQS572, IQS333)
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
    //GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    //GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    //GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//I2C2 for DSP, AMPx2
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
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*!< Configure sEE_I2C pins: SDA */
//  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void I2C2_LowLevel_Deinit(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /*!< GPIO configuration */
  /*!< Configure sEE_I2C pins: SCL */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_ResetBits(GPIOB, GPIO_InitStructure.GPIO_Pin);
}


