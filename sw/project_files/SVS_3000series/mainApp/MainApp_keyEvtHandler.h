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

#include "server.h"
#include "stdint.h"
#include "deviceTypes.h"
#include "qf_port.h"
#include "MainApp.h"

typedef enum
{
	RMK_PRESET_1_LOAD = NORMAL_KEY_ID_MAX,
	RMK_PRESET_2_LOAD,
	RMK_PRESET_3_LOAD,
	RMK_PRESET_4_LOAD,
	RMK_PRESET_1_SAVE,
	RMK_PRESET_2_SAVE,
	RMK_PRESET_3_SAVE,
	RMK_FACTORY_RESET,
	RMK_SCREEN_ON_OFF,
}eRemapKeyId;

typedef struct
{
	uint32_t keyId;
	eRemapKeyId remapKeyId;
}remapKey;


/************************************************************************************/
/* Key Functions*/
QState MainApp_KeyHandler(cMainApp * const me, QEvt const * const e);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_KEY_EVT_HANDLER_H */

