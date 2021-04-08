/*
* This file contains all the necessary settings for the IQS360A and this file can
* be changed from the GUI or edited here
* File:   IQS360A_init.h
* Author: Azoteq
*/

#ifndef IQS360A_INIT_H
#define IQS360A_INIT_H

/* Change the Multipliers (BASE value) of each channel  */
#define MULTIPLIERS_CH0						0x19
#define MULTIPLIERS_CH1						0x60
#define MULTIPLIERS_CH2						0x60
#define MULTIPLIERS_CH3						0x02
#define MULTIPLIERS_CH4						0x60
#define MULTIPLIERS_CH5						0x60
#define MULTIPLIERS_CH6						0x03
#define MULTIPLIERS_CH7						0x60
#define MULTIPLIERS_CH8						0x60
#define MULTIPLIERS_CH9						0x02
#define MULTIPLIERS_CH10					0x60
#define MULTIPLIERS_CH11					0x60
#define MULTIPLIERS_CH12					0x02

/* Change the Compensation for each channel  */
#define COMPENSATION_CH0					0xC8
#define COMPENSATION_CH1					0x38
#define COMPENSATION_CH2					0x41
#define COMPENSATION_CH3					0x70
#define COMPENSATION_CH4					0x65
#define COMPENSATION_CH5					0x71
#define COMPENSATION_CH6					0x71
#define COMPENSATION_CH7					0x70
#define COMPENSATION_CH8					0xA9
#define COMPENSATION_CH9					0x6C
#define COMPENSATION_CH10					0x58
#define COMPENSATION_CH11					0x64
#define COMPENSATION_CH12					0xF2

/* Change the Prox Settings or setup of the IQS360 */
#define PROXSETTINGS0_VAL					0x05
#define PROXSETTINGS1_VAL					0x24
#define PROXSETTINGS2_VAL					0x00
#define PROXSETTINGS3_VAL					0x02
#define PROXSETTINGS4_VAL					0x07
#define PROXSETTINGS5_VAL					0x7F

/* Change the Thresholds for each channel */
#define PROX_THRESHOLD						0x04
#define TOUCH_THRESHOLD_CH1					0x17
#define TOUCH_THRESHOLD_CH2					0x17
#define TOUCH_THRESHOLD_CH3					0x17
#define TOUCH_THRESHOLD_CH4					0x17
#define TOUCH_THRESHOLD_CH5					0x17
#define TOUCH_THRESHOLD_CH6					0x17
#define TOUCH_THRESHOLD_CH7					0x17
#define TOUCH_THRESHOLD_CH8					0x17
#define TOUCH_THRESHOLD_CH9					0x17
#define TOUCH_THRESHOLD_CH10				0x17
#define TOUCH_THRESHOLD_CH11				0x17
#define TOUCH_THRESHOLD_CH12				0x17

/* Change the Snap Thresholds for each channel */
#define SNAP_THRESHOLD_CH1					0x18
#define SNAP_THRESHOLD_CH2					0x18
#define SNAP_THRESHOLD_CH3					0x18
#define SNAP_THRESHOLD_CH4					0x18
#define SNAP_THRESHOLD_CH5					0x18
#define SNAP_THRESHOLD_CH6					0x18
#define SNAP_THRESHOLD_CH7					0x18
#define SNAP_THRESHOLD_CH8					0x18
#define SNAP_THRESHOLD_CH9					0xFF
#define SNAP_THRESHOLD_CH11					0x18
#define SNAP_THRESHOLD_CH11					0x18
#define SNAP_THRESHOLD_CH12					0x18

/* Change the Timing settings */
#define FILTER_HALT							0x96
#define POWER_MODE							0x00
#define TIMEOUT_PERIOD						0x28

/* Change ATI Target values  */
#define ATI_TARGET_CH0						0x80
#define ATI_TARGET_CH0_9					0x80

/* Change PWM settings  */
#define PWM_0								0x00
#define PWM_1								0xBF
#define PWM_2								0xBF
#define PWM_3								0xBF
#define PWM_4								0xBF
#define PWM_5								0xBF
#define PWM_6								0xBF
#define PWM_7								0xBF

/* Change PWM limits and Speed */
#define PWM_LIMITS							0x1E
#define PWM_SPEED							0x02

/* Set Active Channels */
#define ACTIVE_CH0							0xB7
#define ACTIVE_CH1							0x0D
#define TRACKPAD_CH0						0x00
#define TRACKPAD_CH1						0x00

/* Set Buzzer Output */
#define BUZZER_VAL							0x00

#endif	/* IQS360A_INIT_H */
