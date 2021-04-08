/**
*  @file      drv_pb.h
*  @brief     the header file of protocol buffer driver
*  @author    Daniel Qin
*  @date      06-2017
*  @copyright Tymphany Ltd.
*/
#ifndef _DRV_PB_H_
#define _DRV_PB_H_

#include "pb_encode.h"
#include "pb_decode.h"
#include "yahdlc.h"

typedef void (* pb_send_cb)(uint8 *, uint16);

typedef struct
{
    unsigned            txSeq: 3;
    unsigned            reserved: 13;
    yahdlc_control_t    control;
    pb_send_cb          send_cb;
    const pb_field_t    *tx_pb_field;
    const pb_field_t    *rx_pb_field;
    uint8_t             *rxBuffer;
    uint8_t             *rMessage;
} drv_pb_inst_t;


/* Initial protocol driver, set send message callback function and pb_field */
void drv_pb_init(drv_pb_inst_t *me, pb_send_cb callback, const pb_field_t tx_pb_field[], const pb_field_t rx_pb_field[]);

/* Pack message with protocol buffer encode and yahdlc
* Input message pointer: pMessage
* Output packed data pointer: pMessage
* @retval -message size
*/
uint16 drv_pb_packMsg(drv_pb_inst_t *me, void *pMessage, char *txBuffer);

/* Unpack message with protocol buffer decode and yahdlc
* @retval 0 Success
* @retval -EINVAL Invalid parameter
*/
int drv_pb_unpackMsg(drv_pb_inst_t *me, const uint8 *rx, uint16 len);

#endif

