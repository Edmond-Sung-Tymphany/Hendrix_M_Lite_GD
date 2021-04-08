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
*  @file      application.h
*  @brief     Application header
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef APPLICATION_H
#define	APPLICATION_H

#include "persistantObj.h"
#include "object_ids.h"
#include "signals.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define Application_Ctor(object_, handle_, timeOutSignal_, evtQue_, evtQueLen_, appId_)   \
        PersistantObj_Ctor((cPersistantObj*)(object_), (handle_),  \
                    (eSignal)(timeOutSignal_), (evtQue_), (evtQueLen_), (uint8)(appId_))

#define Application_Xtor(object_)  PersistantObj_Xtor( (cPersistantObj*)(object_))


SUBCLASS(cApplication, cPersistantObj)
METHODS
void Application_SendSigToServer(cApplication* me, ePersistantObjID serverId, eSignal signal);
END_CLASS

/** \brief Polymorphic way to assign the Startup function ptr for a app,
* Use in the initialising of the app
*/
#define APP_CTOR_FNC(pFnc)  .super_.super_.Startup = pFnc
/** \brief Polymorphic way to assign the Shutdown function ptr for a app,
* Use in the initialising of the app
*/
#define APP_XTOR_FNC(pFnc)  .super_.super_.Shutdown = pFnc

#ifdef	__cplusplus
}
#endif

#endif	/* APPLICATION_H */

