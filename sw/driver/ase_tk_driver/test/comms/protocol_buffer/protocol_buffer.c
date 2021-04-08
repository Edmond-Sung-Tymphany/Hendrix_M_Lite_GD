/**
*  @file      protocol buffer.c
*  @brief     protocol buffer source file to use nanopb
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include "tym_type.h"
#include "protocol_buffer.h"
#include "pb_encode.h"
#include "pb_decode.h"
#include "ase_tk.pb.h"
#include "framing.h"


/* Debug switch */
#define PROTOCOL_BUFFER_DEBUG_ENABLE
#ifdef PROTOCOL_BUFFER_DEBUG_ENABLE
#define PROTOCOL_BUFFER_DEBUG(x) {printf x;}
#else
#define PROTOCOL_BUFFER_DEBUG(x)
#endif

#define PROTOCOL_BUFFER_ERROR_ENABLE
#ifdef PROTOCOL_BUFFER_ERROR_ENABLE
#define PROTOCOL_BUFFER_ERROR(x) {printf x;}
#else
#define PROTOCOL_BUFFER_ERROR(x)
#endif

static protocol_buffer_func pReceive;

static tAseTkMessage receive_message;

#define BUFFER_SIZE  (100)
static char buff[BUFFER_SIZE];

static void receive_package(char* buff, uint32 size);
static int data_to_message(tAseTkMessage* p_message, char* buff, uint32 size);
static uint32 message_to_data(tAseTkMessage* p_message, char* buff, uint32 size_of_buff);



int protocol_buffer_init()
{
    framing_init();
    framing_register_receive_cb(receive_package);
}

void protocol_buffer_deinit()
{
    framing_deinit();
}

void protocol_buffer_register_receive_cb(protocol_buffer_func func)
{
    pReceive = func;
}


int protocol_buffer_scan()
{
    framing_scan();
}

int protocol_buffer_write(tAseTkMessage* p_message)
{
    uint32 size = message_to_data(p_message, buff, BUFFER_SIZE);
    if(size > 0)
    {
        PROTOCOL_BUFFER_DEBUG(("Protocol_buffer: => Framing\r\n"));
        framing_write(buff, size);
    }
}

static void receive_package(char* buff, uint32 size)
{
    PROTOCOL_BUFFER_DEBUG(("Protocol_buffer: Receive the package\r\n"));
    data_to_message(&receive_message, buff, size);
    PROTOCOL_BUFFER_DEBUG(("Protocol_buffer: => Ase_tk_ctrl\r\n"));
    if(pReceive!= NULL)
        pReceive(&receive_message);
}


static int data_to_message(tAseTkMessage* p_message, char* buff, uint32 size)
{
    /* Create a stream that reads from the buffer. */
    pb_istream_t stream = pb_istream_from_buffer(buff, size);

    /* Now we are ready to decode the message. */
    int status = pb_decode(&stream, tAseTkMessage_fields, p_message);

    /* Check for errors... */
    if (!status)
    {
        PROTOCOL_BUFFER_ERROR(("Protocol_buffer: Can not analyze the protocol buffer\n"));
        return -1;
    }
    PROTOCOL_BUFFER_DEBUG(("Protocol_buffer: Data decode to Message\r\n"));
    return 1;
}


static uint32 message_to_data(tAseTkMessage* p_message, char* buff, uint32 size_of_buff)
{
    /* Create a stream that will write to our buffer. */
    pb_ostream_t stream = pb_ostream_from_buffer(buff, size_of_buff);

    bool status = pb_encode(&stream, tAseTkMessage_fields, p_message);

    uint32 message_length = stream.bytes_written;

    /* Then just check for any errors.. */
    if (!status)
    {
        PROTOCOL_BUFFER_ERROR(("Protocol_buffer: Encoding failed: %s\n", PB_GET_ERROR(&stream)));
        return 0;
    }
    PROTOCOL_BUFFER_DEBUG(("Protocol_buffer: Message encode to data: "));
    tool_print_buff(buff,message_length);
    PROTOCOL_BUFFER_DEBUG(("\r\n"));
    return message_length;
}


