/**
 * @file      ui.h
 * @brief     The header file describe the UI behavior
 * @author    Gavin Lee
 * @date      24-Apr-2014
 * @copyright Tymphany Ltd.
 */

#ifndef UI_H
#define UI_H



/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"


/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
void update_ui(uint32 time_ms);
void dfu_mode_enable(BOOL enable);



#endif    /* UI_H */

