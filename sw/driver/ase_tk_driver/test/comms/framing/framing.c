/**
*  @file      framing.c
*  @brief     framing source file to use the framing
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include "string.h"
#include "tym_type.h"
#include "yahdlc.h"
#include "framing.h"
#include "uart_driver.h"
#include "tool.h"


/* Debug switch */
#define FRAMING_DEBUG_ENABLE
#ifdef FRAMING_DEBUG_ENABLE
#define FRAMING_DEBUG DEBUG_LOG
#else
#define FRAMING_DEBUG(x)
#endif

#define FRAMING_ERROR_ENABLE
#ifdef FRAMING_ERROR_ENABLE
#define FRAMING_ERROR DEBUG_LOG
#else
#define FRAMING_ERROR(x)
#endif

#define BUFF_SIZE   (100)

static int uart_fd;
static framing_func pReceive;

static int yahdlc_seq_advance(int seq)
{
    const int yahdlc_seq_max = 7;   // yahdlc_control_t.seq_no with length of 3 bits

    if (yahdlc_seq_max >= seq)
    {
        seq = 0;
    }
    else
    {
        ++seq;
    }

    return seq;
}

int framing_init()
{
    uart_fd=uart_driver_init();
}

void framing_deinit()
{
    uart_driver_deinit(uart_fd);
}

void framing_register_receive_cb(framing_func func)
{
    pReceive = func;
}


int framing_scan()
{
    if(uart_driver_scan(uart_fd)>0)
    {
        if(uart_driver_ready_to_read(uart_fd))
        {
            static uint32 w_index, valid_index;
            static char buff[BUFF_SIZE];
            static char valid_buff[BUFF_SIZE];
            int size = uart_driver_read(uart_fd, &buff[w_index], 20);
            yahdlc_control_t control;

            w_index += size;
            if(w_index >= BUFF_SIZE)
            {
                FRAMING_ERROR("Framing: Exceed buffer size");
                return;
            }
#ifdef FRAMING_DEBUG_ENABLE
            FRAMING_DEBUG("Framing: Get message from MCU, raw data [%d]: ", w_index);
            tool_print_buff(buff, w_index);
#endif
            // ToDo: check for the correct HDLC start/end flag sequence @ buff
            int ret = yahdlc_get_data(&control, buff, w_index, valid_buff, &valid_index);
            if(ret >= 0)
            {
                /* need to re-write the buffer handle here*/
                w_index = 0;
                memset(buff,0,BUFF_SIZE);
#ifdef FRAMING_DEBUG_ENABLE
                FRAMING_DEBUG("Framing: Analyze success with [%d], => Protocol Buffer: ", ret);
                tool_print_buff(valid_buff, valid_index);
#endif
                if(pReceive!= NULL)
                {
                    pReceive(valid_buff, valid_index);
                    valid_index = 0;
                    memset(valid_buff,0,BUFF_SIZE);
                }
            }
            else
            {
                FRAMING_DEBUG("Framing: Fail to analyze with Framing, error is %d", ret);
            }

        }
    }    
}


int framing_write(char* buff, uint32 size)
{
    static int i = 0;
    static int seq = 0;
    char encode_buff[100];
    uint32 encode_size;
    yahdlc_control_t control;

    control.frame = YAHDLC_FRAME_DATA;
    control.seq_no = seq;
    seq = yahdlc_seq_advance(seq);
    yahdlc_frame_data(&control, buff, size, encode_buff, &encode_size);
    
#ifdef FRAMING_DEBUG_ENABLE
    FRAMING_DEBUG("Framing: Frame the data [%d], => Uart driver\r\n: ", ++i);
    tool_print_buff(encode_buff,encode_size);
#endif
    if(uart_driver_ready_to_read(uart_fd))
    {
        uart_driver_write(uart_fd, encode_buff, encode_size);
    }
}

