/**
 * @file      errorHandleLib.h
 * @brief     Tymphany Error Handler, handle the re-send timeout cases
 * @author    Johnny Fan
 * @date      8-2015
 * @copyright Tymphany Ltd.
 */

#ifndef ERROR_HANDLE_H
#define ERROR_HANDLE_H

#include "ase_tk.pb.h"

typedef void (*Func)(uint8* data, uint8 size);

/**
* @brief           Init the library
* @param[in]    none
*/
void ErrorHandle_Init();


/**
* @brief           Feed the library timer, should be called every x ms
* @param[in]    uint16    timePassMs: the time that has passed since this function is called
*/
void ErrorHandle_TimerFeed(uint16 timePassMs);

/**
* @brief           push a package into the library, then library will handle the re-send work
* @param[in]    eType   the package type, include confirmation and non-confirmation
* @param[in]    uint8*  the package data
* @param[in]    uint8   data size
*/
void ErrorHandle_Push(eType type, uint8* pData, uint8 size);

/**
* @brief           pop out a package from library, then library won't handle the re-send work
* @param[in]    uint8*  the package data
* @param[in]    uint8   data size
* @param[out]  eType  return the data type
*/
eType ErrorHandle_Pop(uint8* pData, uint8 size);


/**
* @brief           response ACK or RESET to ase tk module
* @param[in]    eType  response ACK or RESET type
* @param[in]    uint32   messageId, the id that it response to
* @param[in]    uint8*  the package data
* @param[in]    uint8   data size
*/
void ErrorHandle_PackageResp(eType type, uint32 messageId, uint8* pData, uint8 size);


/**
* @brief           register the timeout handler function when re-send timeout happens
* @param[in]    Func    the timeout functions
*/

void ErrorHandle_RegisterTimeOutFunc(Func func);


#endif /* #ifndef ERROR_HANDLE_H */
