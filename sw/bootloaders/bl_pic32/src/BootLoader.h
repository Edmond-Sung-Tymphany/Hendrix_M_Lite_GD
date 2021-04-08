/**
 * @file      Bootloader.h
 * @brief     Header file for Polk CamdenSquare bootloader
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdint.h> // uint8
#include <stddef.h>  //size_t
#include <GenericTypeDefs.h>
#include "commonTypes.h"
#include "HardwareProfile.h"


/****************************************************************************
 * Feature                                                                  *
 ****************************************************************************/
/* 
 * How to verify bootloader with PC:
 * 1. Enable BL_FORCE_UPGRADE in Bootloader.h, to let bootloaader always stay in upgrading mode, and can not leave
 * 2. For Polk CamdenSquare:
 *      Set uart_table[] to {UART2, UART_NONE} in Uart_PolkCamdenSquare.c.
 * 3. When boot up, bootloader flahs the green LED and wait for image.
 * 3. Execite \tymphany_platform\sw\bootloaders\pic32_bootloaders\PC application\PIC32UBL.exe
 *      Then choose application image (hex file), and click [erase-program-verify]
 * 4. For Polk CamdenSquare: Press SW2 to jump to application
 *    For USB Starter Kit: Press Power button to jump to application
*/
//#define BL_FORCE_UPGRADE

/* Use allplay simulator instead of Qualcomm Allplay library.
 * It help to test bootloader before receiving Allplay library. */
//#define BL_ALLPLAY_SIMULATOR

/* Print message to UART_CONSOLE.
 * Does not support for USB Starter Kit
 */
#ifndef NDEBUG //if debug mode
#define BL_MSG_PRINT
#endif

/* The macro should NOT be enabled only when you are
 * going to build a release for SAM wifi RF test.
 */
//#define MOFA_WIFI_RF_TEST

/* Enable echo server for testing
 * Must define BL_MSG_PRINT
 */
//#define BL_UART_ECHO_SERVER

#endif /* __BOOTLOADER_H__ */
