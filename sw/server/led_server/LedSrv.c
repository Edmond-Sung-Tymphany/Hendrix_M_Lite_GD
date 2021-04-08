/**
*  @file      LedSrv.c
*  @brief     Source file for led Server class
*  @author    Johnny Fan
*  @date      15-Feb-2014
*  @copyright Tymphany Ltd.
*  @Note      State Machine:
 * QState LedSrv_ForeGroundShowing: Shows the fore ground LED pattern if there's
   QState LedSrv_BackGround: super state of ForeGround state, it shows the background state,
 *                           and excuse the command
*/
/*
Change History:
VERSION    : 2               2017-09-06     Alex Li
DESCRIPTION: Change LedSrv_PublishState's behavior to reduce LED event publish times
SCO/ERROR  :
*/

#include "bsp.h"
#include "trace.h"

#include "LedSrv_priv.h"
#include "controller.h"
#include "LedDrv.h"

#ifdef LED_HAS_IOEXPANDER
#include "IoExpanderLedDrv.h"
#endif
#ifdef LED_HAS_PWM
#include "PwmLedDrv.h"
#endif

#include "LedSrv.config"

#define CAST_ME cLedSrv * ledSrv = (cLedSrv *) me;

/* local variables*/
static QEvt const * LedSrvQueueSto[LED_SRV_EVENT_Q_SIZE];     /**< Led Server Event queue */

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
/* @brief       Led Server Startup process, register to controller and invoke by system
 * @param[in]   me          pointer to the base object of the Led Server object
 */
void LedSrv_StartUp(cPersistantObj *me)
{
    /* Add to QSPY directory - only does so if QSPY enabled.
    * Remember to make sure these items exist
    */
    QS_OBJ_DICTIONARY(ledSrv);
    QS_OBJ_DICTIONARY(LedSrv_Active);
    QS_OBJ_DICTIONARY(LedSrv_DeActive);

    QS_SIG_DICTIONARY(LED_REQ_SIG,ledSrv);
    QS_SIG_DICTIONARY(LED_RESP_SIG,ledSrv);
    QS_SIG_DICTIONARY(LED_TIMEOUT_SIG,ledSrv);
    
    /* start up the object and let it run. including the timer*/
    Server_Ctor((cServer*)me, Q_STATE_CAST(&LedSrv_Initial), LED_TIMEOUT_SIG,
                         LedSrvQueueSto, Q_DIM(LedSrvQueueSto), LED_SRV_ID);
}

/* @brief       Led Server Shut Down process, register to controller and invoke by system
 * @param[in]   me          pointer to the base object of the Led Server object
 */
void LedSrv_ShutDown(cPersistantObj *me)
{
    cLedSrv *m = (cLedSrv*) me;
    uint8 i = 0;
    for (i = 0; i < LED_MAX; i++)
    {
        LedDrv_Xtor((cLedDrv*)m->ledDrvList[i]);
    }
    Server_Xtor((cServer*)me);
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/

/* @brief       Led Server Initial state handler
 * @param[in]   me          pointer to Led Server object
 * @param[in]   e           N/A
 */
QState LedSrv_Initial(cLedSrv * const me, QEvt const * const e)
{
     /* suppress the compiler warning about unused parameter */
    (void)e;

    me->pRequestor      = NULL;
    return Q_TRAN(&LedSrv_DeActive);
}

/* @brief       Led Server Active state handler
 * @param[in]   me          pointer to Led Server object
 * @param[in]   e           Event received
 */
QState LedSrv_Active(cLedSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        LedSrv_CtorAllLedObject(me);
        PersistantObj_RefreshTick((cPersistantObj*)me, LED_TICK_MS);
        return Q_HANDLED();
    }
    case SYSTEM_ACTIVE_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
        return Q_HANDLED();
    }
#ifdef LED_SRV_HAS_SLEEP
    case SYSTEM_SLEEP_REQ_SIG:
    {
        LedSrv_XtorAllLedObject(me);
        CommonReqEvt* reqEvt = (CommonReqEvt*) e;
        me->pRequestor = reqEvt->sender;
        CommonEvtResp((QActive*)me, me->pRequestor, RET_SUCCESS, SYSTEM_SLEEP_RESP_SIG);
        me->pRequestor = NULL;
        return Q_TRAN(LedSrv_DeActive);
    }
#endif
    case LED_REQ_SIG:
    {
        LedSrv_HandleReq(me, (LedReqEvt*)e);
        return Q_HANDLED();
    }
    case LED_TIMEOUT_SIG:
    {
        eLed i = LED_MIN;
        ledMask mask = 0;
        ePattern pattId = PAT_MAX_NUMBER;
        ePattern lastPattId = PAT_MAX_NUMBER;
#ifdef LEDSRV_GROUP_ENABLE
        LEDGROUP_s           *dr_ptr = &me->group_s;
        LedReqGroupEvt    *userReq_ptr = &dr_ptr->userReq;

        if (me->grp_enabled)
        {
            if (LedSrvGroup_Show(me))
            {
                LedSrvGroup_Disable(me);
                LedSrv_PublishState(me, 0, PATTERN_GROUP);
            }
        }
#endif
        for ( ; i < LED_MAX; i++)
        {
#ifdef LEDSRV_GROUP_ENABLE
            /* these pin are take over by group mode */
            if (me->grp_enabled && (userReq_ptr->leds&(1 << i)))
                continue;
#endif
            /* show the data */
            cLedDrv *p = (cLedDrv*)me->ledDrvList[i];
            lastPattId = pattId;
            pattId = LedDrv_PattShow(p);
            //publish state when pattern changed
            if (pattId != PAT_MAX_NUMBER)
            {
                if(pattId == lastPattId)
                {
                    mask |= GET_LED_MASK(i);
                }
                else if(mask != 0)
                {
                   LedSrv_PublishState(me, mask, pattId);
                   mask = 0;
                   mask |= GET_LED_MASK(i);
                }
                else
                {
                   mask |= GET_LED_MASK(i);
                }
            }
        }
        if(mask != 0)
        {
            LedSrv_PublishState(me, mask, pattId);
            mask = 0;
        }
        PersistantObj_RefreshTick((cPersistantObj*)me, LED_TICK_MS);
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

/* @brief       Led Server Deactive state handler
 * @param[in]   me          pointer to Led Server object
 * @param[in]   e           Event received
 */
QState LedSrv_DeActive(cLedSrv * const me, QEvt const * const e)
{
    switch (e->sig)
    {
    case Q_ENTRY_SIG:
    {
        return Q_HANDLED();
    }
    case SYSTEM_ACTIVE_REQ_SIG:
    {
        CommonReqEvt* pReq = (CommonReqEvt*) e;
        CommonEvtResp((QActive*)me, pReq->sender, RET_SUCCESS, SYSTEM_ACTIVE_RESP_SIG);
        return Q_TRAN(&LedSrv_Active);
    }


    /* Normal we ensure LedSrv Active then send request.
     * If get request on DeActive, means system have potention problems
     */
    case LED_REQ_SIG:
    {
#ifndef HAS_IOEXPANDER_LED_TEST
        ASSERT(0);
#endif
        return Q_HANDLED();
    }

    default:
        break;
    }
    return Q_SUPER(&QHsm_top);
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/

/* @brief       Prepare event to Led server of the request turn Led(s) ON with color (brightness)
 * @param[in]   sender      the server that send signal to Led server
 * @param[in]   leds        Led bit-mask (set the corresponding bit of LED for taking effect)
 * @param[in]   c           the color (brightness) to be set
 */
void LedSrv_SetEvtOn(QActive* sender, ledMask leds, Color c)
{
    LedReqEvt* req  = Q_NEW(LedReqEvt, LED_REQ_SIG);

    req->sender     = sender;
    req->ledCommand = LED_PURE_ON_CMD;
    req->leds       = leds;
    req->color      = c;
    SendToServer(LED_SRV_ID, (QEvt*)req);
}

/* @brief       Prepare event to Led server of the request turn Led(s) OFF
 * @param[in]   sender      the server that send signal to Led server
 * @param[in]   leds        Led bit-mask (set the corresponding bit of LED for taking effect)
 */
void LedSrv_SetEvtOff(QActive* sender, ledMask leds)
{
    LedReqEvt* req  = Q_NEW(LedReqEvt, LED_REQ_SIG);

    req->sender     = sender;
    req->ledCommand = LED_OFF_CMD;
    req->leds       = leds;
    SendToServer(LED_SRV_ID, (QEvt*)req);
}

/* @brief       Send event to Led server to request Led to display pattern
 * @param[in]   sender      the server that send signal to Led server
 * @param[in]   leds        Led bit-mask (set the corresponding bit of LED for taking effect)
 * @param[in]   patternId   Pattern ID to be set
 */
void LedSrv_SetPatt(QActive* sender, ledMask leds, ePattern patternId)
{
    LedReqEvt* req  = Q_NEW(LedReqEvt, LED_REQ_SIG);

    req->sender     = sender;
    req->ledCommand = LED_PAT_ON_CMD;
    req->leds       = leds;
    req->patternId  = patternId;
    SendToServer(LED_SRV_ID, (QEvt*)req);
}
#ifdef HAS_LED_STRIP
void LedSrv_SetStripPatt(QActive* sender, eStripPattern patternId)
{
    LedReqEvt* req  = Q_NEW(LedReqEvt, LED_REQ_SIG);
    req->sender     = sender;
    req->ledCommand = LED_PAT_STRIP_CMD;
    req->stripPatternId = patternId;
    SendToServer(LED_SRV_ID, (QEvt*)req);
}
#endif

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

/* @brief       Create all Led Driver objects in list
 * @param[in]   me          pointer to Led Server object
 */
static void LedSrv_CtorAllLedObject(cLedSrv * const me)
{
    tIoeLedDevice *pLedDevice;
    uint16 attached_device_index = 0;
    /*static */uint8 ledsAdded = 0;

    uint8 ii;

    for (ii = 0; ii < IO_EXPANDER_NUM;ii++)
    {
        pLedDevice = (tIoeLedDevice*) getDevicebyId(LED_DEV_ID, &attached_device_index);
        ASSERT(pLedDevice);
        {
            uint8 j = 0;
            for ( ; j < pLedDevice->ledNum; j++)
            {
                uint8 id;

                // associate the driver with the correct sub-class
                switch(pLedDevice->deviceInfo.deviceType)
                {
#ifdef LED_HAS_IOEXPANDER
                case IO_EXPANDER_DEV_TYPE:
                {
                    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice *)pLedDevice;

                    id = pIoeLedConfig->pIoExpanderLedMap[j].ledID;
                    //TODO: Avoid using duplicated I2C driver
                    IoExpanderLedDrv_Ctor(&ioExpanderLedDrvList[ledsAdded + j], (const tDevice*)pLedDevice, &ioExpanderDrv[ii], j);

                    me->ledDrvList[id] = (cLedDrv*)&ioExpanderLedDrvList[ledsAdded + j];
                    break;
                }
#endif
#ifdef LED_HAS_PWM
                case PWM_DEV_TYPE:
                {
                    tPwmLedDevice *pPwmLedConfig = (tPwmLedDevice *)pLedDevice;

                    id = pPwmLedConfig->pPwmLedMap[j].ledID;
                    PwmLedDrv_Ctor(&pwmLedDrvList[j], (const tDevice*)pLedDevice, j);
                    me->ledDrvList[id] = (cLedDrv*)&pwmLedDrvList[j];
                    break;
                }
#endif
                default:
                    ASSERT(0);
                    break;
                }

                //LedDrv_Ctor(me->ledDrvList[id], (const tDevice*)pLedDevice);
            }
            ledsAdded += pLedDevice->ledNum;
            // Find next device with LED_ID
            ++attached_device_index;
            pLedDevice = (tIoeLedDevice*) getDevicebyId(LED_DEV_ID, &attached_device_index);
        }
    }
}

#ifdef LED_SRV_HAS_SLEEP
/* @brief       Destroy all Led Driver objects in list
 * @param[in]   me          pointer to Led Server object
 */
static void LedSrv_XtorAllLedObject(cLedSrv * const me)
{
    uint8 i;

    /* xtor the LED driver object */
    for (i = 0; i < LED_MAX; i++)
    {
        LedDrv_Xtor(me->ledDrvList[i]);
    }
}
#endif
/* @brief       Handler of the request event
 * @param[in]   me          pointer to Led driver object
 * @param[in]   req         the request event
 */
static void LedSrv_HandleReq(cLedSrv *me, LedReqEvt* req)
{
    /* check if LED ID is legal */
#ifdef LED_SRV_HAS_RESPONSE
    if (!LedSrv_LedIdIsValid(req->leds))
    {
        LedSrv_Resp(req->sender, RET_FAIL);
        return;
    }
#endif

    switch (req->ledCommand)
    {
    case LED_PURE_ON_CMD:
    {
        eLed i = LED_MIN;
        /* Turn On the corresponding LED */
        for ( ; i < LED_MAX; i++ )
        {
            ledMask m = GET_LED_MASK(i);
            if ((req->leds) & m)
            {
                cLedDrv *p = me->ledDrvList[i];
                /*stop the pattern*/
                LedDrv_PattStop(p);
                p->pLedFunc->LedSetColor(p, req->color);
            }
        }
#ifdef LED_SRV_HAS_RESPONSE
        LedSrv_Resp(req->sender, RET_SUCCESS);
#endif
        break;
    }
    case LED_OFF_CMD:
    {
        eLed i = LED_MIN;
        /* Turn Off the corresponding LED */
        for ( ; i < LED_MAX; i++ )
        {
            ledMask m = GET_LED_MASK(i);
            if (req->leds & m)
            {
                cLedDrv *p = me->ledDrvList[i];
                /*stop the pattern*/
                LedDrv_PattStop(p);
                p->pLedFunc->LedOff(p);
            }
        }
#ifdef LED_SRV_HAS_RESPONSE
        LedSrv_Resp(req->sender, RET_SUCCESS);
#endif
        break;
    }
    case LED_PAT_ON_CMD:
    {
        ePattern patt = req->patternId;

#ifdef LED_SRV_HAS_RESPONSE
        if (patt >= PAT_MAX_NUMBER)
        {   /* check if LED pattern ID is legal */
            LedSrv_Resp(req->sender, RET_NOT_SUPPORTED);
            return;
        }
#endif
#ifdef LEDSRV_GROUP_ENABLE
        LEDGROUP_s   *dr_ptr = &me->group_s;
        if (patt == PATTERN_GROUP)
        {
            LedSrvGroup_Enable(me, req);
            PersistantObj_RefreshTick((cPersistantObj*)me, LED_TICK_MS);
        }
        else if (me->grp_enabled)
        {
            /* if any pin is overlap with others, turn off DYNAMIC mode */
            if (dr_ptr->userReq.leds & req->leds)
                LedSrvGroup_Disable(me);
        }
#endif
        eLed i = LED_MIN;
        for ( ; i < LED_MAX; i++ )
        {
            ledMask m = GET_LED_MASK(i);
            
#ifdef LEDSRV_GROUP_ENABLE
            /* skip if this is NR pin */
            if (me->grp_enabled && (dr_ptr->userReq.leds&m))
                continue;
#endif
            if (req->leds & m)
            {
                cLedDrv *p = me->ledDrvList[i];
                LedDrv_PattSet(p, patt);
                /* refresh tick and show pattern in timeout event*/
                /* Do not show pattern here as the time data is not refresh yet*/
                PersistantObj_RefreshTick((cPersistantObj*)me, LED_TICK_MS);
            }
        }
#ifdef LED_SRV_HAS_RESPONSE
        LedSrv_Resp(req->sender, RET_SUCCESS);
#endif
        break;
    }
    default:
    {
        /*for other command, do not support for now*/
#ifdef LED_SRV_HAS_RESPONSE
        LedSrv_Resp(req->sender, RET_NOT_SUPPORTED);
#endif
        break;
    }
    }
}

#ifdef LED_SRV_HAS_RESPONSE
/* @brief       Send response signal to the sender
 * @param[in]   sender      the server that send signal to Led server
 * @param[in]   result      the response result
 */
static void LedSrv_Resp(QActive * sender, eEvtReturn result)
{
    if (!sender)
    {
        return;
    }
    LedRespEvt* resp = Q_NEW(LedRespEvt, LED_RESP_SIG);
    resp->result = result;
    QACTIVE_POST(sender, (QEvt*)resp, 0);
}

/* @brief       Check if the Leds mask is valid
 * @param[in]   leds        Led mask
 */
static bool LedSrv_LedIdIsValid(uint32 leds)
{
    ledMask m = GET_LED_MASK(LED_MAX);
    m = m - 1;
    bool ret = FALSE;

    if ( leds == ALL_LED)
    {
        ret = TRUE;
    }
    else if ( (leds & m)     /**< Have LED defined in the valid range */
        || !(leds & ~m) /**< Have no LED defined in the invalid range */
        )
    {
        ret = TRUE;
    }
    return ret;
}
#endif

/* @brief       Publish the state that a pattern is finished
 * @param[in]   me          pointer to Led driver object
 * @param[in]   mask        Led bit-mask (set the corresponding bit of LED for taking effect)
 * @param[in]   pattId      pattern ID of the finished pattern
 */
static void LedSrv_PublishState(cLedSrv *me, ledMask mask, ePattern pattId)
{
    LedStateEvt *pe = Q_NEW(LedStateEvt, LED_STATE_SIG);
    pe->mask       = mask;
    pe->patternId   = pattId;
    QF_PUBLISH(&pe->super, NULL);
}

