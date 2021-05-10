/**
*  @file      controller.c
*  @brief     Source file for controller object. Helps control access to resources. Kicks off TP and handles general signals.
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*/
#include "controller_priv.h"
#include "controller.h"
#include "trace.h"
#include "signals.h"


/* QP event pools  */
#if (NUM_OF_SMALL_EVENTS == 0 && NUM_OF_MEDIUM_EVENTS == 0 && NUM_OF_LARGE_EVENTS == 0)
    #error "No pool is defined. A product must have at least one pool for QP events"
#endif

static QSubscrList subscriptPool[MAX_PUB_SIG];

/* storage for event pools... */
#if (NUM_OF_SMALL_EVENTS != 0 && SIZE_OF_SMALL_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_SMALL_EVENTS];} smalltype;
static QF_MPOOL_EL(smalltype) smallEventPool[NUM_OF_SMALL_EVENTS];
#elif (NUM_OF_SMALL_EVENTS != SIZE_OF_SMALL_EVENTS)
  #error "Error, Both SIZE_OF_SMALL_EVENTS and NUM_OF_SMALL_EVENTS should be defined as 0"
#endif

#if (NUM_OF_MEDIUM_EVENTS != 0 && SIZE_OF_MEDIUM_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_MEDIUM_EVENTS];} medtype;
static QF_MPOOL_EL(medtype) mediumEventPool[NUM_OF_MEDIUM_EVENTS];
#elif (NUM_OF_MEDIUM_EVENTS != SIZE_OF_MEDIUM_EVENTS)
  #error "Error, Both SIZE_OF_MEDIUM_EVENTS and NUM_OF_MEDIUM_EVENTS should be defined as 0"
#endif

#if (NUM_OF_LARGE_EVENTS != 0 && SIZE_OF_LARGE_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_LARGE_EVENTS];} largetype;
static QF_MPOOL_EL(largetype) largeEventPool[NUM_OF_LARGE_EVENTS];
#elif (NUM_OF_LARGE_EVENTS != SIZE_OF_LARGE_EVENTS)
  #error "Error, Both SIZE_OF_LARGE_EVENTS and NUM_OF_LARGE_EVENTS should be defined as 0"
#endif

#define CONTROLLER_ID QF_MAX_ACTIVE /* Controller needs an ID which is away from all server/app/delegate ID */

/*****************************************************************************************************************
 *
 * Includes for servers
 *
 * Currently these are added for each server but this is not feasible for multiple projects/products
 * This WILL/MUST be replaced by a clever way to auto-generate and include file
 *
 *****************************************************************************************************************/

/** \brief structure to define a server entity
 * The ID is publicly exposed but the server is not.
 * This way you can send messages to the object without "knowing" it
*/
typedef struct
{
    const ePersistantObjID object_id;
    cPersistantObj * const obj;
    const uint16 modes;
} tObjectListEntity;


#include "persistantObj.h"
#include "./controller.config"



/* local functions*/
static void Controller_SwitchMode(uint16 modeId);
static void Controller_SendServerSwitchRequest(ePersistantObjID serverId, eSignal signal);
static bool Controller_IsServerRegisterMode(uint8 index, uint16 newModeID);


/* Private function declaration*/
static int16 getIndex(uint16 object_id, const tObjectListEntity * pObjectList, uint16 sizeOfList);

/* Internal event queue */
static QEvt const *eventQue[3];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[2];

/* local variables*/
static uint16 previousModeId;
static uint8 ExpectRespNumber;
#ifdef HAS_DELEGATES
uint32 freePriorityFlag = 0;
#endif


/*****************************************************************************************************************
 *
 * Ctor / Xtor functions
 *
 *
 *****************************************************************************************************************/
int16 Controller_Ctor(uint16 mode_id)
{
    /* initialize the framework and the underlying RT kernel */
    QF_init();

    /* object dictionaries */
    QS_OBJ_DICTIONARY(smallEventPool);
    QS_OBJ_DICTIONARY(mediumEventPool);
    QS_OBJ_DICTIONARY(largeEventPool);
    /* init publish-subscribe */
    QF_psInit(subscriptPool, Q_DIM(subscriptPool));

    /* initialize event pools... */
#if (NUM_OF_SMALL_EVENTS != 0 && SIZE_OF_SMALL_EVENTS != 0)
    QF_poolInit(smallEventPool, sizeof(smallEventPool), sizeof(smallEventPool[0]));
#endif

#if (NUM_OF_MEDIUM_EVENTS != 0 && SIZE_OF_MEDIUM_EVENTS != 0)
    QF_poolInit(mediumEventPool, sizeof(mediumEventPool), sizeof(mediumEventPool[0]));
#endif

#if (NUM_OF_LARGE_EVENTS != 0 && SIZE_OF_LARGE_EVENTS != 0)
    QF_poolInit(largeEventPool, sizeof(largeEventPool), sizeof(largeEventPool[0]));
#endif

#if defined(HAS_DEBUGSETT) || defined(HAS_SETTING)
    SettingSrv_InitDB();
#endif
    QActive_ctor(&Controller, Q_STATE_CAST(&Controller_Initial));
    QActive_start(&Controller, CONTROLLER_ID, eventQue, Q_DIM(eventQue), (void *)0, 0U, (QEvt *)0);
    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
    /* Zero expected responses */
    ExpectRespNumber = 0;
    /* Start up other entities and do more initialising */
    Controller_StartUp(mode_id);
    QS_OBJ_DICTIONARY(&Controller);
    
    /* This is a loop so it won't actually (or shouldn't) return */
    return (int16)QF_run();
}

void Controller_Xtor()
{
    QActive_stop((QActive*)&Controller);
    Controller_ShutDown();
    //Start up other entities and do more initialising
}

/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
QState Controller_Initial(QActive * const me)
{
    QS_FUN_DICTIONARY(&Controller_Initial);
    QS_FUN_DICTIONARY(&Controller_Idle);
    /* What does initial mode mean? */
    previousModeId = INITIAL_MODE; 
    return Q_TRAN(&Controller_Idle);
}

/*\brief Idle state 
 * Handles system mode request signal. 
 */
QState Controller_Idle(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            QActive_recall(me, &deferredReqQue);
            return Q_HANDLED();
        }          
        case SYSTEM_MODE_REQ_SIG:
        {
            SwitchModeReqEvt* pReq = (SwitchModeReqEvt*) e;       
            Controller_SwitchMode(pReq->modeId);
            previousModeId = pReq->modeId;
            if(ExpectRespNumber==0)
            {
                SwitchModeRespEvt* respEvt = Q_NEW(SwitchModeRespEvt, SYSTEM_MODE_RESP_SIG);
                respEvt->modeId= previousModeId;
                QACTIVE_POST((QActive*)&MainApp, (QEvt*)respEvt, 0);
                /*
                 * Check deferred Q before return, otherwise, requests may
                 * not be handled if the deferred Q is not empty
                 */
                QActive_recall(me, &deferredReqQue);
                return Q_HANDLED();
            }
            else
            {
                return Q_TRAN(Controller_SwitchingModeState);
            }
        }                   
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*\brief Switch mode counts off responses to sleep/active requests and then shuts down / wakes up the system
 * Handles system mode request signal. 
 */
QState Controller_SwitchingModeState(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case SYSTEM_MODE_REQ_SIG:
        {
            QActive_defer(me, &deferredReqQue, e);
            return Q_HANDLED();
        }
        case SYSTEM_SLEEP_RESP_SIG:
        {
            ExpectRespNumber--;
            if(ExpectRespNumber==0)
            {
                SwitchModeRespEvt* respEvt = Q_NEW(SwitchModeRespEvt, SYSTEM_MODE_RESP_SIG);
                respEvt->modeId= previousModeId;
                QACTIVE_POST((QActive*)&MainApp, (QEvt*)respEvt, 0);
                return Q_TRAN(Controller_Idle);
            }
            return Q_HANDLED();
        }
        case SYSTEM_ACTIVE_RESP_SIG:
        {
            ExpectRespNumber--;
            if(ExpectRespNumber==0)
            {
                SwitchModeRespEvt* respEvt = Q_NEW(SwitchModeRespEvt, SYSTEM_MODE_RESP_SIG);
                respEvt->modeId= previousModeId;
                QACTIVE_POST((QActive*)&MainApp, (QEvt*)respEvt, 0);
                return Q_TRAN(Controller_Idle);
            }
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*\brief switch mode function publishes sleep / wake request based on server modes 
 * 
 */
static void Controller_SwitchMode(uint16 modeId)
{
    uint8 i;    
    for(i=0;i<Q_DIM(serverList);i++)
    {
        if(Controller_IsServerRegisterMode(i,modeId)==FALSE)      
        {/*if the server doesn't support this mode, then deActive it*/
            if(Controller_IsServerRegisterMode(i,previousModeId)==TRUE)
            {
                Controller_SendServerSwitchRequest(serverList[i].object_id, SYSTEM_SLEEP_REQ_SIG);
                ExpectRespNumber++;         
            }                  
        }
        else
        {
            if(Controller_IsServerRegisterMode(i,previousModeId)==FALSE)
            {
                Controller_SendServerSwitchRequest(serverList[i].object_id, SYSTEM_ACTIVE_REQ_SIG);
                ExpectRespNumber++;  
            }  
        }                            
    }
}

/*\brief simple utility function to send the switch request signal
 * 
 */
static void Controller_SendServerSwitchRequest(ePersistantObjID serverId, eSignal signal)
{
    CommonReqEvt* reqEvt = Q_NEW(CommonReqEvt, signal);
    reqEvt->sender = &Controller;                    
    SendToServer(serverId,(QEvt*)reqEvt);
}

/*\brief simply check if the server is operable in the new mode
 * \a index of the server
 * \a new mode to check against
 * \r True if the server belongs to the new mode. False if not.
 */
static bool Controller_IsServerRegisterMode(uint8 index, uint16 newModeID)
{
    if((serverList[index].modes & newModeID)==0)
    {
        return FALSE;
    }
    else 
    {
        return TRUE;
    }
}

/*******************************************************************************
 *
 * public functions
 *
 ******************************************************************************/
 /*\brief Controller start up starts all persistent objects (based on their mode settings)
 * \a mode id for which the objects are started against 
 */
void Controller_StartUp(uint16 mode_id)
{
    uint16 i = 0;
    for (; i < Q_DIM(serverList); i++)
    {
        /* If it belongs to the group we may activate it */
        if(serverList[i].modes & mode_id)
        {
            ((cPersistantObj*)serverList[i].obj)->Startup((cPersistantObj*)serverList[i].obj);
        }
    }

    for (i = 0; i < Q_DIM(appList); i++)
    {
        /* If it belongs to the group we may activate it */
        if(appList[i].modes & mode_id)
        {
            ((cPersistantObj*)appList[i].obj)->Startup((cPersistantObj*)appList[i].obj);
        }
    }

}

 /*\brief Shuts down all persistent objects (based on their mode settings)
 * \a mode id for which the objects are shut down against 
 */
void Controller_ShutDown()
{
    uint16 i = 0;
    /* Regardless of whether its in the group lets shut it down */
    for (; i < Q_DIM(serverList); i++)
    {
        ((cPersistantObj*)serverList[i].obj)->Shutdown((cPersistantObj*)serverList[i].obj);
    }
    for (; i < Q_DIM(appList); i++)
    {
        ((cPersistantObj*)appList[i].obj)->Shutdown((cPersistantObj*)appList[i].obj);
    }
}

 /*\brief Access function allows for messages to be sent to servers using their id alone
 * \a object id of the server
 * \a evt to send to the object
 */
void SendToServer(uint16 object_id, const QEvt * evt)
{
    int16 objIndex = -1;
    QActive *pObj;
    
    /* Post to Server using QPPost*/
    if(object_id <= LAST_SRV_ID)
    {
        objIndex = getIndex(object_id,serverList,Q_DIM(serverList));
        ASSERT(objIndex != -1);
        pObj= (QActive *)serverList[objIndex].obj;
    }
    else if(object_id >= MAIN_APP_ID && object_id <= LAST_APP_ID)
    {
        objIndex = getIndex(object_id,appList,Q_DIM(appList));
        ASSERT(objIndex != -1);
        pObj= (QActive *)appList[objIndex].obj;
    }
    
    /* An ActiveObject is ctor when controlor require it to Active.
     * If someone send to Server before its ctor, system will crash on QACTIVE_POST().
     * Thus we check here, to avoid crash.
     */
    ASSERT( Server_isCtor(pObj) );
    QACTIVE_POST(pObj, evt, 0);
}

 /*\brief Access function allows for messages to be sent to the controller
 * \a evt to send to the controller
 */
void SendToController(const QEvt* evt)
{
    /* Post to Server using QPPost*/
    ASSERT( Server_isCtor(&Controller) );
    QACTIVE_POST(&Controller, evt, 0);
}

#ifdef HAS_DELEGATES

/** \brief This function should be called to get a free priority slot it should give you a number that is unique 
* and calculated to be free between server + app id's and controller so if your project 
* has 3 servers and 1 app and no other running delegates it would return 5 as the next available slot
* \return priority value. Zero if not none available
*/
uint8_t GetNextFreePriority(void)
{
    uint8_t retVal = 0;
    uint8_t i = LAST_SRV_ID + 1;
    for(; i < MAIN_APP_ID; i++)
    {
        if(!(freePriorityFlag >> i))
        {
            freePriorityFlag |= (1 << i);
            return i;
        }
    }
    return retVal;
}
/** \brief Call to release the priority slot you are using for a delegate
* \a priority number you are currently using
*/
void ReleasePriority(uint8_t priority)
{
    freePriorityFlag &= ~(1 << (priority));
}
#endif /* HAS_DELEGATES */

/* response the common request event, used in power resume/sleep flow*/
void CommonEvtResp(QActive* sender, QActive* receiver, eEvtReturn result,  eSignal signal)
{
    if(receiver == NULL)
    {
        return;
    }
    CommonRespEvt* pResp = Q_NEW(CommonRespEvt,signal);
    pResp->evtReturn = result;
    pResp->sender = sender;
    
    ASSERT( Server_isCtor(receiver) );
    QACTIVE_POST(receiver, (QEvt*)pResp, 0);
}

/** \brief Call to get server id from a valid object reference.
* \a valid object pointer
* \r server id or LAST_SRV_ID if pointer is invalid
*/
uint16 GetServerID(QActive* server)
{
    return (server == NULL) ? LAST_SRV_ID : server->prio;
}

/** \brief Public interface to get the pointer of an AO.
* \a AO ID
* \r The pointer of an AO
*/
QActive * GetServerPointer(uint16 object_id)
{
    return (QActive*)serverList[getIndex(object_id,serverList,Q_DIM(serverList))].obj;
}
            
            
/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
/** \brief getIndex is used to find the correct server from the server list
* \a object id of the server to get.
* \r returns the index
*/
static int16 getIndex(uint16 object_id, const tObjectListEntity * pObjectList, uint16 sizeOfList)
{
    int16 i = -1;
    for (i = 0; i < sizeOfList; i++)
    {
        if (pObjectList[i].object_id == object_id)
        {
            return i;
        }
    }
    return -1;
}


bool Server_isCtor(QActive * me) 
{
    QActive *pObj= me;
    if(pObj->super.vptr)
      return 1;
    else
      return 0;
}
