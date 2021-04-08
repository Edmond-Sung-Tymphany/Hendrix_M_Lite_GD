#ifndef __PROTOCOL_BUFFER_H
#define __PROTOCOL_BUFFER_H

#include "ase_tk.pb.h"

typedef void (*protocol_buffer_func)(tAseTkMessage* p_message);

/**
* @brief           init the protocol buffer
* @param[in]    void
* @param[out]  void
*/
int protocol_buffer_init();

/**
* @brief           de-init the ase_tk controller
* @param[in]    void
* @param[out]  void
*/
void protocol_buffer_deinit();

/**
* @brief           register the receive call back function
* @param[in]    protocol_buffer_func     call back function pointer, which takes a message as parameter
* @param[out]  void
*/
void protocol_buffer_register_receive_cb(protocol_buffer_func func);


/**
* @brief           scan to check whether there's message from MCU.
* @param[in]    void
* @param[out]  void
*/
int protocol_buffer_scan();


/**
* @brief           decode message and write to lower layer, sending to MCU.
* @param[in]    tAseTkMessage*     the message pointer that going to send
* @param[out]  void
*/
int protocol_buffer_write(tAseTkMessage* p_message);

#endif  //__PROTOCOL_BUFFER_H