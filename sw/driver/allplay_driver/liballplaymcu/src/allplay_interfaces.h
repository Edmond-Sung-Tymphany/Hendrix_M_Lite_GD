/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/
#ifndef ALLPLAY_INTERFACES_H_
#define ALLPLAY_INTERFACES_H_

#ifdef __cplusplus
extern "C" {
#endif

#define NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME "net.allplay.mcu_system"
#define NET_ALLPLAY_MCUSYSTEM_SERVICE_PATH "/net/allplay/mcu_system"
#define NET_ALLPLAY_MCUSYSTEM_FIRMWARE_PATH "/net/allplay/Firmware"
#define ORG_ALLJOYN_ABOUT_PATH "/About"
#define ORG_ALLJOYN_ABOUT_NAME "org.alljoyn.About"
#define NET_ALLPLAY_MCUSYSTEM_SERVICE_PORT 25

#define NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME "net.allplay.MediaPlayer"
#define NET_ALLPLAY_MEDIAPLAYER_SERVICE_PATH "/net/allplay/MediaPlayer"
#define NET_ALLPLAY_MEDIAPLAYER_SERVICE_PORT 3

static const char* interface_netAllPlayMcuSystem[] = {
	"net.allplay.mcu_system",   /* The first entry is the interface name. */

	"?RequestWps wpsType<i pin<i", /* wpsType: 0-pbc, 1-pin, 2-cancel */
	"!WpsResult result>i", /* result: 0-success, 1-failure, 2-cancelled. */

	"?StartSetup",
	"@NetworkInfo>(iayayayaysy)",
	/*
		i: type (0: no network, 1: wifi, 2: ethernet, 3: wifi+ethernet)
		ay: wifi ip address (4-byte array)
		ay: wifi mac address (6-byte array)
		ay: eth ip address (4-byte array)
		ay: eth mac address (6-byte array)
		s: ssid (32 characters max)
		y: rssi
	*/

	"@SystemMode>i", /* 0-normal, 1-update, 2-setup */
	"!SystemModeChanged mode>i", /* mode: same as SystemMode property + 3-playback */
	"@ResamplingMode=i", /* Switch resampling mode*/

	"?RunScript script<s stdoutMaxLen<u stderrMaxLen<u id>u",
	"!ScriptResult id>u exitstatus>i termsig>i stdout>s stderr>s",
	"@FirmwareVersion>s",
	"?WiFiEnable enable<b", /* enabled if true, disabled if false */
	"?DirectModeEnable enable<b", /* enabled if true, disabled if false */
	"?ResetToFactory action<u", /* 0: reboot, 1: halt */
	"@RSSI>ai",
	"?ConnectWiFi ssid<s password<s",
	"!NetworkInfoChanged networkInfo>(iayayayaysy)",

	"?SetMcuIdle idle<b",
	"?SetBatteryState onBattery<b batteryCharge<y batteryAutonomy<i timeToFullCharge<i",

	"@NetworkInfo2>a(iyayaysyi)",
	/*
			i: type (1: wifi, 2: ethernet) \
			y: state (0: down; 1: up) \
			ay: ip address (4-byte array) \
			ay: mac address (6-byte array) \
			s: ssid (32 characters max) for wifi, empty string for ethernet \
			y: rssi for wifi, 0 for ethernet \
			i: frequency for wifi, 0 for ethernet \
	*/
	"!NetworkInfoChanged2 networkInfo>a(iyayaysyi)",

	"?Shutdown restart<b",
	"!RebootStarted",

	NULL
};

static const char* interface_netAllPlayMcuSystemBluetoothControl[] = {
	"net.allplay.mcu_system.Bluetooth.Control",

	"@Enabled=b",
	"!EnabledChanged enabled>b",

	"@Pairable=b",
	"!PairableChanged pairable>b",

	"@State>(uu)",
	"!StateChanged state>(uu)",

	NULL
};

static const char* interface_netAllPlayFirmware[] = {
	"net.allplay.Firmware",

	"?Check avail>b version>s url>s",
	"?Update",
	"?UpdateFromURL url<s",
	"!UpdateAvailable version>s url>s",
	"!UpdateStatus status>i",

	"?SetMcuVersion version<s reboot<i",
	"!StartMcuUpdate",

	NULL
};

static const AJ_InterfaceDescription interfaces_netAllPlayMcuSystem[] = {
	AJ_PropertiesIface,
	interface_netAllPlayMcuSystem,
	interface_netAllPlayMcuSystemBluetoothControl,
	NULL
};

static const AJ_InterfaceDescription interfaces_netAllPlayMcuSystemFirmware[] = {
	AJ_PropertiesIface,
	interface_netAllPlayFirmware,
	NULL
};


static const char* interface_netAllPlayMediaPlayer[] = {
	"net.allplay.MediaPlayer",   /* The first entry is the interface name. */

	"?Resume",
	"?Pause",
	"?Stop",
	"?Next",
	"?Previous",
	"?SetPosition position<x",

	"@PlayState>(sxuuuiia(ssssxsssa{ss}a{sv}v))",
	"!PlayStateChanged state>(sxuuuiia(ssssxsssa{ss}a{sv}v))",
	/*
		s: playstate (STOPPED, ...)
		x: position (ms)
		u: sample rate
		u: audio channels
		u: bits per sample
		i: index current item
		i: index next item
		a(ssssxsssa{ss}a{sv}v): size 0 (no item), 1 (current item) or 2 (current and next item)
			s: url
			s: title
			s: artist
			s: thumbnail url
			x: duration (ms)
			s: mediaType
			s: album
			s: genre
			a{ss}: other data (country, channel, ...)
			a{sv}: medium description (codec, container, protocol, ...)
			v: userData
	*/

	"?GetPlayerInfo >sasi(siv)",

	"@LoopMode=s",
	"!LoopModeChanged loopMode>s",
	"@ShuffleMode=s",
	"!ShuffleModeChanged shuffleMode>s",

	NULL
};

static const char* interface_orgAlljoynControlVolume[] = {
	"org.alljoyn.Control.Volume",   /* The first entry is the interface name. */

	"@Version>q",
	"@Volume=n",
	"@VolumeRange>(nnn)",
	"@Mute=b",
	"!VolumeChanged newVolume>n",
	"!MuteChanged newMute>b",
	"?AdjustVolume delta<n",

	NULL
};

static const char* interface_netAllPlayMcu[] = {
	"net.allplay.MCU",   /* The first entry is the interface name. */

	"?SetExternalSource name<s interruptible<b volumeCtrlEnabled<b",
	"?PlayItem url<s title<s artist<s thumbnailUrl<s duration<x album<s genre<s",
	"?GetCurrentItemUrl url>s",
	"?AdvanceLoopMode",
	"?ToggleShuffleMode",

	NULL
};

static const AJ_InterfaceDescription interfaces_netAllPlayMediaPlayer[] = {
	AJ_PropertiesIface,
	interface_netAllPlayMediaPlayer,
	interface_orgAlljoynControlVolume,
	interface_netAllPlayMcu,
	NULL
};

static const char* const interface_AboutIface[] = {
	ORG_ALLJOYN_ABOUT_NAME,
	"@Version>q",
	"?GetAboutData <s >a{sv}",
	"?GetObjectDescription >a(oas)",
	"!&Announce >q >q >a(oas) >a{sv}",
	NULL
};
static const AJ_InterfaceDescription interfaces_AboutIfaces[] = {
	AJ_PropertiesIface,
	interface_AboutIface,
	NULL
};

static const AJ_Object netAllPlayObjects[] = {
	{ NET_ALLPLAY_MCUSYSTEM_SERVICE_PATH, interfaces_netAllPlayMcuSystem, 0, NULL },
	{ NET_ALLPLAY_MEDIAPLAYER_SERVICE_PATH, interfaces_netAllPlayMediaPlayer, 0, NULL },
	{ NET_ALLPLAY_MCUSYSTEM_FIRMWARE_PATH, interfaces_netAllPlayMcuSystemFirmware, 0, NULL },
	{ ORG_ALLJOYN_ABOUT_PATH, interfaces_AboutIfaces, 0, NULL },
	{ NULL, NULL, 0, NULL }
};

#define NET_ALLPLAY_MCUSYSTEM_GET_PROP AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_GET)
#define NET_ALLPLAY_MCUSYSTEM_SET_PROP AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_SET)
#define NET_ALLPLAY_MCUSYSTEM_GET_PROP_ALL AJ_PRX_MESSAGE_ID(0, 0, AJ_PROP_GET_ALL)

#define NET_ALLPLAY_MCUSYSTEM_REQUESTWPS AJ_PRX_MESSAGE_ID(0, 1, 0)
#define NET_ALLPLAY_MCUSYSTEM_WPSRESULT AJ_PRX_MESSAGE_ID(0, 1, 1)
#define NET_ALLPLAY_MCUSYSTEM_STARTSETUP AJ_PRX_MESSAGE_ID(0, 1, 2)
//#define NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO AJ_PRX_PROPERTY_ID(0, 1, 3)
#define NET_ALLPLAY_MCUSYSTEM_SYSTEMMODE AJ_PRX_PROPERTY_ID(0, 1, 4)
#define NET_ALLPLAY_MCUSYSTEM_SYSTEMMODECHANGED AJ_PRX_MESSAGE_ID(0, 1, 5)
#define NET_ALLPLAY_MCUSYSTEM_RESAMPLINGMODE AJ_PRX_PROPERTY_ID(0, 1, 6)
#define NET_ALLPLAY_MCUSYSTEM_RUNCMD AJ_PRX_MESSAGE_ID(0, 1, 7)
#define NET_ALLPLAY_MCUSYSTEM_CMDRESULT AJ_PRX_MESSAGE_ID(0, 1, 8)
#define NET_ALLPLAY_MCUSYSTEM_FIRMWARE_VERSION AJ_PRX_PROPERTY_ID(0, 1, 9)
#define NET_ALLPLAY_MCUSYSTEM_WIFIENABLE AJ_PRX_MESSAGE_ID(0, 1, 10)
#define NET_ALLPLAY_MCUSYSTEM_DIRECTENABLE AJ_PRX_MESSAGE_ID(0, 1, 11)
#define NET_ALLPLAY_MCUSYSTEM_RESET_TO_FACTORY AJ_PRX_MESSAGE_ID(0, 1, 12)
#define NET_ALLPLAY_MCUSYSTEM_RSSI AJ_PRX_PROPERTY_ID(0, 1, 13)
#define NET_ALLPLAY_MCUSYSTEM_WIFI_CONNECT AJ_PRX_MESSAGE_ID(0, 1, 14)
//#define NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO_CHANGED AJ_PRX_MESSAGE_ID(0, 1, 15)
#define NET_ALLPLAY_MCUSYSTEM_SETMCUIDLE AJ_PRX_MESSAGE_ID(0, 1, 16)
#define NET_ALLPLAY_MCUSYSTEM_SETBATTERYSTATE AJ_PRX_MESSAGE_ID(0, 1, 17)
#define NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO2 AJ_PRX_PROPERTY_ID(0, 1, 18)
#define NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO2_CHANGED AJ_PRX_MESSAGE_ID(0, 1, 19)
#define NET_ALLPLAY_MCUSYSTEM_SHUTDOWN AJ_PRX_MESSAGE_ID(0, 1, 20)
#define NET_ALLPLAY_MCUSYSTEM_REBOOT_STARTED AJ_PRX_MESSAGE_ID(0, 1, 21)

#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_ENABLED AJ_PRX_PROPERTY_ID(0, 2, 0)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_ENABLEDCHANGED AJ_PRX_MESSAGE_ID(0, 2, 1)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_PAIRABLE AJ_PRX_PROPERTY_ID(0, 2, 2)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_PAIRABLECHANGED AJ_PRX_MESSAGE_ID(0, 2, 3)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_STATE AJ_PRX_PROPERTY_ID(0, 2, 4)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_STATECHANGED AJ_PRX_MESSAGE_ID(0, 2, 5)
#define NET_ALLPLAY_MCUSYSTEM_BTCTRL_ALL_PROP AJ_PRX_MESSAGE_ID(0, 2, 0xff) // hack to identify a GetPropAll

#define NET_ALLPLAY_MEDIAPLAYER_GET_PROP AJ_PRX_MESSAGE_ID(1, 0, AJ_PROP_GET)
#define NET_ALLPLAY_MEDIAPLAYER_SET_PROP AJ_PRX_MESSAGE_ID(1, 0, AJ_PROP_SET)
#define NET_ALLPLAY_MEDIAPLAYER_GET_PROP_ALL AJ_PRX_MESSAGE_ID(1, 0, AJ_PROP_GET_ALL)

#define NET_ALLPLAY_MEDIAPLAYER_RESUME AJ_PRX_MESSAGE_ID(1, 1, 0)
#define NET_ALLPLAY_MEDIAPLAYER_PAUSE AJ_PRX_MESSAGE_ID(1, 1, 1)
#define NET_ALLPLAY_MEDIAPLAYER_STOP AJ_PRX_MESSAGE_ID(1, 1, 2)
#define NET_ALLPLAY_MEDIAPLAYER_NEXT AJ_PRX_MESSAGE_ID(1, 1, 3)
#define NET_ALLPLAY_MEDIAPLAYER_PREVIOUS AJ_PRX_MESSAGE_ID(1, 1, 4)
#define NET_ALLPLAY_MEDIAPLAYER_SETPOSITION AJ_PRX_MESSAGE_ID(1, 1, 5)
#define NET_ALLPLAY_MEDIAPLAYER_PLAYSTATE AJ_PRX_PROPERTY_ID(1, 1, 6)
#define NET_ALLPLAY_MEDIAPLAYER_PLAYSTATECHANGED AJ_PRX_MESSAGE_ID(1, 1, 7)
#define NET_ALLPLAY_MEDIAPLAYER_GETPLAYERINFO AJ_PRX_MESSAGE_ID(1, 1, 8)
#define NET_ALLPLAY_MEDIAPLAYER_LOOPMODE AJ_PRX_PROPERTY_ID(1, 1, 9)
#define NET_ALLPLAY_MEDIAPLAYER_LOOPMODECHANGED AJ_PRX_MESSAGE_ID(1, 1, 10)
#define NET_ALLPLAY_MEDIAPLAYER_SHUFFLEMODE AJ_PRX_PROPERTY_ID(1, 1, 11)
#define NET_ALLPLAY_MEDIAPLAYER_SHUFFLEMODECHANGED AJ_PRX_MESSAGE_ID(1, 1, 12)

#define ORG_ALLJOYN_CONTROL_VOLUME_VERSION AJ_PRX_PROPERTY_ID(1, 2, 0)
#define ORG_ALLJOYN_CONTROL_VOLUME_VOLUME AJ_PRX_PROPERTY_ID(1, 2, 1)
#define ORG_ALLJOYN_CONTROL_VOLUME_VOLUME_RANGE AJ_PRX_PROPERTY_ID(1, 2, 2)
#define ORG_ALLJOYN_CONTROL_VOLUME_MUTE AJ_PRX_PROPERTY_ID(1, 2, 3)
#define ORG_ALLJOYN_CONTROL_VOLUME_VOLUME_CHANGED AJ_PRX_MESSAGE_ID(1, 2, 4)
#define ORG_ALLJOYN_CONTROL_VOLUME_MUTE_CHANGED AJ_PRX_MESSAGE_ID(1, 2, 5)
#define ORG_ALLJOYN_CONTROL_VOLUME_ADJUST_VOLUME AJ_PRX_MESSAGE_ID(1, 2, 6)
#define ORG_ALLJOYN_CONTROL_VOLUME_ALL_PROP AJ_PRX_MESSAGE_ID(1, 2, 0xff) // hack to identify a GetPropAll

#define NET_ALLPLAY_MEDIAPLAYER_SETEXTERNALSOURCE AJ_PRX_MESSAGE_ID(1, 3, 0)
#define NET_ALLPLAY_MEDIAPLAYER_PLAYITEM AJ_PRX_MESSAGE_ID(1, 3, 1)
#define NET_ALLPLAY_MEDIAPLAYER_GETCURRENTITEMURL AJ_PRX_MESSAGE_ID(1, 3, 2)
#define NET_ALLPLAY_MEDIAPLAYER_ADVANCE_LOOP_MODE AJ_PRX_MESSAGE_ID(1, 3, 3)
#define NET_ALLPLAY_MEDIAPLAYER_TOGGLE_SHUFFLE_MODE AJ_PRX_MESSAGE_ID(1, 3, 4)

#define NET_ALLPLAY_FIRMWARE_CHECK AJ_PRX_MESSAGE_ID(2, 1, 0)
#define NET_ALLPLAY_FIRMWARE_UPDATE AJ_PRX_MESSAGE_ID(2, 1, 1)
#define NET_ALLPLAY_FIRMWARE_UPDATE_FROM_URL AJ_PRX_MESSAGE_ID(2, 1, 2)
#define NET_ALLPLAY_FIRMWARE_UPDATE_AVAILABLE AJ_PRX_MESSAGE_ID(2, 1, 3)
#define NET_ALLPLAY_FIRMWARE_UPDATE_STATUS AJ_PRX_MESSAGE_ID(2, 1, 4)
#define NET_ALLPLAY_FIRMWARE_SETMCUVERSION AJ_PRX_MESSAGE_ID(2, 1, 5)
#define NET_ALLPLAY_FIRMWARE_STARTMCUUPDATE AJ_PRX_MESSAGE_ID(2, 1, 6)

#define ORG_ALLJOYN_ABOUT_GET_PROP AJ_PRX_MESSAGE_ID(3, 0, AJ_PROP_GET)
#define ORG_ALLJOYN_ABOUT_SET_PROP AJ_PRX_MESSAGE_ID(3, 0, AJ_PROP_SET)
#define ORG_ALLJOYN_ABOUT_GET_PROP_ALL AJ_PRX_MESSAGE_ID(3, 0, AJ_PROP_GET_ALL)

#define ORG_ALLJOYN_ABOUT_VERSION AJ_PRX_PROPERTY_ID(3, 1, 0)
#define ORG_ALLJOYN_ABOUT_GET_ABOUT_DATA AJ_PRX_MESSAGE_ID(3, 1, 1)
#define ORG_ALLJOYN_ABOUT_GET_ABOUT_DESCRIPTION AJ_PRX_MESSAGE_ID(3, 1, 2)
#define ORG_ALLJOYN_ABOUT_GET_ABOUT_ANNOUNCE AJ_PRX_MESSAGE_ID(3, 1, 3)

#ifdef __cplusplus
}
#endif

#endif /* ALLPLAY_INTERFACES_H_ */
