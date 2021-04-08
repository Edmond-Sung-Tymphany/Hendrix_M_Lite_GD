/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

#ifndef OBJECT_IDS_H
#define OBJECT_IDS_H

/** \brief Persistent ID's represent a handle to each unique objects can be created.
* ID's must be contiguous. That way we can index search using O(1) rather than O(n)
* Note: Could be auto-generated based on the product being built and should probably be defined elsewhere
*/

typedef enum
{
    FIRST_SRV_ID = 1,   // The ID is also used prio    

	/* AllplaySrv usually block 10ms~110ms when process signals, 
	 * and may cause the lower priority active-object can not run.
	 * Thus we assign the lowest priority to it, then it will not impact any other servers.
	 */ 
    ALLPLAY_SRV_ID = FIRST_SRV_ID,  //the lowest priority server

	/* PowerSrv kick watch-dog periodically, or watch-dog timeout cause hardware reset.
	 * The watch-dog mechanism protects two scenarios with risk:
	 *  (1) An active-object execute a long time blocking call, any other active-object can not execute
	 *  (2) An active-object handles too many signals, the lower priority active-object can not execute
	 *
     * To protect (1), 
	 *   we should enable hardware watch dog, and let PowerSrv feet dog periodically
	 * 
	 * To protect (2), 
	 *   we should assign the lowest priority to PowerSrv. 
     * But AllplaySrv is a exception. It need the lowest priority because it cause 
	 * any lower priority active-object can not run sometimes.
	 */
    POWER_SRV_ID,

    KEY_SRV_ID,
    DEBUG_SRV_ID,
    BLE_CTRL_SRV_ID,
    DSS_SRV_ID,
    AUDIO_SRV_ID,
    LED_SRV_ID,
    HDMI_SRV_ID,
    DISPLAY_SRV_ID,
    BT_SRV_ID,
    SETTING_SRV_ID,
    ASETK_SRV_ID, //Common id for ASE series, include ASE-TK, ASE-NG, ...
    ASENG_SRV_ID= ASETK_SRV_ID,
    COMM_SRV_ID,
    PERIODIC_SRV_ID,
    TIMER_SRV_ID,
    I2C_SRV_ID,
    USB_SRV_ID,
    A2B_SRV_ID,
    LAST_SRV_ID = A2B_SRV_ID,      //the highest priority server
    MAIN_APP_ID = QF_MAX_ACTIVE - 1, //QP supports maximum number of 32 Activate object and CONTROLLER_ID takes the last one
    LAST_APP_ID = MAIN_APP_ID,       //the highest priority apps
}ePersistantObjID;



#endif /* OBJECT_IDS_H */
