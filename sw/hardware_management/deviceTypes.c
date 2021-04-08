/**
* @file deviceTypes.c
* @brief The device manager is a set of library functions used to look up the attached devices
* @author Christopher Alexander	
* @date 28-Nov-2013
* @copyright Tymphany Ltd.
*/
#include "attachedDevices.h"
#include "trace.h"


/* Note if many device the same deviceID, this fucntion return the first one
 * The better way is call getDevicebyIdAndType() instead of getDevicebyId()
 */
const tDevice* getDevicebyId(eDeviceID deviceID, uint16 *index)
{
    uint16 i = 0;
    
    if (index)
    {
        i = *index;
    }
    for (;i < NUM_OF_ATTACHED_DEVICES; i++)
    {
        if (deviceID == devices[i]->deviceID)
        {
            if (index)
            {
                *index = i;
            }
            return devices[i];
        }
    }
    return NULL;
}


const tDevice* getDevicebyIdAndType(eDeviceID deviceID, eDeviceType deviceType, uint16 *index)
{
    uint16 i = 0;
    
    if (index)
    {
        i = *index;
    }
    for (;i < NUM_OF_ATTACHED_DEVICES; i++)
    {
        if (deviceID == devices[i]->deviceID && deviceType == devices[i]->deviceType)
        {
            if (index)
            {
                *index = i;
            }
            return devices[i];
        }
    }
    return NULL;
}



char* getVersionString()
{
    return SOFTWARE_VERSION_STRING;
}
