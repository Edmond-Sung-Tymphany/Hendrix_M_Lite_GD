/******************************************************************************
*                                                                             *
*                                                                             *
*                                Copyright by                                 *
*                                                                             *
*                              Azoteq (Pty) Ltd                               *
*                          Republic of South Africa                           *
*                                                                             *
*                           Tel: +27(0)21 863 0033                            *
*                          E-mail: info@azoteq.com                            *
*                                                                             *
*=============================================================================*
* @file 	State_Machine.h								 				      *
* @brief 	Header for state machine of touch driver				          *
* @author 	AJ van der Merwe - Azoteq (PTY) Ltd                            	  *
* @author 	JD Loy - Azoteq (PTY) Ltd                            	  		  *
* @version 	V1.0.1                                                        	  *
* @date 	17/03/2016                                                     	  *
* @note 	Added the Algorithm state machine states						  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STATE_MACHINE_H
#define __STATE_MACHINE_H

// User includes
#include "tch_defines.h"
#include "AzIntegTouchKeyDrv.h"

/** Enum for the state machine */
typedef enum State {
	Set_Active_Channels,
	Init,
	Redo_ATI,
	Check_ATI,
	Run,
	Block_Update,
	Error,
	BL_FirmwareUpgrade, /* Tymphany add */
}State_t;

/** Enum for the algorithm state machine */
typedef enum Algorithm_State {
	Idle,
	Wait_For_Touch,
	First_Touch,
	Wait_For_Swipe,
	Swipe_Event,
	Evaluate_Swipe,
	Reset_SM
}Algorithm_State_t;




/* Function Prototypes -------------------------------------*/
uint8_t state_machine_init(cAzIntegTouchKeyDrv *me);
void state_machine_power_on(cAzIntegTouchKeyDrv *me);
void state_machine_power_off(cAzIntegTouchKeyDrv *me);
void state_machine_process(cAzIntegTouchKeyDrv *me);
void Clear_SM_Buffers(uint8_t* buffer, uint8_t size);

void gesture_state_machine_process(void);
#endif /* __STATE_MACHINE_H */
