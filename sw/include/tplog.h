/**
 *  @file      tplog.h
 *  @brief     Implemented the logging service
 *  @author    Wesley Lee
 *  @date      30-Nov-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _TPLOG_H_
#define _TPLOG_H_

#include <stdio.h>
#include "commonTypes.h"

/*****************************************************************************************************************
 *
 * features
 *
 *****************************************************************************************************************/
#define TPLOG_DETAIL    // details info: tick time, source file, line number
#define TPLOG_COLOR     // log level with color on terminal
//#define TPLOG_SILENCE   // Disable log

/*****************************************************************************************************************
 *
 * configuration
 *
 *****************************************************************************************************************/
#ifdef TPLOG_DETAIL
#define TPLOG_DEBUG_HEADER  "\r\n"TPLC_G"[%u"TPLC_C" %s"TPLC_M" %d]\r\n"
#define TPLOG_HEADER    TPLOG_OUT(TPLOG_DEBUG_HEADER, getSysTime(), __FILE__, __LINE__);
#else
#define TPLOG_DEBUG_HEADER  "\r\n"TPLC_G"[%u]"
#define TPLOG_HEADER    TPLOG_OUT(TPLOG_DEBUG_HEADER, getSysTime());
#endif

#ifdef TPLOG_COLOR
#define TPLC_W  "\x1b[00m"  // white
#define TPLC_R  "\x1b[31m"  // red
#define TPLC_G  "\x1b[32m"  // green
#define TPLC_B  "\x1b[34m"  // blue
#define TPLC_C  "\x1b[36m"  // cyan
#define TPLC_M  "\x1b[35m"  // magenta
#define TPLC_Y  "\x1b[33m"  // yellow
#else
#define TPLC_W
#define TPLC_R
#define TPLC_G
#define TPLC_B
#define TPLC_C
#define TPLC_M
#define TPLC_Y
#endif

/*****************************************************************************************************************
 *
 * customization
 *
 *****************************************************************************************************************/
#define TPLOG_OUT printf

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
#ifdef TPLOG_SILENCE
#define TPLOG_DEBUG(text, ...)  ;
#define TPLOG_INFO(text, ...)   ;
#define TPLOG_WARN(text, ...)   ;
#define TPLOG_ERROR(text, ...)  ;
#define TPLOG_FATAL(text, ...)  ;
#elif defined(NDEBUG)
#define TPLOG_DEBUG(text, ...)  ;
#define TPLOG_INFO(text, ...)   TPLOG_OUT(TPLC_W"\r\n[INFO]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_WARN(text, ...)   TPLOG_OUT(TPLC_Y"\r\n[WARN]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_ERROR(text, ...)  TPLOG_OUT(TPLC_R"\r\n[ERROR]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_FATAL(text, ...)  TPLOG_OUT(TPLC_R"\r\n[FATAL]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#else
#define TPLOG_DEBUG(text, ...)  TPLOG_HEADER; TPLOG_OUT(TPLC_B"[DEBUG]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_INFO(text, ...)   TPLOG_HEADER; TPLOG_OUT(TPLC_W"[INFO]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_WARN(text, ...)   TPLOG_HEADER; TPLOG_OUT(TPLC_Y"[WARN]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_ERROR(text, ...)  TPLOG_HEADER; TPLOG_OUT(TPLC_R"[ERROR]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#define TPLOG_FATAL(text, ...)  TPLOG_HEADER; TPLOG_OUT(TPLC_R"[FATAL]: "); TPLOG_OUT(TPLC_W text, ##__VA_ARGS__);
#endif

#endif  // _TPLOG_H_

