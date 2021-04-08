/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/
#include "allplaymcu.h"
#include "aj_crypto_ecc.h"
#include "aj_cert.h"
#include "aj_peer.h"

#ifndef NDEBUG
#define AJ_MODULE ALLPLAY_MAIN
uint8_t dbgALLPLAY_MAIN = 0;
#endif

#if defined(ALLPLAY_WIN32)
#include <conio.h>
#elif defined(ALLPLAY_LINUX) // ALLPLAY_WIN32
#include <termios.h>
#include <sys/select.h>
#endif // ALLPLAY_LINUX

#include <stdlib.h>
#include <stdio.h>

#if defined(ALLPLAY_LINUX) && defined(AJ_SERIAL_CONNECTION)
#include <aj_serio.h>
AJ_SerIOConfig g_serialConfig;
#endif

// Global data we want to track
enum allplay_player_state_value g_playerState = ALLPLAY_PLAYER_STATE_UNKNOWN;

static const char *commandHelpText = "\
Command summary:\n\
\n\
  System commands:\n\
    sysmode                             - Get the system mode\n\
    devinfo                             - Get device info\n\
    netinfo                             - Get network info\n\
    name                                - Get the friendly name\n\
    directmode <on/off>                 - Turn direct mode on or off\n\
    wifi <on/off>                       - Turn Wi-Fi on or off\n\
    wifi connect <ssid> [passphrase]    - Connect to Wi-Fi network\n\
    wifi rssi                           - Get RSSI info\n\
    wps [pin]                           - Start WPS\n\
    wps cancel                          - Cancel WPS\n\
    bt state                            - Get the Bluetooth power state\n\
    bt power <on/off>                   - Turn Bluetooth on or off\n\
    bt pairing <on/off>                 - Turn Bluetooth pairing on or off\n\
    unconfigure                         - Go to unconfigured mode (start setup)\n\
    freset <reboot/halt>                - Run factory reset\n\
    firmware version                    - Get the firmware version\n\
    firmware check                      - Check if firmware update is available\n\
    firmware update [url]               - Update firmware\n\
    mcu idle <yes/no>                   - Set the MCU idle state\n\
    mcu battery <on> <level> <te> <tf>  - Set MCU battery info\n\
                                            on: On battery power (yes/no)\n\
                                            level: Battery level in percent (0-100) or -1 (unknown)\n\
                                            te: Time in seconds to battery empty\n\
                                            tf: Time in seconds to battery full\n\
    resamplemode [mode]                 - Set or get the resampling mode\n\
                                            mode: (0 for 'sound' configuration in the DTB, x for 'soundx')\n\
    runcmd <command>                    - Run shell command\n\
    shutdown <halt/reboot>              - Halt or reboot the SAM\n\
\n\
  Media player commands:\n\
    play [url]                          - Play a URL (or resume if no URL)\n\
    stop                                - Stop playback\n\
    pause                               - Pause playback\n\
    pause toggle                        - Pause/play toggle\n\
    next                                - Skip to next track\n\
    prev                                - Skip to previous track\n\
    volume [vol]                        - Get or set volume in percent (0-100)\n\
    volume adjust <delta>               - Adjust the volume in raw volume units\n\
    mute <on/off>                       - Turn mute on or off\n\
    playerstate                         - Get the player state\n\
    url                                 - Get the current stream URL\n\
    mcu aux <source> <int> <vol>        - Set MCU aux input\n\
                                            source: MCU source name, 'allplay' for normal AllPlay input\n\
                                            or 'allplay:<source>' for sources on the SAM module\n\
                                            (allplay:linein, allplay:bluetooth, etc)\n\
                                            int: Interruptible mode (yes/no)\n\
                                            vol: Allow volume control (yes/no)\n\
    loop                                - Get the current loop mode\n\
    loop toggle                         - Toggle between the different loop modes\n\
    shuffle                             - Get the current shuffle mode\n\
    shuffle toggle                      - Toggle between the different shuffle modes\n\
\n\
  General commands:\n\
    quit                                - Quit the program\n\
    help                                - Display this help\n";

static void printHelp(void) {
	AJ_Printf("%s", commandHelpText);
}

/* Sample AllJoyn interface to show how to expose a method and send a signal */

#define SAMPLE_SERVICE_NAME_PREFIX "com.example.mcu.service.sample"
#define SAMPLE_SERVICE_PATH "/samplePath"
#define SERVICE_PORT ((uint16_t)101)

/*
 * Default key expiration
 */
static const uint32_t keyexpiration = 0xFFFFFFFF;

/* OME MUST change this to be a new password that is longer than 64 bytes */
static const char psk_char[] = "CH@NG3_th1$_p@s$W0rd_CH@NG3_th1$_p@s$W0rd_CH@NG3_th1$_p@s$W0rd_CH@NG3_th1$_p@s$W0rd_CH@NG3_th1$_p@s$W0rd_CH@NG3_th1$_p@s$W0rd!";

AJ_BusAttachment *bus;
uint32_t sessionId = 0;
char* serviceName = NULL;

static const char* sampleInterface[] = {
	"$com.example.mcu.service.interface.sample", //Secure interface
	"?someMethod value<i",
	"!aSignal value>i",
	NULL
};

static const AJ_InterfaceDescription sampleInterfaces[] = {
	sampleInterface,
	NULL
};

static const AJ_Object MyObjects[] = {
	{ SAMPLE_SERVICE_PATH, sampleInterfaces, 0, NULL },
	{ NULL, NULL, 0, NULL }
};

#define SOME_METHOD AJ_APP_MESSAGE_ID(0, 0, 0)
#define A_SIGNAL AJ_APP_MESSAGE_ID(0, 0, 1)
/* End of AllJoyn service setup */

/* AllJoyn callback to setup the service and process the messages */
static AJ_Status AuthListenerCallback(uint32_t authmechanism, uint32_t command, AJ_Credential*cred)
{
	AJ_Status status = AJ_ERR_INVALID;
	AJ_InfoPrintf(("AuthListenerCallback authmechanism %d command %d\n", authmechanism, command));

	switch (authmechanism) {
		case AUTH_SUITE_ECDHE_PSK:
			switch (command) {
				case AJ_CRED_PUB_KEY: /* 2 */
					break; // Don't use username - use anonymous

				case AJ_CRED_PRV_KEY: /* 1 */
					cred->mask = AJ_CRED_PRV_KEY;
					cred->data = (uint8_t*) psk_char;
					cred->len = strlen(psk_char);
					cred->expiration = keyexpiration;
					status = AJ_OK;
					break;
			}
			break;

		default:
			break;
	}
	return status;
}

static const uint32_t suites[1] = { AUTH_SUITE_ECDHE_PSK };
static const size_t numsuites = sizeof(suites) / sizeof(suites[0]);

void AuthCallback(const void* context, AJ_Status status)
{
	*((AJ_Status*)context) = status;
}

bool_t myProcessMessage(AJ_Message *ajMsg)
{
	AJ_Status status;
	if (ajMsg->hdr == NULL) {
		return FALSE;
	}
	switch(ajMsg->msgId) {
		case AJ_REPLY_ID(AJ_METHOD_BIND_SESSION_PORT):
			if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
				AJ_ErrPrintf(("AJ_METHOD_BIND_SESSION_PORT: AJ_ERR_FAILURE\n"));
			} else {
				status = AJ_BusRequestName(bus, serviceName, AJ_NAME_REQ_DO_NOT_QUEUE);
			}
			return TRUE;
		case AJ_REPLY_ID(AJ_METHOD_REQUEST_NAME):
			if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
				AJ_ErrPrintf(("AJ_METHOD_REQUEST_NAME: AJ_ERR_FAILURE\n"));
			} else {
				status = AJ_BusAdvertiseName(bus, serviceName, AJ_TRANSPORT_ANY, AJ_BUS_START_ADVERTISING, 0);
			}
			return TRUE;
		case AJ_REPLY_ID(AJ_METHOD_ADVERTISE_NAME):
			if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
				AJ_ErrPrintf(("AJ_METHOD_ADVERTISE_NAME: AJ_ERR_FAILURE\n"));
			} else {
				//Now set the password callback since we are using a secure interface
				AJ_BusEnableSecurity(bus, suites, numsuites);
				AJ_BusSetAuthListenerCallback(bus, AuthListenerCallback);
			}
			return TRUE;
		case AJ_METHOD_ACCEPT_SESSION:
			{
				uint16_t port;
				char* joiner;
				AJ_UnmarshalArgs(ajMsg, "qus", &port, &sessionId, &joiner);
				if (port == SERVICE_PORT) {
					status = AJ_BusReplyAcceptSession(ajMsg, TRUE);
					//Authenticate with the other side so they can receive signals right away
					AJ_Status authStatus = AJ_ERR_NULL;
					status = AJ_BusAuthenticatePeer(bus, joiner, AuthCallback, &authStatus);
				} else {
					status = AJ_BusReplyAcceptSession(ajMsg, FALSE);
				}
			}
			return TRUE;
		case SOME_METHOD:
			{
				uint32_t value;
				AJ_Message reply;
				AJ_UnmarshalArgs(ajMsg, "i", &value);
				AJ_InfoPrintf(("Received someMethod call from a remote device with arg: %d\n", value));
				//now send the reply back
				AJ_MarshalReplyMsg(ajMsg, &reply);
				AJ_DeliverMsg(&reply);
			}
			return TRUE;
	}
	return FALSE;
}

void mySetupService(AJ_BusAttachment* theBus, const char *deviceId)
{
	AJ_SessionOpts opts;

	//Store off and save the BusAttachment so that we can use it in our service code
	bus = theBus;

	//Setup our advertisement name
	serviceName = AJ_Malloc(strlen(SAMPLE_SERVICE_NAME_PREFIX) + strlen(deviceId) + 1);
	strcpy(serviceName, SAMPLE_SERVICE_NAME_PREFIX);
	strcpy(serviceName+strlen(SAMPLE_SERVICE_NAME_PREFIX), deviceId);
	AJ_InfoPrintf(("My advertised Service name: %s\n",serviceName));

	//Setup the sessoin options
	opts.isMultipoint = 1;
	opts.traffic = 0x1;
	opts.transports = 0xFFFF;
	opts.proximity = 0xFF;

	//Start the process of setting up the service (Bind -> RequestName -> AdvertiseName)
	AJ_BusBindSessionPort(bus, SERVICE_PORT, &opts, 0);
}

void sendMySignal()
{
	AJ_Message msg;
	AJ_Status status;

	// Check that we have someone connected, IE the sessionId has been set in our multipoint session
	if (sessionId == 0) {
		return;
	}
	AJ_Printf("Sending a sample signal on session id: %u\n", sessionId);

	status = AJ_MarshalSignal(bus, &msg, A_SIGNAL, NULL, sessionId, 0, 0);
	if (status == AJ_OK) {
		status = AJ_MarshalArgs(&msg, "i", 54321);
	}
	if (status == AJ_OK) {
		status = AJ_DeliverMsg(&msg);
	}
	/*
	if (status != AJ_OK) {
		AJ_Printf("Failed to send the signal %d\n", status);
	} else {
		AJ_Printf("Sent the signal\n");
	}
	*/
}


void print_network_info(allplay_network_info_t *netInfo)
{
	char *type;
	switch (netInfo->type) {
		case ALLPLAY_NETWORK_NONE: type = "None"; break;
		case ALLPLAY_NETWORK_WIFI: type = "Wifi"; break;
		case ALLPLAY_NETWORK_ETHERNET: type = "Ethernet"; break;
		default: type = "Unknown"; break;
	}

	AJ_Printf("Network info:\n"
		"   type: %s\n"
		"   wifi ip: %hhu.%hhu.%hhu.%hhu\n"
		"   wifi mac: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n"
		"   eth ip: %hhu.%hhu.%hhu.%hhu\n"
		"   eth mac: %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx\n"
		"   ssid: %s\n"
		"   rssi: %d\n"
		"   frequency: %d\n",
		type,
		netInfo->wifiIp[0], netInfo->wifiIp[1], netInfo->wifiIp[2], netInfo->wifiIp[3],
		netInfo->wifiMac[0], netInfo->wifiMac[1], netInfo->wifiMac[2], netInfo->wifiMac[3], netInfo->wifiMac[4], netInfo->wifiMac[5],
		netInfo->ethernetIp[0], netInfo->ethernetIp[1], netInfo->ethernetIp[2], netInfo->ethernetIp[3],
		netInfo->ethernetMac[0], netInfo->ethernetMac[1], netInfo->ethernetMac[2], netInfo->ethernetMac[3], netInfo->ethernetMac[4], netInfo->ethernetMac[5],
		netInfo->ssid,
		netInfo->rssi,
		netInfo->frequency);
}

#if defined(ALLPLAY_LINUX) && defined(AJ_SERIAL_CONNECTION)
int parseArgs(int argc, char *argv[]) {
	int result;
	char type;

	if (argc < 4) {
		AJ_Printf("Missing parameter:\n\t%s [device] [bitrate] [configuration, e.g. '8N1', '7E2', '5O1']\n", argv[0]);
		return 1;
	}

	g_serialConfig.config = (void*)argv[1];
	g_serialConfig.bitrate = atoi(argv[2]);

	result = sscanf(argv[3], "%hhu%c%hhu", &g_serialConfig.bits, &type, &g_serialConfig.stopBits);
	if (result != 3) {
		AJ_Printf("Wrong configuration '%s' string\n", argv[3]);
		return 1;
	}
	switch (type) {
	case 'N':
	case 'n':
		g_serialConfig.parity = 0;
		break;
	case 'O':
	case 'o':
		g_serialConfig.parity = 1;
		break;
	case 'E':
	case 'e':
		g_serialConfig.parity = 2;
		break;
	default:
		AJ_Printf("Wrong configuration '%s' string\n", argv[3]);
		return 1;
	}

	return 0;
}
#endif

static void cmd_devinfo(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_device_info(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_device_info: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_wifi(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	const char *ssid;
	const char *password;

	if ((argc == 1) && (strcmp(argv[0], "on") == 0)) {
		AJ_Printf("Bringing up Wi-Fi... ");
		status = allplay_wifi_enable(apctx, (void*)0, TRUE);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_wifi_enable (true): %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "off") == 0)) {
		AJ_Printf("Bringing down Wi-Fi... ");
		status = allplay_wifi_enable(apctx, (void*)0, FALSE);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_wifi_enable (false): %d\n", status);
		}
	}
	else if ((argc >= 2) && (argc <= 3) && (strcmp(argv[0], "connect") == 0)) {
		ssid = argv[1];
		password = (argc == 3) ? argv[2] : "";
		AJ_Printf("\nConnecting to \"%s\" with \"%s\"...\n", ssid, password);
		status = allplay_connect_wifi(apctx, (void*)0, ssid, password);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_connect_wifi: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "rssi") == 0)) {
		status = allplay_get_rssi(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_rssi: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_wps(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	int32_t pin;

	if (argc == 0) {
		status = allplay_network_request_wps(apctx, (void*)0, ALLPLAY_NETWORK_WPS_PBC, 0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_network_request_wps (pbc): %d\n", status);
		}
	}
	else if ((argc == 2) && (strcmp(argv[0], "pin") == 0)) {
		pin = atoi(argv[1]);
		status = allplay_network_request_wps(apctx, (void*)0, ALLPLAY_NETWORK_WPS_PIN, pin);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_network_request_wps (pin): %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "cancel") == 0)) {
		status = allplay_network_request_wps(apctx, (void*)0, ALLPLAY_NETWORK_WPS_CANCEL, 0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_network_request_wps (cancel): %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_directmode(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if ((argc == 1) && (strcmp(argv[0], "on") == 0)) {
		AJ_Printf("Enabling Direct Mode... ");
		status = allplay_directmode_enable(apctx, (void*)0, TRUE);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_directmode_enable (true): %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "off") == 0)) {
		AJ_Printf("Disabling Direct Mode... ");
		status = allplay_directmode_enable(apctx, (void*)0, FALSE);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_directmode_enable (false): %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_network_info(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_network_get_info(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_network_get_info: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_get_system_mode(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_system_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_system_mode: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_firmware(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if ((argc == 1) && (strcmp(argv[0], "version") == 0)) {
		status = allplay_get_firmware_version(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_firmware_version: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "check") == 0)) {
		status = allplay_firmware_check(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_firmware_check: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "update") == 0)) {
		status = allplay_firmware_update(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_firmware_update: %d\n", status);
		}
	}
	else if ((argc == 2) && (strcmp(argv[0], "update") == 0)) {
		status = allplay_firmware_update_from_url(apctx, (void*)0, argv[1]);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_firmware_update_from_url: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_start_setup(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_start_setup(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_start_setup: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_factory_reset(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	allplay_reset_action_t action;

	if (argc == 0) {
		action = ALLPLAY_RESET_ACTION_REBOOT;
	}
	else if (argc == 1) {
		if (strcmp("reboot", argv[0]) == 0) {
			action = ALLPLAY_RESET_ACTION_REBOOT;
		}
		else if (strcmp("halt", argv[0]) == 0) {
			action = ALLPLAY_RESET_ACTION_HALT;
		}
		else {
			AJ_Printf("Invalid argument for factory reset: %s\n", argv[0]);
			return;
		}
	}
	else {
		printHelp();
		return;
	}

	status = allplay_reset_to_factory(apctx, (void*)0, action);
	if (status != ALLPLAY_ERROR_NONE) {
		AJ_Printf("Failed to call allplay_reset_to_factory: %d\n", status);
	}
}

static void cmd_play(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_play(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_play: %d\n", status);
		}
	}
	else if (argc == 1) {
		status = allplay_play_item(apctx, (void*)0, argv[0],
			"some title", "some artist", "http://127.0.0.1/thumbnail.jpg", 0, "some album", "some genre");
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_play_item: %d\n", status);
		}
	}
	else {
		printHelp();
	}

}

static void cmd_pause(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_pause(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_pause: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "toggle") == 0)) {
		if (g_playerState == ALLPLAY_PLAYER_STATE_PLAYING) {
			status = allplay_pause(apctx, (void*)0);
			if (status != ALLPLAY_ERROR_NONE) {
				AJ_Printf("Failed to call allplay_pause: %d\n", status);
			}
		}
		else {
			status = allplay_play(apctx, (void*)0);
			if (status != ALLPLAY_ERROR_NONE) {
				AJ_Printf("Failed to call allplay_play: %d\n", status);
			}
		}
	}
	else {
		printHelp();
	}
}

static void cmd_stop(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_stop(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_stop: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_next(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_next(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_next: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_previous(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_previous(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_previous: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_get_player_state(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_player_state(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_player_state: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_get_url(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_current_item_url(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_player_state: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_mute(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	int mute;

	if (argc == 1) {
		if (strcmp(argv[0], "on") == 0) {
			mute = 1;
		}
		else if (strcmp(argv[0], "off") == 0) {
			mute = 0;
		}
		else {
			AJ_Printf("Invalid mute state: %s\n", argv[0]);
			return;
		}

		status = allplay_mute(apctx, (void*)0, mute);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_mute: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_volume(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_volume_info(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_next: %d\n", status);
		}
	}
	else if (argc == 1) {
		status = allplay_set_volume(apctx, (void*)0, atoi(argv[0]));
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_set_volume_info: %d\n", status);
		}
	}
	else if ((argc == 2) && (strcmp(argv[0], "adjust") == 0)) {
		status = allplay_volume_adjust(apctx, (void*)0, atoi(argv[1]));
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_volume_adjust: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_resampling_mode(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_resampling_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_resampling_mode: %d\n", status);
		}
	}
	else if (argc == 1) {
		status = allplay_set_resampling_mode(apctx, (void*)0, atoi(argv[0]));
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_set_resampling_mode: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_loop(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_loop_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_loop_mode: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "toggle") == 0)) {
		status = allplay_advance_loop_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_advance_loop_mode: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_shuffle(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_shuffle_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_shuffle_mode: %d\n", status);
		}
	}
	else if ((argc == 1) && (strcmp(argv[0], "toggle") == 0)) {
		status = allplay_toggle_shuffle_mode(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_toggle_shuffle_mode: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_run_cmd(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 1) {
		status = allplay_run_cmd(apctx, (void*)0, argv[0], 1024, 1024);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_run_cmd: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_shutdown(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;

	if (argc == 1) {
		bool_t restart;
		if (strcmp(argv[0], "halt") == 0) {
			restart = FALSE;
		}
		else if (strcmp(argv[0], "reboot") == 0) {
			restart = TRUE;
		}
		else {
			AJ_Printf("Invalid shutdown value: %s\n", argv[0]);
			return;
		}

		status = allplay_shutdown_sam(apctx, (void*)0, restart);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_run_cmd: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_get_name(allplay_ctx_t *apctx, int argc, char **UNUSED(argv)) {
	allplay_status status;

	if (argc == 0) {
		status = allplay_get_friendly_name(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_get_friendly_name: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static int string_to_bool(const char *str, bool_t *b) {
	if ((strcmp(str, "yes") == 0) || (strcmp(str, "on") == 0)) {
		*b = TRUE;
		return 0;
	}
	else if ((strcmp(str, "no") == 0) || (strcmp(str, "off") == 0)) {
		*b = FALSE;
		return 0;
	}
	return -1;
}

static void cmd_mcu(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	bool_t idle;
	bool_t battPowered;
	bool_t interrupt;
	bool_t allowVol;

	if ((argc == 2) && (strcmp(argv[0], "idle") == 0)) {
		if (string_to_bool(argv[1], &idle) != 0) {
			AJ_Printf("Invalid argument for mcu idle: %s\n", argv[1]);
			return;
		}
		status = allplay_set_mcu_idle(apctx, (void*)0, idle);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_set_mcu_idle: %d\n", status);
		}
	}
	else if ((argc == 5) && (strcmp(argv[0], "battery") == 0)) {
		if (string_to_bool(argv[1], &battPowered) != 0) {
			AJ_Printf("Invalid argument for battery: %s\n", argv[1]);
			return;
		}
		status = allplay_set_battery_state(apctx, (void*)0, battPowered, atoi(argv[2]), atoi(argv[3]), atoi(argv[4]));
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_set_battery_state: %d\n", status);
		}
	}
	else if ((argc == 4) && (strcmp(argv[0], "aux") == 0)) {
		if (string_to_bool(argv[2], &interrupt) != 0) {
			AJ_Printf("Invalid argument for aux interruptible: %s\n", argv[2]);
			return;
		}
		if (string_to_bool(argv[3], &allowVol) != 0) {
			AJ_Printf("Invalid argument for aux allow volume control: %s\n", argv[3]);
			return;
		}
		status = allplay_set_external_source(apctx, (void*)0, argv[1], interrupt, allowVol);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_set_external_source: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_bt(allplay_ctx_t *apctx, int argc, char **argv) {
	allplay_status status;
	bool_t power;
	bool_t pairing;

	if ((argc == 1) && (strcmp(argv[0], "state") == 0)) {
		status = allplay_bluetooth_get_state(apctx, (void*)0);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_bluetooth_get_state: %d\n", status);
		}
	}
	else if ((argc == 2) && (strcmp(argv[0], "power") == 0)) {
		if (string_to_bool(argv[1], &power) != 0) {
			AJ_Printf("Invalid argument for bt power: %s\n", argv[1]);
			return;
		}
		status = allplay_bluetooth_enable(apctx, (void*)0, power);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_bluetooth_enable: %d\n", status);
		}
	}
	else if ((argc == 2) && (strcmp(argv[0], "pairing") == 0)) {
		if (string_to_bool(argv[1], &pairing) != 0) {
			AJ_Printf("Invalid argument for bt pairing: %s\n", argv[1]);
			return;
		}
		status = allplay_bluetooth_enable_pairing(apctx, (void*)0, pairing);
		if (status != ALLPLAY_ERROR_NONE) {
			AJ_Printf("Failed to call allplay_bluetooth_enable_pairing: %d\n", status);
		}
	}
	else {
		printHelp();
	}
}

static void cmd_help(allplay_ctx_t *UNUSED(apctx), int UNUSED(argc), char **UNUSED(argv)) {
	printHelp();
}

static void cmd_signal(allplay_ctx_t *UNUSED(apctx), int argc, char **UNUSED(argv)) {
	if (argc == 0) {
		sendMySignal();
	}
	else {
		printHelp();
	}
}

typedef struct CmdEntry {
	const char *name;
	void (*func)(allplay_ctx_t *apctx, int argc, char **argv);
} CmdEntry;

static CmdEntry cmdTable[] = {
	{ "devinfo", cmd_devinfo },
	{ "wifi", cmd_wifi },
	{ "wps", cmd_wps },
	{ "directmode", cmd_directmode },
	{ "netinfo", cmd_network_info },
	{ "sysmode", cmd_get_system_mode },
	{ "firmware", cmd_firmware },
	{ "unconfigure", cmd_start_setup },
	{ "freset", cmd_factory_reset },
	{ "play", cmd_play },
	{ "pause", cmd_pause },
	{ "stop", cmd_stop },
	{ "next", cmd_next },
	{ "prev", cmd_previous },
	{ "playerstate", cmd_get_player_state },
	{ "url", cmd_get_url },
	{ "mute", cmd_mute },
	{ "volume", cmd_volume },
	{ "resamplemode", cmd_resampling_mode },
	{ "loop", cmd_loop },
	{ "shuffle", cmd_shuffle },
	{ "runcmd", cmd_run_cmd },
	{ "shutdown", cmd_shutdown },
	{ "name", cmd_get_name },
	{ "mcu", cmd_mcu },
	{ "bt", cmd_bt },
	{ "help", cmd_help },
	{ "signal", cmd_signal }
};

static void process_message(allplay_ctx_t *apctx, int timeout) {
	allplay_message_t* apmsg;

	apmsg = allplay_read_message(apctx, timeout);
	if (!apmsg) {
		return;
	}

	switch (apmsg->messageType) {
		// Errors
		case ALLPLAY_ERROR:
			{
				allplay_status errorCode = ((allplay_error_t*)apmsg)->code;
				switch (errorCode) {
					case ALLPLAY_ERROR_FAILED:
						//...
						break;
					case ALLPLAY_ERROR_NONE:
					case ALLPLAY_NOT_CONNECTED:
						// should never happen, just silence the compiler
						break;
				}
			}
			break;
		// Responses
		case ALLPLAY_RESPONSE_VOLUME:
		case ALLPLAY_RESPONSE_VOLUME_ADJUST:
		case ALLPLAY_RESPONSE_MUTE:
		case ALLPLAY_RESPONSE_PLAY:
		case ALLPLAY_RESPONSE_PAUSE:
		case ALLPLAY_RESPONSE_STOP:
		case ALLPLAY_RESPONSE_NEXT:
		case ALLPLAY_RESPONSE_PREVIOUS:
		case ALLPLAY_RESPONSE_SET_POSITION:
		case ALLPLAY_RESPONSE_NETWORK_REQUEST_WPS:
		case ALLPLAY_RESPONSE_SETUP:
		case ALLPLAY_RESPONSE_RESET_TO_FACTORY:
		case ALLPLAY_RESPONSE_SET_RESAMPLING_MODE:
		case ALLPLAY_RESPONSE_WIFIENABLE:
		case ALLPLAY_RESPONSE_DIRECTENABLE:
		case ALLPLAY_RESPONSE_FIRMWARE_UPDATE:
		case ALLPLAY_RESPONSE_CONNECT_WIFI:
		case ALLPLAY_RESPONSE_SET_MCU_IDLE:
		case ALLPLAY_RESPONSE_SET_BATTERY_STATE:
		case ALLPLAY_RESPONSE_SET_EXTERNAL_SOURCE:
		case ALLPLAY_RESPONSE_BLUETOOTH_ENABLE:
		case ALLPLAY_RESPONSE_BLUETOOTH_ENABLE_PAIRING:
		case ALLPLAY_RESPONSE_PLAYITEM:
		case ALLPLAY_RESPONSE_ADVANCE_LOOP_MODE:
		case ALLPLAY_RESPONSE_TOGGLE_SHUFFLE_MODE:
		case ALLPLAY_RESPONSE_SHUTDOWN_SAM:
			// request was successful otherwise we would have received an ALLPLAY_ERROR_FAILED
			AJ_InfoPrintf(("Successful reply\n"));
			break;
		case ALLPLAY_RESPONSE_PLAYER_STATE:
		case ALLPLAY_EVENT_PLAYER_STATE_CHANGED:
			{
				allplay_player_state_t* streamInfo = (allplay_player_state_t*)apmsg;
				g_playerState = streamInfo->state;
				AJ_Printf("Stream info:\n"
						"   state: %d\n"
						"   duration: %d (%#x)\n"
						"   position: %d (%#x)\n"
						"   sample rate: %uHz\n"
						"   audio channels: %u\n"
						"   sample size: %u\n"
						"   title: %s\n"
						"   artist: %s\n"
						"   album: %s\n"
						"   source: %s\n",
						streamInfo->state,
						streamInfo->duration, streamInfo->duration,
						streamInfo->position, streamInfo->position,
						streamInfo->sampleRate,
						streamInfo->audioChannels,
						streamInfo->bitsPerSample,
						(streamInfo->title ? streamInfo->title : "<null>"),
						(streamInfo->artist ? streamInfo->artist : "<null>"),
						(streamInfo->album ? streamInfo->album : "<null>"),
						(streamInfo->contentSource ? streamInfo->contentSource : "<null>")
						);
			}
			break;
		case ALLPLAY_RESPONSE_RSSI:
			{
				size_t i = 0;
				allplay_rssi_t *rssiInfo = (allplay_rssi_t*)apmsg;
				AJ_Printf("RSSI:\n");
				while(i < sizeof(rssiInfo->chain)/sizeof(int32_t)) {
					AJ_Printf("   chain %zu: %d\n", i, rssiInfo->chain[i]);
					++i;
				}
			}
			break;
		case ALLPLAY_RESPONSE_NETWORK_INFO:
		case ALLPLAY_EVENT_NETWORK_INFO_CHANGED:
			{
				allplay_network_info_t *netInfo = (allplay_network_info_t*)apmsg;
				print_network_info(netInfo);
			}
			break;
		case ALLPLAY_RESPONSE_SYSTEM_MODE:
			{
				allplay_system_mode_t *systemMode = (allplay_system_mode_t*)apmsg;
				AJ_Printf("System mode: %d\n", systemMode->mode);
			}
			break;
		case ALLPLAY_RESPONSE_FIRMWARE_VERSION:
			{
				allplay_firmware_version_t *firmwareVersion = (allplay_firmware_version_t*)apmsg;
				AJ_Printf("Firmware Version: %s\n", firmwareVersion->version);
			}
			break;
		case ALLPLAY_RESPONSE_VOLUME_INFO:
			{
				allplay_volume_info_t *volumeInfo = (allplay_volume_info_t*)apmsg;
				AJ_Printf("Volume info: mute:%d, volume: %d (max: %d)\n", volumeInfo->mute, volumeInfo->volume, volumeInfo->max_volume);
			}
			break;
		case ALLPLAY_RESPONSE_RUN_CMD:
			{
				allplay_cmd_t *cmd = (allplay_cmd_t*)apmsg;
				AJ_Printf("Command id: %u\n", cmd->id);
			}
			break;
		case ALLPLAY_RESPONSE_FIRMWARE_CHECK:
			{
				allplay_firmware_update_info_t *firmwareInfo = (allplay_firmware_update_info_t*)apmsg;
				if (!firmwareInfo->version) {
					AJ_Printf("FirmwareCheck result: No new firmware\n");
				}
				else {
					AJ_Printf("FirmwareCheck result: New firmware: version %s (%s)\n", firmwareInfo->version, firmwareInfo->url);
				}
			}
			break;
		case ALLPLAY_RESPONSE_GET_RESAMPLING_MODE:
			{
				allplay_resampling_mode_t *mode = (allplay_resampling_mode_t*)apmsg;
				AJ_Printf("Resampling mode: %d\n", mode->resamplingMode);
			}
			break;
		case ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE:
		case ALLPLAY_EVENT_BLUETOOTH_STATE_CHANGED:
			{
				allplay_bluetooth_state_t *btState = (allplay_bluetooth_state_t*)apmsg;
				AJ_Printf("Bluetooth state:\n"
						"   Enabled: %s\n"
						"   Pairable: %s\n"
						"   # of BT devices: %d\n"
						"   # of A2DP devices: %d\n",
						btState->enabled ? "true" : "false",
						btState->pairable ? "true" : "false",
						btState->btDevicesCount, btState->a2dpDevicesCount);
			}
			break;
		case ALLPLAY_RESPONSE_GET_LOOP_MODE:
		case ALLPLAY_EVENT_LOOP_MODE_CHANGED:
			{
				allplay_loop_mode_t *mode = (allplay_loop_mode_t*)apmsg;
				const char *modeStr;
				switch (mode->mode) {
					case ALLPLAY_LOOP_NONE: modeStr = "None"; break;
					case ALLPLAY_LOOP_ONE: modeStr = "One"; break;
					case ALLPLAY_LOOP_ALL: modeStr = "All"; break;
					default: modeStr = "Unknown"; break;
				}
				AJ_Printf("Loop mode: %s\n", modeStr);
			}
			break;
		case ALLPLAY_RESPONSE_GET_SHUFFLE_MODE:
		case ALLPLAY_EVENT_SHUFFLE_MODE_CHANGED:
			{
				allplay_shuffle_mode_t *mode = (allplay_shuffle_mode_t*)apmsg;
				const char *modeStr;
				switch (mode->mode) {
				case ALLPLAY_SHUFFLE_LINEAR: modeStr = "Linear"; break;
					case ALLPLAY_SHUFFLE_SHUFFLED: modeStr = "Shuffle"; break;
					default: modeStr = "Unknown"; break;
				}
				AJ_Printf("Shuffle mode: %s\n", modeStr);
			}
			break;
		//...
		// Events
		case ALLPLAY_EVENT_CONNECTION_STATUS_CHANGED:
			AJ_InfoPrintf(("####\n####Connection status: %s\n####\n", allplay_is_connected(apctx) ? "true" : "false"));
			break;
		//case ALLPLAY_EVENT_PLAYER_STATE_CHANGED:
			// see ALLPLAY_RESPONSE_PLAYER_STATE
		case ALLPLAY_EVENT_VOLUME_CHANGED:
			{
				allplay_volume_info_t *volumeInfo = (allplay_volume_info_t*)apmsg;
				AJ_Printf("Volume info: mute:%d, volume: %d (max: %d)\n", volumeInfo->mute, volumeInfo->volume, volumeInfo->max_volume);
			}
			break;
		case ALLPLAY_EVENT_NETWORK_WPS_RESULT:
			{
				int result = ((allplay_network_wps_result_t*)apmsg)->result;
				AJ_Printf("WpsResult: %d\n", result);
			}
			break;
		case ALLPLAY_EVENT_SYSTEM_MODE:
			{
				int mode = ((allplay_system_mode_t*)apmsg)->mode;
				AJ_Printf("System mode: %d\n", mode);
			}
			break;
		case ALLPLAY_EVENT_CMD_RESULT:
			{
				allplay_cmd_result_t *cmdResult = (allplay_cmd_result_t*)apmsg;
				AJ_Printf("Command %u result: exitstatus %d, termsig %d\n", cmdResult->id, cmdResult->exitstatus, cmdResult->termsig);
				AJ_Printf("stdout=begin=============\n");
				AJ_Printf("%s\n", cmdResult->stdoutStr);
				AJ_Printf("stdout=end===============\n");
				AJ_Printf("stderr=begin=============\n");
				AJ_Printf("%s\n", cmdResult->stderrStr);
				AJ_Printf("stderr=end===============\n");
			}
			break;
		case ALLPLAY_EVENT_START_MCU_UPDATE:
			AJ_InfoPrintf(("Should reboot into the bootloader/update fw\n"));
			break;
		case ALLPLAY_EVENT_FIRMWARE_UPDATE_AVAILABLE:
			{
				allplay_firmware_update_info_t *firmwareInfo = (allplay_firmware_update_info_t*)apmsg;
				AJ_Printf("New firmware: version %s (%s)\n", firmwareInfo->version, firmwareInfo->url);
			}
			break;
		case ALLPLAY_EVENT_FIRMWARE_UPDATE_STATUS:
			{
				allplay_firmware_update_status_t *updateStatus = (allplay_firmware_update_status_t*)apmsg;
				AJ_Printf("Update status: %d\n", updateStatus->status);
			}
			break;
		case ALLPLAY_EVENT_REBOOT_STARTED:
			{
				AJ_InfoPrintf(("SAM is rebooting\n"));
			}
			break;
		case ALLPLAY_RESPONSE_GET_FRIENDLY_NAME:
			{
				allplay_friendly_name_t *friendly_name = (allplay_friendly_name_t *)apmsg;
				AJ_Printf("Friendly Name: \"%s\"\n",friendly_name->name);
			}
			break;
		case ALLPLAY_RESPONSE_GET_DEVICE_INFO:
			{
				allplay_device_info_t *deviceInfo = (allplay_device_info_t *)apmsg;
				AJ_Printf("Manufacturer: %s\n", deviceInfo->manufacturer);
				AJ_Printf("Model/Device: %s\n", deviceInfo->model);
				AJ_Printf("Device ID: %s\n", deviceInfo->deviceId);
				AJ_Printf("Firmware: %s\n", deviceInfo->firmwareVersion);
			}
			break;
		case ALLPLAY_RESPONSE_GET_CURRENT_ITEM_URL:
			{
				allplay_url_t *url = (allplay_url_t *)apmsg;
				AJ_Printf("Current playing URL: %s\n",url->url);
			}
			break;
	}

	allplay_free_message(apctx, &apmsg);
}

static char *read_input(void) {
#if defined(ALLPLAY_LINUX)
	fd_set readSet;
	struct timeval timeout = {0, 0};
	static char readBuf[1024];
	static int pos = 0;
	static int lastLen = 0;
	int result;
	char *newline;

	// Check for additional command from last read
	if (lastLen > 0) {
		memmove(readBuf, readBuf + lastLen, sizeof(readBuf) - lastLen);
		pos -= lastLen;
		newline = strchr(readBuf, '\n');
		if (newline != NULL) {
			*newline = '\0';
			lastLen = strlen(readBuf) + 1;
			return readBuf;
		}
		lastLen = 0;
	}

	FD_ZERO(&readSet);
	FD_SET(STDIN_FILENO, &readSet);
	select(STDIN_FILENO + 1, &readSet, NULL, NULL, &timeout);

	if (FD_ISSET(STDIN_FILENO, &readSet)) {
		result = read(STDIN_FILENO, readBuf + pos, sizeof(readBuf) - pos - 1);
		if (result > 0) {
			readBuf[pos + result] = '\0';
			readBuf[sizeof(readBuf) - 1] = '\n';
			pos += result;
			newline = strchr(readBuf, '\n');
			if (newline != NULL) {
				*newline = '\0';
				lastLen = strlen(readBuf) + 1;
				return readBuf;
			}
		}
	}

	return NULL;
#else // ALLPLAY_LINUX
	return NULL;  // TODO
#endif // !ALLPLAY_LINUX
}

int main(int argc, char *argv[]) {
	allplay_ctx_t* apctx;
	int timeout = 100; // 100 ms

#if defined(ALLPLAY_LINUX) && defined(AJ_SERIAL_CONNECTION)
	if (parseArgs(argc, argv) != 0) {
		return 1;
	}
#endif

	// initialize AllPlay
	apctx = allplay_new_with_mcu_service("1.2.3", ALLPLAY_REBOOT_SAM, MyObjects, myProcessMessage, mySetupService);
	if (!apctx) {
		return 1;
	}

	while (!allplay_is_connected(apctx)) {
		process_message(apctx, timeout);
	}

	// From anywhere in the code: make requests
	//status = allplay_stop(apctx, userData); // that one will return a basic allplay_message_t
	//status = allplay_get_stream_info(apctx, userData); // that one will return a allplay_stream_info_t*

	// Read initial player state
	allplay_get_player_state(apctx, (void*)0);

	// Get volume info so that set_volume calls can work
	allplay_get_volume_info(apctx, (void*)0);

	// Application main loop
	for (;;) {
		char *input;
		char *cmd;
		char *args[10];
		char *s;
		size_t i, iarg = 0;

		input = read_input();
		if ((input != NULL) && (input[0] != '\0')) {
			cmd = strtok(input, " ");
			if (cmd == NULL) {
				continue;
			}
			if ((strcmp(cmd, "quit") == 0) || (strcmp(cmd, "exit") == 0)) {
				break;
			}

			while ((s = strtok(NULL, " ")) != NULL) {
				if (iarg == sizeof(args)/sizeof(args[0])) {
					break;
				}
				args[iarg] = s;
				iarg++;
			}

			for (i = 0; i < sizeof(cmdTable)/sizeof(cmdTable[0]); i++) {
				if (strcmp(cmd, cmdTable[i].name) == 0) {
					cmdTable[i].func(apctx, iarg, args);
					break;
				}
			}
			if (i == sizeof(cmdTable)/sizeof(cmdTable[0])) {
				AJ_Printf("Unknown command: %s\n", cmd);
				continue;
			}
		}

		// From main loop: read events and responses
		process_message(apctx, timeout);
	}

	// exit
	allplay_free(&apctx);

	if (serviceName) {
		AJ_Free(serviceName);
		serviceName = NULL;
	}

	return 0;
}
