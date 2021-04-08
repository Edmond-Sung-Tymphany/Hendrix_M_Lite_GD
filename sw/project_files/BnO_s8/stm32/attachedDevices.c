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
#include "irLearningDrv.h"

/* @warning
 * This struct cannot be defined in deviceTypes.h as the enum eLed cannot be forward declared
 * enum eLed should be defined in attachedDevices.h as it can be different among projects
 * the workaround is forward declared tIoExpanderLed in deviceTypes.h
 */
/*********************************************************************************************************/

const tPwmLedMap pwmLedMap[] =
{
                /* GPIO Num, Output Compare Reg Num, port num & bit num */
    {LED_RED,   COLOR_COMPONENT_RED,    {NULL, IO_PORT_A, IO_BIT_11, PWM_CH4}},
    {LED_BLUE,  COLOR_COMPONENT_BLUE,   {NULL, IO_PORT_B, IO_BIT_1,  PWM_CH3N}},
    {LED_GREEN, COLOR_COMPONENT_GREEN,  {NULL, IO_PORT_A, IO_BIT_8,  PWM_CH1}},
};


const tPwmLedDevice ledConfig = {
    .deviceInfo.deviceID    = LED_DEV_ID,
    .deviceInfo.deviceType  = PWM_DEV_TYPE,
    .ledNum                 = ArraySize(pwmLedMap),
    .pPwmLedMap      = pwmLedMap,
};
/*********************************************************************************************************/
/*********************************************************************************************************/
const tGPIOPin gpioPinsForKey[] =
{
    {GPIO_0, IO_PORT_A,  IO_BIT_7,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},
    //{GPIO_0, IO_PORT_A,  IO_BIT_0,  GPIO_DIGITAL_INPUT,  GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},        // EVAL BOARD
};

const tGPIODevice GPIOConfigForKey =
{
  .usedGPIOPinNum =  ArraySize(gpioPinsForKey),
  .pGPIOPinSet = gpioPinsForKey
};
/*********************************************************************************************************/
const tGPIOPin gpioPinsForAudio[] =
{
//    {GPIO_19,  IO_PORT_B,  IO_BIT_2,   GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MUTE     
//    {GPIO_20,  IO_PORT_B,  IO_BIT_10, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SDZ 
    {GPIO_21,  IO_PORT_B,  IO_BIT_3,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //SPDIF0 
    {GPIO_22,  IO_PORT_A,  IO_BIT_15, GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE}, //SPDIF1
    {GPIO_23,  IO_PORT_A,  IO_BIT_6,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},  //DET_IN1      
    {GPIO_24,  IO_PORT_A,  IO_BIT_5,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_LOW,  ODC_NOT_APPLICABLE},  //DET_IN2
    {GPIO_25,  IO_PORT_A,  IO_BIT_3,  GPIO_DIGITAL_INPUT,   GPIO_ACTIVE_HIGH,  ODC_NOT_APPLICABLE},  //RJ45_SENSE_IN
};

const tGPIODevice GPIOConfigForAudio =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
  .usedGPIOPinNum =  ArraySize(gpioPinsForAudio),
  .pGPIOPinSet = gpioPinsForAudio
};

/*********************************************************************************************************/
static const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_5,  IO_PORT_F,  IO_BIT_0,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE}, //CODEC_RST_PIN
    {GPIO_6,  IO_PORT_C,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //GREEN_LED_PIN
    {GPIO_7,  IO_PORT_B,  IO_BIT_0,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //PWR_EN_PIN
    {GPIO_8,  IO_PORT_A,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //RELAY_SW
    {GPIO_9,  IO_PORT_B,  IO_BIT_5,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //OPAMP_MUTE
    {GPIO_10,  IO_PORT_A,  IO_BIT_4,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE},  //RJ45_SENSE_OUT
};

static const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = ArraySize(gpioPinsForPower),
    .pGPIOPinSet            = gpioPinsForPower
};

/*********************************************************************************************************/
/*********************************************************************************************************/
static const tAdcPinIoAttr attrADCPinsForAudio[] = 
{
  {ADC_PIN4, ADC_CHANNEL1,IO_PORT_A, IO_BIT_1},     // PLINK_DET
  {ADC_PIN5, ADC_CHANNEL2,IO_PORT_A, IO_BIT_2},     // AUX_DET
};

static const tADCDevice ADCConfigForAudio =
{
    .deviceInfo.deviceID    = AUDIO_DEV_ID,
    .deviceInfo.deviceType  = ADC_DEV_TYPE,
    .ADCEnabledPinNum = ArraySize(attrADCPinsForAudio),
    .pAdcPinIoAttr = attrADCPinsForAudio
};
/*********************************************************************************************************/


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


const tGpioKey gpioKeys[] =
{   /* key_id           */
    {POWER_KEY,         GPIO_0},
};

tGpioKeyboardDevice gpioKeyboardConfig =
{
    .gpioKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .gpioKeyboard.deviceInfo.deviceType = GPIO_DEV_TYPE,
    .gpioKeyboard.keyNum = ArraySize(gpioKeys),
    .attachedDeviceObjConfig = &GPIOConfigForKey,
    .pGpioKeySet = gpioKeys
};




const tIrKey irKeys[]=
{
    {MUTE_KEY,         NULL},
    {VOLUME_UP_KEY,         NULL},
    {VOLUME_DOWN_KEY,         NULL},
};


/* end of keyboard attached devices setting */

const tI2CDevice dsp_BnO_s8_Config =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .address = 0x98, /* device address */
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 400 //KHz
};


const tUARTDevice UartDebugConfig = {
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

const tStorageDevice nvmConfig = {
    .deviceInfo.deviceID    = INT_FLASH_DEV_ID,
    .deviceInfo.deviceType  = NVM_DEV_TYPE,
};

const tDevice * const devices[] =
{
    (tDevice*)&ledConfig,
    //(tDevice*)&adcKeyboardConfig,
    (tDevice*)&gpioKeyboardConfig,
    //(tDevice*)&irKeyboardConfig,
    (tDevice*)&dsp_BnO_s8_Config,
    (tDevice*)&UartDebugConfig,
    (tDevice*)&nvmConfig,
    (tDevice*)&GPIOConfigForPower,
    (tDevice*)&GPIOConfigForAudio,
    (tDevice*)&ADCConfigForAudio,
    
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

void sEE_LowLevel_Init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  /* Configure the I2C clock source. The clock is derived from the HSI */
  RCC_I2CCLKConfig(RCC_I2C1CLK_HSI);

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

  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

