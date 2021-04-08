#ifndef __UART_DRIVER_H
#define __UART_DRIVER_H

#include "tym_type.h"


/**
* @brief           init the uart driver
* @param[in]    void
* @param[out]  int   return the file handle for uart
*/
int uart_driver_init();

/**
* @brief           de-init the uart driver
* @param[in]    int     the uart handler
* @param[out]  void
*/
void uart_driver_deinit(int fd);

/**
* @brief           check whether there's data received in Uart
* @param[in]    int     the uart handler
* @param[out]  void
*/
int uart_driver_scan(int fd);


/**
* @brief           check whether Uart is ready to read (has data)
* @param[in]    int     the uart handler
* @param[out]  int     0 means it's not ready to write
                               not 0 means it's ready to write

*/
int uart_driver_ready_to_read(int fd);


/**
* @brief           check whether Uart is ready to write (has data)
* @param[in]    int     the uart handler
* @param[out]  int     0 means it's not ready to write
                               not 0 means it's ready to write
*/
int uart_driver_ready_to_write(int fd);


/**
* @brief           read data from uart
* @param[in]    int       the uart handler
* @param[in]    char*   the data buffer pointer that store the data read from uart
* @param[in]    int       the size of data that want to read from uart
* @param[out]  int       return the exact data number that read from uart
*/
int uart_driver_read(int fd, char* buff, int size);


/**
* @brief           write data to uart (send out)
* @param[in]    int       the uart handler
* @param[in]    char*   the data buffer pointer that writes to uart
* @param[in]    int       the size of data that writes to uart
* @param[out]  void
*/
void uart_driver_write(int fd, char* buff, uint32 size);


#endif
