/**
*  @file      terminal.c
*  @brief    handle all the user interface (command input)
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include <string.h>
#include <semaphore.h>
#include "tym_type.h"
#include "terminal.h"


/* Debug switch */
#define TERMINAL_DEBUG_ENABLE
#ifdef TERMINAL_DEBUG_ENABLE
#define TERMINAL_DEBUG(x) {printf x;}
#else
#define TERMINAL_DEBUG(x)
#endif

#define TERMINAL_ERROR_ENABLE
#ifdef TERMINAL_ERROR_ENABLE
#define TERMINAL_ERROR(x) {printf x;}
#else
#define TERMINAL_ERROR(x)
#endif

void terminal_init()
{

}

void terminal_deinit()
{

}

int32 terminal_get_input(char* buff, uint8 size)
{
    fgets(buff, size, stdin);
    return (strlen(buff)+1);
}


