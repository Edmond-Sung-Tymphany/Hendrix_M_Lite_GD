/**
 * @file        MainApp_priv.h
 * @brief       Main application for iBT150
 * @author      Christopher 
 * @date        2014-04-24
 * @copyright   Tymphany Ltd.
 */
 

#ifndef MAIN_APP_PRIVATE_H
#define MAIN_APP_PRIVATE_H
 
#ifdef __cplusplus
 extern "C" {
#endif

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
        
    // the following is for mainapp signal only
    MAINAPP_PRODUCT_TEST_CTL_SIG = 200,
    MAINAPP_VOLUME_CTL_SIG = 201,
    MAINAPP_GET_VER_SIG = 202,
    MAINAPP_CH_MUTE_SIG = 203,
    MAINAPP_LED_CTL_SIG = 204,
    MAINAPP_SOURCE_CTL_SIG = 205,
    MAINAPP_BYPASS_CTL_SIG = 206,
    MAINAPP_DFU_REQ_SIG = 207,
    MAINAPP_A2B_MASTER_CTL_SIG = 208,
    MAINAPP_NTC_INFO_CTL_SIG = 209,
    MAINAPP_MUTE_AMP_CTL_SIG = 210,
    MAINAPP_SYSTEM_RESET_CTL_SIG = 211,
    MAINAPP_SYSTEM_STATUS_CTL_SIG = 212,
    MAINAPP_TEST_TONE_CTL_SIG = 213,
	MAINAPP_WRITE_DSP_PARAM_CTL_SIG = 214,
	MAINAPP_WRITE_TYPE_NO_CTL_SIG = 215,
	MAINAPP_WRITE_ITEM_NO_CTL_SIG = 216,
	MAINAPP_WRITE_SERIAL_NO_CTL_SIG = 217,
	MAINAPP_GET_SERIAL_NO_CTL_SIG = 218,
    MAINAPP_MAX_CTL_SIG
};

#define MAINAPP_SOURCE_AUX      AUDIO_CHANNEL_AUXIN
#define MAINAPP_SOURCE_OPTICAL  AUDIO_CHANNEL_OPT
#define MAINAPP_SOURCE_A2B      AUDIO_CHANNEL_I2S_2
#define MAINAPP_SOURCE_CH_MAX       3
#define MAINAPP_SOURCE_DEFAULT      MAINAPP_SOURCE_OPTICAL

#define MAINAPP_SN_MAX_TYPE_NO      9999
#define MAINAPP_SN_MAX_ITEM_NO      9999999
#define MAINAPP_SN_MAX_SERIAL_NO    99999999

// add signal control
REQ_EVT(MainAppCtlEvt)
    uint32_t value;
END_REQ_EVT(MainAppCtlEvt)

#ifdef __cplusplus
}
#endif
 
#endif /* MAIN_APP_PRIVATE_H */
