#ifndef __FRAMING_H
#define __FRAMING_H

#include "tym_type.h"

typedef void (*framing_func)(char* buff, uint32 size);


/**
* @brief           init the framing layer
* @param[in]    void
* @param[out]  void
*/
int framing_init();

/**
* @brief           de-init the framing layer
* @param[in]    void
* @param[out]  void
*/
void framing_deinit();


/**
* @brief           register the receive call back function
* @param[in]    framing_func     call back function pointer, which takes the buffer data as parameter
* @param[out]  void
*/
void framing_register_receive_cb(framing_func func);


/**
* @brief           scan to check whether there's data from MCU.
* @param[in]    void
* @param[out]  void
*/
int framing_scan();

/**
* @brief           frame and write data to lower layer to send to MCU.
* @param[in]    char*   the data buffer to send
* @param[in]    uint32   the size of data to send
* @param[out]  void
*/
int framing_write(char* buff, uint32 size);

#endif  //__FRAMING_H