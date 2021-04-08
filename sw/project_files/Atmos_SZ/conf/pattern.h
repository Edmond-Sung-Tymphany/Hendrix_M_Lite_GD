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
    SOLID_PAT_GREEN,
    SOLID_PAT_BLUE,
    SOLID_PAT_AMBER,
    SOLID_PAT_YELLOW,

    /* dim pattern */
    DIM_PATT_WHITE,

    /* flash pattern */
    FLASH_PAT_AMBER,
    /* quick flash pattern */
    QUICK_FLASH_PAT_RED,
    QUICK_FLASH_PAT_WHITE,
    QUICK_FLASH_ONCE_PAT_WHITE,

    /* pulse pattern */
    PULSE_PAT_WHITE,
    PULSE_PAT_BLUE,
    PULSE_PAT_RED,
    PULSE_PAT_AMBER,

    /* transition pattern */
    TRANS_PAT_RED_1,
    TRANS_PAT_RED_2,
    TRANS_PAT_RED_3,

    TRANS_PAT_BLUE_1,
    TRANS_PAT_BLUE_2,
    TRANS_PAT_BLUE_3,

    TRANS_OFF_PAT_1,
    TRANS_OFF_PAT_2,

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
    ePattern    patternId;
} tLedInd;

/* Please add LED indicaitons according to the UI design of your project. */
typedef enum _eLedIndID
{
    /* Product status */
    LED_IND_ID_POWERING_UP = 0,
    LED_IND_ID_POWERING_DOWN,
    LED_IND_ID_POWERED_ON,
    LED_IND_ID_NETWORK_STANDBY_MODE,
    
    LED_IND_ID_BT_MODE,
    LED_IND_ID_WIFI_MODE,
    LED_IND_ID_AUXIN_MODE,
    LED_IND_ID_SPDIF_MODE,
    LED_IND_ID_DSP_TUNING_MODE,

    /* Network Connectivity */
    LED_IND_ID_ETH_CONNECTED,
    LED_IND_ID_ETH_CONNECTED_IDLE,
    LED_IND_ID_WIFI_SETUP_IN_PROGRESS,
    LED_IND_ID_WIFI_CONNECTED,
    LED_IND_ID_WIFI_CONNECTED_IDLE,
    LED_IND_ID_WIFI_SIG_STRENGTH_STRONG,/*  X > 25%*/ 
    LED_IND_ID_WIFI_SIG_STRENGTH_NORMAL,/*  10 < X <= 25% */
    LED_IND_ID_WIFI_SIG_STRENGTH_WEAK,  /* X<= 10% */
    LED_IND_ID_WIFI_ERROR,
    LED_IND_ID_WIFI_NOT_CONFIGURED,

    /* BT Connectivity */
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
    
    LED_IND_ID_CONN_TRANS_OFF,
    LED_IND_ID_CONN_OFF,
    LED_IND_ID_PROD_OFF,
    LED_IND_ID_ALL_TRANS_OFF,

    LED_IND_ID_ALL_OFF,
    LED_IND_ID_MAX
}eLedIndID;

/* Please config below LED mask according to the HW design of your project. */
typedef enum _eLedMask
{
    LED_MASK_LED0_RED           = 1<<(LED0_RED),
    LED_MASK_LED1_GREEN         = 1<<(LED1_GREEN),
    LED_MASK_LED2_BLUE          = 1<<(LED2_BLUE),
    LED_MASK_LED3_RED           = 1<<(LED3_RED),
    LED_MASK_LED4_GREEN         = 1<<(LED4_GREEN),
    LED_MASK_LED5_BLUE          = 1<<(LED5_BLUE),
    LED_MASK_CONN_LEDS          = (LED_MASK_LED0_RED | LED_MASK_LED1_GREEN | LED_MASK_LED2_BLUE),
    LED_MASK_PROD_LEDS          = (LED_MASK_LED3_RED | LED_MASK_LED4_GREEN | LED_MASK_LED5_BLUE),
    LED_MASK_ALL_LEDS           = (LED_MASK_CONN_LEDS | LED_MASK_PROD_LEDS),

    LED_MASK_MAX
}eLedMask;

#endif
