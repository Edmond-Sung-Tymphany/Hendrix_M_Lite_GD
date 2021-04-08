/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/
/**
 * @file
 * @brief AllPlay Smart Audio Module Control Library for Host MCU header file
 */
#ifndef ALLPLAYMCU_H_
#define ALLPLAYMCU_H_

//! @cond
#include "aj_target.h"
#include "alljoyn.h"
//! @endcond

//! @cond
#if !defined(UNUSED)
#	if defined(__GNUC__)
#		define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#	else
#		define UNUSED(x) UNUSED_ ## x
#	endif
#endif
//! @endcond

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Structures
 */
/**
 * Opaque structure used internally by allplaymcu
 */
typedef struct allplay_ctx allplay_ctx_t;
//! @cond
typedef unsigned char bool_t;
//! @endcond

/*
 * Function pointer definitions for exposing a custom service
 */
/**
 * Callback to process AllJoyn messages.
 *
 * This callback is called when a request is received for one of the MCU
 * service. It should also handle the reply to AJ_METHOD_BIND_SESSION_PORT and
 * call AJ_BusRequestName(), then handle the reply to AJ_METHOD_REQUEST_NAME
 * and call AJ_BusAdvertiseName(), and finally handle the reply to
 * AJ_METHOD_ADVERTISE_NAME and call AJ_BusSetPasswordCallback(). The callback
 * should also handle AJ_METHOD_ACCEPT_SESSION requests.
 *
 * @see myProcessMessage() in the sample code.
 *
 * @param ajMsg the AllJoyn message to handle
 * @return TRUE if the message was processed, FALSE if not
 */
typedef bool_t (*HandleMsgFunc)(AJ_Message *ajMsg);

/**
 * Callback to setup the MCU services.
 *
 * Mostly, this callback should start setting up the session port (AJ_BusBindSessionPort()).
 *
 * @see mySetupService() in the sample code.
 *
 * @param bus The AllJoyn bus created by allplaymcu.
 * @param deviceId The device ID of the SAM. This identifier is computed during
 *        the first boot and will persist until a factory reset.
 */
typedef void (*SetupService)(AJ_BusAttachment *bus, const char *deviceId);

/**
 * Message types.
 * Member of #allplay_message_t returned by #allplay_read_message().
 * In comment is the type of #allplay_message_t extension struct if any
 */
enum allplay_message_type {
	ALLPLAY_ERROR = 0, //!<Error. This #allplay_message_t can be cast to a #allplay_error_t.

	/* Responses */
	ALLPLAY_RESPONSE_VOLUME_INFO = 10, //!<Response to #allplay_get_volume_info(). This #allplay_message_t can be cast to a #allplay_volume_info_t
	ALLPLAY_RESPONSE_VOLUME = 11, //!<Response to #allplay_set_volume(). This response has no data.
	ALLPLAY_RESPONSE_VOLUME_ADJUST = 12, //!<Response to #allplay_volume_adjust(). This response has no data.
	ALLPLAY_RESPONSE_MUTE = 14, //!<Response to #allplay_mute(). This response has no data.
	//
	ALLPLAY_RESPONSE_PLAY = 15, //!<Response to #allplay_play(). This response has no data.
	ALLPLAY_RESPONSE_PAUSE = 16, //!<Response to #allplay_pause(). This response has no data.
	ALLPLAY_RESPONSE_STOP = 17, //!<Response to #allplay_stop(). This response has no data.
	ALLPLAY_RESPONSE_NEXT = 18, //!<Response to #allplay_next(). This response has no data.
	ALLPLAY_RESPONSE_PREVIOUS = 19, //!<Response to #allplay_previous(). This response has no data.
	ALLPLAY_RESPONSE_SET_POSITION = 20, //!<Response to #allplay_set_position(). This response has no data.
	ALLPLAY_RESPONSE_PLAYER_STATE = 21, //!<Response to #allplay_get_player_state(). This #allplay_message_t can be cast to a #allplay_player_state_t.
	//
	ALLPLAY_RESPONSE_GET_FRIENDLY_NAME = 23, //!<Response to #allplay_get_friendly_name(). This #allplay_message_t can be cast to a #allplay_friendly_name_t.
	ALLPLAY_RESPONSE_NETWORK_REQUEST_WPS = 40, //!<Response to #allplay_network_request_wps(). This response has no data.
	ALLPLAY_RESPONSE_NETWORK_INFO = 41, //!<Response to #allplay_network_get_info(). This #allplay_message_t can be cast to a #allplay_network_info_t.
	//
	ALLPLAY_RESPONSE_SYSTEM_MODE = 50, //!<Response to #allplay_get_system_mode(). This #allplay_message_t can be cast to a #allplay_system_mode_t.
	ALLPLAY_RESPONSE_SETUP = 51, //!<Response to #allplay_start_setup().  This response has no data.
	//
	ALLPLAY_RESPONSE_SET_RESAMPLING_MODE = 52, //!<Response to #allplay_set_resampling_mode(). This response has no data.
	ALLPLAY_RESPONSE_RUN_CMD = 53, //!<Response to #allplay_run_cmd(). This #allplay_message_t can be cast to a #allplay_cmd_t.
	ALLPLAY_RESPONSE_FIRMWARE_VERSION = 54, //!<Response to #allplay_get_firmware_version(). This #allplay_message_t can be cast to a #allplay_firmware_version_t.
	ALLPLAY_RESPONSE_WIFIENABLE = 55, //!<Response to #allplay_wifi_enable(). This response has no data.
	ALLPLAY_RESPONSE_DIRECTENABLE = 56, //!<Response to #allplay_directmode_enable(). This response has no data.
	ALLPLAY_RESPONSE_FIRMWARE_CHECK = 57, //!<Response to #allplay_firmware_check(). This #allplay_message_t can be cast to a #allplay_firmware_update_info_t.
	ALLPLAY_RESPONSE_FIRMWARE_UPDATE = 58, //!<Response to #allplay_firmware_update() and #allplay_firmware_update_from_url(). This response has no data.
	ALLPLAY_RESPONSE_RESET_TO_FACTORY = 59, //!<Response to #allplay_reset_to_factory(). This response has no data.
	ALLPLAY_RESPONSE_RSSI = 60, //!<Response to #allplay_get_rssi(). This #allplay_message_t can be cast to an #allplay_rssi_t.
	ALLPLAY_RESPONSE_CONNECT_WIFI = 61, //!<Response to #allplay_connect_wifi(). This response has no data.
	ALLPLAY_RESPONSE_GET_RESAMPLING_MODE = 62, //!<Response to #allplay_get_resampling_mode(). This #allplay_message_t can be cast to an #allplay_resampling_mode_t.
	ALLPLAY_RESPONSE_SET_MCU_IDLE = 63, //!<Response to #allplay_set_mcu_idle(). This response has no data.
	ALLPLAY_RESPONSE_SET_BATTERY_STATE = 64, //!<Response to #allplay_set_battery_state(). This response has no data.
	ALLPLAY_RESPONSE_SET_EXTERNAL_SOURCE = 65, //!<Response to #allplay_set_external_source(). This response has no data.
	ALLPLAY_RESPONSE_BLUETOOTH_ENABLE = 66, //!<Response to #allplay_bluetooth_enable(). This response has no data.
	ALLPLAY_RESPONSE_BLUETOOTH_ENABLE_PAIRING = 67, //!<Response to #allplay_bluetooth_enable_pairing(). This response has no data.
	ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE = 68, //!<Response to #allplay_bluetooth_get_state(). This #allplay_message_t can be cast to an #allplay_bluetooth_state_t.
	ALLPLAY_RESPONSE_PLAYITEM = 69, //!<Response to #allplay_play_item(). This response has no data.
	ALLPLAY_RESPONSE_GET_DEVICE_INFO = 70, //!<Response to #allplay_get_device_info(). This #allplay_message_t can be cast to an #allplay_device_info_t.
	ALLPLAY_RESPONSE_GET_CURRENT_ITEM_URL = 71, //!<Response to #allplay_get_current_item_url(). This #allplay_message_t can be cast to an #allplay_url_t.
	ALLPLAY_RESPONSE_GET_LOOP_MODE = 72, //!<Response to #allplay_get_loop_mode(). This #allplay_message_t can be cast to an #allplay_loop_mode_t.
	ALLPLAY_RESPONSE_ADVANCE_LOOP_MODE = 73, //!<Response to #allplay_advance_loop_mode(). This response has no data.
	ALLPLAY_RESPONSE_GET_SHUFFLE_MODE = 74, //!<Response to #allplay_get_shuffle_mode(). This #allplay_message_t can be cast to an #allplay_shuffle_mode_t.
	ALLPLAY_RESPONSE_TOGGLE_SHUFFLE_MODE = 75, //!<Response to #allplay_toggle_shuffle_mode(). This response has no data.
	ALLPLAY_RESPONSE_SHUTDOWN_SAM = 76, //!<Response to #allplay_shutdown_sam(). This response has no data.

	/* Events */
	ALLPLAY_EVENT_PLAYER_STATE_CHANGED = 100, //!<PlayerStateChanged event. This #allplay_message_t can be cast to a #allplay_player_state_t
	ALLPLAY_EVENT_VOLUME_CHANGED = 101, //!<VolumeChanged event. This #allplay_message_t can be cast to a #allplay_volume_info_t
	ALLPLAY_EVENT_NETWORK_WPS_RESULT = 102, //!<WPS result. This #allplay_message_t can be cast to a #allplay_network_wps_result_t
	ALLPLAY_EVENT_SYSTEM_MODE = 103, //!<SystemMode event. This #allplay_message_t can be cast to a #allplay_system_mode_t
	ALLPLAY_EVENT_CONNECTION_STATUS_CHANGED = 104, //!<Connection status changed. This event has no data. Use #allplay_is_connected() to get status.
	ALLPLAY_EVENT_CMD_RESULT = 105, //!<A command finished running. This #allplay_message_t can be cast to a #allplay_cmd_result_t
	ALLPLAY_EVENT_START_MCU_UPDATE = 106, //!<The MCU must prepare to receive a firmware update. This event has no data.
	ALLPLAY_EVENT_FIRMWARE_UPDATE_AVAILABLE = 107, //!<A firmware update is available. This #allplay_message_t can be cast to a #allplay_firmware_update_info_t.
	ALLPLAY_EVENT_FIRMWARE_UPDATE_STATUS = 108, //!<The firmware update status changed. This #allplay_message_t can be cast to a #allplay_firmware_update_status_t.
	ALLPLAY_EVENT_NETWORK_INFO_CHANGED = 109, //!<The network changed. This #allplay_message_t can be cast to a #allplay_network_info_t.
	ALLPLAY_EVENT_BLUETOOTH_STATE_CHANGED = 110, //!<The Bluetooth state changed. This #allplay_message_t can be cast to a #allplay_bluetooth_state_t. #allplay_bluetooth_get_state() must have been called at least once before this event will be thrown.
	ALLPLAY_EVENT_REBOOT_STARTED = 111, //!<SAM is rebooting. This event has no data. Note that this is a best effort event and it is possible for the SAM to be restarted without the event being received (kernel crash, AllJoyn shutdown before the signal could be sent, ...)
	ALLPLAY_EVENT_LOOP_MODE_CHANGED = 112, //!<Loop mode changed. This #allplay_message_t can be cast to a #allplay_loop_mode_t.
	ALLPLAY_EVENT_SHUFFLE_MODE_CHANGED = 113 //!<Shuffle mode changed. This #allplay_message_t can be cast to a #allplay_shuffle_mode_t.
};

/**
 * AllPlay request status
 */
typedef enum {
	ALLPLAY_ERROR_NONE = 0x0, //!<no error
	ALLPLAY_ERROR_FAILED = 0x1, //!<generic error
	ALLPLAY_NOT_CONNECTED = 0x2 //!<not fully connected to AllJoyn
} allplay_status;

/**
 * Reboot method after MCU firmware update
 */
typedef enum {
	ALLPLAY_REBOOT_SAM = 0, //!<The SAM should reboot when done with the update
	ALLPLAY_REBOOT_MCU = 1, //!<The MCU will trigger a reset of SAM with done with the update
	ALLPLAY_REBOOT_USER = 2 //!<The user will trigger the reboot (reset, power on/off, ...)
} allplay_reboot_method;

/**
 * WPS type
 */
enum allplay_network_wps_type {
	ALLPLAY_NETWORK_WPS_PBC, //!<Configure Wi-Fi using the Push-Button method
	ALLPLAY_NETWORK_WPS_PIN, //!<Configure Wi-Fi using a PIN
	ALLPLAY_NETWORK_WPS_CANCEL //!<Cancel WPS configuration
};

/**
 * Network type
 */
enum allplay_network_type_value {
	ALLPLAY_NETWORK_NONE = 0, //!<Device is not connected
	ALLPLAY_NETWORK_WIFI = 1, //!<Device is connected to Wi-Fi
	ALLPLAY_NETWORK_ETHERNET = 2 //!<Device is connected to Ethernet
};

/**
 * WPS request result
 */
enum allplay_network_wps_result_value {
	ALLPLAY_NETWORK_WPS_SUCCESS, //!<Wi-Fi setup was successful
	ALLPLAY_NETWORK_WPS_FAILURE, //!<Wi-Fi setup failed
	ALLPLAY_NETWORK_WPS_CANCELLED, //!<Wi-Fi setup was canceled
	ALLPLAY_NETWORK_WPS_TIMEOUT, //!<Wi-Fi setup timed out
	ALLPLAY_NETWORK_WPS_OVERLAP //!<WPS Overlap error: occurs when multiple APs were in WPS mode at the same time. Same as failure, but could tell user to try the process again in 2+ minutes.
};

/**
 * SAM System mode
 */
enum allplay_system_mode_value {
	ALLPLAY_SYSTEM_MODE_CONFIGURED, //!<Device is in configured/normal mode
	ALLPLAY_SYSTEM_MODE_CONFIGURING, //!<Device is being configured
	ALLPLAY_SYSTEM_MODE_UNCONFIGURED, //!<Device is unconfigured
	ALLPLAY_SYSTEM_MODE_DIRECT, //!<Device is in direct mode
	ALLPLAY_SYSTEM_MODE_UPDATING //!<Device is being updated
};

/**
 * AllPlay Player state
 */
enum allplay_player_state_value {
	ALLPLAY_PLAYER_STATE_UNKNOWN, //!<Player is in an unknown state
	ALLPLAY_PLAYER_STATE_STOPPED, //!<Player is stopped
	ALLPLAY_PLAYER_STATE_TRANSITIONING, //!<Player is switching track
	ALLPLAY_PLAYER_STATE_BUFFERING, //!<Player is buffering
	ALLPLAY_PLAYER_STATE_PLAYING, //!<Player is playing
	ALLPLAY_PLAYER_STATE_PAUSED //!<Player is paused
};

/**
 * AllPlay Firmware update status
 */
enum allplay_firmware_update_status_value {
	ALLPLAY_FIRMWARE_UPDATE_SUCCESSFUL, //!<The firmware update was successful. If the SAM is rebooted quickly, this event may not have time to be sent/received.
	ALLPLAY_FIRMWARE_UPDATE_NOT_NEEDED, //!<The device is already up-to-date.
	ALLPLAY_FIRMWARE_UPDATE_FAILED //!<The update failed.
};

/**
 * Action to take after factory reset completed
 */
typedef enum {
	ALLPLAY_RESET_ACTION_REBOOT = 0, //!<The SAM should reboot
	ALLPLAY_RESET_ACTION_HALT = 1 //!<The SAM should halt the CPU
} allplay_reset_action_t;

/**
 * AllPlay loop modes
 */
enum allplay_loop_mode_value {
	ALLPLAY_LOOP_NONE, //!<Loop mode is off. The SAM will stop playing at the end of the playlist.
	ALLPLAY_LOOP_ONE, //!<The SAM will keep playing the current track.
	ALLPLAY_LOOP_ALL //!<The SAM will repeat the playlist once it reaches the end.
};

/**
 * AllPlay shuffle modes
 */
enum allplay_shuffle_mode_value {
	ALLPLAY_SHUFFLE_LINEAR, //!<Shuffle is off. The SAM will play the tracks in the playlist order.
	ALLPLAY_SHUFFLE_SHUFFLED //!<Shuffle is on. The SAM will play the tracks in a random order.
};

/**
 * Common fields to all allplay message structures
 */
#define ALLPLAY_MESSAGE_BASE \
	enum allplay_message_type messageType; /*!< @brief Type of message */ \
	void* userData /*! @brief For response messages, value passed in the @a userData parameter of the function, NULL for event messages */

/**
 * Base AllPlay Message structure
 */
typedef struct allplay_message {
	ALLPLAY_MESSAGE_BASE;
} allplay_message_t;

/**
 * AllPlay Error
 */
typedef struct allplay_error {
	ALLPLAY_MESSAGE_BASE;
	allplay_status code; //!<The error code
} allplay_error_t;

/**
 * AllPlay Volume Info
 */
typedef struct allplay_volume_info {
	ALLPLAY_MESSAGE_BASE;
	bool_t mute; //!<TRUE if the volume is muted
	int32_t volume; //!<Current volume
	int32_t max_volume; //!<Maximum volume possible
} allplay_volume_info_t;

/**
 * AllPlay Player State
 */
typedef struct allplay_player_state {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_player_state_value state; //!<Current state of the player
	int32_t duration; //!<Duration of the current audio track, 0 if unknown
	int32_t position; //!<Current position
	uint32_t sampleRate; //!<Sample rate of the audio output
	uint32_t audioChannels; //!<Number of channels in the audio output
	uint32_t bitsPerSample; //!<Number of bits per sample of the audio output
	const char *title; //!<Title
	const char *artist; //!<Artist
	const char *album; //!<Album
	const char *contentSource; //!<Content source
} allplay_player_state_t;

/**
 * AllPlay Network WPS Result
 */
typedef struct allplay_network_wps_result {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_network_wps_result_value result; //!<Result of the WPS request
} allplay_network_wps_result_t;

/**
 * AllPlay Network Info
 */
typedef struct allplay_network_info {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_network_type_value type; //!<Type of the current network connection
	unsigned char wifiIp[4]; //!< wifi IP Address
	unsigned char wifiMac[6]; //!< wifi MAC Address
	unsigned char ethernetIp[4]; //!< ethernet IP Address
	unsigned char ethernetMac[6]; //!< ethernet MAC Address
	char ssid[33]; //!< WiFi SSID (max 32 char + \0), empty string if not available
	char rssi; //!< RSSI, 0 if not available
	int32_t frequency; //!< Wi-Fi center frequency in MHz, 0 if not available
} allplay_network_info_t;

/**
 * AllPlay System Mode
 */
typedef struct allplay_system_mode {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_system_mode_value mode; //!<Current mode of the system
} allplay_system_mode_t;

/**
 * AllPlay Firmware Version
 */
typedef struct allplay_firmware_version {
	ALLPLAY_MESSAGE_BASE;
	char *version; //!<Version of the firmware bundle
} allplay_firmware_version_t;

/**
 * AllPlay command
 */
typedef struct allplay_cmd {
	ALLPLAY_MESSAGE_BASE;
	uint32_t id; //!<ID of the command. This will be used to identify the response in #allplay_cmd_result_t
} allplay_cmd_t;

/**
 * AllPlay command result
 */
typedef struct allplay_cmd_result {
	ALLPLAY_MESSAGE_BASE;
	uint32_t id; //!<ID of the command returned when calling #allplay_run_cmd
	int32_t exitstatus; //!<Exit status of the command extracted from the result of a system() call. This is the value returned by WEXITSTATUS. If the WIFEXITED returned 0, this value will be -1.
	int32_t termsig; //!<Signal that terminated the command extracted from the result of a system() call. This is the value returned by WTERMSIG. If the WIFSIGNALED returned 0, this value will be -1.
	char *stdoutStr; //!<The content of the command's standard output
	char *stderrStr; //!<The content of the command's standard error
} allplay_cmd_result_t;

/**
 * AllPlay firmware update info
 */
typedef struct allplay_firmware_update_info {
	ALLPLAY_MESSAGE_BASE;
	char *version; //!<latest version available
	char *url; //!<URL for this firmware update.
} allplay_firmware_update_info_t;

/**
 * AllPlay firmware update status
 */
typedef struct allplay_firmware_update_status {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_firmware_update_status_value status; //!<status of the firmware update.
} allplay_firmware_update_status_t;

/**
 * RSSI Measurements
 */
typedef struct allplay_rssi_t {
	ALLPLAY_MESSAGE_BASE;
	int32_t chain[2]; //!<chain values
} allplay_rssi_t;

/**
 * Resampling mode.
 */
typedef struct allplay_resampling_mode {
	ALLPLAY_MESSAGE_BASE;
	int32_t resamplingMode; //!<resampling mode
} allplay_resampling_mode_t;

/**
 * AllPlay friendly name
 */
typedef struct allplay_friendly_name {
	ALLPLAY_MESSAGE_BASE;
	char *name; //!<friendly name
} allplay_friendly_name_t;

/**
 * Bluetooth state
 */
typedef struct allplay_bluetooth_state {
	ALLPLAY_MESSAGE_BASE;
	bool_t enabled; //!<bluetooth is enable or not
	bool_t pairable; //!<pairing is enabled or not
	uint32_t btDevicesCount; //!<number of a Bluetooth devices connected to the SAM
	uint32_t a2dpDevicesCount;  //!<number of A2DP-capable devices connected to the SAM
} allplay_bluetooth_state_t;

/**
 * Device info
 */
typedef struct allplay_device_info {
	ALLPLAY_MESSAGE_BASE;
	const char *manufacturer; //!<Device manufacturer
	const char *model; //!<Device model
	const char *deviceId; //!<Unique identifier of this device
	const char *firmwareVersion; //!<Version of the SAM firmware
} allplay_device_info_t;

/**
 * URL
 */
typedef struct allplay_url {
	ALLPLAY_MESSAGE_BASE;
	const char *url; //!<URL
} allplay_url_t;

/**
 * Loop mode
 */
typedef struct allplay_loop_mode {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_loop_mode_value mode; //!<Loop mode
} allplay_loop_mode_t;

/**
 * Shuffle mode
 */
typedef struct allplay_shuffle_mode {
	ALLPLAY_MESSAGE_BASE;
	enum allplay_shuffle_mode_value mode; //!<Shuffle mode
} allplay_shuffle_mode_t;

/**
 * Create a new AllPlay context for use with the other function.
 *
 * @param mcuFirmwareVersion version of the MCU firmware. Used by the SAM to
 *        check if an update is available. The string must be valid for the
 *        lifetime of the AllPlay context, AllPlay will not make a copy.
 * @param rebootMethod Used by the SAM to decide on how to proceed with a
 *        reboot after a successful update.
 *
 * @return the new AllPlay context
 */
allplay_ctx_t* allplay_new(const char* mcuFirmwareVersion, allplay_reboot_method rebootMethod);

/**
 * Create a new AllPlay context for use with the other function.
 *
 * @param mcuFirmwareVersion version of the MCU firmware. Used by the SAM to
 *        check if an update is available. The string must be valid for the
 *        lifetime of the AllPlay context, AllPlay will not make a copy.
 * @param rebootMethod Used by the SAM to decide on how to proceed with a
 *        reboot after a successful update.
 * @param AppObjects    List of AllJoyn Objects to be exposed in the service
 * @param msgFunc   Function pointer that will be used to process the service messages
 * @param setupFunc   Function pointer that will be used to setup the AllJoyn service
 *
 * @return the new AllPlay context
 */
allplay_ctx_t* allplay_new_with_mcu_service(const char* mcuFirmwareVersion, allplay_reboot_method rebootMethod, const AJ_Object* AppObjects, HandleMsgFunc msgFunc, SetupService setupFunc);

/**
 * Free an AllPlay context previously allocated with #allplay_new().
 * The pointer will be set to NULL.
 *
 * @param apctx pointer to the allocated context.
 */
void allplay_free(allplay_ctx_t** apctx);

/**
 * Check if AllPlay is fully connected to the SAM.
 *
 * Once fully connected it is possible to make any AllPlay requests and
 * receive all messages. If not fully connected some requests may return
 * #ALLPLAY_NOT_CONNECTED and some messages may not be received. Which
 * request will fail and which message will not be received are undefined
 * depend on the internal connection state.
 *
 * @param apctx   The allplay context
 * @return non-zero if allplay is ready to hand requests from the app, 0 otherwise
 */
bool_t allplay_is_connected(allplay_ctx_t* apctx);

/*
* Functions to read messages from SAM
*/

/**
 * Reads available AllPlay messages from SAM
 *
 * @param apctx   The allplay context
 * @param timeout How long to wait for a message (miliseconds)
 *
 * @return
 *          - NULL if timeout
 *          - an #allplay_message_t structure that may be cast into other structures depending on #allplay_message_type
 *            The returned structure must be freed with an #allplay_free_message() call when data is no longer used
 */
allplay_message_t* allplay_read_message(allplay_ctx_t* apctx, uint32_t timeout);

/**
 * Frees an allplay message received from #allplay_read_message
 *
 * @param apctx   The allplay context
 * @param papmsg  The structure to free
 */
void allplay_free_message(allplay_ctx_t* apctx, allplay_message_t** papmsg);

/*
* Functions to send commands to SAM
*/
/** player related functions */

/**
 * Request current player volume info.
 *
 * It will trigger a #ALLPLAY_RESPONSE_VOLUME_INFO response message with #allplay_volume_info_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_volume_info(allplay_ctx_t *apctx, void *userData);

/**
 * Set player volume.
 *
 * It will trigger a #ALLPLAY_RESPONSE_VOLUME response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param volume   Volume value
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_volume(allplay_ctx_t *apctx, void *userData, int32_t volume);

/**
 * Increase/Decrease player volume.
 *
 * It will trigger a #ALLPLAY_RESPONSE_VOLUME_ADJUST response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param step     Number of steps to increase/decrease volume
 *
 * @return Return #allplay_status
 */
allplay_status allplay_volume_adjust(allplay_ctx_t *apctx, void *userData, int32_t step);

/**
 * Set SAM mute state.
 *
 * It will trigger a #ALLPLAY_RESPONSE_MUTE response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param mute     Mute state (1 mute, 0 not mute)
 *
 * @return Return #allplay_status
 */
allplay_status allplay_mute(allplay_ctx_t *apctx, void *userData, bool_t mute);

/**
 * Resume playback after paused state.
 *
 * It will trigger a #ALLPLAY_RESPONSE_PLAY response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_play(allplay_ctx_t *apctx, void *userData);

/**
 * Pause playback.
 *
 * It will trigger a #ALLPLAY_RESPONSE_PAUSE response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_pause(allplay_ctx_t *apctx, void *userData);

/**
 * Stop playback.
 *
 * It will trigger a #ALLPLAY_RESPONSE_STOP response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_stop(allplay_ctx_t *apctx, void *userData);

/**
 * Skip to the next playlist item.
 *
 * It will trigger a #ALLPLAY_RESPONSE_NEXT response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_next(allplay_ctx_t *apctx, void *userData);

/**
 * Skip to the previous playlist item.
 *
 * It will trigger a #ALLPLAY_RESPONSE_PREVIOUS response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_previous(allplay_ctx_t *apctx, void *userData);

/**
 * Seek to given position in current stream.
 *
 * It will trigger a #ALLPLAY_RESPONSE_SET_POSITION response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param position Time position in miliseconds
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_position(allplay_ctx_t *apctx, void *userData, signed long long position);

/**
 * Get current player state.
 *
 * It will trigger a #ALLPLAY_RESPONSE_PLAYER_STATE response message with #allplay_player_state_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_player_state(allplay_ctx_t *apctx, void *userData);

/**
 * Request or cancel WPS mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_NETWORK_REQUEST_WPS response message with #allplay_message_t.
 * It will also trigger a ALLPLAY_EVENT_NETWORK_WPS_RESULT when WPS mode is over and include the result in #allplay_network_wps_result_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param type     WPS type
 * @param pin      WPS pin if type is #ALLPLAY_NETWORK_WPS_PIN
 *
 * @return Return #allplay_status
 */
allplay_status allplay_network_request_wps(allplay_ctx_t *apctx, void *userData, enum allplay_network_wps_type type, int32_t pin);

/**
 * Get Network information.
 *
 * It will trigger a #ALLPLAY_RESPONSE_NETWORK_INFO response message with #allplay_network_info_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_network_get_info(allplay_ctx_t *apctx, void *userData);

/**
 * Get RSSI Measurements information.
 *
 * It will trigger a #ALLPLAY_RESPONSE_RSSI response message with #allplay_rssi_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_rssi(allplay_ctx_t *apctx, void *userData);

/**
 * Get system mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_SYSTEM_MODE response message with #allplay_system_mode_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_system_mode(allplay_ctx_t *apctx, void *userData);

/**
 * Get firmware version.
 *
 * It will trigger a #ALLPLAY_RESPONSE_FIRMWARE_VERSION response message with #allplay_firmware_version_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_firmware_version(allplay_ctx_t *apctx, void *userData);

/**
 * Set the resampling mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_SET_RESAMPLING_MODE response message
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param resamplingMode Value set on the switch: index into device tree
 *                 0: sound (default i2s sound setting)
 *                 1: sound1
 *                 etc.
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_resampling_mode(allplay_ctx_t *apctx, void *userData, int32_t resamplingMode);

/**
 * Get the resampling mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_GET_RESAMPLING_MODE response message with an #allplay_resampling_mode_t message
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_resampling_mode(allplay_ctx_t *apctx, void *userData);

/**
 *
 * NOTE: THIS FUNCTION SHOULD NEVER BE USED IN A PRODUCTION DEVICE
 *       THIS METHOD IS STRICTLY FOR FACTORY PROVISIONING
 *       COMMERCIAL CODE PATHS SHOULD USE allplay_reset_to_factory
 *
 * Start network setup mode - will put the SAM in soft AP mode with setup wizard.
 *
 * It will trigger a #ALLPLAY_RESPONSE_SETUP response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_start_setup(allplay_ctx_t *apctx, void *userData);

/**
 * Start factory reset - will reboot sam.
 *
 * It will trigger a #ALLPLAY_RESPONSE_RESET_TO_FACTORY response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param action The action to take after reset completes (reboot or halt)
 *
 * @return Return #allplay_status
 */
allplay_status allplay_reset_to_factory(allplay_ctx_t *apctx, void *userData, allplay_reset_action_t action);

/**
 * Bring Wi-Fi Up or Down.
 *
 * It will trigger a #ALLPLAY_RESPONSE_WIFIENABLE response message
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param enable   TRUE to bring Wi-Fi up, FALSE to bring it down
 *
 * @return Return #allplay_status
 */
allplay_status allplay_wifi_enable(allplay_ctx_t *apctx, void *userData, bool_t enable);

/**
 * Enable/Disable Direct Mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_DIRECTENABLE response message
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param enable   TRUE to enable, FALSE to disable
 *
 * @return Return #allplay_status
 */
allplay_status allplay_directmode_enable(allplay_ctx_t *apctx, void *userData, bool_t enable);

/**
 * Execute a command on the SAM using the system() function.
 *
 * It will trigger a #ALLPLAY_RESPONSE_RUN_CMD response message with
 * #allplay_cmd_t which will contain an ID. This ID can be used to identify
 * the matching result when receiving the #ALLPLAY_EVENT_CMD_RESULT event
 * message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param cmd      The command to execute
 * @param stdoutMaxLen Maximum length for stdout (not including the
 *        null-terminating character).
 *        If the command prints more than stdoutMaxLen characters to the
 *        standard output, the result will be truncated.
 * @param stderrMaxLen Maximum length for stderr. (not including the
 *        null-terminating character).
 *        If the command prints more than stderMaxLen characters to the
 *        standard error, the result will be truncated.
 *
 * @return Return #allplay_status
 */
allplay_status allplay_run_cmd(allplay_ctx_t *apctx, void *userData, const char *cmd, uint32_t stdoutMaxLen, uint32_t stderrMaxLen);

/**
 * Check if there is a new firmware update available.
 * It will trigger a #ALLPLAY_RESPONSE_FIRMWARE_CHECK response message with
 * #allplay_firmware_update_info_t.
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_firmware_check(allplay_ctx_t *apctx, void *userData);

/**
 * Update the firmware to the latest version.
 * It will trigger a #ALLPLAY_RESPONSE_FIRMWARE_UPDATE response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_firmware_update(allplay_ctx_t *apctx, void *userData);

/**
 * Update the firmware to the version specified in @a url.
 * It will trigger a #ALLPLAY_RESPONSE_FIRMWARE_UPDATE response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param url      URL of the firmware to update to.
 *
 * @return Return #allplay_status
 */
allplay_status allplay_firmware_update_from_url(allplay_ctx_t *apctx, void *userData, const char *url);

/**
 * Connect the module to the access point with the given @a ssid and @a password.
 * It will trigger a #ALLPLAY_RESPONSE_CONNECT_WIFI response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param ssid     The SSID of the desired network
 * @param password The password of the desired network
 *
 * @return Return #allplay_status
 */
allplay_status allplay_connect_wifi(allplay_ctx_t *apctx, void *userData, const char *ssid, const char *password);

/**
 * Get the allplay friendly name.
 *
 * It will trigger an #ALLPLAY_RESPONSE_GET_FRIENDLY_NAME response message with  #allplay_friendly_name_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_friendly_name(allplay_ctx_t *apctx, void *userData);

/**
 * Notify the SAM of the MCU idle state.
 *
 * It will trigger an #ALLPLAY_RESPONSE_SET_MCU_IDLE response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param idle     The MCU idle state
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_mcu_idle(allplay_ctx_t *apctx, void *userData, bool_t idle);

/**
 * Notify the SAM of the MCU battery state.
 *
 * It will trigger an #ALLPLAY_RESPONSE_SET_BATTERY_STATE response message.
 *
 * @param apctx            The allplay context
 * @param userData         User data
 * @param onBattery        TRUE if the device is on battery power, FALSE otherwise
 * @param chargeLevel      Percentage of charge of the battery, -1 if unknown
 * @param batteryAutonomy  Estimated time in seconds, until the device will
 *        shutdown given the current charge of the battery, assuming the device
 *        is switched to battery power now if not the case already (-1 if
 *        unknown)
 * @param timeToFullCharge Estimated time in second until the battery is fully
 *        charged (-1 if unknown)
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_battery_state(allplay_ctx_t *apctx, void *userData, bool_t onBattery, char chargeLevel, int32_t batteryAutonomy, int32_t timeToFullCharge);

/**
 * Set the current "external" audio input source. This is used to let the SAM
 * know that the audio output is connected to another source than the SAM
 * (e.g. Line-in > MCU/DSP > Line-out) or to lock the SAM on a specific input
 * (e.g. if the speaker has a physical switch set to Bluetooth, the SAM
 * shouldn't be allowed to switch to a HTTP stream).
 *
 * It will trigger an #ALLPLAY_RESPONSE_SET_EXTERNAL_SOURCE response message.
 *
 * @param apctx                The allplay context
 * @param userData             User data
 * @param name                 Name of the audio source. There are 3 types of
 *        names:
 *        - an empty name, or "allplay", allows the SAM to play any source
 *        supported by the SAM (default SAM state)
 *        - "allplay:<source>" makes the SAM play that specific source.
 *        Currently only "allplay:bluetooth" and "allplay:linein" are
 *        supported. Any other "allplay:[...]" sources are reserved and will
 *        currently return an error. You can optionally set the display name
 *        for Line-In using a URL encoded "title" parameter (e.g.
 *        "allplay:linein?title=My%20AUX%20source")
 *        - any other strings (e.g. "AUX-IN" or "S/PDIF") indicates that MCU
 *        will handle the playback. The SAM will only report playing that
 *        source to AllPlay applications.
 * @param interruptible        TRUE if the SAM can change the input source on
 *        request from other applications (AllPlay controllers, Spotify, ...).
 *        The MCU must be able to update its user interface to reflect the
 *        change of the source as needed. If FALSE, the SAM will not accept any
 *        Play request or playlist change from AllPlay controllers, and other
 *        non-AllPlay sources (e.g. Spotify, AirPlay, ...) are disabled and the
 *        device will not be discoverable in their respective application
 *        (Spotify app, iOS, ...).
 * @param volumeCtrlEnabled    TRUE if AllPlay can still control the volume
 *        when the input source is not AllPlay (i.e. the MCU controls the
 *        volume via the #ALLPLAY_EVENT_VOLUME_CHANGED event from the SAM).
 *
 * @return Return #allplay_status
 */
allplay_status allplay_set_external_source(allplay_ctx_t *apctx, void *userData, const char *name, bool_t interruptible, bool_t volumeCtrlEnabled);

/**
 * Enable or disable bluetooth.
 *
 * It will trigger an #ALLPLAY_RESPONSE_BLUETOOTH_ENABLE response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param enabled  TRUE to enable the Bluetooth, FALSE to disable it
 * @return Return #allplay_status
 */
allplay_status allplay_bluetooth_enable(allplay_ctx_t *apctx, void *userData, bool_t enabled);

/**
 * Enable or disable Bluetooth paring.
 *
 * It will trigger an #ALLPLAY_RESPONSE_BLUETOOTH_ENABLE_PAIRING response message.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param enabled  TRUE to enable the pairing, FALSE to disable it
 * @return Return #allplay_status
 */
allplay_status allplay_bluetooth_enable_pairing(allplay_ctx_t *apctx, void *userData, bool_t enabled);

/**
 * Get the Bluetooth state.
 *
 * It will trigger an #ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE response message with  #allplay_bluetooth_state_t.
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @return Return #allplay_status
 */
allplay_status allplay_bluetooth_get_state(allplay_ctx_t *apctx, void *userData);

/**
 * Play the given media item.
 *
 * It will trigger a #ALLPLAY_RESPONSE_PLAY response message with #allplay_message_t
 *
 * @param apctx         The allplay context
 * @param userData      User data
 * @param url           URL of item to play
 * @param title         Title of item to play
 * @param artist        Aritst of item to play
 * @param thumbnailUrl  Thumbnail url of item to play
 * @param duration      Duration of item to play
 * @param album         Album of item to play
 * @param genre         Genre of item to play
 *
 * @return Return #allplay_status
 */
allplay_status allplay_play_item(allplay_ctx_t *apctx, void *userData, const char *url,
	const char *title, const char *artist, const char *thumbnailUrl, int32_t duration, const char *album, const char *genre);

/**
 * Get the some information about the device.
 *
 * It will trigger a #ALLPLAY_RESPONSE_GET_DEVICE_INFO response message with #allplay_device_info_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_device_info(allplay_ctx_t *apctx, void *userData);

/**
 * Get the URL of the current item.
 *
 * It will trigger a #ALLPLAY_RESPONSE_GET_CURRENT_ITEM_URL response message with #allplay_url_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_current_item_url(allplay_ctx_t *apctx, void *userData);

/**
 * Get the current loop mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_GET_LOOP_MODE response message with #allplay_loop_mode_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_loop_mode(allplay_ctx_t *apctx, void *userData);

/**
 * Advance the loop mode (cycle through the 3 available modes).
 *
 * It will trigger a #ALLPLAY_RESPONSE_ADVANCE_LOOP_MODE response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_advance_loop_mode(allplay_ctx_t *apctx, void *userData);

/**
 * Get the current shuffle mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_GET_SHUFFLE_MODE response message with #allplay_shuffle_mode_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_get_shuffle_mode(allplay_ctx_t *apctx, void *userData);

/**
 * Toggle the shuffle mode.
 *
 * It will trigger a #ALLPLAY_RESPONSE_TOGGLE_SHUFFLE_MODE response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 *
 * @return Return #allplay_status
 */
allplay_status allplay_toggle_shuffle_mode(allplay_ctx_t *apctx, void *userData);

/**
 * Trigger a shutdown of the SAM
 *
 * It will trigger a #ALLPLAY_RESPONSE_SHUTDOWN_SAM response message with #allplay_message_t
 *
 * @param apctx    The allplay context
 * @param userData User data
 * @param restart  If TRUE, the SAM will be rebooted, if FALSE, it will be halted
 *
 * @return Return #allplay_status
 */
allplay_status allplay_shutdown_sam(allplay_ctx_t *apctx, void *userData, bool_t restart);

#ifdef __cplusplus
}
#endif

#endif /* ALLPLAYMCU_H_ */
