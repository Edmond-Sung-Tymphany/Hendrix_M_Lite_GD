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
    SETID_MAX_STACK_USAGE,       // 9
    
    
    // Common part, other
    SETID_SW_VER,               // 10
    SETID_HW_VER,               // 11
    SETID_DSP_VER,              // 12
    SETID_ASETK_CONNECTED,      // 13
    SETID_IQS572_CONNECTED,     // 14
    SETID_IQS360A_CONNECTED,    // 15
    SETID_SYSTEM_SLEEP,         // 16    
    SETID_IS_DC_PLUG_IN,        // 17
    SETID_ERROR_REASON,         // 18
    SETID_RESERVE_B2,           // 19
        
        
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
    SETID_RESERVE_C1,               // 31
    SETID_RESERVE_C2,               // 32
    SETID_RESERVE_C3,               // 33   
    SETID_RESERVE_C4,               // 34
    SETID_RESERVE_C5,               // 35
    SETID_RESERVE_C6,               // 36   
    SETID_RESERVE_C7,               // 37
    SETID_RESERVE_C8,               // 38
    SETID_RESERVE_C9,               // 39
        
    
    //FS1 Audio
    SETID_TEMP_WF_AMP,       // 40, Woofer, ampliifer
    SETID_TEMP_WF_SPK,       // 41, woofer, spekaer
    SETID_TEMP_TW_SPK,       // 42, tweeter spekaer
    SETID_AUDIO_SIGNAL_VOL_INPUT,  // 43
    SETID_AUDIO_SIGNAL_VOL_OUTPUT, // 44
    SETID_RESERVE_D3,        // 45
    SETID_RESERVE_D4,        // 46
    SETID_RESERVE_D5,        // 47
    SETID_RESERVE_D6,        // 48
    SETID_RESERVE_D7,        // 49        
        
    
    //FS1 Audio
    SETID_DSP_CAL_GAIN1_WF,  // 50, woofer
    SETID_DSP_CAL_GAIN2_TW,  // 51, tweeter
    SETID_DSP_OVERHEAT_GAIN_TW,       // 52
    SETID_DSP_OVERHEAT_GAIN_WF,       // 53
    SETID_DSP_OVERHEAT_COIL_TEMP_WF,  // 54
    SETID_DSP_OVERHEAT_COIL_TEMP_TW,  // 55
    SETID_RESERVE_E5,        // 56
    SETID_RESERVE_E6,        // 57
    SETID_RESERVE_E7,        // 58
    SETID_RESERVE_E8,        // 59
        
        
    //FS1 Power
    SETID_CHARGER_STATUS,  // 60
    SETID_5V_SEN,          // 60
    SETID_PVDD_SEN,        // 61
    SETID_RESERVE_F1,      // 63
    SETID_RESERVE_F2,      // 64
    SETID_RESERVE_F3,      // 65
    SETID_RESERVE_F4,      // 66
    SETID_RESERVE_F5,      // 67
    SETID_RESERVE_F6,      // 68
    SETID_RESERVE_F7,      // 69

    
    //FS1 Battery, Charger
    SETID_BATTERY_SN,               // 70
    SETID_BATTERY_HW,               // 71
    SETID_BATTERY_SW,               // 72
    SETID_BATTERY_TEMP,             // 73
    SETID_BATTERY_TEMP_LEVEL,       // 74
    SETID_BATTERY_CAPACITY_RSOC,    // 75
    SETID_BATTERY_CAPACITY_ASOC,    // 76
    SETID_BATTERY_CAPACITY_LEVEL,   // 77
    SETID_BATTERY_HEALTH_SOH,       // 78
    SETID_BATTERY_HEALTH_LEVEL,     // 79
    SETID_BATTERY_CYCLE,            // 80
    SETID_BATTERY_CHARGING_VOLTAGE, // 81
    SETID_BATTERY_CHARGING_CURRENT, // 82
    SETID_BATTERY_SAFETY_STATUS,    // 83
    SETID_BATTERY_TEMP_CELL1,       // 84
    SETID_BATTERY_TEMP_CELL2,       // 85
    SETID_BATTERY_EXIST,            // 86
    SETID_BATTERY_CURRENT,          // 87
    SETID_BATTERY_AVG_CURRENT,      // 88
    SETID_BATTERY_CAPACITY_RSOC_USER,// 89
    SETID_SYSTEM_SLEEP_CHARGING,     // 90
	
    SETID_MAX                       // 91
}eSettingId;

#endif
