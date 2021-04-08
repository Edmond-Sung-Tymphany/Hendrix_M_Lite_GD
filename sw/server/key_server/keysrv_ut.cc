// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "KeySrv.h"
#include "./KeySrv_priv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "keyDrv.h"
#include "IrRxDrv.h"
#include "bsp.h"
#include "persistantObj.h"
#include "qf_port.c"
}

#include "keysrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>

#include "deviceTypes.h"



cKeySrv me;

typedef QState (*QStateHandler)(void * const me, QEvt const * const e);


/********************************************************/
/*********   FAKE function ***********************************/
/********************************************************/

//FAKE_VOID_FUNC(PersistantObj_RefreshTick, cPersistantObj* , uint16);
FAKE_VOID_FUNC(KeyDrv_Ctor,cKeyDrv*, const tKeyboardDevice*,void*);
FAKE_VOID_FUNC(KeyDrv_Xtor,cKeyDrv *,const tKeyboardDevice *);
FAKE_VOID_FUNC(IrRxDrv_Ctor);
FAKE_VOID_FUNC(IrRxDrv_Xtor);

FAKE_VOID_FUNC(KeyUpdateStatusCb,cKeyDrv *);

FAKE_VALUE_FUNC(uint32, getSysTime);

FAKE_VALUE_FUNC(uint16, GetServerID, QActive*);


/* List of fakes used by this unit tester */

#define FFF_FAKES_LIST(FAKE)            \
	FAKE(KeyDrv_Ctor)       \
    FAKE(KeyDrv_Xtor) 		\
    FAKE(getSysTime)        \
    FAKE(KeyUpdateStatusCb) \
    FAKE(GetServerID)		\
	FAKE(IrRxDrv_Ctor)		\
	FAKE(IrRxDrv_Xtor)


/********************************************************/
/*********   Local function ***********************************/
/********************************************************/

tGpioKeyboardDevice gpioKeyboardConfig_fake;


const tDevice* my_getDevicebyId(eDeviceID deviceID, uint16 *index)
{
    (void) deviceID;

	if(*index == 0)
    {
    	gpioKeyboardConfig_fake.gpioKeyboard.keyNum = NUM_OF_ADC_KEY;
	}
	else if(*index == 1)
    {
    	gpioKeyboardConfig_fake.gpioKeyboard.keyNum = NUM_OF_GPIO_KEY;
	}
    return (tDevice*)&gpioKeyboardConfig_fake;
}


static void InitialKey()
{
    
	QEvt evt;
	int i;
	for(i = 0; i < NUM_OF_ALL_KEY; i++)
    {
        me.pKeyObj[i]->KeyUpdateStatusCb = KeyUpdateStatusCb;
    }

	// send key up to reset the process
	me.pKeyObj[0]->keyState = KEY_UP;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
}


class KeySrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {
        getDevicebyId_fake.custom_fake = my_getDevicebyId;
        KeySrv_StartUp((cPersistantObj *)&me);
        // List of fakes we use and need to reset 
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();

    }
    virtual void TearDown()
    {
        KeySrv_ShutDown((cPersistantObj *)&me);

    }
    public:
};
    

//test the initial fuctnion 
TEST_F(KeySrvTest, InitialTest)
{    
	QEvt evt;
	evt.sig = Q_ENTRY_SIG;
    KeySrv_Initial(&me, &evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(KeySrv_DeActive));     
}

//test the state when enter DeActive state 
TEST_F(KeySrvTest, DeActiveEntryTest)
{
	QEvt evt;
	evt.sig = Q_ENTRY_SIG;
    QState ret = KeySrv_DeActive(&me, &evt);	
	// state NOT change
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(KeySrv_DeActive));  
	// make sure it return Q_RET_HANDLED
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED); 
}

//test the case that wake up server by controller active signal
TEST_F(KeySrvTest, DeActiveSystemActiveTest)
{
	QEvt evt;
	evt.sig = SYSTEM_ACTIVE_REQ_SIG;
    KeySrv_DeActive(&me, &evt);
	// make sure state change
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(KeySrv_Active));  
	// make sure it call KeyDrv_Ctor and the number is correct
    ASSERT_EQ(KeyDrv_Ctor_fake.call_count, KEYBOARD_NUMBER);
}


//test the state when enter active state 
TEST_F(KeySrvTest, ActiveEntryTest)
{

	QEvt evt;

	evt.sig = Q_ENTRY_SIG;
    KeySrv_DeActive(&me, &evt);
	
	evt.sig = SYSTEM_ACTIVE_REQ_SIG;
    KeySrv_DeActive(&me, &evt);
	
	evt.sig = Q_ENTRY_SIG;
	KeySrv_Active(&me, &evt);
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(KeySrv_Active));  
    ASSERT_EQ(getSysTime_fake.call_count, NUM_OF_ALL_KEY);
}


//test the case that put the server from active to sleep
TEST_F(KeySrvTest, ActiveSystemSleepTest)
{
	QEvt evt;
	evt.sig = SYSTEM_SLEEP_REQ_SIG;
    KeySrv_Active(&me, &evt);
	// make sure state NOT change
	ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(KeySrv_DeActive));  
    ASSERT_EQ(KeyDrv_Xtor_fake.call_count, KEYBOARD_NUMBER);
}


//Test the debounce fail case 
TEST_F(KeySrvTest, DebounceFailTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce fail 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime-1;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_NE(me.keyAnlysStage[0],KEY_DOWN);
	
	me.pKeyObj[0]->keyState = KEY_UP;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);		
	ASSERT_EQ(me.keyAnlysStage[0],KEY_UP);
}

//Test the debounce successful case 
TEST_F(KeySrvTest, DebounceSucceedTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);
}



//Test the short press case 
TEST_F(KeySrvTest, ShortPressTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// key's up, check if it's short press
	me.pKeyObj[0]->keyState = KEY_UP;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyEvtCtr[0].currEvtState,KEY_EVT_SHORT_PRESS);
}


TEST_F(KeySrvTest, LongPressTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.longPressTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// key's up, check if it's long press
	me.pKeyObj[0]->keyState = KEY_UP;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyEvtCtr[0].currEvtState,KEY_EVT_LONG_PRESS);
}



TEST_F(KeySrvTest, VLongPressTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.veryLongPressTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// key's up, check if it's very long press
	me.pKeyObj[0]->keyState = KEY_UP;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyEvtCtr[0].currEvtState,KEY_EVT_VERY_LONG_PRESS);
}



TEST_F(KeySrvTest, KeyHoldTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// check if it's hold event
	me.keyEvtCtr[0].holdEvtSent = FALSE;
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.longPressTime+1;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyEvtCtr[0].currEvtState,KEY_EVT_HOLD);
	ASSERT_EQ(me.keyEvtCtr[0].holdEvtSent,TRUE);
}

TEST_F(KeySrvTest, KeyVeryLongHoldTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// check if it's hold event
	me.keyEvtCtr[0].veryLongholdEvtSent = FALSE;
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.veryLongPressTime+1;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyEvtCtr[0].currEvtState,KEY_EVT_VERY_LONG_HOLD);
	ASSERT_EQ(me.keyEvtCtr[0].veryLongholdEvtSent,TRUE);
}


//negative test for debouncing
TEST_F(KeySrvTest, DebounceNegativeTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// when wrong state is pass in, check if it will be reset 
	me.pKeyObj[0]->keyState = KEY_INVALIDE_STATE;
	me.keyTimer[0]= me.pConfig->timing.debounceTime+1;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_UP);
}

//negative test for short press
TEST_F(KeySrvTest, ShortPressNegativeTest)
{
	InitialKey();
	QEvt evt;

	// start debounce 
	me.pKeyObj[0]->keyState = KEY_DOWN;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DEBOUNCING);

	// debounce succeed
	me.pKeyObj[0]->keyState = KEY_DOWN;
	me.keyTimer[0]= me.pConfig->timing.debounceTime;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	ASSERT_EQ(me.keyAnlysStage[0],KEY_DOWN);

	// pass the wrong parameter
	me.pKeyObj[0]->keyState = KEY_INVALIDE_STATE;
	evt.sig = KEY_TIMEOUT_SIG;	
    KeySrv_Active(&me, &evt);	
	// check if it will take it as shor press
	ASSERT_NE(me.keyEvtCtr[0].currEvtState,KEY_EVT_SHORT_PRESS);
	// check if the process reset
	ASSERT_EQ(me.keyAnlysStage[0],KEY_UP);
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
