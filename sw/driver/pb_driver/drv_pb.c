/**
*  @file      drv_pb.c
*  @brief     The driver of nanopb
*  @author    Daniel Qin
*  @date      6 -2017
*  @copyright Tymphany Ltd.
*/

#include "pb_config.h"
#include "pb_adaptor.h"
#include "drv_pb.h"


#define DRV_PB_DBG_ENx
#ifdef DRV_PB_DBG_EN
#define DRV_PB_DBG(x) TP_PRINTF(x)
#else
#define DRV_PB_DBG(x)
#endif

#define YAHDLC_SEQ_NO_MAX           (7)
#define YAHDLC_NACK_MSG_LEN         (8)
#define YAHDLC_ACK_MSG_LEN          (8)

static void drv_pb_ACK(drv_pb_inst_t *me);
static void drv_pb_NACK(drv_pb_inst_t *me);
static int yahdlc_seq_advance(unsigned seq);


static uint8_t rxBuffer[PB_MSG_BUFFER_SIZE];
static uint8_t rMessage[PB_MSG_BUFFER_SIZE];

/**
 *  Init pb driver
 *
 * @return void
 */
void drv_pb_init(drv_pb_inst_t *me, pb_send_cb callback, const pb_field_t tx_pb_field[], const pb_field_t rx_pb_field[])
{
    me->txSeq = 0;
    me->send_cb = callback;
    me->tx_pb_field = tx_pb_field;
    me->rx_pb_field = rx_pb_field;

    me->rxBuffer = rxBuffer;
    me->rMessage = rMessage;
}

/* pack the message */
uint16 drv_pb_packMsg(drv_pb_inst_t *me, void *pMessage, char *txBuffer)
{
    uint16 size = 0;
    uint16 message_length = 0;
    uint16 msgBuffer[PB_MSG_BUFFER_SIZE];

    memset(txBuffer, 0, PB_MSG_BUFFER_SIZE);
    message_length = message_to_data(pMessage, (char*)msgBuffer, PB_MSG_BUFFER_SIZE, me->tx_pb_field);
    /* if data encode successfully */
    if(message_length)
    {
        yahdlc_control_t control;
        /* framing the data streaming */
        control.frame = YAHDLC_FRAME_DATA;
        control.seq_no = me->txSeq;
        me->txSeq = yahdlc_seq_advance(me->txSeq);
        yahdlc_frame_data(&control, (char*)msgBuffer, message_length, (char*)txBuffer, (unsigned int*)&size);
    }

    return size;
}

int drv_pb_unpackMsg(drv_pb_inst_t *me, const uint8 *rx, uint16 len)
{
    unsigned int write_len = 0;

    int ret = yahdlc_get_data(&me->control, (char*)rx, len, (char*)me->rxBuffer, (unsigned int*)&write_len);
    if(ret >= 0)
    {
        /* do not handle the command here as it's still in interrupt call back function */
        if(data_to_message(me->rMessage, (char*)me->rxBuffer, write_len, me->rx_pb_field)
            && (me->control.frame == YAHDLC_FRAME_DATA))
        {
            drv_pb_ACK(me);
        }
        memset(me->rxBuffer, 0, PB_MSG_BUFFER_SIZE);
    }
    else if (ret == -EIO)
    {
        drv_pb_NACK(me);
        /* throw the package if the CRC is not correct */
        memset(me->rxBuffer, 0, PB_MSG_BUFFER_SIZE);
    }
    else if (ret == -ENOMSG)
    {
        /* Do nothing in here as we need to wart for a complete frame data. */
    }
    else
    {
        ASSERT(0);
    }

    return ret;
}

static void drv_pb_ACK(drv_pb_inst_t *me)
{
    yahdlc_control_t control;
    unsigned int size = 0;
    uint8  ack_buff[YAHDLC_ACK_MSG_LEN] = {0};
    uint8* fake_src = ack_buff;    /* fake src pointer as yahdlc does not accept src to be null */

    control.frame = YAHDLC_FRAME_ACK;
    if (me->control.seq_no >= YAHDLC_SEQ_NO_MAX)
    {
        control.seq_no = 0;
    }
    else
    {
        control.seq_no= me->control.seq_no + 1;
    }
    yahdlc_frame_data(&control, (char*)fake_src, 1, (char*)ack_buff, (unsigned int*)&size);
    if (me->send_cb)
    {
        me->send_cb((uint8* )ack_buff, size);
    }

    DRV_PB_DBG(("Send ACK \r\n"));
}


static void drv_pb_NACK(drv_pb_inst_t *me)
{
    yahdlc_control_t control;
    uint16 size = 0;
    uint8  nack_buff[YAHDLC_NACK_MSG_LEN] = {0};
    uint8* fake_src = nack_buff;    /* fake src pointer as yahdlc does not accept src to be null */

    control.frame = YAHDLC_FRAME_NACK;
    control.seq_no= me->control.seq_no;
    yahdlc_frame_data(&control, (char*)fake_src, 1, (char*)nack_buff, (unsigned int*)&size);
    if (me->send_cb)
    {
        me->send_cb((uint8* )nack_buff, size);
    }
    DRV_PB_DBG(("Send NACK \r\n"));
}

static int yahdlc_seq_advance(unsigned seq)
{
    if (YAHDLC_SEQ_NO_MAX <= seq)
    {
        seq = 0;
    }
    else
    {
        ++seq;
    }

    return seq;
}


