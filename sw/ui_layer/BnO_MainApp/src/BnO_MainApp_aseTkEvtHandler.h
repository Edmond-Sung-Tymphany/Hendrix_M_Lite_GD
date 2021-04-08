/**
*  @file      Bno_MainApp_allplayEvtHandler.h
*  @brief     header file of Allplay event handler
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef BNO_MAINAPP_ALLPLAY_EVT_HANDLER_H
#define	BNO_MAINAPP_ALLPLAY_EVT_HANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif

void BnO_MainApp_BtEvtHandler(cMainApp * const me, QEvt const * const e);
void BnO_MainApp_AseTkHandler(cMainApp * const me, QEvt const * const e);

#ifdef	__cplusplus
}
#endif

#endif	/* BNO_MAINAPP_ALLPLAY_EVT_HANDLER_H */

