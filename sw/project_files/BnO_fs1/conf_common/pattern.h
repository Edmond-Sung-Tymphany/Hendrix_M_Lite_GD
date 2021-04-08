/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

#ifndef _PATTERN_H
#define _PATTERN_H

#include "attachedDevices.h"


/* the pattern ID which is defined in UI*/
typedef enum
{
    OFF_PATT = 0,
    FG_OFF_PATT,

    //BG solid
    BG_WHITE_PAT,
    BG_RED_PAT,
    BG_ORANGE_PAT,
    BG_BLUE_PAT,
    
    //FG solid
    FG_WHITE_LONG_PAT,
    FG_RED_LONG_PAT,
    FG_RED_LONG_2S_PAT,  //for power down
    FG_BLUE_LONG_2S_PAT,  //BL Paring Success
    FG_WHITE_LONG_10S_PAT, //for playback start/stop
    FG_RED_LONG_10S_PAT, //for playback start/stop
    
    //BG solid dim
    BG_WHITE_DIM_PAT,
    BG_RED_DIM_PAT,
    BG_ORANGE_DIM_PAT,
    BG_BLUE_DIM_PAT,
    
    //BG flash
    BG_WHITE_FLASH_PAT,
    BG_RED_FLASH_PAT,
    BG_ORANGE_FLASH_PAT,
    BG_BLUE_FLASH_PAT,
    
    //BG quick flash
    BG_WHITE_QUICK_FLASH_PAT,
    BG_RED_QUICK_FLASH_PAT,
    BG_ORANGE_QUICK_FLASH_PAT,
    FG_WHITE_QUICK_FLASH_ONCE_PAT,
    FG_RED_QUICK_FLASH_ONCE_PAT,
    FG_WHITE_QUICK_FLASH_TWICE_PAT,
    FG_RED_QUICK_FLASH_TWICE_PAT,
    FG_WHITE_QUICK_FLASH_THRICE_PAT,
    FG_RED_QUICK_FLASH_2S_PAT,
    FG_RED_QUICK_FLASH_10S_PAT,
    
    //BG slow flash
    BG_ORANGE_SLOW_FLASH_PAT,
    
    //BG pulse
    BG_WHITE_PULSE_PAT,
    BG_RED_PULSE_PAT,
    BG_ORANGE_PULSE_PAT,
    BG_BLUE_PULSE_PAT,
            
    //BG trans to OFF
    FG_WHITE_TRANS_TO_OFF_PART1_PAT,
    FG_WHITE_TRANS_TO_OFF_PART2_PAT,
    FG_WHITE_TRANS_TO_OFF_PART3_PAT,
    FG_PREV_TRANS_TO_OFF_PART1_PAT,
    FG_PREV_TRANS_TO_OFF_PART2_PAT,
    FG_PREV_TRANS_TO_OFF_PART3_PAT,
    
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
    LED_IND_ID_POWERING_UP = 0,
    LED_IND_ID_POWERING_DOWN_CONN_WHITE,
    LED_IND_ID_POWERING_DOWN_CONN_PRIV,
    LED_IND_ID_POWERING_DOWN_PROD_WHITE,
    LED_IND_ID_POWERING_DOWN_PROD_PREV,
    LED_IND_ID_POWERING_DOWN_ERROR,
    
    LED_IND_ID_NETWORK_STANDBY_PROD,
    LED_IND_ID_NETWORK_STANDBY_CONN,
    LED_IND_ID_ENTER_STORAGE,
    LED_IND_ID_WIFI_UNCONFIGURED,
    LED_IND_ID_WIFI_UNCONFIGURED_IDLE,
    LED_IND_ID_RST_IN_PROG,
    
    LED_IND_ID_WIFI_MODE_QUALITY_EXELENT,
    LED_IND_ID_WIFI_MODE_QUALITY_EXELENT_IDLE,
    LED_IND_ID_WIFI_MODE_QUALITY_GOOD,
    LED_IND_ID_WIFI_MODE_QUALITY_GOOD_IDLE,
    LED_IND_ID_WIFI_MODE_QUALITY_POOR,
    LED_IND_ID_WIFI_MODE_QUALITY_POOR_IDLE,
    LED_IND_ID_WIFI_CONNECTING,
    LED_IND_ID_WIFI_CONNECT_FAIL,
    LED_IND_ID_ETHERNET_MODE,
    LED_IND_ID_ETHERNET_MODE_IDLE,
    LED_IND_ID_SOFTAP_MODE,
    LED_IND_ID_ERROR,
    
    LED_IND_ID_TOUCH_KEY_WHT_ON_LONG,
    LED_IND_ID_TOUCH_KEY_RED_ON_LONG,
    LED_IND_ID_TOUCH_KEY_FLASH_WHT_ONCE,
    
    LED_IND_ID_TOUCH_KEY_FLASH_RED_ONCE,    
    LED_IND_ID_TOUCH_KEY_FLASH_WHT_TWICE,
    LED_IND_ID_TOUCH_KEY_FLASH_RED_TWICE,
    LED_IND_ID_TOUCH_KEY_FLASH_WHT_THRICE,
    LED_IND_ID_TOUCH_KEY_FLASH_RED_THRICE,
    LED_IND_ID_KEY_SP,
    LED_IND_ID_KEY_LP,
    LED_IND_ID_KEY_VLP,
    LED_IND_ID_KEY_DP,
    LED_IND_ID_KEY_SWIPE,
    
    LED_IND_ID_FACTORY_RESET_TRIGGER,
    LED_IND_ID_PLAYBACK_START,
    LED_IND_ID_PLAYBACK_STOP,
    LED_IND_ID_BT_PAIRING_ENABLED,
    LED_IND_ID_BT_PAIRING_FAIL,
    LED_IND_ID_BT_PAIRING_SUCCESS,
        
    LED_IND_ID_BATT_LOW_NO_DC,
    LED_IND_ID_BATT_LOW_HAVE_DC,
    
    LED_IND_ID_FIRST_BOOT,
    LED_IND_ID_SW_UPDATING,
    LED_IND_ID_EXT_SOURCE, //fs1:SPDIF, fs2:LINE-IN
    LED_IND_ID_DSP_ONLINE_TUNING,
    LED_IND_ID_FAST_BOOT,
    
    LED_IND_ID_CONN_FG_OFF,
    LED_IND_ID_PROD_FG_OFF,
    LED_IND_ID_ALL_FG_OFF,

    LED_IND_ID_CONN_OFF,
    LED_IND_ID_PROD_OFF,    
    LED_IND_ID_ALL_BG_OFF,
    LED_IND_ID_MAX
}eLedIndID;

/* Please config below LED mask according to the HW design of your project. */
typedef enum _eLedMask
{
    LED_MASK_CONN_LED_WHT   = 1<<(LED_CONN_WHT),
    LED_MASK_CONN_LED_BLUE  = 1<<(LED_CONN_BLUE),
    LED_MASK_CONN_LED_ORG   = 1<<(LED_CONN_ORG),
    LED_MASK_CONN_LED_RED   = 1<<(LED_CONN_RED),
    LED_MASK_PROD_LED_WHT   = 1<<(LED_PROD_WHT),
    LED_MASK_PROD_LED_RED   = 1<<(LED_PROD_RED),
    LED_MASK_CONN_LEDS      = (LED_MASK_CONN_LED_WHT | LED_MASK_CONN_LED_BLUE | LED_MASK_CONN_LED_ORG | LED_MASK_CONN_LED_RED),
    LED_MASK_PROD_LEDS      = (LED_MASK_PROD_LED_WHT | LED_MASK_PROD_LED_RED),
    LED_MASK_ALL_LEDS       = (LED_MASK_CONN_LEDS | LED_MASK_PROD_LEDS),

    LED_MASK_MAX
}eLedMask;

#endif
