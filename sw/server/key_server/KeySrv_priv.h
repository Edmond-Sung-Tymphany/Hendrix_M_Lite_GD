/**
 * @file        KeySrv_priv.h
 * @brief       This file defines the interfaces of the key server and the implementation of the interfaces.
 * @author      Bob.Xu 
 * @date        2014-02-17
 * @copyright   Tymphany Ltd.
 */

#ifndef KEYSRV_PRIV_H
#define KEYSRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../server_priv.h"
#include "KeySrv.h"


/* states handler*/
QState KeySrv_Initial(cKeySrv *const me, QEvt const * const e);
QState KeySrv_Active(cKeySrv *const me, QEvt const *const e);
QState KeySrv_DeActive(cKeySrv *const me, QEvt const *const e);
QState KeySrv_PreActive(cKeySrv *const me, QEvt const *const e);

void KeySrv_SendKeyEvt_Direct(eKeyEvent evt, eKeyID keyId);

#ifdef __cplusplus
}
#endif

#endif /* KEYSRV_PRIV_H */
 