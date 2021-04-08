/**
 * @file        BluetoothWitronDrv_priv.h
 * @brief       BT Driver as provided by Witron
 * @author      Edmond Sung
 * @date        2014-02-10
 * @copyright   Tymphany Ltd.
 */

#define BT_RSP_ID        0x0200
#define BT_IND_ID        0x0400

#define GEN_INIT_SUCCESS_IND            0x0400
#define GEN_SHUTDOWN_COMPLETE_IND       0x0401
#define GEN_PAIRING_STATUS_IND          0x0402
#define GEN_SCAN_ENABLE_IND             0x0403
#define GEN_PIO_STATUS_IND              0x0404
#define GEN_AUDIO_CODEC_IND             0x0405
#define GEN_CONNECTION_STATUS_IND       0x0406
#define GEN_GET_SOFTWARE_VER_IND        0x0407

#define HF_STATUS_IND                   0x0420
#define HF_SCO_STATUS_IND               0x0421
#define HF_INBAND_RING_IND              0x0422
#define HF_RING_IND                     0x0423
#define HF_VOLUME_IND                   0x0427
#define A2DP_STATUS_IND                 0x0440
#define A2DP_VOLUME_IND                 0x0441
#define AVRCP_STATUS_IND                0x0460
#define GEN_TW_PAIRING_STATUS_IND       0x0461
#define GEN_TW_SHUTDOWN_IND             0x0462
#define GEN_TW_VOL_SYNC_IND             0x0463

/***************************************************************/
#define GEN_STARTUP_REQ_ID                0x0000
#define GEN_STARTUP_RSP_ID                0x0200
#define GEN_SHUTDOWN_REQ_ID               0x0001
#define GEN_SHUTDOWN_RSP_ID               0x0201
#define GEN_ENTER_PAIRING_REQ_ID          0x0002
#define GEN_ENTER_PAIRING_RSP_ID          0x0202
#define GEN_RESET_PAIRED_LIST_REQ_ID      0x0003
#define GEN_RESET_PAIRED_LIST_RSP_ID      0x0203
#define GEN_ENTER_TEST_MODE_REQ_ID        0x0004
#define GEN_ENTER_TEST_MODE_RSP_ID        0x0204
#define GEN_ENTER_DFU_MODE_REQ_ID         0x0005
#define GEN_ENTER_DFU_MODE_RSP_ID         0x0205
#define GEN_SET_LOCAL_DEVICE_NAME_REQ_ID  0x0006
#define GEN_SET_LOCAL_DEVICE_NAME_RSP_ID  0x0206

#define GEN_SET_PIN_CODE_REQ_ID           0x0007
#define GEN_SET_PIN_CODE_RSP_ID           0x0207

#define GEN_PLAY_TONE_REQ                 0x0007
#define GEN_PLAY_TONE_RSP                 0x0207

#define GEN_SET_PIO_REQ_ID                0x0008
#define GEN_SET_PIO_RSP_ID                0x0208

#define GEN_PLAY_FILE_REQ_ID              0x000a
#define GEN_CONNECT_ADDR_REQ              0x000c
#define GEN_CONNECT_ADDR_RSP              0x020c

#define GEN_DISCONNECT_REQ_ID             0x000e
#define GEN_DISCONNECT_RSP_ID             0x020e




#define GEN_GET_SW_VERSION_REQ          0x000d
#define GEN_GET_SW_VERSION_RSP          0x020d

#define GEN_SET_TONE_VOL_REQ            0x000b
#define GEN_SET_TONE_VOL_RSP            0x020b

//#define GEN_GET_SOFTWARE_VER_REQ        0x000d
//#define GEN_GET_SOFTWARE_VER_RSP        0x020d

#define HF_SLC_CONNECT_REQ_ID           0x0020
#define HF_SLC_CONNECT_RSP_ID           0x0220
#define HF_SLC_DISCONNECT_REQ_ID        0x0021
#define HF_SLC_DISCONNECT_RSP_ID        0x0221
#define HF_VOICE_DIAL_REQ_ID            0x0022
#define HF_VOICE_DIAL_RSP_ID            0x0222
#define HF_LAST_NUMBER_REDIAL_REQ_ID    0x0023
#define HF_LAST_NUMBER_REDIAL_RSP_ID    0x0223
#define HF_ANSWER_CALL_REQ_ID           0x0024
#define HF_ANSWER_CALL_RSP_ID           0x0224
#define HF_REJECT_CALL_REQ_ID           0x0025
#define HF_REJECT_CALL_RSP_ID           0x0225
#define HF_HANGUP_CALL_REQ_ID           0x0026
#define HF_HANGUP_CALL_RSP_ID           0x0226
#define HF_AUDIO_TRANSFER_REQ_ID        0x0027
#define HF_AUDIO_TRANSFER_RSP_ID        0x0227
#define HF_AUDIO_MUTE_UNMUTE_REQ_ID     0x0028
#define HF_AUDIO_MUTE_UNMUTE_RSP_ID     0x0228
#define HF_CHANGE_VOLUME_REQ_ID         0x0029
#define HF_CHANGE_VOLUME_RSP_ID         0x0229
#define HF_GET_VOLUME_REQ_ID            0x002a
#define HF_GET_VOLUME_RSP_ID            0x022a
#define AV_SLC_CONNECT_REQ_ID           0x0040
#define AV_SLC_CONNECT_RSP_ID           0x0240
#define AV_SLC_DISCONNECT_REQ_ID        0x0041
#define AV_SLC_DISCONNECT_RSP_ID        0x0241
#define AV_CHANGE_VOLUME_REQ_ID         0x0042
#define AV_CHANGE_VOLUME_RSPID          0x0242
#define AV_GET_VOLUME_REQ_ID            0x0043
#define AV_GET_VOLUME_RSP_ID            0x0243
#define A2DP_GET_STATUS_REQ_ID          0x0044
#define A2DP_GET_STATUS_RSP_ID          0x0244
#define AVRCP_SEND_CMD_REQ_ID           0x0060

#define TRUEWIRELESS_PAIR_REQ_ID        0x0062
#define TRUEWIRELESS_PAIR_RSP_ID        0x0262

#define TRUEWIRELESS_SET_VOLUME_REQ_ID  0x0063
#define TRUEWIRELESS_SET_VOLUME_RSP_ID  0x0263

#define AVRCP_SEND_CMD_RSP_ID           0x0260

#define BATTERY_INDICATION_REQ_ID       0x0080
#define BATTERY_INDICATION_RSP_ID       0x0280


#define REQ_COMPLETED_SUCCESSFUL        0x00        // For all response
#define REQ_INITIATED                   0x01
#define REQ_FAILED                      0x02

#define TEST_MODE_DUT                   0x00        //For  GEN_ENTER_TEST_MODE_REQ_ID
#define TEST_MODE_TX_CONT               0x01

#define HF_AUDIO_TRANSFER_TO_BT_DEVICE       0x00        //for HF_AUDIO_TRANSFER_REQ_ID
#define HF_AUDIO_TRANSFER_TO_MOBILE_PHONE    0x01

#define HP_AUDIO_MUTE                       0x00        //for HF_AUDIO_MUTE_UNMUTE_REQ_ID
#define HP_AUDIO_UNMUTE                     0x01

#define HP_CHANGE_VOLUME_UP                 0x00        //for HF_CHANGE_VOLUME_REQ_ID
#define HP_CHANGE_VOLUME_DN                 0x01

#define     AV_REMOTE_CONTROL_PLAY           0x00    //for AV_REMOTE_CONTROL_REQ_ID
#define     AV_REMOTE_CONTROL_PAUSE          0x01
#define     AV_REMOTE_CONTROL_STOP           0x02
#define     AV_REMOTE_CONTROL_FORWARD        0x03
#define     AV_REMOTE_CONTROL_BACKWARD       0x04

#define MAX_PAYLOAD_LENGTH 64

typedef enum
{
    PAIRING_SUCCESS,
    PAIRING_FAILED,
    PAIRING_ENTER,
    PAIRING_EXIT,
    PAIRING_UNKNOWN=0xff    
}ePairingStatus;

typedef enum
{
    A2DP_STATUS_CONNECTABLE_OR_DISCONNECTED,
    A2DP_STATUS_CONNECTED_PHONE_ONLY,
    A2DP_STATUS_STREAMING,
    A2DP_STATUS_PAUSED,
	A2DP_STATUS_CONNECTED_PHONE_AND_SPEAKER,
	A2DP_STATUS_CONNECTED_OTHER_SPEAKER_ONLY,
    END_A2DP_STATUS
}eA2dpStatus;


typedef enum
{
    WITRON_AVRCP_CMD_PLAY,
    WITRON_AVRCP_CMD_PAUSE,
    WITRON_AVRCP_CMD_STOP,
    WITRON_AVRCP_CMD_SKIP_FORWARD,
    WITRON_AVRCP_CMD_SKIP_BACKWARD,
    WITRON_AVRCP_CMD_FF_PRESS,
    WITRON_AVRCP_CMD_FF_RELEASE,
    WITRON_AVRCP_CMD_REWIND_PRESS,
    WITRON_AVRCP_CMD_REWIND_RELEASE,
    WITRON_AVRCP_CMD_PLAY_PAUSE,
    END_WITRON_AVRCP_CMD
}eWitronAvrcpCmd ;

typedef enum
{
	WITRON_BT_DUT_MODE,
	WITRON_BT_TX_CONT_MODE,
	WITRON_BT_PCM_LOOPBACK,
	WITRON_BT_A2DP_CODEC,
    WITRON_BT_STEREO_CODEC_LOOPBACK,
    END_WITRON_BT_TEST_MODE
}eWitronBtTestmode;

typedef enum
{
    RX_STATE_ID_LOW,
    RX_STATE_ID_HIGH,
    RX_STATE_LEN_LOW,        //len only include data len
    RX_STATE_LEN_HIGH,
    RX_STATE_PAYLOAD
}eRxParserState;

typedef struct
{
    uint16 id;
    uint16 len;
    uint8* payload;
}tBluetoothRxMessage;
