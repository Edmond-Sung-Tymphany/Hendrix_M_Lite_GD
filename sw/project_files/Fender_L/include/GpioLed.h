#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

#define GPIO_LED_ALWAYS_FLASH       0xf0

typedef enum tagGpioLedID
{
    GLED_ID_START,
    GLED_ID_AUX=GLED_ID_START,
    GLED_ID_POWER,
    GLED_ID_RCA,
    GLED_ID_BT,
    GLED_ID_END,
// the following LED index will be ignored, if you want to add more LED, put it before GLED_ID_END.
    GLED_ID_END_end_end
}GLED_ID_t;

typedef enum tagGpioLedMode
{
    GLED_MODE_OFF = 0,
    GLED_MODE_ON = 1,   // the OFF/ON mode must be fixed 0/1
    GLED_MODE_SLOW_FLASH,
    GLED_MODE_FLASH,
    GLED_MODE_DOUBLE_FLASH,
    GLED_MODE_QUICK_FLASH,
    GLED_MODE_LAST_MODE
}GLED_MODE_t;

typedef struct tagGpioLedController
{
    GLED_ID_t       gled_id;
    GLED_MODE_t     gled_mode;
    uint8_t         repeat;
    uint8_t         cur_tick;
    uint8_t         cur_on;
    uint8_t         finally_on;
}GLED_CONTROLLER_t;

void GLED_Init(void);
void GLED_SetupMode(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt);
void GLED_SetupModeWithCheck(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt);
void GLED_SetupMode_2(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt, uint8_t finally_on);
void GLED_SetupOnOffMode(GLED_ID_t gled_id, GLED_MODE_t mode);
void GLED_PeriodicRefresh(void);

#endif  // __GPIO_LED_H__

