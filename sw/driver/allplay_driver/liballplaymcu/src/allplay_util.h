/**************************************************************
 * Copyright (C) 2013, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef ALLPLAY_UTIL_H_
#define ALLPLAY_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "allplay_common.h"

struct ApReplyContext {
	uint32_t serial;
	uint32_t propertyId;
	void *userData;
};

struct ApReplyContext *getApReplyContext(uint32_t serial, int32_t allocate);
void resetApReplyContext(void);
allplay_message_t* makeApError(allplay_status code);
allplay_message_t* makeVolumeInfo(allplay_ctx_t *apctx, enum allplay_message_type msgType);
allplay_message_t* makeBtState(allplay_ctx_t *apctx, enum allplay_message_type msgType);
enum allplay_player_state_value playStateFromString(const char *state);
AJ_Status buildPlayerNameId(allplay_ctx_t *ctx, char *dst, size_t len);
AJ_Status buildSystemNameId(allplay_ctx_t *ctx, char *dst, size_t len);
allplay_status sendGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t msgId, const char *service, uint32_t sessionId, uint32_t propertyId);
allplay_status sendGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t msgId, const char *service, uint32_t sessionId, uint32_t propertyId);


#ifdef __cplusplus
}
#endif


#endif /* ALLPLAY_UTIL_H_ */
