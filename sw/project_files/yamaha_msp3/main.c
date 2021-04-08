/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "controller.h"
#include "bsp.h"
/*..........................................................................*/
int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;
    
    /* initialize the Board Support Package */
    BSP_init();
    
    /* let 'er rip */
    return Controller_Ctor(NORMAL_MODE);
}
