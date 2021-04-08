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
#include "MainApp_keyEvtHandler.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_bleEvtHandler.h"

#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "tym_qp_lib.h"
#include "SettingSrv.h"
#include "KeySrv.h"
#include "UsbSrv.h"

#define EQ_SETTING_START_INDEX      4

/*****************************************************************
 * Global Variable
 *****************************************************************/
/*refer to the data transfer protocol */
static remapKey remapKeyTable[] =
{
    {24,    RMK_PRESET_1_LOAD},
    {25,    RMK_PRESET_2_LOAD},
    {26,    RMK_PRESET_3_LOAD},
    {27,    RMK_PRESET_4_LOAD},
    {28,    RMK_PRESET_1_SAVE},
    {29,    RMK_PRESET_2_SAVE},
    {30,    RMK_PRESET_3_SAVE},
    {35,    RMK_FACTORY_RESET},
    {46,    RMK_SCREEN_ON_OFF},
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/
void MainApp_SwitchPage(cMainApp * const me, ePageSettingId id)
{
    /* Check the value is value or not */
    if(id == me->pageSetting)
    {
        return;
    }

    /* turn on current page led */
    me->pageSetting = id;
}


void MainApp_SelectStandbyMode(cMainApp * const me)
{
    uint8 settIndex;

    TP_PRINTF("STANDBY_MODE= ");
    if(STANDBY_MODE_TRIGGER != me->standbyMode)
    {
        settIndex = MainApp_GetSettIndex(PAGE_SETTING_STANDBY);

        if(STANDBY_MODE_ON == me->standbyMode)
        {
            // switch to Auto Mode
            me->standbyMode = STANDBY_MODE_AUTO;
            TP_PRINTF("AUTO\r\n");
        }
        else
        {
            // switch to ON Mode
            me->standbyMode = STANDBY_MODE_ON;
            TP_PRINTF("ON\r\n");
        }

        me->pMenuData[settIndex] = me->standbyMode;

        MainApp_BleReadDataResp(&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
    }
}


static void MainApp_KeyPresetLoad(cMainApp * const me, eSettingId settId)
{
    /* load a preset to SETTID_MENU_DATA */
    int16 *pPreset = (int16 *)Setting_Get(settId);

    for(int16 i = EQ_SETTING_START_INDEX; i < NUM_OF_MENU_DATA; i++)
    {
        me->pMenuData[i] = pPreset[i];
    }

    me->standbyMode = MainApp_GetMenuData(me, PAGE_SETTING_STANDBY);
    me->pageSetting = PAGE_SETTING_VOL;

    /* refresh DSP */
    AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
    MainApp_LoadPreset(me);

    /*delay a few seconds to unmute the dsp, to avoid a bigger pop noise*/
    me->tickHandlers[TIMER_ID_DSPMUTE_TIMEOUT].timer = TIMER_ID_DSPMUTE_TIMEOUT_IN_MS;
    /* response to APP*/
    MainApp_BleReadDataResp(&(me->pMenuData[0]), SETID_MENU_DATA, NUM_OF_MENU_DATA * sizeof(uint16), 0);
}

static void MainApp_KeyDefaultPresetLoad(cMainApp * const me)
{
    /* load default settings */
    for(int16 i = EQ_SETTING_START_INDEX; i < NUM_OF_MENU_DATA; i++)
    {
        me->pMenuData[i] = MainApp_GetMenuDefault(i);
    }

    me->standbyMode = MainApp_GetMenuData(me, PAGE_SETTING_STANDBY);
    me->pageSetting = PAGE_SETTING_VOL;

    /* refresh DSP */
    AudioSrv_SendMuteReq((QActive*)me, AUDIO_DSP_DACOUT_MUTE, TRUE);
    MainApp_LoadPreset(me);

    /*delay a few seconds to unmute the dsp, to avoid a bigger pop noise*/
    me->tickHandlers[TIMER_ID_DSPMUTE_TIMEOUT].timer = TIMER_ID_DSPMUTE_TIMEOUT_IN_MS;
    /* response to APP*/
    MainApp_BleReadDataResp(&(me->pMenuData[0]), SETID_MENU_DATA, NUM_OF_MENU_DATA * sizeof(uint16), 0);
}

static void MainApp_KeyPresetSave(cMainApp * const me, eSettingId settId)
{
    int16 *pPreset = (int16 *)Setting_Get(settId);
    for(int16 i = EQ_SETTING_START_INDEX; i < NUM_OF_MENU_DATA; i++)
    {
      pPreset[i] = me->pMenuData[i];
    }

}

static QState MainApp_KeyUpEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_UNHANDLED();

    return ret;
}

static QState MainApp_KeyDownEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_HANDLED();

    switch(keyId)
    {
        case VOL_KEY:
        {
            TP_PRINTF("VOL_KEY \r\n");
            MainApp_SwitchPage(me, PAGE_SETTING_VOL);
            break;
        }
        case LPF_KEY:
        {
            MainApp_SwitchPage(me, PAGE_SETTING_LP_FRE);
            break;
        }
        case PHASE_KEY:
        {
            MainApp_SwitchPage(me, PAGE_SETTING_PHASE);
            break;
        }
        case PLUS_KEY:
        {
            MainApp_CalcAndSet(me, TRUE);
            break;
        }
        case MINUS_KEY:
        {
            MainApp_CalcAndSet(me, FALSE);
            break;
        }
        case STANDBY_KEY:
        {
            MainApp_SelectStandbyMode(me);
            break;
        }
        case STANDBY_TEST_KEY:
        {
            /*this key is used by TP_MONITOR to enter standby quickly */
            ret = Q_TRAN(&MainApp_Standby);
            break;
        }

        /* below keys are sent by APP, so MUST keep the eKeyId is the same */
        case RMK_SCREEN_ON_OFF:
        {
            bool screenStatus = *(bool*)Setting_Get(SETID_SCREEN_STATUS);
            screenStatus = screenStatus ? FALSE : TRUE;
            Setting_Set(SETID_SCREEN_STATUS, &screenStatus);

            /* previous value of @ret is Q_TRAN(&MainApp_Active), MUST set this to forbid wakeup */
            ret = Q_HANDLED();
            break;
        }
        case RMK_FACTORY_RESET:
        {
            ret = Q_TRAN(&MainApp_FactoryReset);
            break;
        }
        case RMK_PRESET_1_LOAD:
        {
            MainApp_KeyPresetLoad(me, SETID_PRESET_1);
            break;
        }
        case RMK_PRESET_1_SAVE:
        {
            MainApp_KeyPresetSave(me, SETID_PRESET_1);
            break;
        }
        case RMK_PRESET_2_LOAD:
        {
            MainApp_KeyPresetLoad(me, SETID_PRESET_2);
            break;
        }
        case RMK_PRESET_2_SAVE:
        {
           MainApp_KeyPresetSave(me, SETID_PRESET_2);
            break;
        }
        case RMK_PRESET_3_LOAD:
        {
            MainApp_KeyPresetLoad(me, SETID_PRESET_3);
            break;
        }
        case RMK_PRESET_3_SAVE:
        {
            MainApp_KeyPresetSave(me, SETID_PRESET_3);
            break;
        }

        case RMK_PRESET_4_LOAD:
        {
            /* here should save the  default setting on SETID_MENU_DATA */
            MainApp_KeyDefaultPresetLoad(me);
            break;
        }

        /*if necessary, add the handler for keys here */
        default:
        {
            ret = Q_UNHANDLED();
            break;
        }
    }

    return ret;
}


static QState MainApp_KeySPressEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_UNHANDLED();

    return ret;
}

static QState MainApp_KeyLPressEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret= Q_UNHANDLED();

    return ret;
}


static QState MainApp_KeyVLPressEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_UNHANDLED();

    return ret;
}


static QState MainApp_KeyHoldEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_UNHANDLED();

    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret = Q_HANDLED();

    switch(keyId)
    {
        case PLUS_KEY:
        {
            MainApp_CalcAndSet(me, TRUE);
            break;
        }
        case MINUS_KEY:
        {
            MainApp_CalcAndSet(me, FALSE);
            break;
        }
        /*if necessary, add the handler for keys here */
        default:
        {
            ret = Q_UNHANDLED();
            break;
        }
    }

    return ret;
}

static QState MainApp_KeyVLHoldEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret= Q_HANDLED();

    switch(keyId)
    {
        case PHASE_KEY:
        {
            me->upgradeMethod = UPGRADE_BY_USB;
            return Q_TRAN(&MainApp_EnterUpgrading);
        }
        /*if necessary, add the handler for more keys here */
        /*if necessary, add the handler for keys here */
        default:
        {
            ret = Q_UNHANDLED();
            break;
        }
    }

    return ret;
}

static QState MainApp_KeyCombEvtAction(cMainApp * const me, uint32_t keyId)
{
    QState ret= Q_UNHANDLED();

    return ret;
}

QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;

    uint32_t realKeyId = evt->keyId;

    /*remap the keyId which is from APP */
    for(uint32_t i = 0; i < ArraySize(remapKeyTable); i++)
    {
        if(remapKeyTable[i].keyId == realKeyId)
        {
            realKeyId = remapKeyTable[i].remapKeyId;
            break;
        }
    }

    TP_PRINTF("key=(%d, %d), evt=%d\r\n", evt->keyId, realKeyId, evt->keyEvent);

    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            ret = MainApp_KeyUpEvtAction(me, realKeyId);
            break;
        case KEY_EVT_DOWN:
            ret = MainApp_KeyDownEvtAction(me, realKeyId);
            break;
        case KEY_EVT_SHORT_PRESS:
            ret = MainApp_KeySPressEvtAction(me, realKeyId);
            break;
        case KEY_EVT_LONG_PRESS:
            ret= MainApp_KeyLPressEvtAction(me, realKeyId);
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            ret= MainApp_KeyVLPressEvtAction(me, realKeyId);
            break;
        case KEY_EVT_HOLD:
            ret= MainApp_KeyHoldEvtAction(me, realKeyId);
            break;
        case KEY_EVT_REPEAT:
            ret= MainApp_KeyRepeatEvtAction(me, realKeyId);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            ret= MainApp_KeyVLHoldEvtAction(me, realKeyId);
            break;
        case COMB_KEY_EVT:
            ret = MainApp_KeyCombEvtAction(me, realKeyId);
            break;
        default:
            TP_PRINTF("known key event\r\n");
            break;
    }

    return ret;
}

