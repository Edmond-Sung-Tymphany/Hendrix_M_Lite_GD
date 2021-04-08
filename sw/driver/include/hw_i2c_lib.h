/**
 *  @file      hw_i2c_lib.h
 *  @brief     This file presents a low level interface to communicate via the i2C bus
 *  @author    Alfredo Fernandez Franco
 *  @date      11-2011
 *  @copyright Tymphany Ltd.
 */
#ifndef HW_I2C_LIB_H
#define HW_I2C_LIB_H
//#include "hwsetup.h"
//#include "product.h"
#include "commonTypes.h"

#define     SDA1_PIN_OFF           LATBCLR=0x00000200
#define     SCL1_PIN_OFF           LATBCLR=0x00000100



void I2C_init(void);
void I2C_exit(void);

void I2C_HW_Reset(void);



void IIC_communication_TX(uint8 device_add, uint8 bytes, const uint8 *data);
void IIC_communication_RX(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes);

/************* Library Usage Example *************/

/***** I2C Related pins and Macros *****/
#define I2C_BAUD_RATE               100000
#define I2C_BRG                     255//157             /* To make sure the I2C clock is around 100k */
//#define I2C_BRG                     39             /* To make sure the I2C clock is around 100k */



#define I2C_HW_START_COND_ENABLE()        I2C1CONbits.SEN=1
#define I2C_INTERRUPT_FLAG                    IFS1bits.I2C1MIF 
// Stop Condition
#define I2C_HW_STOP_COND_ENABLE()            I2C1CONbits.PEN=1
#define I2C_IS_HW_STOP_COND_ENABLED()    (I2C1CONbits.PEN==1)

#define I2C_HW_REPEATED_START_COND_ENABLE()     I2C1CONbits.RSEN=1
#define I2C_IS_HW_REPEATED_START_COND_ENABLED() (I2C1CONbits.RSEN==1)

#define I2C_SEND_RCVD_ACK()                     I2C1CONbits.ACKDT=0
#define I2C_SEND_RCVD_NOT_ACK()                 I2C1CONbits.ACKDT=1

#define I2C_HW_START_ACK_SEQ()                  I2C1CONbits.ACKEN=1
#define I2C_IS_HW_ACK_SEQ_ACTIVE()              (I2C1CONbits.ACKEN==1)

#define I2C_ENABLE_RECEIVE_MODE()               I2C1CONbits.RCEN=1
#define I2C_DISABLE_RECEIVE_MODE()              I2C1CONbits.RCEN=0

#define I2C_IS_BUFFER_FULL()                    (I2C1STATbits.RBF==1)
#define I2C_RECEIVE_BUF                         I2C1RCV

#define I2C_SP_CON                              I2C1CON
#define I2C_SP_STAT                             I2C1STAT
#define I2C_CON_MASK                            0x001f
#define I2C_STAT_MASK                           0x4000

#endif
