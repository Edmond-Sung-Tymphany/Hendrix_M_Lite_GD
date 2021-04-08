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
    DEBUG_DEV_ID,
    KEYBOARD_DEV_ID,
    LED_DEV_ID,
    INT_FLASH_DEV_ID,
    BLE_DEV_ID,
    DISPLAY_DEV_ID,
    POWER_DEV_ID,
    AUDIO_DEV_ID,
    BATT_DEV_ID,
} eDeviceID;

typedef enum {
    I2C_DEV_TYPE,    /* devices, connected to i2c bus */
    UART_DEV_TYPE,
    SPI_DEV_TYPE,
    ADC_DEV_TYPE,
    KNOB_DEV_TYPE,
    GPIO_DEV_TYPE,
    INFRARED_DEV_TYPE,
    IO_EXPANDER_DEV_TYPE,
    KEY_MATRIX_DEV_TYPE,
    PWM_DEV_TYPE,
    NVM_DEV_TYPE,
    EEPROM_DEV_TYPE,
	LINEAR_KNOB_DEV_TYPE,
} eDeviceType;

typedef struct tDevice
{
    eDeviceID deviceID;
    eDeviceType deviceType;
}tDevice;

#define DEVICE(nameOfDevice_)     typedef struct nameOfDevice_ nameOfDevice_; \
    struct nameOfDevice_ {      \
        tDevice deviceInfo;

#define END_DEVICE };

#define SUBDEVICE(nameOfDevice_, superDevice_) \
    DEVICE(nameOfDevice_) \
        superDevice_ super_;

/*________LED data types_______________________________*/
#define MAX_BRT  255
#define MIN_BRT  0
#define DIM_BRT  40
#define MORE_DIM_BRT  8
typedef uint32 ledMask; /**< type for Led mapping, i.e. 0x0000 0001 => first Led ID
                                                        0x0000 0002 => second Led ID, etc. */

#ifdef LED_HAS_RGB
typedef uint32 Color;

#define RGBA(r,g,b,a)   ((((r) & 0xFF) << 24) \
                        | (((g) & 0xFF) << 16) \
                        | (((b) & 0xFF) <<  8) \
                        |  ((a) & 0xFF))
#define RED             RGBA(MAX_BRT,       0,       0, MAX_BRT)
#define GREEN           RGBA(      0, MAX_BRT,       0, MAX_BRT)
#define BLUE            RGBA(      0,       0, MAX_BRT, MAX_BRT)
#define AMBER           RGBA(MAX_BRT, MAX_BRT/20+1,  0, MAX_BRT)
#define YELLOW          RGBA(MAX_BRT, MAX_BRT,       0, MAX_BRT)
#define PURPLE          RGBA(MAX_BRT,       0, MAX_BRT, MAX_BRT)
#define WHITE           RGBA(MAX_BRT, MAX_BRT, MAX_BRT, MAX_BRT)
#define BLACK           RGBA(      0,       0,       0,       0)
#define PREVIOUS_COLOR  RGBA(      0,       0,       0, MAX_BRT)

#define DIM_RED             RGBA(DIM_BRT,       0,       0, DIM_BRT)
#define DIM_GREEN           RGBA(      0, DIM_BRT,       0, DIM_BRT)
#define DIM_BLUE            RGBA(      0,       0, DIM_BRT, DIM_BRT)
#define DIM_AMBER           RGBA(DIM_BRT, DIM_BRT/20+1,  0, DIM_BRT)
#define DIM_YELLOW          RGBA(DIM_BRT, DIM_BRT,       0, DIM_BRT)
#define DIM_PURPLE          RGBA(DIM_BRT,       0, DIM_BRT, DIM_BRT)
#define DIM_WHITE           RGBA(DIM_BRT, DIM_BRT, DIM_BRT, DIM_BRT)
#define MORE_DIM_GREEN      RGBA(      0, DIM_BRT,       0, MORE_DIM_BRT)

#define GET_RED(c)      (((c) >> 24) & 0xFF)
#define GET_GREEN(c)    (((c) >> 16) & 0xFF)
#define GET_BLUE(c)     (((c) >>  8) & 0xFF)
#define GET_ALPHA(c)            ((c) & 0xFF)

typedef enum eColorComponent
{
    COLOR_COMPONENT_RED = 0,
    COLOR_COMPONENT_GREEN,
    COLOR_COMPONENT_BLUE,
    COLOR_COMPONENT_BRIGHTNESS,
    COLOR_COMPONENT_MAX = COLOR_COMPONENT_BRIGHTNESS
} eColorComponent;
#else
typedef uint8 Color;
#endif
#define GET_BRIGHTNESS(c)       ((c) & 0xFF)

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
#endif
    uint8           port:4;
    uint8           pin:4;
}tIoExpanderLedMap;

DEVICE(tLedDevice)
    uint8       ledNum;
END_DEVICE

DEVICE(tPwmLedDevice)
    uint8       ledNum;
    const tPwmLedMap    *pPwmLedMap;
END_DEVICE

DEVICE(tIoeLedDevice)
    uint8       ledNum;
    //eDeviceID   i2cDevice;
    const tIoExpanderLedMap *pIoExpanderLedMap;
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
    REG_LEN_16BITS
}eI2CRegAddLen;

/* Defines the prototype to slave callback function */
typedef void (*pdI2C_SLAVE_CALLBACK)( void * );
/*_______________________________________________________*/

/* Devices, connected to I2C bus*/
DEVICE(tI2CDevice)
    eI2CMode i2cMode;
    pdI2C_SLAVE_CALLBACK pvSlaveCallback;
    uint16 address; /* device address */
    eI2C_Channel channel:8;
    eI2CRegAddLen regAddrLen:8;
    uint32 baudRate;
END_DEVICE

/*________UART data types_______________________________*/
typedef enum
{
    TP_UART_DEV_1 = 0,
    TP_UART_DEV_2,
    TP_UART_DEV_3,
    TP_UART_DEV_4,
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
    eTpUartParity   parity;
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
DEVICE(tSPIDevice)
END_DEVICE
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
    IO_PORT_A,IO_PORT_B,IO_PORT_C,IO_PORT_D,IO_PORT_E,IO_PORT_F,IO_PORT_G
}eIoPort;

typedef enum
{
    IO_BIT_0, IO_BIT_1, IO_BIT_2, IO_BIT_3,IO_BIT_4, IO_BIT_5, IO_BIT_6, 
    IO_BIT_7,IO_BIT_8, IO_BIT_9, IO_BIT_10,IO_BIT_11,IO_BIT_12,IO_BIT_13,
    IO_BIT_14,IO_BIT_15,IO_BIT_16,IO_BIT_17,IO_BIT_18,IO_BIT_19,IO_BIT_20,
    IO_BIT_21,IO_BIT_22,IO_BIT_23,IO_BIT_24,IO_BIT_25,IO_BIT_26,IO_BIT_27,
    IO_BIT_28,IO_BIT_29,IO_BIT_30,IO_BIT_31,
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
    GPIO_ACTIVE_HIGH,
    GPIO_ACTIVE_LOW,
    GPIO_ACTIVE_FLOATING
}eGPIOInitAttr;

typedef enum
{
    ODC_ENABLE,
    ODC_DISABLE
}eODC;

typedef struct tGPIOPin
{
    eGPIOId             gpioId:5;
    eIoPort             gpioPort:3;
    eIoBit              gpioBit:5;
    eGPIODrection       gpioDirection:3;
    eGPIOInitAttr       gpioInitAttr:2;
    eODC                openDrainControl:2;
}tGPIOPin;

DEVICE(tGPIODevice)
    uint32              usedGPIOPinNum;     /* how many GPIO pins are Used */
    const tGPIOPin      *pGPIOPinSet;
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
    eIoPort             ioPort:3;         /* IO port */
    eIoBit              gpioBit:5;          /* IO bit */
}tAdcPinIoAttr;

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
    KEY_EVT_MAX
} eKeyEvent;

typedef enum
{
    INVALID_KEY = 0,
    STANDBY_KEY,                // 1
    NET_RESET_KEY,              // 2
    DIRECT_MODE_KEY,            // 3
    VOLUME_DOWN_KEY,            // 4
    VOLUME_UP_KEY,              // 5
    PAIRING_KEY,                // 6
    INPUT_KEY,                  // 7
    MUTE_KEY,                   //
    PLAY_KEY,                   //
    NEXT_KEY,                   //
    PREV_KEY,                   //
    VOL_UP_DOWN_KEY,            //
    PLAY_PAUSE_KEY,             //
    NEXT_PREV_KEY,              //
    STANDBY_PLAY_PAUSE_KEY,     //
    POWER_KEY,                  // 16
    POWER_IR_KEY,
    EJECT_KEY,                  //
    BT_KEY,
    SOURCE_SWITCH_IR_KEY,
    ENTER_KEY,
    RETURN_KEY,
    PRESET_KEY_1_LOAD,
    PRESET_KEY_2_LOAD,
    PRESET_KEY_3_LOAD,
    PRESET_KEY_1_SAVE,
    PRESET_KEY_2_SAVE,
    PRESET_KEY_3_SAVE,
    PRESET_KEY_LOAD_CURR,//Load current data button
    NFC_TAG_KEY,
    /*  knobs buttons with predefined positions */
    VOLUME_KNOB_BASE_KEY_ID,
    BASS_KNOB_BASE_KEY_ID,
    TREBLE_KNOB_BASE_KEY_ID,
    FACTORY_RESET_KEY,
    IR_VOLUME_DOWN_KEY,
    IR_VOLUME_UP_KEY,
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
}tKeyboardDevice;

typedef struct tAdcKey
{
    eKeyID          keyId;
    eAdcPin         adcPin;
    uint16          adcMinimumValue;
    uint16          adcMaxValue;
}tAdcKey;

DEVICE(tADCDevice)
    uint32               ADCEnabledPinNum;      /* how many ADC pins are enabled */
    const tAdcPinIoAttr  *pAdcPinIoAttr;        /* the Io attr of adc pins */
END_DEVICE
    
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
}tGpioKeyboardDevice;

#ifdef HAS_IR_LEARNING

typedef struct 
{
    eKeyID          keyId;
    void          *irMap; // type is actually IR_MAP
}tIrKey;

typedef struct tIrKeyboardDevice
{
    tKeyboardDevice  irKeyboard;
    void                      *attachedDeviceObj;/* void type to solve include confilict problem */
    const tIrKey            *pIrKeySet;
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
#endif


/*__________________________End of keyboard data type_________________________*/

/** \brief get the device by the device ID defined in enum eDeviceID
 * \a devicedID to find
* \a parameter index is used in the case that several devices share the same ID, like LED.
 * this function will change the index value to the first device with the ID, then user
 * need to increase the index to search for next device with the same ID.
*/
const tDevice* getDevicebyId(eDeviceID deviceID, uint16* index);

char* getVersionString();

#ifdef __cplusplus
}
#endif

#endif /* DEVICETYPES_H */
