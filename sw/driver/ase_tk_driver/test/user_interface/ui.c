/**
*  @file      ui.c
*  @brief    handle all the user interface (command input)
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "tym_type.h"
#include "ui.h"
#include "terminal.h"
#include "batch.h"


/* Debug switch */
#define UI_DEBUG_ENABLE
#ifdef UI_DEBUG_ENABLE
#define UI_DEBUG(x) {printf x;}
#else
#define UI_DEBUG(x)
#endif

#define UI_ERROR_ENABLE
#ifdef UI_ERROR_ENABLE
#define UI_ERROR(x) {printf x;}
#else
#define UI_ERROR(x)
#endif

static pthread_t handler;

#define INPUT_BUFFER_SIZE (50)

static sem_t cmd_sem;
static char command_buf[INPUT_BUFFER_SIZE];

static void* ui_routine(void *arg);


void ui_init()
{
    memset(command_buf,0,INPUT_BUFFER_SIZE);
    terminal_init();
}

void ui_deinit()
{
    sem_destroy(&cmd_sem);
    terminal_deinit();
}

void ui_run()
{
    int argc, thread_ret;
    int ret= sem_init(&cmd_sem,0,0);
    if(ret !=0)
    {
        UI_ERROR(("UI: create sem fail\r\n"));
        return;
    }
    ret = pthread_create(&handler, NULL, ui_routine, &argc);
    if(ret!=0)
    {
        UI_ERROR(("UI: create thread fail\r\n"));
        return;
    }

    pthread_detach(handler);
}

void ui_get_input(char* input)
{
    sem_wait(&cmd_sem);
    memcpy(input, command_buf, strlen(command_buf)+1);
}

static void* ui_routine(void *arg)
{
    UI_DEBUG(("ui_routine start\r\n"));
    while(1)
    {
        int32 nbyte = terminal_get_input(command_buf,INPUT_BUFFER_SIZE);
        if(strncmp(command_buf, "batch", strlen("batch")) == 0)
        {
            UI_DEBUG(("UI: get the batch command\r\n"));
            batch_excute(&command_buf[strlen("batch")], strlen(command_buf) - strlen("batch"));
        }
        else
        {
            sem_post(&cmd_sem);
        }
    }
    UI_DEBUG(("ui_routine stop\r\n"));
}


