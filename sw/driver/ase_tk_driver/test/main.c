#include <stdio.h>
#include "ase_tk_ctrl.h"
#include "ui.h"


static void create_user_interface_thread();
static void create_ase_tk_ctrl_thread();

static const char* WELCOM_MESSAGE =
{
    "/*******************************************/\r\n\
          ASE_TK Module Simulator \r\n\
/*******************************************/ \r\n"
};


int main()
{
    char buff[50];
    printf("%s",WELCOM_MESSAGE);
    create_user_interface_thread();
    create_ase_tk_ctrl_thread();
    while(1)
    {
        ui_get_input(buff);        
        ase_tk_ctrl_execute_cmd(buff);
    }
    ase_tk_ctrl_deinit();
    return 0;
}

static void create_user_interface_thread()
{
    ui_init();
    ui_run();
}

static void create_ase_tk_ctrl_thread()
{
    ase_tk_ctrl_init();
    ase_tk_ctrl_run();
}

