#ifndef _TERMINAL_H_
#define _TERMINAL_H_


void terminal_init();

void terminal_deinit();

int32 terminal_get_input(char* buff, uint8 size);

#endif  //_TERMINAL_H_