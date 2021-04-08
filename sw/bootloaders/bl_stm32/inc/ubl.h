/**
 *  @file      ubl.h
 *  @brief     Implemented a boot-loader image which allowed to be upgraded
 *  @author    Wesley Lee
 *  @date      30-Nov-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _UBL_H_
#define _UBL_H_

#include "bl_common.h"

void UBL_cust_init(void);
void UBL_cust_before_stbl(void);

#endif

