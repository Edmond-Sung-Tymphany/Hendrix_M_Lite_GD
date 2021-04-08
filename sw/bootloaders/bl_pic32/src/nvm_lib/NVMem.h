/**
 * @file      NVMem.h
 * @brief     Header file for NV Memory operation
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef __NVMEM_H__
#define __NVMEM_H__



/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include <stdint.h>  //uint8
#include <stddef.h>  //size_t
#include <GenericTypeDefs.h>
#include "commonTypes.h"



/*****************************************************************************
 * Function Protocol                                                         *
 *****************************************************************************/
uint32 NVMemEraseApplication();


#endif
