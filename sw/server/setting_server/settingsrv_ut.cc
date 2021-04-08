// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "NvmDrv.h"
#include "SettingSrv.h"
#include "./SettingSrv_priv.h"
#include "./SettingSrv.config"
#include "StorageDrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "persistantObj.h"

#include "qf_port.c"
}
#include "settingsrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>

#define SETID_DSP_INIT_TEST_DATA_SIZE 48
#define TEST_SIZE                       sizeof(arrMenuData)

#define DATA_ENTRIES_TO_SYNC    4    // number of ROM/RAM data entries

cSettingSrv me;

FAKE_VOID_FUNC(StorageDrv_Ctor, cStorageDrv*, const tStorageDevice*);

FAKE_VALUE_FUNC(bool, Dummy_WriteWords, cStorageDrv*, uint32, uint8*, uint32);
FAKE_VALUE_FUNC(bool, Dummy_ReadWords, cStorageDrv*, uint32, uint8*, uint32);
FAKE_VALUE_FUNC(bool, Dummy_ErasePage, cStorageDrv*, uint32);


FAKE_VALUE_FUNC(uint16, GetServerID, QActive*);


/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)            \
    FAKE(StorageDrv_Ctor)       \
    FAKE(Dummy_WriteWords)      \
    FAKE(Dummy_ReadWords)       \
    FAKE(Dummy_ErasePage)        \
    FAKE(GetServerID) 

class SettingSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {
         nvmDrv.super_.SetValue = Dummy_WriteWords;
         nvmDrv.super_.GetValue = Dummy_ReadWords;
         nvmDrv.super_.ErasePage = Dummy_ErasePage;

        SettingSrv_StartUp((cPersistantObj *)&me);
        /* List of fakes we use and need to reset */
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
        SettingSrv_InitDB();
    }
    virtual void TearDown()
    {
        SettingSrv_ShutDown((cPersistantObj *)&me);

    }
    public:
};


/* do simulate set setting and do get seeting after and compare results
* first direct write access (using Setting_Set())
* second through SETTING_xxx_REQ_SIG(s)
*/

TEST_F(SettingSrvTest, SetGetSetting)
{
    uint8 test_menu_data[TEST_SIZE];
    uint8 resp_test_menu_data[TEST_SIZE];
    const void* ptr;
    for (int i=0; i < TEST_SIZE; i++)
    {
        test_menu_data[i] = i;
        resp_test_menu_data[i] = 0x00; // reset second check buffer
    }

    Setting_Set(SETID_MENU_DATA, &test_menu_data[0]);

    ptr = Setting_Get(SETID_MENU_DATA);
    memcpy(&resp_test_menu_data[0], ptr, TEST_SIZE);

    ASSERT_EQ(strcmp((const char *)&test_menu_data[0], (const char *)&resp_test_menu_data[0]), 0);

    for (int i = 0; i < TEST_SIZE; i++)
    {
        resp_test_menu_data[i] = 0x00; // reset second check buffer
    }
    Setting_Set(SETID_MENU_DATA, &resp_test_menu_data[0]); // fill test memory with 0x00

    SettingStartReqEvt Req;
    Req.direction = SETTING_DIRECTION_DOWNLOAD;
    Req.id = SETID_MENU_DATA;
    Req.sender = NULL;
    Req.size = TEST_SIZE;
    Req.super.sig = SETTING_START_REQ_SIG;
    SettingSrv_Active(&me, (QEvt*)&Req);

    uint8 unFullChunks = TEST_SIZE / SETTING_CHUNK_SIZE;

    for (int i=0; i < unFullChunks; i++)
    {
        /*_____SETTING_DATA_REQ_SIG___________________________________________*/
        SettingDataReqEvt Req1; 
        Req1.sender = NULL;
        memcpy(&Req1.data[0], &test_menu_data[i*SETTING_CHUNK_SIZE], SETTING_CHUNK_SIZE);
        SettingSrv_BusyWrite(&me, (QEvt*)&Req1);
    }
    /*_____SETTING_END_REQ_SIG_____________________________________________*/
    SettingEndReqEvt Req2;
    Req2.sender = NULL;
    Req2.rest_size = TEST_SIZE - (unFullChunks*SETTING_CHUNK_SIZE);
    memcpy(&Req2.data[0], &test_menu_data[unFullChunks*SETTING_CHUNK_SIZE], Req2.rest_size);
    SettingSrv_BusyWrite(&me, (QEvt*)&Req2);
    

    ptr = Setting_Get(SETID_MENU_DATA);
    memcpy(&resp_test_menu_data[0], ptr, TEST_SIZE); // get setting into second check buffer

    ASSERT_EQ(strcmp((const char *)&test_menu_data[0], (const char *)&resp_test_menu_data[0]), 0);
}


/* simulate switching to SettingSrv_BusyWrite state */
TEST_F(SettingSrvTest, switchToBusyWrite)
{
    
    /*_____SETTING_START_REQ_SIG___________________________________________*/
    SettingStartReqEvt Req;
    Req.direction = SETTING_DIRECTION_DOWNLOAD;
    Req.id = SETID_DSP_INIT_DATA;
    Req.sender = NULL;
    Req.size = SETID_DSP_INIT_TEST_DATA_SIZE;
    Req.super.sig = SETTING_START_REQ_SIG;
    QState state = SettingSrv_Active(&me, (QEvt*)&Req);
    ASSERT_EQ((QState)Q_RET_TRAN, state);   // transition should happen
    /*_________________________________________________________*/
    
       ASSERT_EQ(((QMsm *)&me)->temp.fun ,Q_STATE_CAST(SettingSrv_BusyWrite)); // are we switched into SettingSrv_BusyWrite
    /*_________________________________________________________*/

    QEvt Req0;
    Req0.sig = Q_ENTRY_SIG;  // simulate Q_TRAN into the busy write state
    SettingSrv_BusyWrite(&me, &Req0);

    ASSERT_EQ(Dummy_ErasePage_fake.call_count, 1);
    ASSERT_EQ(me.size, SETID_DSP_INIT_TEST_DATA_SIZE);

    /*_____SETTING_DATA_REQ_SIG___________________________________________*/
    SettingDataReqEvt Req1;
    Req1.sender = NULL;
    Req1.super.sig = SETTING_DATA_REQ_SIG;
    // SEND_EVT((QActive*)&me, pReq1);
    SettingSrv_BusyWrite(&me, (QEvt*)&Req1);

    ASSERT_EQ(Dummy_WriteWords_fake.call_count, 1);
    /*_____SETTING_END_REQ_SIG_____________________________________________*/
    SettingEndReqEvt Req2;
    Req2.sender = NULL;
    Req2.rest_size = 8;
    Req2.super.sig = SETTING_END_REQ_SIG;
    SettingSrv_BusyWrite(&me, (QEvt*)&Req1);

    ASSERT_EQ(Dummy_WriteWords_fake.call_count, 2);
}
/* simulate switching to SettingSrv_BusyRead state */
TEST_F(SettingSrvTest, switchToBusyRead)
{

     /*_____SETTING_START_REQ_SIG___________________________________________*/
    SettingStartReqEvt Req;
    Req.direction = SETTING_DIRECTION_UPLOAD;
    Req.id = SETID_DSP_INIT_DATA;
    Req.sender = (QActive*)&me;
    Req.size = 48; // ignored as we'll fake the read from mcu flash
    Req.super.sig = SETTING_START_REQ_SIG;
    QState state = SettingSrv_Active(&me, (QEvt*)&Req);
    
    ASSERT_EQ((QState)Q_RET_TRAN, state);   // transition should happen
    /*_________________________________________________________*/
    
       ASSERT_EQ(((QMsm *)&me)->temp.fun ,Q_STATE_CAST(SettingSrv_BusyRead)); // are we switched into SettingSrv_BusyWrite
    /*_________________________________________________________*/

    
    QEvt Req0;
    Req0.sig = Q_ENTRY_SIG;  // simulate Q_TRAN into the busy read state
    SettingSrv_BusyRead(&me, &Req0);

    ASSERT_EQ(Dummy_ReadWords_fake.call_count, 1);

    /*_____SETTING_DATA_RESP_SIG___________________________________________*/
    SettingDataRespEvt Resp;
    Resp.evtReturn = RET_SUCCESS;
    Resp.super.sig = SETTING_DATA_RESP_SIG;
    SettingSrv_BusyRead(&me, (QEvt*)&Resp);
    
    ASSERT_EQ(Dummy_ReadWords_fake.call_count, 2);

    for (int i=0; i < 3; i++)
    {
        SettingDataRespEvt Resp1;
        Resp1.evtReturn = RET_FAIL;
        Resp1.super.sig = SETTING_DATA_RESP_SIG;
       SettingSrv_BusyRead(&me, (QEvt*)&Resp1);
        
        ASSERT_EQ(Dummy_ReadWords_fake.call_count, 3 + i);
    }
}

/* negative test, simulate switching to SettingSrv_BusyWrite state
* with passing wrong id(data entry which is only localted in RAM)
*/
TEST_F(SettingSrvTest, switchToBusyWriteNegativeTest)
{
    /*_____SETTING_START_REQ_SIG___________________________________________*/
    SettingStartReqEvt Req;
    Req.direction = SETTING_DIRECTION_DOWNLOAD;
    Req.id = SETID_BATT_INFO; // invalid setting id; this data entry should not be saved in rom
    Req.sender = NULL;
    Req.size = SETID_DSP_INIT_TEST_DATA_SIZE;
    Req.super.sig  = SETTING_START_REQ_SIG;
    QState state = SettingSrv_Active(&me, (QEvt*)&Req);

    ASSERT_EQ(Dummy_ErasePage_fake.call_count, 0);
    ASSERT_NE(me.size, SETID_DSP_INIT_TEST_DATA_SIZE);  // we should not expect this size be equal
    ASSERT_NE((QState)Q_RET_TRAN, state);  // transition should not happen
    ASSERT_NE(((QMsm *)&me)->temp.fun ,Q_STATE_CAST(SettingSrv_BusyWrite)); // we should not switched into SettingSrv_BusyWrite, arent't we?
}



