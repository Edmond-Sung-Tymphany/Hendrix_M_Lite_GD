// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "NvmDrv.h"
#include "DebugSettSrv.h"
#include "./DebugSettSrv_priv.h"
#include "./DebugSettSrv.config"
#include "UartDrv.h"
#include "StorageDrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "KeySrv.h"
#include "BluetoothSrv.h"
#include "persistantObj.h"
#include "qf_port.h"
#include "qf_port.c"
}
#include "debugsettsrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>
FAKE_VOID_FUNC(UartDrv_Ctor, cUartDrv*, const tUARTDevice *, cRingBuf *, cRingBuf *);
FAKE_VOID_FUNC(UartDrv_Xtor, cUartDrv*);
FAKE_VOID_FUNC(CommonEvtResp,QActive* , QActive*, eEvtReturn, eSignal);
FAKE_VOID_FUNC(SendToServer, uint16, const QEvt*);
FAKE_VOID_FUNC(StorageDrv_Ctor, cStorageDrv*, const tStorageDevice*);

FAKE_VALUE_FUNC(uint32,UartDrv_Write, cUartDrv*, const uint8*,uint32);
FAKE_VALUE_FUNC(uint32, UartDrv_RegisterRxCallback, cUartDrv*, uartRxCb);

#ifdef ENABLE_WAKEUP_BY_UART
FAKE_VALUE_FUNC(uint32, UartDrv_RegisterWakeUpCallback, cUartDrv*, uartWakeUpCb);
#endif
#ifdef ENABLE_WAKEUP_BY_UART
FAKE_VOID_FUNC(UartDrv_EnableWakeUp, cUartDrv* );
#endif



/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)            \
	FAKE(UartDrv_Ctor)					\
	FAKE(UartDrv_Xtor)					\
	FAKE(CommonEvtResp)                 \
	FAKE(UartDrv_Write)					\
	FAKE(SendToServer)                  \
	FAKE(UartDrv_RegisterRxCallback)	\
    FAKE(UartDrv_RegisterWakeUpCallback) \
    FAKE(UartDrv_EnableWakeUp) \
    FAKE(StorageDrv_Ctor)	


cDebugSettSrv me;
uartRxCb * debugRXCb;

//_____________________________________________________
uint16  Dummy_WriteWords_called;
static bool Dummy_WriteWords(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes)
{
    Dummy_WriteWords_called++;
    return TRUE;
}
uint16  Dummy_ReadWords_called;
static bool Dummy_ReadWords(cStorageDrv *me, uint32 addr, uint8* pBuf, uint32 sizeInBytes)
{
    Dummy_ReadWords_called++;
    return TRUE;
}
uint16  Dummy_ErasePage_called;
static bool Dummy_ErasePage(cStorageDrv *me, uint32 addr)
{
    Dummy_ErasePage_called++;
    return TRUE;
}
			

class DebugSettSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp() 
    {	
        nvmDrv.super_.SetValue = Dummy_WriteWords;
        nvmDrv.super_.GetValue = Dummy_ReadWords;
        nvmDrv.super_.ErasePage = Dummy_ErasePage;

        DebugSettSrv_StartUp((cPersistantObj *)&me);
        /* List of fakes we use and need to reset */
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
    }
    virtual void TearDown() 
    {
        DebugSettSrv_ShutDown((cPersistantObj *)&me);
    }
    public:
};


TEST_F(DebugSettSrvTest, basicSleep)
{
    DebugSettSrv_StartUp((cPersistantObj *)&me);
    ASSERT_EQ(UartDrv_Ctor_fake.call_count, 1);
    ASSERT_EQ(UartDrv_RegisterRxCallback_fake.call_count, 1);
    ASSERT_EQ(UartDrv_RegisterWakeUpCallback_fake.call_count, 1);
}

TEST_F(DebugSettSrvTest, ignoreSigInSleep)
{
    KeyStateEvt keyReq;
	keyReq.keyId = VOLUME_DOWN_KEY;
    keyReq.keyEvent = KEY_EVT_LONG_PRESS;
	SEND_EVT((QActive*)&me, &keyReq);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 0);

    BtStatusEvt pe;
    pe.btStatus = BT_TEST_MODE_STA;
    SEND_EVT((QActive*)&me, &pe);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 0);
    
}

TEST_F(DebugSettSrvTest, handleSigInReady)
{
	CommonReqEvt Req;
	SEND_EVT((QActive*)&me, &Req);

    KeyStateEvt keyReq;
	keyReq.keyId = VOLUME_DOWN_KEY;
    keyReq.keyEvent = KEY_EVT_LONG_PRESS;
	SEND_EVT((QActive*)&me, &keyReq);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 1);
    
    BtStatusEvt pe;
    pe.btStatus = BT_TEST_MODE_STA;
    SEND_EVT((QActive*)&me, &pe);
	ASSERT_EQ(UartDrv_Write_fake.call_count, 2);
    
}

TEST_F(DebugSettSrvTest, basicDebugPrint)
{
	CommonReqEvt Req;
	SEND_EVT((QActive*)&me, &Req);

    DebugPrintEvt print_msg;
    print_msg.sender = NULL;
    SEND_EVT((QActive*)&me, &print_msg);
    ASSERT_EQ(UartDrv_Write_fake.call_count, 1);
}

TEST_F(DebugSettSrvTest, switchToBusyWrite)
{
	CommonReqEvt Req0;
	SEND_EVT((QActive*)&me, &Req0);
    
    Dummy_ErasePage_called = 0;

    /*_____SETTING_START_REQ_SIG___________________________________________*/
    SettingStartReqEvt req;
    req.direction = SETTING_DIRECTION_DOWNLOAD;
    req.settingId = SETID_DSP_INIT_DATA;
    req.sender = NULL;
    req.size = 48;
    SEND_EVT((QActive*)&me, &req);

    ASSERT_EQ(Dummy_ErasePage_called, 1);
    ASSERT_EQ(me.size, 48); 
    /*_____SETTING_DATA_REQ_SIG___________________________________________*/
    Dummy_WriteWords_called = 0;
    SettingDataReqEvt req1;
    req1.sender = NULL;
    SEND_EVT((QActive*)&me, &req1);

    ASSERT_EQ(Dummy_WriteWords_called, 1);
    /*_____SETTING_END_REQ_SIG_____________________________________________*/
    SettingEndReqEvt req2;
    req2.sender = NULL;
    req2.rest_size = 8;
    SEND_EVT((QActive*)&me, &req2);
   
    ASSERT_EQ(Dummy_WriteWords_called, 2);
}

TEST_F(DebugSettSrvTest, switchToBusyRead)
{
	CommonReqEvt Req0;
	SEND_EVT((QActive*)&me, &Req0);

    Dummy_ReadWords_called = 0;
     /*_____SETTING_START_REQ_SIG___________________________________________*/
    SettingStartReqEvt req;
    req.direction = SETTING_DIRECTION_UPLOAD;
    req.settingId = SETID_DSP_INIT_DATA;
    req.sender = (QActive*)&me;
    req.size = 48; // ignored as we'll fake the read from mcu flash
    SEND_EVT((QActive*)&me, &req);
	
    ASSERT_EQ(Dummy_ReadWords_called, 1);

    /*_____SETTING_DATA_RESP_SIG___________________________________________*/
    SettingDataRespEvt resp;
    resp.evtReturn = RET_SUCCESS;
    SEND_EVT((QActive*)&me, &resp);
    ASSERT_EQ(Dummy_ReadWords_called, 2);
	
	
    for (int i=0; i < 3; i++)
    {
        SettingDataRespEvt pesp;
        resp.evtReturn = RET_FAIL;  
        SEND_EVT((QActive*)&me, &resp);
        ASSERT_EQ(Dummy_ReadWords_called, 3 + i); 
    }
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
