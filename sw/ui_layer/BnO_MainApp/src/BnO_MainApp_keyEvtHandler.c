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
#include "BnO_MainApp.h"

/*****************************************************************
 * Global Variable
 *****************************************************************/
extern KeyEvtToFepAseCommand keyEvtToFepAseCommandList[];


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

static void BnO_MainApp_KeyUpEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void BnO_MainApp_KeyDownEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void BnO_MainApp_KeySPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            break;
        }
        case VOLUME_UP_KEY:
        {
            break;
        }
        case POWER_KEY:
        {
            break;
        }
        case CONFIG_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}
static void BnO_MainApp_KeyLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_IR_KEY:
        {
            break;
        }
        case POWER_KEY:
        {
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void BnO_MainApp_KeyVLPressEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        /*if necessary, add the handler for keys here */
        default:break;
    }
}
static void BnO_MainApp_KeyHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            break;
        }
        case CONFIG_KEY:
        {
            break;
        }
        /*if necessary, add the handler for keys here */
        default:break;
    }
}

static void BnO_MainApp_KeyRepeatEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case VOLUME_DOWN_KEY:
        {
            break;
        }
        case VOLUME_UP_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void BnO_MainApp_KeyVeryLHoldEvtAction(cMainApp * const me, KeyStateEvt const * const e)
{
    switch(e->keyId)
    {
        case POWER_KEY:
        {
            break;
        }
        /*if necessary, add the handler for more keys here */
        default:break;
    }
}

static void BnO_MainApp_CombineKeyHandler(cMainApp * const me, QEvt const * const e, eCombinedKey comKey)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(comKey)
    {
        case COM_KEY_STANDBY:
        {
            break;
        }
        case COM_KEY_FACTORY_RESET:
        {
            me->combinedKey = COM_KEY_INVALID;
            BnO_MainApp_SendLedReq(me, LED_IND_ID_RST_IN_PROG);
            NvmDrv_EraseAll(NULL);
            NVM_STORAGE_VALUE_SET(NVM_STORAGE_ADDR_IGNORE_PWR_KEY);
            me->nextState = (QStateHandler*)&MainApp_SoftReset;
            break;
        }
        default:break;
    }
}

void BnO_MainApp_AseTkKeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    KeyEvtToFepAseCommand *p = MainApp_GetFepAseCommandList();
    uint8 i = 0;
    for(i = 0; i < MainApp_GetFepAseCommandListSize(); i++)
    {
        if((evt->keyId == p[i].keyId) && (evt->keyEvent == p[i].keyEvent))
        {
            AseTkSrv_SendFepAseCmd(p[i].command);
            break;
        }
    }
}


void BnO_MainApp_KeyHandler(cMainApp * const me, QEvt const * const e)
{
    KeyStateEvt *evt = (KeyStateEvt*)e;
    switch(evt->keyEvent)
    {
        case KEY_EVT_UP:
            BnO_MainApp_KeyUpEvtAction(me, evt);
            break;
        case KEY_EVT_DOWN:
            BnO_MainApp_KeyDownEvtAction(me, evt);
            break;
        case KEY_EVT_SHORT_PRESS:
            BnO_MainApp_KeySPressEvtAction(me, evt);
            break;
        case KEY_EVT_LONG_PRESS:
            BnO_MainApp_KeyLPressEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_PRESS:
            BnO_MainApp_KeyVLPressEvtAction(me, evt);
            break;
        case KEY_EVT_HOLD:
            BnO_MainApp_KeyHoldEvtAction(me, evt);
            break;
        case KEY_EVT_REPEAT:
            BnO_MainApp_KeyRepeatEvtAction(me, evt);
            break;
        case KEY_EVT_VERY_LONG_HOLD:
            BnO_MainApp_KeyVeryLHoldEvtAction(me, evt);
            break;
        default:break;
    }

    BnO_MainApp_CombineKeyHandler(me, e, me->combinedKey);
}

