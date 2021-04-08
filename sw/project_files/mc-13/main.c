/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "stm32f0xx.h"
#include "controller.h"

/*..........................................................................*/

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;
    /* let 'er rip */
    BSP_init();
    return Controller_Ctor(NORMAL_MODE);
}
