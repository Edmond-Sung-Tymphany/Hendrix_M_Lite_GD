/**
*  @file      MainApp_keyEvtHandler.h
*  @brief     header file of Key event handler
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

#ifndef MAINAPP_BLEEVTHANDLER_H
#define	MAINAPP_BLEEVTHANDLER_H

#ifdef	__cplusplus
extern "C" {
#endif


/************************************************************************************/
#define MAX_DATA_SIZE       (SIZE_OF_LARGE_EVENTS - 8- sizeof(eSettingId) - sizeof(uint16) - sizeof(uint16))
#define SMALL_DATA_SIZE     4
#define MEDIUM_DATA_SIZE    16

/* command and audio q will be added here*/
typedef enum
{
    BT_ENTER_PAIRING_CMD,
    BT_ENTER_CONNECTABLE_CMD,
    BT_CONNECT_CMD,
    BT_PLAY_PAUSE_CMD,
    BT_RESET_PAIR_LIST_CMD,
    BT_ANSWER_CMD,
    BT_REJECT_CMD,
    BT_CANCEL_END_CMD,
    BT_TWC_ACCEPT_WAITING_HOLD_ACTIVE_CMD,
    BT_TWC_RELEASE_ALL_HELD_CMD,
    BT_AVRCP_SKIP_FORWARD_CMD,
    BT_AVRCP_SKIP_BACKWORD_CMD,
    BT_OFF_CMD,
/*********** TONE CMD ****************/
    BT_TONE_RESET_PDL_CMD,
    BT_TONE_BAT_DOCK_CMD,
    BT_TONE_AC_IN_CMD,
    BT_TONE_LIM_VOL_CMD,
    BT_TONE_PAIRING_CMD,
    BT_TONE_POWER_OFF_CMD,
    BT_TONE_CONNECTED_CMD,
    BT_TONE_PAIR_FAIL_CMD,
    BT_COMMON_MAX_CMD,
/******** PRODUCTION  TEST CMD *************/
    BT_PWR_ON_CMD,
    BT_PWR_OFF_CMD,
    BT_MAX_CMD,
}eBtCmd;


REQ_EVT(BleWriteDataReq)
    eSettingId       setting_id;     /* which setting have been updated */
    uint16           offset;
    uint16           size;
    uint8 data[MAX_DATA_SIZE];
END_REQ_EVT(BleWriteDataReq)

REQ_EVT(BleReadDataReq)
    eSettingId       setting_id;     /* which setting will be read */
    uint16           offset;
    uint16           size;
END_REQ_EVT(BleReadDataReq)

RESP_EVT(BleReadDataSmallResp)
    eSettingId       setting_id;     /* a response which could carry the data with size of SMALL_DATA_SIZE */
    uint16           offset;
    uint16           size;
    int8             data[SMALL_DATA_SIZE];
END_RESP_EVT(BleReadDataSmallResp)

RESP_EVT(BleReadDataMediumResp)
    eSettingId       setting_id;     /* a response which could carry the data with size of MEDIUM_DATA_SIZE*/
    uint16           offset;
    uint16           size;
    int8             data[MEDIUM_DATA_SIZE];
END_RESP_EVT(BleReadDataMediumResp)

RESP_EVT(BleReadDataLargeResp)
    eSettingId       setting_id;     /* a response which could carry data with the maximum size */
    uint16           offset;
    uint16           size;
    int8             data[MAX_DATA_SIZE];
END_RESP_EVT(BleReadDataLargeResp)

REQ_EVT(BleResetItemReq)
    uint8    featureId;
END_REQ_EVT(BleResetItemReq)

/********* For sync the features **********/
REQ_EVT(BleGetFeatureReq)
END_REQ_EVT(BleGetFeatureReq)

RESP_EVT(BleFeatureResp)
    uint16           numberOfFeature;
    int8             data[MAX_DATA_SIZE];
END_RESP_EVT(BleFeatureResp)

RESP_EVT(BleDfuResp)
END_RESP_EVT(BleDfuResp)

RESP_EVT(BleVersionResp)
    uint8  lenOfVer;
    char   mcuVer[10];
END_RESP_EVT(BleVersionResp)

RESP_EVT(BleProductNameResp)
    uint8  lenOfName;
    char   productName[16];
END_RESP_EVT(BleProductNameResp)

/* BT cmd event*/
REQ_EVT(BtCmdEvt)
    eBtCmd btCmd;
END_REQ_EVT(BtCmdEvt)

/* Key Functions*/
static void MainApp_BleResetPresetName(void);
QState MainApp_BleEvtHandler(cMainApp * const me, QEvt const * const e);
void MainApp_BleReadDataResp(void * pDataToSend, eSettingId settId, uint16 size, uint16 offset);

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_BLEEVTHANDLER_H */

