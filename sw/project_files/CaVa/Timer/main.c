/*****************************************************************************
*  @file      main.c
*  @brief     Contains main and using for testing the timer
*  @author    Ones Yang
*  @date      5-April-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/
#include "stm32f0xx.h"
#include "uartdrv.h"
#include "junit_report.h"
#include "trace.h"
#include "signals.h"
#include "timer.h"
#include "bsp.h"
#include "timer.config"

/*************************************************************
********************For Timer test****************************
*************************************************************/
#define MAX_TIMER_NUM       (10)
#define DELAY_TIME_STEP     (100)
#define LOWER_LIMIT         (-10)
#define UPPER_LIMIT         (10)
#define TIMEOUT             (1000)

static uint16    glTimerId[MAX_TIMER_NUM];
static int32 timeFirstGet = 0;
static int32 timeSecondGet = 0;
static bool  timerPrecisionTestResult = false;
static bool  timerMaxTestResult = false;
static uint16 timerRegisterCNT = 0;

static void TimeoutCallback(void* pCbPara);
static void MaxTimerTestFun(uint8 Num);
static void TimeoutCallbackPrecisionTest(void* pCbPara);
static void TimerPrecisionTestFun(uint32 timeout);

static void State_Transfer_Signal_Send(void); 


static void MaxTimerTestFun(uint8 Num)
{
    for(uint8 i = 0;i < Num;i++)
    {
        if( !Timer_StartTimer((i+1)*DELAY_TIME_STEP,&glTimerId[i],TimeoutCallback,&glTimerId[i]))
        {
            if(i < NUM_OF_MAX_TIMER)
            {
                timerMaxTestResult = false;
                State_Transfer_Signal_Send();
                return;
            }
        }
        else
        {
            if(i >= NUM_OF_MAX_TIMER)
            {
                timerMaxTestResult = false;
                State_Transfer_Signal_Send();
                return;
            }
        }
    }
    timerMaxTestResult = true;
    State_Transfer_Signal_Send();
}

static void TimeoutCallback(void* pCbPara)
{
    timerRegisterCNT++;
}

static void TimerPrecisionTestFun(uint32 timeout)
{
    uint16 timerId = 0;
    timeFirstGet = getSysTime();
    Timer_StartTimer(timeout,&timerId,TimeoutCallbackPrecisionTest,&timeFirstGet);
}

static void TimeoutCallbackPrecisionTest(void* pCbPara)
{
    int32 Tem = 0;
    timeSecondGet = getSysTime();
    Tem = timeSecondGet - *((int32*) pCbPara);
    if( ((Tem - TIMEOUT) < LOWER_LIMIT) || ((Tem - TIMEOUT) > UPPER_LIMIT))
    {
        timerPrecisionTestResult = false;
    }
    else
    {
        timerPrecisionTestResult = true;
    }
   
    State_Transfer_Signal_Send();

}


/*************************************************************
********************For UART Function*************************
*************************************************************/
#define DMSG_MIN_SIZE   0x07

#define DBG_UART_TX_BUF_SIZE (((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE)* DBG_SRV_EVENT_Q_SIZE) + 1)
#define DBG_UART_RX_BUF_SIZE ((SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE) + 1)

cUartDrv debug_dev;
cRingBuf txBuf;
cRingBuf rxBuf;

static bool CaVaPcReady = FALSE;

tUARTDevice UartDevice =
{
    .deviceInfo.deviceID    = DEBUG_DEV_ID,
    .deviceInfo.deviceType  = UART_DEV_TYPE,
    .baudrate   = 115200,
    .uartId     = TP_UART_DEV_1,
    .byteSize   = TP_UART_BYTE_SIZE_8_BITS,
    .parity     = TP_UART_PARITY_NONE,
    .stopBits   = TP_UART_STOP_BIT_1,
    .dmaEnable  = FALSE,
    .interrupt.priority    = 0,
    .interrupt.subPriority = 0,
};



static data uartTxBuf[DBG_UART_TX_BUF_SIZE] = {0};
static data uartRxBuf[DBG_UART_RX_BUF_SIZE] = {0};

char startCommand[] = "COMMJUSTA";
char stopCommand[]  = "COMMJUSTO";

void uartPrint(char * stream, uint16 size)
{
    UartDrv_Write(&debug_dev, (uint8 const *) stream, size);
}

void makeReport()
{
    char * report;

    report = makeJunitXmlReport("Timer Component Test");

    uartPrint(startCommand, strlen(startCommand));
    uartPrint(report, strlen(report));
    uartPrint(stopCommand, strlen(stopCommand));
}

void uartRxCallback (void *p)
{
    if (FALSE == CaVaPcReady)
    {
        CaVaPcReady = TRUE;
    }
}
static void InitUart()
{
    InitJunitXmlReport();

    RingBuf_Ctor(&txBuf, uartTxBuf, DBG_UART_TX_BUF_SIZE);
    RingBuf_Ctor(&rxBuf, uartRxBuf, DBG_UART_RX_BUF_SIZE);

    UartDrv_Ctor(&debug_dev, &UartDevice, &txBuf, &rxBuf);

    UartDrv_RegisterRxCallback(&debug_dev, uartRxCallback);
}

void timerPrecisionTestReport(void)
{
    if (timerPrecisionTestResult)
    {
        testCasePassReport PassTestCaseReport;

        InittestCasePass(&PassTestCaseReport);
        sprintf(PassTestCaseReport.testCaseName,"Timer Precision Test");
        addPassTestCase(&PassTestCaseReport);

    }
    else
    {
        testCaseFailReport FailTestCaseReport;

        InittestCaseFail(&FailTestCaseReport);
        sprintf(FailTestCaseReport.testCaseName,"Timer Precision Test");
        sprintf(FailTestCaseReport.failMessage,"Test Fail");
        addFailTestCase(&FailTestCaseReport);

    }
 }

void MaxTimerTestReport(void)
{
    if (timerMaxTestResult)
    {
        testCasePassReport PassTestCaseReport;

        InittestCasePass(&PassTestCaseReport);
        sprintf(PassTestCaseReport.testCaseName,"Max Timer Test");
        addPassTestCase(&PassTestCaseReport);
    }
    else
    {
        testCaseFailReport FailTestCaseReport;

        InittestCaseFail(&FailTestCaseReport);
        sprintf(FailTestCaseReport.testCaseName,"Max Timer Test");
        sprintf(FailTestCaseReport.failMessage,"Test Fail");
        addFailTestCase(&FailTestCaseReport);

    }
}

/*************************************************************
*******************Something about QP*************************
*************************************************************/
/* QP event pools  */
#if (NUM_OF_SMALL_EVENTS == 0 && NUM_OF_MEDIUM_EVENTS == 0 && NUM_OF_LARGE_EVENTS == 0)
    #error "No pool is defined. A product must have at least one pool for QP events"
#endif

static QSubscrList subscriptPool[MAX_PUB_SIG];

/* storage for event pools... */
#if (NUM_OF_SMALL_EVENTS != 0 && SIZE_OF_SMALL_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_SMALL_EVENTS];} smalltype;
static QF_MPOOL_EL(smalltype) smallEventPool[NUM_OF_SMALL_EVENTS];
#elif (NUM_OF_SMALL_EVENTS != SIZE_OF_SMALL_EVENTS)
  #error "Error, Both SIZE_OF_SMALL_EVENTS and NUM_OF_SMALL_EVENTS should be defined as 0"
#endif

#if (NUM_OF_MEDIUM_EVENTS != 0 && SIZE_OF_MEDIUM_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_MEDIUM_EVENTS];} medtype;
static QF_MPOOL_EL(medtype) mediumEventPool[NUM_OF_MEDIUM_EVENTS];
#elif (NUM_OF_MEDIUM_EVENTS != SIZE_OF_MEDIUM_EVENTS)
  #error "Error, Both SIZE_OF_MEDIUM_EVENTS and NUM_OF_MEDIUM_EVENTS should be defined as 0"
#endif

#if (NUM_OF_LARGE_EVENTS != 0 && SIZE_OF_LARGE_EVENTS != 0)
typedef struct { uint8 t[SIZE_OF_LARGE_EVENTS];} largetype;
static QF_MPOOL_EL(largetype) largeEventPool[NUM_OF_LARGE_EVENTS];
#elif (NUM_OF_LARGE_EVENTS != SIZE_OF_LARGE_EVENTS)
  #error "Error, Both SIZE_OF_LARGE_EVENTS and NUM_OF_LARGE_EVENTS should be defined as 0"
#endif

#define CONTROLLER_ID 32 /* Controller needs an ID which is away from all server/app/delegate ID */


/* State declaration  */
QState Controller_Initial(QActive * const me);

QState Controller_Idle(QActive * const me, QEvt const * const e);

QState Timer_Precision_Test(QActive * const me, QEvt const * const e);

QState Timer_Max_Test(QActive * const me,QEvt const * const e);

QState Report_To_CavaPCTool(QActive * const me,QEvt const * const e);

QActive Controller; /* Controller object. Singleton */

static QEvt const *eventQue[3];
static QEQueue deferredReqQue;
static QEvt const *pDeferredReqQueSto[2];

#define NORMAL_MODE             0x0001


/*****************************************************************************************************************
 *
 * Qp initialize functions
 *
 *
 *****************************************************************************************************************/
int16 Controller_Ctor(uint16 mode_id)
{
    /* initialize the framework and the underlying RT kernel */
    QF_init();

    /* object dictionaries */
    QS_OBJ_DICTIONARY(smallEventPool);
    QS_OBJ_DICTIONARY(mediumEventPool);
    QS_OBJ_DICTIONARY(largeEventPool);
    /* init publish-subscribe */
    QF_psInit(subscriptPool, Q_DIM(subscriptPool));

    /* initialize event pools... */
#if (NUM_OF_SMALL_EVENTS != 0 && SIZE_OF_SMALL_EVENTS != 0)
    QF_poolInit(smallEventPool, sizeof(smallEventPool), sizeof(smallEventPool[0]));
#endif

#if (NUM_OF_MEDIUM_EVENTS != 0 && SIZE_OF_MEDIUM_EVENTS != 0)
    QF_poolInit(mediumEventPool, sizeof(mediumEventPool), sizeof(mediumEventPool[0]));
#endif

#if (NUM_OF_LARGE_EVENTS != 0 && SIZE_OF_LARGE_EVENTS != 0)
    QF_poolInit(largeEventPool, sizeof(largeEventPool), sizeof(largeEventPool[0]));
#endif

#if defined(HAS_DEBUGSETT) || defined(HAS_SETTING)
    SettingSrv_InitDB();
#endif
    QActive_ctor(&Controller, Q_STATE_CAST(&Controller_Initial));
    QActive_start(&Controller, CONTROLLER_ID, eventQue, Q_DIM(eventQue), (void *)0, 0U, (QEvt *)0);
    QEQueue_init(&deferredReqQue, pDeferredReqQueSto, Q_DIM(pDeferredReqQueSto));
    /* Zero expected responses */
    /* Start up other entities and do more initialising */
    QS_OBJ_DICTIONARY(&Controller);
    
    /* This is a loop so it won't actually (or shouldn't) return */
    return (int16)QF_run();
}


void SendToController(const QEvt* evt)
{
     /* Post to Server using QPPost*/
	QACTIVE_POST(&Controller, evt, 0);
}


static void State_Transfer_Signal_Send(void)
{
    QEvt* reqEvt = Q_NEW(QEvt, SYSTEM_MODE_REQ_SIG);
    SendToController((QEvt*)reqEvt);
}


/*******************************************************************************
 * State functions
 *
 ******************************************************************************/
/* Initial state */
QState Controller_Initial(QActive * const me)
{
    QS_FUN_DICTIONARY(&Controller_Initial);
    QS_FUN_DICTIONARY(&Controller_Idle);
    QS_FUN_DICTIONARY(&Timer_Precision_Test);
    QS_FUN_DICTIONARY(&Timer_Max_Test);
    QS_FUN_DICTIONARY(&Report_To_CavaPCTool);
    
    return Q_TRAN(&Timer_Precision_Test);
}

/*\brief Timer_Precision_Test state 
 *  
 */
QState Timer_Precision_Test(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            TimerPrecisionTestFun(TIMEOUT);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_REQ_SIG:
        {
            timerPrecisionTestReport();
            return Q_TRAN(&Timer_Max_Test);
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

/*\brief Timer_Max_Test state 
 *
 */
QState Timer_Max_Test(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            MaxTimerTestFun(MAX_TIMER_NUM);
            return Q_HANDLED();
        }
        case SYSTEM_MODE_REQ_SIG:
        {
            MaxTimerTestReport();
            return Q_TRAN(&Report_To_CavaPCTool);
        }
    default:    
            break;
    }
    return Q_SUPER(&QHsm_top);    
}

/*\brief Report_To_CavaPCTool state 
 *  
 */
QState Report_To_CavaPCTool(QActive * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            makeReport();
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

int main(int argc, char** argv)
{
    (void) argc;
    (void) argv;

    BSP_init();

    InitUart();
    
    while(!CaVaPcReady);
    
    return Controller_Ctor(NORMAL_MODE);
}