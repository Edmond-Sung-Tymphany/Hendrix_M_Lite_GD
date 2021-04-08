/**
 * @file      Triggers.h
 * @brief     Implements the bootload mode trigger function
 * @author    Gavin Lee
 * @date      16-Mar-2014
 * @copyright Tymphany Ltd.
 */

#ifndef TRIGGERS_H
#define TRIGGERS_H

/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include "BootLoader.h" //each file must include Bootloader.h
#include <stdint.h> // uint8
#include <stddef.h>  //size_t
#include <GenericTypeDefs.h>
#include "commonTypes.h"
#include "NvmDrv.h"


/*****************************************************************************
 * Function Protocol                                                         *
 *****************************************************************************/
BOOL check_trigger(void);
void clear_trigger(void);


#endif    /* TRIGGERS_H */

