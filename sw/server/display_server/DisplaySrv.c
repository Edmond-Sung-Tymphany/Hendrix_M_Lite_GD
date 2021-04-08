/**
*  @file      DisplaySrv.c
*  @brief     Source file for Display Server class
*  @author    Edmond sung, Bob.Xu
*  @date      18-Apr-2014
*  @copyright Tymphany Ltd.
*  @Note      State Machine:
*  QState DisplaySrv_PreActive: wait for the Power Server Active state
   QState DisplaySrv_ForeGroundShowing: Shows the fore ground Display
   QState DisplaySrv_BackGround: super state of ForeGround state, it shows
   the background state and excuse the command
*/
#include "DisplaySrv_priv.h"
#include "DisplayDrv.h"
#include "product.config"
#include "bsp.h"
#include "trace.h"
#include "string.h"
#include "controller.h"
#include "DisplaySrv.Config"
//Q_DEFINE_THIS_FILE

static cDisplayDrv displayDrvObj;
static uint8 brightnessLevel = DEFAULT_BRIGHTNESS_LEVEL;
static QState DisplaySrv_Initial(cDisplaySrv * const me, QEvt const * const e);
static QState DisplaySrv_Active(cDisplaySrv * const me, QEvt const * const e);
static void DisplaySrv_DispatchReq(DisplayReqEvt* req);
#ifdef HAS_SCREEN_DIM_CTRL
static void DisplaySrv_DimCtrl(cDisplaySrv * const me);
#endif
static void DisplaySrv_DisplayPatternCtrl(cDisplaySrv * const me);

/* internal signals */
enum InternalSignals
{
    TIMEOUT_SIG = MAX_SIG,
};

/* local variables*/
#define CAST_ME cDisplaySrv* DisplaySrv = (cDisplaySrv*) me;

static QEvt const * DisplaySrvQueueSto[DISPLAY_SRV_EVENT_Q_SIZE];
const uint16 DISPLAY_SRV_TICK_PER_SEC = BSP_TICKS_PER_SEC ;
const uint16 DISPLAY_SRV_TICK_TIME_MS = 1000 / BSP_TICKS_PER_SEC ;

#ifdef HAS_SCREEN_DIM_CTRL
static int32 dimLightTick = 0;
#endif

static uint8 const *previousString = NULL;

static tPattDisplayCtrl pattDisplayCtrl =
{
    .pString = NULL,
    .patternStart = FALSE,
};

void DisplaySrv_StartUp(cPersistantObj *me)
{
    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&DisplaySrv_Initial), TIMEOUT_SIG,
                   DisplaySrvQueueSto, Q_DIM(DisplaySrvQueueSto), DISPLAY_SRV_ID);
    /* Subscribe */
    QActive_subscribe((QActive*)me, KEY_STATE_SIG);
    QActive_subscribe((QActive*)me, SETTING_UPDATE_SIG);
}

void DisplaySrv_ShutDown(cPersistantObj *me)
{
    /* zero all memory that resets an AObject*/
    Server_Xtor((cServer*)me);
}

static QState DisplaySrv_Initial(cDisplaySrv * const me, QEvt const * const e)
{
    (void)e; /* suppress the compiler warning about unused parameter */
    me->displaySrvConf = &displaySrvConf;
    return Q_TRAN(&DisplaySrv_Active);
}

/* it is the state for Display working */
static QState DisplaySrv_Active(cDisplaySrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            DisplayDrv_Ctor(&displayDrvObj);
            DisplayDrv_CleanScrean(&displayDrvObj);
            DisplayDrv_SetBrightness(&displayDrvObj,brightnessLevel);
            QTimeEvt_armX(TIME_EVT_OF(me), 1000, 0);
            return Q_HANDLED();
        }
        case TIMEOUT_SIG:
        {
#ifdef HAS_SCREEN_DIM_CTRL
            DisplaySrv_DimCtrl(me);
#endif
            DisplaySrv_DisplayPatternCtrl(me);            
            return Q_HANDLED();
        }
#ifdef HAS_SCREEN_DIM_CTRL
        case DISPLAY_SET_DIM_TIME_SIG:
        {
            DisplaySetDimTimeEvt *pte = (DisplaySetDimTimeEvt*)e;
            me->displaySrvConf->dimEnabled = pte->enable;
            me->displaySrvConf->dimTimeThreshold = pte->dimTime;
            return Q_HANDLED();
        }
#endif
        case DISPLAY_REQ_SIG:
        {
            DisplaySrv_DispatchReq((DisplayReqEvt*)e);
            PersistantObj_RefreshTick((cPersistantObj*)me, DISPLAY_SRV_TICK_TIME);
            return Q_HANDLED();
        }
        case DISPLAY_DEBUG_REQ_SIG:
        {
            DisplayDebugReqEvt *pEvt = (DisplayDebugReqEvt*)e;
            DisplayDrv_SetDebugType(&displayDrvObj,pEvt->digitId,pEvt->segmentId);
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_REQ_SIG:
        {
            CommonReqEvt* pReq = (CommonReqEvt*)e;
            CommonEvtResp((QActive*)me, pReq->sender,RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
            return Q_HANDLED();
        }
        case DISPLAY_SET_BRIGHTNESS_SIG:
        {
            DisplayBrightnessEvt *pReq = (DisplayBrightnessEvt*)e;
            brightnessLevel = pReq->brightnessLevel;
            DisplayDrv_SetBrightness(&displayDrvObj,pReq->brightnessLevel);
            return Q_HANDLED();
        }
        case SETTING_UPDATE_SIG:
        case KEY_STATE_SIG:
        {
#ifdef HAS_SCREEN_DIM_CTRL
            /* Any activity will wake up screen from dim light */
            DisplayDrv_SetBrightness(&displayDrvObj, brightnessLevel);
            dimLightTick = 0;
#endif
            return Q_HANDLED();
        }
        case Q_EXIT_SIG:
        {
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

void DisplaySrv_SendString(uint8 const *pString)
{
    DisplayReqEvt *pte = Q_NEW(DisplayReqEvt,DISPLAY_REQ_SIG);
    previousString = pString;
    pte->displayString = pString;
    pte->cleanScreen = FALSE;
    SendToServer(DISPLAY_SRV_ID, (QEvt*)pte);
}

void DispalySrv_SetBrightnessLevel(uint16 brightness)
{
    DisplayBrightnessEvt * pBrightnessReq = Q_NEW(DisplayBrightnessEvt,DISPLAY_SET_BRIGHTNESS_SIG);
    pBrightnessReq->brightnessLevel = brightness;
    SendToServer(DISPLAY_SRV_ID,(QEvt*)pBrightnessReq);
}


void DisplaySrv_CleanScreen()
{
    DisplayReqEvt *pte = Q_NEW(DisplayReqEvt,DISPLAY_REQ_SIG);
    /* 
     * when pte->cleanScreen is assigned TRUE, display server will simply ignore 
     * pte->displayString, so it is not necessary to assign any string to pte->displayString
    */
    pte->cleanScreen = TRUE;
    SendToServer(DISPLAY_SRV_ID, (QEvt*)pte);
}

void DisplaySrv_ResumeScreen()
{
    DisplaySrv_SendString(previousString);
}
#ifdef HAS_SCREEN_DIM_CTRL
void DisplaySrv_SetDimTime(uint32 dimTime, BOOL enable)
{
    DisplaySetDimTimeEvt *pte = Q_NEW(DisplaySetDimTimeEvt,DISPLAY_SET_DIM_TIME_SIG);
    pte->dimTime = dimTime;
    pte->enable  = enable;
    SendToServer(DISPLAY_SRV_ID, (QEvt*)pte);
}
#endif
/******************************************************************************
 *
 * private functions
 *
 ******************************************************************************/

static void DisplaySrv_DispatchReq(DisplayReqEvt* req)
{
    uint16 strLength;
    if(req->cleanScreen)
    {
        DisplayDrv_CleanScrean(&displayDrvObj);
        return;
    }
    strLength = strlen((const char *)req->displayString);
    if(strLength <= NUM_OF_SCREEN_DIGIT+1)
    {
        pattDisplayCtrl.patternStart = FALSE;
        DisplayDrv_SendString(&displayDrvObj, req->displayString);
    }
    else
    {
        pattDisplayCtrl.patternStart = TRUE;
        pattDisplayCtrl.pString = req->displayString;
        pattDisplayCtrl.pStringHead = req->displayString;
        DisplayDrv_SendString(&displayDrvObj, pattDisplayCtrl.pString);
    }
}
#ifdef HAS_SCREEN_DIM_CTRL
static void DisplaySrv_DimCtrl(cDisplaySrv * const me)
{
    if(me->displaySrvConf->dimEnabled && me->displaySrvConf->dimTimeThreshold != 0)
    {
        dimLightTick += DISPLAY_SRV_TICK_TIME;
        if(dimLightTick >= GET_TICKS_IN_MS(me->displaySrvConf->dimTimeThreshold * 1000))
        {
            DisplayDrv_SetBrightness(&displayDrvObj,DIM_BRIGHTNESS_LEVEL);
            dimLightTick = 0;
        }
    }
    else
    {
        dimLightTick = 0;
    }
    PersistantObj_RefreshTick((cPersistantObj*)me, DISPLAY_SRV_TICK_TIME);
}
#endif

static void DisplaySrv_PatternScrollLeft(cDisplaySrv * const me)
{
    if(pattDisplayCtrl.currentTicks >= GET_TICKS_IN_MS((me->displaySrvConf->frameInterval)/DISPLAY_SRV_TICK_TIME))
    {
        uint16 strLength;
        memset(pattDisplayCtrl.strTempBuff,' ',sizeof(uint8)*NUM_OF_SCREEN_DIGIT);
        strLength = strlen((const char *)pattDisplayCtrl.pString);
        if(strLength >= NUM_OF_SCREEN_DIGIT)
        {
            DisplayDrv_SendString(&displayDrvObj, pattDisplayCtrl.pString);
            pattDisplayCtrl.pString++;
        }
        else if(NULL != *(pattDisplayCtrl.pString))
        {
            strcpy((char *)pattDisplayCtrl.strTempBuff,(char *)pattDisplayCtrl.pString);
            DisplayDrv_SendString(&displayDrvObj, pattDisplayCtrl.strTempBuff);
            pattDisplayCtrl.pString++;
        }
        else
        {
            DisplayDrv_SendString(&displayDrvObj, pattDisplayCtrl.strTempBuff);
            pattDisplayCtrl.pString = pattDisplayCtrl.pStringHead;
        }
        pattDisplayCtrl.currentTicks = RESET_TICK;
    }
}

static void DisplaySrv_DisplayPatternCtrl(cDisplaySrv * const me)
{
    if(pattDisplayCtrl.patternStart)
    {
        pattDisplayCtrl.currentTicks++;
    }
    else
    {
        pattDisplayCtrl.currentTicks = RESET_TICK;
    }
    switch(me->displaySrvConf->displayStyle)
    {
      case DISPLAY_STYLE_SCROLL_LEFT:
          DisplaySrv_PatternScrollLeft(me);
          break;
      case DISPLAY_STYLE_SCROLL_RIGHT:
          break;
      case DISPLAY_STYLE_SCROLL_UP:
          break;
      case DISPLAY_STYLE_SCROLL_DOWN:
          break;
      default:break;
    }

}