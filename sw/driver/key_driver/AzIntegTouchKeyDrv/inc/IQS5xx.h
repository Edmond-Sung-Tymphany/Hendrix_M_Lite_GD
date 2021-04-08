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
* @file 	IQS5xx.h													      *
* @brief 	Header file for the IQS5xx "object" specific commands	          *
* @author 	AJ van der Merwe - Azoteq PTY Ltd                             	  *
* @version 	V1.0.0                                                        	  *
* @date 	24/08/2015                                                     	  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IQS5XX_H
#define __IQS5XX_H


// User includes
#include "tch_defines.h"
#include "I2C_Comms.h"

/* Defines -------------------------------------------------*/
//#define IQS5XX_RDY_PIN                  GPIO_Pin_4
//#define IQS5XX_RDY_PORT                 GPIOB        //GPIOA
//#define IQS5XX_RDY_PIN_SOURCE           GPIO_PinSource4
//#define IQS5XX_RDY_PORT_SOURCE          GPIO_PortSourceGPIOB
//#define IQS5XX_EXTI_IRQ                 (uint32_t)EXTI4_IRQn
//#define IQS5XX_EXTI_LINE                (uint32_t)EXTI_Line4


/* I2C Address of IQS333 */
#define IQS5XX_ADDR						0x74
#define BOOTLOADER_IQS5XX_ADDR 			0x34

#define PRODUCT_NUMBER 					0x20

/* IQS5xx Command Structure */

// Definitions of read-only Address-commands implemented on IQS5xx
#define	INFO_BYTE						0x00	// Info and Gestures				:3 bytes
#define XY_STREAM						0x01	// XY data				            :4 bytes
#define	PROX_STATUS						0x03	// Proximity status registers 		:2 bytes
#define	TOUCH_STATUS					0x04	// Touch status registers			:2 bytes
#define	COUNTS_5XX						0x05	// Counts registers					:2 bytes
#define	LTA_5XX							0x06	// Long term averages				:2 bytes
#define	ATI_COMP						0x07	// ATI compensation values			:2 bytes
#define	PORT_CONTROL					0x08	// Port control registers			:2 bytes
#define SNAP_STATUS						0x09	// Snap channel status		    	:4 bytes
#define	CONTROL_SETTINGS				0x10	// Threshold settings				:2 bytes
#define THRESHOLD_SETTINGS				0x11	// Threshold settings			    :4 bytes
#define ATI_SETTINGS					0x12	// ATI Settings					    :4 bytes
#define FILTER_SETTINGS					0x13	// Filter settings				    :4 bytes
#define TIMING_SETTINGS					0x14	// Timing settings				    :4 bytes
#define CHANNEL_SETUP					0x15	// Channel setups				    :4 bytes
#define HARDWARE_CONFIG					0x16	// Hardware configurations		    :4 bytes
#define ACTIVE_CH						0x17	// Active channels setup    		:4 bytes
#define DEBOUNCE_SETTINGS				0x18	// Debounce settings			    :4 bytes
#define PM_PROX_STATUS					0x19	// Touch and snap channels    		:4 bytes
#define JUMP_TO_BOOTLOADER              0xFF    // Jump to bootloader (tymphany add):1 bytes

/* Bit Definitions */
/*	System Flags	*/
#define SHOW_RESET						0x80
#define IQS5XX_ACK_RESET				0x80
#define IQS5XX_EVENT_MODE				0x01
#define DIS_PROX_EVENT					0x80
#define DIS_TOUCH_EVENT					0x40
#define DIS_SNAP_EVENT					0x20
#define DIS_PMPROX_EVENT				0x10

/* Gesture Byte Bit Definitions */
#define TAP								0x01
#define TAP_HOLD						0x02
#define LEFT_SWIPE						0x04
#define RIGHT_SWIPE						0x08
#define UP_SWIPE						0x10
#define DOWN_SWIPE						0x20

/* Tymphany add */
#define DOUBLE_TAP                      0x40



/* IQS5xx Bootloader Command Structure (Tymphany add) */
#define IQS5XX_BL_READ_VER              0x00    // Read bootloader version          :2 bytes
#define IQS5XX_BL_READ_MEM              0x01    // Read memory map                  :64 bytes
#define IQS5XX_BL_JUMP_TO_APP           0x02    // Execute applicatino firmware     :0 bytes
#define IQS5XX_BL_READ_CRC_RESULT       0x03    // Calculate CRC                    :1 bytes
//Other address: write data bytes to firmware application address if valid address presented

/* IQS5xx Bootloader definition (Tymphany add) */
#define IQS5XX_CHECKSUM_START_ADDR      0x83C0 // starting address of checksum
#define IQS5XX_CHECKSUM_SIZE            64
#define IQS5XX_APP_START_ADDR           0x8400 // starting address of application firmware
#define IQS5XX_APP_SIZE                 15360
#define IQS5XX_APP_WRITE_COUNT          240    // the application size is fixed at 15360 bytes - thus program the complete space with 240 writes of 64 bytes per write
#define IQS5XX_APP_BLOCK_SIZE           64     // size data that the bootloader requires for a write to occur

#define IQS5XX_BL_TRIGGER_SLAVE_ADDR    0xA8   // App Slave address XOR 0x40

/* Function Prototypes -------------------------------------*/
I2C_Device_t* IQS5xx_Get_Device (void);

// Read
uint8_t IQS5xx_Init(void);
uint8_t IQS5xx_Check_Version(void);
uint8_t IQS5xx_Read_Data(uint8_t* buffer);

// Setup
uint8_t IQS5xx_Set_EventMode(Event_Mode_t event_on_off);
uint8_t IQS5xx_Set_ControlSettings(void);
void IQS5xx_Clear_Buffers(void);
void IQS5xx_Copy_Buffer(uint8_t* buffer);

/* Tymphany add */
uint8_t IQS5xx_BL_FirmwareUpgrade();

#endif /* __IQS5XX_H */
