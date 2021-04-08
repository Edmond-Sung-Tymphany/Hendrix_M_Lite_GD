/*****************************************************************************
*  @file      auxin_dlg.h
*  @brief     Header file to all play delegate.
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef AUX_IN_DLG_H
#define	AUX_IN_DLG_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "qp_port.h"
#include "qf.h"
#include "signals.h"
#include "cplus.h"
#include "commonTypes.h"

#include "delegate.h"

SUBCLASS(cAuxInDlg, cDelegate)
METHODS
/** \brief Constructs the AuxInDlg Object.
* This does the base construction/registration for the delegate
* \sa QActive_ctor()
* 
* \a me is a pointer to the active object (typeof cAuxInDlg) structure.
*/
cAuxInDlg * AuxInDlg_Ctor(cAuxInDlg *me);

/** \brief Destructs the AuxInDlg Object.
* This does the base destruction/de-reg for the object
* \sa QActive_ctor()
* \a me is a pointer to the active object (typeof cAuxInDlg) structure.
*/
void AuxInDlg_Xtor(cAuxInDlg *me);
END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* AUX_IN_DLG_H */

