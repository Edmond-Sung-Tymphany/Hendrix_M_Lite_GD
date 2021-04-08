/**
 *  @file       HWManager.h
 *  @brief      This file presents the interface to Hardware manager
 *  @author     Chris Alexander
 *  @date       20-May-2014
 *  @copyright  Tymphany Ltd.
 */

#ifndef HWMANAGER_H
#define HWMANAGER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "attachedDevices.h"


CLASS(cHWManager)
METHODS
/* PUBLIC FUNCTION PROTOTYPES */

/**
* HW Manager object constructor
* This function is called on start up and does any registration of resources and pre initialisation of HW drivers
* @param[in]    pConfig        Config for HW manager start up
*/
void HWManager_Ctor(const void * pConfig);

/**
* HW Manager object destructor
*/
void HWManager_Xtor();

/**
* HW Manager get driver by device
*
*/
cDriver HWManger_GetDriverByDevice(cDriver * driver, tDevice * device);

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif /* HWMANAGER_H */