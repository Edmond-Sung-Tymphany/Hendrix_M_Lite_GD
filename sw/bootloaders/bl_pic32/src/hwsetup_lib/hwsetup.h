/**
 *  @file      hwsetup.h
 *  @brief     This file defines the interface between the HW and the SW for the iDeck product family
 *  @author    Jerry Guo / HK Wong
 *  @date      11-2011
 *  @copyright Tymphany Ltd.
 *
 *  TODO: Split this file for every project, current setting is for MOFA only
 *
 */
#ifndef HWSETUP_H
#define HWSETUP_H


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include <p32xxxx.h>


/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/

/*********************************************/
/***** UART1 Related *****/
#define UART1_RX_BUFFER_REG     U1RXREG
#define UART1_TX_BUFFER_REG     U1TXREG
#define UART1_BAUD_RATE_REG     U1BRG
#define UART1_MODE_REG          U1MODE
#define UART1_STATUS_REG        U1STA

#define UART1_RX_BUFFER_FULL    U1STAbits.URXDA
#define UART1_TX_BUFFER_EMPTY   U1STAbits.TRMT

#define UART1_HI_BAUD_RATE_BIT  U1MODEbits.BRGH
#define UART1_LOOP_BACK_MODE    U1MODEbits.LPBACK
#define UART1_ENABLE_BIT        U1MODEbits.UARTEN //tp_uart.c used
#define UART2_ENABLE_BIT        U2MODEbits.UARTEN //tp_uart.c used
#define UART1_TX_ENABLE_BIT     U1STAbits.UTXEN   //tp_uart.c used
#define UART1_RX_ENABLE_BIT     U1STAbits.URXEN   //tp_uart.c used
#define UART2_TX_ENABLE_BIT     U2STAbits.UTXEN   //tp_uart.c used
#define UART2_RX_ENABLE_BIT     U2STAbits.URXEN   //tp_uart.c used

#define UART1_TX_INT_MODE       U1STAbits.UTXISEL
#define UART1_RX_INT_MODE       U1STAbits.URXISEL

#define UART1_INT_PRIORITY      IPC8bits.U1IP

#define UART1_OVERRUN_FLAG      U1STAbits.OERR
#define UART1_RX_INT_ENABLE_BIT IEC1bits.U1RXIE
#define UART1_RX_INT_FLAG       IFS1bits.U1RXIF
#define UART1_TX_INT_ENABLE_BIT IEC1bits.U1TXIE
#define UART1_TX_INT_FLAG       IFS1bits.U1TXIF


/****** Interrupts *****/
#define ENABLE_INTERRUPT()      INTEnableInterrupts()
#define DISABLE_INTERRUPT()     INTDisableInterrupts()


/****** Button *****/
#define    VBAT_DET_PIN         PORTAbits.RA0
#define    VDC_DET_PIN          PORTAbits.RA1
#define    CH_STATUS_PIN        1 //PORTAbits.RA4
#define    BT_STATUS_PIN        PORTBbits.RB4
#define    POWER_KEY_PIN        PORTDbits.RD0
#define    DIRECT_MODE_KEY_PIN  PORTGbits.RG8
#define    BUTTON_PIN           PORTBbits.RB15
#define    RESET_PIN            PORTBbits.RB14

#define   SAM_RESET_PIN_IO      TRISBbits.TRISB12
#define   SAM_RESET_PIN_PORT    PORTBbits.RB12
#define   SAM_RESET_PIN_LAT     LATBbits.LATB12
#define   SAM_RESET_PIN_ANSEL   ANSELBbits.ANSB12

#define    RESET_PIN_IO         TRISBbits.TRISB2
#define    RESET_PIN_ON         do{LATBSET=1<<2;TRISBbits.TRISB2=0;}while(0);
#define    RESET_PIN_OFF        do{LATBCLR=1<<2;TRISBbits.TRISB2=0;}while(0);

#define    SAM3V3_EN_PIN_IO     TRISGbits.TRISG8
#define    SAM3V3_EN_PIN_ON     do{ LATGSET = 1<<8; } while(0)
#define    SAM3V3_EN_PIN_OFF    do{ LATGCLR = 1<<8; } while(0)

#define    DC3V3_EN_PIN_ON      do{ LATBSET = 1<<6; TRISBbits.TRISB6=0;} while(0) //output high
#define    DC3V3_EN_PIN_OFF     do{ LATBCLR = 1<<6; TRISBbits.TRISB6=0;} while(0) //output low

#define    AMP_UNMUTE           do{TRISBbits.TRISB7=1;}while(0)  //input
#define    AMP_MUTE             do{LATBCLR=1<<7;  TRISBbits.TRISB7=0;}while(0) //output low

#define    RCA_OUT_UNMUTE       do{LATECLR=1<<0;  TRISEbits.TRISE0=0;}while(0) //output low
#define    RCA_OUT_MUTE         do{LATESET=1<<0;  TRISEbits.TRISE0=0;}while(0) //output high

#define    SDZ_PIN_ON           //LATBSET=0x00000400
#define    SDZ_PIN_OFF          //LATBCLR=0x00000400
#define    SDA1_PIN_OFF         LATBCLR=0x00000200
#define    SCL1_PIN_OFF         LATBCLR=0x00000100




/****************************************************************************
 * Global Variable                                                          *
 ****************************************************************************/


/****************************************************************************
 * Function Prototype                                                       *
 ****************************************************************************/
void bsp_init(void);
void sam_init(BOOL real_init);
void sam_destroy(void);
void bsp_destroy(void);
void bsp_enable_watchdog(void);
void bsp_disable_watchdog(void);
void bsp_feed_watchdog(void);


#endif