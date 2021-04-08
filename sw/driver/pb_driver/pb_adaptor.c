/**
*  @file      pb_adaptor.c
*  @brief     The adaptor of nanopb
*  @author    Daniel Qin
*  @date      7 -2017
*  @copyright Tymphany Ltd.
*/
#include "pb_config.h"
#include "pb_adaptor.h"

#define PB_ADAPTOR_DBG_EN
#ifdef PB_ADAPTOR_DBG_EN
#define PB_ADAPTOR_DBG(x) TP_PRINTF(x)
#else
#define PB_ADAPTOR_DBG(x)
#endif

bool data_to_message(void *p_message, char* buff, uint16 size, const pb_field_t fields[])
{
    bool status = FALSE;

    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer((uint8*)buff, size);
    /* Now we are ready to decode the message. */
    status = pb_decode(&stream, fields, p_message);

    /* Check for errors... */
    if (!status)
    {
        PB_ADAPTOR_DBG(("Pb_decode error!\r\n"));
        return FALSE;
    }
    return TRUE;
}

uint16 message_to_data(void *p_message, char* buff, uint16 size_of_buff, const pb_field_t fields[])
{
    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer((uint8*)buff, size_of_buff);

    bool status = pb_encode(&stream, fields, p_message);

    uint16 message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status)
    {
        PB_ADAPTOR_DBG(("Protocol_buffer: Encoding failed\r\n"));
        return 0;
    }
    return message_length;
}

