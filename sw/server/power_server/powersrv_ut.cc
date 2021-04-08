// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "PowerSrv.h"
#include "./PowerSrv_priv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "PowerDrv.h"
#include "bsp.h"
#include "persistantObj.h"
#include "qf_port.c"
}

#include "powersrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>

#include "deviceTypes.h"

cPowerSrv me;

typedef QState (*QStateHandler)(void * const me, QEvt const * const e);

/********************************************************/
/*********   FAKE function ***********************************/
/********************************************************/
FAKE_VOID_FUNC(PowerDrv_Ctor, cPowerDrv *);
FAKE_VOID_FUNC(PowerDrv_Xtor, cPowerDrv *);
FAKE_VALUE_FUNC(uint16, PowerDrv_InitialPower, cPowerDrv *);
FAKE_VOID_FUNC(PowerDrv_DeinitialPower, cPowerDrv *);
FAKE_VALUE_FUNC(int32, PowerDrv_EjectExtBattery, cPowerDrv *);
FAKE_VOID_FUNC(PowerDrv_HandleExtBattDetInterrupt);
FAKE_VOID_FUNC(PowerDrv_RegisterIntEvent, QActive*);
FAKE_VOID_FUNC(PowerDrv_UnRegisterIntEvent);
FAKE_VOID_FUNC(PowerDrv_SetExtraCommand, cPowerDrv *, eExtraCommand);
FAKE_VOID_FUNC(PowerDrv_SetInputSource, cPowerDrv *, eInputSource);
FAKE_VOID_FUNC(PowerDrv_GetInputSourceState, cPowerDrv *, tInputSourceState*);
FAKE_VALUE_FUNC(bool,PowerDrv_GetBatteryVol,cPowerDrv *, tBatteryVol*);
FAKE_VOID_FUNC(PowerDrv_SetBatterySource, cPowerDrv *, eBatterySource);
FAKE_VOID_FUNC(PowerDrv_PowerSaveSleep);
FAKE_VOID_FUNC(PowerDrv_HandleDcDetInterrupt);
FAKE_VOID_FUNC(BSP_FeedWatchdog);
FAKE_VOID_FUNC(Setting_Set, eSettingId, const void*);
FAKE_VALUE_FUNC(const void*, Setting_Get, eSettingId);

FAKE_VALUE_FUNC(uint32, getSysTime);
FAKE_VALUE_FUNC(uint16, GetServerID, QActive*);


/* List of fakes used by this unit tester */

#define FFF_FAKES_LIST(FAKE)            \
    FAKE(PowerDrv_Ctor)        \
    FAKE(PowerDrv_Xtor)        \
    FAKE(PowerDrv_InitialPower)        \
    FAKE(PowerDrv_DeinitialPower)        \
    FAKE(PowerDrv_EjectExtBattery)        \
    FAKE(PowerDrv_HandleExtBattDetInterrupt)        \
    FAKE(PowerDrv_RegisterIntEvent)        \
    FAKE(PowerDrv_UnRegisterIntEvent)        \
    FAKE(PowerDrv_SetExtraCommand)        \
    FAKE(PowerDrv_SetInputSource)        \
    FAKE(PowerDrv_GetInputSourceState)        \
    FAKE(PowerDrv_GetBatteryVol)        \
    FAKE(PowerDrv_SetBatterySource)        \
    FAKE(PowerDrv_PowerSaveSleep)        \
    FAKE(PowerDrv_HandleDcDetInterrupt)        \
    FAKE(BSP_FeedWatchdog)        \
    FAKE(Setting_Set)        \
    FAKE(Setting_Get)        \
    FAKE(getSysTime)        \
    FAKE(GetServerID)


/********************************************************/
/*********   Local function ***********************************/
/********************************************************/

enum
{
    POW_SRV_TIMEOUT_SIG = MAX_SIG ,
}eInterSig;

static int16 intBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND;
static int16 extBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND;

const void* my_Setting_Get(eSettingId id)
{    
    bool ret = TRUE;
    return (void*)&ret;
}

bool  my_PowerDrv_GetBatteryVol(cPowerDrv *me, tBatteryVol* batteryVol)
{    
    batteryVol->intBatteryVol = intBatteryVol;
    batteryVol->extBatteryVol = extBatteryVol;
    return TRUE;
}

/********************************************************/
/*********   Main Test Class ***********************************/
/********************************************************/

class PowerSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {        
        PowerSrv_StartUp((cPersistantObj *)&me);
        // List of fakes we use and need to reset 
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
        Setting_Get_fake.custom_fake = my_Setting_Get;
        PowerDrv_GetBatteryVol_fake.custom_fake = my_PowerDrv_GetBatteryVol;

    }
    virtual void TearDown()
    {
        PowerSrv_ShutDown((cPersistantObj *)&me);

    }
    public:
};


TEST_F(PowerSrvTest, PowerInitTest)
{        
	QEvt evt;    
    PowerSrv_Initial(&me, &evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(PowerSrv_DeActive));   
    ASSERT_EQ(me.batteryInfo.voltage.intBatteryVol, intBatteryVol);
}


//test the deActive fuctnion 
TEST_F(PowerSrvTest, DeActiveSystemActiveTest)
{    
	QEvt evt;
    evt.sig = SYSTEM_ACTIVE_REQ_SIG;
    PowerSrv_DeActive(&me, &evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(PowerSrv_PreActive));     
}

//test the deActive fuctnion 
TEST_F(PowerSrvTest, ActiveSystemSleepTest)
{    
	QEvt evt;
    evt.sig = SYSTEM_SLEEP_REQ_SIG;
    PowerSrv_Active(&me, &evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(PowerSrv_DeActive));     
}

TEST_F(PowerSrvTest, PowerSleepTest)
{    
	QEvt evt;
    evt.sig = POWER_MCU_SLEEP_SIG;
    PowerSrv_DeActive(&me, &evt);
    ASSERT_EQ(PowerDrv_RegisterIntEvent_fake.call_count, 1);
    ASSERT_EQ(PowerDrv_PowerSaveSleep_fake.call_count, 1);
}


/* test the filter if it get the highest sample */
TEST_F(PowerSrvTest, PowerBatteryHighestFilterTest)
{        
	QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j;    
    int16 maxValue = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND-1;    
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        intBatteryVol = maxValue;
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
            intBatteryVol--;
        }
    }
    ASSERT_EQ(me.batteryInfo.voltage.intBatteryVol, maxValue);   
}

/* test the filter if it ignore the invalid sample*/

TEST_F(PowerSrvTest, PowerInvalidSampleTest)
{        
	QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j;    
    int16 maxValue = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND-1;    
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        intBatteryVol = maxValue;
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
            intBatteryVol--;
        }
        /* the filter should ignore this invalid value */
        intBatteryVol = maxValue - 500;
        PowerSrv_Active(&me, &evt);
    }
    ASSERT_EQ(me.batteryInfo.voltage.intBatteryVol, maxValue);   
}


/* Test if the state is correct */
TEST_F(PowerSrvTest, PowerBatteryStateTest)
{        
	QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j;    
    intBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND +1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
        }
    }
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_76_94_PERT_STA);   
}

/* Test if the state is stable */
TEST_F(PowerSrvTest, PowerBatteryStateStableTest)
{        
	QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j;    
    
    intBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND + 1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
        }
    }
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_76_94_PERT_STA); 
    
    intBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND - 1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
        }
    }
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_76_94_PERT_STA);   
}



/* Test if the state is changed */
TEST_F(PowerSrvTest, PowerBatteryStateChangeTest)
{        
	QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j;    

    /* pass the data to make battery state at BATT_76_94_PERT_STA*/
    intBatteryVol = BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND + 1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {            
            PowerSrv_Active(&me, &evt);
        }
    }
    ASSERT_EQ(me.batteryInfo.voltage.intBatteryVol, BATT_75_PERCENTAGE_mVOLT_HIGH_BOUND + 1);   
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_76_94_PERT_STA); 

    /* pass the data to change battery state to BATT_51_75_PERT_STA*/
    int k;
    intBatteryVol = BATT_75_PERCENTAGE_mVOLT_LOW_BOUND - 1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {
            for(k=0;k<OUT_OF_RANGE_ACCEPT_NUMBER;k++)
            {
                intBatteryVol = BATT_75_PERCENTAGE_mVOLT_LOW_BOUND - 1;
                PowerSrv_Active(&me, &evt);
            }
        }
    }  
    ASSERT_EQ(me.batteryInfo.voltage.intBatteryVol, BATT_75_PERCENTAGE_mVOLT_LOW_BOUND - 1);  
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_51_75_PERT_STA);   
}


/* Negative Test */
TEST_F(PowerSrvTest, PowerNegativeTest)
{
    QEvt evt;    
    evt.sig = POW_SRV_TIMEOUT_SIG;
    int i,j,k;
    /* pass the wrong battery voltage */
    intBatteryVol = -1;
    for(i=0;i<BATT_FILTER_LEN;i++)
    {       
        for(j=0;j<BATT_FILTER_LEN;j++)
        {
            for(k=0;k<OUT_OF_RANGE_ACCEPT_NUMBER;k++)
            {
                
                PowerSrv_Active(&me, &evt);
            }
        }
    }
    /* get the 0~5% battery state */
    ASSERT_EQ(me.batteryInfo.intBattState, BATT_0_5_PERT_STA);   
}

TEST_F(PowerSrvTest, PowerNegativeSetInputTest)
{
    PowerSrvSetEvt evt;    
    evt.super.sig = POWER_SET_SIG;
    evt.sender = NULL;
    evt.inputSource = (eInputSource)0xff;
    PowerSrv_Active(&me, (QEvt*)&evt);

    ASSERT_EQ(PowerDrv_SetInputSource_fake.call_count, 0);
}

TEST_F(PowerSrvTest, PowerNegativeSetBatteryTest)
{
    PowerSrvSetEvt evt;    
    evt.super.sig = POWER_SET_SIG;
    evt.sender = NULL;
    evt.batterySource = (eBatterySource)0xff;
    PowerSrv_Active(&me, (QEvt*)&evt);

    ASSERT_EQ(PowerDrv_SetBatterySource_fake.call_count, 0);
}

TEST_F(PowerSrvTest, PowerNegativeSetExtraCmdTest)
{
    PowerSrvSetEvt evt;    
    evt.super.sig = POWER_SET_SIG;
    evt.sender = NULL;
    evt.extraCommand = (eExtraCommand)0xff;
    PowerSrv_Active(&me, (QEvt*)&evt);
    ASSERT_EQ(PowerDrv_SetExtraCommand_fake.call_count, 0);
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
