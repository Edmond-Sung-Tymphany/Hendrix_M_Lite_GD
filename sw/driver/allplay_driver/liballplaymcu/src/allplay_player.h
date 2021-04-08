/**************************************************************
 * Copyright (C) 2013-2015, Qualcomm Connected Experiences, Inc.
 * All rights reserved. Confidential and Proprietary.
 **************************************************************/

#ifndef ALLPLAY_PLAYER_H_
#define ALLPLAY_PLAYER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "allplay_common.h"

allplay_message_t* processPlayStateArg(AJ_Message *ajMsg, enum allplay_message_type msgType);
allplay_message_t* processPlayStateChanged(AJ_Message *ajMsg);
allplay_message_t* processPlayerVolumeChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg);
allplay_message_t* processPlayerMuteChanged(allplay_ctx_t *apctx, AJ_Message *ajMsg);
allplay_message_t* processPlayStateReply(AJ_Message *ajMsg, enum allplay_message_type msgType);
allplay_message_t* processVolumeInfo(allplay_ctx_t *apctx, AJ_Message *ajMsg);
allplay_message_t* processGetPlayerInfo(AJ_Message *ajMsg);
allplay_message_t* processGetCurrentItemUrl(AJ_Message *ajMsg);

allplay_message_t* processLoopModeArg(AJ_Message *ajMsg, enum allplay_message_type msgType);
allplay_message_t* processLoopModeChanged(AJ_Message *ajMsg);
allplay_message_t* processLoopModeReply(AJ_Message *ajMsg);
allplay_message_t* processShuffleModeArg(AJ_Message *ajMsg, enum allplay_message_type msgType);
allplay_message_t* processShuffleModeChanged(AJ_Message *ajMsg);
allplay_message_t* processShuffleModeReply(AJ_Message *ajMsg);

allplay_status sendPlayerGetProperty(allplay_ctx_t *apctx, void *userData, uint32_t propertyId);
allplay_status sendPlayerGetAllProperties(allplay_ctx_t *apctx, void *userData, uint32_t allPropId);

allplay_status allplay_get_volume_info(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_set_volume(allplay_ctx_t *apctx, void *userData, int32_t volume);
allplay_status allplay_volume_adjust(allplay_ctx_t *apctx, void *userData, int32_t step);
allplay_status allplay_mute(allplay_ctx_t *apctx, void *userData, bool_t mute);
allplay_status allplay_play(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_pause(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_stop(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_next(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_previous(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_set_position(allplay_ctx_t *apctx, void *userData, signed long long position);
allplay_status allplay_get_player_state(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_set_external_source(allplay_ctx_t *apctx, void *userData, const char *name, bool_t interruptable, bool_t volumeControlable);
allplay_status allplay_play_item(allplay_ctx_t *apctx, void *userData, const char *url,
	const char *title, const char *artist, const char *thumbnailUrl, int32_t duration, const char *album, const char *genre);
allplay_status allplay_get_current_item_url(allplay_ctx_t *apctx, void *userData);

allplay_status allplay_get_loop_mode(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_advance_loop_mode(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_get_shuffle_mode(allplay_ctx_t *apctx, void *userData);
allplay_status allplay_toggle_shuffle_mode(allplay_ctx_t *apctx, void *userData);


#ifdef __cplusplus
}
#endif

#endif /* ALLPLAY_PLAYER_H_ */
