/**
 *  @file      tp_hwsetup_pic32.h
 *  @brief     This file defines the interface between the HW and the SW
 *              for the PIC32 family
 *  @author    Dmitry Abdulov
 *  @date      28-Aug-2013
 *  @copyright Tymphany Ltd.
 */

#ifndef TP_HWSETUP_PIC32_H
#define	TP_HWSETUP_PIC32_H

#ifdef	__cplusplus
extern "C" {
#endif

/* Clock Constants */
#define SYS_FREQ               (40000000)
#define CPU_OSC_FREQ_HZ         SYS_FREQ
#define PERIPHERAL_CLOCK        GetPeripheralClock(CPU_OSC_FREQ_HZ)
#define INSTRUCTION_CLOCK       (4 * CPU_OSC_FREQ_HZ / 2)
#define GetPeripheralClock(SystemClockHz)   (SystemClockHz/(1 << OSCCONbits.PBDIV))

/* for timer */
#define CORE_COUNT_PER_MS       (CPU_OSC_FREQ_HZ/2/1000)
#define CORE_COUNT_PER_US       (CPU_OSC_FREQ_HZ/2/1000000)    

/*** PIC32 I2C DEFINES ***/
#define I2C_READBUF_TIMEOUT                     0xFFFF  
#define I2C_TRANSMIT_TIMEOUT                    0xFFFF
#define I2C_STOP_TIMEOUT                        0xFFFF
#define I2C_START_TIMEOUT                       0xFFFF
#define I2C_ISTBUFFULL_TIMEOUT                  0xFFFF
#define I2C_START_INIT_DELAY                    0xFFFF  

#define I2C_BAUD_RATE               100000
#define I2C_BRG                     255             /* To make sure the I2C clock is around 100k */

#define LOW_I2C_SPEED_I2C   0x32
#define DEFAULT_I2C_SPEED   0x09


#define I2C_HW_START_COND_ENABLE()              I2C1CONbits.SEN=1
#define I2C_INTERRUPT_FLAG                      IFS1bits.I2C1MIF

#define I2C_CLEAR_INTERRUPT()                   IFS1bits.I2C1MIF=0

// Stop Condition
#define I2C_HW_STOP_COND_ENABLE()               I2C1CONbits.PEN=1
#define I2C_IS_HW_STOP_COND_ENABLED()           (I2C1CONbits.PEN==1)

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

#define I2C_IS_TBUFFER_FULL()                   (I2C1STATbits.TBF==1)
#define I2C_TRANSMIT_IN_PROGRESS()              (I2C1STATbits.TRSTAT==1)
#define I2C_IS_NACK_RECEIVED()                  (I2C1STATbits.ACKSTAT==1)
#define I2C_IS_ACK_RCVD_FROM_SLAVE()            (I2C2STATbits.ACKSTAT==0)


#define I2C_ENABLE()                            I2C1CONbits.ON=1
#define I2C_DISABLE()                           I2C1CONbits.ON=0

#define I2C_CLEAR_WRITECOLLISION_DBIT()         I2C1STATbits.IWCOL=0
#define I2C_CLEAR_MASTERBUSCOLLISION_DBIT()     I2C1STATbits.BCL=0

#define I2C_BAUDRATE_REG                        I2C1BRG

#define I2C_SP_CON                              I2C1CON
#define I2C_SP_STAT                             I2C1STAT

#define I2C_TRANSMIT_BUF                        I2C1TRN

#define I2C_CON_MASK                            0x001F
#define I2C_STAT_MASK                           0x4000
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/



#ifdef	__cplusplus
}
#endif

#endif	/* TP_HWSETUP_PIC32_H */

