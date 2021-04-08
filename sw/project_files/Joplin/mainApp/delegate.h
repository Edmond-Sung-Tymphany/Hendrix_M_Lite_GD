/*****************************************************************************
*  @file      delegate.h
*  @brief     Header file private to delegate objects.
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef DELEGATE_H
#define	DELEGATE_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "qp_port.h"
#include "qf.h"
#include "commonTypes.h"
#include "object_ids.h"
#include "signals.h"
#include "cplus.h"


/** Delegates represent UI control logic for a specific interaction.
 * An application can "delegate" to a delegate the responsibility of this interaction.
 * See: https://sw.tymphany.com/redmine/projects/freertos/wiki/Architecture/delegate
 *
 *  \msc
 *    app, allplay_dlg,auxin_dlg, qp;
 *    ---  [label = "Delegate is simply a base. Create a specialization object (i.e allplay_dlg)" ];
 *    app->allplay_dlg [label="Delegate_Ctor()", URL="\ref Delegate_Ctor()"];
 *    ---  [label = "Delegate runs its control logic" ];
 *    qp->app [label="SourceSwitchInd(AuxIn)"];
 *    app->allplay_dlg [label="Delegate_Xtor()"];
 *    app->auxin_dlg [label="Delegate_Ctor()", URL="\ref Delegate_Ctor()"];
 *  \endmsc
 *
*****************************************************************************/


#ifdef DLG_DYN_MEM

#define CREATE_DLG(DLG, DLG_CLASS, INIT_STATE) if(DLG == NULL) {
                                                malloc(sizeof(DLG_CLASS)); \
                                                Delegate_Ctor((cDelegate *)DLG, Q_STATE_CAST(INIT_STATE)); \
                                                                }
#define DESTROY_DLG(DLG) if(DLG != NULL) {Delegate_Xtor((cDelegate *)DLG); free(DLG); DLG = NULL;};

#else
/** \brief DEF_DLG is a helper macro to define the delegate.
 * There are 2 versions. One for dynamically created dlg's and 1 for static
 * Used only internally
*/

/** \brief CREATE_DLG creates the delegate instance. Used only internally
*/
#define CREATE_DLG(DLG, DLG_CLASS, DLG_OWNER, INIT_STATE)  \
    Delegate_Ctor((cDelegate *)DLG, DLG_OWNER, Q_STATE_CAST(INIT_STATE));
/** \brief DESTROY_DLG destroys the delegate. Used only internally
*/
#define DESTROY_DLG(DLG) Delegate_Xtor((cDelegate*)DLG);

#endif

SUBCLASS(cDelegate, QActive)
    QActive *delegateOwner;
METHODS
    /* public functions */
    
/** \brief Constructs the Delegate Object.
* This does the base construction/registration for the object
* \sa QActive_ctor()
* The function takes 2 argument.
* \a me is a pointer to the active object (typeof cDelegate) structure.
* \a initState - the initial state to set for the dlg
*/
cDelegate * Delegate_Ctor( cDelegate *me, QActive *pDlgOwner, QStateHandler initState );

/** \brief Starts a Delegate Object.
* This does the base start up for a delegate
* \sa QActive_start()
*/
void Delegate_Start( cDelegate *me, QEvt const *eventQueue[],uint32_t evtQueueLen);
/** \brief Destructs the Delegate Object.
* This does the base destruction/de-reg for the object
* \sa QActive_ctor()
* The function takes 1 argument.
* \a me is a pointer to the active object (typeof cDelegate) structure.
*/
void Delegate_Xtor( cDelegate *me);
END_CLASS

/** \brief Polymorphic way to get the owner object ptr for a delegate,
*/
#define GET_DLG_OWNER(dlg)  (QActive *)dlg->super_.delegateOwner

/** \brief Polymorphic way to clear the owner object ptr for a delegate,
* Use in the de-initialising of the delegate
*/
#define CLR_DLG_OWNER(dlg)  dlg->super_.delegateOwner = NULL

/** \brief Base data for Dlg Ctor
* \a priority must be unique and > LAST_APP_ID
*/
typedef struct tDlgCtorData {
	uint16 priority;
} tDlgCtorData;

#ifdef	__cplusplus
}
#endif

#endif	/* DELEGATE_H */

