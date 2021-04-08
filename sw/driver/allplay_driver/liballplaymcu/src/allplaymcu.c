/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/
#include "allplaymcu.h"

#include <stdlib.h>
#include <stdio.h>

#include <aj_link_timeout.h>

#include "allplay_common.h"
#include "allplay_interfaces.h"
#include "allplay_player.h"
#include "allplay_util.h"
#include "allplay_system.h"

#ifndef NDEBUG
uint8_t dbgALLPLAY_LIB = 0;
#endif

static enum Allplay_State getAllplayState(allplay_ctx_t *apctx) {
	return (apctx->state & ALLPLAY_STATE_MASK);
}
static void setAllplayState(allplay_ctx_t *apctx, enum Allplay_State state) {
	apctx->state = (apctx->state & ~ALLPLAY_STATE_MASK) | state;
}
static void resetAllplayState(allplay_ctx_t *apctx) {
	uint32_t notify;
	if (getAllplayState(apctx) == ALLPLAY_STATE_IDLE) {
		notify = ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS;
	}
	else {
		notify = (apctx->state & ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS);
	}

	AJ_Disconnect(&(apctx->bus));

	apctx->state = ALLPLAY_STATE_STOPPED | notify;
	apctx->playerSessionId = 0;
	apctx->systemSessionId = 0;

	resetApReplyContext();
}

static void resetSession(allplay_ctx_t *apctx, uint32_t sessionId) {
	if (sessionId == 0) {
		return;
	}

	if (apctx->playerSessionId == sessionId) {
		apctx->playerSessionId = 0;
	}
	else if (apctx->systemSessionId == sessionId) {
		apctx->systemSessionId = 0;
	}
	else {
		AJ_InfoPrintf(("[resetSession] Unknown session: %x\n", sessionId));
		return;
	}

	if (getAllplayState(apctx) == ALLPLAY_STATE_IDLE) {
		apctx->state |= ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS;
	}
	setAllplayState(apctx, ALLPLAY_STATE_WAITING_NAME);
}

void processError(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	if (ajMsg->hdr->msgType != AJ_MSG_ERROR) {
		return;
	}
	if (strcmp(ajMsg->error, AJ_ErrTimeout) == 0) {
		if (AJ_BusLinkStateProc(&(apctx->bus)) == AJ_ERR_LINK_TIMEOUT) {
			// Bus dead => reconnect
			AJ_WarnPrintf(("[processError] Bus timeout"));
			resetAllplayState(apctx);
		}
		else {
			AJ_WarnPrintf(("[processError] Reply timeout\n"));
		}
	}
	else {
		if (strcmp(ajMsg->signature, "s") == 0) {
			const char *errorMsg;
			AJ_Status status = AJ_UnmarshalArgs(ajMsg, "s", &errorMsg);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processError] Failed to unmarshal \"s\" error reply: " AJ_STATUS_DEBUG(status)));
			}
			else {
				AJ_ErrPrintf(("[processError] Error reply: %s - %s\n", ajMsg->error, errorMsg));
			}
		}
		else if (strcmp(ajMsg->signature, "sq") == 0) {
			const char *errorMsg = NULL;
			uint16_t errorCode;
			AJ_Status status = AJ_UnmarshalArgs(ajMsg, "sq", &errorMsg, &errorCode);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processError] Failed to unmarshal \"sq\" error reply: " AJ_STATUS_DEBUG(status)));
			}
			else {
                                /* Tymphany modified. */
				AJ_ErrPrintf(("[processError] Error reply: %s(%d) - %s\n", ajMsg->error, errorCode, (errorMsg ? errorMsg : "<null>")));

			}
		}
		else {
			AJ_ErrPrintf(("[processError] Error reply: %s\n", ajMsg->error));
		}
	}
}

static allplay_message_t* processNoDataMsg(enum allplay_message_type msgType) {
	allplay_message_t *result;
	result = AJ_Malloc(sizeof(allplay_message_t));
	result->messageType = msgType;
	return result;
}

static allplay_message_t* processGetAllPropReply(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	struct ApReplyContext *replyCtx;

	replyCtx = getApReplyContext(ajMsg->replySerial, FALSE);
	if (!replyCtx) {
		AJ_ErrPrintf(("[processGetAllPropReply] Failed to get reply context\n"));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	switch (replyCtx->propertyId) {
		case ORG_ALLJOYN_CONTROL_VOLUME_ALL_PROP:
			return processVolumeInfo(apctx, ajMsg);
		case NET_ALLPLAY_MCUSYSTEM_BTCTRL_ALL_PROP:
			return processBtInfo(apctx, ajMsg);
		default:
			AJ_WarnPrintf(("[processGetAllPropReply] Unknown GetProperty response\n"));
			return makeApError(ALLPLAY_ERROR_FAILED);
	}

	// Unreachable
}

static allplay_message_t* processGetPropReply(AJ_Message *ajMsg) {
	struct ApReplyContext *replyCtx;

	replyCtx = getApReplyContext(ajMsg->replySerial, FALSE);
	if (!replyCtx) {
		AJ_ErrPrintf(("[processGetPropReply] Failed to get reply context\n"));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	switch (replyCtx->propertyId) {
		case NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO2:
			return processNetworkInfo2Reply(ajMsg);
		case NET_ALLPLAY_MCUSYSTEM_RSSI:
			return processRSSIReply(ajMsg);
		case NET_ALLPLAY_MCUSYSTEM_SYSTEMMODE:
			return processSystemModeReply(ajMsg);
		case NET_ALLPLAY_MCUSYSTEM_FIRMWARE_VERSION:
			return processFirmwareVersionReply(ajMsg);
		case NET_ALLPLAY_MCUSYSTEM_RESAMPLINGMODE:
			return processResamplingModeReply(ajMsg);
		case NET_ALLPLAY_MEDIAPLAYER_PLAYSTATE:
			return processPlayStateReply(ajMsg, ALLPLAY_RESPONSE_PLAYER_STATE);
		case NET_ALLPLAY_MEDIAPLAYER_LOOPMODE:
			return processLoopModeReply(ajMsg);
		case NET_ALLPLAY_MEDIAPLAYER_SHUFFLEMODE:
			return processShuffleModeReply(ajMsg);
		default:
			AJ_WarnPrintf(("[processGetPropReply] Unknown GetProperty response: %x\n", replyCtx->propertyId));
			return makeApError(ALLPLAY_ERROR_FAILED);
	}

	// Unreachable
}

static allplay_message_t* processSetPropReply(AJ_Message *ajMsg) {
	struct ApReplyContext *replyCtx;

	replyCtx = getApReplyContext(ajMsg->replySerial, FALSE);
	if (!replyCtx) {
		AJ_ErrPrintf(("[processSetPropReply] Failed to get reply context\n"));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	switch (replyCtx->propertyId) {
		case ORG_ALLJOYN_CONTROL_VOLUME_VOLUME:
			return processNoDataMsg(ALLPLAY_RESPONSE_VOLUME);
		case ORG_ALLJOYN_CONTROL_VOLUME_MUTE:
			return processNoDataMsg(ALLPLAY_RESPONSE_MUTE);
		case NET_ALLPLAY_MCUSYSTEM_RESAMPLINGMODE:
			return processNoDataMsg(ALLPLAY_RESPONSE_SET_RESAMPLING_MODE);
		case NET_ALLPLAY_MCUSYSTEM_BTCTRL_ENABLED:
			return processNoDataMsg(ALLPLAY_RESPONSE_BLUETOOTH_ENABLE);
		case NET_ALLPLAY_MCUSYSTEM_BTCTRL_PAIRABLE:
			return processNoDataMsg(ALLPLAY_RESPONSE_BLUETOOTH_ENABLE_PAIRING);
		default:
			AJ_WarnPrintf(("[processSetPropReply] Unknown SetProperty response: %x\n", replyCtx->propertyId));
			return makeApError(ALLPLAY_ERROR_FAILED);
	}

	// Unreachable
}

static void processFindNameReply(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	uint32_t failed;

	if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
		processError(apctx, ajMsg);
		AJ_ErrPrintf(("[processFindNameReply] Reply is error\n"));
		failed = TRUE;
	}
	else {
		uint32_t disposition;
		AJ_UnmarshalArgs(ajMsg, "u", &disposition);
		if ((disposition != AJ_FIND_NAME_STARTED) && (disposition != AJ_FIND_NAME_ALREADY)) {
			AJ_ErrPrintf(("[processFindNameReply] Invalid disposition: %u\n", disposition));
			failed = TRUE;
		}
		else {
			failed = FALSE;
		}
	}

	if (failed) {
		resetAllplayState(apctx);
	}
	else if (getAllplayState(apctx) == ALLPLAY_STATE_FIND_PLAYER_PENDING) {
		setAllplayState(apctx, ALLPLAY_STATE_FIND_PLAYER_DONE);
	}
	else if (getAllplayState(apctx) == ALLPLAY_STATE_FIND_SYSTEM_PENDING) {
		setAllplayState(apctx, ALLPLAY_STATE_WAITING_NAME);
	}
}

static void processFoundName(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	const char *name;
	int nameLen;

	AJ_UnmarshalArgs(ajMsg, "s", &name);

	nameLen = strlen(name);
	if (strcmp(name + nameLen - strlen(".quiet"), ".quiet") == 0) {
		// ignore quiet name
		return;
	}

	AJ_InfoPrintf(("[processFoundName] Found name %s\n", name));

	if (strncmp(name, NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME, strlen(NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME)) == 0) {
		int len = nameLen - strlen(NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME);
		if (apctx->playerId != NULL) {
			AJ_Free(apctx->playerId);
		}
		apctx->playerId = AJ_Malloc(len + 1);
		strcpy(apctx->playerId, name + strlen(NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME));

		apctx->state |= ALLPLAY_STATE_PLAYER_FOUND;
	}
	else if (strncmp(name, NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME, strlen(NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME)) == 0) {
		int len = nameLen - strlen(NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME);
		if (apctx->systemId != NULL) {
			AJ_Free(apctx->systemId);
		}
		apctx->systemId = AJ_Malloc(len + 1);
		strcpy(apctx->systemId, name + strlen(NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME));

		apctx->state |= ALLPLAY_STATE_SYSTEM_FOUND;
	}
}

static void processLostName(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	const char *name;
	int nameLen;

	AJ_UnmarshalArgs(ajMsg, "s", &name);

	nameLen = strlen(name);
	if (strcmp(name + nameLen - strlen(".quiet"), ".quiet") == 0) {
		// ignore quiet name
		return;
	}

	AJ_InfoPrintf(("[processLostName] Lost name %s\n", name));

	if (strncmp(name, NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME, strlen(NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME)) == 0) {
		if (apctx->playerId != NULL) {
			AJ_Free(apctx->playerId);
			apctx->playerId = NULL;
		}
		apctx->state &= ~ALLPLAY_STATE_PLAYER_FOUND;
		resetSession(apctx, apctx->playerSessionId);
	}
	else if (strncmp(name, NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME, strlen(NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME)) == 0) {
		if (apctx->systemId != NULL) {
			AJ_Free(apctx->systemId);
			apctx->systemId = NULL;
		}
		apctx->state &= ~ALLPLAY_STATE_SYSTEM_FOUND;
		resetSession(apctx, apctx->systemSessionId);
	}
}

static void processJoinSessionReply(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	uint32_t replyCode;
	uint32_t sessionId;

	if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
		processError(apctx, ajMsg);
		AJ_ErrPrintf(("[processJoinSessionReply] Reply is error\n"));
		resetAllplayState(apctx);
		return;
	}

	AJ_UnmarshalArgs(ajMsg, "uu", &replyCode, &sessionId);
	if (replyCode != AJ_JOINSESSION_REPLY_SUCCESS) {
		AJ_ErrPrintf(("[processJoinSessionReply] Join refused: %d\n", replyCode));
		resetAllplayState(apctx);
		return;
	}

	if (getAllplayState(apctx) == ALLPLAY_STATE_JOINING_PLAYER) {
		apctx->playerSessionId = sessionId;
		AJ_InfoPrintf(("[processJoinSessionReply] Player joined session %x\n", sessionId));
	}
	else if (getAllplayState(apctx) == ALLPLAY_STATE_JOINING_SYSTEM) {
		apctx->systemSessionId = sessionId;
		AJ_InfoPrintf(("[processJoinSessionReply] System joined session %x\n", sessionId));
	}
	else {
		AJ_WarnPrintf(("[processJoinSessionReply] Unexpected state: %d\n", apctx->state));
		return;
	}

	if ((apctx->playerSessionId != 0) && (apctx->systemSessionId != 0)) {
		// All apps joined
		setAllplayState(apctx, ALLPLAY_STATE_JOINED);
	}
	else {
		// Still need to join the other app
		setAllplayState(apctx, ALLPLAY_STATE_WAITING_NAME);
	}
}

static void processSessionLost(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	uint32_t sessionId;
	AJ_UnmarshalArgs(ajMsg, "u", &sessionId);
	AJ_InfoPrintf(("[processSessionLost] Session lost: %x\n", sessionId));

	resetSession(apctx, sessionId);
}

static void processMcuVersionReply(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	if (ajMsg->hdr->msgType == AJ_MSG_ERROR) {
		processError(apctx, ajMsg);
		AJ_ErrPrintf(("[processMcuVersionReply] Reply is error\n"));
		resetAllplayState(apctx);
		return;
	}

	setAllplayState(apctx, ALLPLAY_STATE_SETUP_CUSTOM_SERVICE);
	apctx->state |= ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS;
}

void checkAllPlayState(allplay_ctx_t *apctx) {
	AJ_Status status;
	allplay_status apStatus;

	switch (getAllplayState(apctx)) {
		case ALLPLAY_STATE_STOPPED:
			AJ_InfoPrintf(("*** Connecting bus\n"));
			// Not connected -> create the bus
			status = AJ_FindBusAndConnect(&(apctx->bus),
				NULL, CONNECT_TIMEOUT);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[checkAllPlayState] Failed to connect to daemon: " AJ_STATUS_DEBUG(status)));
				setAllplayState(apctx, ALLPLAY_STATE_STOPPED);
				break;
			}
			setAllplayState(apctx, ALLPLAY_STATE_CONNECTED);

			status = AJ_SetBusLinkTimeout(&(apctx->bus), BUS_TIMEOUT);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[checkAllPlayState] Failed to set bus timeout: " AJ_STATUS_DEBUG(status)));
			}
			// no break: we can process the next state immediately

		case ALLPLAY_STATE_CONNECTED:
			AJ_InfoPrintf(("*** Finding player\n"));
			status = AJ_BusFindAdvertisedNameByTransport(&(apctx->bus), NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME, AJ_TRANSPORT_LOCAL, AJ_BUS_START_FINDING);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[checkAllPlayState] Failed to start finding player: " AJ_STATUS_DEBUG(status)));
				resetAllplayState(apctx);
				break;
			}
			setAllplayState(apctx, ALLPLAY_STATE_FIND_PLAYER_PENDING);
			break;

		case ALLPLAY_STATE_FIND_PLAYER_PENDING:
			// Nothing to do but wait
			AJ_InfoPrintf(("*** Waiting find player\n"));
			break;

		case ALLPLAY_STATE_FIND_PLAYER_DONE:
			AJ_InfoPrintf(("*** Finding system\n"));
			status = AJ_BusFindAdvertisedNameByTransport(&(apctx->bus), NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME, AJ_TRANSPORT_LOCAL, AJ_BUS_START_FINDING);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[checkAllPlayState] Failed to start finding system: " AJ_STATUS_DEBUG(status)));
				resetAllplayState(apctx);
				break;
			}
			setAllplayState(apctx, ALLPLAY_STATE_FIND_SYSTEM_PENDING);
			break;

		case ALLPLAY_STATE_FIND_SYSTEM_PENDING:
			// Nothing to do but wait
			AJ_InfoPrintf(("*** Waiting find system\n"));
			break;

		case ALLPLAY_STATE_WAITING_NAME:
			if (((apctx->state & ALLPLAY_STATE_PLAYER_FOUND) == ALLPLAY_STATE_PLAYER_FOUND) && (apctx->playerSessionId == 0)) {
				// We have the name but no session => join
				char playerName[100];
				buildPlayerNameId(apctx, playerName, sizeof(playerName));
				AJ_InfoPrintf(("*** Joining player %s\n", playerName));

				status = AJ_BusJoinSession(&(apctx->bus), playerName, NET_ALLPLAY_MEDIAPLAYER_SERVICE_PORT, NULL);
				if (status != AJ_OK) {
					AJ_ErrPrintf(("[checkAllPlayState] Failed to start joining player: " AJ_STATUS_DEBUG(status)));
					resetAllplayState(apctx);
					break;
				}
				setAllplayState(apctx, ALLPLAY_STATE_JOINING_PLAYER);
			}
			else if (((apctx->state & ALLPLAY_STATE_SYSTEM_FOUND) == ALLPLAY_STATE_SYSTEM_FOUND) && (apctx->systemSessionId == 0)) {
				// We have the name but no session => join
				char systemName[100];
				buildSystemNameId(apctx, systemName, sizeof(systemName));
				AJ_InfoPrintf(("*** Joining system %s\n", systemName));
				status = AJ_BusJoinSession(&(apctx->bus), systemName, NET_ALLPLAY_MCUSYSTEM_SERVICE_PORT, NULL);
				if (status != AJ_OK) {
					AJ_ErrPrintf(("[checkAllPlayState] Failed to start joining system: " AJ_STATUS_DEBUG(status)));
					resetAllplayState(apctx);
					break;
				}
				setAllplayState(apctx, ALLPLAY_STATE_JOINING_SYSTEM);
			}
			else {
				// Waiting for the names
				AJ_InfoPrintf(("*** Waiting name\n"));
			}
			break;

		case ALLPLAY_STATE_JOINING_PLAYER:
			// Nothing to do but wait
			AJ_InfoPrintf(("*** Waiting join player\n"));
			break;

		case ALLPLAY_STATE_JOINING_SYSTEM:
			// Nothing to do but wait
			AJ_InfoPrintf(("*** Waiting join system\n"));
			break;

		case ALLPLAY_STATE_JOINED:
#if defined(DISABLE_MCU_VERSION)
			setAllplayState(apctx, ALLPLAY_STATE_SETUP_CUSTOM_SERVICE);
			apctx->state |= ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS;
#else
			AJ_InfoPrintf(("*** Setting MCU version\n"));

			apStatus = sendMcuVersion(apctx);
			if (apStatus != ALLPLAY_ERROR_NONE) {
				AJ_ErrPrintf(("[checkAllPlayState] Failed to send MCU version\n"));
				resetAllplayState(apctx);
				break;
			}
			setAllplayState(apctx, ALLPLAY_STATE_SETTING_MCU_VERSION);
#endif
			break;

		case ALLPLAY_STATE_SETTING_MCU_VERSION:
			// Nothing to do but wait
			AJ_InfoPrintf(("*** Waiting on MCU version\n"));
			break;

		case ALLPLAY_STATE_SETUP_CUSTOM_SERVICE:
			if(apctx->customService.setupFunc != NULL) {
				char * deviceId = apctx->systemId; //playerId and systemId have the same deviceId values encoded in the ID, just use this string
				AJ_InfoPrintf(("*** Setting up custom service\n"));
				apctx->customService.setupFunc(&apctx->bus, deviceId);
			}
			setAllplayState(apctx, ALLPLAY_STATE_IDLE);
			break;

		case ALLPLAY_STATE_IDLE:
			// Waiting
			break;

		case ALLPLAY_STATE_MASK:
		case ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS:
		case ALLPLAY_STATE_PLAYER_FOUND:
		case ALLPLAY_STATE_SYSTEM_FOUND:
			// Those are just to silence the compiler warnings, there are not
			// valid states
			break;
	}

}

allplay_message_t *processUserMessage(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	allplay_message_t *apMsg = NULL;

	if (ajMsg->hdr != NULL && ajMsg->hdr->msgType == AJ_MSG_ERROR) {
		processError(apctx, ajMsg);
		apMsg = makeApError(ALLPLAY_ERROR_FAILED);
	}
	else {
		switch (ajMsg->msgId) {
			case NET_ALLPLAY_MCUSYSTEM_WPSRESULT:
				apMsg = processWpsResult(ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_SYSTEMMODECHANGED:
				apMsg = processSystemModeChanged(ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_CMDRESULT:
				apMsg = processCmdResult(ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO2_CHANGED:
				apMsg = processNetworkInfo2Changed(ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_BTCTRL_STATECHANGED:
				apMsg = processBtStateChanged(apctx, ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_BTCTRL_ENABLEDCHANGED:
				apMsg = processBtEnabledChanged(apctx, ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_BTCTRL_PAIRABLECHANGED:
				apMsg = processBtPairableChanged(apctx, ajMsg);
				break;
			case NET_ALLPLAY_MEDIAPLAYER_PLAYSTATECHANGED:
				apMsg = processPlayStateChanged(ajMsg);
				break;
			case NET_ALLPLAY_MEDIAPLAYER_LOOPMODECHANGED:
				apMsg = processLoopModeChanged(ajMsg);
				break;
			case NET_ALLPLAY_MEDIAPLAYER_SHUFFLEMODECHANGED:
				apMsg = processShuffleModeChanged(ajMsg);
				break;
			case ORG_ALLJOYN_CONTROL_VOLUME_VOLUME_CHANGED:
				apMsg = processPlayerVolumeChanged(apctx, ajMsg);
				break;
			case ORG_ALLJOYN_CONTROL_VOLUME_MUTE_CHANGED:
				apMsg = processPlayerMuteChanged(apctx, ajMsg);
				break;
			case NET_ALLPLAY_FIRMWARE_STARTMCUUPDATE:
				apMsg = processNoDataMsg(ALLPLAY_EVENT_START_MCU_UPDATE);
				break;
			case NET_ALLPLAY_FIRMWARE_UPDATE_AVAILABLE:
				apMsg = processUpdateAvailable(ajMsg);
				break;
			case NET_ALLPLAY_FIRMWARE_UPDATE_STATUS:
				apMsg = processUpdateStatus(ajMsg);
				break;
			case NET_ALLPLAY_MCUSYSTEM_REBOOT_STARTED:
				apMsg = processNoDataMsg(ALLPLAY_EVENT_REBOOT_STARTED);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_GETPLAYERINFO):
				apMsg = processGetPlayerInfo(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_REQUESTWPS):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_NETWORK_REQUEST_WPS);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_STARTSETUP):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SETUP);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_WIFIENABLE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_WIFIENABLE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_DIRECTENABLE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_DIRECTENABLE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_RESET_TO_FACTORY):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_RESET_TO_FACTORY);
				break;
			case AJ_REPLY_ID(ORG_ALLJOYN_CONTROL_VOLUME_MUTE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_MUTE);
				break;
			case AJ_REPLY_ID(ORG_ALLJOYN_CONTROL_VOLUME_ADJUST_VOLUME):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_VOLUME_ADJUST);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_RESUME):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_PLAY);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_PAUSE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_PAUSE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_STOP):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_STOP);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_NEXT):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_NEXT);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_PREVIOUS):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_PREVIOUS);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_SETPOSITION):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SET_POSITION);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_SETEXTERNALSOURCE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SET_EXTERNAL_SOURCE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_PLAYITEM):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_PLAYITEM);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_GETCURRENTITEMURL):
				apMsg = processGetCurrentItemUrl(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_GET_PROP):
				apMsg = processGetPropReply(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_GET_PROP):
				apMsg = processGetPropReply(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_SET_PROP):
				apMsg = processSetPropReply(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_SET_PROP):
				apMsg = processSetPropReply(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_GET_PROP_ALL):
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_GET_PROP_ALL):
				apMsg = processGetAllPropReply(apctx, ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_RUNCMD):
				apMsg = processRunCmdReply(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_FIRMWARE_CHECK):
				apMsg = processFirmwareCheck(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_FIRMWARE_UPDATE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_FIRMWARE_UPDATE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_FIRMWARE_UPDATE_FROM_URL):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_FIRMWARE_UPDATE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_SETMCUIDLE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SET_MCU_IDLE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_SETBATTERYSTATE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SET_BATTERY_STATE);
				break;
			case AJ_REPLY_ID(ORG_ALLJOYN_ABOUT_GET_ABOUT_DATA):
				apMsg = processGetAboutData(ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MCUSYSTEM_SHUTDOWN):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_SHUTDOWN_SAM);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_ADVANCE_LOOP_MODE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_ADVANCE_LOOP_MODE);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_MEDIAPLAYER_TOGGLE_SHUFFLE_MODE):
				apMsg = processNoDataMsg(ALLPLAY_RESPONSE_TOGGLE_SHUFFLE_MODE);
				break;
			default:
				AJ_BusHandleBusMessage(ajMsg);
				break;
		}
	}

	if (!apMsg) {
		// Should only happen for unexpected messages so we don't have a
		// ApReplyContext
	}
	else {
		struct ApReplyContext *replyCtx = getApReplyContext(ajMsg->replySerial, FALSE);
		if (!replyCtx) {
			apMsg->userData = NULL;
		}
		else {
			apMsg->userData = replyCtx->userData;
			replyCtx->serial = 0;
		}
	}

	return apMsg;
}

allplay_ctx_t* allplay_new(const char* mcuFirmwareVersion, allplay_reboot_method rebootMethod) {
	return allplay_new_with_mcu_service(mcuFirmwareVersion, rebootMethod, NULL, NULL, NULL);
}

allplay_ctx_t* allplay_new_with_mcu_service(const char* mcuFirmwareVersion, allplay_reboot_method rebootMethod, const AJ_Object* AppObjects, HandleMsgFunc msgFunc, SetupService setupFunc) {
	allplay_ctx_t *ctx = AJ_Malloc(sizeof(allplay_ctx_t));
	memset(ctx, 0, sizeof(allplay_ctx_t));

	ctx->bluetooth.initialized = FALSE;

	ctx->mcuFirmwareVersion = mcuFirmwareVersion;
	ctx->rebootMethod = rebootMethod;

	ctx->customService.msgFunc = msgFunc;
	ctx->customService.setupFunc = setupFunc;

	AJ_Initialize();
	AJ_RegisterObjects(AppObjects, netAllPlayObjects);

	checkAllPlayState(ctx);

	return ctx;
}

void allplay_free(allplay_ctx_t** ctx) {
	if ((ctx == NULL) || ((*ctx) == NULL)) {
		return;
	}

	AJ_Disconnect(&((*ctx)->bus));

	if ((*ctx)->playerId != NULL) {
		AJ_Free((*ctx)->playerId);
	}
	if ((*ctx)->systemId != NULL) {
		AJ_Free((*ctx)->systemId);
	}
	AJ_Free(*ctx);
	*ctx = NULL;
}

bool_t allplay_is_connected(allplay_ctx_t* apctx) {
	return ((apctx->playerSessionId != 0) && (apctx->systemSessionId != 0));
}

void allplay_free_message(allplay_ctx_t* UNUSED(apctx), allplay_message_t** papmsg) {
	if ((papmsg == NULL) || ((*papmsg) == NULL)) {
		return;
	}
	AJ_Free(*papmsg);
	papmsg = NULL;
}

allplay_message_t* allplay_read_message(allplay_ctx_t* apctx, uint32_t timeout) {
	AJ_Status status = AJ_OK;
	int timerInitialized = 0;
	AJ_Time timer;
	AJ_Message ajMsg;
	allplay_message_t *apMsg = NULL;

	while ((status == AJ_OK) && (apMsg == NULL)) {
		uint32_t processed;
		uint32_t elapsed;

		if (timerInitialized == 0) {
			timerInitialized = 1;
			AJ_InitTimer(&timer);
			elapsed = 0;
		}
		else {
			elapsed = AJ_GetElapsedTime(&timer, TRUE);
			if (elapsed >= timeout) {
				status = AJ_ERR_TIMEOUT;
				break;
			}
		}

		checkAllPlayState(apctx);
		if ((apctx->state & ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS) == ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS) {
			apctx->state &= ~ALLPLAY_STATE_NOTIFY_CONNECTION_STATUS;
			return processNoDataMsg(ALLPLAY_EVENT_CONNECTION_STATUS_CHANGED);
		}
		if (getAllplayState(apctx) == ALLPLAY_STATE_STOPPED) {
			// We are not connected => can't unmarshal a message
			continue;
		}

		status = AJ_UnmarshalMsg(&(apctx->bus), &ajMsg, timeout - elapsed);
		if ((status == AJ_ERR_READ) || (status == AJ_ERR_LINK_DEAD)) {
			resetAllplayState(apctx);
			continue;
		}
		else if (status == AJ_ERR_TIMEOUT) {
			// Timeout, check if it's because the bus is dead or just because
			// there is no message
			if (AJ_BusLinkStateProc(&(apctx->bus)) == AJ_ERR_LINK_TIMEOUT) {
				// Bus dead => reconnect
				AJ_WarnPrintf(("[allplay_read_message] Bus timeout: " AJ_STATUS_DEBUG(status)));
				resetAllplayState(apctx);
				continue;
			}
			else {
				// No message
				break;
			}
		}
		else if (status != AJ_OK) {
			break;
		}

		processed = TRUE;
		switch (ajMsg.msgId) {
			case AJ_REPLY_ID(AJ_METHOD_FIND_NAME_BY_TRANSPORT):
				processFindNameReply(apctx, &ajMsg);
				break;
			case AJ_REPLY_ID(AJ_METHOD_JOIN_SESSION):
				processJoinSessionReply(apctx, &ajMsg);
				break;
			case AJ_REPLY_ID(NET_ALLPLAY_FIRMWARE_SETMCUVERSION):
				processMcuVersionReply(apctx, &ajMsg);
				break;
			case AJ_SIGNAL_FOUND_ADV_NAME:
				processFoundName(apctx, &ajMsg);
				break;
			case AJ_SIGNAL_LOST_ADV_NAME:
				processLostName(apctx, &ajMsg);
				break;
			case AJ_SIGNAL_SESSION_LOST:
				processSessionLost(apctx, &ajMsg);
				break;
			default:
				// Let the custom service have a chance at processing the message
				if(apctx->customService.msgFunc != NULL) {
					processed = apctx->customService.msgFunc(&ajMsg);
				} else {
					processed = FALSE;
				}
				break;
		}

		if (!processed) {
			apMsg = processUserMessage(apctx, &ajMsg);
		}

		// Reset probe timer if the message was not created by ourselves
		if ((ajMsg.sender != NULL) && (ajMsg.sender != AJ_GetUniqueName(&(apctx->bus)))) {
			AJ_NotifyLinkActive();
		}

		AJ_CloseMsg(&ajMsg);
	}

	if (status == AJ_ERR_TIMEOUT) {
		return NULL;
	}
	else if (apMsg != NULL) {
		return apMsg;
	}
	else {
		AJ_ErrPrintf(("[allplay_read_message] Error: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}
}
