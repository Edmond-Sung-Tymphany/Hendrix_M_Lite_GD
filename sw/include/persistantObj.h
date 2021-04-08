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
*  @file      persistantObj.h
*  @brief     Concrete base for base persistent active objects. This includes Servers & Applications
              see: http://sw.tymphany.com/redmine/projects/freertos/wiki/Architecture
*  @author    Christopher Alexander
*  @date      14-Jan-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef PERSISTANT_OBJ_H
#define	PERSISTANT_OBJ_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "qp_port.h"
#include "qf.h"
#include "cplus.h"
#include "commonTypes.h"
#include "object_ids.h"
#include "signals.h"

#define TIME_EVT_OF(x)  (&((cPersistantObj*)(x))->timeEvt)

SUBCLASS(cPersistantObj, QActive)

    /** \brief Initialises the Object and registers the object.
    * with the framework. This must be assigned and do a Ctor() on its parent.
    * This function MUST be implemented and must call the a CTOR() function
    * The function takes 1 argument.
    * \a me is a pointer to the active object (typeof cPersistantObj) structure.
    */
     void (*Startup)(cPersistantObj*);
    /** \brief de-initializes and stops the Object
    * with the framework. This must be assigned and do a Xtor() on its parent.
    * The function takes 1 argument.
    * \a me is a pointer to the active object (typeof cPersistantObj) structure.
    */
     void (*Shutdown)(cPersistantObj*);
     QTimeEvt timeEvt;
METHODS
    /* public functions */


    /** @brief Consructs the Persistant Object.
    * This does the base contruction/registration for the object, and initial the timer in persistant object
    * @param[in]    me                        Persistant active object
    * @param[in]    initState                QStateHandler initial function pointer
    * @param[in]    timeOutSignal        signal for timer
    * @param[in]    evtQue                  event que of object
    * @param[in]    evtQueLen             size of the event que of object
    * @param[in]    objectId                Persistant object ID
    * The function takes 6 argument.
    */
    void PersistantObj_Ctor(cPersistantObj *me, QStateHandler initState,eSignal timeOutSignal, 
                            QEvt const *evtQue[], uint32 evtQueLen, uint8 objectId);


    /** @brief Destructs the Persistant Object.
    * This does the base desturction/de-reg for the object
    * @param[in]    me              Persistant active object
    * @sa QActive_stop()
    * The function takes 1 argument.
    * @a me is a pointer to the active object (typeof cServer) structure.
    */
    void PersistantObj_Xtor( cPersistantObj *me);
    
    /* Since we "hide" the object ref any public function that passes the object should be private anyway */
    void PersistantObj_RefreshTick(cPersistantObj* me, const uint32 tickTime);
END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* PERSISTANT_OBJ_H */
