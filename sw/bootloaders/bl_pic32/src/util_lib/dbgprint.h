/**
 * @file      dbgprint.h
 * @brief     Header file for dbgprint
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"


/****************************************************************************
 * Function Prototype                                                       *
 ****************************************************************************/
void uart_console_init();
void uart_console_destroy();
size_t console_read_stream(uint8 *buf, size_t size);
size_t console_write_stream(uint8 *buf, size_t len);

#ifdef BL_MSG_PRINT
    void dbgprint(const char *fmt, ...);
    void dbgprint_data( uint8* buf, uint32 len,  char *header,  char *tail);
    #define DBG_PRINT(...) dbgprint(__VA_ARGS__)
    #define DBG_PRINT_DATA(...) dbgprint_data(__VA_ARGS__)
#else
    #define DBG_PRINT(...)
    #define DBG_PRINT_DATA(...)
#endif



#endif /* __DBGPRINT_H__ */
