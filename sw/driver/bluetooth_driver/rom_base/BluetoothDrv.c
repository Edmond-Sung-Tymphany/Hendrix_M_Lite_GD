/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Bluetooth Driver
                  -------------------------

                  SW Module Document




@file        BluetoothDrv.c
@brief       It's the bluetooth driver to control the ROM base CSR module by toggling GPIO
@author      Johnny Fan
@date        2014-05-13
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-05-13     Johnny Fan
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "GpioDrv.h"
#include "bsp.h"
#include "timer.h"
#include "BluetoothDrv_priv.h"
#include "BluetoothDrv.config"
#include "trace.h"

#ifdef BT_DEBUG_ENABLE
#define BT_DRV_PRINT(x) printf x
#else
#define BT_DRV_PRINT(...)
#endif

#define BT_NO_DEADLOOP_TIMEOUT (-1)
#define BT_INVALID_TIMER       (0xFFFF)
#define BT_STATE_MASK_NONE     (0xFFFFFFFF)
#define BT_STATE_MASK_ALL      (0x00000000)

static cBluetoothDrv* pBluetoothDrv = NULL;
static cGpioDrv btGpioDrv;

static int8  glReconnectCount    = 0;
static uint16 glPulseRelaxTimerId = BT_INVALID_TIMER;
static uint16 glDebounceTimerId   = BT_INVALID_TIMER;
static uint16 glReconnectTimerId  = BT_INVALID_TIMER;
static bool  glStateStable       = FALSE;
static bool  glBtIsBusy          = FALSE;
static eBtStatus glStateToSend   = BT_MAX_STA;
static eBtStatus glBtStatus      = BT_OFF_STA;

static btDrvReportCallback cbReportCb = NULL;

s_bt_cmd_queue byCmdQueue[BT_CMD_QUEUE_COUNT];

/* Private functions / variables. Declare and drivers here */
/* internal use for excuting the BT comand*/

/*****************************************************************************************************************
 *
 *  Interrupt functions
 *
 *****************************************************************************************************************/


/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
static uint32 startDeb   = 0;
static uint32 stopDeb    = 0;
static uint32 startRelax = 0;
static uint32 stopRelax  = 0;
static uint32 startRec   = 0;
static uint32 stopRec    = 0;
static uint32 startPulse = 0;
static uint32 stopPulse  = 0;

#define WAIT_B4_OFF_MS (2000 - (getSysTime() - stopRelax) <= 0) ? 0 : (2000 - (getSysTime() - stopRelax))

void btCmdPulseTmrCb(void* pCbPara)
{
    stopPulse = getSysTime();

    BT_DRV_PRINT(("\nbt_drv: stop_pulse [time:%d]", stopPulse));
    btCmdTimeUp(pBluetoothDrv);
}

void btCmdRelaxTmrCb(void* pCbPara)
{
    stopRelax = getSysTime();

    BT_DRV_PRINT(("\nbt_drv: stop_relax[time:%d]",stopRelax));

    if (FALSE == btCmdQueueGo())
    {
        BluetoothDrv_SendCmdDoneToServer(pBluetoothDrv,pBluetoothDrv->cmd);
    }
}

void btCmdReconnectTmrCb(void* pCbPara)
{
    stopRec = getSysTime();

    BT_DRV_PRINT(("\nbt_drv: stop_rec[time:%d]",stopRec));

    glReconnectTimerId = BT_INVALID_TIMER;

    if (--glReconnectCount > 0)
    {
        if ((BT_CONNECTED_STA != BluetoothDrv_GetBtStatus(pBluetoothDrv))
         && (BT_STREAMING_A2DP_STA!= BluetoothDrv_GetBtStatus(pBluetoothDrv)))
        {
            btCmdStartRecTmr(RECCONNECT_ATTEMPT_PERIOD_MS);
            btCmdQueueAdd(BT_CONNECT_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
        }
        else
        {
            glReconnectCount = 0;
        }
    }
    else
    {
        BluetoothDrv_SendStatusToSever(pBluetoothDrv, BT_CONNECTABLE_STA);
    }
}

void btStatDebTmrCb(void* pCbPara)
{
    stopDeb = getSysTime();

    BT_DRV_PRINT(("\nbt_drv: stat_stop_deb [time:%d]",stopDeb));
    if (BT_MAX_STA != glStateToSend)
    {
        if (TRUE == glStateStable)
        {
            BluetoothDrv_SendStatusToSever(pBluetoothDrv, glStateToSend);
            glStateToSend = BT_MAX_STA;
            glStateStable = FALSE;
        }
        else
        {
            glStateStable = TRUE;
            btStatStartDebTmr(BT_DRIVER_DEBOUNCE_TIME_MS);
        }
    }
    else
    {
        ASSERT(0);
    }
}

void btCmdStartPulseTmr(uint32 timeOut)
{
    startPulse = getSysTime();

    if (FALSE == Timer_StartTimer(timeOut, &glPulseRelaxTimerId, btCmdPulseTmrCb, NULL))
    {
        ASSERT(0);
    }

    BT_DRV_PRINT(("\nbt_drv: start_pulse[time:%d][id:%d]",startPulse,glPulseRelaxTimerId));
}

void btCmdStartRelaxTmr(uint32 timeOut)
{
    startRelax = getSysTime();
    if (FALSE == Timer_StartTimer(timeOut, &glPulseRelaxTimerId, btCmdRelaxTmrCb, NULL))
    {
        ASSERT(0);
    }

    BT_DRV_PRINT(("\nbt_drv: start_relax[timeout:%d][time:%d][id:%d]",timeOut,startRelax,glPulseRelaxTimerId));
}

void btCmdStartRecTmr(uint32 timeOut)
{
    startRec = getSysTime();

    if (FALSE == Timer_StartTimer(timeOut, &glReconnectTimerId, btCmdReconnectTmrCb, NULL))
    {
        ASSERT(0);
    }

    BT_DRV_PRINT(("\nbt_drv: start_rec[time:%d][id:%d]",startRec,glReconnectTimerId));
}

void btStatStartDebTmr(uint32 timeOut)
{
    startDeb = getSysTime();

    if (FALSE == Timer_StartTimer(timeOut, &glDebounceTimerId, btStatDebTmrCb, NULL))
    {
        ASSERT(0);
    }

    BT_DRV_PRINT(("\nbt_drv: stat_start_deb[time:%d][id:%d]",startDeb,glDebounceTimerId));
}

void btStopReconnection()
{
    if (BT_INVALID_TIMER != glReconnectTimerId)
    {
        Timer_StopTimer(glReconnectTimerId);
        glReconnectCount = 0;
        glReconnectTimerId = BT_INVALID_TIMER;
    }
}

void btCmdQueueAdd(eBtCmd cmd, uint32 btStateMask, uint32 deadLoopTimeoOut)
{
    uint8 ii;


    for (ii = 0; ii < BT_CMD_QUEUE_COUNT; ii++)
    {
        if (BT_MAX_CMD == byCmdQueue[ii].cmd)
        {
            byCmdQueue[ii].cmd             = cmd;
            byCmdQueue[ii].btStateMask     = btStateMask;
            byCmdQueue[ii].deadLoopTimeOut = deadLoopTimeoOut;
            break;
        }
    }

    if (BT_CMD_QUEUE_COUNT == ii)
    {
        ASSERT(0);
    }
}

void btCmdGo(cBluetoothDrv *me, eBtCmd cmd)
{
    if (cmd < BT_COMMON_MAX_CMD)
    {
        tBtGpioPressConfig* pPressConfig = (tBtGpioPressConfig*)&GPIO_PRESS_CONFIG[BT_CMD_CONFIG[cmd].pressType];

        if (cmd != BT_WAIT_CMD)
            me->cmd = cmd;
        if(me->step != INITIAL_STEP)
        {
            ASSERT(0);
        }
        me->step = INITIAL_STEP;
        SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[cmd].gpio);

        btCmdStartPulseTmr(pPressConfig->onTime);
    }
    else
    {   if (cmd != BT_WAIT_CMD)
            me->cmd = cmd;
        switch (cmd)
        {
            case BT_PWR_ON_CMD:
            {
                EXTI_Config(TRUE);
                BT_PWR_ENABLE(btGpioDrv);
                btCmdStartRelaxTmr(3000);
                break;
            }
            case BT_PWR_OFF_CMD:
            {
                BT_PWR_DISABLE(btGpioDrv);
                EXTI_Config(FALSE);
                btCmdStartRelaxTmr(1000);
                break;
            }
            case BT_WAIT_CMD:
            {
                btCmdStartRelaxTmr(BT_MINIMAL_RELAX_TIME_MS);
                break;
            }
            default:
                break;
        }
    }
}

void btCmdTimeUp(cBluetoothDrv *me)
{
    tBtGpioPressConfig* pPressConfig = (tBtGpioPressConfig*) &GPIO_PRESS_CONFIG[BT_CMD_CONFIG[me->cmd].pressType];
    me->step++;
    if(me->step == pPressConfig->maxStep)
    { /* if reaching max step, clear GPIO*/
        me->step = INITIAL_STEP;
        CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);

        btCmdStartRelaxTmr(BT_CMD_CONFIG[me->cmd].busyPeriodAfter);
    }
    else if ((me->step%2) == 0)
    { /* if even number step, then set GPIO on*/
        SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);
        btCmdStartPulseTmr(pPressConfig->onTime);
    }
    else
    { /* if odd number step, then clear GPIO, for double press type*/
        CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);
        btCmdStartPulseTmr(pPressConfig->internalTime);
    }
}

bool btCmdQueueGo()
{
    uint8 ii;

    for (ii = 0; ii < BT_CMD_QUEUE_COUNT; ii++)
    {
        if (BT_MAX_CMD != byCmdQueue[ii].cmd)
        {
            if (((byCmdQueue[ii].btStateMask & BluetoothDrv_GetBtStatus(pBluetoothDrv)) > 0)
              || (byCmdQueue[ii].deadLoopTimeOut <= 0))
            {
                btCmdGo(pBluetoothDrv, byCmdQueue[ii].cmd);
                byCmdQueue[ii].cmd = BT_MAX_CMD;
            }
            else
            {
                byCmdQueue[ii].deadLoopTimeOut -= BT_WAIT_STATE_INTERVAL_MS;
                btCmdStartRelaxTmr(BT_WAIT_STATE_INTERVAL_MS);
            }
            return TRUE;
        }
    }
    return FALSE;
}

void BluetoothDrv_Ctor(cBluetoothDrv *me)
{
    /* Fill me in! */
    tGPIODevice *pBtGPIOConf;
    pBtGPIOConf = (tGPIODevice*)getDevicebyId(BT_DEV_ID,NULL);
    GpioDrv_Ctor(&btGpioDrv,pBtGPIOConf);

    uint8 ii;

    for(ii = 0; ii<pBtGPIOConf->usedGPIOPinNum; ii++)
    {/* initial the output GPIO status */
        if(pBtGPIOConf->pGPIOPinSet[ii].gpioDirection == GPIO_DIGITAL_OUTPUT)
        {
            CLEAR_GPIO(&btGpioDrv, pBtGPIOConf->pGPIOPinSet[ii].gpioId);
        }
    }

    me->step = INITIAL_STEP;
    pBluetoothDrv = me;

    for (ii = 0; ii < BT_CMD_QUEUE_COUNT; ii++)
    {
        byCmdQueue[ii].cmd = BT_MAX_CMD;
        byCmdQueue[ii].btStateMask = BT_STATE_MASK_NONE;
        byCmdQueue[ii].deadLoopTimeOut = BT_NO_DEADLOOP_TIMEOUT;
    }
}

void BluetoothDrv_Xtor(cBluetoothDrv *me)
{
    /* Fill me in! */
    me->step = 0;
    uint8 i=0;
    for(;i<btGpioDrv.gpioConfig->usedGPIOPinNum;i++)
    {/* clear the output GPIO status */
        if(btGpioDrv.gpioConfig->pGPIOPinSet[i].gpioDirection == GPIO_DIGITAL_OUTPUT)
        {
            CLEAR_GPIO(&btGpioDrv, btGpioDrv.gpioConfig->pGPIOPinSet[i].gpioId);
        }
    }
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
eBtErrorStatus BluetoothDrv_BtCmd_GoDiscoverable(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);
    btStopReconnection();

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            btCmdQueueAdd(BT_PWR_ON_CMD,        BT_STATE_MASK_NONE,  BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD, BT_STATE_MASK_NONE,  BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,          BT_DISCOVERABLE_STA, 1000);
            btCmdQueueGo();
            break;
        case BT_CONNECTABLE_STA:
        case BT_RECONNECTING_STA:
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,          BT_DISCOVERABLE_STA, 1000);
            btCmdQueueGo();
            break;
        case BT_DISCOVERABLE_STA:
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,          BT_DISCOVERABLE_STA, 1000);
            btCmdQueueGo();
            break;
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
        default:
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,          BT_DISCOVERABLE_STA, 1000);
            btCmdQueueGo();
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}
eBtErrorStatus BluetoothDrv_BtCmd_GoConnectable(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);
    btStopReconnection();

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            btCmdQueueAdd(BT_PWR_ON_CMD,            BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
            break;
        case BT_CONNECTABLE_STA:
        case BT_RECONNECTING_STA:
            me->cmd = BT_ENTER_CONNECTABLE_CMD;
            btCmdStartRelaxTmr(BT_MINIMAL_RELAX_TIME_MS);
            break;
        case BT_DISCOVERABLE_STA:
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,              BT_CONNECTABLE_STA, 1000);
            btCmdQueueGo();
            break;
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD,     BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_DISCOVERABLE_STA, 1000);
            btCmdQueueAdd(BT_WAIT_CMD, BT_CONNECTABLE_STA, 1000);
            btCmdQueueGo();
            break;
        default:
            ASSERT(0);
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}
eBtErrorStatus BluetoothDrv_BtCmd_GoReconnect(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            btCmdQueueAdd(BT_PWR_ON_CMD,        BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_CONNECT_CMD,       BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
            glReconnectCount = RECCONNECT_ATTEMPT;
            btCmdStartRecTmr(RECCONNECT_ATTEMPT_PERIOD_MS);
            break;
        case BT_CONNECTABLE_STA:
            btCmdQueueAdd(BT_CONNECT_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();

            glReconnectCount = RECCONNECT_ATTEMPT;
            btCmdStartRecTmr(RECCONNECT_ATTEMPT_PERIOD_MS);
            break;
        case BT_RECONNECTING_STA:
            me->cmd = BT_CONNECT_CMD;
            btCmdStartRelaxTmr(BT_MINIMAL_RELAX_TIME_MS);
            break;
        case BT_DISCOVERABLE_STA:
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_CONNECT_CMD,           BT_CONNECTABLE_STA, 1000);
            btCmdQueueGo();

            glReconnectCount = RECCONNECT_ATTEMPT;
            btCmdStartRecTmr(RECCONNECT_ATTEMPT_PERIOD_MS);
            break;
        case BT_STREAMING_A2DP_STA:
        case BT_CONNECTED_STA:
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD,     BT_STATE_MASK_NONE,  BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_DISCOVERABLE_STA, 1000);
            btCmdQueueAdd(BT_CONNECT_CMD,           BT_CONNECTABLE_STA,  1000);
            btCmdQueueGo();

            glReconnectCount = RECCONNECT_ATTEMPT;
            btCmdStartRecTmr(RECCONNECT_ATTEMPT_PERIOD_MS);
            break;
        default:
            ASSERT(0);
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}
eBtErrorStatus BluetoothDrv_BtCmd_ResetPDL(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);
    btStopReconnection();

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            btCmdQueueAdd(BT_PWR_ON_CMD,          BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_RESET_PAIR_LIST_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
            break;
        case BT_CONNECTABLE_STA:
        case BT_RECONNECTING_STA:
        case BT_DISCOVERABLE_STA:
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
            btCmdQueueAdd(BT_RESET_PAIR_LIST_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
            break;
        default:
            ASSERT(0);
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}
eBtErrorStatus BluetoothDrv_BtCmd_PowerOn(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);
    btStopReconnection();

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            btCmdQueueAdd(BT_PWR_ON_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueGo();
            break;
        case BT_CONNECTABLE_STA:
        case BT_RECONNECTING_STA:
        case BT_DISCOVERABLE_STA:
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
            me->cmd = BT_PWR_ON_CMD;
            btCmdStartRelaxTmr(BT_MINIMAL_RELAX_TIME_MS);
            break;
        default:
            ASSERT(0);
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}
eBtErrorStatus BluetoothDrv_BtCmd_PowerOff(cBluetoothDrv *me)
{
    btSetIsBusy(pBluetoothDrv, TRUE);
    btStopReconnection();

    eBtStatus BtState = BluetoothDrv_GetBtStatus(me);

    switch (BtState)
    {
        case BT_OFF_STA:
            me->cmd = BT_PWR_OFF_CMD;
            btCmdStartRelaxTmr(BT_MINIMAL_RELAX_TIME_MS);
            break;
        case BT_LINK_LOSS_RECONNECT:
        case BT_CONNECTABLE_STA:
        case BT_RECONNECTING_STA:
            btCmdQueueAdd(BT_PWR_OFF_CMD, BT_STATE_MASK_ALL, WAIT_B4_OFF_MS);
            btCmdQueueGo();
            break;
        case BT_DISCOVERABLE_STA:
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,              BT_CONNECTABLE_STA, 3000);
            btCmdQueueAdd(BT_PWR_OFF_CMD,           BT_STATE_MASK_ALL,  WAIT_B4_OFF_MS);
            btCmdQueueGo();
            break;
        case BT_CONNECTED_STA:
        case BT_STREAMING_A2DP_STA:
            btCmdQueueAdd(BT_ENTER_PAIRING_CMD,     BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_ENTER_CONNECTABLE_CMD, BT_STATE_MASK_NONE, BT_NO_DEADLOOP_TIMEOUT);
            btCmdQueueAdd(BT_WAIT_CMD,              BT_CONNECTABLE_STA, 3000);
            btCmdQueueAdd(BT_PWR_OFF_CMD,           BT_STATE_MASK_ALL,  WAIT_B4_OFF_MS );
            btCmdQueueGo();
            break;
        default:
            ASSERT(0);
            break;
    }
    return BT_ERROR_STATUS_SUCCESS;
}

void BluetoothDrv_RegisterReportCallback(cBluetoothDrv *me, btDrvReportCallback cb)
{
    cbReportCb = cb;
}

eBtStatus BluetoothDrv_GetBtStatus(cBluetoothDrv* me)
{
    return glBtStatus;
}
bool BluetoothDrv_IsBusy(cBluetoothDrv* me)
{
    return glBtIsBusy;
}
void btSetBtStatus(cBluetoothDrv* me, eBtStatus status)
{
    glBtStatus = status;
}
void btSetIsBusy(cBluetoothDrv* me, bool state)
{
    glBtIsBusy = state;
}

void BluetoothDrv_DebounceSend(eBtStatus status)
{
    if (BT_MAX_STA != glStateToSend)
    {
        glStateToSend = status;
        glStateStable = FALSE;
    }
    else
    {
        glStateToSend = status;
        glStateStable = TRUE;

        btStatStartDebTmr(BT_DRIVER_DEBOUNCE_TIME_MS);
    }
}

void BluetoothDrv_ReadBtLinkLossStatus()
{
    if(pBluetoothDrv)
    {
        if(GpioDrv_ReadBit(&btGpioDrv, BT_LINK_LOSS))
        {
            BluetoothDrv_SendStatusToSever(pBluetoothDrv, BT_LINK_LOSS_RECONNECT);
        }
        else
        {
            BluetoothDrv_SendStatusToSever(pBluetoothDrv, BT_LINK_LOSS_RECONNECT_OFF);
        }
    }
}

void BluetoothDrv_ReadBtLedsStatus()
{
    uint8 led0 = 0;
    uint8 led1 = 0;
    static eBtDrvStatusLinesState statusCode = BT_DRV_STATUS_LINES_STATE_0;
    static eBtStatus status = BT_MAX_STA;

    if(pBluetoothDrv)
    {
        if(GpioDrv_ReadBit(&btGpioDrv, BT_LED0))
        {
            led0 = 1;
        }
        else
        {
            led0 = 0;
        }
        if(GpioDrv_ReadBit(&btGpioDrv, BT_LED1))
        {
            led1 = 1;
        }
        else
        {
            led1 = 0;
        }
#ifdef DONT_SEND_REPEAT_STATE
        if (((led0 << 1) + led1) != statusCode)
#endif
        {
            statusCode = (eBtDrvStatusLinesState)((led0 << 1) + led1);

            switch (statusCode)
            {
                case BT_DRV_STATUS_LINES_STATE_0:
                {
                    status = BT_STREAMING_A2DP_STA;
                    break;
                }
                case BT_DRV_STATUS_LINES_STATE_1:
                {
                    status = BT_DISCOVERABLE_STA;
                    break;
                }
                case BT_DRV_STATUS_LINES_STATE_2:
                {
                    status = BT_CONNECTED_STA;
                    break;
                }
                case BT_DRV_STATUS_LINES_STATE_3:
                {
                    status = BT_CONNECTABLE_STA;
                    break;
                }
            }
            BluetoothDrv_DebounceSend(status);
        }
    }
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/


static void BluetoothDrv_SendStatusToSever(cBluetoothDrv *me, eBtStatus btStatus)
{
    switch (btStatus)
    {
        case BT_STREAMING_A2DP_STA:
        case BT_DISCOVERABLE_STA:
        case BT_CONNECTED_STA:
        {
            if (BT_RECONNECTING_STA == BluetoothDrv_GetBtStatus(pBluetoothDrv))
            {
                btStopReconnection();
            }
            break;
        }
        case BT_CONNECTABLE_STA:
            break;
    }

    btSetBtStatus(pBluetoothDrv, btStatus);
    BT_DRV_PRINT(("\nbt_drv: report_status[%d]", btStatus));
    cbReportCb(BT_REPORT_TYPE_STATUS_IND, btStatus);
}

static void BluetoothDrv_SendCmdDoneToServer(cBluetoothDrv *me, eBtCmd cmd)
{
    switch(cmd)
    {
        case BT_CONNECT_CMD:
            btSetBtStatus(pBluetoothDrv,BT_RECONNECTING_STA);
            break;
        case BT_PWR_ON_CMD:
            btSetBtStatus(pBluetoothDrv,BT_CONNECTABLE_STA);
            break;
        case BT_PWR_OFF_CMD:
            btSetBtStatus(pBluetoothDrv,BT_OFF_STA);
            break;
        default:
            break;
    }

    btSetIsBusy(pBluetoothDrv,FALSE);

    cbReportCb(BT_REPORT_TYPE_COMMAND_RESP, cmd);
    BT_DRV_PRINT(("\nbt_drv: report_cmd_done[%d]", cmd));
    cbReportCb(BT_REPORT_TYPE_STATUS_IND, BluetoothDrv_GetBtStatus(pBluetoothDrv));
    BT_DRV_PRINT(("\nbt_drv: report_status[%d]", BluetoothDrv_GetBtStatus(pBluetoothDrv)));
}

