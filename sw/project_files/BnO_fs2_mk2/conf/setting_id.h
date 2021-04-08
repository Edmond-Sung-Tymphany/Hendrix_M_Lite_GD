#ifndef SETTING_ID_H
#define SETTING_ID_H


/* We resever empty slot on every part.
 * Then we are easy to add itmes to each part, and do not impact setting id. It is good for produciton tool
 */
typedef enum
{
    SETID_START  = 0,           // 0
        
    // Common part, audio
    SETID_VOLUME = SETID_START, // 0
    SETID_CHANNEL,           // 1
    SETID_AUDIO_SOURCE,      // 2
    SETID_MUSIC_DET,         // 3 
    SETID_TEMP_LEVEL_AUDIO,  // 4
    SETID_AMP_HEALTH,        // 5
    SETID_RESERVE_A1,        // 6
    SETID_BOOT_REQUEST,      // 7
    SETID_AMP_ERROR_REASON,  // 8
    SETID_MAX_STACK_USAGE,   // 9
    
    
    // Common part, other
    SETID_SW_FW_VER,            // 10
    SETID_HW_VER,               // 11
    SETID_DSP_VER,              // 12
    SETID_ASETK_CONNECTED,      // 13
    SETID_IQS572_CONNECTED,     // 14
    SETID_IQS360A_CONNECTED,    // 15
    SETID_SYSTEM_SLEEP,         // 16    
    SETID_IS_DC_PLUG_IN,        // 17
    SETID_ERROR_REASON,         // 18
    SETID_GAIN_ALLOW,           // 19
        
        
    // Common part, system tuning
    SETID_QUEUE_MIN_DEBUG_SRV,      // 20
    SETID_QUEUE_MIN_SETTING_SRV,    // 21
    SETID_QUEUE_MIN_AUDIO_SRV,      // 22
    SETID_QUEUE_MIN_LEDS_SRV,       // 23
    SETID_QUEUE_MIN_BLUETOOTH_SRV,  // 24
    SETID_QUEUE_MIN_KEYS_SRV,       // 25
    SETID_QUEUE_MIN_POWER_SRV,      // 26
    SETID_QUEUE_MIN_ASE_TK_SRV,     // 27
    SETID_POOL_MIN_SMALL,           // 28
    SETID_POOL_MIN_MEDIUM,          // 29
    SETID_POOL_MIN_LARGE,           // 30   
    SETID_DSP_PARAM_POSITION,       // 31
    SETID_DSP_PARAM_BASS,           // 32
    SETID_DSP_PARAM_LOUDNESS,       // 33   
    SETID_DSP_PARAM_TREBLE,         // 34
    SETID_TEMP_LEVEL_SYSTEM,        // 35
    SETID_RESERVE_C6,               // 36   
    SETID_AUDIO_SIGNAL_VOL_INPUT,   // 37
    SETID_SW_PIU_VER,               // 38
    SETID_SW_UBL_VER,               // 39
        
    
    //FS2 Temperature
    SETID_TEMP_WF_AMP_1,    // 40, PC0, RT1
    SETID_TEMP_WF_AMP_2,    // 41, PA6, RT3
    SETID_TEMP_WF_SPK,      // 42, PC1
    SETID_TEMP_MID_SPK_A,   // 43, PC2
    SETID_TEMP_MID_SPK_B,   // 44, PC3
    SETID_TEMP_TW_AMP,      // 45, PA5 
    SETID_AUDIO_SIGNAL_VOL_WF_OUTPUT,       // 46
    SETID_AUDIO_SIGNAL_VOL_MID_A_OUTPUT,    // 47
    SETID_AUDIO_SIGNAL_VOL_MID_B_OUTPUT,    // 48   
    SETID_AUDIO_SIGNAL_VOL_TW_OUTPUT,       // 49
        
    
    //FS2 Audio
    SETID_DSP_CAL_GAIN1_WF,     // 50, woofer
    SETID_DSP_CAL_GAIN2_MID_A,  // 51, middle-A
    SETID_DSP_CAL_GAIN3_MID_B,  // 52, middle-B
    SETID_DSP_CAL_GAIN4_TW,     // 53, tweeter

    SETID_SENSITIVITY_LINEIN,   // 54 sensitivity on line in
    SETID_AUDIO_AUXIN_IN_DB,    // 55 
    SETID_AUDIO_SYS_IN_DB,      // 56
    SETID_AUDIO_OUT_WF_DB,      // 57
    SETID_AUDIO_OUT_MID_A_DB,   // 58
    SETID_AUDIO_OUT_MID_B_DB,   // 59
    SETID_AUDIO_OUT_TW_DB,      // 60
    
    SETID_DSP_OVERHEAT_GAIN_WF, //61
    SETID_DSP_OVERHEAT_GAIN_MID, //62
    SETID_DSP_OVERHEAT_GAIN_TW,  //63
    SETID_DSP_OVERHEAT_COIL_TEMP_WF, //64
    SETID_DSP_OVERHEAT_COIL_TEMP_MID_A, //65
    SETID_DSP_OVERHEAT_COIL_TEMP_MID_B, //66
    SETID_DSP_OVERHEAT_COIL_TEMP_TW, //67
    SETID_DSP_CHANNEL, //68

    SETID_MAX                   // 69
}eSettingId;

#endif
