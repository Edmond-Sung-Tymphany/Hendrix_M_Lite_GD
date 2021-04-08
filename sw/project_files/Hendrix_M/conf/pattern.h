#ifndef _PATTERN_H
#define _PATTERN_H

#include "attachedDevices.h"


/* the pattern ID which is defined in UI*/
typedef enum
{
    OFF_PATT = 0,

    /* solid pattern */
    SOLID_PAT_WHITE,
    SOLID_PAT_RED,
    SOLID_PAT_BLUE,
    SOLID_PAT_GREEN,

    /* dim pattern */
    DIM_PATT_WHITE,
    DIM_PATT_RED,
    DIM_PATT_BLUE,

    /* flash pattern */
    FLASH_PAT_WHITE,
    FLASH_PAT_RED,
    FLASH_PAT_BLUE,
    SLOW_FLASH_RED,

    /* flash dim pattern*/
    SLOW_FLASH_RED_DIM,
    FLASH_PAT_RED_DIM,

    /* quick flash pattern */
    QUICK_FLASH_PAT_WHITE,
    QUICK_FLASH_PAT_RED,
    QUICK_FLASH_PAT_BLUE,

    QUICK_FLASH_ONCE_PAT_RED,
    QUICK_FLASH_ONCE_PAT_WHITE_DIM,

    /* pulse pattern */
    PULSE_PAT_WHITE,
    PULSE_PAT_RED,
    PULSE_PAT_BLUE,
    PULSE_PAT_ONECE_RED,

    TRANS_OFF_2_ON_PAT_RED,
    TRANS_OFF_2_ON_PAT_DIM_RED,
    TRANS_PREV_2_ON_PAT_DIM_RED,
    TRANS_PREV_2_ON_PAT_RED,
    TRANS_PREV_2_OFF_PAT,
    PAT_MAX_NUMBER
} ePattern;

typedef enum eIoeAutoPatt
{
    IOE_AUTO_PATT_STBL = 0,
    IOE_AUTO_PATT_ERROR,
    IOE_AUTO_PATT_MAX,
} eIoeAutoPatt;

typedef struct _tLed_Ind
{
    ledMask     leds;
    ePattern    patternId;
} tLedInd;

/* Please add LED indicaitons according to the UI design of your project. */
typedef enum _eLedIndID
{
    /* Product status */
    LED_IND_ID_POWERING_UP = 0,
    LED_IND_ID_POWERING_DOWN,
    LED_IND_ID_POWERED_ON,
    LED_IND_ID_PRODUCT_IDLE,

    LED_IND_ID_AUXIN,
    LED_IND_ID_AUXIN_IDLE,

    /* BT Connectivity */
    LED_IND_ID_BLUETOOTH,
    LED_IND_ID_BLUETOOTH_IDLE,
    LED_IND_ID_BLUETOOTH_CONNECTABLE,
    LED_IND_ID_BLUETOOTH_CONNECTABLE_IDLE,
    LED_IND_ID_BLUETOOTH_PAIRING,
    LED_IND_ID_BLUETOOTH_PAIRING_IDLE,

    /* Batt level */
    LED_IND_ID_BATT_LV_0,
    LED_IND_ID_BATT_LV_10,
    LED_IND_ID_BATT_CHARGING,
    LED_IND_ID_BATT_NOT_CHARGING,
    LED_IND_ID_BATT_LOW,
    LED_IND_ID_BATT_CRITCAL,

    /* Other status */
    LED_IND_ID_TOO_COLD_BATT,
    LED_IND_ID_PROD_FW_ERROR,
    LED_IND_ID_HW_OVER_HEAT,
    LED_IND_ID_FACTORY_RESET_TRIGGERED,
    LED_IND_ID_SHORT_PRESS,
    LED_IND_ID_CONTINUOUS_PRESS, /* only for volume key */
    LED_IND_ID_LP_AND_VLP,       /* for long press and very long press */

    LED_IND_ID_PROD_TRANS_OFF,
    LED_IND_ID_PROD_TRANS_RED,
    LED_IND_ID_PROD_PREV_TRANS_RED,
    LED_IND_ID_PROD_PREV_TRANS_DIM_RED,
    LED_IND_ID_PROD_OFF,

    LED_IND_ID_ALL_OFF,
    LED_IND_ID_MAX
} eLedIndID;

/* Please config below LED mask according to the HW design of your project. */
typedef enum _eLedMask
{
    LED_MASK_PWR_RED           = 1<<(LED_PWR_R),
    LED_MASK_PWR_GREEN         = 1<<(LED_PWR_G),
    LED_MASK_PWR_BLUE          = 1<<(LED_PWR_B),
    LED_MASK_BAT_1           = 1<<(LED_BAT_1),
    LED_MASK_BAT_2           = 1<<(LED_BAT_2),
    LED_MASK_BAT_3           = 1<<(LED_BAT_3),
    LED_MASK_BAT_4           = 1<<(LED_BAT_4),
    LED_MASK_BAT_5           = 1<<(LED_BAT_5),
    LED_MASK_BAT_6           = 1<<(LED_BAT_6),
    LED_MASK_BAT_7           = 1<<(LED_BAT_7),
    LED_MASK_BAT_8           = 1<<(LED_BAT_8),
    LED_MASK_BAT_9           = 1<<(LED_BAT_9),
    LED_MASK_BAT_10          = 1<<(LED_BAT_10),
    LED_MASK_BAT_LV_1        = LED_MASK_BAT_1,
    LED_MASK_BAT_LV_2        = LED_MASK_BAT_LV_1 | LED_MASK_BAT_2,
    LED_MASK_BAT_LV_3        = LED_MASK_BAT_LV_2 | LED_MASK_BAT_3,
    LED_MASK_BAT_LV_4        = LED_MASK_BAT_LV_3 | LED_MASK_BAT_4,
    LED_MASK_BAT_LV_5        = LED_MASK_BAT_LV_4 | LED_MASK_BAT_5,
    LED_MASK_BAT_LV_6        = LED_MASK_BAT_LV_5 | LED_MASK_BAT_6,
    LED_MASK_BAT_LV_7        = LED_MASK_BAT_LV_6 | LED_MASK_BAT_7,
    LED_MASK_BAT_LV_8        = LED_MASK_BAT_LV_7 | LED_MASK_BAT_8,
    LED_MASK_BAT_LV_9        = LED_MASK_BAT_LV_8 | LED_MASK_BAT_9,
    LED_MASK_PROD_LEDS          = (LED_MASK_PWR_RED | LED_MASK_PWR_GREEN | LED_MASK_PWR_BLUE),
    LED_MASK_BAT_CRITCAL     = LED_MASK_BAT_1 | LED_MASK_PROD_LEDS,
    LED_MASK_BAT_LEDS          = (LED_MASK_BAT_1 | LED_MASK_BAT_2 | LED_MASK_BAT_3 | LED_MASK_BAT_4 | LED_MASK_BAT_5 \
                                  | LED_MASK_BAT_6 | LED_MASK_BAT_7 | LED_MASK_BAT_8 | LED_MASK_BAT_9 | LED_MASK_BAT_10),
    LED_MASK_ALL_LEDS           = (LED_MASK_PROD_LEDS | LED_MASK_BAT_LEDS),

    LED_MASK_MAX
} eLedMask;

#endif
