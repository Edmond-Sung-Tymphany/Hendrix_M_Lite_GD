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
* @file 	IQS360A.h														  *
* @brief 	IQS360A Object													  *
* @author 	JD Loy - Azoteq PTY Ltd											  *
* @version 	V1.0.0                                                            *
* @date 	10/03/2016                                                        *
*******************************************************************************/

/** @file
 *
 * @defgroup 	IQS360A Object Description
 * @{
 * @ingroup 	IQS360A Code
 * @brief       The IQS360A, make up IQS360A as an object with registers and bit
 *		defines
 *
 * @details 	This module defines the IQS360A and makes it into an object. In
 *              order to use this object, include it into the project and
 *              create a variable of type IQS360A_t. This will contain all
 *              the member fields of the IQS360A object.
 *
 * @note 
 *
 */

#ifndef IQS360A_H__
#define IQS360A_H__

// User includes
#ifndef TOUCH_OFFLINE_TEST
#include "I2C_Comms.h"
#endif


/* Defines -------------------------------------------------*/
#define IQS360A_RDY_PIN					GPIO_Pin_5
#define IQS360A_RDY_PORT				GPIOB
#define IQS360A_RDY_PIN_SOURCE			GPIO_PinSource5
#define IQS360A_RDY_PORT_SOURCE			GPIO_PortSourceGPIOB
#define IQS360A_EXTI_IRQ				(uint32_t)EXTI9_5_IRQn
#define IQS360A_EXTI_LINE				(uint32_t)EXTI_Line5

/*      IQS360A Address  */
#define	IQS360A_ADDR                     0x64

/* Base Values */
#define BASE_75						0x40	// Base 75
#define BASE_100					0x80	// Base 100
#define BASE_150					0xC0	// Base 150
#define BASE_200					0x00	// Base 200 (default)

/* Number of Active channels on IQS3360 */
#define NR_OF_ACTIVE_CHANNELS			13		// 12 active channels in this setup + Prox

/*
//      Device Info
typedef union
{
    struct
    {
        unsigned char ProductNumber;
        unsigned char VersionNumber;
    };
    unsigned char DeviceInfo;
}DeviceInfo_t;

//      System Flags
typedef union
{
    struct
    {
        unsigned char Zoom				:1;
        unsigned char Noise				:1;
        unsigned char ATI_Busy			:1;
        unsigned char LP_Active			:1;
        unsigned char Is_CH0			:1;
        unsigned char Charge_8M_4M		:1;
        unsigned char Filter_Halted		:1;
        unsigned char Show_Reset		:1;
    };
    unsigned char SystemFlags;
}SystemFlags_t;

//      Events (System Flags Byte 2)
typedef union
{
    struct
    {
        unsigned char ATI_Event			:1;
        unsigned char Prox_Event		:1;
        unsigned char Touch_Event		:1;
        unsigned char Track_Event		:1;
        unsigned char None				:1;
        unsigned char Snap_Event		:1;
        unsigned char Block_Event		:1;
        unsigned char ATI_Error 		:1;
    };
    unsigned char Events;
}Events_t;

//      X-Data
typedef union
{
    struct
    {
        unsigned char X_Low                     :8;
        unsigned char X_High                    :2;
    };
    unsigned int 	X;  // 10-bit number
}X_t;

//      Y-Data
typedef union
{
    struct
    {
        unsigned char Y_Low                     :8;
        unsigned char Y_High                    :2;
    };
    unsigned int 	Y;  // 10 -bit number
}Y_t;

//      Status
//	Touch Channels
typedef union
{
    struct
    {
        unsigned char Touch_Channels_0;
        unsigned char Touch_Channels_1;
    };
    unsigned int Touch_Channels;
}TouchChannels_t;

//	Snap Channels
typedef union
{
    struct
    {
        unsigned char Snap_Channels_0;
        unsigned char Snap_Channels_1;
    };
    unsigned int Snap_Channels;
}SnapChannels_t;

//	Counts
typedef union
{
    struct
    {
        unsigned char Counts_Low;
        unsigned char Counts_High;
    };
    unsigned int Counts;
}Counts_t;

//	LTA
typedef union
{
    struct
    {
        unsigned char LTA_Low;
        unsigned char LTA_High;
    };
    unsigned int LTA;
}LTA_t;

//	Multipliers
typedef union
{
    struct
    {
        unsigned char Sens_Multi		:4;
        unsigned char Comp_Multi		:2;
        unsigned char Base_Value		:2;
    };
    unsigned char Multi;
}Multi_t;

//	Settings
typedef union
{
    struct
    {
        unsigned char Proj_Bias			:2;
        unsigned char Cs_Size			:1;
        unsigned char Reseed			:1;
        unsigned char Redo_ATI			:1;
        unsigned char Snap_Enable		:1;
        unsigned char ATI_Partial		:1;
        unsigned char ATI_Off			:1;
    };
    unsigned char Settings0;
}Settings0_t;

typedef union
{
    struct
    {
        unsigned char ATI_Band			:1;
        unsigned char Force_Sleep		:1;
        unsigned char Prox_Proj			:1;
        unsigned char None				:1;
        unsigned char Halt_Charge		:1;
        unsigned char Turbo_Mode		:1;
        unsigned char Xfer				:1;
        unsigned char ACK_Reset			:1;
    };
    unsigned char Settings1;
}Settings1_t;

typedef union
{
    struct
    {
        unsigned char Halt				:2;
        unsigned char Event_Mode		:1;
        unsigned char Timeout			:1;
        unsigned char ACF_Disable		:1;
        unsigned char Force_Halt		:1;
        unsigned char WDT_Off			:1;
        unsigned char Soft_Reset		:1;
    };
    unsigned char Settings2;
}Settings2_t;

typedef union
{
    struct
    {
        unsigned char Coord_Filter		:1;
        unsigned char Relative_Coord	:1;
        unsigned char RX_On_Multiple	:1;
        unsigned char Reject_Touch		:1;
        unsigned char Beta				:2;
        unsigned char Border_Correct	:1;
        unsigned char Block_7_5 		:1;
    };
    unsigned char Settings3;
}Settings3_t;

typedef union
{
    struct
    {
        unsigned char Pass			:3;
        unsigned char Up_Enable     :1;
        unsigned char UP			:3;
        unsigned char None          :1;
    };
    unsigned char Settings4;
}Settings4_t;

//      Buzzer
typedef union
{
    struct
    {
        unsigned char Burst			:1;
        unsigned char Perm			:1;
        unsigned char DC			:1;
        unsigned char Buz_None      :4;
        unsigned char Buz_Enable	:1;
    };
    unsigned char Buzzer;
}Buzzer_t;

//      IQS360A Object
typedef struct
{
    //  Device Info
    DeviceInfo_t DeviceInfo;

    //  System Flags
    SystemFlags_t SystemFlags;

    //  Events
    Events_t Events;

    //  X Coordinates
    X_t X;

    //  Y Coordinates
    Y_t Y;

    //  Touch Channels (Events)
    TouchChannels_t Touch_Channels;

    //  Snap Channels (Events)
    SnapChannels_t Snap_Channels;

    //  Channel Counts
    Counts_t Counts_CH0;
    Counts_t Counts_CH1;
    Counts_t Counts_CH2;
    Counts_t Counts_CH3;
    Counts_t Counts_CH4;
    Counts_t Counts_CH5;
    Counts_t Counts_CH6;
    Counts_t Counts_CH7;
    Counts_t Counts_CH8;
    Counts_t Counts_CH9;
    Counts_t Counts_CH10;
    Counts_t Counts_CH11;
    Counts_t Counts_CH12;

    //  LTA
    LTA_t LTA_CH0;
    LTA_t LTA_CH1;
    LTA_t LTA_CH2;
    LTA_t LTA_CH3;
    LTA_t LTA_CH4;
    LTA_t LTA_CH5;
    LTA_t LTA_CH6;
    LTA_t LTA_CH7;
    LTA_t LTA_CH8;
    LTA_t LTA_CH9;
    LTA_t LTA_CH10;
    LTA_t LTA_CH11;
    LTA_t LTA_CH12;

    //  Multipliers (BASE)
    Multi_t Multi_CH0;
    Multi_t Multi_CH1;
    Multi_t Multi_CH2;
    Multi_t Multi_CH3;
    Multi_t Multi_CH4;
    Multi_t Multi_CH5;
    Multi_t Multi_CH6;
    Multi_t Multi_CH7;
    Multi_t Multi_CH8;
    Multi_t Multi_CH9;
    Multi_t Multi_CH10;
    Multi_t Multi_CH11;
    Multi_t Multi_CH12;

    //  Settings
    Settings0_t Settings0;
    Settings1_t Settings1;
    Settings2_t Settings2;
    Settings3_t Settings3;
    Settings4_t Settings4;
    unsigned char Settings5;

    //	Thresholds
    unsigned char Threshold_CH0;
    unsigned char Threshold_CH1;
    unsigned char Threshold_CH2;
    unsigned char Threshold_CH3;
    unsigned char Threshold_CH4;
    unsigned char Threshold_CH5;
    unsigned char Threshold_CH6;
    unsigned char Threshold_CH7;
    unsigned char Threshold_CH8;
    unsigned char Threshold_CH9;
    unsigned char Threshold_CH10;
    unsigned char Threshold_CH11;
    unsigned char Threshold_CH12;

    //	Timings
    unsigned char Filter_Halt;
    unsigned char Power_Mode;
    unsigned char Timeout_Period;

    //	Ati Targets
    unsigned char ATI_Target_CH0;
    unsigned char ATI_Target_CH1_12;

    //  Active Channels
    TouchChannels_t Active_Channels;

    //  Trackpad Active Channels
    TouchChannels_t Trackpad_Active_Channels;

    //	Snap Thresholds
    unsigned char Snap_Threshold_CH0;
    unsigned char Snap_Threshold_CH1;
    unsigned char Snap_Threshold_CH2;
    unsigned char Snap_Threshold_CH3;
    unsigned char Snap_Threshold_CH4;
    unsigned char Snap_Threshold_CH5;
    unsigned char Snap_Threshold_CH6;
    unsigned char Snap_Threshold_CH7;
    unsigned char Snap_Threshold_CH8;
    unsigned char Snap_Threshold_CH9;
    unsigned char Snap_Threshold_CH10;
    unsigned char Snap_Threshold_CH11;
    unsigned char Snap_Threshold_CH12;

    //  Correction Constant (Trackpad Edge Correction
    unsigned char Correction_Constant;

    //  Buzzer
    Buzzer_t Buzzer;
	
} IQS360A_t;
*/

//	Registers
#define VERSION_INFO		                0x00    //    R  -  2 unsigned chars
#define FLAGS       		                0x01    //    R  -  2 unsigned chars
#define XY_DATA                             0x02    //    R  -  4 unsigned chars
#define STATUS                              0x03    //    R  -  4 unsigned chars
#define COUNTS                              0x04    //    R  - 26 unsigned chars
#define LTA_REG                                 0x05    //    R  - 26 unsigned chars
#define MULTIPLIERS                         0x06    //  R/W  - 26 unsigned chars
#define COMPENSATION                        0x07    //  R/W  - 26 unsigned chars
#define PROXSETTINGS                        0x08    //  R/W  -  6 unsigned chars
#define THRESHOLDS                          0x09    //  R/W  - 26 unsigned chars
#define TIMINGS                             0x0A    //  R/W  -  3 unsigned chars
#define ATI_TARGETS                         0x0B    //  R/W  -  2 unsigned chars
#define EVENT_MASKS                         0x0C    //  R/W  -  2 unsigned chars
#define NOT_IMPLEMETED                      0x0D    //  not implemented
#define ACTIVE_CHANNELS                     0x0E    //  R/W  -  4 unsigned chars
#define SNAP_THRESHOLDS                     0x0F    //  R/W  - 12 unsigned chars
#define CORRECTION_CONSTANT                 0x10    //  R/W  -  1 unsigned chars
#define BUZZER                              0x11    //  R/W  -  2 unsigned chars

/* Base Values */
#define BASE_75						0x40	// Base 75
#define BASE_100					0x80	// Base 100
#define BASE_150					0xC0	// Base 150
#define BASE_200					0x00	// Base 200 (default)

//	Bit Definitions

//	Channels
#define CH0_B                                 0x0001
#define CH1_B                                 0x0002
#define CH2_B                                 0x0004
#define CH3_B                                 0x0008
#define CH4_B                                 0x0010
#define CH5_B                                 0x0020
#define CH6_B                                 0x0040
#define CH7_B                                 0x0080
#define CH8_B                                 0x0100	// Remeber High Byte
#define CH9_B                                 0x0200	// Remeber High Byte
#define CH10_B                                0x0400	// Remeber High Byte
#define CH11_B                                0x0800	// Remeber High Byte
#define CH12_B                                0x1000	// Remeber High Byte

//	System Flags
#define ZOOM                                0x01
//#define NOISE                               0x02	// removed in IQS360A
#define ATI_BUSY                            0x04
#define LP_ACTIVE                           0x08
#define IS_CH0                              0x10
#define CHARGE_8M_4M                        0x20
#define FILTER_HALTED                       0x40
#define SHOW_RESET                          0x80

/*      Events  */
#define ATI_EVENT                           0x01
#define PROX_EVENT                          0x02
#define TOUCH_EVENT                         0x04
#define TRACK_EVENT                         0x08
#define WAKEUP_EVENT						0x10	// added in IQS360A
#define SNAP_EVENT                          0x20
//#define BLOCK_EVENT                         0x40	// removed in IQS360A
#define ATI_ERROR                           0x80

/*	Multipliers	*/
#define MUTLIPLIER_BITS                     0x3F
#define BASE_VALUE                          0xC0

/*	Settings	*/
/*	Settings 0	*/
#define PROJ_BIAS                           0x03
#define CS_SIZE                             0x04
#define RESEED                              0x08
#define REDO_ATI                            0x10
#define SNAP_ENABLE                         0x20
#define ATI_PARTIAL                         0x40
#define ATI_OFF                             0x80

/*	Settings 1	*/
#define ATI_BAND                           0x01
#define FORCE_SLEEP                        0x02
#define PROX_PROJ                          0x04
//#define	NOISE_DETECT_ENABLE			   0x08	// removed in IQS360A		
#define HALT_CHARGE                        0x10
#define TURBO_MODE                         0x20
#define XFER                               0x40
#define ACK_RESET                          0x80

/*      Settings 2	*/
#define HALT                               0x03
#define EVENT_MODE                         0x04
#define TIMEOUT_DISABLE                    0x08
#define ACF_DISABLE                        0x10
#define FORCE_HALT                         0x20
#define WDT_OFF                            0x40
#define SOFT_RESET                         0x80

/*	Settings 3	*/
#define COORD_FILTER                       0x01
#define RELATIVE_COORD                     0x02
#define RX_ON_MULTIPLE                     0x04
//#define REJECT_TOUCH                       0x08	// removed in IQS360A		
#define BETA                               0x30
//#define BORDER_CORRECT                     0x40	// removed in IQS360A	
//#define BLOCK_7_5                          0x80	// removed in IQS360A	

/*	PWM	*/
//#define COMPARE                            0x1F	// removed in IQS360A	
//#define MODE                               0xE0	// removed in IQS360A	
//#define PWM_LIMIT_BITS                     0x1F	// removed in IQS360A	
//#define PWM_SPEED_BITS                     0x0F	// removed in IQS360A	

/*	Buzzer	*/
#define BURST                              0x01
#define PERM                               0x02
#define DC                                 0x04
#define PWM_ENABLE                         0x80



#ifndef TOUCH_OFFLINE_TEST
/* Function Prototypes -------------------------------------*/
I2C_Device_t* IQS360A_Get_Device (void);
// Read
uint8_t IQS360A_Init(void);
uint8_t IQS360A_Read_Flags(I2C_Stop_t i);
uint8_t IQS360A_Read_Data(uint8_t* buffer);
uint8_t IQS360A_Check_Version(void);
uint8_t IQS360A_ATI_Status(void);
// Setup
uint8_t IQS360A_Set_Active_Channels(void);
uint8_t IQS360A_Set_Targets(void);
uint8_t IQS360A_Set_Base(void);
uint8_t IQS360A_Set_Thresholds(void);
uint8_t IQS360A_Set_ProxSettings(void);
uint8_t IQS360A_Set_Timings(void);
uint8_t IQS360A_Set_EventMode(Event_Mode_t event_on_off);
uint8_t IQS360A_Redo_ATI(void);
uint8_t IQS360A_Reseed(void);
void IQS360A_Clear_Buffers(void);
void IQS360A_Copy_Buffer(uint8_t* buffer);
#endif

#endif /*	IQS360A_H__	*/

/** @} */
