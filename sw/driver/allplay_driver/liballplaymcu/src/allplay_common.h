/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef ALLPLAY_COMMON_H_
#define ALLPLAY_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif


#include <alljoyn.h>
#include "allplaymcu.h"

#ifndef NDEBUG
#define AJ_STATUS_DEBUG(x) "%d - %s\n", x, AJ_StatusText(x)
#define AJ_MODULE ALLPLAY_LIB
extern uint8_t dbgALLPLAY_LIB;
#else // !NDEBUG
#define AJ_STATUS_DEBUG(x) "%d\n", x
#endif // NDEBUG

/* Tymphany modified.*/
#define CONNECT_TIMEOUT (100) // 100ms

/* Please see NOTE1 of aj_config.h, BUS_TIMEOUT should be small enough to let MCU knwo SAM is disconnected
 */
#define BUS_TIMEOUT (5) // 5sec (Note AllJoyn minimum is AJ_MIN_BUS_LINK_TIMEOUT)
    
#define METHOD_TIMEOUT (10 * 1000) // 10s

/*
 * bit 0-7: state
 * bit 8-28: reserved
 * bit 29: need to notify all of connection status change
 * bit 30: player name found
 * bit 31: system name found
 */
enum Allplay_State {
	ALLPLAY_STATE_MASK = 0x00ff,

	// Bus not connected
	ALLPLAY_STATE_STOPPED = 0x0000,
	// Bus is connected to the daemon
	ALLPLAY_STATE_CONNECTED = 0x0001,
	// Finding player (AJ_BusFindAdvertisedName pending)
	ALLPLAY_STATE_FIND_PLAYER_PENDING = 0x0002,
	// Finding player (AJ_BusFindAdvertisedName done)
	ALLPLAY_STATE_FIND_PLAYER_DONE = 0x0003,
	// Finding system (AJ_BusFindAdvertisedName pending)
	ALLPLAY_STATE_FIND_SYSTEM_PENDING = 0x0004,
	// Waiting to discover advertised names
	ALLPLAY_STATE_WAITING_NAME = 0x0005,
	// Joining player
	ALLPLAY_STATE_JOINING_PLAYER = 0x0006,
	// Joining system
	ALLPLAY_STATE_JOINING_SYSTEM = 0x0007,
	// SAM fully joined
	ALLPLAY_STATE_JOINED = 0x0008,
	// Notifying the SAM of the MCU version
	ALLPLAY_STATE_SETTING_MCU_VERSION = 0x0009,
	// Allow OEM to setup custom service
	ALLPLAY_STATE_SETUP_CUSTOM_SERVICE = 0x0010,
	// Connection fully setup
	ALLPLAY_STATE_IDLE = 0x0011,

	ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS = 0x2000,
	ALLPLAY_STATE_PLAYER_FOUND = 0x4000,
	ALLPLAY_STATE_SYSTEM_FOUND = 0x8000
};

struct allplay_ctx {
	enum Allplay_State state;

	const char* mcuFirmwareVersion;
	allplay_reboot_method rebootMethod;

	AJ_BusAttachment bus;
	char *playerId;
	char *systemId;
	uint32_t systemSessionId;
	uint32_t playerSessionId;

	// We need to store all this info because we only get partial data from
	// the player events.
	// Volume info are stored as returned by the AllPlay interfaces, not as
	// used by this API, i.e. [min-max] and not [0-(max-min)].
	bool_t mute;
	int16_t volume;
	int16_t volumeMin;
	int16_t volumeMax;
	int16_t volumeStep;

	// We need to store all this info because we only get partial data from
	// the Bluetooth events.
	struct {
		bool_t initialized;
		bool_t enabled;
		bool_t pairable;
		uint32_t btCount;
		uint32_t a2dpCount;
	} bluetooth;

	struct {
		HandleMsgFunc msgFunc;
		SetupService setupFunc;
	} customService;
};


#ifdef __cplusplus
}
#endif

#endif /* ALLPLAY_COMMON_H_ */
