#ifndef __IoeLedDrv_H__
#define __IoeLedDrv_H__

typedef enum tagIoeLedIndex
{
    LED_ID_START=0,
    LED_ID_RED=LED_ID_START,
    LED_ID_GREEN,
    LED_ID_BLUE,
    LED_ID_MAX
}IoeLedIndex_t;

// led mode
#define IOE_LED_MODE_NORMAL     0
#define IOE_LED_MODE_BLINK      1
#define IOE_LED_MODE_MAX        2

// default dim value of NORMAL mode
#define IOE_DEFAULT_DIM_VALUE   0xff
#define IOE_DIM_VALUE_OFF       0x00

// blink mode 
#define LED_BLINK_MODE_NONE     0
#define LED_BLINK_MODE_FAST     1
#define LED_BLINK_MODE_MEDIUM   2
#define LED_BLINK_MODE_SLOW     3
#define LED_BLINK_MODE_APPEND   4
#define LED_FLASH_MODE_FAST     5
#define LED_FLASH_MODE_MEDIUM   6
#define LED_FLASH_MODE_SLOW     7
#define LED_BLINK_MODE_MAX      8

void IoeLed_I2cEnable(bool enable);
void IoeLed_Ctor(void);
void IoeLed_Reset(void);
void IoeLed_Init(void);
void IoeLed_Refresh(void);
void IoeLed_SetupMode(IoeLedIndex_t led_id, uint8_t mode, uint8_t blink_mode, uint8_t dim_value);
void IoeLed_Standby(void);

#endif  // __IoeLedDrv_H__

