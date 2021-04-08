/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/**
* @file deviceTypes.h
* @brief The file that contains the the data types for peripherals (attached via serial comms or otherwise)
* @author Christopher Alexander
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

#ifndef DEVICETYPES_H
#define DEVICETYPES_H
#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "commonTypes.h"

#define PLATFORM_VERSION "TymPlat v0.2 "

typedef enum {
    BT_DEV_ID,
    ALLPLAY_DEV_ID,
    ASETK_DEV_ID,
    DSP_DEV_ID,
    AUDIO_DAC_DEV_ID,
    AMP_DEV_ID,
    AMP_WOOFER_1_DEV_ID,
    AMP_WOOFER_2_DEV_ID,
    AMP_TWEETER_1_DEV_ID,
    AMP_MIDDLE_1_DEV_ID,
    DEBUG_DEV_ID,
    KEYBOARD_DEV_ID,
    LED_DEV_ID,
    INT_FLASH_DEV_ID,
    BLE_DEV_ID,
    DISPLAY_DEV_ID,
    POWER_DEV_ID,
    AUDIO_DEV_ID,
    AUDIO_ADC_DEV_ID,
    SPDIF_REV_DEV_ID,
    BATT_DEV_ID,
    TOUCH_KEY_DEV_ID,
    IO_EXPANDER_DEV_ID,
    DBG_I2C1_ID,
    DBG_I2C2_ID,
    SPI1_DEV_ID,
    SPI2_DEV_ID,
    SW_I2C1_DEV_ID,
    SW_I2C2_DEV_ID,
    SW_I2C3_DEV_ID,
    SSM3582_W_L_DEV_ID,     // woofer left amp chip
    SSM3582_T_L_LU_DEV_ID,     // tweeter left + left-up channel amp chip
    SSM3582_T_C_DEV_ID,     // tweeter center amp chip
    SSM3582_W_C_DEV_ID,     // woofer center amp chip
    SSM3582_T_R_RU_DEV_ID,     // tweeter right + right-up channel amp chip
    SSM3582_W_R_DEV_ID,     // woofer right amp chip
    PCM9211_DEV_ID,
    EP_HDMI_DEV_ID,         //[HDMI]  
    HDMI_DEV_I2C_GPIO_ID,   //[HDMI]       
    COMM_DEV_ID,
    POWER_DETECT_ID,
    AUDIO_CODEC_ID,
    A2B_MASTER_DEV_ID,
    MUSIC_DETECT_ID,    // for 'music' detect
    AUDIO_RESET_DEV_ID,  // for codec/dsp reset GPIO
    DSP_DECODER_DEV_ID,     // dsp with dolby/dts decoder
    DEVICE_ID_MAX
} eDeviceID;

typedef enum {
    I2C_DEV_TYPE,    /* devices, connected to i2c bus */
    UART_DEV_TYPE,
    SPI_DEV_TYPE,
    ADC_DEV_TYPE,
    KNOB_DEV_TYPE,
    GPIO_DEV_TYPE,
    GPIO_ENCODER_DEV_TYPE,
    INFRARED_DEV_TYPE,
    IO_EXPANDER_DEV_TYPE,
    KEY_MATRIX_DEV_TYPE,
    PWM_DEV_TYPE,
    NVM_DEV_TYPE,
    FLASH_DEV_TYPE,
    EEPROM_DEV_TYPE,
    AUDIO_DSP_DEV_TYPE,
    AUDIO_SRC_DEV_TYPE,
    AUDIO_ADC_DEV_TYPE,
    AUDIO_GPIO_DEV_TYPE,
    AUDIO_SPDIF_DEV_TYPE,
    LINEAR_KNOB_DEV_TYPE,
    AZ_INTEG_TOUCH_KEY_DEV_TYPE,
    TI_TOUCH_KEY_DEV_TYPE,
    ROTATER_DEV_TYPE,
    SWI2C_DEV_TYPE,
    I2C_SLAVE_DEV_TYPE,

    TILT_SENSOR_DEV_TYPE,
    PROXIMITY_SENSOR_DEV_TYPE,
    DEV_TYPE_MAX_ID,
    INVALID_DEV_TYPE = DEV_TYPE_MAX_ID
} eDeviceType;

typedef enum {
    AW9110B,
    AW9523,
    AW9120
} eDeviceSubType;

typedef enum
{
    USB_STA_UNCONNECTED,
    USB_STA_DEFAULT,
    USB_STA_ADDRESSED,
    USB_STA_CONFIGURED,
    USB_STA_SUSPENDED,
}eUsbDeviceStatus;

/* BLE COMMAND*/
typedef enum
{
    BLE_PWR_ON_CMD,
    BLE_PWR_OFF_CMD,
    BLE_ENABLE_CMD,
    BLE_PAIRING_CMD,
    BLE_FACTORY_RESET_CMD,

    BLE_MAX_CMD,
}eBleCmd;

typedef struct tDevice
{
    eDeviceID deviceID;
    eDeviceType deviceType;
    eDeviceSubType deviceSubType;
}tDevice;

#define DEVICE(nameOfDevice_)     typedef struct nameOfDevice_ nameOfDevice_; \
    struct nameOfDevice_ {      \
        tDevice deviceInfo;

#define END_DEVICE };

#define SUBDEVICE(nameOfDevice_, superDevice_) \
    DEVICE(nameOfDevice_) \
        superDevice_ super_;

/*________LED data types_______________________________*/
#ifdef LED_SRV_SUPPORT_64_LEDS
typedef uint64 ledMask; /**< type for Led mapping, i.e. 0x0000 0001 => first Led ID
                                                        0x0000 0002 => second Led ID, etc. */
#else
typedef uint32 ledMask; /**< type for Led mapping, i.e. 0x0000 0001 => first Led ID
                                                        0x0000 0002 => second Led ID, etc. */
#endif

typedef uint32 Color;

#define RGBA(r,g,b,a)   ((((r) & 0xFF) << 24) \
                        | (((g) & 0xFF) << 16) \
                        | (((b) & 0xFF) <<  8) \
                        |  ((a) & 0xFF))
/* Move the color definition into "colorPalette.config" in project folder for customization. */
#include "colorPalette.config"

#define GET_RED(c)      (((c) >> 24) & 0xFF)
#define GET_GREEN(c)    (((c) >> 16) & 0xFF)
#define GET_BLUE(c)     (((c) >>  8) & 0xFF)

#define GET_PURE_COLOR(c) ((c) & 0xFFFFFF00)

#define GET_BRIGHTNESS(c)       ((c) & 0xFF)
#define SET_BRIGHTNESS(c, brightness)   ( ((c)&0xFFFFFF00) | (brightness) )

typedef enum eColorComponent
{
    COLOR_COMPONENT_RED = 0,
    COLOR_COMPONENT_GREEN,
    COLOR_COMPONENT_BLUE,
    COLOR_COMPONENT_BRIGHTNESS,
    COLOR_COMPONENT_MAX = COLOR_COMPONENT_BRIGHTNESS
} eColorComponent;

/* For OC register. probably needs removing - we can figure this out based on the config.*/
typedef enum eOCRegister
{
    OCREG_NONE,
    OCREG_1,
    OCREG_2,
    OCREG_3,
    OCREG_4,
    OCREG_5,
} eOCRegister;
/*__________________________IO Expander GPIO data type____________________________*/
typedef enum
{
    IOE_0,IOE_1,IOE_2,IOE_3,IOE_4,IOE_5,IOE_6,IOE_7,IOE_8,IOE_9,IOE_10,
    IOE_11,IOE_12,IOE_13,IOE_14,IOE_15,IOE_16,IOE_17,IOE_18,IOE_19,
    IOE_20,IOE_21,IOE_22,IOE_23,IOE_24,IOE_25,IOE_26,IOE_27,IOE_28,
    IOE_29,IOE_30,IOE_31
} eIOEId;

typedef enum
{
    IoeChip0,IoeChip1,IoeChip2,IoeChip3
} eIOEChipId;

typedef enum
{
    IOE_PORT_A,IOE_PORT_B,IOE_PORT_C,IOE_PORT_D,IOE_PORT_E,IOE_PORT_F,IOE_PORT_G,IOE_PORT_MAX
} eIoePort;

typedef enum
{
    IOE_BIT_0, IOE_BIT_1, IOE_BIT_2, IOE_BIT_3,IOE_BIT_4, IOE_BIT_5, IOE_BIT_6,
    IOE_BIT_7,IOE_BIT_8, IOE_BIT_9, IOE_BIT_10,IOE_BIT_11,IOE_BIT_12,IOE_BIT_13,
    IOE_BIT_14,IOE_BIT_15,IOE_BIT_16,IOE_BIT_17,IOE_BIT_18,IOE_BIT_19,IOE_BIT_20,
    IOE_BIT_21,IOE_BIT_22,IOE_BIT_23,IOE_BIT_24,IOE_BIT_25,IOE_BIT_26,IOE_BIT_27,
    IOE_BIT_28,IOE_BIT_29,IOE_BIT_30,IOE_BIT_31
} eIoeBit;
/*__________________________End of IO Expander GPIO data type____________________________*/
typedef struct tPwmPin
{
    uint16     gpioId;            // Based on this id we can generate the port and bit needed to address
    uint8      gpioPort:4;
    uint8      gpioBit:4;
    uint8      pOCRegister;       // pointer to Ouput Compare register
}tPwmPin;

typedef struct tPwmLedMap
{
    uint8           ledID;
#ifdef LED_HAS_RGB
    eColorComponent colorComp;
#endif
    tPwmPin         pwmPin;
}tPwmLedMap;

typedef struct tIoExpanderLedMap
{
    uint8           ledID;
#ifdef LED_HAS_RGB
    eColorComponent colorComp;
#else
    uint32          pure_color;  //A color with maximum (0xFF) brightness
#endif
    uint8           port;
    uint32           pin;
}tIoExpanderLedMap;

typedef struct tIoExpanderConfig
{
    eIOEChipId      chipId;
    uint8           swResetValue;
    uint8           ledModePortA;
    uint8           ledModePortB;
    uint8           outPutPortA;
    uint8           outPutPortB;
    uint8           ioDirectionA;
    uint8           ioDirectionB;
    uint8           controlValue;
}tIoExpanderConfig;

DEVICE(tLedDevice)
    uint8       ledNum;
END_DEVICE

DEVICE(tPwmLedDevice)
    uint8       ledNum;
    const tPwmLedMap    *pPwmLedMap;
END_DEVICE


DEVICE(tStorageDevice)
END_DEVICE


DEVICE(tDSPDevice)
END_DEVICE

/*________I2C data types_______________________________*/
typedef enum{
    I2C_MASTER_MODE,
    I2C_SLAVE_MODE,
}eI2CMode;

typedef enum {
    I2C_CHANNEL_ONE,
    I2C_CHANNEL_TWO,
}eI2C_Channel;

typedef enum
{
    REG_LEN_8BITS,
    REG_LEN_16BITS,
    REG_LEN_24BITS,
    REG_LEN_NONE
}eI2CRegAddLen;

typedef enum
{
    I2C_SPEED_100K,     // standard i2c speed
    I2C_SPEED_400K,     // i2c speed : fast mode
    I2C_SPEED_MAX
}eI2CSpeed;

/* Defines the prototype to slave callback function */
typedef void (*pdI2C_SLAVE_CALLBACK)( void * );
/*_______________________________________________________*/

/* Devices, connected to I2C bus*/
DEVICE(tI2CDevice)
    eI2CMode i2cMode;
    pdI2C_SLAVE_CALLBACK pvSlaveCallback;
    uint16 devAddress; /* device address */
    eI2C_Channel channel:8;
    eI2CRegAddLen regAddrLen:8;
    uint32 baudRate;
END_DEVICE

/* I2C slave device, only 7bit address now */
DEVICE(tI2CSlaveDevice)
    uint8   channel;
    uint8   speed;
    uint8   dev_addr;
    uint8   reg_len;
END_DEVICE

#ifdef HAS_PCM9211_CODEC
typedef struct tagPcm9211Device
{
    tDevice deviceInfo;
    const tI2CDevice *p_I2cDevice;
}stPcm9211Device_t;
#endif

#ifdef HAS_SWi2c_DEVICE
/*__________________________software i2c data type_____________________________*/

typedef enum tagSWi2cSpeed
{
    eSWi2c_SPEED_NORMAL,
    eSWi2c_SPEED_SLOW,
    eSWi2c_SPEED_FAST,
    eSWi2c_SPEED_MAX	
}eSWi2cSpeed_t;

typedef enum tagSWi2cBusNumber
{
    eSWi2cBus_IDStart,
    eSWi2cBus_ID1,
    eSWi2cBus_ID2,
    eSWi2cBus_ID3,
    eSWi2cBus_ID_MAX
}eSWi2cBusID_t;

typedef struct tagSWi2cDevice
{
    tDevice     deviceInfo;         // device info
    eSWi2cBusID_t   busID;          // which I2C bus used?
    eSWi2cSpeed_t   swi2cSpeed;     // speed
    uint8         devAddress;     // i2c device address
    uint8       delayMSAfterWrite;      // some i2c device (e.g EEPROM-24C02) need some delay after receiver the data
    void    (*vSWi2cSCL_Set)(int high);     // SCL GPIO setup high/low 
    void    (*vSWi2cSDA_Set)(int high);     // SDA GPIO setup high/low 
    void    (*vSWi2cSDA_DirSetup)(int dir);     // SDA direction, dir 1:output, 0:input
    int     (*iSWi2cSDA_Sense)(void);       // read the GPIO status from SDA
    void    (*vSWi2cSCL_Delay)(void);     // SCL delay for i2c speed control
}stSWi2cDevice_t;
/*__________________________End of software i2c data type_____________________________*/
#endif  // HAS_SWi2c_DEVICE


/*________UART data types_______________________________*/
typedef enum
{
    TP_UART_DEV_1 = 0,
    TP_UART_DEV_2,
    TP_UART_DEV_3,
    TP_UART_DEV_4,
    TP_UART_DEV_5,
    TP_UART_DEV_6,
    TP_UART_DEV_7,
    TP_UART_DEV_8,
    TP_UART_DEV_MAX,
} eTpUartDevice;

typedef enum
{
    TP_UART_BYTE_SIZE_8_BITS,
    TP_UART_BYTE_SIZE_9_BITS,
    TP_UART_BYTE_SIZE_MAX
}eTpUartByteSize;

typedef enum
{
    TP_UART_PARITY_NONE,
    TP_UART_PARITY_ODD,
    TP_UART_PARITY_EVEN,
    TP_UART_PARITY_MAX
}eTpUartParity;

typedef enum
{
    TP_UART_STOP_BIT_1 = 0,    /**< 1 stop bit */
    TP_UART_STOP_BIT_1_5,      /**< 1.5 stop bit */
    TP_UART_STOP_BIT_2,        /**< 2 stop bit */
    TP_UART_STOP_BIT_MAX,
} eTpUartStopBit;

DEVICE(tUARTDevice)
    uint32          baudrate;   /**< target baudrate */
    eTpUartDevice   uartId;
    
    eTpUartByteSize byteSize; 
    eTpUartParity   parity;  /**< 8bit data + 1bit even parity:
                                        byteSize=TP_UART_BYTE_SIZE_9_BITS
                                        parity=TP_UART_PARITY_EVEN
                                  8bit data + no parity:
                                        byteSize=TP_UART_BYTE_SIZE_8_BITS
                                        parity=TP_UART_PARITY_NONE    
                                */
    
    eTpUartStopBit  stopBits;
    uint8           dmaEnable;
    struct
    {
        uint8       priority;       /**< priority setting for PIC32 and STM32 */
        uint8       subPriority;    /**< sub-priority for PIC32 */
    } interrupt;

    bool            bBatchProcessInIRQ;         /**< a flag to set whether to process hardware UART buffer in batch
                                                     this flag describes whether the UART module would
                                                     process all the UART hw buffer in one IRQ entry. Or, the
                                                     UART module will process only one byte from UART hw
                                                     in one IRQ entry

                                                     ===Suggested use===:
                                                     It is suggested to turn this on for Important UART modules.
                                                     and turn it off for non-important UART modules.

                                                     Turn it off if you are not sure.
                                                 */
END_DEVICE

/*______________________________SPI data types_______________________________*/
#ifdef HAS_HW_SPI_DEVICE
typedef enum
{
    TP_SPI_CH_START,
    TP_SPI_CH_1,
    TP_SPI_CH_2,
    TP_SPI_CH_MAX
}eTpSPIChannel;

// copy the SPI_InitTypeDef struct from stm32f0xx_spi.h, just for compiler.
typedef struct tagSpiConfigParameter
{
    uint16 SPI_Direction;           /*!< Specifies the SPI unidirectional or bidirectional data mode.
                                        This parameter can be a value of @ref SPI_data_direction */

    uint16 SPI_Mode;                /*!< Specifies the SPI mode (Master/Slave).
                                        This parameter can be a value of @ref SPI_mode */

    uint16 SPI_DataSize;            /*!< Specifies the SPI data size.
                                        This parameter can be a value of @ref SPI_data_size */

    uint16 SPI_CPOL;                /*!< Specifies the serial clock steady state.
                                        This parameter can be a value of @ref SPI_Clock_Polarity */

    uint16 SPI_CPHA;                /*!< Specifies the clock active edge for the bit capture.
                                        This parameter can be a value of @ref SPI_Clock_Phase */

    uint16 SPI_NSS;                 /*!< Specifies whether the NSS signal is managed by
                                        hardware (NSS pin) or by software using the SSI bit.
                                        This parameter can be a value of @ref SPI_Slave_Select_management */

    uint16 SPI_BaudRatePrescaler;   /*!< Specifies the Baud Rate prescaler value which will be
                                        used to configure the transmit and receive SCK clock.
                                        This parameter can be a value of @ref SPI_BaudRate_Prescaler
                                        @note The communication clock is derived from the master
                                        clock. The slave clock does not need to be set. */

    uint16 SPI_FirstBit;            /*!< Specifies whether data transfers start from MSB or LSB bit.
                                        This parameter can be a value of @ref SPI_MSB_LSB_transmission */

    uint16 SPI_CRCPolynomial;       /*!< Specifies the polynomial used for the CRC calculation. */
}stSpiConfigParameter_t;

typedef struct tSpiDevice{
    tDevice 	deviceInfo;
    eTpSPIChannel   channel;
    stSpiConfigParameter_t	spiConfig;
}tSpiDevice;

#else

DEVICE(tSPIDevice)
END_DEVICE

#endif	// HAS_HW_SPI_DEVICE
/*__________________________End of SPI data type_____________________________*/

/*__________________________GPIO data type___________________________________*/
typedef enum
{
    GPIO_0,GPIO_1,GPIO_2,GPIO_3,GPIO_4,GPIO_5,GPIO_6,GPIO_7,GPIO_8,GPIO_9,GPIO_10,
    GPIO_11,GPIO_12,GPIO_13,GPIO_14,GPIO_15,GPIO_16,GPIO_17,GPIO_18,GPIO_19,
    GPIO_20,GPIO_21,GPIO_22,GPIO_23,GPIO_24,GPIO_25,GPIO_26,GPIO_27,GPIO_28,
    GPIO_29,GPIO_30,GPIO_31
}eGPIOId;

typedef enum
{
    IO_PORT_A,IO_PORT_B,IO_PORT_C,IO_PORT_D,IO_PORT_E,IO_PORT_F,IO_PORT_G,IO_PORT_MAX
}eIoPort;

typedef enum
{
    IO_BIT_0, IO_BIT_1, IO_BIT_2, IO_BIT_3,IO_BIT_4, IO_BIT_5, IO_BIT_6, 
    IO_BIT_7,IO_BIT_8, IO_BIT_9, IO_BIT_10,IO_BIT_11,IO_BIT_12,IO_BIT_13,
    IO_BIT_14,IO_BIT_15,IO_BIT_16,IO_BIT_17,IO_BIT_18,IO_BIT_19,IO_BIT_20,
    IO_BIT_21,IO_BIT_22,IO_BIT_23,IO_BIT_24,IO_BIT_25,IO_BIT_26,IO_BIT_27,
    IO_BIT_28,IO_BIT_29,IO_BIT_30,IO_BIT_31
}eIoBit;

typedef enum
{
    GPIO_DIGITAL_INPUT,
    GPIO_DIGITAL_OUTPUT,
    GPIO_ANALOG_INPUT,
    GPIO_ANALOG_OUTPUT,
    GPIO_BIDIRECTION
}eGPIODrection;

typedef enum
{ 
  //For GPIO input
    GPIO_ACTIVE_HIGH, // internal pull low
    GPIO_ACTIVE_LOW,  // internal pull high
    GPIO_ACTIVE_FLOATING,  // do not pull internally

  //For GPIO output
    GPIO_DEFAULT_OUT_HIGH,     //set() when initialize
    GPIO_DEFAULT_OUT_LOW,      //clear() when initialize
    GPIO_DEFAULT_OUT_FLOATING, //No default value when initialize
}eGPIOInitAttr;

typedef enum
{
  //For GPIO output
    ODC_ENABLE,  // floating when set(), pull low when clear()
    ODC_DISABLE, // pull high when set(), pull low when clear()

  //For GPIO input
    ODC_NOT_APPLICABLE, //not applicable for input pin
}eODC;

typedef struct tGPIOPin
{
    eGPIOId             gpioId:5;
    eIoPort             gpioPort:3;
    eIoBit              gpioBit:5;
    eGPIODrection       gpioDirection:3;
    eGPIOInitAttr       gpioInitAttr:3;
    eODC                openDrainControl:3;
    uint8               reserve:2;
}tGPIOPin;

DEVICE(tGPIODevice)
    uint32              usedGPIOPinNum;     /* how many GPIO pins are Used */
    const tGPIOPin      *pGPIOPinSet;
END_DEVICE


DEVICE(tIoeLedDevice)    
    uint8       ledNum;
    tI2CDevice *i2cDeviceConf;
    tIoExpanderLedMap *pIoExpanderLedMap;
    tIoExpanderConfig *pIoExpanderConfig;
    tGPIODevice       *pResetGpioDevice;
END_DEVICE


typedef struct tVIoExpanderGpioMap
{
    uint8           gpioID;
}tVIoExpanderGpioMap;


DEVICE(tVIoeGpioDevice)
    uint8  chipID;
    uint8  gpioNum;
    tVIoExpanderGpioMap *pVIoExpanderGpioMap;
    tIoeLedDevice *pIoeDevice;
END_DEVICE

/*__________________________End of GPIO data type____________________________*/


/*__________________________ADC data type____________________________________*/

typedef enum
{
    ADC_PIN0,ADC_PIN1,ADC_PIN2,ADC_PIN3,ADC_PIN4,ADC_PIN5,ADC_PIN6,ADC_PIN7,
    ADC_PIN8,ADC_PIN9,ADC_PIN10,ADC_PIN11,ADC_PIN12,ADC_PIN13,ADC_PIN14,
    ADC_PIN15,ADC_PIN16,ADC_PIN17,ADC_PIN18,ADC_PIN19,ADC_PIN20,ADC_PIN21,
    ADC_PIN22,ADC_PIN23,ADC_PIN24,ADC_PIN25,ADC_PIN26,ADC_PIN27,ADC_PIN28,
    ADC_PIN29,ADC_PIN30,ADC_PIN31,
    ADC_PIN_MAX,
}eAdcPin;

typedef enum
{
    ADC_CHANNEL_MIN = 0,
    ADC_CHANNEL0 = ADC_CHANNEL_MIN,
    ADC_CHANNEL1,
    ADC_CHANNEL2,
    ADC_CHANNEL3,
    ADC_CHANNEL4,
    ADC_CHANNEL5,
    ADC_CHANNEL6,
    ADC_CHANNEL7,
    ADC_CHANNEL8,
    ADC_CHANNEL9,
    ADC_CHANNEL10,
    ADC_CHANNEL11,
    ADC_CHANNEL12,
    ADC_CHANNEL13,
    ADC_CHANNEL14,
    ADC_CHANNEL15,
    ADC_CHANNEL_MAX,
}eADCChannel;

/**/
typedef struct tAdcIoAttr
{
    eAdcPin             adcPin:6;
    eADCChannel         adcChannel:5;    /* channel mapped to the pin */
    eIoPort             ioPort:3;        /* IO port */
    eIoBit              gpioBit:5;       /* IO bit */
}tAdcPinIoAttr;

DEVICE(tADCDevice)
    uint32               ADCEnabledPinNum;      /* how many ADC pins are enabled */
    const tAdcPinIoAttr  *pAdcPinIoAttr;        /* the Io attr of adc pins */
END_DEVICE

/*__________________________End of ADC data type_____________________________*/

/*_____________________________keyboard data type____________________________*/

typedef enum
{
    KEY_EVT_INVALID,
    KEY_EVT_DOWN,
    KEY_EVT_HOLD,
    KEY_EVT_REPEAT,
    KEY_EVT_VERY_LONG_HOLD,
    COMB_KEY_EVT,
    KEY_EVT_UP, /* if you have any new key evt, add it before KEY_EVT_UP */
    KEY_EVT_SHORT_PRESS,
    KEY_EVT_LONG_PRESS,
    KEY_EVT_VERY_LONG_PRESS,
    KEY_EVT_DOUBLE_PRESS,
    KEY_EVT_MAX
} eKeyEvent;

typedef enum
{
    INVALID_KEY = 0,
    STANDBY_KEY,
    NET_RESET_KEY,
    DIRECT_MODE_KEY,
    VOLUME_DOWN_KEY,
    VOLUME_UP_KEY,
    PAIRING_KEY,
    INPUT_KEY,
    MUTE_KEY,
    PLAY_KEY,
    NEXT_KEY,
    PREV_KEY,
    VOL_UP_DOWN_KEY,
    PLAY_PAUSE_KEY,
    NEXT_PREV_KEY,
    STANDBY_PLAY_PAUSE_KEY,
    POWER_KEY,
    POWER_IR_KEY,
    EJECT_KEY,
    BT_KEY,
    TALK_KEY,
    SOURCE_SWITCH_IR_KEY,
    ENTER_KEY,
    RETURN_KEY,
    CONNECT_KEY,
    PRESET_KEY_1_LOAD,
    PRESET_KEY_2_LOAD,
    PRESET_KEY_3_LOAD,
    PRESET_KEY_4_LOAD,
    PRESET_KEY_1_SAVE,
    PRESET_KEY_2_SAVE,
    PRESET_KEY_3_SAVE,
    NFC_TAG_KEY,
    TOUCH_TAP_KEY,
    TOUCH_TAP_MBUTTON_1,
    TOUCH_TAP_MBUTTON_2,
    TOUCH_TAP_MBUTTON_3,
    TOUCH_TAP_MBUTTON_4,
    TOUCH_TAP_MIC,
    TOUCH_NOISE_KEY,
    TOUCH_SWIPE_RIGHT_KEY,
    TOUCH_SWIPE_LEFT_KEY,
    TOUCH_SWIPE_UNKNOW,
    FACTORY_RESET_KEY,
    WAC_KEY,

    BASSPORT_CLOSE,
    BASSPORT_OPEN,
    BASSPORT_RUNNING,

    TI_TOUCH_KEY_0,
    TI_TOUCH_KEY_1,
    TI_TOUCH_KEY_2,
    TI_TOUCH_KEY_3,
          
    TI_TOUCH_KEY_4,
    TI_TOUCH_KEY_5,
    TI_TOUCH_KEY_6,
    TI_TOUCH_KEY_7,

    /* Bosch Sensortec */
    TILT_SENSOR_FLOOR_KEY,
    TILT_SENSOR_WALL_KEY,

    PROXIMITY_KEY_0,

    /*  knobs buttons with predefined positions */
    VOLUME_KNOB_BASE_KEY_ID,
    BASS_KNOB_BASE_KEY_ID,
    TREBLE_KNOB_BASE_KEY_ID,

    IR_VOLUME_DOWN_KEY,
    IR_VOLUME_UP_KEY,
    IR_PRESET_KEY_1,
    IR_PRESET_KEY_2,
    IR_PRESET_KEY_3,
    SOFT_AP_KEY,
    CONFIG_KEY,
    SCREEN_OFF_ON_KEY,
    RESET_KEY,
    SHOP_MODE_KEY,
    VOL_KEY,
    LPF_KEY,
    PHASE_KEY,
    MINUS_KEY,
    PLUS_KEY,
    BASS_DOWN_KEY,
    BASS_UP_KEY,
    TREBLE_DOWN_KEY,
    TREBLE_UP_KEY,
    STANDBY_TEST_KEY,
    NORMAL_KEY_ID_MAX,

    /*
     * The comb key id is quite project related, different project may have different
     * comb key name, in this case, it may be better to just use genaric names as
     * below, the main app is responsible to redefine those macros based on the
     * project requirements.
     */
    COMB_KEY_ID_0,
    COMB_KEY_ID_1,
    COMB_KEY_ID_2,
    COMB_KEY_ID_3,
    COMB_KEY_ID_4,
    COMB_KEY_ID_5,
    COMB_KEY_ID_MAX
}eKeyID;

typedef struct tKeyboardDevice
{
    tDevice deviceInfo;
    uint32  keyNum;
    bool isNeedInitSeq;
    bool isInitSeqDone;
    void(*KeyboardInitSeqStartCb)(void *me);
    bool(*KeyboardInitSeqProcessCb)(void *me);
    void(*KeyboardInitSeqDoneCb)(void *me);
}tKeyboardDevice;

typedef struct tPga460KeyDevice
{
    tKeyboardDevice           proximityKeyboard;
    const tUARTDevice         *uartConfig;
    const eKeyID              keyId;
}tPga460KeyDevice;

typedef struct tTiTouchKey
{
    eKeyID        keyId;
    uint8         seqNo;
    uint32        gainSettId;
}tTiTouchKey;

typedef struct tTouchKeyboardDevice
{
    tKeyboardDevice TiTouchKeyboard;
    tI2CDevice *i2cConfig;
    const tTiTouchKey *pTiTouch;
} tTouchKeyboardDevice;

#ifdef HAS_BMI160_TILT_SENSOR_KEY

typedef struct tBmi160KeyboardDevice
{
    tKeyboardDevice    bmi160Keyboard;
    const tI2CDevice   *attachedDeviceObjConfig;
    uint16 DelayTimeOut;
}tBmi160KeyboardDevice;
#endif

typedef struct tAdcKey
{
    eKeyID          keyId;
    eAdcPin         adcPin;
    uint32          adcMinimumValue;
    uint32          adcMaxValue;
}tAdcKey;
typedef struct tAdcKeyboardDevice
{
    tKeyboardDevice           adcKeyboard;
    const tADCDevice          *attachedDeviceObjConfig;
    void                      *attachedDeviceObj; /* void type to solve include confilict problem */
    const tAdcKey             *pAdcKeySet;
} tAdcKeyboardDevice;

typedef enum
{
    KNOB_STATE_LOW = 0,
    KNOB_STATE_MID,
    KNOB_STATE_HIGH
}eKnobState;

typedef enum{
    ROTATE_FORWARD_0,
    ROTATE_FORWARD_1,
    ROTATE_FORWARD_2,
    ROTATE_FORWARD_3,
    ROTATE_BACKWARD_1,
    ROTATE_BACKWARD_2,
    ROTATE_BACKWARD_3,
}eRotateStage;//the wave of clockwise and anti-clockwise is composed of stages representted with different voltage

#ifdef HAS_LINEAR_ADC_KNOB_KEY
typedef struct tKnobKey
{
    eKeyID          keyId;
    eAdcPin         adcPin;
    eRotateStage    releaseStage;
}tKnobKey;

typedef struct tKnobKeyboardDevice
{
    tKeyboardDevice           knobKeyboard;
    const tADCDevice          *attachedDeviceObjConfig;
    void                      *attachedDeviceObj; /* void type to solve include confilict problem */
    const tKnobKey            *pKnobKeySet;
} tKnobKeyboardDevice;
/* Linear adc key */
typedef struct tLinearKnobKey
{
    eKeyID      keyId;
    eAdcPin     adcPin;
    uint8       settIdOfAdcMin; /* This is for calibration, you can use this id to get data from setting server */
    uint8       settIdOfAdcMax; /* This is for calibration, you can use this id to get data from setting server */
    uint16      resolution;
    uint16      deltaBuffer; /* This is the area that makes sure the adc value jumped from the current step to another*/
    uint16      adcDefaultMinValue; /* This is the default value */
    uint16      adcDefaultMaxValue;     /* This is the default value */
}tLinearKnobKey;

typedef struct tLinearKnobKeyDevice
{
    tKeyboardDevice           linearKnobKeyboard;
    const tADCDevice          *attachedDeviceObjConfig;
    void                      *attachedDeviceObj; /* void type to solve include confilict problem */
    const tLinearKnobKey      *pLinearKnobKeySet;
}tLinearKnobKeyDevice;
/* End of Linear adc key */
#endif

#ifdef GPIO_ENCODER_KEY         //start of gpio encoder key

typedef struct tGpioEncoderKeyDevice
{
    tKeyboardDevice           encoderKeyboard;
    const tGPIODevice         *attachedDeviceObjConfig;
    void                      *attachedDeviceObj; /* void type to solve include confilict problem */
}tGpioEncoderKeyDevice;

#endif  //endif of gpio encoder key

typedef struct tGpioKey
{
    eKeyID          keyId;
    eGPIOId         gpioId;
}tGpioKey;

typedef struct tGpioKeyboardDevice
{
    tKeyboardDevice           gpioKeyboard;
    const tGPIODevice         *attachedDeviceObjConfig;
    void                      *attachedDeviceObj;/* void type to solve include confilict problem */
    const tGpioKey            *pGpioKeySet;
} tGpioKeyboardDevice;

#ifdef HAS_IR_LEARNING

typedef struct 
{
    eKeyID          keyId;
    void          *irMap; // type is actually IR_MAP
}tIrKey;

typedef struct tIrKeyboardDevice
{
    tKeyboardDevice  irKeyboard;
    void             *attachedDeviceObj;/* void type to solve include confilict problem */
    const tIrKey     *pIrKeySet;
}tIrKeyboardDevice;
typedef struct tIrKeyDeviceConfig
{
    tKeyboardDevice           irKeyboard;
    void                      *attachedDeviceObj;/* void type to solve include confilict problem */
    const tIrKey*            pIrKeySet;
} tIrKeyDeviceConfig;
#else
typedef struct tIrKeyIdMapCode
{
    eKeyID    irKeyId;
    uint32    irCode;
}tIrKeyIdMapCode;

typedef struct tIRKeyboardDevice
{
    tKeyboardDevice         irKeyboard;
    void                    *attachedDeviceObj;/* void type to solve include confilict problem */
    const tIrKeyIdMapCode   *keyIdIrCodeMap;
    const tGPIOPin          ioAttr;
}tIRKeyboardDevice;
#endif /* #ifdef HAS_IR_LEARNING */

#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV
typedef struct tAzInteqTouchKeyDevice
{
    tKeyboardDevice  touchKeyboard;
}tAzInteqTouchKeyDevice;

typedef struct tAzIntegTouchKeyboardDevice
{
    tKeyboardDevice         touchKeyboard;
    const tI2CDevice        *i2cConfig572;
    const tI2CDevice        *i2cConfig572Bl;
    const tI2CDevice        *i2cConfig360a;
    const tI2CDevice        *i2cConfig333;
    const tI2CDevice        *i2cConfig263;
    const tGPIODevice       *gpioConfig;
} tAzIntegTouchKeyboardDevice;
#endif /* #ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV */

typedef struct tRotaterKeyboardDevice
{
    tKeyboardDevice         kbDev;
    const tGPIODevice       *gpioConfig;
    eKeyID                  keyCw;
    eKeyID                  keyCcw;
} tRotaterKeyboardDevice;

/*__________________________End of keyboard data type_________________________*/


/*_____________________________Audio data type____________________________*/
typedef enum
{
    AUDIO_CHANNEL_INVALID,
    AUDIO_CHANNEL_0,
    AUDIO_CHANNEL_1,
    AUDIO_CHANNEL_2,
    AUDIO_CHANNEL_AUXIN,
    AUDIO_CHANNEL_OPT,
    AUDIO_CHANNEL_SPDIF_0,
    AUDIO_CHANNEL_SPDIF_1,
    AUDIO_CHANNEL_RCA,
    AUDIO_CHANNEL_I2S_1,
    AUDIO_CHANNEL_I2S_2,
    AUDIO_CHANNEL_I2S_3,
    AUDIO_CHANNEL_I2S_4,
    AUDIO_CHANNEL_ANALOG_1,
    AUDIO_CHANNEL_ANALOG_2,
    AUDIO_CHANNEL_ANALOG_3,
    AUDIO_CHANNEL_ANALOG_4,
    AUDIO_CHANNEL_ANALOG_MIXED,
    AUDIO_CHANNEL_BT,
    BT_TONE_CHANNEL,
    AUDIO_CHANNEL_MAX
}eAudioChannel;

typedef enum
{
    AUDIO_SOURCE_MUTE,          //Cut the input source
    AUDIO_DSP_LINEOUT_MUTE,     //Mute by DSP line out
    AUDIO_DSP_DACOUT_MUTE,      //Mute DAC output
    AUDIO_DSP_HPAOUT_MUTE,      //Mute hap output
    AUDIO_AMP_SOFT_MUTE,
    AUDIO_AMP_SOFT_MUTE_MID,
    AUDIO_AMP_SOFT_MUTE_TW1,
    AUDIO_AMP_SOFT_MUTE_TW2,
    AUDIO_AMP_SOFT_MUTE_WF,
    AUDIO_AMP_MUTE,           //Mute the output, normally it is AMP mute
    AUDIO_CTRL_POWERLINK_MUTE, //10
    AUDIO_DSP_POWERLINK_MUTE,
    AUDIO_DSP_SPDIF_MUTE,
    AUDIO_DSP_ASE_MUTE,
    AUDIO_CODEC_DAC_MUTE,         //14
    AUDIO_DSP_OUT_CH_MIN,
    AUDIO_DSP_OUT_CH1_MUTE = AUDIO_DSP_OUT_CH_MIN,
    AUDIO_DSP_OUT_CH2_MUTE,
    AUDIO_DSP_OUT_CH3_MUTE,
    AUDIO_DSP_OUT_CH4_MUTE,
    AUDIO_MUTE_TYPE_MAX
}eAudioMuteType;

typedef enum
{
    LINE_IN_SENSITIVITY_HIGH,
    LINE_IN_SENSITIVITY_MEDIUM,
    LINE_IN_SENSITIVITY_LOW,
    LINE_IN_SENSITIVITY_DISABLE,
}eLineinSensitivity;


typedef enum
{
    SPEAKER_ROLE_MIN,
    SPEAKER_ROLE_LEFT = SPEAKER_ROLE_MIN,
    SPEAKER_ROLE_RIGHT,
    SPEAKER_ROLE_AMBIENT,
    SPEAKER_ROLE_MAX
}eSpeakerRole;

typedef enum
{
    SPEAKER_POSITION_MIN,
    SPEAKER_POSITION_UNDEFINED = SPEAKER_POSITION_MIN,
    SPEAKER_POSITION_FREE,
    SPEAKER_POSITION_WALL,
    SPEAKER_POSITION_CORNER,
    SPEAKER_POSITION_TABLE,
    SPEAKER_POSITION_MAX
}eSpeakerPosition;

typedef enum
{
    /* HDMI operations, will be sent to HDMI server */
    HDMI_ENTER_ISP_ID,
    HDMI_POWER_ID,
    HDMI_VOLUME_ID,
    HDMI_MUTE_ID,

    /**/

    /* HDMI part from HDMI chip, will be pulished to server */
    HDMI_TX_PLUGIN_ID,
    HDMI_RX_PLUGIN_ID,
    HDMI_CEC_POWER_ID,
    HDMI_CEC_VOLUME_ID,
    HDMI_CEC_MUTE_ID,
    HDMI_CEC_PORT_SOURCE_ID,

    /* HDMI external request */
    HDMI_TEST_ID,

    HDMI_SETT_ID_MAX,
}eHdmiSettId;

/* set ID of audio drivers, including DSP, ADC, AMP etc. chips. */
typedef enum
{
    DSP_VOLUME_SETT_ID,  // 0 Using this ID, user must update volume in setting database and then send the event
    DSP_EQ_CTRL_RGC,     // 1
    DSP_EQ_CTRL_USER_LP, // 2
    DSP_EQ_CTRL_USER_HP, // 3
    DSP_EQ_CTRL_TUNING,  // 4
    DSP_EQ_CTRL_PEQ1,    // 5
    DSP_EQ_CTRL_PEQ2,    // 6
    DSP_EQ_CTRL_PEQ3,    // 7
    DSP_PHASE_SETT_ID,   // 8
    DSP_TUNNING_SETT_ID, // 9
    DSP_DELAY_SETT_ID,   // 10
    DSP_PLAINEQ_SETT_ID, // 11
    DSP_POLARITY_SETT_ID,  // 12
    DSP_BASS_SETT_ID,      // 13
    DSP_TREBLE_SETT_ID,    // 14
    DSP_SETT_ID_MAX,       // 15
    SYSTEM_TUNNING_ID_START, // 16
    SYSTEM_GAIN_SETT_ID,     // 17
    SYSTEM_LIMITTER_SETT_ID, // 18
    DSP_PASSTHROUGH_SETT_ID, // 19
    DSP_DC_STATUS_SETT_ID,   // 20
    
    AMP_SLEEP_MODE_ID,       // 21, use to enable/disable AMP sleep mode
    AUDIO_OVERHEAT_MODE_ID,  // 22, use to notify system is too hot
    
    DSP_CAL_GAIN1_ID,        // 23
    DSP_CAL_GAIN2_ID,        // 24
    DSP_CAL_GAIN3_ID,        // 25
    DSP_CAL_GAIN4_ID,        // 26
    DSP_CAL_GAIN5_ID,        // 27
    DSP_CAL_GAIN_MAX=DSP_CAL_GAIN5_ID,

    AUDIO_VOL_FADE_SETT_ID,
    AUDIO_LINEIN_SENSITIVITY_SETT_ID,
    AUDIO_LINEIN_MULTI_ROOM_CHANNEL_SETT_ID,
    AUDIO_SPDIF_SET_REG_VOL_CTRL_SETT_ID,
    AUDIO_RESET_LINEIN_JACK_AND_MUSIC_STA_SETT_ID,
    AUDIO_POS_SOUND_MODE_SETT_ID,  // for setting positioin of speaker
    AUDIO_DSP_CHENNEL_L_R,

    AUDIO_SPEAKER_ROLE_SETT_ID, // for setting speaker role
    AUDIO_CLICK_SOUND_SETT_ID,
    AUDIO_REPEAT_CLICK_SOUND_SETT_ID,
        
    // for production test
    AUDIO_CHANNEL_MUTE_ID,

    DSP_SAMPLE_RATE_SETT_ID,
    DSP_WRITE_TONE_TOUCH_SETT_ID,
    DSP_SPDIF_TX_SETT_ID,	

    //DSP parameters
    AUDIO_DSP_BASS_SETT_ID,
    AUDIO_DSP_TREBLE_SETT_ID,
    AUDIO_DSP_LOUDNESS_SETT_ID,
        
    AUDIO_SETT_ID_MAX
}eAudioSettId;

typedef enum
{
    TL_CRITICAL, // 0
    TL_SERIOUS,  // 1
    TL_WARN,     // 2
    TL_NORMAL,   // 3
    TL_SUBNORMAL,    // 4
    TL_CRITICAL_COLD,// 5
    TL_NUM       // 4
}eTempLevel;

typedef enum
{
    AMP_TL_NORMAL,
    AMP_TL_WARN_1,
    AMP_TL_WARN_2,
    AMP_TL_WARN_3,
    AMP_TL_WARN_4,
    AMP_TL_WARN_5,
    AMP_TL_CRITICAL,
}eAmpTempLevel;

typedef enum
{
    BASS_PORT_STATE_RUNNING,
    BASS_PORT_STATE_CLOSED,
    BASS_PORT_STATE_OPENED,
    BASS_PORT_STATE_NUM,
}eBassPortState;

typedef struct
{
    int16 upper;
    int16 lower;
}sRange;

typedef enum
{
    AUXIN_JACK,
    SPDIF1_IN_JACK,
    SPDIF2_IN_JACK,
    RJ45_IN_JACK,
    RCA_IN_JACK,
    BLUETOOTH_JACK,
    JACK_IN_INVALID=0xff
}eAudioJackId;

typedef struct tJackinIdGpioIdMap
{
    eAudioJackId jackinId;
    eGPIOId      gpioId;
}tJackinIdGpioIdMap;

typedef union
{
    struct
    {
        uint32 eqDataPackageId : 16;
        uint32 eqDataPackageIdex : 16;
    };
    uint32 eqDataPackage;
}uEqDataPackage;
/*__________________________End of Aduio data type_________________________*/

/*_____________________________Comm data type____________________________*/

typedef enum
{
    COMM_EVT_INVALID,
    COMM_EVT_STANDBY,
    COMM_EVT_VOLUME,
    KEY_EVT_MUTE,
    COMM_EVT_DSPUPGRADE,
    COMM_EVT_MAX
} eSocEvt;
/*__________________________End of Comm data type_________________________*/


/** \brief get the device by the device ID defined in enum eDeviceID
 * \a devicedID to find
* \a parameter index is used in the case that several devices share the same ID, like LED.
 * this function will change the index value to the first device with the ID, then user
 * need to increase the index to search for next device with the same ID.
*/
const tDevice* getDevicebyId(eDeviceID deviceID, uint16* index);
const tDevice* getDevicebyIdAndType(eDeviceID deviceID, eDeviceType deviceType, uint16* index);

char* getVersionString();

#ifdef __cplusplus
}
#endif

#endif /* DEVICETYPES_H */
