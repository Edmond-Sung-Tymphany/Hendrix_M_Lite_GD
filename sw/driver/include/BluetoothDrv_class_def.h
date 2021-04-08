/**
 * @file        BluetoothDrv_class_def.h
 * @brief       BT Driver class definition file
 * @author      Edmond Sung
 * @date        2014-02-10
 * @copyright   Tymphany Ltd.

 
 */

#ifndef BLUETOOTH_DRIVER_CLASS_DEF_H
#define BLUETOOTH_DRIVER_CLASS_DEF_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "commonTypes.h"

typedef struct{
    /* private data */
    uint8 step;
    eBtCmd cmd;
}cBluetoothDrv;


#endif /* BLUETOOTH_DRIVER_CLASS_DEF_H */

