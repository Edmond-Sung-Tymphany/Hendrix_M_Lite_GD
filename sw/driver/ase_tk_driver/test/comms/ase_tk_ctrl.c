/**
*  @file      ase_tk_ctrl.c
*  @brief    ase_tk controller, simulate the ase_tk module, put all the stuff related with ase_tk module here
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include <pthread.h>
#include "tym_type.h"
#include "ase_tk_ctrl.h"
#include "protocol_buffer.h"

/* Debug switch */
#define ASE_TK_CTRL_DEBUG_ENABLE
#ifdef ASE_TK_CTRL_DEBUG_ENABLE
#define ASE_TK_CTRL_DEBUG(x) {printf x;}
#else
#define ASE_TK_CTRL_DEBUG(x)
#endif

#define ASE_TK_CTRL_ERROR_ENABLE
#ifdef ASE_TK_CTRL_ERROR_ENABLE
#define ASE_TK_CTRL_ERROR(x) {printf x;}
#else
#define ASE_TK_CTRL_ERROR(x)
#endif

typedef void (*Func)();
/* the functions to deal with the help command */
static void action_for_help();


/* define the command here for the project */
typedef enum
{
    ASE_TK_VOL_UP_CMD,
    ASE_TK_VOL_DOWN_CMD,
    ASE_TK_PLAY_PAUSE_CMD,
    ASE_TK_DSP_SET_CMD,
    ASE_TK_ACK_CMD,
    ASE_TK_IGNORE_CMD,
    ASE_TK_MAX_CMD,
} eAseTkCmd;


static const char* command[ASE_TK_MAX_CMD]=
{
    "VOL_UP",
    "VOL_DOWN",
    "PLAY_PAUSE",
    "DSP_SET",
    "ACK",
    "IGNORE"
};

static const char* ASE_SUPPORTED_COMMAND[] =
{
    "VOL_UP",
    "VOL_DOWN",
    "PLAY_PAUSE",
    "DSP_SET"
};

static const char* USER_CTRL_COMMAND[] =
{
    "HELP"
};

static Func ACTION_FUNC[] =
{
    action_for_help
};


static pthread_t handler;
static tAseTkMessage w_message;


/* receive data from MCU */
static void* ase_tk_ctrl_scan(void* arg);
static void on_receive(tAseTkMessage* p_message);
static void read_message(tAseTkMessage* p_message);

/* execute command to MCU */
static void analyze_input_cmd(char* buff);
static void write_message(uint32 message_id, eType type, eAseTkCmd cmd, uint32 data);


void ase_tk_ctrl_init()
{
    protocol_buffer_init();
    protocol_buffer_register_receive_cb(on_receive);
}

void ase_tk_ctrl_deinit()
{
    protocol_buffer_deinit();
}

void ase_tk_ctrl_run()
{
    int argc, thread_ret;
    int ret = pthread_create(&handler, NULL, ase_tk_ctrl_scan, &argc);
    if(ret!=0)
    {
        ASE_TK_CTRL_ERROR(("Ase_tk_ctrl: create thread fail\r\n"));
        return;
    }
    pthread_detach(handler);
}

/* execute user input command from terminal */
void ase_tk_ctrl_execute_cmd(char* cmd)
{
    analyze_input_cmd(cmd);
}

/* analyze the user input command from terminal/patch file */
static void analyze_input_cmd(char* buff)
{
    int i;
    buff[strlen(buff)-1] = 0;
    int length;
    for(i=0; i<ARRAY_LENGTH(USER_CTRL_COMMAND); i++)
    {

        length = max(strlen(buff),strlen(USER_CTRL_COMMAND[i]));
        if(strncasecmp(buff, USER_CTRL_COMMAND[i], length)==0)
        {
            ACTION_FUNC[i]();
            return;
        }
    }
    for(i=0; i<ARRAY_LENGTH(ASE_SUPPORTED_COMMAND); i++)
    {
        length = max(strlen(buff),strlen(ASE_SUPPORTED_COMMAND[i]));
        if(strncasecmp(buff, ASE_SUPPORTED_COMMAND[i], length)==0)
        {
            ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: got command %s\r\n", ASE_SUPPORTED_COMMAND[i]));
            write_message(error_handle_lib_get_message_id(),eType_CONFIRMATION,(eAseTkCmd)i,0);
            break;
        }
    }
    if(i==ARRAY_LENGTH(ASE_SUPPORTED_COMMAND))
    {
        ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: command %s not supported\r\n", buff));
    }
}


/* check whether there's any data from MCU */
static void* ase_tk_ctrl_scan(void* arg)
{
    ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: Waiting for MCU message\n"));
    while(1)
    {
        protocol_buffer_scan();
    }
    ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: scanning stop\n"));
}

/* Call back frunction from lower layer (protocol buffer) */
static void on_receive(tAseTkMessage* p_message)
{
    ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: Receive the message\n"));
    read_message(p_message);
}

/* analyze the message from lower layer (protocol buffer) */
static void read_message(tAseTkMessage* p_message)
{
    if(p_message->payload.command <ASE_TK_MAX_CMD)
    {
        ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: The Header are: version: %d, type: %d, id: %d\n",
                           p_message->header.version, p_message->header.type, p_message->header.message_id));
        switch(p_message->header.type)
        {
            case eType_CONFIRMATION:
                ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: CONFIMATION package, command %s, data %d\n\n",
                                   command[p_message->payload.command], p_message->payload.data));
                write_message(p_message->header.message_id, eType_ACK, ASE_TK_ACK_CMD, 0);
                break;
            case eType_NON_CONFIMATION:
                ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: NON_CONFIMATION package, command %s, data %d\n\n",
                                   command[p_message->payload.command], p_message->payload.data));
                break;
            case eType_ACK:
                ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: Get the ACK Package\n\n"));
                break;
            case eType_RESET:
                ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: Get the Reset Package\n\n"));
                break;
        }
    }
    else
    {
        ASE_TK_CTRL_ERROR(("Ase_tk_ctrl: The command is not support \n"));
    }
}

/* write the message to lower layer (protocol buffer) */
static void write_message(uint32 message_id, eType type, eAseTkCmd cmd, uint32 data)
{
    w_message.header.message_id = message_id;
    w_message.header.type = type;
    w_message.payload.command = cmd;
    w_message.payload.data = data;
    ASE_TK_CTRL_DEBUG(("Ase_tk_ctrl: Send Command To MCU: command %s, data field %d\n",
                       command[w_message.payload.command], w_message.payload.data));
    protocol_buffer_write(&w_message);

}

/* user input ctrl command analyze functions */
static void action_for_help()
{
    printf("It supports command list:\r\n");
    uint8 i;
    for(i=0; i<ARRAY_LENGTH(ASE_SUPPORTED_COMMAND); i++)
    {
        printf("  %s\r\n",ASE_SUPPORTED_COMMAND[i]);
    }
    printf("Please type the command that you want to send out(upper/lower letter is acceptable):\r\n");
}



