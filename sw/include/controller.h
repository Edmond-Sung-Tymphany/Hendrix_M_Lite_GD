/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/**
 * @file        controller.h
 * @brief       Public header for controller
 * @author      Chris Alexander 
 * @date        14-Jan-2013
 * @copyright   Tymphany Ltd.
 */

/** Controller object is responsible for launching all servers and apps at system start up
 * and ensuring they are swapped out when the when the mode of operation must change.
 *
 *  \msc
 *    main, controller, ServerList, AppList;
 *    main->controller [label="Controller_Ctor(mode)", URL="\ref Controller_Ctor()"];
 *    controller->ServerList [label="StartUp()"];
 *    controller->AppList [label="StartUp()"];
 *  \endmsc
 *
 *****************************************************************************/
#ifndef CONTROLLER_H
#define	CONTROLLER_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "commonTypes.h"
#include "server.h"
#include "application.h"
#include "modes.h"
#include "product.config"
/******************************************
 define a common request/response event
*******************************************/
REQ_EVT(CommonReqEvt)
END_REQ_EVT(CommonReqEvt)

RESP_EVT(CommonRespEvt)
   QActive* sender;
END_RESP_EVT(CommonRespEvt)


/******************************************
 switch mode 
*******************************************/

REQ_EVT(SwitchModeReqEvt)
   uint16 modeId;
END_REQ_EVT(SwitchModeReqEvt)

RESP_EVT(SwitchModeRespEvt)
   QActive* sender;
   uint16 modeId;
END_RESP_EVT(SwitchModeRespEvt)


/* public functions */
/** \brief Controller constructor. You can startup in an supported mode
* \a mode you wish to use \sa modes.h
*/
int16 Controller_Ctor(uint16 mode_id);
/** \brief Controller destructor. Will shutdown ALL persistent objects
*/
void Controller_Xtor();
/** \brief Controller shutdown allows you to shutdown. Maybe overlaps with Xtor?
*/
void Controller_ShutDown();
/** \brief Controller start up allows you to switch mode of operation
* \a mode you wish to use \sa modes.h
*/
void Controller_StartUp(uint16 mode_id);
/** \brief You may use this to call any active server without "knowing it". Simple pass the server_id in.
* \a server_id is a unique id for the server you wish to send msg's to
* \a evt is the event you wish to send.
*/
void SendToServer(uint16 server_id, const QEvt * evt);


void SendToController(const QEvt* evt);

/** \brief You may use this functions to respond a common event request
* \a sender is the active object that call this function to respond event
* \a receive is the active object that this function will send the event to
* \a result is the return result
* \a signal is the signal that this functions will send
*/
void CommonEvtResp(QActive* sender, QActive* receiver, eEvtReturn result,  eSignal signal);

/** \brief get server ID by its pointer
* @param server - pointer to server
* @return server id or LAST_SRV_ID if pointer is invalid
*/
uint16 GetServerID(QActive* server);
QActive *GetServerPointer(uint16 object_id);

bool Server_isCtor(QActive * me) ;


#ifdef HAS_DELEGATES

void ReleasePriority(uint8 priority);

uint8 GetNextFreePriority(void);
#endif /* HAS_DELEGATES */

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_H */

