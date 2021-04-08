/**
* @file deviceTypes.c
* @brief The device manager is a set of library functions used to look up the attached devices
* @author Christopher Alexander
* @date 28-Nov-2013
* @copyright Tymphany Ltd.
*/
#include "attachedDevices.h"

void HWManager_Ctor()
{
    for (uint16 i=0;i < NUM_OF_ATTACHED_DEVICES; i++)
    {
        Driver_Ctor(devices[i]);
    }
}

/**
* Returns an instance of the driver as specified by the type parameter.
*
*
*/
cDriver * HWManager_getDriver()
{

}

/**
* Releases the driver as specified by the type parameter.
*
*
*/
void HWManager_releaseDriver(cDriver * driver)
{

}

void HWManager_Xtor()
{

}


char* getVersionString()
{
    return SOFTWARE_VERSION_STRING;
}
