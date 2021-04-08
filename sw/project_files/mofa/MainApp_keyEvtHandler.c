/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "AllPlaySrv.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/
eCombinedKey MainApp_CombinedKey = 0;
extern tPatternData patternConfig[PAT_MAX_NUMBER];

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static void MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_V_L_HOLD);
            break;
        }
        case POWER_IR_KEY:
        {
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, POWER_IN_V_L_HOLD);
            break;
        }
        case NET_RESET_KEY:
        {
            TYM_CLR_BIT(MainApp_CombinedKey, NET_RESET_IN_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, NET_RESET_IN_L_HOLD);
            TYM_CLR_BIT(MainApp_CombinedKey, NET_RESET_IN_V_L_HOLD);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
            break;
        }
        case POWER_IR_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_HOLD);
            break;
        }
        case NET_RESET_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, NET_RESET_IN_HOLD);
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(0 < vol)
            {
                vol--;
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(MAX_VOLUME > vol)
            {
                vol++;
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        case NEXT_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_NEXT);
            break;
        }
        case PLAY_PAUSE_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_PLAY_PAUSE);
            break;
        }
        case PREV_KEY:
        {
            AllPlaySrv_SendCmd(ALLPLAY_CMD_PREV);
            break;
        }
        case SOURCE_SWITCH_IR_KEY:
        {
            me->audioSource++;
            if(MAX_SOURCE == me->audioSource)
            {
                me->audioSource = 0;
            }
            /* will go into source switching state */
            me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            break;
        }
        case SWITCH_SOURCE_KEY:
        {
            if(MainApp_CombinedKey&POWER_IN_HOLD)
            {
                TYM_SET_BIT(MainApp_CombinedKey, POWER_SP_RELEASE);
            }
            break;
        }
        case POWER_IR_KEY:
        {
            if(MainApp_CombinedKey&POWER_IN_HOLD)
            {
                TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_IR_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            break;
        }
        case POWER_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            if(MainApp_CombinedKey&POWER_IN_HOLD)
            {
                TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            }
            break;
        }
        case POWER_IR_KEY:
        {
            if(MainApp_CombinedKey&POWER_IN_HOLD)
            {
                TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_L_HOLD);
            }
            break;
        }
        case NET_RESET_KEY:
        {
            TYM_SET_BIT(MainApp_CombinedKey, NET_RESET_IN_L_HOLD);
            break;
        }
        case SOURCE_SWITCH_IR_KEY: // Hold without release to start BT pairing at any audio source mode.
        {
            if(me->audioSource == BLUETOOTH)
            {
                MainApp_SendLedReq(me, BT_PAIRING_ENABLE_PAT);
                AllPlaySrv_BluetoothPairable(TRUE);
            }
            else
            {
                me->audioSource = BLUETOOTH;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            me->tickHandlers[BT_PAIRING_TIMER].timer = MAINAPP_BT_PAIRING_TIMEOUT_IN_MS;
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}

static void MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(0 < vol)
            {
                vol = (vol < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                    0 : (vol - MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
            if(MAX_VOLUME > vol)
            {
                vol = ((MAX_VOLUME - vol) < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                    MAX_VOLUME : (vol + MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                /* Set volume level */
                AudioSrv_SetVolume(vol);
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
            TYM_SET_BIT(MainApp_CombinedKey, POWER_IN_V_L_HOLD);
            break;
        case NET_RESET_KEY:
            TYM_SET_BIT(MainApp_CombinedKey, NET_RESET_IN_V_L_HOLD);
            break;
        case SOURCE_SWITCH_IR_KEY: // very long hold to reset network at any audio source.
        {
            AllPlaySrv_netReset();
            if(me->audioSource != ALLPLAY_AP_MODE)
            {
                me->audioSource = ALLPLAY_AP_MODE;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void MainApp_CombineKeyHandler(cMainApp * const me, QEvt const * const e, eCombinedKey comKey)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(comKey)
    {
        case SWITCH_SOURCE_TRIGGER:
        {
            MainApp_CombinedKey = INVILAD_COM_KEY;
            me->audioSource++;
            if(MAX_SOURCE == me->audioSource)
            {
                me->audioSource = 0;
            }
            /* will go into source switching state */
            me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            break;
        }
        case STANDBY_TRIGGER:
        {
            MainApp_CombinedKey = INVILAD_COM_KEY;
            /* will go into standby state */
            me->nextState = (QStateHandler*)&MainApp_PoweringDown;
            break;
        }
        case BT_PAIRING_TRIGGER:
        {
            if(me->audioSource == BLUETOOTH)
            {
                MainApp_SendLedReq(me, BT_PAIRING_ENABLE_PAT);
                AllPlaySrv_BluetoothPairable(TRUE);
            }
            else
            {
                me->audioSource = BLUETOOTH;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            me->tickHandlers[BT_PAIRING_TIMER].timer = MAINAPP_BT_PAIRING_TIMEOUT_IN_MS;
            break;
        }
        case NETWORK_RESET_TRIGGER:
        {
            MainApp_CombinedKey = INVILAD_COM_KEY;
            AllPlaySrv_netReset();
            if(me->audioSource != ALLPLAY_AP_MODE)
            {
                me->audioSource = ALLPLAY_AP_MODE;
                me->nextState = (QStateHandler*)&MainApp_SourceSwitching;
            }
            break;
        }
        case FACTORY_RESET_TRIGGER:
        {
            MainApp_CombinedKey = INVILAD_COM_KEY;
            AllPlaySrv_resetToFactory();
            MainApp_SendLedReq(me, RST_IN_PROG_PAT);
            NvmDrv_EraseAll(NULL);
            NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);

            /*TODO: The below code should be removed after QualComm fixing the issue *
            * about no response for factory reset*/
            /* will go into soft reset state */
            me->nextState = (QStateHandler*)&MainApp_SoftReset;
            break;
        }
        default:break;
    }
}

void MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            MainApp_KeyUpEvtAction(me, evt);
            break;
        case KEY_EVT_DOWN:
            MainApp_KeyDownEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            MainApp_KeyLPressEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            MainApp_KeyVLPressEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            MainApp_KeyVeryLHoldEvtAction(me, evt);
            break;
        default:break;
    }

    MainApp_CombineKeyHandler(me, e, MainApp_CombinedKey);

    if((KEY_EVT_REPEAT == (evt->keyEvent) || KEY_EVT_SHORT_PRESS == (evt->keyEvent) ||
        KEY_EVT_LONG_PRESS == (evt->keyEvent)) && SOURCE_SWITCH_IR_KEY != evt->keyId &&
        SWITCH_SOURCE_KEY != evt->keyId)
    {
        AllPlaySrvInfoEvt* allPlayInfo= (AllPlaySrvInfoEvt*)Setting_Get(SETID_ALLPLAY_INFO);
        /* According to UI design, led should NOT be dim if led indication is WIFI connecting or BT pairing pattern. */
        if((me->audioSource == ALLPLAY_AP_MODE && (ALLPLAY_NETWORK_WIFI != allPlayInfo->networkType
            || ALLPLAY_SYSTEM_MODE_CONFIGURED != allPlayInfo->eSystemMode))
            || (me->tickHandlers[BT_PAIRING_TIMER].timer > 0))
        {
            MAINAPP_DEBUG_MSG(" According to UI design, led should NOT be dim if led indication is WIFI connecting or BT pairing pattern. \n");
        }
        else
        {
            if(0 == me->tickHandlers[LED_DIM_TIMER].timer)
            {
                patternConfig[SOLID_PAT].color = me->sourceHandler[me->audioSource].sourceColor;
                MainApp_SendLedReq(me, SOLID_PAT);
            }
            me->tickHandlers[LED_DIM_TIMER].timer = MAINAPP_LED_DIM_TIMEOUT_IN_MS;
        }
    }
}

void MainApp_AllPlayKeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_SHORT_PRESS:
        {
            switch(evt->keyId)
            {
                case VOLUME_DOWN_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(0 < vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol-1);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                case VOLUME_UP_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(MAX_VOLUME > vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol+1);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case KEY_EVT_REPEAT:
        {
            switch(evt->keyId)
            {
                case VOLUME_DOWN_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    /* update allplay for the mobile app. vol. bar display */
                    vol = (vol < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                        0 : (vol - MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                    uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol);
                    AllPlaySrv_SetVol(allplayVol);
                    break;
                }
                case VOLUME_UP_KEY:
                {
                    uint8 vol = *(uint8*)Setting_Get(SETID_VOLUME);
                    if(MAX_VOLUME > vol)
                    {
                        /* update allplay for the mobile app. vol. bar display */
                        vol = ((MAX_VOLUME - vol) < MAINAPP_UPDATE_SAM_VOL_INTERVAL) ?
                            MAX_VOLUME : (vol + MAINAPP_UPDATE_SAM_VOL_INTERVAL);
                        uint8 allplayVol = MainApp_mcuVol2AllplayVol(vol);
                        AllPlaySrv_SetVol(allplayVol);
                    }
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case KEY_EVT_VERY_LONG_HOLD:
        {
            switch(evt->keyId)
            {
                case NET_RESET_KEY:
                {
                    MainApp_DisplayAllplaySystemMode(me, ALLPLAY_SYSTEM_MODE_UNCONFIGURED);
                    break;
                }
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
}
