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
* @file 	tch_defines.h									 				      *
* @brief 	General defines header for frequently used values		          *
* @author 	AJ van der Merwe - Azoteq (PTY) Ltd                            	  *
* @version 	V1.0.1                                                        	  *
* @date 	21/08/2015                                                     	  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DEFINES_H
#define __DEFINES_H


/************************************************
 * Include
 ***********************************************/
#ifdef TOUCH_OFFLINE_TEST
#include <stdint.h>
#define uint32 uint32_t
#define uint16 uint16_t
#define uint8 uint8_t
#else
#ifdef HAS_AZOTEQ_PRIV
#include "AzIntegTouchKeyDrv_priv.h"
#else
#include "AzIntegTouchKeyDrv.h"
#endif
#include "deviceTypes.h"
#include "bsp.h"
#include "attachedDevices.h"
#include "product.config"
#include "trace.h"   //TP_PRINTF()
#endif


/************************************************
 * Global Variable
 ***********************************************/
 #ifndef TOUCH_OFFLINE_TEST
extern cAzIntegTouchKeyDrv *pTouchKeyDrv;
#endif


/************************************************
 * Function Prototype
 ***********************************************/
//#define __asm__(cmd) asm(cmd)
//#define timerEnable(...)

//TODO: BSP_BlockingDelayUs() is not accurary, confirm if it have problem

#define Delay_ms(ms) BSP_BlockingDelayMs(ms)
#define Delay_us(us) BSP_BlockingDelayMs(((us)/1000)+1)



/************************************************
 * Type
 ***********************************************/
typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;



/************************************************
 * Definition (Tymphany add)
 ***********************************************/
/* When detect noise, pause touch for a period */
#define NOISE_EXTEND_PERIOD_MS 10*1000


/* Tymphany add, for touch fail recovery:
 * 1. When IQS572/IQS333/IQ-Expender do not response for I2C for continuous [10 times], 
 *    ==> wait [30s], then reset touch power. 
 * 2. When IQS360 do not report events for [30s] 
      ==> reset touch power
 */
#define TOUCH_I2C_FAIL_READ_TIMES_LIMIT   10
#define TOUCH_360_IDLE_TIMEOUT_MS         20*1000  //30sec
#define TOUCH_PAUSE_AFTER_FAIL_READ_MS    20*1000  //30sec


/* This parameter is to avoid unexpected condition. Normally say, 
 * IQS333/IQS572 access I2C only if their window is open, and should not close soon.
 * But when something block CPU too long, or 333 close windows due to STOP bug,
 * we sholud not access I2C, and wait no more than IQS_WAIT_RDY_TIMEOUT_MS
 */
#define IQS_WAIT_RDY_TIMEOUT_MS (5)
#define TIMER_NOT_STARTED   (-1)
#define TIMER_STARTED       (0)

/* IQS572 Bootloader */
//#define IQS572_BOOTLOADER_DUMP_MEM   
//#define IQS572_BOOTLOADER_VERIFY_CHECKSUM


/* Original sample have handshake, but Azoteq Anson say it is NOT a good way,
 * thus we disable handshke, and never verify handshake.
 * when enable, please careful to verify
 */
//#define AZOTEQ_INTERGRATE_TOUCH_HANDSHAKE_ENABLE

#ifdef AZOTEQ_INTERGRATE_TOUCH_DRV_DEBUG
  #define ATOUCH_PRINTF(...)  TP_PRINTF(__VA_ARGS__)
  //#define ATOUCH_DUMP_IQS360A_DATA
#else  
  #define ATOUCH_PRINTF(...)
#endif


/* When total event number exceed IQSxxx_EVT_NUM_START_CHECK, start to check
 * if miss event (percentage) > IQSxxx_MISS_EVT_PERCENT_LIMIT
 * IQSxxx_MISS_EVT_PERCENT_LIMIT==-1 means do not check
 *
 * When IQS572 have events, IQS333 sometimes sense IQS572's STOP then close 
 * IQS333's windows. This is IQS333's bug.
 */ 
#define IQS5XX_MISS_EVT_PERCENT_LIMIT  -1 
#define IQS360a_MISS_EVT_PERCENT_LIMIT  -1
//#define IQS5XX_MISS_EVT_PERCENT_LIMIT  7   // IQS572 sould not have over 7% unread event
//#define IQS360a_MISS_EVT_PERCENT_LIMIT  20  // IQS333 sould not have over 20% unread event

#define IQS5XX_TOTAL_EVT_START_CHECK   100
#define IQS360A_TOTAL_EVT_START_CHECK   100

/*  When total event number exceed IQSxxx_TOTAL_EVT_MAX, 
 *  set miss/total event number to ZERO
 *  -1 means disable
 */ 
#define IQS5XX_TOTAL_EVT_MAX   -1 
#define IQS360a_TOTAL_EVT_MAX   -1
//#define IQS5XX_TOTAL_EVT_MAX   1000 
//#define IQS360a_TOTAL_EVT_MAX   1000

/* Double click period */
#define DOUBLE_CLICK_PERIOD_MS 300

/* Ignore HOLD event if period <= HOLD_FILTER_TIME_MS */
#define HOLD_FILTER_TIME_MS  200


/************************************************
 * Definition
 ***********************************************/
/* Define some errors */

#define RETURN_OK					(uint8_t)0x00
#define ERR_SETTTING_UP				(uint8_t)0x01
#define ERR_DEVICE_NOT_READY  		(uint8_t)0x03
#define ERR_VDDHI_NOT_OFF	  		(uint8_t)0x04
#define ERR_VDDHI_NOT_ON	  		(uint8_t)0x05
#define ERR_NO_DEVICE		  		(uint8_t)0x06

/* Tymphany add */
#define ERR_VER_NEED_FW_UPGRADE      (uint8_t)0x07  // Product/Project version correct, but Major/Minor version wrong, need to upgrade
#define ERR_VER_WRONG_PRODUCT        (uint8_t)0x08  // Product/Project version wrong, maybe I2C problem
#define ERR_CAN_NOT_ENTER_BL         (uint8_t)0x09  // Fail to enter bootloader

/* IC Errors */
#define ERR_IQS360A_RESET	  		(uint8_t)0x11
#define ERR_IQS360A_ATI_FAIL		(uint8_t)0x12
#define ERR_IQS5XX_RESET	  		(uint8_t)0x13
#define ERR_IQS333_RESET             (uint8_t)0x14
#define ERR_IQS333_ATI_FAIL          (uint8_t)0x15
#define ERR_IQS263_RESET            (uint8_t)0x16
#define ERR_IQS263_ATI_FAIL          (uint8_t)0x17

/* Gesture Engine Errors */
#define ERR_ADDING_TO_BUFFER  		(uint8_t)0x21
#define ERR_COPYING_DATA	  		(uint8_t)0x22
#define ERR_BUFFER_SIZE_INV  		(uint8_t)0x23

/* General terms */
#define OFF         0x00
#define ON          0xFF
#define WAIT        0x00
#define READY       0xFF
#define LOW         0x00
#define HIGH        0xFF
#define DOWN        0x00
#define UP      	0xFF
#define NOT_TRI     0x00
#define TRISTATE    0xFF
#define INACTIVE    0x00
#define ACTIVE      0xFF
#define DISCONNECT  0x00
#define CONNECT     0xFF

// Generel Byte Defines
#define BYTE_0		0
#define BYTE_1		1
#define BYTE_2		2
#define BYTE_3		3
#define BYTE_4		4
#define BYTE_5		5
#define BYTE_6		6
#define BYTE_7		7
#define BYTE_8		8
#define BYTE_9		9
#define BYTE_10		10


#endif /* __DEFINES_H */
