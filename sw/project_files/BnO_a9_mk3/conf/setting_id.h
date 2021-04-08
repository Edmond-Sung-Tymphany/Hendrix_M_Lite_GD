#ifndef SETTING_ID_H
#define SETTING_ID_H

typedef enum
{
    SETID_START = 0,
    SETID_VOLUME = SETID_START,     // 0
    SETID_IS_DSP_ERROR,             // 1
    SETID_AUXIN_JACK_DET,           // 2
    SETID_IS_SPDIF_IN_PLUG_IN,      // 3, not used
    SETID_AUXIN_MUSIC_DET,          // 4, not used
    SETID_SHOP_MODE,                // 5
    SETID_AUDIO_SOURCE,             // 6
    SETID_ASETK_CONNECTED,          // 7, not used
    SETID_SW_VER,                   // 8
    SETID_HW_VER,                   // 9
    SETID_DSP_VER,                  // 10
    SETID_PCB_SN,                   // 11
    SETID_WF_TEMP,                  // 12
    SETID_AMP_TEMP,                 // 13
    SETID_DSP_TEMP,                 // 14
    SETID_AMP_TEMP_LEVEL,           // 15
    SETID_IS_DSP_TUNING,            // 16

    SETID_QUEUE_MIN_DEBUG_SRV,      // 17
    SETID_QUEUE_MIN_SETTING_SRV,    // 18
    SETID_QUEUE_MIN_AUDIO_SRV,      // 19
    SETID_QUEUE_MIN_LEDS_SRV,       // 20
    SETID_QUEUE_MIN_BLUETOOTH_SRV,  // 21
    SETID_QUEUE_MIN_KEYS_SRV,       // 22
    SETID_QUEUE_MIN_POWER_SRV,      // 23
    SETID_QUEUE_MIN_ASE_TK_SRV,     // 24

    SETID_POOL_MIN_SMALL,           // 25
    SETID_POOL_MIN_MEDIUM,          // 26
    SETID_POOL_MIN_LARGE,           // 27

    SETID_AUXIN_PLAYING,             // 28   line in music detect
    SETID_HW_VER_INDEX,              // 29
    SETID_SENSITIVITY_LINEIN,        // 30 sensitivity on line in
    SETID_MUSIC_SPDIF_DET,           // 31
    SETID_MAX_STACK_USAGE,           // 32 maximum stack usage

    SETID_RESERVE_C0,                // 33
    SETID_RESERVE_C1,                // 34
    SETID_ASE_AUDIO_INPUT,           // 35
    SETID_AMP_MUTE_STATUS,           // 36
    SETID_SPEAKER_POSITION,          // 37
    SETID_SPEAKER_ROLE,              // 38
    SETID_AUDIO_AUXIN_IN_DB,         // 39

    SETID_TONE_TOUCH_ENABLED,        // 40
    SETID_TONE_TOUCH_GX1,            // 41
    SETID_TONE_TOUCH_GX2,            // 42
    SETID_TONE_TOUCH_GY1,            // 43
    SETID_TONE_TOUCH_GY2,            // 44
    SETID_TONE_TOUCH_GZ,             // 45
    SETID_TONE_TOUCH_K5,             // 46
    SETID_TONE_TOUCH_K6,             // 47
    SETID_AUDIO_OPTICAL_IN_DB,       //49
    SETID_MAX,                       // 48
}eSettingId;



#endif
