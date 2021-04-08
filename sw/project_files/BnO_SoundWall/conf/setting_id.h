#ifndef SETTING_ID_H
#define SETTING_ID_H

// A2B/system mode
#define A2B_MODE_STANDALONE     0
#define A2B_MODE_MASTER         1
#define A2B_MODE_SLAVE          2
#define A2B_MODE_MAX            3
#define A2B_MODE_DEFAULT        A2B_MODE_STANDALONE     // temporary for standalone mode, will change to slave mode later
//#define A2B_MODE_DEFAULT        A2B_MODE_SLAVE

typedef enum
{
    SETID_START = 0,
    SETID_AUDIO_SOURCE = SETID_START,
    SETID_VOLUME,
    SETID_A2B_MODE, // actually, it is NOT the A2B mode, it is system working mode.
    SETID_ENTER_DFU,
    SETID_TYPE_NO,
    SETID_ITEM_NO,
    SETID_SERIAL_NO,
    SETID_HW_VER,
    SETID_BTL_VER,
    SETID_APP_VER,
    SETID_DSP_VER,
    SETID_NTC_INFO,
    SETID_MAX,
}eSettingId;

#endif
