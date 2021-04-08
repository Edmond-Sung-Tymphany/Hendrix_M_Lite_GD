/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  Main Application
                  -------------------------

                  SW Module Document

@file        MainApp.c
@brief       Main application for SVS14_Ultra
@author      Christopher Alexander,Bob.Xu
@date        2014-04-24
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2016-06-30     Viking Wang
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "./MainApp_priv.h"
#include "stm32f0xx.h"
#include "bsp.h"
#include "projBsp.h"
#include "SettingSrv.h"
#include "trace.h"
#include "devicetypes.h"
#include "AudioSrv.h"
#include "AudioDrv.h"
#include "keySrv.h"
#include "DebugSSrv.h"
#ifdef HAS_SYSTEM_CONTROL
#include "SystemSrv.h"
#include "SystemDrv.h"
#endif
#ifdef HAS_GPIO_LED
#include "GpioLed.h"
#endif
#ifdef HAS_ADAU1761_DSP
#include "Adau1761_Drv.h"
#endif
#ifdef HAS_BLUETOOTH
#include "BluetoothDrv.h"
#endif

#define MAIN_APP_TIMEOUT_IN_MS          20

#define COMB_KEY_FACTORY_RESET      COMB_KEY_ID_0

#ifdef Q_SPY
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#else
#define CAST_ME cMainApp * MainApp = (cMainApp *) me;
#endif

#define POWER_KEY_DEBOUNCE_CNT  25
static uint8_t power_switch_cnt=0;
#ifdef POWER_OFF_TO_ON_DEBOUNCE
static int16_t power_on_debounce_timeout=0;
static int32_t power_down_timeout=0;    // unit : ms
#endif

#ifdef BT_LINK_LOST_WORKAROUND
#define BT_IGNORE_STATUS_TIMEOUT    700     // unit : ms
static int16_t bt_ignore_status_timeout = 0;
#endif

#define IGNORE_PAIRING_WHEN_RECONNECT_TIMEOUT   2000
static int16_t ignore_pairing_key_timeout=0;
#ifdef AUDIO_CUE_FIX_VOLUME
static int16_t audio_cue_playing_timeout = 0;
typedef enum tagAudioCueID
{
    AUDIO_CUE_ID_PAIRING,
    AUDIO_CUE_ID_CONNECTED,
    AUDIO_CUE_ID_MAX
}AudioCueId_t;
const static int16_t audio_cue_timeout[AUDIO_CUE_ID_MAX]=
{
    3000,
    3000,
};
#endif

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,
    // the following is for mainapp signal only
    MAINAPP_VOLUME_CTL_SIG = 201,
    MAINAPP_TREBLE_CTL_SIG = 202,
    MAINAPP_BASS_CTL_SIG = 203,
    MAINAPP_LED_CTL_SIG = 204,
    MAINAPP_SOURCE_CTL_SIG = 205,
    MAINAPP_BYPASS_CTL_SIG = 206,
    MAINAPP_MUTE_CTL_SIG = 207,
    MAINAPP_MAX_CTL_SIG
};

static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[6];
/* Internal event queue - Size as needed */
static QEvt const *MainEvtQue[10];

#define CAST_ME cMainApp * MainApp = (cMainApp *) me;

static QState MainApp_Initial(cMainApp * const me, QEvt const *const e);

/* Active state  - super state for "normal" behaviour */
static QState MainApp_Active(cMainApp * const me, QEvt const * const e);

static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e);

static QState MainApp_DeActive(cMainApp * const me, QEvt const * const e);

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId);

static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e);

static void MainApp_SourceLedDisplay(void);

static void powerKey_handler(cMainApp * const me, eKeyEvent keyEvent);

static bool firmware_version_show=TRUE;
static uint8_t gpio_key_pressed=GPIO_KEY_RELEASED;
static uint8_t gpio_key_ignore_cnt = 0;
static uint8_t cur_source_id=0;
static uint32_t bt_pre_status=BT_MAX_STA;
static eAudioChannel current_source=AUDIO_CHANNEL_INVALID;
const static eAudioChannel source_list[MAINAPP_SOURCE_CH_MAX+1] =
{
    MAINAPP_SOURCE_AUX,
    MAINAPP_SOURCE_RCA,
    MAINAPP_SOURCE_BT,
    MAINAPP_SOURCE_AUX  // this is default source
};

#ifdef AUDIO_CUE_FIX_VOLUME
static void MainApp_AudioCueTimeout(cMainApp * const me, int16_t timeout)
{
    uint32_t cur_vol;

    if( MAINAPP_SOURCE_BT != current_source )   // do nothing when the active source is not BT
        return ;

#ifdef MUTE_AMP_WHEN_VOL_IS_0
    cur_vol = *(uint32_t*)Setting_Get(SETID_VOLUME);
    if( cur_vol ==0 )
    {   // demute the amplifier
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
    }
#endif
    cur_vol = AUDIO_CUE_FIX_VOLUME;
    AudioSrv_SetVolume(cur_vol);
    Setting_Set(SETID_VOLUME, &cur_vol);

    audio_cue_playing_timeout = timeout;
}
#endif

static void MainApp_ShowVersion(void)
{
    ALWAYS_printf("\n\r-------------------------------------------------------\n\r");
    ALWAYS_printf("\n\r Fender Watermelon MCU:V%s, DSP:V%s.\n\r", PRODUCT_VERSION_MCU, PRODUCT_DSP_VERSION);
    ALWAYS_printf("\n\r-------------------------------------------------------\n\r");
}

static void MainApp_Version2TPMonitor(void)
{
    char ver_msg[24];
#ifdef HAS_HW_VERSION_TAG
    char hw_msg[][8]={"Unknown", "ES1", "ES2", "ES3", "EVT1", "EVT2", \
                      "DVT1", "DVT2", "PVT", "MP1", "MP2"};
    HwVersion_t hw_ver;
    hw_ver = SystemDrv_GetHWversion();
    sprintf(ver_msg, "[HW Ver:%s].", hw_msg[hw_ver]);
    DebugSSrv_Printf(ver_msg);
#endif
    sprintf(ver_msg, "[MCU:V%s].[DSP:V%s]", PRODUCT_VERSION_MCU, PRODUCT_DSP_VERSION);
    DebugSSrv_Printf(ver_msg);
}

static void MainApp_VolumeCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;
    if( value < MAX_VOLUME_STEPS )
    {
        AudioSrv_SetVolume(value);
        Setting_Set(SETID_VOLUME, &value);
        sprintf(vol_msg, "Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Volume=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

static void MainApp_TrebleCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;

    if( value < MAX_TREBLE_STEPS )
    {
        AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, value, 0);
        Setting_Set(SETID_TREBLE, &value);
        sprintf(vol_msg, "Treble=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Treble=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

static void MainApp_BassCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value;
    char vol_msg[24];
    value = p_ctl_evt->value;

    if( value < MAX_BASS_STEPS )
    {
        AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, value, 0);
        Setting_Set(SETID_BASS, &value);
        sprintf(vol_msg, "Bass=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
    else
    {
        sprintf(vol_msg, "ERROR:Bass=%d", value);
        DebugSSrv_Printf(vol_msg);
    }
}

/* LED define for TP monitor, 
 * the jason file should edit according the following define 
 */
/*
#define LED_AUX_ON        1
#define LED_POWER_ON      2
#define LED_RCA_ON        4
#define LED_BT_ON         8
*/
static void MainApp_LedCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint16_t ctl_mode, ctl_value;
    uint32_t value, led_mask;
    GLED_ID_t id;

    value = p_ctl_evt->value;
    ctl_mode = (uint16_t)(value & 0xffff);
    ctl_value = (uint16_t)((value>>16) & 0xffff);

    // if ctl_mode > 255, we use ctr_value to control
    if( ctl_mode & 0xff00 )
    {
        led_mask = (uint32_t)ctl_value;
    }
    else
    {
        led_mask = (uint32_t)ctl_mode;
    }

    for(id=GLED_ID_START; id<GLED_ID_END; id++)
    {
        if( led_mask & 0x01 )
            GLED_SetupMode(id, GLED_MODE_ON, 0);
        else
            GLED_SetupMode(id, GLED_MODE_OFF, 0);
        led_mask >>= 1;
    }
}

static void MainApp_SourceCtlSig(cMainApp * const me, MainAppCtlEvt *p_ctl_evt)
{
    uint32_t value, i;
    value = p_ctl_evt->value;

    switch( value )
    {
    case MAINAPP_SOURCE_AUX:
    case MAINAPP_SOURCE_BT:
    case MAINAPP_SOURCE_RCA:
        for(i=0; i<MAINAPP_SOURCE_CH_MAX; i++)
        {
            if( value == source_list[i] )
            {
                cur_source_id = i;
                break;
            }
        }
        current_source = source_list[cur_source_id];
        Setting_Set(SETID_AUDIO_SOURCE, (void *)&current_source);
        AudioSrv_SetChannel((QActive *)me, current_source);
        MainApp_SourceLedDisplay();
        break;
    default:    // error
        DebugSSrv_Printf("ERROR Source.");
        break;
    }
}

static void MainApp_BypassCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    bool enable;

    if( p_ctl_evt->value )
    {
        enable = TRUE;
        DebugSSrv_Printf("Bypass ENABLE");
    }
    else
    {
        enable = FALSE;
        DebugSSrv_Printf("Bypass Disable");
    }
    
    AudioSrv_SetAudio(DSP_PASSTHROUGH_SETT_ID, enable, 0, 0);
}

static void MainApp_MuteCtlSig(MainAppCtlEvt *p_ctl_evt)
{
    uint32_t unmute_ch;

    unmute_ch = p_ctl_evt->value;

#if 0
    char mute_msg[28];
    sprintf(mute_msg, "value = %d.", unmute_ch);
    DebugSSrv_Printf(mute_msg);
#endif

    // woofer-left channel
    if( unmute_ch & (uint32_t)0x01 )
    {
        Adau1761Drv_Mute_WL(TRUE);
    }
    else
    {
        Adau1761Drv_Mute_WL(FALSE);
    }
    // woofer-right channel
    if( unmute_ch & (uint32_t)0x02 )
    {
        Adau1761Drv_Mute_WR(TRUE);
    }
    else
    {
        Adau1761Drv_Mute_WR(FALSE);
    }
    // tweeter-left channel
    if( unmute_ch & (uint32_t)0x04 )
    {
        Adau1761Drv_Mute_TL(TRUE);
    }
    else
    {
        Adau1761Drv_Mute_TL(FALSE);
    }
    // tweeter-right channel
    if( unmute_ch & (uint32_t)0x08 )
    {
        Adau1761Drv_Mute_TR(TRUE);
    }
    else
    {
        Adau1761Drv_Mute_TR(FALSE);
    }
}

static eAudioChannel MainApp_GetSource(void)
{
    eAudioChannel audio_channel;
    uint32_t i;

    audio_channel = *(eAudioChannel*)Setting_Get(SETID_AUDIO_SOURCE);

    for(i=0; i<MAINAPP_SOURCE_CH_MAX; i++)
    {
        if( audio_channel == source_list[i] )
            break;
    }

    if( i == MAINAPP_SOURCE_CH_MAX )
    {   // source error, set to default.
        audio_channel = source_list[i];
        Setting_Set(SETID_AUDIO_SOURCE, (void *)&audio_channel);
        cur_source_id = 0;  // need to refer to source_list[] table
    }
    else
    {
        cur_source_id = i;
    }

    return audio_channel;
}

static void MainApp_BTLedDisplay(eBtStatus btStatus)
{
    switch( btStatus )
    {
    case BT_CONNECTABLE_STA:
        GLED_SetupModeWithCheck(GLED_ID_BT, GLED_MODE_DOUBLE_FLASH, GPIO_LED_ALWAYS_FLASH);
        break;
    case BT_DISCOVERABLE_STA:
        GLED_SetupModeWithCheck(GLED_ID_BT, GLED_MODE_FLASH, GPIO_LED_ALWAYS_FLASH);
        break;
    case BT_CONNECTED_STA:
    case BT_STREAMING_A2DP_STA:
        GLED_SetupModeWithCheck(GLED_ID_BT, GLED_MODE_ON, 0);
        break;
    default :
        GLED_SetupMode(GLED_ID_BT, GLED_MODE_DOUBLE_FLASH, GPIO_LED_ALWAYS_FLASH);
        break;
    }
}

static void MainApp_SourceLedDisplay(void)
{
    eBtStatus btStatus;
    
    // turn off LED first
    GLED_SetupMode(GLED_ID_RCA, GLED_MODE_OFF, 0);
    GLED_SetupMode(GLED_ID_AUX, GLED_MODE_OFF, 0);
    GLED_SetupMode(GLED_ID_BT, GLED_MODE_OFF, 0);
    
    switch( current_source )
    {
    case MAINAPP_SOURCE_AUX:
        GLED_SetupMode(GLED_ID_AUX, GLED_MODE_ON, 0);
        TP_PRINTF("\n\rSource->AUX.\n\r");
        DebugSSrv_Printf("Source->AUX.");
        break;
    case MAINAPP_SOURCE_RCA:
        GLED_SetupMode(GLED_ID_RCA, GLED_MODE_ON, 0);
        TP_PRINTF("\n\rSource->RCA.\n\r");
        DebugSSrv_Printf("Source->RCA.");
        break;
    case MAINAPP_SOURCE_BT:
        btStatus = *(eBtStatus*)Setting_Get(SETID_BT_STATUS);
        MainApp_BTLedDisplay(btStatus);
        TP_PRINTF("\n\rSource->BT.\n\r");
        DebugSSrv_Printf("Source->BT.");
        break;
    default:
        ASSERT(0);
        break;
    }
}

static void MainApp_AllLedOff(void)
{
    GLED_SetupOnOffMode(GLED_ID_POWER, GLED_MODE_OFF);
    GLED_SetupOnOffMode(GLED_ID_RCA, GLED_MODE_OFF);
    GLED_SetupOnOffMode(GLED_ID_AUX, GLED_MODE_OFF);
    GLED_SetupOnOffMode(GLED_ID_BT, GLED_MODE_OFF);
}

static void MainApp_DisconnectBT(void)
{
    eBtStatus bt_status;
    bt_status = *(eBtStatus *)Setting_Get(SETID_BT_STATUS);
    if( (bt_status == BT_CONNECTED_STA) || (bt_status == BT_STREAMING_A2DP_STA) )
    {   // disconnect the device
        AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);
        BluetoothDrv_DisconnectBT();
        BSP_BlockingDelayMs(1500);
    }
}

static void MainApp_PowerupInit(cMainApp * const me)
{
    current_source = MainApp_GetSource();
    AudioSrv_SetChannel((QActive *)me, current_source);
    GLED_SetupMode(GLED_ID_POWER, GLED_MODE_ON, 0);
    MainApp_SourceLedDisplay();

    // if active source is BT, manual reconnect the device
    if( MAINAPP_SOURCE_BT == current_source )
    {
        uint32_t pdl_empty;
        pdl_empty = *(uint32_t *)Setting_Get(SETID_PDL_EMPTY);
        if( ! pdl_empty )
        {
            BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
            ignore_pairing_key_timeout = IGNORE_PAIRING_WHEN_RECONNECT_TIMEOUT;
        }
    }
}

#ifdef WAKE_UP_FROM_AUTO_STANDBY
#define WAKE_UP_KEY_CNT     3
static uint32_t wake_up_key_cnt=0;
// PA0:BT, PA1:Source
static void MainApp_WakeUpKeyScan(void)
{
    if( SystemDrv_IsAutoStandby() )
    {
        GPIO_InitTypeDef  GPIO_InitStructure;

        // init the GPIO as digital input
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0 )
        {   // bt key pressed
            wake_up_key_cnt ++;
            if( (wake_up_key_cnt >= WAKE_UP_KEY_CNT) && (current_source != MAINAPP_SOURCE_BT) )
            {
                current_source = MAINAPP_SOURCE_BT;
                Setting_Set(SETID_AUDIO_SOURCE, &current_source);
                SettingSrv_BookkeepingEx();
            }
        }
        else if( GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == 0 )
        {   // input key pressed
            wake_up_key_cnt ++;
        }
        else
        {
            wake_up_key_cnt = 0;
        }

        if( wake_up_key_cnt >= WAKE_UP_KEY_CNT )
        {
            BSP_SoftReboot();
        }
    }
}
#endif

static void MainApp_ProcBtKey(cMainApp * const me, QEvt const * const e)
{
    uint32_t i;
    eBtStatus bt_status;
    
    AudioSrv_SetAudio(AUDIO_RESET_LINEIN_JACK_AND_MUSIC_STA_SETT_ID, TRUE, 0, 0);

    if( ignore_pairing_key_timeout > 0 )
        return ;

    ignore_pairing_key_timeout = 2000;

    bt_status = *(eBtStatus *)Setting_Get(SETID_BT_STATUS);

    if( (bt_status == BT_STREAMING_A2DP_STA) || (bt_status == BT_CONNECTED_STA) )
    {
        BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
    }

    if( current_source != MAINAPP_SOURCE_BT )
    {
        // jump to BT source first
        current_source = MAINAPP_SOURCE_BT;
        for(i=0; i<MAINAPP_SOURCE_CH_MAX; i++)
        {
            if( current_source == source_list[i] )
            {
                cur_source_id = i;
                break;
            }
        }
        Setting_Set(SETID_AUDIO_SOURCE, (void *)&current_source);
        AudioSrv_SetChannel((QActive *)me, current_source);
        MainApp_SourceLedDisplay();    
    }
    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);
}

static void MainApp_ProcInputKey(cMainApp * const me)
{
    eBtStatus bt_status;

    AudioSrv_SetAudio(AUDIO_RESET_LINEIN_JACK_AND_MUSIC_STA_SETT_ID, TRUE, 0, 0);

#ifdef BT_AVRCP_ENABLE
    // pause the BT if necessary
    if( MAINAPP_SOURCE_BT == current_source )
    {
        bt_status = *(eBtStatus *)Setting_Get(SETID_BT_STATUS);
        if( bt_status == BT_STREAMING_A2DP_STA )
        {
            BluetoothSrv_SendBtCmd((QActive*)me, BT_AVRCP_PAUSE_CMD);
            BRINGUP_printf("\n\r pause BT \n\r");
        }
    }
#endif

    cur_source_id ++;
    if( cur_source_id >= MAINAPP_SOURCE_CH_MAX )
        cur_source_id = 0;
    current_source = source_list[cur_source_id];
    Setting_Set(SETID_AUDIO_SOURCE, (void *)&current_source);
    AudioSrv_SetChannel((QActive *)me, current_source);
    MainApp_SourceLedDisplay();
    
    if( MAINAPP_SOURCE_BT == current_source )
    {
        uint32_t pdl_empty;
        bt_status = *(eBtStatus *)Setting_Get(SETID_BT_STATUS);
        if( (bt_status < BT_CONNECTED_STA) || (bt_status > BT_DUMMY_STA_START) )  // not connected
        {
            pdl_empty = *(uint32_t *)Setting_Get(SETID_PDL_EMPTY);
            if( pdl_empty )
            {
//               BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_PAIRING_CMD);    // do nothing according to the UI.
            }
            else
            {
                if( bt_status == BT_DISCOVERABLE_STA )  // in pairing status.
                {
                }
                else
                {
                    BluetoothSrv_SendBtCmd((QActive*)me, BT_ENTER_CONNECTABLE_CMD);
                    ignore_pairing_key_timeout = IGNORE_PAIRING_WHEN_RECONNECT_TIMEOUT;
                }
            }
        }
    }
}

static void MainApp_ProcFactoryReset(cMainApp * const me)
{
    ALWAYS_printf("\n\rFactory reset.\n\r");
    
#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
    GLED_ID_t led_id;
    uint32_t pdl_empty;
    // mute the amplifier first
    AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_SOFT_MUTE, TRUE);
    // audio source switch to AUX
    if( current_source != source_list[MAINAPP_SOURCE_CH_MAX] )
    {
        current_source = source_list[MAINAPP_SOURCE_CH_MAX];
        Setting_Set(SETID_AUDIO_SOURCE, (void *)&current_source);
    }
    pdl_empty = 1;
    Setting_Set(SETID_PDL_EMPTY, &pdl_empty);
    SettingSrv_BookkeepingEx();
    // flash the LED
    for(led_id=GLED_ID_START; led_id<GLED_ID_END; led_id++)
        GLED_SetupMode_2(led_id, GLED_MODE_FLASH, 1, 0);
#else
    uint32_t i;
    // audio source switch to AUX
    if( current_source != source_list[MAINAPP_SOURCE_CH_MAX] )
    {
        current_source = source_list[MAINAPP_SOURCE_CH_MAX];
        for(i=0; i<MAINAPP_SOURCE_CH_MAX; i++)
        {
            if( current_source == source_list[i] )
            {
                cur_source_id = i;
                break;
            }
        }
        Setting_Set(SETID_AUDIO_SOURCE, (void *)&current_source);
        AudioSrv_SetChannel((QActive *)me, current_source);
        MainApp_SourceLedDisplay();    
    }
#endif        

    // reset bt PDL
    BluetoothSrv_SendBtCmd((QActive*)me, BT_RESET_PAIR_LIST_CMD);
}

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void MainApp_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(MainApp);

     /* start up the object and let it run. including the timer*/
    Application_Ctor((cApplication*)me, Q_STATE_CAST(&MainApp_Initial), MAINAPP_TIMEOUT_SIG,
                            MainEvtQue, Q_DIM(MainEvtQue), MAIN_APP_ID);

    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
        /* Subscribe */
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
#endif
#ifdef HAS_BLUETOOTH
    QActive_subscribe((QActive*) me, BT_STATE_SIG);
    QActive_subscribe((QActive*) me, BT_RESP_SIG);
#endif
}

void MainApp_ShutDown(cPersistantObj *me)
{
    /* Clean memory and shut-down. Called by the controller */
    Application_Xtor((cApplication*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState MainApp_Initial(cMainApp * const me, QEvt const *const e)
{
    // set a dummy volume in initial status
    uint32_t dummy_volume;
    dummy_volume = MAX_VOLUME_STEPS;
    Setting_Set(SETID_VOLUME, &dummy_volume);
    SystemDrv_SetSystemStatus(SYSTEM_STATUS_STANDBY);
    return Q_TRAN(&MainApp_DeActive);
}

static QState MainApp_PoweringUp(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
#ifdef POWER_OFF_TO_ON_DEBOUNCE
            power_on_debounce_timeout = 0;
#endif
            gpio_key_pressed = GPIO_KEY_RELEASED;
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_POWERING_UP);
            GLED_SetupMode(GLED_ID_POWER, GLED_MODE_ON, 0);
#ifdef QUICKLY_POWER_DOWN
            SystemDrv_SetAmpMuteEnable(FALSE);
#endif
            MainApp_SwitchMode(MainApp, POWERING_UP_MODE);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
#ifdef QUICKLY_POWER_DOWN
            if( ! SystemDrv_IsPowerSwitchOn() )
            {
                GLED_SetupMode(GLED_ID_POWER, GLED_MODE_OFF, 0);
                if( SystemDrv_GetAmpMuteEnable() )
                {
                    AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);   // mute the ampilfier to avoid the audio cue
                }
            }
#endif                
            return Q_HANDLED();
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            MainApp_SwitchMode(MainApp, NORMAL_MODE);
            return Q_TRAN(&MainApp_Active);
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/* Active state  - super state for "normal" behaviour */
static QState MainApp_Active(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
            power_switch_cnt = 0;
            gpio_key_ignore_cnt = 0;
            gpio_key_pressed = GPIO_KEY_RELEASED;
            power_down_timeout = 0;
#ifdef BT_LINK_LOST_WORKAROUND
            bt_ignore_status_timeout = 0;
#endif
#ifdef AUDIO_CUE_FIX_VOLUME
            audio_cue_playing_timeout = 0;
            ignore_pairing_key_timeout = 0;
#endif
#ifdef QUICKLY_POWER_DOWN
            if( SystemDrv_IsPowerSwitchOn() )
            {   // normally case 
                SystemSrv_Set(SYSTEM_SET_ID_AMP_SD, FALSE, 0);
                AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_SOFT_MUTE, FALSE);
                MainApp_PowerupInit(me);
            }
            else
            {   // force to power down
                power_down_timeout = 10;
            }
#else
            SystemSrv_Set(SYSTEM_SET_ID_AMP_SD, FALSE, 0);
            AudioSrv_SendMuteReq((QActive*)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            MainApp_PowerupInit(me);
#endif            
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            while (QActive_recall((QActive*)me, &deferredReqQue)) 
                {} // check if we have events(key_state_sig) in deferred queue
            break;
        case MAINAPP_TIMEOUT_SIG:
#ifdef HAS_POWER_SWITCH_KEY
            if( power_down_timeout > 0 )
            {
                power_down_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if( power_down_timeout == 100 )
                {
                    AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);   // soft mute the ampilfier to avoid the pop noise
                }
                if( power_down_timeout <= 0 )
                {
                    power_down_timeout = 0;
#ifdef POWER_OFF_TO_ON_DEBOUNCE
                    power_on_debounce_timeout = 2800;
#endif
                    MainApp_SwitchMode(MainApp, SLEEP_MODE);
                    return Q_TRAN(&MainApp_DeActive);
                }
            }
            else
            {
                if( ! SystemDrv_IsPowerSwitchOn() )
                {
                    power_switch_cnt ++;
                    if( power_switch_cnt > POWER_KEY_DEBOUNCE_CNT )
                    {
#if 1
                        uint32_t cur_vol;
                        cur_vol = *(uint32_t*)Setting_Get(SETID_VOLUME);
                        if( cur_vol > 20 )
                        {
                            AudioDrv_SetDspVol(20);
                            BSP_BlockingDelayMs(20);
                        }
                        if( cur_vol > 10 )
                        {
                            AudioDrv_SetDspVol(10);
                            BSP_BlockingDelayMs(20);
                        }
                        AudioDrv_SetDspVol(1);
//                        BSP_BlockingDelayMs(20);
#else
                        AudioDrv_SetDspVol(1);
#endif
//                        AudioDrv_SetChannel(MAINAPP_SOURCE_BT);
//                        AudioDrv_Mute(AUDIO_AMP_SOFT_MUTE, TRUE);   // soft mute the ampilfier to avoid the pop noise
                        power_down_timeout = 200;   // unit : ms
                        // turn off LED immediately
                        GLED_ID_t led_id;
                        for(led_id=GLED_ID_START; led_id<GLED_ID_END; led_id++)
                            GLED_SetupOnOffMode(led_id, GLED_MODE_OFF);
                        GLED_PeriodicRefresh();
                    }
                }
                else
                {
                    power_switch_cnt = 0;
                }
            }
#endif      
#ifdef HAS_AUTO_STANDBY
            if( SYSTEM_STATUS_AUTO_STANDBY == SystemDrv_GetSystemStatus() )
            {
                SystemDrv_SetAutoStandby(TRUE);
                MainApp_SwitchMode(MainApp, SLEEP_MODE);
                return Q_TRAN(&MainApp_DeActive);
            }
#endif
#ifdef BT_LINK_LOST_WORKAROUND
            if( bt_ignore_status_timeout > 0 )
            {
                bt_ignore_status_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if( bt_ignore_status_timeout < 0 )
                {
                    bt_ignore_status_timeout = 0;
                }
            }
#endif
#ifdef AUDIO_CUE_FIX_VOLUME
            if( audio_cue_playing_timeout > 0 )
            {
                audio_cue_playing_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if( audio_cue_playing_timeout <= 0 )
                {   
                    audio_cue_playing_timeout = 0;
                    gpio_key_pressed = 0;
#ifdef HAS_LINEAR_ADC_KNOB_KEY
                    // clear the volume index to force the volume refresh
                    KeySrv_ResetKnobKeyIndex();
#else
    #error "Need resume the volume in your case..."
#endif
                }
            }
#endif
            if( ignore_pairing_key_timeout > 0 )
            {
                ignore_pairing_key_timeout -= MAIN_APP_TIMEOUT_IN_MS;
            }
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            break;
/*************** tp monitor control signal start here ******************/
        case DISPLAY_DEBUG_REQ_SIG:
            MainApp_Version2TPMonitor();
            break;
        case MAINAPP_VOLUME_CTL_SIG:
            MainApp_VolumeCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_TREBLE_CTL_SIG:
            MainApp_TrebleCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_BASS_CTL_SIG:
            MainApp_BassCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_LED_CTL_SIG:
            MainApp_LedCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_SOURCE_CTL_SIG:
            MainApp_SourceCtlSig(me, (MainAppCtlEvt *)e);
            break;
        case MAINAPP_BYPASS_CTL_SIG:
            MainApp_BypassCtlSig((MainAppCtlEvt *)e);
            break;
        case MAINAPP_MUTE_CTL_SIG:
            MainApp_MuteCtlSig((MainAppCtlEvt *)e);
            break;
/*************** tp monitor control signal end here ******************/
        case SYSTEM_MODE_RESP_SIG:
        {
            if( firmware_version_show )
            {
                firmware_version_show = FALSE;
                MainApp_ShowVersion();
            }
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_WORKING);
            return Q_HANDLED();
        }
        case KEY_STATE_SIG:
            MainApp_ParseKeyEvent(me, e);
            return Q_HANDLED();
#ifdef HAS_BLUETOOTH
        case BT_STATE_SIG:
        {
            BtStatusEvt *pEvt = (BtStatusEvt *)e;
            if( pEvt->isBtStatus )  // this is BT status
            {
                if( current_source == MAINAPP_SOURCE_BT )
                    MainApp_BTLedDisplay(pEvt->btStatus);

                if( pEvt->btStatus == BT_CONNECTED_STA )    // connected...
                {   // check & refresh the PDL empty status
#ifdef BT_LINK_LOST_WORKAROUND
                    if( bt_ignore_status_timeout <= 0 )
#endif
                    {
                        uint32_t pdl_empty;
                        pdl_empty = *(uint32_t *)Setting_Get(SETID_PDL_EMPTY);
                        if( pdl_empty )
                        {
                            pdl_empty = 0;
                            Setting_Set(SETID_PDL_EMPTY, &pdl_empty);
                        }
#ifdef AUDIO_CUE_FIX_VOLUME
                        if( (bt_pre_status != BT_CONNECTED_STA) && (bt_pre_status != BT_STREAMING_A2DP_STA) )
                        {
                            MainApp_AudioCueTimeout(me, audio_cue_timeout[AUDIO_CUE_ID_CONNECTED]);
                        }
                        else
                        {
                            if( audio_cue_playing_timeout )
                            {
                                audio_cue_playing_timeout = audio_cue_timeout[AUDIO_CUE_ID_CONNECTED];
                            }
                        }
#endif
                    }
                }
                else if( pEvt->btStatus == BT_DISCOVERABLE_STA )    // pairing...
                {
#ifdef AUDIO_CUE_FIX_VOLUME
                    if( bt_pre_status != BT_DISCOVERABLE_STA )
                    {
                        MainApp_AudioCueTimeout(me, audio_cue_timeout[AUDIO_CUE_ID_PAIRING]);
                    }
                    else
                    {
                        if( audio_cue_playing_timeout )
                        {
                            audio_cue_playing_timeout = audio_cue_timeout[AUDIO_CUE_ID_PAIRING];
                        }
                    }
#endif
                }
                else
                {
                }

                bt_pre_status = pEvt->btStatus;
            }
            else    // this is BT event
            {
                if( pEvt->btIndEvt == BT_LINK_LOSS_EVT )
                {
#ifdef BT_LINK_LOST_WORKAROUND
                    bt_ignore_status_timeout = BT_IGNORE_STATUS_TIMEOUT;
#endif
                }
            }
            return Q_HANDLED();
        }
#endif
        case Q_EXIT_SIG:
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static QState MainApp_DeActive(cMainApp * const me, QEvt const * const e)
{
    CAST_ME;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            power_switch_cnt = 0;
            gpio_key_pressed = GPIO_KEY_RELEASED;
#ifdef WAKE_UP_FROM_AUTO_STANDBY
            wake_up_key_cnt = 0;
#endif
            bt_pre_status = BT_MAX_STA;
            MainApp_AllLedOff();
            MainApp_DisconnectBT();
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
            return Q_HANDLED();
        }
        case MAINAPP_TIMEOUT_SIG:
        {
            PersistantObj_RefreshTick((cPersistantObj*)MainApp, MAIN_APP_TIMEOUT_IN_MS);
#ifdef POWER_OFF_TO_ON_DEBOUNCE
            if( power_on_debounce_timeout )
            {
                power_on_debounce_timeout -= MAIN_APP_TIMEOUT_IN_MS;
                if( power_on_debounce_timeout < 0 )
                {
                    power_on_debounce_timeout = 0;
                }
                return Q_HANDLED();
            }
#endif
#ifdef WAKE_UP_FROM_AUTO_STANDBY
            MainApp_WakeUpKeyScan();
#endif
#ifdef HAS_POWER_SWITCH_KEY
#ifdef HAS_AUTO_STANDBY
            if( SystemDrv_IsPowerSwitchOn() )
            {
                if( ! SystemDrv_IsAutoStandby() )
                {
                    power_switch_cnt ++;
                    if( power_switch_cnt > POWER_KEY_DEBOUNCE_CNT )                    
                    {
                        return Q_TRAN(&MainApp_PoweringUp);
                    }
                }
            }
            else
            {
                power_switch_cnt = 0;
                SystemDrv_SetAutoStandby(FALSE);
            }
            return Q_HANDLED();
#else
            if( SystemDrv_IsPowerSwitchOn() )
                return Q_TRAN(&MainApp_PoweringUp);
            else
                return Q_HANDLED();
#endif
#else
            return Q_TRAN(&MainApp_PoweringUp);
#endif            
        }
        case SYSTEM_MODE_RESP_SIG:
        {
            SystemDrv_SetSystemStatus(SYSTEM_STATUS_STANDBY);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            QTimeEvt_disarm(TIME_EVT_OF(me));
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}


static void MainApp_ParseKeyEvent(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    uint32_t cur_vol;
    char vol_msg[24];

#ifdef AUDIO_CUE_FIX_VOLUME
    if( audio_cue_playing_timeout > 0 )
    {
        // ignore any key when audio playing...
        return ;
    }
#endif

    switch(evt->keyId)
    {
        case VOLUME_KNOB_BASE_KEY_ID:
        {
            uint32_t prev_vol;
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_VOLUME_STEPS - 1 - evt->index;
#endif
            cur_vol = evt->index;
            AudioSrv_SetVolume(cur_vol);
            prev_vol = *(uint32_t*)Setting_Get(SETID_VOLUME);
            Setting_Set(SETID_VOLUME, &cur_vol);
            sprintf(vol_msg, "Volume=%d.Adc=%d", cur_vol, evt->adcRawValue);
            DebugSSrv_Printf(vol_msg);
            ALWAYS_printf("\n\rVolume=%d.\n\r", cur_vol);
#ifdef MUTE_AMP_WHEN_VOL_IS_0
            if( (prev_vol == 0) && cur_vol )
            {   // demute the amplifier
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, FALSE);
            }
            else if( prev_vol && (cur_vol == 0) )
            {   // mute the amplifier
                AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, TRUE);
            }
            else
                ;
#endif
            break;
        }
        case BASS_KNOB_BASE_KEY_ID:
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_BASS_STEPS - 1 - evt->index;
#endif
            AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, evt->index, 0);
            cur_vol = evt->index;
            Setting_Set(SETID_BASS, &cur_vol);
            sprintf(vol_msg, "Bass=%d.Adc=%d", cur_vol, evt->adcRawValue);
            DebugSSrv_Printf(vol_msg);
            ALWAYS_printf("\n\rBass=%d.\n\r", cur_vol);
            break;
        case TREBLE_KNOB_BASE_KEY_ID:
#ifdef KNOB_KEY_INVERT_VALUE
            evt->index = MAX_TREBLE_STEPS - 1 - evt->index;
#endif
            AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, evt->index, 0);
            cur_vol = evt->index;
            Setting_Set(SETID_TREBLE, &cur_vol);
            sprintf(vol_msg, "Treble=%d.Adc=%d", cur_vol, evt->adcRawValue);
            DebugSSrv_Printf(vol_msg);
            ALWAYS_printf("\n\rTreble=%d.\n\r", cur_vol);
            break;
        case BT_KEY:
//            BRINGUP_printf("\n\r BT evt : %d \n\r", evt->keyEvent);
            if( evt->keyEvent == KEY_EVT_HOLD ) // long press
            {
//                if( ! (gpio_key_pressed & GPIO_KEY_SOURCE_PRESSED) )
//                    MainApp_ProcBtKey(me ,e);
            }
            else if( evt->keyEvent == KEY_EVT_DOWN )
            {
                gpio_key_pressed |= GPIO_KEY_BT_PRESSED;
            }
            else if( evt->keyEvent == KEY_EVT_UP )
            {
                gpio_key_pressed &= ~GPIO_KEY_BT_PRESSED;
                if( gpio_key_ignore_cnt )
                    gpio_key_ignore_cnt --;
                else if( ! (gpio_key_pressed & GPIO_KEY_SOURCE_PRESSED) )
                    MainApp_ProcBtKey(me ,e);
            }
            else
                ;
            break;
        case INPUT_KEY:
//            BRINGUP_printf("\n\r Input evt : %d \n\r", evt->keyEvent);
            if( evt->keyEvent == KEY_EVT_UP )
            {
                gpio_key_pressed &= ~GPIO_KEY_SOURCE_PRESSED;
                if( gpio_key_ignore_cnt )
                    gpio_key_ignore_cnt --;
                else if( ! (gpio_key_pressed & GPIO_KEY_BT_PRESSED) )
                    MainApp_ProcInputKey(me);
            }
            else if( evt->keyEvent == KEY_EVT_DOWN )
            {
                gpio_key_pressed |= GPIO_KEY_SOURCE_PRESSED;
            }
            else
                ;
            break;
#ifdef HAS_COMB_KEY
        case COMB_KEY_FACTORY_RESET: // factory reset
            gpio_key_ignore_cnt = 2;
            MainApp_ProcFactoryReset(me);
            break;
#endif
        case POWER_KEY:
            powerKey_handler(me, evt->keyEvent);
            break;
        default:
            break;
    }
}

static void powerKey_handler(cMainApp * const me, eKeyEvent keyEvent)
{
    switch(keyEvent)
    {
    case KEY_EVT_SHORT_PRESS:
        break;
    case KEY_EVT_LONG_PRESS:
        break;
    default:
        break;
    }
}

static void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}



