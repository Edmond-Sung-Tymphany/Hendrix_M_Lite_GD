/**
*  @file      MainApp_keyEvtHandler.h
*  @brief     header file of Key event handler
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_KEY_EVT_HANDLER_H
#define	MAINAPP_KEY_EVT_HANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

/************************************************************************************/
/* Key Functions*/
void BnO_MainApp_AseTkKeyHandler(cMainApp * const me, QEvt const * const e);

void BnO_MainApp_KeyHandler(cMainApp * const me, QEvt const * const e);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_KEY_EVT_HANDLER_H */

