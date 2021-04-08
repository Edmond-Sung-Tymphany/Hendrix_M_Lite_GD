/**
 * @file        controller_priv.h
 * @brief       Private header for controller
 * @author      Chris Alexander 
 * @date        14-Jan-2013
 * @copyright   Tymphany Ltd.
 */
#ifndef CONTROLLER_PRIV_H
#define	CONTROLLER_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "controller.h"

/* State declaration  */
QState Controller_Initial(QActive * const me);

QState Controller_Idle(QActive * const me, QEvt const * const e);

QState Controller_SwitchingModeState(QActive * const me, QEvt const * const e);

QActive Controller; /* Controller object. Singleton */

#ifdef	__cplusplus
}
#endif

#endif	/* CONTROLLER_PRIV_H */