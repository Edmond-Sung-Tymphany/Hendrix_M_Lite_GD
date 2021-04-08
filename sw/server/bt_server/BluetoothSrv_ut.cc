// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "BluetoothSrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "persistantObj.h"
#include "qf.h"
#include "qf_port.c"
}
#include "BluetoothSrv_ut.h"
#include "gtest/gtest.h"

#include <string.h>


#define SETID_DSP_INIT_TEST_DATA_SIZE 48
#define TEST_SIZE                       sizeof(arrMenuData)

#define DATA_ENTRIES_TO_SYNC    4    // number of ROM/RAM data entries 

cBluetoothSrv me;

FAKE_VOID_FUNC(BluetoothDrv_Ctor, cBluetoothDrv*);
FAKE_VOID_FUNC(BluetoothDrv_RegisterTimeReqCb, cBluetoothDrv*, timeReqCb);
FAKE_VOID_FUNC(BluetoothDrv_RegisterDriverSig, cBluetoothDrv*, QActive*);
FAKE_VOID_FUNC(BluetoothDrv_TurnOnBT, cBluetoothDrv*);
FAKE_VALUE_FUNC(const void*, Setting_Get, eSettingId);
FAKE_VOID_FUNC(Setting_Set, eSettingId, const void*);
FAKE_VOID_FUNC(BluetoothDrv_TurnOffBT, cBluetoothDrv*);
FAKE_VOID_FUNC(BluetoothDrv_UnRegisterDriverSig, cBluetoothDrv*);
FAKE_VOID_FUNC(BluetoothDrv_TimeIsUp, cBluetoothDrv*);
FAKE_VOID_FUNC(BluetoothDrv_ExecuteCmd, cBluetoothDrv*, eBtCmd);
FAKE_VOID_FUNC(AudioSrv_SendMuteReq, QActive*, bool);


/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)        \
    FAKE(BluetoothDrv_Ctor)         \
    FAKE(BluetoothDrv_RegisterTimeReqCb)    \
    FAKE(BluetoothDrv_RegisterDriverSig)    \
    FAKE(Setting_Get)                       \
    FAKE(Setting_Set)                       \
    FAKE(BluetoothDrv_TurnOnBT)             \
    FAKE(BluetoothDrv_TurnOffBT)            \
    FAKE(BluetoothDrv_UnRegisterDriverSig)  \
    FAKE(BluetoothDrv_TimeIsUp)             \
    FAKE(BluetoothDrv_ExecuteCmd)           \
    FAKE(AudioSrv_SendMuteReq)           \

class BluetoothSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {
        BluetoothSrv_StartUp((cPersistantObj *)&me);
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
    }
    virtual void TearDown()
    {
        BluetoothSrv_ShutDown((cPersistantObj *)&me);
    }
    public:
};

TEST_F(BluetoothSrvTest, Initial)
{
    BluetoothSrv_StartUp((cPersistantObj *)&me);
	QEvt* evt = Q_NEW(QEvt, Q_ENTRY_SIG);
    BluetoothSrv_Initial(&me, evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(BluetoothSrv_DeActive));  
    ASSERT_EQ(BluetoothDrv_Ctor_fake.call_count, 1);
    ASSERT_EQ(BluetoothDrv_RegisterTimeReqCb_fake.call_count, 1);
    ASSERT_EQ(BluetoothDrv_RegisterDriverSig_fake.call_count, 1);
 
}
TEST_F(BluetoothSrvTest, BluetoothSrv_DeActiveTest)
{
    QEvt* evt = Q_NEW(QEvt, Q_ENTRY_SIG);

    QState ret = BluetoothSrv_DeActive(&me, evt);
    ASSERT_EQ(BluetoothDrv_UnRegisterDriverSig_fake.call_count, 1);
    ASSERT_EQ(BluetoothDrv_TurnOffBT_fake.call_count, 1);
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED);   

    evt = Q_NEW(QEvt, SYSTEM_ACTIVE_REQ_SIG);
    ret = BluetoothSrv_DeActive(&me, evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(BluetoothSrv_PreActive));  


 
}
TEST_F(BluetoothSrvTest, BluetoothSrv_PreActiveTest_BT_TIMEOUT)
{
    QEvt* evt = Q_NEW(QEvt, BT_TIMEOUT_SIG);
    me.timeCount = 3;
    BluetoothSrv_PreActive(&me, evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(BluetoothSrv_Active));  
 
}

TEST_F(BluetoothSrvTest, BluetoothSrv_Active_BTCMDDONE)
{
    QEvt* evt = Q_NEW(QEvt, BT_TIMEOUT_SIG);
    QState ret = BluetoothSrv_Active(&me, evt);
    uint8 BT_CMD_MIN_GAP_MS = 250;
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED);   

    evt = Q_NEW(QEvt, BT_CMD_DONE_SIG);
    ret = BluetoothSrv_Active(&me, evt);
    ASSERT_EQ(me.isQueueWaiting, 1);   
    ASSERT_EQ(me.cmdQueueDelayMs, BT_CMD_MIN_GAP_MS);   
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED); 


}

TEST_F(BluetoothSrvTest, BluetoothSrv_ActiveNegativeTest_BT_TONE_RESET_PDL_CMD)
{
    QState ret;
    BtCmdEvt* evt = Q_NEW(BtCmdEvt, BT_REQ_SIG); //(BtCmdEvt*) evt;


    evt->btCmd = BT_TONE_RESET_PDL_CMD;
    ret = BluetoothSrv_Active(&me, (QEvt*)evt);
    ASSERT_NE(BluetoothDrv_TurnOffBT_fake.call_count, 1);
    ASSERT_NE(BluetoothDrv_TurnOnBT_fake.call_count, 1);
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED);   

}


TEST_F(BluetoothSrvTest, BluetoothSrv_Active_BT_REQ_SIG_Test)
{
    QState ret;
    BtCmdEvt* evt = Q_NEW(BtCmdEvt, BT_REQ_SIG); //(BtCmdEvt*) evt;
    evt->btCmd = BT_PWR_ON_CMD;    
    ret = BluetoothSrv_Active(&me, (QEvt*)evt);
    ASSERT_EQ(BluetoothDrv_TurnOnBT_fake.call_count, 1);
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED);   

    evt->btCmd = BT_PWR_OFF_CMD;
    ret = BluetoothSrv_Active(&me, (QEvt*)evt);
    ASSERT_EQ(BluetoothDrv_TurnOffBT_fake.call_count, 1);
    ASSERT_EQ(ret, (QState)Q_RET_HANDLED);   
}

