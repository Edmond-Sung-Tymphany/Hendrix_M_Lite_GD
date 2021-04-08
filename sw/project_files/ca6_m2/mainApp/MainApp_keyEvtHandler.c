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
#include "AseTkSrv.h"
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
typedef enum
{
    VOL_ROTATION_FREE = 0,
    VOL_ROTATION_START_TURNING,
    VOL_ROTATION_IN_TURNING,
    VOL_ROTATION_UP_HOLD,
    VOL_ROTATION_UP_RELEASE,
    VOL_ROTATION_DOWN_HOLD,
    VOL_ROTATION_DOWN_RELEASE,
}eVolRotationState;
static eVolRotationState volRotationSta= VOL_ROTATION_FREE;

const uint8 VolTable[CUST_MAX_VOL+1] =
{
    0, 20, 30, 35, 40, 43, 46, 49, 52, 55,
    59, 61, 63, 65, 67, 69, 71, 73, 75, 77,
    78, 79, 80, 81, 82, 83, 84, 85, 86, 87,
    88, 89, 90
};

/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static uint8 MainApp_AseTkVolToCustVol(uint8 aseTkVol)
{
    uint8 i;
    for (i = MIN_VOLUME; i < CUST_MAX_VOL; i++)
    {
        if ((aseTkVol >= VolTable[i]) && (aseTkVol < VolTable[i+1]))
        {
            break;
        }
    }
    return i;
}

static uint8 MainApp_VolumeDownOneStep(cMainApp * const me)
{
    uint8 custVol= 0;

    if (me->absoluteVol == INVALID_VALUE)
    {
        me->absoluteVol = *(uint8*)Setting_Get(SETID_VOLUME);
    }

    if(me->absoluteVol <= MIN_VOLUME)
    {
        TP_PRINTF("VOLUME_DOWN_KEY, already reached minimum volume absoluteVol= %d\r\n", me->absoluteVol);
    }
    else
    {
        custVol = MainApp_AseTkVolToCustVol(me->absoluteVol);
        if (me->absoluteVol <= VolTable[custVol])
        {
            me->absoluteVol = VolTable[custVol-1];
        }
        else
        {
            me->absoluteVol = VolTable[custVol];
        }
        //me->isVolChanged = TRUE;
        TP_PRINTF("VOLUME_DOWN_KEY, absoluteVol= %d\r\n", me->absoluteVol);
        MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
    }
    ASSERT(me->absoluteVol>=MIN_VOLUME && me->absoluteVol<=MAX_VOLUME);
    AudioSrv_SetVolume(me->absoluteVol);    
}

static uint8 MainApp_VolumeUpOneStep(cMainApp * const me)
{
    uint8 custVol= 0;

    if (me->absoluteVol == INVALID_VALUE)
    {
        me->absoluteVol = *(uint8*)Setting_Get(SETID_VOLUME);
    }

    if(me->absoluteVol >= MAX_VOLUME)
    {
        TP_PRINTF("VOLUME_UP_KEY, already reached maximun volume absoluteVol= %d\r\n", me->absoluteVol);
    }
    else
    {
        custVol = MainApp_AseTkVolToCustVol(me->absoluteVol);
        me->absoluteVol = VolTable[custVol+1];
        //me->isVolChanged = TRUE;
        TP_PRINTF("VOLUME_UP_KEY, absoluteVol= %d\r\n", me->absoluteVol);
        MainApp_SendLedReq(me, LED_IND_ID_SHORT_PRESS);
    }
    ASSERT(me->absoluteVol>=MIN_VOLUME && me->absoluteVol<=MAX_VOLUME);
    AudioSrv_SetVolume(me->absoluteVol);
}

static QState MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();

    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            TP_PRINTF("\nDOWN\n");
            //if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            TP_PRINTF("\nUP\n");
            //if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeUpOneStep(me);
            }
            break;
        }
        case CONFIG_KEY:
        {
            TP_PRINTF("\nCONFIG\n");
            bool enableDspTuning = TRUE;
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, enableDspTuning, 0);
            break;
        }
        default:break;
    }

    return ret;
}

static QState MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    QState ret = Q_UNHANDLED();

    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            printf("\nDOWN\n");
            //if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeDownOneStep(me);
            }
            break;
        }
        case VOLUME_UP_KEY:
        {
            printf("\nUP\n");
            //if (me->systemStatus == SYSTEM_STA_ON || me->systemStatus == SYSTEM_STA_IDLE)
            {
                MainApp_VolumeUpOneStep(me);
            }
            break;
        }
        case CONFIG_KEY:
        {
            printf("\nCONFIG\n");
            bool enableDspTuning = TRUE;
            AudioSrv_SetAudio(DSP_TUNNING_SETT_ID, enableDspTuning, 0);
            break;
        }
        default:break;
    }

    return ret;
}

QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_REPEAT:
            ret = MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            ret = MainApp_KeySPressEvtAction(me, evt);
            break;
        default:break;
    }

    //MainApp_CombineKeyHandler(me, e, me->combinedKey);
    return ret;
}

