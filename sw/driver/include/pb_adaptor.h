/**
*  @file      pb_adaptor.h
*  @brief     the header file of protocol buffer adaptor
*  @author    Daniel Qin
*  @date      07-2017
*  @copyright Tymphany Ltd.
*/
#ifndef _PB_ADAPTOR_H_
#define _PB_ADAPTOR_H_

#include "pb_encode.h"
#include "pb_decode.h"

/* Encode message with protocol buffer */
uint16 message_to_data(void *p_message, char* buff, uint16 size_of_buff, const pb_field_t fields[]);
/* Dncode message with protocol buffer */
bool data_to_message(void *p_message, char* buff, uint16 size, const pb_field_t fields[]);

#endif

