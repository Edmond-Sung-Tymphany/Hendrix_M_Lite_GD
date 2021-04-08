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
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "Setting_id.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "AudioSrv.h"
/*****************************************************************
 * Definition
 *****************************************************************/



/*****************************************************************
 * Function Implemenation
 *****************************************************************/

void MainApp_PteCmdHandler(cMainApp * const me,  QEvt const * const e)
{
    PteTestCmdEvt * evt = (PteTestCmdEvt*)e;


    switch(evt->pteTestCmd)
    {
        case PTE_TEST_CMD_ID_VOLUME:
        {
            me->vol = evt->param * CLICK_NUM_PER_VOLUME_STEP;
            AudioSrv_SetVolume(me->vol/CLICK_NUM_PER_VOLUME_STEP);
            LedSrv_SetPatt((QActive*)me, LED_MASK_VOL_LEDS, OFF_PATT);
            MainApp_SetRotaterLedOn(me, LED_VOL_0, vol_led_mapping[me->vol/CLICK_NUM_PER_VOLUME_STEP]);
            if (me->audioSource == AUDIO_SOURCE_BT)
            {
                if (me->vol/CLICK_NUM_PER_VOLUME_STEP < (MAX_VOLUME_STEPS-1))
                {
                    uint32 vol = VOLUME_STEP_TO_ABSOLUTE(me->vol/CLICK_NUM_PER_VOLUME_STEP);
                    BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_VOLUME_CHANGE_EVENT, sizeof(vol), (uint8 *)&vol);
                    TP_PRINTF("bt->Mcu: volume = %d\r\n", VOLUME_STEP_TO_ABSOLUTE(me->vol/CLICK_NUM_PER_VOLUME_STEP));
                }
                else
                {
                    uint32 vol = MAX_ABSOLUTE_VOLUME;
                    BluetoothSrv_SendBtCmdWithParam((QActive*)me, MCU_VOLUME_CHANGE_EVENT, sizeof(vol), (uint8 *)&vol);
                    TP_PRINTF("bt->Mcu: volume = 127\r\n");
                }
            }
            break;
        }

        case PTE_TEST_CMD_ID_SOURCE_SWITCH:
        {
            MainApp_SwitchAudioSource(me,(eAudioSource)evt->param,TRUE);
            break;
        }

        case PTE_TEST_CMD_ID_SET_BRIGHTNESS:
        {
            if (evt->param <= LED_BRIGHTNESS_MAX)
            {
                MainApp_UpdateSteadyBrightness(evt->param);
            }
            if (evt->param2 <= LED_BRIGHTNESS_MAX)
            {
                MainApp_UpdateDimBrightness(evt->param2);
            }
            MainApp_UpdateLed(me, me->systemStatus);
            break;
        }
        case PTE_TEST_CMD_ID_SET_WORK_MODE:
        {
            uint32 workMode = evt->param;
            if (workMode >= WORK_MODE_ID_MAX)
            {
                workMode = WORK_MODE_ID_NORMAL;
            }
            Setting_Set(SETID_WORK_MODE, &workMode);
            if (workMode == WORK_MODE_ID_SHOP)
            {
                MainApp_EnterShopMode(me);
            }
            else
            {
                MainApp_SwitchAudioSource(me, AUDIO_SOURCE_AUXIN, TRUE);
            }
            break;
        }

        default:
            break;
    }
    return;
}
