#ifndef SETTING_ID_H
#define SETTING_ID_H

typedef enum
{
    SETID_START = 0,
    SETID_VOLUME = SETID_START,     // 0
    SETID_CHANNEL,                  // 1
    SETID_IS_AUXIN_PLUG_IN,         // 2
    SETID_IS_SPDIF_IN_PLUG_IN,      // 3
    SETID_AUXIN_MUSIC_DET,          // 4
    SETID_SPDIF_IN_MUSIC_DET,       // 5
    SETID_AUDIO_SOURCE,             // 6
    SETID_ASETK_CONNECTED,          // 7
    SETID_SW_VER,                   // 8
    SETID_HW_VER,                   // 9
    SETID_DSP_VER,                  // 10
    SETID_PCB_SN,                   // 11
    SETID_WF_TEMP,                  // 12
    SETID_TW_TEMP,                  // 13
    SETID_AMP1_TEMP,                // 14
    SETID_AMP2_TEMP,                // 15
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

    SETID_VOLUME_FADE_PARAM,         // 28
    SETID_ASETK_MUSIC_DET,           // 29
    SETID_MAX                       // 30
}eSettingId;



#endif
