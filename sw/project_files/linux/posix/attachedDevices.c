/**
* @file attachedDevices.c
* @brief The devices attached to the product.
* @author Christopher Alexander, Wesley Lee
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/


#include "deviceTypes.h"
#include "attachedDevices.h"
#include "trace.h"

#include "product.config"

const tGPIOPin gpioPinsForPower[] =
{
    {GPIO_5,  IO_PORT_A,  IO_BIT_8,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SOL_ON_PIN
};

const tGPIODevice GPIOConfigForPower =
{
    .deviceInfo.deviceID    = POWER_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = NUM_OF_GPIO_POWER_PIN,
    .pGPIOPinSet            = gpioPinsForPower
};

const tGPIOPin gpioPinsForAudio[] =
{
    {GPIO_8,  IO_PORT_B,  IO_BIT_2,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //MUTE     
    {GPIO_9,  IO_PORT_B,  IO_BIT_10, GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_HIGH,  ODC_DISABLE}, //SDZ 
};

const tGPIODevice GPIOConfigForAudio =
{
  .usedGPIOPinNum =  NUM_OF_GPIO_AUDIO_PIN,
  .pGPIOPinSet = gpioPinsForAudio
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
#define ADC_THRESHOLD_LEVEL_2   700
#define ADC_THRESHOLD_LEVEL_3   1800
#define ADC_THRESHOLD_LEVEL_4   2500
#define ADC_THRESHOLD_LEVEL_5   3500
#define ADC_THRESHOLD_RANGE_1   ADC_THRESHOLD_LEVEL_1,ADC_THRESHOLD_LEVEL_2
#define ADC_THRESHOLD_RANGE_2   ADC_THRESHOLD_LEVEL_2,ADC_THRESHOLD_LEVEL_3
#define ADC_THRESHOLD_RANGE_3   ADC_THRESHOLD_LEVEL_3,ADC_THRESHOLD_LEVEL_4
#define ADC_THRESHOLD_RANGE_4   ADC_THRESHOLD_LEVEL_4,ADC_THRESHOLD_LEVEL_5

static tAdcPinIoAttr ADCPinsForKey[] =
{
  {ADC_PIN3, 6,IO_PORT_A, 6},
};

const tADCDevice ADCConfigForKey =
{
    .ADCEnabledPinNum = NUM_OF_ADC_KEY_PIN,
    .pAdcPinIoAttr = ADCPinsForKey
};

tAdcKey adcKeys[]=
{   /* key_id         key_ADC_channel   adc threshold */
    {NEXT_KEY,          ADC_PIN3,   ADC_THRESHOLD_RANGE_1},
    {RETURN_KEY,        ADC_PIN3,   ADC_THRESHOLD_RANGE_2},
    {PREV_KEY,          ADC_PIN3,   ADC_THRESHOLD_RANGE_3},
    {ENTER_KEY,         ADC_PIN3,   ADC_THRESHOLD_RANGE_4},    
};

tAdcKeyboardDevice adcKeyboardConf =
{
    .adcKeyboard.deviceInfo.deviceID = KEYBOARD_DEV_ID,
    .adcKeyboard.deviceInfo.deviceType = ADC_DEV_TYPE,
    .adcKeyboard.keyNum = NUM_OF_ADC_KEY,
    .attachedDeviceObjConfig = &ADCConfigForKey,
    .pAdcKeySet = adcKeys
};

/* end of keyboard attached devices setting */

const tI2CDevice dspSvsConfig =
{
    .deviceInfo.deviceID = DSP_DEV_ID,
    .deviceInfo.deviceType = I2C_DEV_TYPE,
    .i2cMode = I2C_MASTER_MODE,
    .channel = I2C_CHANNEL_ONE,
    .pvSlaveCallback = NULL,
    .address = 0x70, /* device address */
    .regAddrLen = REG_LEN_8BITS,
    .baudRate = 0xc2,
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
    .interrupt.priority    = 1,     /* priority can be 0..3 from highest to lowest */
    .interrupt.subPriority = 0,     // not-used, for compatibility with PIC32
};

const tUARTDevice UartBleConfig = {
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

tGPIOPin displayGpioPins[] =
{
    {GPIO_10,  IO_PORT_B,  IO_BIT_11,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*STB*/
    {GPIO_11,  IO_PORT_B,  IO_BIT_12,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*CLK*/
    {GPIO_12,  IO_PORT_B,  IO_BIT_14,  GPIO_DIGITAL_OUTPUT,   GPIO_DEFAULT_OUT_LOW,  ODC_DISABLE},/*DIO*/
};


const tGPIODevice displayGpioConf =
{
    .deviceInfo.deviceID    = DISPLAY_DEV_ID,
    .deviceInfo.deviceType  = GPIO_DEV_TYPE,
    .usedGPIOPinNum         = NUM_OF_GPIO_DISPLAY_PIN,
    .pGPIOPinSet            = displayGpioPins
};

const tStorageDevice nvmConfig = {
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
};

const uint8 NUM_OF_ATTACHED_DEVICES = ArraySize(devices);

void UartDrv_Init(eTpUartDevice id)
{
    switch(id)
    {
    case TP_UART_DEV_1:
      {  

      }
        break;
    case TP_UART_DEV_2:
      {

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

