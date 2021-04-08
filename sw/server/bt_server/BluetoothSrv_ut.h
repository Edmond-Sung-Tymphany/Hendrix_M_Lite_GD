typedef enum
{
    BT_START_TIME,  /* start calculating*/
    BT_END_TIME,    /* end calculating*/
    BT_SET_TIME,    /* set a time */
    BT_MAX_TIME,
}eTimeType;

enum InternalSignals
{
    BT_TIMEOUT_SIG = MAX_SIG,
};


typedef uint32(*timeReqCb)(eTimeType reqTimeType, int32 timeInMs);

CLASS(cBluetoothDrv)   
    /* private data */
    uint8 step;
    eBtCmd cmd;
    QActive* pRequester;
    timeReqCb pTimeReq;
METHODS

typedef enum
{
    SETID_BATT_INFO = 0,
    SETID_VOLUME,        // 1
    SETID_AC_STATUS,     // 2
    SETID_ALLPLAY_INFO,  // 3
    SETID_DSP_INIT_DATA,
    SETID_DSP_TUNABLE_PART,
    SETID_CHANNEL,
    SETID_IS_AUXIN_PLUG_IN,
    SETID_MAX_VOLUME,
    SETID_POWER_MODE,
    SETID_BT_STATUS,
    SETID_CALLING_STATUS,
    SETID_MUSIC_DET,
    SETID_MAX
}eSettingId;


