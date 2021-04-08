// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "DebugSrv.h"
#include "UartDrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "KeySrv.h"
}
#include "gtest/gtest.h"
#include <string.h>

#include "fff.h"


DEFINE_FFF_GLOBALS;

//cUartDrv* me, const tUARTDevice* pConfig, cRingBuf* pTx, cRingBuf* pRx
FAKE_VOID_FUNC(UartDrv_Ctor, cUartDrv*, const tUARTDevice *, cRingBuf *, cRingBuf *);
FAKE_VOID_FUNC(UartDrv_Xtor, cUartDrv*);
//uint32          UartDrv_Write(cUartDrv* me, const uint8 *p, uint32 size);
FAKE_VALUE_FUNC(uint32,UartDrv_Write, cUartDrv*, const uint8*,uint32);
FAKE_VALUE_FUNC(uint32, UartDrv_RegisterRxCallback, cUartDrv*, uartRxCb);

/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)            \
	FAKE(UartDrv_Ctor)					\
	FAKE(UartDrv_Xtor)					\
	FAKE(UartDrv_Write)					\
	FAKE(UartDrv_RegisterRxCallback)	\


cDebugSrv me;
uartRxCb * debugRXCb;
				

class DebugSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp() 
    {	
		DebugSrv_StartUp((cPersistantObj *)&me);
		/* List of fakes we use and need to reset */
		FFF_FAKES_LIST(RESET_FAKE);
		FFF_RESET_HISTORY();	
        
    }
    virtual void TearDown() 
    {
		DebugSrv_ShutDown((cPersistantObj *)&me);
		
    }
    public:
};

TEST_F(DebugSrvTest, basicWakeUp)
{
	CommonReqEvt* Req = Q_NEW(CommonReqEvt, SYSTEM_ACTIVE_REQ_SIG);
	SEND_EVT((QActive*)&me, Req);
	ASSERT_EQ(UartDrv_Ctor_fake.call_count, 1);
	KeyStateEvt* keyReq = Q_NEW(KeyStateEvt, KEY_STATE_SIG);
	keyReq->keyId = VOLUME_DOWN_KEY;
    keyReq->keyEvent = KEY_EVT_SHORT_PRESS;

	SEND_EVT((QActive*)&me, keyReq);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 1);

}

TEST_F(DebugSrvTest, basicSleep)
{
	
	CommonReqEvt* Req = Q_NEW(CommonReqEvt, SYSTEM_ACTIVE_REQ_SIG);
	SEND_EVT((QActive*)&me, Req);
	ASSERT_EQ(UartDrv_Ctor_fake.call_count, 1);
	KeyStateEvt* keyReq = Q_NEW(KeyStateEvt, KEY_STATE_SIG);
	keyReq->keyId = VOLUME_DOWN_KEY;
    keyReq->keyEvent = KEY_EVT_LONG_PRESS;

	SEND_EVT((QActive*)&me, keyReq);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 1);
	
	
	Req = Q_NEW(CommonReqEvt,SYSTEM_SLEEP_REQ_SIG);
	SEND_EVT((QActive*)&me, Req);

}

/* TEST_F(DebugSrvTest, basicUARTComms)
{

	tDbgSnkyMsgEvt * msgHeader;
	
	CommonReqEvt* Req = Q_NEW(CommonReqEvt, SYSTEM_ACTIVE_REQ_SIG);
	SEND_EVT((QActive*)&me, Req);
	ASSERT_EQ(UartDrv_Ctor_fake.call_count, 1);
	debugRXCb = UartDrv_RegisterRxCallback_fake.arg1_val;
	
	KeyDebugReqEvt keyEvt;
	keyEvt.req = DEBUG_KEY_EVT_SIMULATION;
    keyEvt.keyEvent = KEY_EVT_REPEAT;
    keyEvt.keyId = VOLUME_UP_KEY;
	msgHeader = malloc(sizeof(tDbgSnkyMsgEvt) + sizeof(KeyDebugReqEvt) + 2);

	memcpy(msgHeader + sizeof(tDbgSnkyMsgEvt),(uint8*)&keyEvt,sizeof(KeyDebugReqEvt));
	
	uint16 crc = crc16( msgHeader, sizeof(tDbgSnkyMsgEvt) + sizeof(KeyDebugReqEvt) );
	memcpy(msgHeader + sizeof(tDbgSnkyMsgEvt) + sizeof(KeyDebugReqEvt), (uint8*)&crc,2);
	
	debugRXCb((void *)msgHeader);

} */

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
