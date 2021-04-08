/*****************************************************************************
*  @file      Driver.h
*  @brief     Base class for drivers in Tymphany Platform. Allows for polymorphic creation
*  @author    Christopher Alexander
*  @date      14-Jan-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef DRIVER_H
#define	DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"

CLASS(cDriver)
METHODS
    /* public functions */
    /** I need a way to make this interface work for all drivers.
    Questions:
        How do I make it work for multi-instance drivers?
        Do I treat high level drivers the same as low level IO drivers?
        What about drivers that may be used by multiple clients (i2c for example)? Perhaps one large i2c driver is created and as new ones are "added" it just adds references to the channel?
        Drivers that owb drivers that own drivers!!??!?
    */
    Driver_Ctor(tDevice * device);
END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* DRIVER_H */

