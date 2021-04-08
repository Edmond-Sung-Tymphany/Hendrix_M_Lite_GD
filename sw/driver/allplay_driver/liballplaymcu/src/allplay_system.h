/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef ALLPLAY_SYSTEM_H_
#define ALLPLAY_SYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "allplay_common.h"

allplay_message_t* processWpsResult(AJ_Message *ajMsg);
allplay_message_t* processSystemModeChanged(AJ_Message *ajMsg);
allplay_message_t* processCmdResult(AJ_Message *ajMsg);
allplay_message_t* processNetworkInfo2Reply(AJ_Message *ajMsg);
allplay_message_t* processRSSIReply(AJ_Message *ajMsg);
allplay_message_t* processSystemModeReply(AJ_Message *ajMsg);
allplay_message_t* processFirmwareVersionReply(AJ_Message *ajMsg);
allplay_message_t* processResamplingModeReply(AJ_Message *ajMsg);
allplay_message_t* processRunCmdReply(AJ_Message *ajMsg);
allplay_message_t* processFirmwareCheck(AJ_Message *ajMsg);
allplay_message_t* processUpdateAvailable(AJ_Message *ajMsg);
allplay_message_t* processUpdateStatus(AJ_Message *ajMsg);
allplay_message_t* processNetworkInfo2Changed(AJ_Message *ajMsg);
allplay_message_t* processNetworkInfo2(AJ_Message *ajMsg, enum allplay_message_type message_type);
allplay_message_t* processBtInfo(allplay_ctx_t *apctx, AJ_Message *ajMsg);
allplay_message_t* processBtStateChanged(allplay_ctx_t *apctx,AJ_Message *ajMsg);
allplay_message_t* processBtEnabledChanged(allplay_ctx_t *apctx,AJ_Message *ajMsg);
allplay_message_t* processBtPairableChanged(allplay_ctx_t *apctx,AJ_Message *ajMsg);
allplay_message_t* processGetAboutData(AJ_Message *ajMsg);
allplay_status sendMcuSystemGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t propertyId);
allplay_status sendMcuSystemGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t allPropId);

allplay_status sendMcuVersion(allplay_ctx_t *apctx);

allplay_status allplay_network_request_wps(allplay_ctx_t *apctx, void *userData, enum allplay_network_wps_type type, int32_t pin);
allplay_status allplay_start_setup(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_reset_to_factory(allplay_ctx_t *apctx, void *userData, allplay_reset_action_t action);
allplay_status allplay_network_get_info(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_get_rssi(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_get_system_mode(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_get_firmware_version(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_set_resampling_mode(allplay_ctx_t *apctx, void *userData, int resamplingMode);
allplay_status allplay_get_resampling_mode(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_run_cmd(allplay_ctx_t *apctx, void *userData, const char *cmd, uint32_t stdoutMaxLen, uint32_t stderrMaxLen);
allplay_status allplay_connect_wifi(allplay_ctx_t *apctx, void *userData, const char *ssid, const char *password);

allplay_status allplay_firmware_check(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_firmware_update(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_firmware_update_from_url(allplay_ctx_t *apctx, void *userData, const char *url);

allplay_status allplay_set_mcu_idle(allplay_ctx_t *apctx, void *userData, bool_t idle);
allplay_status allplay_set_battery_state(allplay_ctx_t *apctx, void *userData, bool_t onBattery, char chargeLevel, int32_t batteryAutonomy, int32_t timeToFullCharge);

allplay_status allplay_bluetooth_get_state(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_bluetooth_enable(allplay_ctx_t *apctx, void *userData, bool_t enabled);
allplay_status allplay_bluetooth_enable_pairing(allplay_ctx_t *apctx, void *userData, bool_t enabled);

allplay_status allplay_get_device_info(allplay_ctx_t *apctx, void *userData);

allplay_status allplay_shutdown_sam(allplay_ctx_t *apctx, void *userData, bool_t restart);

#ifdef __cplusplus
}
#endif

#endif /* ALLPLAY_SYSTEM_H_ */
