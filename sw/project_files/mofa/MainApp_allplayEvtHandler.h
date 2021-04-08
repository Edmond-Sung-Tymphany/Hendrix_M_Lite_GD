/**
*  @file      MainApp_allplayEvtHandler.h
*  @brief     header file of Allplay event handler
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_ALLPLAY_EVT_HANDLER_H
#define	MAINAPP_ALLPLAY_EVT_HANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

void MainApp_DisplayAllplaySystemMode(cMainApp * const me, enum allplay_system_mode_value currentSystemMode);
void MainApp_BTEvtHandler(cMainApp * const me, QEvt const * const e);
void MainApp_AllPlayStateHandler(cMainApp * const me, QEvt const * const e);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_ALLPLAY_EVT_HANDLER_H */

