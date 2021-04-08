/**
 * @file        AllPlayDrv.h
 * @brief       All Play Driver provides an interface to AllPlay/AllJoyn modules
 * @author      Wesley Lee 
 * @date        2014-03-05
 * @copyright   Tymphany Ltd.
 */

#ifndef ALLPLAYDRV_H
#define ALLPLAYDRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "aj_serio.h"
#include "AllPlaySrv.h"

#define FRAME_DELIMITER (0xC0)

IND_EVT(AllPlayPlayerStateIndEvt)
    enum allplay_player_state_value state;
END_IND_EVT(AllPlayPlayerStateIndEvt)

CLASS(cAllPlayDrv)
    /* private data */
    bool                            bAllplayConnected: 1;    /*the state of connection between MCU and SAM 0: disconnected 1: connected*/
    bool                            bAllplayBtEnabled:1;    //!<bluetooth is enable or not
    bool                            bAllplayBtPairable:1;   //!<pairing is enabled or not
    bool                            bAllplayBtConnected: 1; //!<a Bluetooth device connected to the SAM
    bool                            bIsGrouped: 1;          /* Indicate if the speaker is in a group or not. */
    bool                            isNetworkMode: 1;
    enum eAllplayContentSource      contentSource;
    allplay_ctx_t*                  apctx;
    uint16                          initPhase;
    enum allplay_system_mode_value  eSystemMode; /* SAM System mode */
    enum allplay_player_state_value ePlayerState;
    enum allplay_network_type_value networkType;
METHODS
    /* public functions */

/**
* void AllPlayDrv_Ctor(cAllPlayDrv *me, bool bResetSam);
* @param[in]    me              All-Play Driver instance
* @param[in]    bResetSam       setting on whether the contructor should assert the reset line for SAM
* @return       void
*/
void AllPlayDrv_Ctor(cAllPlayDrv *me, bool bAssertSamResetLine);
void AllPlayDrv_Xtor(cAllPlayDrv *me);

#ifdef ALLPLAY_GPIO_TRIGGER_FACTORY_RESET
/**
* Sequences of toggle the SAM GPIO 13 (aka factory reset)
* @param[in]    me              All-Play Driver object
* @param[in]    reInit          Restart the whole sequence
* @return       uint16          0: sequence finish, otherwise: delay time in ms
*/
uint16 AllPlayDrv_SAMReset(cAllPlayDrv* me, bool reInit);
#endif

/**
* Construct the All-Play context object
* @param[in]    me              All-Play Driver object
* @return       uint16          0: success, otherwise: fail
*/
eTpRet AllPlayDrv_newCtx(cAllPlayDrv* me);

/**
* Free the All Play context object
* @param[in]    me              All-Play Driver object
*/
void AllPlayDrv_freeCtx(cAllPlayDrv* me);

/**
* All-Play ticker from qualplat
* @param[in]    me              All-Play Driver object
* @return       allplay_message_type          Message types.
*/
enum allplay_message_type AllPlayDrv_Ticker(cAllPlayDrv* me);

/**
* All-Play command handler
* @param[in]    me              All-Play Driver object
* @param[in]    cmd             All-Play command to process
* @return       allplay_message_type          Message types.
*/
enum allplay_message_type AllPlayDrv_cmd(cAllPlayDrv* me, const AllPlayCmdEvt* e);

void AllPlayDrv_SetRx(uint8_t* buf, uint32_t len);
void AllPlayDrv_SetRxCB(AJ_SerIORxCompleteFunc rx_cb);
void AllPlayDrv_RxIntCb(void* p);
/**
* void AllPlayDrv_assertReset(bool bAssert);
* @param[in]    bAssert         setting on whether to assert the reset line or not
* @return       void
*/
void AllPlayDrv_assertReset(bool bAssert);

const char* AllPlayDrvHelper_convertSysmodeToString(enum allplay_system_mode_value systemMode);
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* ALLPLAYDRV_H */

