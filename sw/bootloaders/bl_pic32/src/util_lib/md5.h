/**
 * @file      md5.h
 * @brief     The header file describe the MD5
 * @author    Gavin Lee
 * @date      14-Aug-2015
 * @copyright Tymphany Ltd.
 */
#ifndef MD5_H
#define MD5_H

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"

/*****************************************************************************
 * Definition                                                                *
 *****************************************************************************/
#define MD5_LEN 16

/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
char* md5_to_str(unsigned char* md5_buf);
void md5_init(void);
void md5_update (const char *input, int inputlen);
unsigned char *md5_final();



#endif    /* MD5_H */

