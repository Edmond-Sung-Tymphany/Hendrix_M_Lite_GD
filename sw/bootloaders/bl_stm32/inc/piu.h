/**
 *  @file      piu.h
 *  @brief     Implemented a boot-loader which will NOT be updated through OTA
 *  @author    Wesley Lee
 *  @date      30-Nov-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _PIU_H_
#define _PIU_H_

#include "bl_common.h"

void PIU_cust_init(void);
void PIU_cust_new_init(void);
void PIU_cust_normal_init(void);
void PIU_cust_before_stbl(void);

#endif

