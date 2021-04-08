#ifndef _PATTERN_H
#define _PATTERN_H

#include "attachedDevices.h"


/* the pattern ID which is defined in UI*/
typedef enum
{
    OFF_PATT = 0,

    /* solid pattern */
    SOLID_PAT_RED,

    /* dim pattern */
    DIM_PATT_RED,


    /* flash pattern */
    FLASH_PAT_RED,

    /* quick flash pattern */
    QUICK_FLASH_PAT_RED,

    QUICK_FLASH_ONCE_PAT_RED,

    /* pulse pattern */
    PULSE_PAT_RED,

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

    LED_IND_ID_BT_MODE,
    LED_IND_ID_AUXIN_MODE,
    LED_IND_ID_RCA_MODE,
    LED_IND_ID_DSP_TUNING_MODE,
    LED_IND_ID_FAST_BOOT_MODE,

    /* Network Connectivity */
    LED_IND_ID_BLUETOOTH,
    LED_IND_ID_BLUETOOTH_IDLE,

    /* BT Connectivity */
    LED_IND_ID_BT_PAIRING_ENABLED,
    LED_IND_ID_BT_PAIRING_SUCCESSFULL,
    LED_IND_ID_BT_RECONNECTING,
    
    /* Batt level */
    LED_IND_ID_BATT_LV_0,
    LED_IND_ID_BATT_LV_1,
    LED_IND_ID_BATT_LV_2,
    LED_IND_ID_BATT_LV_3,
    LED_IND_ID_BATT_LV_4,
    LED_IND_ID_BATT_LV_5,
    LED_IND_ID_BATT_LV_6,
    LED_IND_ID_BATT_LV_7,
    LED_IND_ID_BATT_LV_8,
    LED_IND_ID_BATT_LV_9,
    LED_IND_ID_BATT_LV_10,


    /* Other status */
    LED_IND_ID_OFF_ALL_SRC_LED,
    LED_IND_ID_SW_UPDATING,
    LED_IND_ID_PROD_FW_ERROR,
    LED_IND_ID_FACTORY_RESET_TRIGGERED,
    LED_IND_ID_SHORT_PRESS,
    LED_IND_ID_CONTINUOUS_PRESS, /* only for volume key */
    LED_IND_ID_LP_AND_VLP,       /* for long press and very long press */


    LED_IND_ID_PROD_TRANS_OFF,
    LED_IND_ID_CONN_OFF,
    LED_IND_ID_PROD_OFF,
    LED_IND_ID_ALL_TRANS_OFF,

    LED_IND_ID_ALL_OFF,
    LED_IND_ID_MAX
} eLedIndID;

/* Please config below LED mask according to the HW design of your project. */
typedef enum _eLedMask
{
    LED_MASK_PWR            = 1<<(LED_PWR),
    LED_MASK_BT             = 1<<(LED_BT),
    LED_MASK_AUX            = 1<<(LED_AUX),
    LED_MASK_RCA            = 1<<(LED_RCA),
    LED_MASK_ALL_SOURCE_LEDS = ((LED_MASK_BT) | (LED_MASK_AUX) | (LED_MASK_RCA)),

    LED_MASK_VOL_1           = 1<<(LED_VOL_1),
    LED_MASK_VOL_2           = 1<<(LED_VOL_2),
    LED_MASK_VOL_3           = 1<<(LED_VOL_3),
    LED_MASK_VOL_4           = 1<<(LED_VOL_4),
    LED_MASK_VOL_5           = 1<<(LED_VOL_5),
    LED_MASK_VOL_6           = 1<<(LED_VOL_6),
    LED_MASK_VOL_7           = 1<<(LED_VOL_7),
    LED_MASK_VOL_8           = 1<<(LED_VOL_8),
    LED_MASK_VOL_9           = 1<<(LED_VOL_9),
    LED_MASK_VOL_10          = 1<<(LED_VOL_10),
    LED_MASK_VOL_LV_1        = LED_MASK_VOL_1,
    LED_MASK_VOL_LV_2        = LED_MASK_VOL_LV_1 | LED_MASK_VOL_2,
    LED_MASK_VOL_LV_3        = LED_MASK_VOL_LV_2 | LED_MASK_VOL_3,
    LED_MASK_VOL_LV_4        = LED_MASK_VOL_LV_3 | LED_MASK_VOL_4,
    LED_MASK_VOL_LV_5        = LED_MASK_VOL_LV_4 | LED_MASK_VOL_5,
    LED_MASK_VOL_LV_6        = LED_MASK_VOL_LV_5 | LED_MASK_VOL_6,
    LED_MASK_VOL_LV_7        = LED_MASK_VOL_LV_6 | LED_MASK_VOL_7,
    LED_MASK_VOL_LV_8        = LED_MASK_VOL_LV_7 | LED_MASK_VOL_8,
    LED_MASK_VOL_LV_9        = LED_MASK_VOL_LV_8 | LED_MASK_VOL_9,
    LED_MASK_PROD_LEDS       = (LED_MASK_PWR),
    LED_MASK_VOL_LEDS        = (LED_MASK_VOL_1 | LED_MASK_VOL_2 | LED_MASK_VOL_3 | LED_MASK_VOL_4 | LED_MASK_VOL_5 \
                                  | LED_MASK_VOL_6 | LED_MASK_VOL_7 | LED_MASK_VOL_8 | LED_MASK_VOL_9 | LED_MASK_VOL_10),
    LED_MASK_ALL_LEDS        = (0x0fffffffffffffffULL),

    LED_MASK_MAX
} eLedMask;

#endif
