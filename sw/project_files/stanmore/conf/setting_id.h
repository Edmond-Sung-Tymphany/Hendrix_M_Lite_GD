#ifndef SETTING_ID_H
#define SETTING_ID_H


typedef enum
{
    SETID_START = 0,
    SETID_VOLUME = SETID_START,
    SETID_BASS,
    SETID_TREBLE,
    SETID_ADC_VOLUME,
    SETID_ADC_BASS,
    SETID_ADC_TREBLE,
    SETID_CHANNEL,
    SETID_BT_STATUS,
    SETID_MUSIC_DET,
    SETID_IS_AUXIN_PLUG_IN,
    SETID_VOLUME_ADC_MAX,    
    SETID_VOLUME_ADC_MIN,
    SETID_BASS_ADC_MAX,
    SETID_BASS_ADC_MIN,
    SETID_TREBLE_ADC_MAX,
    SETID_TREBLE_ADC_MIN,
    SETID_MAX
}eSettingId;

#endif