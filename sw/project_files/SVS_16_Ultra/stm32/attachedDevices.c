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


static const tGPIOPin powerGpioPins[] =
{
    {GPIO_0,  IO_PORT_B,  IO_BIT_1,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, //TRIGER
    {GPIO_1,  IO_PORT_A,  IO_BIT_4,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE}, //Amplify fail sig
    {GPIO_2,  IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //MCU Power on
    {GPIO_3,  IO_PORT_B,  IO_BIT_15, GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //AMP Stand by
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = NUM_OF_GPIO_POWER_PIN,
    .pGPIOPinSet            = powerGpioPins
};


static const tGPIOPin ampMuteGpioPin[] =
{
    {GPIO_4,  IO_PORT_B,  IO_BIT_2,  GPIO_DIGITAL_OUTPUT,  GPIO_DEFAULT_OUT_HIGH, ODC_DISABLE}, //Amplify mute
};

static const tGPIODevice GPIOConfigForAmpMute =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = NUM_OF_GPIO_AMP_MUTE_PIN,
    .pGPIOPinSet            = ampMuteGpioPin
};

/* Amplify adc object and config */
static const tAdcPinIoAttr attrADCPinsForAmp[] =
{
  {ADC_PIN5, ADC_CHANNEL5,IO_PORT_A, IO_BIT_5}, // temperature 
  {ADC_PIN7, ADC_CHANNEL7,IO_PORT_A, IO_BIT_7}  // for AC detect
};

static const tADCDevice ADCConfigForAMP =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = NUM_OF_ADC_AMP_PIN,
    .pAdcPinIoAttr = attrADCPinsForAmp
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
#define ADC_THRESHOLD_LEVEL_1   0
#define ADC_THRESHOLD_LEVEL_2   500

#define ADC_THRESHOLD_LEVEL_3   750
#define ADC_THRESHOLD_LEVEL_4   1800

#define ADC_THRESHOLD_LEVEL_5   1800
#define ADC_THRESHOLD_LEVEL_6   2600

#define ADC_THRESHOLD_LEVEL_7   2600
#define ADC_THRESHOLD_LEVEL_8   3500

#define ADC_THRESHOLD_RANGE_1   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_2   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4
#define ADC_THRESHOLD_RANGE_3   ADC_THRESHOLD_LEVEL_5,ADC_THRESHOLD_LEVEL_6
#define ADC_THRESHOLD_RANGE_4   ADC_THRESHOLD_LEVEL_7,ADC_THRESHOLD_LEVEL_8

static const tAdcPinIoAttr attrADCPinsForKey[] =
{
  {ADC_PIN6, ADC_CHANNEL6,IO_PORT_A, IO_BIT_6},
};

static const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = NUM_OF_ADC_KEY_PIN,
    .pAdcPinIoAttr = attrADCPinsForKey
};

static const tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {NEXT_KEY,          ADC_PIN6,   ADC_THRESHOLD_RANGE_1},
    {RETURN_KEY,        ADC_PIN6,   ADC_THRESHOLD_RANGE_2},
    {PREV_KEY,          ADC_PIN6,   ADC_THRESHOLD_RANGE_3},
    {ENTER_KEY,         ADC_PIN6,   ADC_THRESHOLD_RANGE_4},    
};

static tAdcKeyboardDevice adcKeyboardConf =
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = NUM_OF_ADC_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pAdcKeySet = adcKeys
};

/* IR key configuration */
#define POWER_IR_CODE          0X020240BF
#define VOL_DOWN_IR_CODE       0x020220DF
#define VOL_UP_IR_CODE         0x0202609F
#define PREV_IR_CODE           0x0202906F
#define NEXT_IR_CODE           0x0202B04F
#define ENTER_IR_CODE          0x0202708F
#define BACK_IR_CODE           0x020230CF
#define MUTE_IR_CODE           0X0202906F
#define PRESET1_IR_CODE        0x020228D7
#define PRESET2_IR_CODE        0x0202A857
#define PRESET3_IR_CODE        0x02026897
#define BRIGHTNESS_BTN_IR_CODE 0x02028877

static tIrKeyIdMapCode keyIdIrCodeMap[NUM_OF_IR_KEY] = 
{
    {SCREEN_OFF_ON_KEY,     BRIGHTNESS_BTN_IR_CODE},
    {IR_VOLUME_UP_KEY,      VOL_UP_IR_CODE},
    {IR_VOLUME_DOWN_KEY,    VOL_DOWN_IR_CODE},
    {PREV_KEY,              PREV_IR_CODE},
    {NEXT_KEY,              NEXT_IR_CODE},
    {ENTER_KEY,             ENTER_IR_CODE},
    {RETURN_KEY,            BACK_IR_CODE},
    {IR_PRESET_KEY_1,       PRESET1_IR_CODE},
    {IR_PRESET_KEY_2,       PRESET2_IR_CODE},
    {IR_PRESET_KEY_3,       PRESET3_IR_CODE},
    {MUTE_KEY,              MUTE_IR_CODE},
};

static tIRKeyboardDevice IrKeyBoardConfig =
{
    .irKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .irKeyboard.deviceInfo.deviceType = INFRARED_DEV_TYPE,
    .irKeyboard.keyNum = NUM_OF_IR_KEY,
    .keyIdIrCodeMap = keyIdIrCodeMap,
    .ioAttr = 
      {
        GPIO_10,
        IO_PORT_A,
        IO_BIT_11,
        GPIO_DIGITAL_INPUT,
        GPIO_ACTIVE_HIGH,
        ODC_DISABLE
      }
};

/* end of keyboard attached devices setting */


static const tI2CDevice dspSvsConfig =
{
    .deviceInfo.deviceID = AUDIO_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .regAddrLen = REG_LEN_16BITS,
    .baudRate = 100,//KHz
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
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

static const tUARTDevice UartBleConfig = {
    .deviceInfo.deviceID    = BLE_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_2,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

static const tGPIOPin displayGpioPins[] =
{
    {GPIO_10,  IO_PORT_B,  IO_BIT_11,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*STB*/
    {GPIO_11,  IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*CLK*/
    {GPIO_12,  IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*DIO*/
};

static const tGPIODevice displayGpioConf =
{
    .deviceInfo.deviceID    = DISPLAY_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = NUM_OF_GPIO_DISPLAY_PIN,
    .pGPIOPinSet            = displayGpioPins
};

static const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

const tDevice * const devices[] =
{
    (tDevice*)&adcKeyboardConf,
    (tDevice*)&displayGpioConf,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&dspSvsConfig,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&nvmConfig,
    (tDevice*)&UartBleConfig,
    (tDevice*)&IrKeyBoardConfig,
    (tDevice*)&ADCConfigForAMP,
    (tDevice*)&GPIOConfigForAmpMute
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
    case TP_UART_DEV_2:
      {
        GPIO_InitTypeDef GPIO_InitStructure;

        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_1);

        /* Configure USART2(BLE) pins:  Rx and Tx */
        GPIO_InitStructure.GPIO_Pin     = GPIO_Pin_2 | GPIO_Pin_3;
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


