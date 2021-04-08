/**
*  @file      batch.c
*  @brief    handle all the user interface (command input)
*  @author    Johnny Fan
*  @date      8 -2015
*  @copyright Tymphany Ltd.
*/
#include <stdio.h>      /*standard input and output*/
#include <stdlib.h>     /*standard library*/
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include "tym_type.h"
#include "batch.h"


/* Debug switch */
#define BATCH_DEBUG_ENABLE
#ifdef BATCH_DEBUG_ENABLE
#define BATCH_DEBUG(x) {printf x;}
#else
#define BATCH_DEBUG(x)
#endif

#define BATCH_ERROR_ENABLE
#ifdef BATCH_ERROR_ENABLE
#define BATCH_ERROR(x) {printf x;}
#else
#define BATCH_ERROR(x)
#endif

#define COMMENT_HEAD   '/'
#define COMMAND_START  '#'

typedef enum
{
    COMMAND_TYPE = 0,
    COMMENT_TYPE,
    MAX_HEADER_TYPE
} e_header_type;


static int fd;
static int batch_open_file(char* file);
static void batch_analyze_file_loop(int fd);
static e_header_type search_header(int fd);
static void excute_cmd(int fd, e_header_type header);


void batch_init()
{

}

void batch_deinit()
{

}


void batch_excute(char* file, uint8 size)
{
    char* file_p = file;
    /* remove the blank before the file name*/
    while(file_p!=NULL)
    {
        if((*file_p) != ' ')
        {
            break;
        }
        file_p++;
    }

    fd = batch_open_file(file_p);
    if(fd<0)
    {
        BATCH_ERROR(("BATCH ERROR: can not open the batch file %s\r\n", file_p));
        return;
    }
    else
    {
        BATCH_DEBUG(("BATCH: successfully open the batch file %s\r\n", file_p));
    }
    batch_analyze_file_loop(fd);
}

static int batch_open_file(char* file)
{
    file[strlen(file)-1] = 0;
    int fd = open(file, O_RDONLY);
    return fd;
}

static void batch_analyze_file_loop(int fd)
{
    BATCH_DEBUG(("BATCH: start excuting the batch file\r\n"));
    e_header_type header = search_header(fd);
    while(header < MAX_HEADER_TYPE)
    {
        BATCH_DEBUG(("BATCH: the header type is %d\r\n", header));
        excute_cmd(fd, header);
        header = search_header(fd);
    }
    BATCH_DEBUG(("BATCH: finish the batch file\r\n"));
}

static e_header_type search_header(int fd)
{
    e_header_type header = MAX_HEADER_TYPE;
    char buff[1];
    while(header == MAX_HEADER_TYPE)
    {
        if(read(fd, buff, 1) == 1)
        {
            switch(buff[0])
            {
                case COMMENT_HEAD:
                    header = COMMENT_TYPE;
                    break;
                case COMMAND_START:
                    header = COMMAND_TYPE;
                    break;
                default:
                    break;
            }
        }
        else
        {
            // no data left
            break;
        }
    }
    return header;

}

static void excute_cmd(int fd, e_header_type header)
{
    
}


