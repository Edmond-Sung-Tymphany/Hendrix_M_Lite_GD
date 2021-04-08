/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and is a simple entry point into the SW
*  @author    Christopher Alexander
*  @date      11-Nov-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "controller.h"
#include "bsp.h"
#include "stm32f0xx.h"
#include "piu_common.h"

/*..........................................................................*/
int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    stackMagicNumberFill();

    // remapping vector table
    remappingVectorTable(ADDR_APP_MEMORY);

    /* initialize the Board Support Package */
    BSP_init();

    /* let 'er rip */
    return Controller_Ctor(NORMAL_MODE);
}
