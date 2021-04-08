/**
 * @file      assert.h
 * @brief     Assert Function
 * @author    Gavin Lee
 * @date      10-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef ASSERT_H
#define    ASSERT_H

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "Bootloader.h"
#include "tp_uart.h"
#include "util.h"


/*****************************************************************************
 * Macro                                                                     *
 *****************************************************************************/
void assert_handler(const char* p_file, uint32 line);
#define assert(exp) {\
    if ( !(exp) ) { \
        assert_handler(__FILE__, __LINE__); \
    }\
}\


#endif    /* ASSERT_H */

