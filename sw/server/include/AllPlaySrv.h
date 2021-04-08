/*****************************************************************************
*  @file      AllPlaySrv.h
*  @brief     Header file for base allplay server class
*  @author    Christopher Alexander
*  @date      21-Jan-2014
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef ALLPLAYSRV_H
#define	ALLPLAYSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"
#include "allplaymcu.h"

typedef enum
{
    ALLPLAY_STATE_PREPARATION,
    ALLPLAY_STATE_ACTIVE,
    ALLPLAY_STATE_PLAYER_UNKNOWN, //!<Player is in an unknown state
    ALLPLAY_STATE_PLAYER_STOPPED, //!<Player is stopped
    ALLPLAY_STATE_PLAYER_TRANSITIONING, //!<Player is switching track
    ALLPLAY_STATE_PLAYER_BUFFERING, //!<Player is buffering
    ALLPLAY_STATE_PLAYER_PLAYING, //!<Player is playing
    ALLPLAY_STATE_PLAYER_PAUSED,//!<Player is paused
    ALLPLAY_STATE_MCU_SAM_CONNECTED,//!<MCU is  connected to SAM
    ALLPLAY_STATE_MCU_SAM_DISCONNECTED,//!<MCU is  disconnected to SAM
    ALLPLAY_STATE_VOLUME_INFO,//!<Allplay volume info
    ALLPLAY_STATE_VOLUME_CHANGED,//!<Allplay volume changed
    ALLPLAY_STATE_SYSTEM_MODE_CHANGED,//!<Allplay system mode changed
    ALLPLAY_STATE_NETWORK_INFO_CHANGED,//!<Allplay network info changed
    ALLPLAY_STATE_BT_ENABLE,//!<Bluetooth is enable
    ALLPLAY_STATE_BT_DISABLE,//!<Bluetooth is disable
    ALLPLAY_STATE_BT_PAIRING_ENABLE,//!<Bluetooth pairing is enable
    ALLPLAY_STATE_BT_PAIRING_DISABLE,//!<Bluetooth pairing is disable
    ALLPLAY_STATE_BT_DEVICE_CONNECTED,//!<Bluetooth device is connected to SAM.
    ALLPLAY_STATE_BT_DEVICE_DISCONNECTED,//!<Bluetooth device is disconnected to SAM.
    ALLPLAY_STATE_SAM_FACTORY_RESET_DONE,//!<Allplay factory reset done
    ALLPLAY_STATE_SAM_FIRMWARE_UPDATE_SUCCESSFUL,
    ALLPLAY_STATE_SAM_FIRMWARE_UPDATE_FAILED,
    ALLPLAY_STATE_RESET,
    ALLPLAY_STATE_OFF,
    ALLPLAY_STATE_MAX
} eAllPlayStateIndEvt;

typedef enum
{
    ALLPLAY_CMD_VOL_INC,
    ALLPLAY_CMD_VOL_DEC,
    ALLPLAY_CMD_SET_VOL,
    ALLPLAY_CMD_PLAY,
    ALLPLAY_CMD_PLAY_PAUSE,
    ALLPLAY_CMD_PREV,
    ALLPLAY_CMD_NEXT,
    ALLPLAY_CMD_NET_RESET,
    ALLPLAY_CMD_DIRECT_MODE_ENABLE,
    ALLPLAY_CMD_RESET_TO_FACTORY,
    ALLPLAY_CMD_SET_MCU_IDLE,
    ALLPLAY_CMD_SET_BATTERY_STATE,
    ALLPLAY_CMD_SET_EXTERNAL_SOURCE,
#ifdef HAS_ALLPLAY_BT
    ALLPLAY_CMD_BT_ENABLE,
    ALLPLAY_CMD_BT_PAIRABLE,
#endif
    ALLPLAY_CMD_MAX
} eAllPlayCmdEvt;

typedef struct
{
    bool    onBattery;
    uint8   chargeLevel;
    int32   batteryAutonomy;
    int32   timeToFullCharge;
}tBatteryState;

#define SOURCE_NAME_LENGTH (32)
typedef struct
{
    bool    interruptible;
    bool    volumeCtrlEnabled;
    char    name[SOURCE_NAME_LENGTH];
}tExternalSourceInfo;

enum eAllplayContentSource {
    ALLPLAY_AP,
    ALLPLAY_BT,
    ALLPLAY_LINE_IN,
    MCU_AUXIN
};

IND_EVT(AllPlayStateIndEvt)
    eAllPlayStateIndEvt allPlayState;
    union{
        int currentVol;         //!<read this for a ALLPLAY_STATE_VOLUME_CHANGED event
        struct{
            bool mute;
            int32 volume;
            int32 max_volume;
        }volChangeInfo;
        int currentSystemMode;  //!<read this for a ALLPLAY_STATE_SYSTEM_MODE_CHANGED event
        enum allplay_network_type_value networkType;   //!<read this for a ALLPLAY_STATE_NETWORK_INFO_CHANGED event
    }payload;
END_IND_EVT(AllPlayStateIndEvt)

IND_EVT(AllPlayEqIndEvt)
    int gains[5];
END_IND_EVT(AllPlayEqIndEvt)

IND_EVT(AllPlayLedIndEvt)
    int rValue;
    int gValue;
    int bValue;
END_IND_EVT(AllPlayLedIndEvt)

REQ_EVT(AllPlayCmdEvt)
    eAllPlayCmdEvt allPlayCmd;
    union{
        uint8 setVol;         //!<set this for a ALLPLAY_CMD_SET_VOL request
        int8 volStep;  //!<set this for a ALLPLAY_CMD_VOL_INC / ALLPLAY_CMD_VOL_DEC request
        bool bEnableDirect;     //!< set this when sending ALLPLAY_CMD_DIRECT_MODE_ENABLE
        bool bIsIdle;
#ifdef HAS_ALLPLAY_BT
        bool bBtEnable;
        bool bBtPairable;
#endif
        tBatteryState batteryState;
        tExternalSourceInfo externalSourceInfo;
    }payload;
END_REQ_EVT(AllPlayCmdEvt)

/* Server => UI response/public event*/
IND_EVT(AllPlaySrvInfoEvt)
    bool bAllplayConnected: 1;    /*the state of connection between MCU and SAM 0: disconnected 1: connected*/
    bool bAllplayBtEnabled:1;    //!<bluetooth is enable or not
    bool bAllplayBtPairable:1;   //!<pairing is enabled or not
    bool bAllplayBtConnected: 1;
    bool bIsGrouped: 1;          /* Indicate if the speaker is in a group or not. */
    enum eAllplayContentSource contentSource;
    enum allplay_system_mode_value eSystemMode; /* SAM System mode */
    enum allplay_player_state_value ePlayerState; /* AllPlay Player state */
    enum allplay_network_type_value networkType;
END_IND_EVT(AllPlaySrvInfoEvt)

SUBCLASS(cAllPlaySrv, cServer)
    /* private: */
//    tAllPlayConfig config;
    QTimeEvt resetAssertEvt;
    int32 timer;
METHODS
    /* public functions */
END_CLASS

void AllPlaySrv_StartUp(cPersistantObj *me);
void AllPlaySrv_ShutDown(cPersistantObj *me);

void AllPlaySrv_netReset(void);
void AllPlaySrv_enableDirectMode(bool bEnable);
void AllPlaySrv_resetToFactory(void);
void AllPlaySrv_SendCmd(eAllPlayCmdEvt allplayCmd);
void AllPlaySrv_IncVol(int8 volStep);
void AllPlaySrv_DecVol(int8 volStep);
void AllPlaySrv_SetVol(const uint8 setVol);
void AllPlaySrv_SetMcuIdle(const bool bIsIdle);
void AllPlaySrv_SetBatteryState(const tBatteryState batteryState);
void AllPlaySrv_SetExternalSource(const tExternalSourceInfo externalSourceInfo);
#ifdef HAS_ALLPLAY_BT
void AllPlaySrv_BluetoothEnable(const bool bBtEnable);
void AllPlaySrv_BluetoothPairable(const bool bBtPairable);
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* ALLPLAYSRV_H */
