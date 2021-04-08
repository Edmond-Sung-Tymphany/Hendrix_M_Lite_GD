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
#include "Setting_id.h"
#include "product.config"

/* @warning
 * This struct cannot be defined in deviceTypes.h as the enum eLed cannot be forward declared
 * enum eLed should be defined in attachedDevices.h as it can be different among projects
 * the workaround is forward declared tIoExpanderLed in deviceTypes.h
 */

static const tGPIOPin gpioPinsForKey[] =
{
//    {GPIO_0, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be
    {GPIO_1, IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // source (input key), PA0
    {GPIO_2, IO_PORT_A,  IO_BIT_1,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // bt (bt key), PA1


};

static const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  ArraySize(gpioPinsForKey),
  .pGPIOPinSet = gpioPinsForKey
};

static const tGPIOPin gpioPinsForBT[] =
{
     {GPIO_3, IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB14   LED0
     {GPIO_4, IO_PORT_B,  IO_BIT_15,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, // PB15   LED1
/*______*/
     {GPIO_5, IO_PORT_B,  IO_BIT_7,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB7
     {GPIO_6, IO_PORT_B,  IO_BIT_6,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB6
     {GPIO_7, IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB5
     {GPIO_8, IO_PORT_B,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB4
     {GPIO_9, IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //PB3


     {GPIO_10, IO_PORT_A, IO_BIT_15,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //BT_PWR_EN  PA15
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
    {GPIO_11, IO_PORT_A, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //BT RESET, PA12
    {GPIO_12, IO_PORT_F, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP TAS5711 RESET, PF6
    {GPIO_13, IO_PORT_F, IO_BIT_7, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},   //AMP PDN, PF7
    {GPIO_14, IO_PORT_A, IO_BIT_6, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE},    // EN_+3V3 - PA6; probably set to  GPIO_ACTIVE_FLOATING
    {GPIO_15, IO_PORT_A, IO_BIT_7, GPIO_DIGITAL_INPUT, GPIO_ACTIVE_HIGH, ODC_NOT_APPLICABLE},    // AUDIO_DET PA7

    {GPIO_16, IO_PORT_A,  IO_BIT_2,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, // power key, PA2 -todo: check if GPIO_ACTIVE_HIGH have to be

    {GPIO_17, IO_PORT_B, IO_BIT_12, GPIO_DIGITAL_OUTPUT, GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //  ADC_3.3V_EN
};

static const tGPIODevice GPIOConfigForPower =
{
  .deviceInfo.deviceID = POWER_DEV_ID,
  .deviceInfo.deviceType = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForPower),
  .pGPIOPinSet = gpioPinsForPower
};


static const tGPIOPin gpioPinsForLed[] =
{
    {GPIO_21,  IO_PORT_B,  IO_BIT_1,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //LED AUX2 - RCA
    {GPIO_22,  IO_PORT_B,  IO_BIT_2, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //LED AUX1 - AUX IN
    {GPIO_23,  IO_PORT_B,  IO_BIT_10, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //LED BT - BT
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
    {ADC_PIN3, ADC_CHANNEL3,IO_PORT_A, IO_BIT_3}, // PA3 - volume
    {ADC_PIN4, ADC_CHANNEL4,IO_PORT_A, IO_BIT_4}, // PA4 - bass
    {ADC_PIN5, ADC_CHANNEL5,IO_PORT_A, IO_BIT_5}, // PA5 - treble
};
static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = ArraySize(attrADCPinsForKey),
    .pAdcPinIoAttr = attrADCPinsForKey
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

// will be redefine
//#define ADC_THRESHOLD_LEVEL_3   165
//#define ADC_THRESHOLD_LEVEL_4   315

//#define ADC_THRESHOLD_LEVEL_5   330
//#define ADC_THRESHOLD_LEVEL_6   480

//#define ADC_THRESHOLD_LEVEL_7   495
//#define ADC_THRESHOLD_LEVEL_8   645

//#define ADC_THRESHOLD_LEVEL_9   660
//#define ADC_THRESHOLD_LEVEL_10   810

//#define ADC_THRESHOLD_LEVEL_11   825
//#define ADC_THRESHOLD_LEVEL_12   975

//#define ADC_THRESHOLD_LEVEL_13   990
//#define ADC_THRESHOLD_LEVEL_14   1140

//#define ADC_THRESHOLD_LEVEL_15   1155
//#define ADC_THRESHOLD_LEVEL_16   1305

//#define ADC_THRESHOLD_LEVEL_17   1320
//#define ADC_THRESHOLD_LEVEL_18   1470

//#define ADC_THRESHOLD_LEVEL_19   1485
//#define ADC_THRESHOLD_LEVEL_20   1650


#define ADC_THRESHOLD_RANGE    0, 4095

#define ADC_THRESHOLD_FAKE_RANGE    13001, 13000

//#define ADC_THRESHOLD_LEVEL_TEST_1   1000
//#define ADC_THRESHOLD_LEVEL_TEST_2   1600
//#define ADC_THRESHOLD_RANGE_TEST   ADC_THRESHOLD_LEVEL_TEST_1,ADC_THRESHOLD_LEVEL_TEST_2

static const tLinearKnobKey linearKnobKeys[]=
{   /* key_id         key_ADC_channel  settingID, resolution  deltaBuffer adc threshold */
    {VOLUME_KNOB_BASE_KEY_ID, ADC_PIN3, SETID_VOLUME_ADC_MIN, SETID_VOLUME_ADC_MAX, 30,    4,  ADC_THRESHOLD_RANGE},
    {BASS_KNOB_BASE_KEY_ID,   ADC_PIN4, SETID_BASS_ADC_MIN,   SETID_BASS_ADC_MAX,    9,    4,  ADC_THRESHOLD_RANGE},
    {TREBLE_KNOB_BASE_KEY_ID, ADC_PIN5, SETID_TREBLE_ADC_MIN, SETID_TREBLE_ADC_MAX,  9,    4,  ADC_THRESHOLD_RANGE},
};
static tLinearKnobKeyDevice linearKnobKeyConfig = // const
{
    .linearKnobKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .linearKnobKeyboard.deviceInfo.deviceType = LINEAR_KNOB_DEV_TYPE,
    .linearKnobKeyboard.keyNum = NUM_OF_LINEAR_KNOB_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pLinearKnobKeySet = linearKnobKeys
};


static const tGpioKey gpioKeys[] =
{   /* key_id           */
    {INPUT_KEY,      GPIO_1},
    {BT_KEY,         GPIO_2},


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

static const tI2CDevice dsp_Stanmore_Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .address = 0x30, /* device address */
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400 //KHz,
};

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

static const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};


const tDevice * const devices[] =
{
    (tDevice*)&linearKnobKeyConfig,
    (tDevice*)&gpioKeyboardConfig,
    (tDevice*)&dsp_Stanmore_Config,
    (tDevice*)&GPIOConfigForBT,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&gpioConfigForLed,
    (tDevice*)&nvmConfig,
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


