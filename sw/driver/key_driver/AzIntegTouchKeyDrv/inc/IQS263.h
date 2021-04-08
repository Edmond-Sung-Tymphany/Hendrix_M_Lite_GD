/*******************************************************************************
 *
 *		Azoteq IQS263_Example_Code
 *
 *              IQS263.h - Registers & Memory Map
 *
*******************************************************************************/

#ifndef IQS263_H
#define	IQS263_H

#include "tch_defines.h"
#include "I2C_Comms.h"

// I2C DEFAULT SLAVE ADDRESS
#define IQS263_ADDR             0x44

/*********************** IQS263 REGISTERS *************************************/

#define IQS263_DEVICE_INFO             0x00
#define IQS263_SYS_FLAGS               0x01
#define IQS263_COORDINATES             0x02
#define IQS263_TOUCH_BYTES             0x03
#define IQS263_COUNTS                  0x04
#define IQS263_LTA                     0x05
#define IQS263_DELTAS                  0x06
#define IQS263_MULTIPLIERS             0x07
#define IQS263_COMPENSATION            0x08
#define IQS263_PROX_SETTINGS           0x09
#define IQS263_THRESHOLDS              0x0A
#define IQS263_TIMINGS_AND_TARGETS     0x0B
#define IQS263_GESTURE_TIMERS          0x0C
#define IQS263_ACTIVE_CHANNELS         0x0D

#define IQ263_SHOW_RESET               0x80
#define IQ263_REDO_ATI                 0x10

/* Bit Definitions    */
/*    System Flags    */
#define IQS263_LP_ACTIVE               0x01
#define IQS263_IND_HALT                0x02
#define IQS263_ATI_BUSY                0x04
#define IQS263_FILTER_HALTED           0x08
#define IQS263_PROJ_MODE               0x10
#define IQS263_ATI_ERROR               0x20
#define IQS263_MOVE_MENT               0x40
#define IQS263_SHOW_RESET              0x80

I2C_Device_t* IQS263_Get_Device (void);
uint8_t IQS263_Check_Version(void);
uint8_t IQS263_Set_Active_Channels(void);
uint8_t IQS263_Read_Flags(void);
uint8_t IQS263_Read_Data(uint8_t* buffer);
void IQS263_Clear_Buffers(void);
void IQS263_Copy_Buffer(uint8_t* buffer);
uint8_t IQS263_Read_wheel(uint8_t* buffer);
uint8_t IQS263_Set_ProxSettings(void);
uint8_t IQS263_Set_Thresholds(void);
uint8_t IQS263_Set_Targets(void);
uint8_t IQS263_Set_Base(void);
uint8_t IQS263_Redo_ATI(void);
uint8_t IQS263_ATI_Status(void);

#endif	/* IQS263_H */

