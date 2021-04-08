/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#include "allplay_util.h"

#include <stdlib.h>
#include "allplay_interfaces.h"

/* Tymphany modified.*/
#define NUM_REPLY_CONTEXTS 10 // See NUM_REPLY_CONTEXTS in AllJoyn-TC aj_introspect.c (increased by Tym for intense EQ polling msg)
struct ApReplyContext replyContexts[NUM_REPLY_CONTEXTS];

struct ApReplyContext *getApReplyContext(uint32_t serial, int allocate) {
	int freeContext = -1;
	int i;

	for (i = ArraySize(replyContexts) - 1; i >= 0; --i) {
		if (replyContexts[i].serial == 0) {
			freeContext = i;
		}
		else if (replyContexts[i].serial == serial) {
			return &replyContexts[i];
		}
	}

	if (!allocate) {
		// Event or unknown reply
		return NULL;
	}
	else if (freeContext == -1) {
		// If it happens, NUM_REPLY_CONTEXTS is too small
		AJ_ErrPrintf(("[getApReplyContext] Not enough ApReplyContext\n"));
		return NULL;
	}
	else {
		replyContexts[freeContext].serial = serial;
		return &replyContexts[freeContext];
	}
}

void resetApReplyContext(void) {
	size_t i;
	for (i = 0; i < ArraySize(replyContexts); ++i) {
		replyContexts[i].serial = 0;
	}
}

allplay_message_t* makeApError(allplay_status code) {
	allplay_error_t *error;
	error = AJ_Malloc(sizeof(allplay_error_t));
	error->messageType = ALLPLAY_ERROR;
	error->userData = NULL;
	error->code = code;
	return (allplay_message_t*)error;
}

allplay_message_t* makeVolumeInfo(allplay_ctx_t *apctx, enum allplay_message_type msgType) {
	allplay_volume_info_t *volumeInfo;

	if (apctx->volumeStep == 0) {
		// allplay_get_volume_info() must be called first
		return NULL;
	}

	volumeInfo = AJ_Malloc(sizeof(allplay_volume_info_t));
	volumeInfo->messageType = msgType;
	volumeInfo->userData = NULL;
	volumeInfo->mute = apctx->mute;
	volumeInfo->volume = (apctx->volume - apctx->volumeMin) / apctx->volumeStep;
	volumeInfo->max_volume = (apctx->volumeMax - apctx->volumeMin) / apctx->volumeStep;

	return (allplay_message_t*)volumeInfo;
}

allplay_message_t* makeBtState(allplay_ctx_t *apctx, enum allplay_message_type msgType) {
	allplay_bluetooth_state_t *btState;

	if (!apctx->bluetooth.initialized) {
		// allplay_bluetooth_get_state() must be called first
		return NULL;
	}

	btState = AJ_Malloc(sizeof(allplay_bluetooth_state_t));
	btState->messageType = msgType;
	btState->userData = NULL;
	btState->enabled = apctx->bluetooth.enabled;
	btState->pairable = apctx->bluetooth.pairable;
	btState->btDevicesCount = apctx->bluetooth.btCount;
	btState->a2dpDevicesCount = apctx->bluetooth.a2dpCount;

	return (allplay_message_t*)btState;
}

enum allplay_player_state_value playStateFromString(const char *state) {
	if (strcmp(state, "STOPPED") == 0) {
		return ALLPLAY_PLAYER_STATE_STOPPED;
	}
	else if (strcmp(state, "TRANSITIONING") == 0) {
		return ALLPLAY_PLAYER_STATE_TRANSITIONING;
	}
	else if (strcmp(state, "BUFFERING") == 0) {
		return ALLPLAY_PLAYER_STATE_BUFFERING;
	}
	else if (strcmp(state, "PLAYING") == 0) {
		return ALLPLAY_PLAYER_STATE_PLAYING;
	}
	else if (strcmp(state, "PAUSED") == 0) {
		return ALLPLAY_PLAYER_STATE_PAUSED;
	}
	else {
		return ALLPLAY_PLAYER_STATE_UNKNOWN;
	}
}

AJ_Status buildPlayerNameId(allplay_ctx_t *ctx, char *dst, size_t len) {
	size_t nameLen = strlen(NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME);
	if ((nameLen + strlen(ctx->playerId)) >= len) {
		// Buffer too small
		return AJ_ERR_RESOURCES;
	}
	strcpy(dst, NET_ALLPLAY_MEDIAPLAYER_SERVICE_NAME);
	strcpy(dst + nameLen, ctx->playerId);
	return AJ_OK;
}

AJ_Status buildSystemNameId(allplay_ctx_t *ctx, char *dst, size_t len) {
	size_t nameLen = strlen(NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME);
	if ((nameLen + strlen(ctx->systemId)) >= len) {
		// Buffer too small
		return AJ_ERR_RESOURCES;
	}
	strcpy(dst, NET_ALLPLAY_MCUSYSTEM_SERVICE_NAME);
	strcpy(dst + nameLen, ctx->systemId);
	return AJ_OK;
}

allplay_status sendGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t msgId, const char *service, uint32_t sessionId, uint32_t propertyId) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		msgId,
		service,
		sessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetProperty] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->propertyId = propertyId;
	replyCtx->userData = userData;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetProperty] fail to marshal property name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetProperty] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[sendGetProperty] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status sendGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t msgId, const char *service, uint32_t sessionId, uint32_t propertyId) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	const char *iface;

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		msgId,
		service,
		sessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetAllProperties] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->propertyId = propertyId;
	replyCtx->userData = userData;

	switch (propertyId) {
		case ORG_ALLJOYN_CONTROL_VOLUME_ALL_PROP:
			iface = interface_orgAlljoynControlVolume[0];
			break;
		case NET_ALLPLAY_MCUSYSTEM_BTCTRL_ALL_PROP:
			iface = interface_netAllPlayMcuSystemBluetoothControl[0];
			break;
		default:
			AJ_ErrPrintf(("[sendGetAllProperties] unknown interface for property %x\n", propertyId));
			return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "s", iface);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetAllProperties] fail to marshal interface: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendGetAllProperties] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[sendGetAllProperties] done\n"));
	return ALLPLAY_ERROR_NONE;
}
