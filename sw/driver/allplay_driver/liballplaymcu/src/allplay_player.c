/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#include "allplay_player.h"

#include <stdlib.h>
#include "allplay_interfaces.h"
#include "allplay_util.h"

#define LOOPMODE_STR_NONE "NONE"
#define LOOPMODE_STR_ONE "ONE"
#define LOOPMODE_STR_ALL "ALL"
#define SHUFFLEMODE_STR_LINEAR "LINEAR"
#define SHUFFLEMODE_STR_SHUFFLE "SHUFFLE"

allplay_message_t* processPlayStateArg(AJ_Message *ajMsg, enum allplay_message_type msgType) {
	allplay_player_state_t *result;
	AJ_Status status;
	AJ_Arg stateArg;
	const char *playstate;
	int64_t position;
	int32_t itemIndex;
	int32_t nextItemIndex;
	uint32_t sampleRate;
	uint32_t audioChannels;
	uint32_t bitsPerSample;
	AJ_Arg itemArray;
	AJ_Arg item;
	const char *url;
	const char *title;
	const char *artist;
	const char *thumbnailUrl;
	int64_t duration;
	const char *mediaType;
	const char *album;
	const char *genre;
	AJ_Arg otherDataArray;
	AJ_Arg otherDataEntry;
	const char *entryKey;
	const char *entryValue;
	const char *contentSource;
	size_t metadataSize;

	status = AJ_UnmarshalContainer(ajMsg, &stateArg, AJ_ARG_STRUCT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayStateArg] Failed to open struct: " AJ_STATUS_DEBUG(status)));
		goto error;
	}
	status = AJ_UnmarshalArgs(ajMsg, "sxuuuii", &playstate, &position,
			&sampleRate, &audioChannels, &bitsPerSample,
			&itemIndex, &nextItemIndex);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayStateArg] Failed to get state: " AJ_STATUS_DEBUG(status)));
		goto error;
	}
	status = AJ_UnmarshalContainer(ajMsg, &itemArray, AJ_ARG_ARRAY);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayStateArg] Failed to open item array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	// We only want the current/first item (if it exists) so we don't need a loop
	status = AJ_UnmarshalContainer(ajMsg, &item, AJ_ARG_STRUCT);
	if (status == AJ_ERR_NO_MORE) {
		// no current item
		duration = 0;
		title = NULL;
		artist = NULL;
		album = NULL;
		contentSource = NULL;
		metadataSize = 0;
	}
	else if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayStateArg] Failed to open item: " AJ_STATUS_DEBUG(status)));
		goto error;
	}
	else {
		status = AJ_UnmarshalArgs(ajMsg, "ssssxsss", &url, &title, &artist, &thumbnailUrl, &duration, &mediaType, &album, &genre);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processPlayStateArg] Failed to get item: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		metadataSize = strlen(title) + 1 + strlen(artist) + 1 + strlen(album) + 1;

		status = AJ_UnmarshalContainer(ajMsg, &otherDataArray, AJ_ARG_ARRAY);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processPlayStateArg] Failed to open otherData: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		contentSource = NULL;
		for(;;) {
			status = AJ_UnmarshalContainer(ajMsg, &otherDataEntry, AJ_ARG_DICT_ENTRY);
			if (status == AJ_ERR_NO_MORE) {
				break;
			}
			else if (status != AJ_OK) {
				AJ_ErrPrintf(("[processPlayStateArg] Failed to open otherData entry: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "ss", &entryKey, &entryValue);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processPlayStateArg] Failed to get otherData fields: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			if (strcmp(entryKey, "contentSource") == 0) {
				contentSource = entryValue;
				metadataSize += strlen(contentSource) + 1;
			}

			status = AJ_UnmarshalCloseContainer(ajMsg, &otherDataEntry);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processPlayStateArg] Failed to close otherData entry: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
		}

		status = AJ_UnmarshalCloseContainer(ajMsg, &otherDataArray);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processPlayStateArg] Failed to close otherData: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

	}

	// We can't close the container because we didn't read everything yet
	// but that's OK, AllJoyn can handle that.

	result = AJ_Malloc(sizeof(allplay_player_state_t) + metadataSize);
	result->messageType = msgType;
	result->state = playStateFromString(playstate);
	result->position = (int)position;
	result->duration = (int)duration;
	result->sampleRate = sampleRate;
	result->audioChannels = audioChannels;
	result->bitsPerSample = bitsPerSample;
	if ((title == NULL) || (artist == NULL) || (album == NULL)) {
		result->title = NULL;
		result->artist = NULL;
		result->album = NULL;
		result->contentSource = NULL;
	}
	else {
		result->title = (const char*)(result + 1);
		strcpy((char*)result->title, title);
		result->artist = result->title + strlen(result->title) + 1;
		strcpy((char*)result->artist, artist);
		result->album = result->artist + strlen(result->artist) + 1;
		strcpy((char*)result->album, album);

		if (contentSource == NULL) {
			result->contentSource = NULL;
		}
		else {
			result->contentSource = result->album + strlen(result->album) + 1;
			strcpy((char*)result->contentSource, contentSource);
		}
	}

	return (allplay_message_t*)result;

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}


allplay_message_t* processPlayStateChanged(AJ_Message *ajMsg) {
	return processPlayStateArg(ajMsg, ALLPLAY_EVENT_PLAYER_STATE_CHANGED);
}

allplay_message_t* processPlayerVolumeChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Arg arg;
	AJ_Status status = AJ_UnmarshalArg(ajMsg, &arg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayerVolumeChanged] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	apctx->volume = *(arg.val.v_int16);

	return makeVolumeInfo(apctx, ALLPLAY_EVENT_VOLUME_CHANGED);
}

allplay_message_t* processPlayerMuteChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Arg arg;
	AJ_Status status = AJ_UnmarshalArg(ajMsg, &arg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayerMuteChanged] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	apctx->mute = *(arg.val.v_bool) ? 1 : 0;

	return makeVolumeInfo(apctx, ALLPLAY_EVENT_VOLUME_CHANGED);
}


allplay_message_t* processPlayStateReply(AJ_Message *ajMsg, enum allplay_message_type msgType) {
	AJ_Status status;
	const char *sig;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processPlayStateReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "(sxuuuiia(ssssxsssa{ss}a{sv}v))") != 0) {
		AJ_ErrPrintf(("[processPlayStateReply] Unexpected signature: %s\n", sig));
		goto error;
	}

	return processPlayStateArg(ajMsg, msgType);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processVolumeInfo(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Status status;
	AJ_Arg allProps;
	AJ_Arg prop;
	const char *propName;
	const char *propValueSig;

	status = AJ_UnmarshalContainer(ajMsg, &allProps, AJ_ARG_ARRAY);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processVolumeInfo] Failed to open array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	for(;;) {
		status = AJ_UnmarshalContainer(ajMsg, &prop, AJ_ARG_DICT_ENTRY);
		if (status == AJ_ERR_NO_MORE) {
			break;
		}
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processVolumeInfo] Failed to open dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		status = AJ_UnmarshalArgs(ajMsg, "s", &propName);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processVolumeInfo] Failed to get propName: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (strcmp(propName, "Mute") == 0) {
			uint32_t mute;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "b", &mute);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get mute: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			apctx->mute = mute ? 1 : 0;
		}
		else if (strcmp(propName, "Volume") == 0) {
			int16_t volume;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "n", &volume);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get volume: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			apctx->volume = volume;
		}
		else if (strcmp(propName, "VolumeRange") == 0) {
			AJ_Arg range;
			int16_t min, max, step;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalContainer(ajMsg, &range, AJ_ARG_STRUCT);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to open volumeRange: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			status = AJ_UnmarshalArgs(ajMsg, "nnn", &min, &max, &step);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to get volumeRange: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalCloseContainer(ajMsg, &range);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to close volumeRange: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			apctx->volumeMin = min;
			apctx->volumeMax = max;
			apctx->volumeStep = step;
		}
		else {
			// Unknown or unwanted property
			status = AJ_SkipArg(ajMsg);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processVolumeInfo] Failed to skip: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
		}

		status = AJ_UnmarshalCloseContainer(ajMsg, &prop);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processVolumeInfo] Failed to close dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}
	}

	status = AJ_UnmarshalCloseContainer(ajMsg, &allProps);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processVolumeInfo] Failed to close array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	AJ_InfoPrintf(("[processVolumeInfo] done\n"));

	return makeVolumeInfo(apctx, ALLPLAY_RESPONSE_VOLUME_INFO);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processGetPlayerInfo( AJ_Message *ajMsg) {
	allplay_friendly_name_t *result;
	AJ_Status status;
	const char *name;

	status =  AJ_UnmarshalArgs(ajMsg, "s", &name);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processGetPlayerInfo] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_friendly_name_t) + strlen(name) + 1);
	result->messageType = ALLPLAY_RESPONSE_GET_FRIENDLY_NAME;
	result->name = ((char*)result) + sizeof(allplay_friendly_name_t);
	strcpy(result->name, name);

	return (allplay_message_t *)result;
}

allplay_message_t* processGetCurrentItemUrl( AJ_Message *ajMsg) {
	allplay_url_t *result;
	AJ_Status status;
	const char *url;

	status =  AJ_UnmarshalArgs(ajMsg, "s", &url);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processGetCurrentItemUrl] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_url_t) + strlen(url) + 1);
	result->messageType = ALLPLAY_RESPONSE_GET_CURRENT_ITEM_URL;
	result->url = (const char*)(result + 1);
	strcpy((char*)result->url, url);

	return (allplay_message_t *)result;
}

allplay_message_t* processLoopModeArg(AJ_Message *ajMsg, enum allplay_message_type msgType) {
	allplay_loop_mode_t *result;
	AJ_Status status;
	const char *modeStr;
	enum allplay_loop_mode_value mode;

	status = AJ_UnmarshalArgs(ajMsg, "s", &modeStr);
	if (status != AJ_OK) {
		AJ_Printf("[processLoopModeArg] Failed to get mode: " AJ_STATUS_DEBUG(status));
		goto error;
	}

	if (strcasecmp(modeStr, LOOPMODE_STR_NONE) == 0) {
		mode = ALLPLAY_LOOP_NONE;
	}
	else if (strcasecmp(modeStr, LOOPMODE_STR_ONE) == 0) {
		mode = ALLPLAY_LOOP_ONE;
	}
	else if (strcasecmp(modeStr, LOOPMODE_STR_ALL) == 0) {
		mode = ALLPLAY_LOOP_ALL;
	}
	else {
		AJ_Printf("[processLoopModeArg] Unknown mode: %s\n", modeStr);
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_loop_mode_t));
	memset(result, 0, sizeof(allplay_loop_mode_t));
	result->messageType = msgType;
	result->mode = mode;

	return (allplay_message_t*)result;

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processLoopModeChanged(AJ_Message *ajMsg) {
	return processLoopModeArg(ajMsg, ALLPLAY_EVENT_LOOP_MODE_CHANGED);
}

allplay_message_t* processLoopModeReply(AJ_Message *ajMsg) {
	AJ_Status status;
	const char *sig;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_Printf("[processLoopModeReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status));
		goto error;
	}

	if (strcmp(sig, "s") != 0) {
		AJ_Printf("[processLoopModeReply] Unexpected signature: %s\n", sig);
		goto error;
	}

	return processLoopModeArg(ajMsg, ALLPLAY_RESPONSE_GET_LOOP_MODE);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processShuffleModeArg(AJ_Message *ajMsg, enum allplay_message_type msgType) {
	allplay_shuffle_mode_t *result;
	AJ_Status status;
	const char *modeStr;
	enum allplay_shuffle_mode_value mode;

	status = AJ_UnmarshalArgs(ajMsg, "s", &modeStr);
	if (status != AJ_OK) {
		AJ_Printf("[processShuffleModeArg] Failed to get mode: " AJ_STATUS_DEBUG(status));
		goto error;
	}

	if (strcasecmp(modeStr, SHUFFLEMODE_STR_LINEAR) == 0) {
		mode = ALLPLAY_SHUFFLE_LINEAR;
	}
	else if (strcasecmp(modeStr, SHUFFLEMODE_STR_SHUFFLE) == 0) {
		mode = ALLPLAY_SHUFFLE_SHUFFLED;
	}
	else {
		AJ_Printf("[processShuffleModeArg] Unknown mode: %s\n", modeStr);
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_shuffle_mode_t));
	memset(result, 0, sizeof(allplay_shuffle_mode_t));
	result->messageType = msgType;
	result->mode = mode;

	return (allplay_message_t*)result;

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processShuffleModeChanged(AJ_Message *ajMsg) {
	return processShuffleModeArg(ajMsg, ALLPLAY_EVENT_SHUFFLE_MODE_CHANGED);
}

allplay_message_t* processShuffleModeReply(AJ_Message *ajMsg) {
	AJ_Status status;
	const char *sig;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_Printf("[processShuffleModeReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status));
		goto error;
	}

	if (strcmp(sig, "s") != 0) {
		AJ_Printf("[processShuffleModeReply] Unexpected signature: %s\n", sig);
		goto error;
	}

	return processShuffleModeArg(ajMsg, ALLPLAY_RESPONSE_GET_SHUFFLE_MODE);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_status sendPlayerGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t propertyId) {
	AJ_Status status;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendPlayerGetProperty] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	return sendGetProperty(apctx, userData, NET_ALLPLAY_MEDIAPLAYER_GET_PROP, objectName, apctx->playerSessionId, propertyId);
}

allplay_status sendPlayerGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t allPropId) {
	AJ_Status status;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendPlayerGetAllProperties] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	return sendGetAllProperties(apctx, userData, NET_ALLPLAY_MEDIAPLAYER_GET_PROP_ALL, objectName, apctx->playerSessionId, allPropId);
}


allplay_status allplay_get_volume_info(allplay_ctx_t *apctx, void *userData) {
	return sendPlayerGetAllProperties(apctx, userData, ORG_ALLJOYN_CONTROL_VOLUME_ALL_PROP);
}

allplay_status allplay_set_volume(allplay_ctx_t *apctx, void *userData, int32_t volume) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];
	int16_t newVolume;

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	if (apctx->volumeStep == 0) {
		AJ_ErrPrintf(("[allplay_set_volume] Unknown volume info (call allplay_get_volume_info first)\n"));
		return ALLPLAY_ERROR_FAILED;
	}
	newVolume = (int16_t)(volume * apctx->volumeStep + apctx->volumeMin);
	newVolume = max(apctx->volumeMin, min(apctx->volumeMax, newVolume));

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_volume] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_SET_PROP,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_volume] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;
	replyCtx->propertyId = ORG_ALLJOYN_CONTROL_VOLUME_VOLUME;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_volume] fail to marshal property: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "n", newVolume);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_volume] fail to marshal volume: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_volume] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	apctx->volume = newVolume;

	AJ_InfoPrintf(("[allplay_set_volume] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_volume_adjust(allplay_ctx_t *apctx, void *userData, int32_t step) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];
	int16_t newStep;

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	if (apctx->volumeStep == 0) {
		AJ_ErrPrintf(("[allplay_volume_adjust] Unknown volume info (call allplay_get_volume_info first)\n"));
		return ALLPLAY_ERROR_FAILED;
	}
	newStep = step * apctx->volumeStep;

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_volume_adjust] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		ORG_ALLJOYN_CONTROL_VOLUME_ADJUST_VOLUME,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_volume_adjust] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "n", newStep);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_volume_adjust] fail to marshal step: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_volume_adjust] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	apctx->volume = max(apctx->volumeMin, min(apctx->volumeMax, apctx->volume + newStep));

	AJ_InfoPrintf(("[allplay_volume_adjust] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_mute(allplay_ctx_t *apctx, void *userData, bool_t mute) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_mute] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_SET_PROP,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_mute] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;
	replyCtx->propertyId = ORG_ALLJOYN_CONTROL_VOLUME_MUTE;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_mute] fail to marshal property: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "b", (uint32_t)mute);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_mute] fail to marshal mute: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_mute] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	apctx->mute = mute;

	AJ_InfoPrintf(("[allplay_mute] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_play(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_RESUME,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_play] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_pause(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_pause] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_PAUSE,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_pause] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_pause] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_pause] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_stop(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_stop] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_STOP,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_stop] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_stop] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_stop] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_next(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_next] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_NEXT,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_next] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_next] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_next] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_previous(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_previous] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_PREVIOUS,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_previous] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_previous] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_previous] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_set_position(allplay_ctx_t *apctx, void *userData, signed long long position) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_position] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_SETPOSITION,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_position] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "i", position);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_position] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_position] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_set_position] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_get_player_state(allplay_ctx_t *apctx, void *userData) {
	return sendPlayerGetProperty(apctx, userData, NET_ALLPLAY_MEDIAPLAYER_PLAYSTATE);
}


allplay_status allplay_get_friendly_name(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_position] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_GETPLAYERINFO,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_get_friendly_name] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_get_friendly_name] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_get_friendly_name] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_set_external_source(allplay_ctx_t *apctx, void *userData, const char *name, bool_t interruptible, bool_t volumeCtrlEnabled) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_external_source] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_SETEXTERNALSOURCE,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_external_source] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "sbb", name, (uint32_t)interruptible, (uint32_t)volumeCtrlEnabled);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_external_source] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_external_source] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_set_external_source] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_play_item(allplay_ctx_t *apctx, void *userData, const char *url,
    const char *title, const char *artist, const char *thumbnailUrl, int32_t duration, const char *album, const char *genre) {

	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play_item] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_PLAYITEM,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play_item] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "ssssxss", url, title, artist, thumbnailUrl, (int64_t) duration, album, genre);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play_item] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_play_item] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_play_item] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_get_current_item_url(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_get_current_item_url] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_GETCURRENTITEMURL,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_get_current_item_url] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_get_current_item_url] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_get_current_item_url] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_get_loop_mode(allplay_ctx_t *apctx, void *userData) {
	return sendPlayerGetProperty(apctx, userData, NET_ALLPLAY_MEDIAPLAYER_LOOPMODE);
}

allplay_status allplay_advance_loop_mode(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_Printf("[allplay_advance_loop_mode] Failed build name: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_ADVANCE_LOOP_MODE,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_Printf("[allplay_advance_loop_mode] fail to marshal call: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_Printf("[allplay_adavance_loop_mode] fail to deliver message: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_Printf("[allplay_advance_loop_mode] done\n");
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_get_shuffle_mode(allplay_ctx_t *apctx, void *userData) {
	return sendPlayerGetProperty(apctx, userData, NET_ALLPLAY_MEDIAPLAYER_SHUFFLEMODE);
}
allplay_status allplay_toggle_shuffle_mode(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->playerSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildPlayerNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_Printf("[allplay_toggle_shuffle_mode] Failed build name: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MEDIAPLAYER_TOGGLE_SHUFFLE_MODE,
		objectName,
		apctx->playerSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_Printf("[allplay_toggle_shuffle_mode] fail to marshal call: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_Printf("[allplay_toggle_shuffle_mode] fail to deliver message: " AJ_STATUS_DEBUG(status));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_Printf("[allplay_toggle_shuffle_mode] done\n");
	return ALLPLAY_ERROR_NONE;
}
