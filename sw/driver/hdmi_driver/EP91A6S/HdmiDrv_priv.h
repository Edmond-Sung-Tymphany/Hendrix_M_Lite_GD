/*****************************************************************************
* Model: tym_platform.qm
* File:  F:\Project_Atmos\tymphany_platform\sw/driver\include\HdmiDrv.h
*
* This code has been generated by QM tool (see state-machine.com/qm).
* DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
*
* This program is open source software: you can redistribute it and/or
* modify it under the terms of the GNU General Public License as published
* by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
* for more details.
*****************************************************************************/
/*${F:\Project_Atmos~::driver\include\HdmiDrv.h} ...........................*/
/**
 *  @file      hdmi_driver.h
 *  @brief     This file contains the EP91A6S HDMI driver implementation.
 *  @author    Albert
 *  @date      23-May-2016
 */
#ifndef HDMI_DRIVER__PRIV_H
#define HDMI_DRIVER__PRIV_H

#ifdef STM32F0XX
#include "stm32f0xx.h"
#endif

/* ---- Begin ---------------Register Setting of HDMI Chip */
#define SA_IF_ADDR 0xC8

#define HDMI_REG_X8 0x8
// [7]Power, <0>off, <1>on
// [5]Audio path, <0>TV speaker, <1>external speaker
// [2]CEC dis, <0>on, <1>off
// [1]CEC mute, <0>unmute, <1>mute
// [0]ARC en, <0>disable, <1>enable
#define HDMI_REG_X10 0x10

// [3:0]RX_Sel, <1>port1, <6>Non HDMI port
#define HDMI_REG_X11 0x11

#define HDMI_REG_X12 0x12

// [7:0]Volume, 0~100
#define HDMI_REG_X13 0x13

// [1:0]ARC support, <00>default, <10>success, <01>fail
// ## host should read this reg and write 00 to it ##
#define HDMI_REG_X14 0x14

/* for HDMI_REG_X10 */
#define HDMI_BITMASK_POWER (1<<7)
#define HDMI_BITMASK_AUDIO_PATH (1<<5)
#define HDMI_BITMASK_CEC_DISABLE (1<<2)
#define HDMI_BITMASK_CEC_MUTE (1<<1)
#define HDMI_BITMASK_ARC_ENABLE (1<<0)

#define HDMI_BITMASK_POWER_ON 1
#define HDMI_BITMASK_POWER_OFF 0
#define HDMI_BITMASK_AUDIO_PATH_AMP 1
#define HDMI_BITMASK_AUDIO_PATH_TV 0
#define HDMI_BITMASK_CEC_DISABLE_ON 1
#define HDMI_BITMASK_CEC_DISABLE_OFF 0
#define HDMI_BITMASK_CEC_MUTE_ON 1
#define HDMI_BITMASK_CEC_MUTE_OFF 0
#define HDMI_BITMASK_ARC_ENABLE_ON 1
#define HDMI_BITMASK_ARC_ENABLE_OFF 0

/* for HDMI_REG_X11 */
#define HDMI_BITMASK_PORT (0x0F)

/* for HDMI_REG_X14 */
#define HDMI_BITMASK_ARC_SUCCESS (1<<1)
#define HDMI_BITMASK_ARC_FAIL (1<<0)
#define HDMI_BITMASK_ARC_CLEAR ((1<<0)|(1<<1))

/* ---- End ---------------Register Setting of HDMI Chip */
typedef struct tagREG_8 {
    unsigned char TV_ARC_ON  :1;
    unsigned char Rx_HDCP1x  :1;
    unsigned char Rx_HDCP2x  :1;
    unsigned char Tx_HDCP2x  :1;
    unsigned char Rx_Hot_Plug:1;
    unsigned char Rx_DDC     :1;
    unsigned char TX_HDMI    :1;
    unsigned char Tx_Hot_Plug:1;
} REG_8;

typedef struct tagREG_9 {
    unsigned char ADO_CHF   :1;    //format change
    unsigned char CEC_ECF   :1;    //CEC event
    unsigned char unused_2  :4;
    unsigned char I2C_Ready :1;
    unsigned char unused_1  :1;
} REG_9;

typedef struct tagREG_10 {
    unsigned char ARC_EN     :1;
    unsigned char CEC_Mute   :1;
    unsigned char CEC_DIS    :1;
    unsigned char Video_Path :1;
    unsigned char A_Reset    :1;
    unsigned char Audio_Path :1;
    unsigned char unused     :1;
    unsigned char Power      :1;
} REG_10;

typedef struct tagREG_11 {
    unsigned char RX2_Sel   :4;
    unsigned char ARP_FREQ  :4;
} REG_11;

typedef struct tagREG_12 {
    unsigned char RX2_Sel    :4;
    unsigned char Unlock     :1;
    unsigned char unused_2   :1;
    unsigned char ARC_DIS    :1;
    unsigned char unused_1   :1;
} REG_12;

typedef struct tagREG_14 {
    unsigned char LinkON0       :1;
    unsigned char LinkON1       :1;
    unsigned char LinkON2       :1;
    unsigned char LinkON3       :1;
    unsigned char PD_Detect     :1;
    unsigned char VMode_Select  :1;
    unsigned char Dual_In       :1;
    unsigned char VMode_Enable  :1;
} REG_14;

#endif /* HDMI_DRIVER__PRIV_H */
