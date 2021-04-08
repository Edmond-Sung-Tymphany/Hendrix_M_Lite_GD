

extern "C"{

#include "controller.h"
#include "controller_priv.h"
#include "deviceTypes.h"
#include "qf_port.h"

#include "qf_port.c"
}
#include "gtest/gtest.h"
#include <string.h>



/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)            \
	FAKE(QActive_post)					\
	FAKE(QActive_recall)				\



class ControllerTest : public ::testing::Test {
 protected:
    virtual void SetUp() 
    {	
		
		/* List of fakes we use and need to reset */
		FFF_FAKES_LIST(RESET_FAKE);
		FFF_RESET_HISTORY();
        Controller_Ctor(1);
        Controller_Initial(&Controller);
        
    }
    virtual void TearDown() 
    {
		Controller_Xtor();
    }
    public:
};

/**
Test: To test that a controller object can accept an entry signal in idle state
Assumptions: Clean controller (that is that set-up and tear-down should have cleared all data
Expectations: Idle state should return Q_Handled
*/
TEST_F(ControllerTest, Idle_Entry_OK)
{
    /* data setup */
    QEvt e;
    e.sig = Q_ENTRY_SIG;
    
    /* Test & check */
	ASSERT_EQ(Q_HANDLED(), Controller_Idle(&Controller, &e));
}

/**
Test: To test that a controller object can accept the SYSTEM_MODE_REQ_SIG signal in idle state
Assumptions: There is no outstanding request for mode switch
Expectations: Should send out a SYSTEM_MODE_RESP_SIG to the caller and transition Q_RET_TRAN
*/
TEST_F(ControllerTest, Idle_SysModeReq_OK)
{
    /* data setup */
    SwitchModeReqEvt e1;
    e1.super.sig = SYSTEM_MODE_REQ_SIG;
    e1.modeId = NORMAL_MODE;    
    
    /* Test */
    ASSERT_EQ(Q_RET_TRAN, Controller_Idle(&Controller, (QEvt*)&e1));
     /* check */   
    ASSERT_EQ(QActive_post_fake.call_count, 2);

}

/**
Test: To test that a controller object can accept an entry signal in SwitchingMode state
Assumptions:
Expectations: SwitchingMode state should return Q_Handled
*/
TEST_F(ControllerTest, SwitchingMode_Entry_OK)
{
    /* data setup */
    QEvt e;
    e.sig = Q_ENTRY_SIG;
    
    ASSERT_EQ(Q_HANDLED(),Controller_SwitchingModeState(&Controller, &e));
    /* Test  & check */
}

/**
Test: To test that a controller object can accept SYSTEM_MODE_REQ_SIG signal in SwitchingMode state
Assumptions: None
Expectations: SwitchingMode state should return Q_Handled
*/
TEST_F(ControllerTest, SwitchingMode_SysModeMode_OK)
{
    /* data setup */
    SwitchModeReqEvt e1;
    e1.super.sig = SYSTEM_MODE_REQ_SIG;
    e1.modeId = NORMAL_MODE; 
    
    /* Test */
    ASSERT_EQ(Q_HANDLED(),Controller_SwitchingModeState(&Controller,(QEvt*)&e1));
    /* check */  
    ASSERT_EQ(QActive_defer_fake.call_count, 1);
}
