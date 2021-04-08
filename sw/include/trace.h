/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/**
*  @file      trace.h
*  @brief     This the trace library interface file.
*  @author    Wesley Lee
*  @date      19-Jul-2013
*  @copyright Tymphany Ltd.
*/
#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>
#include "ProjBsp.h"

#ifdef PC_PLATFORM

#define TRACE_ERR(str)  printf("<%s %d> %s\n", __FILE__, __LINE__, str);
#define TRACE(x)        printf("<%s %d> %s: %d\n", __FILE__, __LINE__, #x, x);
#define ASSERT(x)       if(!(x)) printf("Assert: %s %d\n", __FILE__, __LINE__);
#define TP_PRINTF       printf

#else   // !PC_PLATFORM
#include "qp_port.h"

#ifdef NDEBUG

#define TRACE_ERR(str)
#define TRACE(x)
#define ASSERT(x)
#define TP_PRINTF(...)

#else   // !NDEBUG

#define TRACE_ERR(str)  printf("<%s %d> %s\n", __FILE__, __LINE__, str);
#define TRACE(x)        printf("<%s %d> %s: %d\n", __FILE__, __LINE__, #x, x);
#define ASSERT(x)       if(!(x)) Q_onAssert(__FILE__, __LINE__)
#define TP_PRINTF       printf

#endif  // NDEBUG

#ifdef HAS_PROJ_PRINTF
    #undef TP_PRINTF
    #define TP_PRINTF(...)    ProjBsp_Printf(/*toAsetk:*/TRUE, __FILE__, __LINE__, __VA_ARGS__)
#endif

#endif  // PC_PLATFORM

#endif    /* TRACING_H */

