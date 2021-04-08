/**
*  @file      application.c
*  @brief     Source file application class
*  @author    Christopher Alexander
*  @date      15-Jan-2014
*  @copyright Tymphany Ltd.
*/

#include "application.h"
#include "controller.h"


/* A generic API for send signal  to servers */
void Application_SendSigToServer(cApplication* me, ePersistantObjID serverId, eSignal signal)
{
    CommonReqEvt* reqEvt = Q_NEW(CommonReqEvt, signal);
    reqEvt->sender = (QActive*)me;
    SendToServer(serverId,(QEvt*)reqEvt);
}

