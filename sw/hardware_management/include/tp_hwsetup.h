/**
 *  @file      tp_hwsetup.h
 *  @brief     This file includes the appropriate hardware definition based on
 *              the platform and\or the project
 *  @author    Dmitry Abdulov
 *  @date      28-Aug-2013
 *  @copyright Tymphany Ltd.
 *
 *  Example:
 *
 * #if defined PLATFORM1
 *  #include "tp_hwsetup_platform1.h"
 *  #if defined PROJECT1
 *      #include "hwsetup_project1.h"
 *  #elif (defined PROJECT2)
 *      #include "hwsetup_project2.h"
 * #elif defined PLATFORM2
 *  #include "tp_hwsetup_platform2.h"
 * #else
 *  #error "No platform\project or unknown platform defined."
 * #endif
 * 
 */

#ifndef TP_HWSETUP_H
#define TP_HWSETUP_H

#ifdef  __cplusplus
extern "C" {
#endif

#include "commonTypes.h"

#if defined MICROCHIP_PIC_32

#include "tp_hwsetup_pic32.h"

#elif defined PC_PLATFORM

#include "tp_hwsetup_pc.h"

#else

#error "No platform\project or unknown platform defined."

#endif

#ifdef  __cplusplus
}
#endif

#endif  /* TP_HWSETUP_H */

