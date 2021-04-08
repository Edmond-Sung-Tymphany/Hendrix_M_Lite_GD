// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "./LedSrv_priv.h"
#include "LedDrv.h"
#include "PwmLedDrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "persistantObj.h"
#include "qf_port.c"
}
#include "LedSrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>

cLedSrv me;
static cPwmLedDrv pwmLedDrvList[NUM_OF_PWM_LED];
#define TIMEOUT_SIG MAX_SIG

FAKE_VOID_FUNC(LedDrv_Xtor, cLedDrv*);
FAKE_VOID_FUNC(LedDrv_Ctor,cLedDrv*,const tDevice*);
FAKE_VOID_FUNC(LedDrv_SetBrightness,cLedDrv*,uint8);
FAKE_VOID_FUNC(LedDrv_On,cLedDrv*);
FAKE_VOID_FUNC(LedDrv_Off,cLedDrv*);
FAKE_VOID_FUNC(PwmLedDrv_Ctor,cPwmLedDrv*, const tDevice*, uint8);
FAKE_VOID_FUNC(LedDrv_PattStop,cLedDrv *);
FAKE_VOID_FUNC(LedDrv_PattSet, cLedDrv *, ePattern);
FAKE_VOID_FUNC(PwmLedDrv_On,cLedDrv*);
FAKE_VOID_FUNC(PwmLedDrv_SetColor,cLedDrv*,Color);
FAKE_VOID_FUNC(PwmLedDrv_Off,cLedDrv*);
FAKE_VOID_FUNC(PwmLedDrv_Xtor,cLedDrv*);
FAKE_VALUE_FUNC(uint32, getSysTime);
FAKE_VALUE_FUNC(ePattern, LedDrv_PattShow, cLedDrv *);


/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)       \
    FAKE(LedDrv_Xtor)              \
	FAKE(LedDrv_Ctor)              \
	FAKE(LedDrv_SetBrightness)     \
	FAKE(LedDrv_Off)               \
	FAKE(getSysTime)               \
	FAKE(LedDrv_PattShow)          \
	FAKE(PwmLedDrv_Ctor)           \
	FAKE(LedDrv_PattStop)          \
	FAKE(LedDrv_PattSet)           \
	FAKE(PwmLedDrv_On)             \
	FAKE(PwmLedDrv_Off)            \
	FAKE(PwmLedDrv_SetColor)       \
	FAKE(PwmLedDrv_Xtor)           \

static const tLedFunc pwmLedDrvFunc = 
{
    .LedOn          = PwmLedDrv_On,
    .LedOff         = PwmLedDrv_Off,
    .LedSetColor    = PwmLedDrv_SetColor,
    .LedXtor        = PwmLedDrv_Xtor,
};
static void InitLed()
{
	int i;
	for(i = 0; i < LED_MAX; i++)
    {
	    pwmLedDrvList[i].super_.pLedFunc = &pwmLedDrvFunc;
	    me.ledDrvList[i] = (cLedDrv*)&pwmLedDrvList[i];
    }
}

class LedSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {
        /* List of fakes we use and need to reset */
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
    }
    virtual void TearDown()
    {
        LedSrv_ShutDown((cPersistantObj *)&me);
    }
    public:
};


TEST_F(LedSrvTest, LedSrv_Active_EntrySig)
{
    QEvt evt;
    evt.sig = Q_ENTRY_SIG;
	LedSrv_Initial(&me, &evt);
	/* Transfer to Deactive state */
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(LedSrv_DeActive));
}

TEST_F(LedSrvTest, LedSrv_Active_SysSleepReq)
{
    QEvt evt;
    evt.sig = SYSTEM_SLEEP_REQ_SIG;
	LedSrv_Active(&me, &evt);	
#if LED_SRV_HAS_SLEEP
    /* Make sure all led objs are extored */
	ASSERT_EQ(LedDrv_Xtor_fake.call_count, LED_MAX);
    /* Transfer to Deactive state */
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(LedSrv_DeActive));
#else    
    /* nothing will be called if the macro is not enable */
    ASSERT_EQ(LedDrv_Xtor_fake.call_count, 0);
    /* state does not change to sleep */
    ASSERT_NE(((QMsm *)&me)->temp.fun, Q_STATE_CAST(LedSrv_DeActive));
#endif
	
}

TEST_F(LedSrvTest, LedSrv_Active_LedReqPureOn)
{
    LedReqEvt ledReq;
	QState funRetValue;
    ledReq.super.sig = LED_REQ_SIG;
	ledReq.ledCommand = LED_PURE_ON_CMD;
	ledReq.leds = (1 << LED_RED);
	ledReq.sender = NULL;
	InitLed();
	funRetValue = LedSrv_Active(&me, (QEvt*)(&ledReq));
	/* LedDrv_PattStop excuted means set color is also excuted */
	ASSERT_EQ(LedDrv_PattStop_fake.call_count, 1);
	ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
}

TEST_F(LedSrvTest, LedSrv_Active_LedReqOff)
{
    LedReqEvt ledReq;
	QState funRetValue;
    ledReq.super.sig = LED_REQ_SIG;
	ledReq.ledCommand = LED_OFF_CMD;
	ledReq.sender = NULL;
	ledReq.leds = (1 << LED_RED);
    InitLed();
	funRetValue = LedSrv_Active(&me, (QEvt*)(&ledReq));
	/* LedDrv_PattStop excuted means LedOff is also excuted */
	ASSERT_EQ(LedDrv_PattStop_fake.call_count, 1);
	ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
}

TEST_F(LedSrvTest, LedSrv_Active_LedPattOn)
{
    LedReqEvt ledReq;
	QState funRetValue;
    /* use number 1 instead of a project pattern name */
	ledReq.patternId = (ePattern)1;
    ledReq.super.sig = LED_REQ_SIG;
	ledReq.ledCommand = LED_PAT_ON_CMD;
	ledReq.sender = NULL;
	ledReq.leds = (1 << LED_RED);
	InitLed();
	funRetValue = LedSrv_Active(&me, (QEvt*)(&ledReq));
	/* make sure pattern is set */
	ASSERT_EQ(LedDrv_PattSet_fake.call_count, 1);
	ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
}

//negative test for pass wrong LED bit mask
TEST_F(LedSrvTest, LedSrv_Active_LedPattOnNegTest)
{
    LedReqEvt ledReq;
	QState funRetValue;
    ledReq.super.sig = LED_REQ_SIG;
	ledReq.ledCommand = LED_PAT_ON_CMD;
	ledReq.leds = (1 << LED_MAX);
	ledReq.sender = NULL;
	InitLed();
	funRetValue = LedSrv_Active(&me, (QEvt*)(&ledReq));
	/* make sure pattern is set */
	ASSERT_EQ(LedDrv_PattSet_fake.call_count, 0);
	ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
}


// Step 3. Call RUN_ALL_TESTS() in main().
//
// We do this by linking in src/gtest_main.cc file, which consists of
// a main() function which calls RUN_ALL_TESTS() for us.
//
// This runs all the tests you've defined, prints the result, and
// returns 0 if successful, or 1 otherwise.
//
// Did you notice that we didn't register the tests?  The
// RUN_ALL_TESTS() macro magically knows about all the tests we
// defined.  Isn't this convenient?
