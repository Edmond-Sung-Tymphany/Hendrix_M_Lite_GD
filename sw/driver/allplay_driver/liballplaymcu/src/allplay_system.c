/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/
#include "allplay_system.h"

#include <stddef.h>
#include <stdlib.h>
#include "allplay_interfaces.h"
#include "allplay_util.h"

allplay_message_t* processWpsResult(AJ_Message *ajMsg) {
	allplay_network_wps_result_t *result;
	AJ_Arg arg;
	AJ_Status status = AJ_UnmarshalArg(ajMsg, &arg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processWpsResult] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_network_wps_result_t));
	result->messageType = ALLPLAY_EVENT_NETWORK_WPS_RESULT;
	result->userData = NULL;
	result->result = *(arg.val.v_int32);

	return (allplay_message_t*)result;
}

allplay_message_t* processSystemModeChanged(AJ_Message *ajMsg) {
	allplay_system_mode_t *result;
	AJ_Arg arg;
	AJ_Status status = AJ_UnmarshalArg(ajMsg, &arg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processWpsResult] Failed to unmarshal: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_system_mode_t));
	result->messageType = ALLPLAY_EVENT_SYSTEM_MODE;
	result->userData = NULL;
	result->mode = *(arg.val.v_int32);

	return (allplay_message_t*)result;
}

allplay_message_t* processNetworkInfo2Changed(AJ_Message *ajMsg) {
	return processNetworkInfo2(ajMsg, ALLPLAY_EVENT_NETWORK_INFO_CHANGED);
}

allplay_message_t* processCmdResult(AJ_Message *ajMsg) {
	allplay_cmd_result_t *result;
	AJ_Status status;
	uint32_t id;
	int32_t exitstatus;
	int32_t termsig;
	const char *stdoutStr;
	const char *stderrStr;

	status = AJ_UnmarshalArgs(ajMsg, "uiiss", &id, &exitstatus, &termsig, &stdoutStr, &stderrStr);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processCmdResult] Failed to unmarshal args: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_cmd_result_t) + strlen(stdoutStr) + 1 + strlen(stderrStr) + 1);
	result->messageType = ALLPLAY_EVENT_CMD_RESULT;
	result->id = id;
	result->exitstatus = exitstatus;
	result->termsig = termsig;
	result->stdoutStr = ((char*)result) + sizeof(allplay_cmd_result_t);
	strcpy(result->stdoutStr, stdoutStr);
	result->stderrStr = ((char*)result) + sizeof(allplay_cmd_result_t) + strlen(stdoutStr) + 1;
	strcpy(result->stderrStr, stderrStr);

	return (allplay_message_t*)result;

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processNetworkInfo2Reply(AJ_Message *ajMsg) {
	AJ_Status status;
	const char *sig;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processNetworkInfo2Reply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "a(iyayaysyi)") != 0) {
		AJ_ErrPrintf(("[processNetworkInfo2Reply] Unexpected signature: %s\n", sig));
		goto error;
	}

	return processNetworkInfo2(ajMsg, ALLPLAY_RESPONSE_NETWORK_INFO);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processNetworkInfo2(AJ_Message *ajMsg, enum allplay_message_type message_type) {
	allplay_network_info_t *result;
	AJ_Status status;
	AJ_Arg arg;
	AJ_Arg intfArray;
	int32_t type;
	unsigned char state;
	AJ_Arg addrArray;
	const char *ssid;
	unsigned char rssi;
	int32_t frequency;

	result = AJ_Malloc(sizeof(allplay_network_info_t));
	memset(result, 0, sizeof(allplay_network_info_t));
	result->messageType = message_type;

	status = AJ_UnmarshalContainer(ajMsg, &intfArray, AJ_ARG_ARRAY);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	for(;;) {
		status = AJ_UnmarshalContainer(ajMsg, &arg, AJ_ARG_STRUCT);
		if (status == AJ_ERR_NO_MORE) {
			break;
		}
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal struct: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		status = AJ_UnmarshalArgs(ajMsg, "i", &type);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal type: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		status = AJ_UnmarshalArgs(ajMsg, "y", &state);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal state: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (state == 1) {
			// interface is up, make it the connection type
			// And to behave like NetworkInfo, ethernet has priori over wifi
			// which has priority over none. This happens to also be the order
			// of the enum values, so we can just use '<'
			if (((int32_t)result->type) < type) {
				result->type = type;
			}
		}

		status = AJ_UnmarshalArg(ajMsg, &addrArray);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal IP array: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (addrArray.len != 4) {
			AJ_ErrPrintf(("[processNetworkInfo2] Unexpected IP array length: %d\n", addrArray.len));
			goto error;
		}
		if (type == ALLPLAY_NETWORK_ETHERNET) {
			memcpy(result->ethernetIp, addrArray.val.v_byte, 4);
		}
		else if (type == ALLPLAY_NETWORK_WIFI) {
			memcpy(result->wifiIp, addrArray.val.v_byte, 4);
		}

		status = AJ_UnmarshalArg(ajMsg, &addrArray);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal MAC array: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (addrArray.len != 6) {
			AJ_ErrPrintf(("[processNetworkInfo2] Unexpected MAC array length: %d\n", addrArray.len));
			goto error;
		}
		if (type == ALLPLAY_NETWORK_ETHERNET) {
			memcpy(result->ethernetMac, addrArray.val.v_byte, 6);
		}
		else if (type == ALLPLAY_NETWORK_WIFI) {
			memcpy(result->wifiMac, addrArray.val.v_byte, 6);
		}

		status = AJ_UnmarshalArgs(ajMsg, "syi", &ssid, &rssi, &frequency);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to unmarshal ssid/rssi: " AJ_STATUS_DEBUG(status)));
			goto error;
		}
		if (type == ALLPLAY_NETWORK_WIFI) {
			strncpy(result->ssid, ssid, ArraySize(result->ssid) - 1);
			result->ssid[ArraySize(result->ssid) - 1] = '\0';
			result->rssi = rssi;
			result->frequency = frequency;
		}

		status = AJ_UnmarshalCloseContainer(ajMsg, &arg);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processNetworkInfo2] Failed to close struct: " AJ_STATUS_DEBUG(status)));
			goto error;
		}
	}

	status = AJ_UnmarshalCloseContainer(ajMsg, &intfArray);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processNetworkInfo2] Failed to close array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	return (allplay_message_t*)result;

error:
	AJ_Free(result);
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processSystemModeReply(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_system_mode_t *result;
	AJ_Status status;
	const char *sig;
	int32_t mode;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processSystemModeReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "i") != 0) {
		AJ_ErrPrintf(("[processSystemModeReply] Unexpected signature: %s\n", sig));
		goto error;
	}

	status = AJ_UnmarshalArgs(ajMsg, "i", &mode);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processSystemModeReply] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_system_mode_t));
	result->messageType = ALLPLAY_RESPONSE_SYSTEM_MODE;
	result->mode = mode;

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processFirmwareVersionReply(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_firmware_version_t *result;
	AJ_Status status;
	const char *sig;
	const char *version;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processFirmwareVersionReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "s") != 0) {
		AJ_ErrPrintf(("[processFirmwareVersionReply] Unexpected signature: %s\n", sig));
		goto error;
	}

	status = AJ_UnmarshalArgs(ajMsg, "s", &version);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processFirmwareVersionReply] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_firmware_version_t) + strlen(version) + 1);
	result->messageType = ALLPLAY_RESPONSE_FIRMWARE_VERSION;
	result->version = ((char*)result) + sizeof(allplay_firmware_version_t);
	strcpy(result->version, version);

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processResamplingModeReply(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_resampling_mode_t *result;
	AJ_Status status;
	const char *sig;
	int resamplingMode;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processResamplingModeReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "i") != 0) {
		AJ_ErrPrintf(("[processResamplingModeReply] Unexpected signature: %s\n", sig));
		goto error;
	}

	status = AJ_UnmarshalArgs(ajMsg, "i", &resamplingMode);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processResamplingModeReply] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_resampling_mode_t));
	result->messageType = ALLPLAY_RESPONSE_GET_RESAMPLING_MODE;
	result->resamplingMode = resamplingMode;

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processRunCmdReply(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_cmd_t *result;
	AJ_Status status;
	uint32_t id;

	status = AJ_UnmarshalArgs(ajMsg, "u", &id);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processRunCmdReply] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_cmd_t));
	result->messageType = ALLPLAY_RESPONSE_RUN_CMD;
	result->id = id;

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processRSSIReply(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_rssi_t *result;
	AJ_Status status;
	AJ_Arg arg;
	const char *sig;
	size_t i = 0;
	size_t chain_count = 0;
	size_t max_result_count = 0;

	status = AJ_UnmarshalVariant(ajMsg, &sig);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processRSSIReply] Failed to unmarshal variant: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (strcmp(sig, "ai") != 0) {
		AJ_ErrPrintf(("[processRSSIReply] Unexpected signature: %s\n", sig));
		goto error;
	}

	status = AJ_UnmarshalArg(ajMsg, &arg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processRSSIReply] Failed to unmarshal RSSI measurement array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_rssi_t));
	memset(result, 0, sizeof(allplay_rssi_t));
	result->messageType = ALLPLAY_RESPONSE_RSSI;
	chain_count = arg.len/(sizeof(int32_t));
	max_result_count = sizeof(result->chain)/sizeof(result->chain[0]);

	chain_count = chain_count < max_result_count ? chain_count : max_result_count;

	for (; i < chain_count; ++i)  {
		result->chain[i] = ((int32_t*)arg.val.v_data)[i];
	}

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processFirmwareCheck(AJ_Message *ajMsg) {
	allplay_message_t *error;
	allplay_firmware_update_info_t *result;
	AJ_Status status;
	uint32_t available;
	const char *version;
	const char *url;

	status = AJ_UnmarshalArgs(ajMsg, "bss", &available, &version, &url);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processFirmwareCheck] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_firmware_update_info_t) + (available ? (strlen(version) + 1 + strlen(url) + 1) : 0));
	result->messageType = ALLPLAY_RESPONSE_FIRMWARE_CHECK;

	if (!available) {
		result->version = NULL;
		result->url = NULL;
	}
	else {
		result->version = ((char *)result) + sizeof(allplay_firmware_update_info_t);
		strcpy(result->version, version);
		result->url = result->version + strlen(version) + 1;
		strcpy(result->url, url);
	}

	return (allplay_message_t*)result;

error:
	error = makeApError(ALLPLAY_ERROR_FAILED);
	return error;
}

allplay_message_t* processUpdateAvailable(AJ_Message *ajMsg) {
	allplay_firmware_update_info_t *result;
	AJ_Status status;
	const char *version;
	const char *url;

	status = AJ_UnmarshalArgs(ajMsg, "ss", &version, &url);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processFirmwareCheck] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_firmware_update_info_t) + strlen(version) + 1 + strlen(url) + 1);
	result->messageType = ALLPLAY_EVENT_FIRMWARE_UPDATE_AVAILABLE;

	result->version = ((char *)result) + sizeof(allplay_firmware_update_info_t);
	strcpy(result->version, version);
	result->url = result->version + strlen(version) + 1;
	strcpy(result->url, url);

	return (allplay_message_t*)result;
}

allplay_message_t* processUpdateStatus(AJ_Message *ajMsg) {
	allplay_firmware_update_status_t *result;
	AJ_Status status;
	int32_t updateStatus;

	status = AJ_UnmarshalArgs(ajMsg, "i", &updateStatus);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processFirmwareCheck] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	result = AJ_Malloc(sizeof(allplay_firmware_update_status_t));
	result->messageType = ALLPLAY_EVENT_FIRMWARE_UPDATE_STATUS;

	result->status = updateStatus;

	return (allplay_message_t*)result;
}

allplay_message_t* processBtInfo(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Status status;
	AJ_Arg allProps;
	AJ_Arg prop;
	const char *propName;
	const char *propValueSig;

	status = AJ_UnmarshalContainer(ajMsg, &allProps, AJ_ARG_ARRAY);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtInfo] Failed to open array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	for(;;) {
		status = AJ_UnmarshalContainer(ajMsg, &prop, AJ_ARG_DICT_ENTRY);
		if (status == AJ_ERR_NO_MORE) {
			break;
		}
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processBtInfo] Failed to open dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		status = AJ_UnmarshalArgs(ajMsg, "s", &propName);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processBtInfo] Failed to get propName: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (strcmp(propName, "Enabled") == 0) {
			uint32_t enabled;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "b", &enabled);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get enabled: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			apctx->bluetooth.enabled = enabled ? 1 : 0;
		}
		else if (strcmp(propName, "Pairable") == 0) {
			uint32_t pairable;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "b", &pairable);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get pairable: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			apctx->bluetooth.pairable = pairable ? 1 : 0;
		}
		else if (strcmp(propName, "State") == 0) {
			AJ_Arg state;
			uint32_t btCount, a2dpCount;
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalContainer(ajMsg, &state, AJ_ARG_STRUCT);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to open state: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			status = AJ_UnmarshalArgs(ajMsg, "uu", &btCount, &a2dpCount);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to get state: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalCloseContainer(ajMsg, &state);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to close state: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			apctx->bluetooth.btCount = btCount;
			apctx->bluetooth.a2dpCount = a2dpCount;
		}
		else {
			// Unknown or unwanted property
			status = AJ_SkipArg(ajMsg);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processBtInfo] Failed to skip: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
		}

		status = AJ_UnmarshalCloseContainer(ajMsg, &prop);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processBtInfo] Failed to close dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}
	}

	status = AJ_UnmarshalCloseContainer(ajMsg, &allProps);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtInfo] Failed to close array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	apctx->bluetooth.initialized = TRUE;
	AJ_InfoPrintf(("[processBtInfo] done\n"));

	return makeBtState(apctx, ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE);

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_message_t* processBtStateChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Status status;
	AJ_Arg state;
	uint32_t btCount, a2dpCount;

	status = AJ_UnmarshalContainer(ajMsg, &state, AJ_ARG_STRUCT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtStateChanged] Failed to open state: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}
	status = AJ_UnmarshalArgs(ajMsg, "uu", &btCount, &a2dpCount);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtStateChanged] Failed to get state: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	status = AJ_UnmarshalCloseContainer(ajMsg, &state);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtStateChanged] Failed to close state: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	apctx->bluetooth.btCount = btCount;
	apctx->bluetooth.a2dpCount = a2dpCount;

	return makeBtState(apctx, ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE);
}

allplay_message_t* processBtEnabledChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Status status;
	uint32_t enabled;

	status = AJ_UnmarshalArgs(ajMsg, "b", &enabled);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtEnabledChanged] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	apctx->bluetooth.enabled = enabled;

	return makeBtState(apctx, ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE);
}

allplay_message_t* processBtPairableChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg) {
	AJ_Status status;
	uint32_t pairable;

	status = AJ_UnmarshalArgs(ajMsg, "b", &pairable);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processBtPairableChanged] Failed to unmarshal arg: " AJ_STATUS_DEBUG(status)));
		return makeApError(ALLPLAY_ERROR_FAILED);
	}

	apctx->bluetooth.pairable = pairable;

	return makeBtState(apctx, ALLPLAY_RESPONSE_BLUETOOTH_GET_STATE);
}

allplay_message_t* processGetAboutData(AJ_Message *ajMsg) {
	allplay_device_info_t *result;
	AJ_Status status;
	AJ_Arg allProps;
	AJ_Arg prop;
	const char *propName;
	const char *propValueSig;
	const char *manufacturer = "";
	size_t manufacturerLen = 0;
	const char *model = "";
	size_t modelLen = 0;
	const char *deviceId = "";
	size_t deviceIdLen = 0;
	const char *version = "";
	size_t versionLen = 0;

	status = AJ_UnmarshalContainer(ajMsg, &allProps, AJ_ARG_ARRAY);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processGetAboutData] Failed to open array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	for(;;) {
		status = AJ_UnmarshalContainer(ajMsg, &prop, AJ_ARG_DICT_ENTRY);
		if (status == AJ_ERR_NO_MORE) {
			break;
		}
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processGetAboutData] Failed to open dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		status = AJ_UnmarshalArgs(ajMsg, "s", &propName);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processGetAboutData] Failed to get propName: " AJ_STATUS_DEBUG(status)));
			goto error;
		}

		if (strcmp(propName, "Manufacturer") == 0) {
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "s", &manufacturer);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get manufacturer: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			manufacturerLen = strlen(manufacturer);
		}
		else if (strcmp(propName, "ModelNumber") == 0) {
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "s", &model);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get model: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			modelLen = strlen(model);
		}
		else if (strcmp(propName, "DeviceId") == 0) {
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "s", &deviceId);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get deviceId: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			deviceIdLen = strlen(deviceId);
		}
		else if (strcmp(propName, "SoftwareVersion") == 0) {
			status = AJ_UnmarshalVariant(ajMsg, &propValueSig);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get propValue: " AJ_STATUS_DEBUG(status)));
				goto error;
			}

			status = AJ_UnmarshalArgs(ajMsg, "s", &version);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to get version: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
			versionLen = strlen(version);
		}
		else {
			// Unknown or unwanted property
			status = AJ_SkipArg(ajMsg);
			if (status != AJ_OK) {
				AJ_ErrPrintf(("[processGetAboutData] Failed to skip: " AJ_STATUS_DEBUG(status)));
				goto error;
			}
		}

		status = AJ_UnmarshalCloseContainer(ajMsg, &prop);
		if (status != AJ_OK) {
			AJ_ErrPrintf(("[processGetAboutData] Failed to close dict entry: " AJ_STATUS_DEBUG(status)));
			goto error;
		}
	}

	status = AJ_UnmarshalCloseContainer(ajMsg, &allProps);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[processGetAboutData] Failed to close array: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	if (!(manufacturer && model && deviceId && version)) {
		AJ_ErrPrintf(("[processGetAboutData] Missing some information: " AJ_STATUS_DEBUG(status)));
		goto error;
	}

	result = AJ_Malloc(sizeof(allplay_device_info_t) + manufacturerLen + 1 + modelLen + 1 + deviceIdLen + 1 + versionLen + 1);
	result->messageType = ALLPLAY_RESPONSE_GET_DEVICE_INFO;
	result->manufacturer = (const char*)(result + 1);
	strcpy((char*)result->manufacturer, manufacturer);
	result->model = result->manufacturer + manufacturerLen + 1;
	strcpy((char*)result->model, model);
	result->deviceId = result->model + modelLen + 1;
	strcpy((char*)result->deviceId, deviceId);
	result->firmwareVersion = result->deviceId + deviceIdLen + 1;
	strcpy((char*)result->firmwareVersion, version);

	AJ_InfoPrintf(("[processGetAboutData] done\n"));

	return (allplay_message_t*)result;

error:
	return makeApError(ALLPLAY_ERROR_FAILED);
}

allplay_status sendMcuSystemGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t propertyId) {
	AJ_Status status;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuSystemGetProperty] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	return sendGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_GET_PROP, objectName, apctx->systemSessionId, propertyId);
}

allplay_status sendMcuSystemGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t allPropId) {
	AJ_Status status;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuSystemGetAllProperties] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	return sendGetAllProperties(apctx, userData, NET_ALLPLAY_MCUSYSTEM_GET_PROP_ALL, objectName, apctx->systemSessionId, allPropId);
}

allplay_status sendMcuVersion(allplay_ctx_t *apctx) {
	AJ_Status status;
	AJ_Message msg;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuVersion] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_FIRMWARE_SETMCUVERSION,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuVersion] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "si", apctx->mcuFirmwareVersion, (int32_t)apctx->rebootMethod);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuVersion] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[sendMcuVersion] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[sendMcuVersion] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_network_request_wps(allplay_ctx_t *apctx, void *userData, enum allplay_network_wps_type type, int32_t pin) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_network_request_wps] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_REQUESTWPS,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_network_request_wps] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "ii", (int32_t)type, pin);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_network_request_wps] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_network_request_wps] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_network_request_wps] done\n"));
	return ALLPLAY_ERROR_NONE;
}


allplay_status allplay_set_resampling_mode(allplay_ctx_t *apctx, void *userData, int32_t resamplingMode) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_resampling_mode] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SET_PROP,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_resampling_mode] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;
	replyCtx->propertyId = NET_ALLPLAY_MCUSYSTEM_RESAMPLINGMODE;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_resampling_mode] fail to marshal property: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "i", resamplingMode);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_resampling_mode] fail to marshal mode: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_resampling_mode] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_set_resampling_mode] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_start_setup(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_start_setup] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_STARTSETUP,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_start_setup] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_start_setup] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_start_setup] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_reset_to_factory(allplay_ctx_t *apctx, void *userData, allplay_reset_action_t action) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_reset_to_factory] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_RESET_TO_FACTORY,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_reset_to_factory] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "u", (uint32_t)action);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_reset_to_factory] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_reset_to_factory] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_reset_to_factory] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_network_get_info(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_NETWORK_INFO2);
}

allplay_status allplay_get_rssi(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_RSSI);
}

allplay_status allplay_get_system_mode(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_SYSTEMMODE);
}

allplay_status allplay_get_firmware_version(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_FIRMWARE_VERSION);
}

allplay_status allplay_get_resampling_mode(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetProperty(apctx, userData, NET_ALLPLAY_MCUSYSTEM_RESAMPLINGMODE);
}

allplay_status allplay_wifi_enable(allplay_ctx_t *apctx, void *userData, bool_t enable) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_wifi_enable] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_WIFIENABLE,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_wifi_enable] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "b", (int32_t)enable);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_wifi_enable] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_wifi_enable] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_wifi_enable] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_directmode_enable(allplay_ctx_t *apctx, void *userData, bool_t enable) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_wifi_enable] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_DIRECTENABLE,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_directmode_enable] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "b", (int32_t)enable);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_directmode_enable] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_directmode_enable] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_wifi_enable] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_run_cmd(allplay_ctx_t *apctx, void *userData, const char *cmd, uint32_t stdoutMaxLen, uint32_t stderrMaxLen) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	if (!cmd) {
		AJ_ErrPrintf(("[allplay_run_cmd] Null command\n"));
		return ALLPLAY_ERROR_FAILED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_run_cmd] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_RUNCMD,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_run_cmd] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "suu", cmd, stdoutMaxLen, stderrMaxLen);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_run_cmd] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_run_cmd] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_run_cmd] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_connect_wifi(allplay_ctx_t *apctx, void *userData, const char *ssid, const char *password) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	if (!ssid) {
		AJ_ErrPrintf(("[allplay_connect_wifi] Null SSID\n"));
		return ALLPLAY_ERROR_FAILED;
	}

	if (!password) {
		AJ_ErrPrintf(("[allplay_connect_wifi] Null password (provide empty string if necessary)\n"));
		return ALLPLAY_ERROR_FAILED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_connect_wifi] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_WIFI_CONNECT,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_connect_wifi] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "ss", ssid, password);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_connect_wifi] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_connect_wifi] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_connect_wifi] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_firmware_check(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_check] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_FIRMWARE_CHECK,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_check] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_check] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_firmware_check] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_firmware_update(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_FIRMWARE_UPDATE,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_firmware_update] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_firmware_update_from_url(allplay_ctx_t *apctx, void *userData, const char *url) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update_from_url] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_FIRMWARE_UPDATE_FROM_URL,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update_from_url] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "s", url);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update_from_url] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_firmware_update_from_url] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_firmware_update_from_url] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_set_mcu_idle(allplay_ctx_t *apctx, void *userData, bool_t idle) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_mcu_idle] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SETMCUIDLE,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_mcu_idle] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "b", (uint32_t)idle);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_mcu_idle] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_mcu_idle] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_set_mcu_idle] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_set_battery_state(allplay_ctx_t *apctx, void *userData, bool_t onBattery, char chargeLevel, int32_t batteryAutonomy, int32_t timeToFullCharge) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_battery_state] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SETBATTERYSTATE,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_battery_state] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "byii", (uint32_t)onBattery, chargeLevel, batteryAutonomy, timeToFullCharge);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_battery_state] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_set_battery_state] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_set_battery_state] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_bluetooth_get_state(allplay_ctx_t *apctx, void *userData) {
	return sendMcuSystemGetAllProperties(apctx, userData, NET_ALLPLAY_MCUSYSTEM_BTCTRL_ALL_PROP);
}

allplay_status allplay_bluetooth_enable(allplay_ctx_t *apctx, void *userData, bool_t enabled) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SET_PROP,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;
	replyCtx->propertyId = NET_ALLPLAY_MCUSYSTEM_BTCTRL_ENABLED;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable] fail to marshal property: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "b", (uint32_t)enabled);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_bluetooth_enable] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_bluetooth_enable_pairing(allplay_ctx_t *apctx, void *userData, bool_t enabled) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable_pairing] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SET_PROP,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable_pairing] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;
	replyCtx->propertyId = NET_ALLPLAY_MCUSYSTEM_BTCTRL_PAIRABLE;

	status = AJ_MarshalPropertyArgs(&msg, replyCtx->propertyId);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable_pairing] fail to marshal property: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalArgs(&msg, "b", (uint32_t)enabled);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable_pairing] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_bluetooth_enable_pairing] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_bluetooth_enable_pairing] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_get_device_info(allplay_ctx_t *apctx, void *userData) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_getDeviceInfo] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		ORG_ALLJOYN_ABOUT_GET_ABOUT_DATA,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_getDeviceInfo] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "s", "en");
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_getDeviceInfo] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_getDeviceInfo] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_getDeviceInfo] done\n"));
	return ALLPLAY_ERROR_NONE;
}

allplay_status allplay_shutdown_sam(allplay_ctx_t *apctx, void *userData, bool_t restart) {
	AJ_Status status;
	AJ_Message msg;
	struct ApReplyContext *replyCtx;
	char objectName[128];

	if (apctx->systemSessionId == 0) {
		return ALLPLAY_NOT_CONNECTED;
	}

	status = buildSystemNameId(apctx, objectName, sizeof(objectName));
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_shutdown_sam] Failed build name: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_MarshalMethodCall(&(apctx->bus), &msg,
		NET_ALLPLAY_MCUSYSTEM_SHUTDOWN,
		objectName,
		apctx->systemSessionId, 0, METHOD_TIMEOUT);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_shutdown_sam] fail to marshal call: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	replyCtx = getApReplyContext(msg.hdr->serialNum, TRUE);
	if (!replyCtx) {
		return ALLPLAY_ERROR_FAILED;
	}
	replyCtx->userData = userData;

	status = AJ_MarshalArgs(&msg, "b", (uint32_t)restart);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_shutdown_sam] fail to marshal args: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	status = AJ_DeliverMsg(&msg);
	if (status != AJ_OK) {
		AJ_ErrPrintf(("[allplay_shutdown_sam] fail to deliver message: " AJ_STATUS_DEBUG(status)));
		return ALLPLAY_ERROR_FAILED;
	}

	AJ_InfoPrintf(("[allplay_shutdown_sam] done\n"));
	return ALLPLAY_ERROR_NONE;
}
