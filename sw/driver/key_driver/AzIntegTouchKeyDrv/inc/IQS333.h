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
* @file     IQS333.h                                                          *
* @brief    Header file for the IQS333 "object" specific commands             *
* @author   AJ van der Merwe - Azoteq PTY Ltd                                 *
* @version  V1.0.0                                                            *
* @date     24/08/2015                                                        *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IQS333_H
#define __IQS333_H


// User includes
#include "tch_defines.h"
#include "I2C_Comms.h"

/* Defines -------------------------------------------------*/
//#define IQS333_RDY_PIN              GPIO_Pin_5
//#define IQS333_RDY_PORT             GPIOB
//#define IQS333_RDY_PIN_SOURCE       GPIO_PinSource5
//#define IQS333_RDY_PORT_SOURCE      GPIO_PortSourceGPIOB
//#define IQS333_EXTI_IRQ             (uint32_t)EXTI9_5_IRQn
///#define IQS333_EXTI_LINE            (uint32_t)EXTI_Line5

/* I2C Address of IQS333 */
#define IQS333_ADDR                 0x64

/* IQS333 Command Structure */
// Communication Command / Address Structure on IQS333 - ie. Memory Map
#define VERSION_INFO                0x00    // Product number can be read      : 2 bytes
#define FLAGS                       0x01    // System flags and events         : 1 byte
#define WHEEL_COORDS                0x02    // Wheel coordinates - 2 wheels    : 4 bytes
#define TOUCH_BYTES                 0x03    // Touch channels                  : 2 bytes
#define COUNTS                      0x04    // Count Values                    :20 bytes
#define LTA_REG                     0x05    // LTA Values                      :20 bytes
#define MULTIPLIERS                 0x06    // Multipliers Values              :10 bytes
#define COMPENSATION                0x07    // Compensation Values (PCC)       :10 bytes
#define PROXSETTINGS                0x08    // Prox Settings - Various         : 6 bytes
#define THRESHOLDS                  0x09    // Threshold Values                :10 bytes
#define TIMINGS                     0x0A    // Timings                         : 5 bytes
#define ATI_TARGETS                 0x0B    // Targets for ATI                 : 2 bytes
#define PWM                         0x0C    // PWM Settings                    : 4 bytes
#define PWM_LIMIT                   0x0D    // PWM Limits and speed            : 2 bytes
#define ACTIVE_CHANNELS             0x0E    // Active channels                 : 2 bytes
#define BUZZER                      0x0F    // Buzzer                          : 1 byte

/* Base Values */
#define BASE_75                     0x40    // Base 75
#define BASE_100                    0x80    // Base 100
#define BASE_150                    0xC0    // Base 150
#define BASE_200                    0x00    // Base 200 (default)

/* Channel Filter Bits */
#define CH0_B             0x01
#define CH1_B             0x02
#define CH2_B             0x04
#define CH3_B             0x08
#define CH4_B             0x10

/* Bit Definitions    */
/*    System Flags    */
#define ZOOM                            0x01
#define NOISE                           0x02
#define ATI_BUSY                        0x04
#define LP_ACTIVE                       0x08
#define IS_CH0                          0x10
#define PROJ_MODE                       0x20
#define FILTER_HALTED                   0x40
#define SHOW_RESET                      0x80

/*    Multipliers      */
#define SENS_MUTLIPLIERS                0x0F
#define COMP_MULITPLIERS                0x30
#define BASE_VALUE                      0x0C

/*    ProxSettings      */
/*    ProxSettings 0    */
#define PROJ_BIAS                       0x03
#define CS_SIZE                         0x04
#define RESEED                          0x08
#define REDO_ATI                        0x10
#define ALT_ATI                         0x20
#define PARTIAL_ATI                     0x40
#define ATI_OFF                         0x80

/*    ProxSettings 1    */
#define BAND                            0x01
#define IQS_ERROR                       0x02
#define CH0_1TX                         0x04
#define HALT_CHARGE                     0x10
#define TURBO_MODE                      0x20
#define CHARGE_TRANSFER_SPEED           0x0C

/*    ProxSettings 2    */
#define HALT                            0x03
#define EVENT_MODE                      0x04
#define TIMEOUT_DISABLE                 0x08
#define ACF_DISABLE                     0x10
#define FORCE_HALT                      0x20
#define WDT_OFF                         0x40
#define SOFT_RESET                      0x80

/*    ProxSettings 3    */
#define ACK_RESET                       0x01
#define WHEEL1_DISABLE                  0x02
#define WHEEL2_DISABLE                  0x04
#define WHEEL_FILTER_DISABLE            0x08
#define WHEEL_RESOLUTION                0x70

/* Function Prototypes -------------------------------------*/
I2C_Device_t* IQS333_Get_Device (void);
// Read
uint8_t IQS333_Init(void);
uint8_t IQS333_Read_Flags(void);
uint8_t IQS333_Read_Data(uint8_t* buffer);
uint8_t IQS333_Check_Version(void);
uint8_t IQS333_ATI_Status(void);
// Setup
uint8_t IQS333_Set_Active_Channels(void);
uint8_t IQS333_Set_Targets(void);
uint8_t IQS333_Set_Base(void);
uint8_t IQS333_Set_Thresholds(void);
uint8_t IQS333_Set_ProxSettings(void);
uint8_t IQS333_Set_Timings(void);
uint8_t IQS333_Set_EventMode(Event_Mode_t event_on_off);
uint8_t IQS333_Redo_ATI(void);
uint8_t IQS333_Reseed(void);
void IQS333_Clear_Buffers(void);
void IQS333_Copy_Buffer(uint8_t* buffer);


#endif /* __IQS333_H */
