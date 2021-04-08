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
    SETID_DSP_HEALTH,               // 36   
    SETID_AUDIO_SIGNAL_VOL_INPUT,   // 37
    SETID_SW_PIU_VER,               // 38
    SETID_SW_UBL_VER,               // 39
        
    
    //FS1 Audio
    SETID_TEMP_WF_AMP,       // 40, Woofer, ampliifer
    SETID_TEMP_WF_SPK,       // 41, woofer, spekaer
    SETID_TEMP_TW_SPK,       // 42, tweeter spekaer
    SETID_AUDIO_SIGNAL_VOL_WF_OUTPUT,  // 43
    SETID_AUDIO_SIGNAL_VOL_TW_OUTPUT,  // 44
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
    SETID_DSP_DBOOST_LEVEL,  // 56
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

    
    //FS1 Battery
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
    SETID_BATTERY_SAFETY_STATUS_HIGH,// 83
    SETID_BATTERY_SAFETY_STATUS_LOW, // 84
    SETID_BATTERY_TEMP_CELL1,       // 85
    SETID_BATTERY_TEMP_CELL2,       // 86
    SETID_BATTERY_EXIST,            // 87
    SETID_BATTERY_CURRENT,          // 88
    SETID_BATTERY_AVG_CURRENT,      // 89
    SETID_BATTERY_CAPACITY_RSOC_USER,// 90
    SETID_BATTERY_PACK_STATUS,       // 91
    SETID_BATTERY_TOTAL_VOL,         // 92
    SETID_BATTERY_STATUS,            // 93
    SETID_BATTERY_REMAIN_CAPACITY,   // 94
    SETID_BATTERY_FULL_CH_CAPACITY,  // 95
    SETID_BATTERY_DESIGN_CAPACITY,   // 96
    SETID_BATTERY_LOW_BATT_SHUT_DOWN,// 97
    SETID_BATTERY_PF_STATUS_HIGH,    // 98
    SETID_BATTERY_PF_STATUS_LOW,     // 99
    SETID_BATTERY_VOLT_CELL1,        // 100
    SETID_BATTERY_VOLT_CELL2,        // 101
                                          
                                          
    //FS1 Charger                         
    SETID_SYSTEM_SLEEP_CHARGING,     // 102
    SETID_RESERVE_H1,                // 103
    SETID_RESERVE_H2,                // 104
    SETID_RESERVE_H3,                // 105
    SETID_RESERVE_H4,                // 106
    SETID_RESERVE_H5,                // 107
    SETID_RESERVE_H6,                // 108
    SETID_RESERVE_H7,                // 109
    SETID_RESERVE_H8,                // 110
    SETID_RESERVE_H9,                // 111


    //FS1 Battery Faultlog
    SETID_BATTERY_FL_OVP_CNT,           // 112
    SETID_BATTERY_FL_UVP_CNT,           // 113
    SETID_BATTERY_FL_COT_CNT,           // 114
    SETID_BATTERY_FL_CUT_CNT,           // 115
    SETID_BATTERY_FL_DOT_CNT,           // 116
    SETID_BATTERY_FL_DUT_CNT,           // 117
    SETID_BATTERY_FL_AFEE_CNT,          // 118
    SETID_BATTERY_FL_DOC_CNT,           // 119
    SETID_BATTERY_FL_COC_CNT,           // 120
    SETID_BATTERY_FL_CHRG_AUTH_FAIL_CNT,// 121
    SETID_BATTERY_FL_CHRG_OVP_CNT,      // 122
    SETID_BATTERY_FL_CHRG_COT_CNT,      // 123
    SETID_BATTERY_FL_ADC_SAT_FAIL_CNT,  // 124
    SETID_BATTERY_FL_AFECF_COUNT,       // 125
    SETID_BATTERY_FL_HWDOC_COUNT,       // 126
    SETID_BATTERY_FL_HWCOC_COUNT,       // 127
    SETID_BATTERY_FL_HWBCD_COUNT,       // 128
    SETID_BATTERY_FL_HWSCP_COUNT,       // 129
    SETID_BATTERY_FL_PFCBF_COUNT,       // 130
    SETID_BATTERY_FL_PFOVP_COUNT,       // 131
    SETID_BATTERY_FL_PFCOC_COUNT,       // 132
    SETID_BATTERY_FL_PFFETF_COUNT,      // 133
    SETID_BATTERY_FL_PFUVP,             // 134
    SETID_BATTERY_FL_MAX_CELL_VOLT,     // 135
    SETID_BATTERY_FL_MIN_CELL_VOLT,     // 136
    SETID_BATTERY_FL_MAX_CHRG_TEMP,     // 137
    SETID_BATTERY_FL_MIN_CHRG_TEMP,     // 138
    SETID_BATTERY_FL_MAX_DISCHRG_TEMP,  // 139
    SETID_BATTERY_FL_MIN_DISCHRG_TEMP,  // 140
    SETID_BATTERY_FL_MAX_CHRG_CURR,     // 141
    SETID_BATTERY_FL_MAX_DISCHRG_CURR,  // 142
    SETID_BATTERY_FL_BATTERY_STATUS,    // 143
    SETID_BATTERY_FL_AVG_CURRENT,       // 144
    SETID_BATTERY_FL_TEMP1,             // 145
    SETID_BATTERY_FL_TEMP2,             // 146
    SETID_BATTERY_FL_MOS_TEMP1,         // 147
    SETID_BATTERY_FL_CELL_VOLT1,        // 148
    SETID_BATTERY_FL_CELL_VOLT2,        // 149
    
    //For Debug Tool
    SETID_TEMP_PRINT_PERIOD,
    SETID_BATT_PRINT_PERIOD,
    
    SETID_MAX                           // 150
}eSettingId;

#endif
