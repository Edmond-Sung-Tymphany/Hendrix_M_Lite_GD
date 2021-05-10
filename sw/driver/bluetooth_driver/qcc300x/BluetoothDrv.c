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

#include "stm32f0xx.h"
#include "trace.h"
#include "attacheddevices.h"
#include "./BluetoothDrv_priv.h"
#include "BluetoothDrv.config"
#include "BluetoothSrv.config"
#include "BluetoothSrv.h"
#include "bsp.h"
#ifdef HAS_POWER_CONTROL
#include "PowerDrv.h"
#endif
#ifdef HAS_SYSTEM_CONTROL
#include "SystemDrv.h"
#endif
#ifdef HAS_BT_AUDIO_CUE
#include "AudioDrv.h"
#endif
#include "SettingSrv.h"
typedef enum
{
    BT_VERSION_ES=0,
    BT_VERSION_EVT,
    BT_VERSION_DVT,
    BT_VERSION_MP,
    BT_VERSION_MAX
} BtVersion_t;

typedef struct btVersionTag
{
    BtVersion_t bt_ver;
    char btVersionStr[BT_VERSION_LENGTH];
} btVersionTag;
static const btVersionTag btVerMap[]=
{
    {BT_VERSION_ES,          "ES" },
    {BT_VERSION_EVT,         "EVT"},
    {BT_VERSION_DVT,         "DVT"},
    {BT_VERSION_MP,          "MP" },
};
static uint8 version_bt;

#ifdef HAS_FIX_VOL_AUDIO_CUE
const static uint16 bt_audio_cue_time[BT_CUE_MAX] =
{
    BT_AUDIO_CUE_PAIRING_TIME,  // BT_PAIRING_START_CUE_CMD,
    BT_AUDIO_CUE_PAIRING_SUCESS_TIME,   // BT_PAIRING_SUCCESS_CUE_CMD,
    BT_AUDIO_CUE_PAIRING_FAILED_TIME,   // BT_PAIRING_FAIL_CUE_CMD,
    BT_AUDIO_CUE_PLAY_TIME,     // BT_PLAY_CUE_CMD,
    BT_AUDIO_CUE_PAUSE_TIME,    // BT_PAUSE_CUE_CMD,
    BT_AUDIO_CUE_BACKWARD_TIME, // BT_SKIP_BACKWARDS_CUE_CMD,
    BT_AUDIO_CUE_FORWARD_TIME,  // BT_SKIP_FORWARDS_CUE_CMD,
    BT_AUDIO_CUE_BAT_CHARGING_TIME, // BT_CHARGING_CUE_CMD,
    BT_AUDIO_CUE_BAT_LOW_TIME,  // BT_BAT_LOW_CUE_CMD,
    BT_AUDIO_CUE_BAT_FULL_TIME, // BT_BAT_FULL_CUE_CMD,
    BT_AUDIO_CUE_TEST_TIME,
};
#endif

#ifdef HAS_BT_SEQ_CONTROL
const static tSeqSection bt_startup_seq[] =
{
    { &BtDrv_PwrOnStage1,    50 },
    { &BtDrv_PwrOnStage2,    50 },   // power on process
    { &BtDrv_PwrOnStage3,    BT_POWER_ON_PRESS_TIME_MS },   // power on process
    { &BtDrv_PwrOnStage4,    30 },   // power on process
};

const static tSeqSection bt_power_off_seq[] =
{
    { &BtDrv_PwrOffStage1,    30 },
    { &BtDrv_PwrOffStage2,    BT_POWER_OFF_PRESS_TIME_MS },   // power off process
    { &BtDrv_PwrOffStage3,    BT_POWER_OFF_WAITING_TIME_MS }, // wait for bt powering down
};

#endif

#define BT_DEBUG_ENABLE
#ifndef BT_DEBUG_ENABLE
#undef  TP_PRINTF
#define TP_PRINTF(...)
#endif

static cBluetoothDrv* pBluetoothDrv;
static cGpioDrv btGpioDrv;
static bool isBtInterruptEnable = FALSE;

static tGPIODevice *p_GPIOForBT=NULL;

#ifdef HAS_FIX_VOL_AUDIO_CUE
static int16 bluetooth_audio_cue_playing_time = 0;
static int16 bluetooth_delay_start_audio_cue = 0;
static bool bluetooth_audio_cue_is_pairing = FALSE;
#endif

static bool isBluetoothVersionGet = FALSE;

#ifdef BT_FASTER_CMD_STATE
#define BT_STATUS_GAP_MS    30
const static uint16_t BT_status_timing[]=
{
    (BT_STATUS_GAP_MS + 50),
    (BT_STATUS_GAP_MS + 100),
    (BT_STATUS_GAP_MS + 150),
    (BT_STATUS_GAP_MS + 200),
    (BT_STATUS_GAP_MS + 250),
    (BT_STATUS_GAP_MS + 300),
    (BT_STATUS_GAP_MS + 350),
    (BT_STATUS_GAP_MS + 400),
    (BT_STATUS_GAP_MS + 450),
    (BT_STATUS_GAP_MS + 500),
    (BT_STATUS_GAP_MS + 550),
    (BT_STATUS_GAP_MS + 600),
    (BT_STATUS_GAP_MS + 650),
    (BT_STATUS_GAP_MS + 700),
    (BT_STATUS_GAP_MS + 750),
    (BT_STATUS_GAP_MS + 800),
};

const static uint16_t BT_status_timing_size = (sizeof(BT_status_timing)/sizeof(BT_status_timing[0]));
#endif

/* Private functions / variables. Declare and drivers here */
/* internal use for excuting the BT comand*/
const tBtGpioPressConfig GPIO_PRESS_CONFIG[BT_MAX_PRESS] =
{
    {1,  SHORT_PRESS_TIME_MS,          0                           },  //BT_SINGLE_PRESS
    {1,  LONG_PRESS_TIME_MS,           0                           },  //BT_LONG_PRESS
    {1,  VERY_LONG_PRESS_TIME_MS,      0                           },  //BT_VLONG_PRESS
    {1,  VERYVERY_LONG_PRESS_TIME_MS,  0                           },  //BT_VVLONG_PRESS
    {3,  DOUBLE_PRESS_TIME_MS,         DOUBLE_PRESS_INTERAL_TIME_MS},  //BT_DOUBLE_PRESS
    {5,  TRIPLE_PRESS_TIME_MS,         TRIPLE_PRESS_INTERAL_TIME_MS},  //BT_DOUBLE_PRESS
    {1,  0,                            0                           },  //BT_HOLD_HIGH
    {1,  0,                            0                           },  //BT_HOLD_LOW
};

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void BluetoothDrv_Ctor(cBluetoothDrv *me)
{
    p_GPIOForBT = (tGPIODevice *) getDevicebyIdAndType(BT_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(p_GPIOForBT);

    /* Fill me in! */
    GpioDrv_Ctor(&btGpioDrv, p_GPIOForBT);


    me->pTimeReq = NULL;
#ifdef HAS_BT_SEQ_CONTROL
    me->startup_step = 0;
    me->poweroff_step = 0;
#endif

    pBluetoothDrv = me;
}

void BluetoothDrv_Xtor(cBluetoothDrv *me)
{
    // never mind the XTor, it is useless on tymphany platform
    (void)me;
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
#ifdef HAS_FIX_VOL_AUDIO_CUE
void BluetoothDrv_AudioCueHold(uint16 delay_time)
{
    bluetooth_audio_cue_playing_time += delay_time;
}

void BluetoothDrv_SetDelayStartAudioCue(uint16 delay_time)
{
    if(pBluetoothDrv->cmd < BT_CUE_MIN_CMD || pBluetoothDrv->cmd > BT_CUE_MAX_CMD)
    {
        return;
    }
    if (bluetooth_audio_cue_is_pairing == TRUE)
    {
        bluetooth_delay_start_audio_cue = AUDIO_CUE_START_DELAY_TIME_FOR_PAIRING;
    }
    else
    {
        bluetooth_delay_start_audio_cue = delay_time;
    }
}

bool BluetoothDrv_CheckAudioCuePlaying(uint16 interval)
{
    bool ret = FALSE;

    if (bluetooth_audio_cue_playing_time <= INVALID_VALUE)
    {
        // ignore
    }
    else if (bluetooth_audio_cue_playing_time <= interval)
    {
        bluetooth_audio_cue_playing_time = INVALID_VALUE;
        ret = TRUE;
    }
    else
    {
        bluetooth_audio_cue_playing_time -= interval;
    }

    return (ret);
}


bool BluetoothDrv_DelayStartAudioCue(uint16 interval)
{
    bool ret = FALSE;

    if (bluetooth_delay_start_audio_cue <= INVALID_VALUE)
    {
        // ignore
    }
    else if (bluetooth_delay_start_audio_cue <= interval)
    {
        bluetooth_delay_start_audio_cue = INVALID_VALUE;
        ret = TRUE;
    }
    else
    {
        bluetooth_delay_start_audio_cue -= interval;
    }

    return (ret);
}
#endif

void BluetoothDrv_PowerEnable(bool enable)
{
    if (enable)
    {

    }
    else
    {

    }
}

#ifdef HAS_BT_RESET_PIN
static void BluetoothDrv_ResetEnable(bool enable)
{
    if (enable)
    {
        BT_RESET_ENABLE(btGpioDrv);
    }
    else
    {
        BT_RESET_DISABLE(btGpioDrv);
    }
}
#endif

/* same as "enter pairing" command but used as disconnect BT before power off if BT already connected */
void BluetoothDrv_DisconnectBT(void)
{
    tBtGpioPressConfig* pPressConfig = (tBtGpioPressConfig*)&GPIO_PRESS_CONFIG[BT_CMD_CONFIG[BT_ENTER_PAIRING_CMD].pressType];
    SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[BT_ENTER_PAIRING_CMD].gpio);
    BSP_BlockingDelayMs(pPressConfig->onTime);
    CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[BT_ENTER_PAIRING_CMD].gpio);
}


void BluetoothDrv_BT_PWR_PIO_ON(void)
{
    SET_GPIO(&btGpioDrv, BT_POWER_PIN_ID);   // power pin id define at project config
}
void BluetoothDrv_BT_PWR_PIO_OFF(void)
{
    CLEAR_GPIO(&btGpioDrv, BT_POWER_PIN_ID); // power pin id define at project config
}


void BluetoothDrv_TurnOnBT(cBluetoothDrv *me)
{
    /*initial and turn on the LED interrupt*/
    EXTI_Config(TRUE);
}

void BluetoothDrv_TurnOffBT(cBluetoothDrv *me)
{
    uint8_t i=0;

    me->step = 0;
    for(; i<p_GPIOForBT->usedGPIOPinNum; i++)
    {
        /* clear the output GPIO status */
        if(p_GPIOForBT->pGPIOPinSet[i].gpioDirection == GPIO_DIGITAL_OUTPUT)
        {
            CLEAR_GPIO(&btGpioDrv, p_GPIOForBT->pGPIOPinSet[i].gpioId);
        }
    }

    /*enable BT power*/

    /*turn off the LED interrupt*/
    EXTI_Config(FALSE);
}

void BluetoothDrv_ExecuteCmd(cBluetoothDrv *me, eBtCmd cmd)
{
    tBtGpioPressConfig* pPressConfig = (tBtGpioPressConfig*)
                                       &GPIO_PRESS_CONFIG[BT_CMD_CONFIG[cmd].pressType];
    me->cmd = cmd;
    if(me->step!=INITIAL_STEP)
    {
        TP_PRINTF("start exe command but step is not initial\r\n");
        ASSERT(0);
    }
    me->step = INITIAL_STEP;
    switch(BT_CMD_CONFIG[cmd].pressType)
    {
        case BT_HOLD_HIGH:
            SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[cmd].gpio);
            BluetoothDrv_SendCmdDoneToServer(pBluetoothDrv,me->cmd);
            break;
        case BT_HOLD_LOW:
            CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[cmd].gpio);
            BluetoothDrv_SendCmdDoneToServer(pBluetoothDrv,me->cmd);
            break;
        default:
            SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[cmd].gpio);
            //    TP_PRINTF("start exe cmd %d, the step is %d\r\n", cmd, me->step);

            break;
    }
    me->pTimeReq(BT_SET_TIME, pPressConfig->onTime);
}

void BluetoothDrv_TimeIsUp(cBluetoothDrv *me)
{
    tBtGpioPressConfig* pPressConfig = (tBtGpioPressConfig*)
                                       &GPIO_PRESS_CONFIG[BT_CMD_CONFIG[me->cmd].pressType];
    me->step++;
//    TP_PRINTF("time is up for cmd %d, the step is %d\r\n", me->cmd, me->step);
    if(me->step == pPressConfig->maxStep)
    {
        /* if reaching max step, clear GPIO and set delayTime to 0*/
        me->step = INITIAL_STEP;
        CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);
        BluetoothDrv_SendCmdDoneToServer(pBluetoothDrv,me->cmd);
    }
    else if ((me->step%2) == 0)
    {
        /* if even number step, then set GPIO on*/
        SET_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);
        me->pTimeReq(BT_SET_TIME, pPressConfig->onTime);
    }
    else
    {
        /* if odd number step, then clear GPIO, for double press type*/
        CLEAR_GPIO(&btGpioDrv, BT_CMD_CONFIG[me->cmd].gpio);
        me->pTimeReq(BT_SET_TIME, pPressConfig->internalTime);
    }
}

void BluetoothDrv_RegisterTimeReqCb(cBluetoothDrv* me, timeReqCb fCb)
{
    ASSERT(me);
    ASSERT(fCb);
    /* clent can reset the callback if it needs */
    me->pTimeReq = fCb;
}

void BluetoothDrv_RegisterDriverSig(cBluetoothDrv* me, QActive* req)
{
    ASSERT(me);
    ASSERT(req);
    /* clent can reset the receiver if it needs */
    me->pRequester = req;
}

void BluetoothDrv_UnRegisterDriverSig(cBluetoothDrv* me)
{
    ASSERT(me);
    me->pRequester = NULL;
}

void BluetoothDrv_Led0_IRQ_Handler(void)
{
    if( isBtInterruptEnable )
        BluetoothDrv_ReadBtLedStatus(BT_INPUT0);
}

void BluetoothDrv_Led1_IRQ_Handler(void)
{
    if( isBtInterruptEnable )
        BluetoothDrv_ReadBtLedStatus(BT_INPUT1);
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
/* initial the pin interrupt for PA0*/
static void EXTI_Config(bool isTurnOnInterrupt)
{
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Enable SYSCFG clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    /* Connect EXTI14 Line to PB14 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource14);
    /* Connect EXTI15 Line to PB15 pin */
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource15);
    EXTI_InitStructure.EXTI_Line = EXTI_Line14 | EXTI_Line15;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    if(isTurnOnInterrupt)
    {
        isBtInterruptEnable = TRUE;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    }
    else
    {
        isBtInterruptEnable = FALSE;
        EXTI_InitStructure.EXTI_LineCmd = DISABLE;
    }
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI4_15_IRQn;
    //NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void BluetoothDrv_ReadBtLedStatus(eGPIOId btInputId)
{
    if(GpioDrv_ReadBit(&btGpioDrv, btInputId))
    {
        /* if input 0, then it is rising edge */
        if(pBluetoothDrv)
        {
            uint32 pulseTime, bt_status;
            pulseTime = pBluetoothDrv->pTimeReq(BT_END_TIME, 0);
#ifdef BT_FASTER_CMD_STATE
            bt_status = BluetoothDrv_GetBTModuleStatus(pulseTime, btInputId);
            if( bt_status != BT_MAX_STA )
                BluetoothDrv_SendStatusToSever(pBluetoothDrv, bt_status);
#else
            if(pulseTime>=BT_STATE_START_TIME_MS)
            {
                /* only send status when the pusle time is valid*/
                BluetoothDrv_SendStatusToSever(pBluetoothDrv,
                                               BluetoothDrv_GetBTModuleStatus(pulseTime, btInputId));
            }
#endif
        }
    }
    else
    {
        /* if falling edge */
        if(pBluetoothDrv)
        {
            pBluetoothDrv->pTimeReq(BT_START_TIME, 0);
        }
    }
}

static uint8 BluetoothDrv_GetBTModuleStatus(uint32 pulseTime, eGPIOId ledId)
{
#ifdef BT_FASTER_CMD_STATE
    uint8_t bt_status=BT_MAX_STA;
    uint16_t i;

    if( pulseTime < BT_STATUS_GAP_MS )  // rubbish, ignore it
        return bt_status;

    for(i=0; i<BT_status_timing_size; i++)
    {
        if( pulseTime < BT_status_timing[i] )
            break;
    }

    if( i >= BT_MAX_STA )
        return bt_status;

    bt_status = i;
    if( BT_INPUT1 == ledId )    // event LED(LED1) from BT module
    {
        bt_status += (BT_MAX_STA + 1);
    }

    TP_PRINTF("\n\r LED[%d] = %dms, bt status = %d\n\r", (ledId-BT_INPUT0), pulseTime, bt_status);

    return bt_status;
#else
    uint8 status = BT_MAX_EVT;
    uint8 i = (pulseTime + BT_STATE_STEP_OFFSET_TIME_MS) / BT_STATE_STEP_TIME_MS;
    if(ledId == BT_INPUT0)
    {

        TP_PRINTF("BT_INPUT0: get the status, and pulseTime is %d\r\n", pulseTime);
        if(i<BtLed0StaMaxNum)
        {
            status = tBtLed0StatusMap[i];
        }
        else
        {
            status = BT_MAX_LED0_STA;
        }

    }
    else if(ledId == BT_INPUT1)
    {

        TP_PRINTF("BT_INPUT1: get the status, and pulseTime is %d\r\n", pulseTime);
        if(i<BtLed1StaMaxNum)
        {
            status = tBtLed1StatusMap[i];
        }
        else
        {
            status = BT_MAX_EVT;
        }
    }
    return status;
#endif
}

static void BluetoothDrv_GetVersionHendle(cBluetoothDrv *me)
{

    if(isBluetoothVersionGet)
    {
        version_bt+=1;
        Setting_Set(SETID_BT_VER, &btVerMap[version_bt].btVersionStr[0]);
    }
    else
    {
        isBluetoothVersionGet = TRUE;
        version_bt = BT_VERSION_EVT;        // version detect start from EVT
        Setting_Set(SETID_BT_VER, &btVerMap[version_bt].btVersionStr[0]);
    }
}

static void BluetoothDrv_SendStatusToSever(cBluetoothDrv *me, uint8 btStatus)
{
    /*Flitering the version state for BT version detect*/
    if(btStatus == BT_GET_VERION_STA)
    {
        BluetoothDrv_GetVersionHendle(me);
        return;
    }

    if(me->pRequester)
    {
        BtDrvStatusEvt* pEvt = Q_NEW(BtDrvStatusEvt, BT_STATUS_SIG);
        pEvt->btStatus = btStatus;
        QACTIVE_POST(me->pRequester,(QEvt*)pEvt,0);
    }
}

static void BluetoothDrv_SendCmdDoneToServer(cBluetoothDrv *me, eBtCmd cmd)
{
    if(me->pRequester)
    {
        BtDrvCmdEvt* pEvt = Q_NEW(BtDrvCmdEvt, BT_CMD_DONE_SIG);
        pEvt->cmd = cmd;
        QACTIVE_POST(me->pRequester,(QEvt*)pEvt,0);
        // do something after BT command executed.
        switch( pEvt->cmd )
        {
            case BT_RESET_PAIR_LIST_CMD:
#ifdef SYSTEM_RESTART_AFTER_FACTORY_RESET
                // To Do:  System restart delay 1000ms
                // unit : MS, wait the BT module finish.
#endif
                break;

            case BT_PAIRING_START_CUE_CMD:
            case BT_PAIRING_SUCCESS_CUE_CMD:
            case BT_PAIRING_FAIL_CUE_CMD:
            case BT_PLAY_CUE_CMD:
            case BT_PAUSE_CUE_CMD:
            case BT_SKIP_BACKWARDS_CUE_CMD:
            case BT_SKIP_FORWARDS_CUE_CMD:
            case BT_CHARGING_CUE_CMD:
            case BT_BAT_LOW_CUE_CMD:
            case BT_BAT_FULL_CUE_CMD:
            case BT_TEST_CUE_CMD:
#ifdef HAS_FIX_VOL_AUDIO_CUE
                bluetooth_audio_cue_is_pairing = FALSE;
                BluetoothDrv_AudioCueHold(bt_audio_cue_time[(pEvt->cmd - BT_CUE_MIN_CMD)]);
#endif
                break;

            default:
#ifdef HAS_FIX_VOL_AUDIO_CUE
                bluetooth_audio_cue_is_pairing = TRUE;
#endif
                break;
        }
    }
}


#ifdef HAS_BT_SEQ_CONTROL


static void BtDrv_PwrOnStage1(void* me)
{
    isBluetoothVersionGet = FALSE;
    BluetoothDrv_TurnOnBT(pBluetoothDrv);

    BluetoothDrv_ResetEnable(TRUE);

    BluetoothDrv_BT_PWR_PIO_OFF();
}

static void BtDrv_PwrOnStage2(void* me)
{
    BluetoothDrv_ResetEnable(FALSE);
}

static void BtDrv_PwrOnStage3(void* me)
{
    BluetoothDrv_BT_PWR_PIO_ON();
}

static void BtDrv_PwrOnStage4(void* me)
{
    BluetoothDrv_BT_PWR_PIO_OFF();
}


static void BtDrv_PwrOffStage1(void* me)
{
    BluetoothDrv_BT_PWR_PIO_OFF();
}

static void BtDrv_PwrOffStage2(void* me)
{
    BluetoothDrv_BT_PWR_PIO_ON();
}

static void BtDrv_PwrOffStage3(void* me)
{
    BluetoothDrv_BT_PWR_PIO_OFF();
}

int32_t BluetoothDrv_StartupInit(cBluetoothDrv* me)
{
    int32_t delay_time, total_steps;
#ifdef HAS_SYSTEM_CONTROL
    if( POWER_STAGE_POWER_OFF == SystemDrv_GetPowerStage() )    // if power is not ready, waiting...
        return BT_SRV_PER_TICK_TIME_MS;
#endif

    total_steps = sizeof(bt_startup_seq) / sizeof(bt_startup_seq[0]);
    if(me->startup_step < total_steps)
    {
        /* call the power up function according to step*/
        bt_startup_seq[me->startup_step].pfSect(me);
        /* get delay time*/
        delay_time = bt_startup_seq[me->startup_step].seqDelay;
        me->startup_step ++;
    }
    else
    {
        // initial done
        me->startup_step = 0;
        delay_time = 0;
    }

#ifdef HAS_SYSTEM_CONTROL
    if( delay_time == 0 )
        SystemDrv_NextPowerStage();
#endif

    return delay_time;
}


int32_t BluetoothDrv_PowerOffInit(cBluetoothDrv* me)
{
    int32_t delay_time, total_steps;

    total_steps = sizeof(bt_power_off_seq) / sizeof(bt_power_off_seq[0]);
    if(me->poweroff_step < total_steps)
    {
        /* call the power off function according to step*/
        bt_power_off_seq[me->poweroff_step].pfSect(me);
        /* get delay time*/
        delay_time = bt_power_off_seq[me->poweroff_step].seqDelay;
        me->poweroff_step ++;
    }
    else
    {
        // initial done
        me->poweroff_step = 0;
        delay_time = 0;
    }


    return delay_time;
}
#endif
