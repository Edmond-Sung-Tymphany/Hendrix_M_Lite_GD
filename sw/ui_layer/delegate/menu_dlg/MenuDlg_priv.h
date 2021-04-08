/**
 * @file        MenuDlg_priv.h
 * @brief       This file declare and implement the menu dlg
 * @author      Bob.Xu 
 * @date        2014-11-19
 * @copyright   Tymphany Ltd.
 */
 
#ifndef MENUDLG_PRIV_H
#define MENUDLG_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "delegate.h"
#include "KeySrv.h"
#include "DisplaySrv.h"

REQ_EVT(MenuDlgStandbyEvt)
END_REQ_EVT(MenuDlgStandbyEvt)

REQ_EVT(MenuDlgActivateEvt)
END_REQ_EVT(MenuDlgActivateEvt)

/* State function definitions */
static QState MenuDlg_Initial(QActive * const me);
static QState MenuDlg_Active(QActive * const me, QEvt const * const e);
static QState MenuDlg_PreActive(QActive * const me, QEvt const * const e);
static void MenuDlg_HandleKeyEvt(cMenuDlg * const me,KeyStateEvt const * const e);
static void MenuDlg_HandleBackKey(cMenuDlg * const me, KeyStateEvt const * const e);
static void MenuDlg_HandlePreKey(cMenuDlg * const me, KeyStateEvt const * const e);
static void MenuDlg_HandleNextKey(cMenuDlg * const me, KeyStateEvt const * const e);
static void MenuDlg_HandleEnterKey(cMenuDlg * const me, KeyStateEvt const * const e);
static void MenuDlg_ScrollPre(cMenuDlg * const me, eKeyEvent keyEvent);
static void MenuDlg_ScrollNext(cMenuDlg * const me, eKeyEvent keyEvent);
static void MenuDlg_Enter(cMenuDlg * const me, eKeyEvent keyEvent);
static void MenuDlg_BackToParent(cMenuDlg * const me, eKeyEvent keyEvent);
static void MenuDlg_IndicateController(cMenuDlg * const me,eKeyID keyId, eKeyEvent keyEvt);
static void MenuDlg_Push(cMenuDlg * const me);
static void MenuDlg_Pop(cMenuDlg * const me);
static void MenuDlg_RefleshTick(cMenuDlg * const me, const uint16 tickTime);
static void MenuDlg_BackToNode(cMenuDlg * const me, uint8 nodeLevel);
static void MenuDlg_UpdateToNode(cMenuDlg * const me);
static void MenuDlg_UpdateNodeStatus(cMenuDlg * const me);
static QState MenuDlg_DeActive(QActive * const me, QEvt const * const e);
/* Add some private data here if needed */

#ifdef __cplusplus
}
#endif

#endif /* MENUDLG_PRIV_H */