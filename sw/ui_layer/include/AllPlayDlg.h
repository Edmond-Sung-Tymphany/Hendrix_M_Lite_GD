/*****************************************************************************
*  @file      allplay_dlg.h
*  @brief     Header file to all play delegate.
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef ALL_PLAY_DLG_H
#define	ALL_PLAY_DLG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "qp_port.h"
#include "qf.h"
#include "signals.h"
#include "cplus.h"
#include "commonTypes.h"

#include "delegate.h"

SUBCLASS(cAllPlayDlg, cDelegate)
METHODS
/** \brief Constructs the Delegate Object.
* This does the base construction/registration for the delegate
* \sa QActive_ctor()
* 
* \a me is a pointer to the active object (typeof cAllPlayDlg) structure.
*/
cAllPlayDlg * AllPlayDlgCtor(cAllPlayDlg *me);

/** \brief Destructs the AllPlayDlg Object.
* This does the base destruction/de-reg for the object
* \sa QActive_ctor()
* \a me is a pointer to the active object (typeof cAllPlayDlg) structure.
*/
void AllPlayDlgXtor(cAllPlayDlg *me);
END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* ALL_PLAY_DLG_H */

