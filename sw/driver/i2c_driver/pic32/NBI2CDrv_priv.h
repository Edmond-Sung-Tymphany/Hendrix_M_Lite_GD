/**
 * @file        NBI2CDrv_priv.h
 * @brief       This file defines the implementation of non-blcoking I2C driver
 * @author      Bob.Xu 
 * @date        2014-04-25
 * @copyright   Tymphany Ltd.
 */
#ifndef NBI2C_DRIVER_PRIVATE_H
#define NBI2C_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "NBI2CDrv.h"

/* private functions / data */
#ifndef I2CDRV_PRIV_H
#define	I2CDRV_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif


#define    SET_I2C1_SCL_INPUT     TRISDbits.TRISD10 = 1
#define    SET_I2C1_SDA_INPUT     TRISDbits.TRISD9 = 1

#define    SET_I2C1_SCL_OUTPUT    TRISDbits.TRISD10 = 0
#define    SET_I2C1_SDA_OUTPUT    TRISDbits.TRISD9 = 0

/*** PIC32 I2C DEFINES ***/
#define I2C_ONE                                 1
#define I2C_TWO                                 2
#define I2C_READBUF_TIMEOUT                     0xFFFF
#define I2C_TRANSMIT_TIMEOUT                    0xFFFF
#define I2C_STOP_TIMEOUT                        0xFFFF
#define I2C_START_TIMEOUT                       0xFFFF
#define I2C_ISTBUFFULL_TIMEOUT                  0xFFFF
#define I2C_START_INIT_DELAY                    0xFFFF

#define I2C_BAUD_RATE                           100000
#define I2C_BRG                                 255             /* To make sure the I2C clock is around 100k */

#define LOW_I2C_SPEED_I2C                       0x32
#define DEFAULT_I2C_SPEED                       0x09


#define I2C_CON_MASK                            0x001F
#define I2C_STAT_MASK                           0x4000


/**********************
 *
 * Macros to hide the actual I2C handling
 * These subsitite the chan value so you may call any iic channel
 *
 *
 *********************/
#define I2C_HW_START_ENABLE_(chan)                   I2C##chan##CONbits.SEN=1

#define I2C_INTERRUPT_CLR_(chan)                     IFS1bits.I2C##chan##MIF=0

#define I2C_INTERRUPT_FLAG_(chan)                    IFS1bits.I2C##chan##MIF


// Stop Condition
#define I2C_HW_STOP_COND_ENABLE_(chan)               I2C##chan##CONbits.PEN=1
#define I2C_IS_HW_STOP_COND_ENABLED_(chan)           (I2C##chan##CONbits.PEN==1)

#define I2C_HW_REPEATED_START_COND_ENABLE_(chan)     I2C##chan##CONbits.RSEN=1
#define I2C_IS_HW_REPEATED_START_COND_ENABLED_(chan) (I2C##chan##CONbits.RSEN==1)

#define I2C_SEND_RCVD_ACK_(chan)                     I2C##chan##CONbits.ACKDT=0
#define I2C_SEND_RCVD_NOT_ACK_(chan)                 I2C##chan##CONbits.ACKDT=1

#define I2C_HW_START_ACK_SEQ_(chan)                  I2C##chan##CONbits.ACKEN=1
#define I2C_IS_HW_ACK_SEQ_ACTIVE_(chan)              (I2C##chan##CONbits.ACKEN==1)

#define I2C_ENABLE_RECEIVE_MODE_(chan)               I2C##chan##CONbits.RCEN=1
#define I2C_DISABLE_RECEIVE_MODE_(chan)              I2C##chan##CONbits.RCEN=0

#define I2C_IS_BUFFER_FULL_(chan)                    (I2C##chan##STATbits.RBF==1)
#define I2C_RECEIVE_BUF_(chan)                       I2C##chan##RCV

#define I2C_IS_TBUFFER_FULL_(chan)                   (I2C##chan##STATbits.TBF==1)
#define I2C_TRANSMIT_IN_PROGRESS_(chan)              (I2C##chan##STATbits.TRSTAT==1)
#define I2C_IS_NACK_RCV_(chan)                       (I2C##chan##STATbits.ACKSTAT==1)
#define I2C_IS_ACK_RCV_(chan)                        (I2C##chan##STATbits.ACKSTAT==0)


#define I2C_ENABLE_(chan)                            I2C##chan##CONbits.ON=1
#define I2C_DISABLE_(chan)                           I2C##chan##CONbits.ON=0

#define I2C_CLEAR_WRITECOLLISION_DBIT_(chan)         I2C##chan##STATbits.IWCOL=0
#define I2C_CLEAR_MASTERBUSCOLLISION_DBIT_(chan)     I2C##chan##STATbits.BCL=0

#define I2C_BAUDRATE_SET_(chan, val)                 I2C##chan##BRG=val
#define I2C_BAUDRATE_GET_(chan)                      I2C##chan##BRG

#define I2C_SP_CON_(chan)                            I2C##chan##CON
#define I2C_SP_STAT_(chan)                           I2C##chan##STAT

#define I2C_TRANSMIT_BUF_(chan)                      I2C##chan##TRN

/**********************
 *
 * Actual macros to use with I2C
 * chan == channel
 *
 *********************/
#define I2C_HW_START_ENABLE(chan)                       I2C_HW_START_ENABLE_(chan)


#define I2C_CLEAR_INTERRUPT(chan)                       I2C_INTERRUPT_CLR_(chan)
#define I2C_INTERRUPT_FLAG(chan)                        I2C_INTERRUPT_FLAG_(chan)

// Stop Condition
#define I2C_HW_STOP_COND_ENABLE(chan)                   I2C_HW_STOP_COND_ENABLE_(chan)
#define I2C_IS_HW_STOP_COND_ENABLED(chan)               I2C_IS_HW_STOP_COND_ENABLED_(chan)

#define I2C_HW_REPEATED_START_COND_ENABLE(chan)         I2C_HW_REPEATED_START_COND_ENABLE_(chan)
#define I2C_IS_HW_REPEATED_START_COND_ENABLED(chan)     I2C_IS_HW_REPEATED_START_COND_ENABLED_(chan)

#define I2C_SEND_RCVD_ACK(chan)                         I2C_SEND_RCVD_ACK_(chan)
#define I2C_SEND_RCVD_NOT_ACK(chan)                     I2C_SEND_RCVD_NOT_ACK_(chan)

#define I2C_HW_START_ACK_SEQ(chan)                      I2C_HW_START_ACK_SEQ_(chan)
#define I2C_IS_HW_ACK_SEQ_ACTIVE(chan)                  I2C_IS_HW_ACK_SEQ_ACTIVE_(chan)

#define I2C_ENABLE_RECEIVE_MODE(chan)                   I2C_ENABLE_RECEIVE_MODE_(chan)
#define I2C_DISABLE_RECEIVE_MODE(chan)                  I2C_DISABLE_RECEIVE_MODE_(chan)

#define I2C_IS_BUFFER_FULL(chan)                        I2C_IS_BUFFER_FULL_(chan)
#define I2C_RECEIVE_BUF(chan)                           I2C_RECEIVE_BUF_(chan)

#define I2C_IS_TBUFFER_FULL(chan)                       I2C_IS_TBUFFER_FULL_(chan)
#define I2C_TRANSMIT_IN_PROGRESS(chan)                  I2C_TRANSMIT_IN_PROGRESS_(chan)
#define I2C_IS_NACK_RCV(chan)                           I2C_IS_NACK_RCV_(chan)
#define I2C_IS_ACK_RCV(chan)                            I2C_IS_ACK_RCV_(chan)


#define I2C_ENABLE(chan)                                I2C_ENABLE_(chan)
#define I2C_DISABLE(chan)                               I2C_DISABLE_(chan)

#define I2C_CLEAR_WRITECOLLISION_DBIT(chan)             I2C_CLEAR_WRITECOLLISION_DBIT_(chan)
#define I2C_CLEAR_MASTERBUSCOLLISION_DBIT(chan)         I2C_CLEAR_MASTERBUSCOLLISION_DBIT_(chan)

#define I2C_BAUDRATE_SET(chan, val)                     I2C_BAUDRATE_SET_(chan, val)
#define I2C_BAUDRATE_GET(chan)                          I2C_BAUDRATE_GET_(chan)
#define I2C_SP_CON(chan)                                I2C_SP_CON_(chan)
#define I2C_SP_STAT(chan)                               I2C_SP_STAT_(chan)
#define I2C_TRANSMIT_BUF(chan)                          I2C_TRANSMIT_BUF_(chan)

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

#ifdef __cplusplus
}
#endif

#endif /* NBI2C_DRIVER_PRIVATE_H */