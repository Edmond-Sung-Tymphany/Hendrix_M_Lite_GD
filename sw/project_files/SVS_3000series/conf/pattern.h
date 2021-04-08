#ifndef _PATTERN_H
#define _PATTERN_H

#include "attachedDevices.h"


/* the pattern ID which is defined in UI*/
typedef enum
{
    OFF_PATT = 0,

    PAT_MAX_NUMBER
}ePattern;

typedef enum eIoeAutoPatt
{
    IOE_AUTO_PATT_STBL = 0,
    IOE_AUTO_PATT_ERROR,
    IOE_AUTO_PATT_MAX,
}eIoeAutoPatt;

typedef struct _tLed_Ind
{
    ledMask     leds;
    Color       brightness;
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
    LED_IND_ID_WIFI_MODE,
    LED_IND_ID_AUXIN_MODE,
    LED_IND_ID_SPDIF_MODE,
    LED_IND_ID_DSP_TUNING_MODE,
    LED_IND_ID_FAST_BOOT_MODE,

    /* Network Connectivity */
    LED_IND_ID_ETH_CONNECTED,
    LED_IND_ID_ETH_CONNECTED_IDLE,
    LED_IND_ID_WIFI_CONNECTING,
    LED_IND_ID_WIFI_SETUP_IN_PROGRESS,
    LED_IND_ID_WIFI_SIG_STRENGTH_STRONG,/*  X > 25%*/
    LED_IND_ID_WIFI_SIG_STRENGTH_STRONG_IDLE,
    LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL,/*  10 < X <= 25% */
    LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL_IDLE,
    LED_IND_ID_WIFI_SIG_STRENGTH_WEAK,  /* X<= 10% */
    LED_IND_ID_WIFI_SIG_STRENGTH_WEAK_IDLE,
    LED_IND_ID_WIFI_ERROR,
    LED_IND_ID_WIFI_ERROR_IDLE,
    LED_IND_ID_WIFI_UNCONFIGURED,
    LED_IND_ID_WIFI_UNCONFIGURED_IDLE,
    LED_IND_ID_BLUETOOTH,
    LED_IND_ID_BLUETOOTH_IDLE,

    /* BT Connectivity */
    LED_IND_ID_BLE_PAIRING_ENABLED,
    LED_IND_ID_BT_PAIRING_SUCCESSFULL,
    LED_IND_ID_BT_PAIRING_FAIL,

    /* Other status */
    LED_IND_ID_SW_UPDATING,
    LED_IND_ID_PROD_FW_ERROR,
    LED_IND_ID_HW_OVER_HEAT,
    LED_IND_ID_FACTORY_RESET_TRIGGERED,
    LED_IND_ID_SHORT_PRESS,
    LED_IND_ID_CONTINUOUS_PRESS, /* only for volume key */
    LED_IND_ID_LP_AND_VLP,       /* for long press and very long press */
    LED_IND_ID_SHOP_MODE_POWERING_UP_PROD,
    LED_IND_ID_SHOP_MODE_POWERING_UP_CONN,
    LED_IND_ID_SHOP_MODE_PROD,
    LED_IND_ID_SHOP_MODE_CONN,
    
    LED_IND_ID_CONN_TRANS_ON_2_OFF_WHITE,
    LED_IND_ID_CONN_TRANS_ON_2_OFF_AMBER,
    LED_IND_ID_CONN_TRANS_ON_2_OFF_RED,
    LED_IND_ID_CONN_TRANS_ON_2_OFF_BLUE,
    LED_IND_ID_CONN_TRANS_DIM_WHITE,
    LED_IND_ID_CONN_TRANS_DIM_AMBER,
    LED_IND_ID_CONN_TRANS_DIM_RED,
    LED_IND_ID_CONN_TRANS_DIM_BLUE,
    LED_IND_ID_CONN_TRANS_DIM_2_OFF_WHITE,
    LED_IND_ID_CONN_TRANS_DIM_2_OFF_AMBER,
    LED_IND_ID_CONN_TRANS_DIM_2_OFF_RED,
    LED_IND_ID_CONN_TRANS_DIM_2_OFF_BLUE,
    LED_IND_ID_PROD_TRANS_DIM_2_OFF_WHITE,
    LED_IND_ID_PROD_TRANS_OFF,
    LED_IND_ID_CONN_OFF,
    LED_IND_ID_PROD_OFF,
    LED_IND_ID_ALL_TRANS_OFF,

    LED_IND_ID_ALL_OFF,
    LED_IND_ID_MAX
}eLedIndID;

/* Please config below LED mask according to the HW design of your project. */
typedef enum _eLedMask
{
    LED_MASK_LED_BAR_1          = 1 << (LED_BAR_1),
    LED_MASK_LED_BAR_2          = 1 << (LED_BAR_2),
    LED_MASK_LED_BAR_3          = 1 << (LED_BAR_3),
    LED_MASK_LED_BAR_4          = 1 << (LED_BAR_4),
    LED_MASK_LED_BAR_5          = 1 << (LED_BAR_5),
    LED_MASK_LED_BAR_6          = 1 << (LED_BAR_6),
    LED_MASK_LED_BAR_7          = 1 << (LED_BAR_7),
    LED_MASK_LED_BAR_8          = 1 << (LED_BAR_8),
    LED_MASK_LED_BAR_9          = 1 << (LED_BAR_9),
    LED_MASK_LED_BAR_10         = 1 << (LED_BAR_10),
    LED_MASK_LED_BAR_11         = 1 << (LED_BAR_11),

    LED_MASK_LED_PLUS           = 1 << (LED_PLUS),
    LED_MASK_LED_MINUS          = 1 << (LED_MINUS),
    LED_MASK_LED_VOL            = 1 << (LED_VOL),
    LED_MASK_LED_STBY_BLUE      = 1 << (LED_STBY_BLUE),
    LED_MASK_LED_STBY_AMBER     = 1 << (LED_STBY_AMBER),

    LED_MASK_LED_UPGRADING      = (LED_MASK_LED_BAR_1 | LED_MASK_LED_BAR_2  |
                                   LED_MASK_LED_BAR_3 | LED_MASK_LED_BAR_9  |
                                   LED_MASK_LED_BAR_10 | LED_MASK_LED_BAR_11),

    LED_MASK_LED_STBY           = (LED_MASK_LED_STBY_BLUE | LED_MASK_LED_STBY_AMBER),

    LED_MASK_LED_PLUS_MINUS     = (LED_MASK_LED_PLUS  | LED_MASK_LED_MINUS),

    LED_MASK_LED_BAR            = (LED_MASK_LED_BAR_1 | LED_MASK_LED_BAR_2  |
                                   LED_MASK_LED_BAR_3 | LED_MASK_LED_BAR_4  |
                                   LED_MASK_LED_BAR_5 | LED_MASK_LED_BAR_6  |
                                   LED_MASK_LED_BAR_7 | LED_MASK_LED_BAR_8  |
                                   LED_MASK_LED_BAR_9 | LED_MASK_LED_BAR_10 |
                                   LED_MASK_LED_BAR_11),

    LED_MASK_ALL_LEDS           = (LED_MASK_LED_BAR         | LED_MASK_LED_STBY   |
                                   LED_MASK_LED_PLUS_MINUS  | LED_MASK_LED_VOL),

    LED_MASK_MAX
}eLedMask;

#endif
