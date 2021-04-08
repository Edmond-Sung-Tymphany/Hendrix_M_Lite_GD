/**
*  @file      MainApp_util.c
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "bit_shift_64.h"
#include "bl_common.h"
#include "fep_addr.h"

/*****************************************************************
 * Definition
 *****************************************************************/
#define AUX_IN_STATUS_BITMASK       AUXIN_JACK


extern tPatternData patternConfig[PAT_MAX_NUMBER];


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_InitStatusVariables(cMainApp * const me)
{
    uint32 temp;

    me->connState = Proto_BtState_ConnState_CONNECTABLE;
    me->systemStatus = SYSTEM_STA_OFF;
    me->muteStatus = FALSE;
    me->vol = DEFAULT_VOLUME*CLICK_NUM_PER_VOLUME_STEP;

    /* Resume data from flash */
    temp = *(uint32_t *)Setting_Get(SETID_BASS);
    me->bass = (temp < MAX_BASS_STEPS) ? temp : DEFAULT_BASS;
    me->bass = me->bass*CLICK_NUM_PER_BASS_STEP;

    temp = *(uint32_t *)Setting_Get(SETID_TREBLE);
    me->treble = (temp < MAX_TREBLE_STEPS) ? temp : DEFAULT_TREBLE;
    me->treble = me->treble*CLICK_NUM_PER_TREBLE_STEP;

    temp = *(uint32_t *)Setting_Get(SETID_AUDIO_SOURCE);
    me->audioSource = (temp < AUDIO_SOURCE_MAX) ? temp : AUDIO_SOURCE_DEFAULT;

    uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
    if (workMode == WORK_MODE_ID_SHOP)
    {
        /* Only support analog mixed source in Shop mode. */
        me->audioSource = AUDIO_SOURCE_ANALOG_MIXED;
    }
    else if (workMode >= WORK_MODE_ID_MAX)
    {
        workMode = WORK_MODE_ID_NORMAL;
        Setting_Set(SETID_WORK_MODE, &workMode);
    }

    temp = *(uint32_t *)Setting_Get(SETID_STEADY_BRIGHTNESS);
    if (temp <= STEADY_BRIGHTNESS_MAX)
    {
        MainApp_UpdateSteadyBrightness((uint8)temp);
    }
    temp = *(uint32_t *)Setting_Get(SETID_DIM_BRIGHTNESS);
    if (temp <= DIM_BRIGHTNESS_MAX)
    {
        MainApp_UpdateDimBrightness((uint8)temp);
    }

    Setting_Set(SETID_SW_VER_STR, PRODUCT_VERSION_MCU);
}

void MainApp_UpdateSteadyBrightness(uint8 steady_brightness)
{
    uint8  i = 0;
    for (i = STEADY_PAT_MIN; i <= STEADY_PAT_MAX; i++)
    {
        patternConfig[i].color = RGBA(MAX_BRT,0,0, steady_brightness);
    }
    Setting_Set(SETID_STEADY_BRIGHTNESS, &steady_brightness);
}

void MainApp_UpdateDimBrightness(uint8 dim_brightness)
{
    uint8  i = 0;
    for (i = DIM_PAT_MIN; i <= DIM_PAT_MAX; i++)
    {
        patternConfig[i].color = RGBA(MAX_BRT,0,0, dim_brightness);
    }
    Setting_Set(SETID_DIM_BRIGHTNESS, &dim_brightness);
}

void MainApp_SendLedReq(cMainApp* me, eLedIndID ledIndID)
{
    ASSERT(ledIndID < LED_IND_ID_MAX);
    LedSrv_SetPatt((QActive*)me, me->ledInds[ledIndID].leds, me->ledInds[ledIndID].patternId);
}

void MainApp_SetRotaterLedOn(cMainApp* me, eLed startLed, uint32 level)
{
    uint8 i;
    eLedMask leds;
    leds = (1ULL)<<(startLed);
    for(i = 0; i <= level; i++)
    {
        leds |= shift_left64(1ULL, (startLed + i));
    }

    if (me->systemStatus == SYSTEM_STA_ACTIVE || me->systemStatus == SYSTEM_STA_POWERING_UP)
    {
        LedSrv_SetPatt((QActive*)me, leds, SOLID_PAT_RED);
    }
    else
    {
        LedSrv_SetPatt((QActive*)me, leds, DIM_PATT_RED);
    }
}

void MainApp_SetRotaterLedOff(cMainApp* me, eLed startLed, uint32 level)
{
    uint8 i;
    eLedMask leds = 0ULL;
    for(i = level+1; i <= ROTATER_LED_NUM; i++)
    {
        leds |= shift_left64(1ULL, (startLed + i));
    }
    LedSrv_SetPatt((QActive*)me, leds, OFF_PATT);
}

void MainApp_SetRotaterLedDim(cMainApp* me, eLed startLed, uint32 level)
{
    uint8 i;
    eLedMask leds;
    leds = (1ULL)<<(startLed);
    for(i = 0; i <= level; i++)
    {
        leds |= shift_left64(1ULL, (startLed + i));
    }
    LedSrv_SetPatt((QActive*)me, leds, DIM_PATT_RED);
}

void MainApp_UpdateBtLed(cMainApp * const me, Proto_BtState_ConnState connState)
{
    eLedIndID btConnLedInd = LED_IND_ID_BT_CONNECTABLE;
    TP_PRINTF("bt connection state: %d \r\n", connState);
    if (me->systemStatus == SYSTEM_STA_SEMI_ACTIVE)
    {
        btConnLedInd = LED_IND_ID_BT_CONNECTABLE_DIM;
    }
    btConnLedInd = btConnLedInd + connState; // use connect state as offset;
    MainApp_SendLedReq(me, btConnLedInd);
}

void MainApp_UpdateLed(cMainApp* me, eSystemStatus newStatus)
{
    eLedMask leds = me->sourceHandler[me->audioSource].leds;

    if (newStatus == SYSTEM_STA_ACTIVE || newStatus == SYSTEM_STA_POWERING_UP)
    {
        //update source led
        if (me->audioSource == AUDIO_SOURCE_ANALOG_MIXED)
        {
            /* Do NOT display source LED indication in Shop mode. */
        }
        else if (me->audioSource == AUDIO_SOURCE_BT)
        {
            MainApp_UpdateBtLed(me, me->connState);
        }
        else
        {
            LedSrv_SetPatt((QActive*)me, leds, SOLID_PAT_RED);
        }

        //update power led
        MainApp_SendLedReq(me, LED_IND_ID_POWERED_ON);
        //update rotaters led
        MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[me->vol/CLICK_NUM_PER_VOLUME_STEP]);
        MainApp_SetRotaterLedOn(me, LED_BAS_0, me->bass/CLICK_NUM_PER_BASS_STEP);
        MainApp_SetRotaterLedOn(me, LED_TRE_0, me->treble/CLICK_NUM_PER_BASS_STEP);
    }
    else if (newStatus == SYSTEM_STA_SEMI_ACTIVE)
    {
        //update source led
        if (me->audioSource == AUDIO_SOURCE_ANALOG_MIXED)
        {
            /* Do NOT display source LED indication in Shop mode. */
        }
        else if (me->audioSource == AUDIO_SOURCE_BT)
        {
            MainApp_UpdateBtLed(me, me->connState);
        }
        else
        {
            LedSrv_SetPatt((QActive*)me, leds, DIM_PATT_RED);
        }

        //update power led
        MainApp_SendLedReq(me, LED_IND_ID_PRODUCT_IDLE);
        //update rotaters led
        MainApp_SetRotaterLedDim(me, LED_VOL_0, vol_led_mapping[me->vol/CLICK_NUM_PER_VOLUME_STEP]);
        MainApp_SetRotaterLedDim(me, LED_BAS_0, me->bass/CLICK_NUM_PER_BASS_STEP);
        MainApp_SetRotaterLedDim(me, LED_TRE_0, me->treble/CLICK_NUM_PER_BASS_STEP);
    }
    else if (newStatus == SYSTEM_STA_STANDBY)
    {
        /* Firstly turn off all LED */
        MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
        //update source led
        LedSrv_SetPatt((QActive*)me, leds, DIM_PATT_RED);
        //update power led
        MainApp_SendLedReq(me, LED_IND_ID_PRODUCT_IDLE);
    }
    else if (newStatus == SYSTEM_STA_POWERING_DOWN)
    {
        MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);
    }
}

void MainApp_UpdateSystemStatus(cMainApp* me, eSystemStatus newStatus)
{
    /* update system satus and timer */
    if (me->systemStatus != newStatus)
    {
        me->systemStatus = newStatus;
        MainApp_UpdateLed(me, newStatus);
    }

    if (me->systemStatus == SYSTEM_STA_ACTIVE)
    {
        MainApp_ResetSemiActiveTimer(me);
    }
}

void MainApp_DelayedErrorReboot(cMainApp * const me)
{
    //delayed reboot for release biuld
    MainApp_SendLedReq(me, LED_IND_ID_PROD_FW_ERROR);

    /* wait 5 second(for finished LED indication), then reboot system. */
    me->tickHandlers[TIMER_ID_DELAYED_ERROR_REBOOT].timer = MAINAPP_DELAYED_ERROR_REBOOT_TIMEOUT_IN_MS;
}

void MainApp_ResetSemiActiveTimer(cMainApp * const me)
{
    uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
    if (workMode == WORK_MODE_ID_SHOP)
    {
        /* Use different semiActive timeout in shop mode. */
        me->tickHandlers[TIMER_ID_SEMI_ACTIVE_TIMEOUT].timer = MAINAPP_SEMI_ACTIVE_TIMEOUT_IN_SHOPMODE_IN_MS;
    }
    else
    {
        me->tickHandlers[TIMER_ID_SEMI_ACTIVE_TIMEOUT].timer = MAINAPP_SEMI_ACTIVE_TIMEOUT_IN_MS;
    }
}

void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

void MainApp_Mute(cMainApp* me, bool muteEnable2)
{

    AudioSrv_SendMuteReq((QActive *)me, AUDIO_AMP_SOFT_MUTE, muteEnable2);
}

eAudioSource MainApp_GetNextAvailableSource(cMainApp * const me)
{
    eAudioSource audioSrc = me->audioSource;
    uint8 i = 0;
    for(i = 0; i < AUDIO_SOURCE_MAX; i++)
    {
        /* goto check next audio source*/
        audioSrc++;
        if(audioSrc >= AUDIO_SOURCE_MAX)
        {
            audioSrc = AUDIO_SOURCE_MIN;
        }
        if(TRUE == me->sourceHandler[audioSrc].bIsValid)
        {
            break;
        }
    }
    return audioSrc;
}

void MainApp_SwitchAudioSource(cMainApp * const me, eAudioSource source, bool force)
{
    eAudioSource currentSrc = *(uint8 *)Setting_Get(SETID_AUDIO_SOURCE);
    TP_PRINTF("MainApp_SwitchAudioSource: source=%d \r\n", source);

    if (source >= AUDIO_SOURCE_MAX || source <= AUDIO_SOURCE_MIN)
    {
        source = AUDIO_SOURCE_MIN;
    }

    uint32 workMode = *(uint32 *)Setting_Get(SETID_WORK_MODE);
    if ((workMode == WORK_MODE_ID_SHOP) && (TRUE != force) && (source != AUDIO_SOURCE_ANALOG_MIXED))
    {
        /* Only support analog mixed source in Shop mode. */
        return;
    }

    me->audioSource = source;
    if ((currentSrc != source) || (TRUE == force))
    {
        AudioSrv_SendMuteReq((QActive *)me, AUDIO_SOURCE_MUTE, TRUE);
        me->tickHandlers[TIMER_ID_SOURCE_SWITCH_TIMEOUT].timer = MAINAPP_SOURCE_SWITCH_TIMEOUT;
        me->tickHandlers[TIMER_ID_UNMUTE_SYS_TIMEOUT].timer = 0;

        MainApp_SendLedReq(me, LED_IND_ID_OFF_ALL_SRC_LED);
        eLedMask leds = me->sourceHandler[me->audioSource].leds;
        if (me->audioSource == AUDIO_SOURCE_ANALOG_MIXED)
        {
            /* Do NOT display source LED indication in Shop mode. */
        }
        else if (me->audioSource == AUDIO_SOURCE_BT)
        {
            MainApp_UpdateBtLed(me, me->connState);
        }
        else if (me->systemStatus == SYSTEM_STA_ACTIVE)
        {
            LedSrv_SetPatt((QActive*)me, leds, SOLID_PAT_RED);
        }
        else
        {
            LedSrv_SetPatt((QActive*)me, leds, DIM_PATT_RED);
        }
    }
}


void MainApp_FactoryResetSettings(cMainApp * const me)
{
    uint32 temp;
    temp = DEFAULT_BASS;
    Setting_Set(SETID_BASS, &temp);

    temp = DEFAULT_TREBLE;
    Setting_Set(SETID_TREBLE, &temp);

    temp = AUDIO_SOURCE_DEFAULT;
    Setting_Set(SETID_AUDIO_SOURCE, &temp);

    temp = REBOOT_CODE_FACTORY_RESET;
    Setting_Set(SETID_REBOOT_CODE, &temp);

    SettingSrv_FlashReq((QActive *)me, TRUE);
}


void MainApp_UpdateVolumeSyncTimer(cMainApp * const me)
{
    if(me->tickHandlers[TIMER_ID_VOLUME_SYNC_TIMEOUT].timer == 0)
    {
        me->tickHandlers[TIMER_ID_VOLUME_SYNC_TIMEOUT].timer = MAINAPP_VOLUME_SYNC_TIMEOUT_IN_MS;
    }
}

bool MainApp_IsAutoBoot(cMainApp * const me)
{
    bool autoBoot = FALSE;

    /* Powering on directly if reboot from factory reset. */
    uint32 rebootCode = *(uint32_t *)Setting_Get(SETID_REBOOT_CODE);

    if (REBOOT_CODE_FACTORY_RESET == rebootCode)
    {
        TP_PRINTF("System is booting up after factory reset. \r\n");
        rebootCode = REBOOT_CODE_INVALID; //Reset reboot code
        Setting_Set(SETID_REBOOT_CODE, &rebootCode);
        autoBoot = TRUE;
    }

    /* Auto boot up after firmware upgrade. */
    eFepFirmwareStatus fw_status = bl_getFirmwareStatus();
    if (FEP_FIRMWARE_NEW == fw_status)
    {
        TP_PRINTF("System is booting up from firmware upgrade. \r\n");
        bl_setFirmwareStatus(FEP_FIRMWARE_NORMAL);
        autoBoot = TRUE;
    }

    if (me->audioSource == AUDIO_SOURCE_ANALOG_MIXED)
    {
        TP_PRINTF("System is auto booting up in shop mode. \r\n");
        autoBoot = TRUE;
    }

    return autoBoot;
}

void MainApp_EnterShopMode(cMainApp * const me)
{
    uint32 temp;

    MainApp_SwitchAudioSource(me, AUDIO_SOURCE_ANALOG_MIXED, TRUE);

    /* Set default Bass level */
    temp = DEFAULT_BASS_IN_SHOPMODE;
    Setting_Set(SETID_BASS, &temp);
    if (me->bass/CLICK_NUM_PER_BASS_STEP > temp)
    {
        MainApp_SetRotaterLedOff(me, LED_BAS_0, temp);
    }
    else
    {
        MainApp_SetRotaterLedOn(me, LED_BAS_0, temp);
    }
    me->bass = temp*CLICK_NUM_PER_BASS_STEP;
    AudioSrv_SetAudio(DSP_BASS_SETT_ID, TRUE, me->bass/CLICK_NUM_PER_BASS_STEP, NULL);

    /* Set default Treble level */
    temp = DEFAULT_TREBLE_IN_SHOPMODE;
    Setting_Set(SETID_TREBLE, &temp);
    if (me->treble/CLICK_NUM_PER_TREBLE_STEP > temp)
    {
        MainApp_SetRotaterLedOff(me, LED_TRE_0, temp);
    }
    else
    {
        MainApp_SetRotaterLedOn(me, LED_TRE_0, temp);
    }
    me->treble = temp*CLICK_NUM_PER_TREBLE_STEP;
    AudioSrv_SetAudio(DSP_TREBLE_SETT_ID, TRUE, me->treble/CLICK_NUM_PER_TREBLE_STEP, NULL);

    /* Set default Treble level */
    temp = DEFAULT_VOLUME_IN_SHOPMODE;
    Setting_Set(SETID_VOLUME, &temp);
    if (me->vol/CLICK_NUM_PER_VOLUME_STEP > temp)
    {
        MainApp_SetRotaterLedOff(me, LED_VOL_0, vol_led_mapping[temp]);
    }
    else
    {
        MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[temp]);
    }
    me->vol = CLICK_NUM_PER_VOLUME_STEP * temp;
    AudioSrv_SetVolume(temp);

    /* Flush settings into flash immediately */
    SettingSrv_FlashReq((QActive *)me, TRUE);

    BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_SOURCE_CHANGE_EVENT, sizeof(eAudioSource), (uint8 *)&me->audioSource);
    BluetoothSrv_SendBtCmd((QActive*)me, BT_FACTORY_RESET_REQ);
}

