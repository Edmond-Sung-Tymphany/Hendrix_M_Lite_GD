/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/*****************************************************************************
*  @file      server.h
*  @brief     Header file for base Server class see: http://sw.tymphany.com/redmine/projects/freertos/wiki/Architecture
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

/** Server object supplies a service to other objects through a set of API's
 * Each server is a singleton. This shows the subscribe/publish model
 *  \msc
 *    server, client;
 *              ---  [label = "Subscribe is actually done to the thread scoped queue provided by QP" ];
 *    client->server [label = "Subscribe(event)"];
 *              ---  [label = "Server gets some external resource event that would cause it to publish the event" ];
 *    server=>server [label = "Process()" ];
 *              ---  [label = "Again publish is actually sent to the thread scoped queue provided by QP" ];
 *    server->client [label = "Publish(event)"];;
 *  \endmsc
 *
 ****************************************************************************/
 /**
 * This shows direct calls to the server from a client
 *  \msc
 *    server, client, server2;
 *              ---  [label = "Direct sending is done through using a globally visible list of server ID's" ];
 *    client->server [label = "SendToServer(server_id, event)" URL="\ref SendToServer(uint16 server_id, const QEvt * evt)"];];
 *    server=>server [label = "Process()" ];
 *              ---  [label = "Based on the processing the server can then action some other event" ];
 *    server->server2 [label = "SendToServer(new event)"];;
 *  \endmsc
 *
 *****************************************************************************/
#ifndef SERVER_H
#define	SERVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "product.config"

#include "persistantObj.h"
#include "object_ids.h"
#include "signals.h"


#define Server_Ctor(object_, handle_, timeOutSignal_, evtQue_, evtQueLen_, serverId_)   \
    PersistantObj_Ctor((cPersistantObj*)(object_), (handle_), \
                    (eSignal)(timeOutSignal_), (evtQue_), (evtQueLen_), (uint8)(serverId_))

#define Server_Xtor(object_)  PersistantObj_Xtor( (cPersistantObj*)(object_))


SUBCLASS(cServer, cPersistantObj)
METHODS
/* public functions */
END_CLASS

/** \brief Polymorphic way to assign the serverStartup function ptr for a server,
* Use in the initialising of the server
*/
#define SRV_CTOR_FNC(pFnc)  .super_.super_.Startup = pFnc

/** \brief Polymorphic way to assign the serverShutdown function ptr for a server,
* Use in the initialising of the server
*/
#define SRV_XTOR_FNC(pFnc)  .super_.super_.Shutdown = pFnc

#ifdef	__cplusplus
}
#endif

#endif	/* SERVER_H */
