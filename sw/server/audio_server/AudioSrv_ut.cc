// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "./AudioSrv_priv.h"
#include "SettingSrv.h"
#include "controller.h"
#include "deviceTypes.h"
#include "persistantObj.h"
#include "DspDrv.h"
#include "qf_port.h"
#include "qf_port.c"
}
#include "AudioSrv_ut.h"
#include "gtest/gtest.h"
#include <string.h>

cAudioSrv me;

#define TIMEOUT_SIG MAX_SIG

FAKE_VOID_FUNC(DSPDrv_Ctor, cDSPDrv*);
FAKE_VOID_FUNC(DSPDrv_Xtor, cDSPDrv*);
FAKE_VOID_FUNC(AudioDrv_Ctor);
FAKE_VOID_FUNC(Setting_Set, eSettingId, const void*);
FAKE_VOID_FUNC(AudioDrv_Mute,bool);
FAKE_VOID_FUNC(CommonEvtResp,QActive* , QActive*, eEvtReturn, eSignal);
FAKE_VOID_FUNC(DSPDrv_SetAudio,cDSPDrv *, eDspSettId, bool);
FAKE_VOID_FUNC(DSPDrv_SetDrcForAdaptorMode, cDSPDrv *);
FAKE_VOID_FUNC(DSPDrv_SetDrcForNormalPower, cDSPDrv *);
FAKE_VOID_FUNC(DSPDrv_SetDrcForLowPower,cDSPDrv *);
FAKE_VOID_FUNC(DSPDrv_set_Input, cDSPDrv*, eAudioCtrlDriverInput);
FAKE_VOID_FUNC(SendToServer, uint16, const QEvt*);
FAKE_VOID_FUNC(DSPDrv_SetVol, cDSPDrv *, uint8);
#ifdef HAS_BT_TONE
FAKE_VOID_FUNC(BluetoothSrv_SendBtCmd, QActive*, eBtCmd);
FAKE_VOID_FUNC(BluetoothDrv_EnableCodecStatus, bool);
#endif
FAKE_VALUE_FUNC(bool, Setting_IsReady,eSettingId);
FAKE_VALUE_FUNC(uint16, DSPDrv_Init, cDSPDrv*);
FAKE_VALUE_FUNC(const void*, Setting_Get, eSettingId);
FAKE_VALUE_FUNC(bool, DSPDrv_HasMusicStream, cDSPDrv *);
FAKE_VALUE_FUNC(bool, DSPDrv_IsAuxin, cDSPDrv *);

/* List of fakes used by this unit tester */
#ifdef HAS_BT_TONE
#define FFF_FAKES_LIST(FAKE)            \
    FAKE(DSPDrv_Ctor)                   \
    FAKE(DSPDrv_Xtor)                   \
    FAKE(AudioDrv_Ctor)                 \
    FAKE(Setting_Set)                   \
    FAKE(AudioDrv_Mute)                 \
    FAKE(DSPDrv_HasMusicStream)         \
	FAKE(DSPDrv_SetVol)                 \
    FAKE(CommonEvtResp)                 \
    FAKE(DSPDrv_SetAudio)               \
    FAKE(DSPDrv_SetDrcForAdaptorMode)   \
    FAKE(DSPDrv_SetDrcForNormalPower)   \
    FAKE(DSPDrv_SetDrcForLowPower)      \
    FAKE(DSPDrv_set_Input)              \
    FAKE(Setting_IsReady)               \
    FAKE(DSPDrv_Init)                   \
    FAKE(DSPDrv_IsAuxin)                \
    FAKE(SendToServer)                  \
    FAKE(Setting_Get)                   \
    FAKE(BluetoothSrv_SendBtCmd)        \
	FAKE(BluetoothDrv_EnableCodecStatus)
#else
#define FFF_FAKES_LIST(FAKE)            \
    FAKE(DSPDrv_Ctor)                   \
    FAKE(DSPDrv_Xtor)                   \
    FAKE(AudioDrv_Ctor)                 \
    FAKE(Setting_Set)                   \
    FAKE(AudioDrv_Mute)                 \
    FAKE(DSPDrv_HasMusicStream)         \
	FAKE(DSPDrv_SetVol)                 \
    FAKE(CommonEvtResp)                 \
    FAKE(DSPDrv_SetAudio)               \
    FAKE(DSPDrv_SetDrcForAdaptorMode)   \
    FAKE(DSPDrv_SetDrcForNormalPower)   \
    FAKE(DSPDrv_SetDrcForLowPower)      \
    FAKE(DSPDrv_set_Input)              \
    FAKE(Setting_IsReady)               \
    FAKE(DSPDrv_Init)                   \
    FAKE(DSPDrv_IsAuxin)                \
    FAKE(SendToServer)                  \
    FAKE(Setting_Get)                   
#endif


uint16 my_DSPDrv_Init(cDSPDrv* me)
{
    (void) me;
	return 0;
}

class AudioSrvTest : public ::testing::Test {
 protected:
    virtual void SetUp()
    {
        /* List of fakes we use and need to reset */
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
        AudioSrv_StartUp((cPersistantObj *)&me);
        DSPDrv_Init_fake.custom_fake = my_DSPDrv_Init;
    }
    virtual void TearDown()
    {
        AudioSrv_ShutDown((cPersistantObj *)&me);
    }
    public:
};

TEST_F(AudioSrvTest, InitialTest)
{
    AudioSrv_Initial(&me);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(AudioSrv_DeActive));
}

//Test signal handling between super state and sub state,
//This test will test the case that sub state got a signal which could not be handled and 
//transfer to super state
TEST_F(AudioSrvTest, AudioSrv_AudioMainHandler_SleepReq)
{
    QEvt evt;
    evt.sig = SYSTEM_SLEEP_REQ_SIG;
    AudioSrv_AudioMainHandler(&me, &evt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(AudioSrv_Active));
}

//Test DeActive state SYSTEM_ACTIVE_REQ_SIG
TEST_F(AudioSrvTest, AudioSrv_DeActive_SysActiveSig)
{
    QState funRetValue;
    /* Test SYSTEM_ACTIVE_REQ_SIG */
    QEvt sysActReq;
    sysActReq.sig = SYSTEM_ACTIVE_REQ_SIG;
    funRetValue = AudioSrv_DeActive(&me, &sysActReq);
    ASSERT_EQ(DSPDrv_Ctor_fake.call_count,1); //make sure DSP driver is Ctored
    ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
}

//Test DeActive state TIMEOUT_SIG
TEST_F(AudioSrvTest, AudioSrv_DeActive_TimeOutSig)
{
    QState funRetValue;
    /* Test when DSP is not created, i.e. dsp is not initialized */
    QEvt timeOutEvt;
    timeOutEvt.sig = TIMEOUT_SIG;
    dspDrv.isCreated = false;
    funRetValue = AudioSrv_DeActive(&me, &timeOutEvt);
    ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);
    /* Test when DSP is created */
    timeOutEvt.sig = TIMEOUT_SIG;
    dspDrv.isCreated = true;
    AudioSrv_DeActive(&me, &timeOutEvt);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(AudioSrv_AudioMainHandler));
}

//Test Active state getting SYSTEM_SLEEP_REQ_SIG
TEST_F(AudioSrvTest, AudioSrv_Active_SleepSig)
{
    /* test sleep request signal */
    QEvt sleepEvt;
    sleepEvt.sig = SYSTEM_SLEEP_REQ_SIG;
    AudioSrv_Active(&me, &sleepEvt);
    ASSERT_EQ(DSPDrv_Xtor_fake.call_count,1);// DSP driver should be xtored
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(AudioSrv_DeActive));
}

//Test AudioSrv_AudioMainHandler state channel switch test
TEST_F(AudioSrvTest, AudioSrv_AudioMainHandler_ChangeCh)
{
    QState funRetValue;
    /* Switch channel test when channel is not changed*/
    AudioChannelSwitchReqEvt switchChEvt;
    switchChEvt.sender = (QActive*)&me;
    switchChEvt.super.sig = AUDIO_SWITCH_CHANNEL_SIG;
    switchChEvt.channel = AUDIO_CHANNEL_0;
    me.channel = AUDIO_CHANNEL_0;
    funRetValue = AudioSrv_AudioMainHandler(&me, (QEvt*)(&switchChEvt));
    ASSERT_EQ(funRetValue, (QState)Q_RET_HANDLED);

	/* Switch channel test when channel is changed*/
    switchChEvt.sender = (QActive*)&me;
    switchChEvt.super.sig = AUDIO_SWITCH_CHANNEL_SIG;
    switchChEvt.channel = AUDIO_CHANNEL_1;
    me.channel = AUDIO_CHANNEL_0;
    AudioSrv_AudioMainHandler(&me, (QEvt*)(&switchChEvt));
    ASSERT_EQ(AudioDrv_Mute_fake.call_count,1);
    /* Make sure that the current channel is saved */
    ASSERT_EQ(me.channel, switchChEvt.channel);
}
//Test AudioSrv_AudioMainHandler mute test
TEST_F(AudioSrvTest, AudioSrv_AudioMainHandler_MuteSig)
{
    QState funRetValue;
    AudioMuteReqEvt muteReqEvt;
    muteReqEvt.sender = NULL;
    muteReqEvt.super.sig = AUDIO_MUTE_SIG;
    muteReqEvt.mute = true;
    funRetValue = AudioSrv_AudioMainHandler(&me, (QEvt*)(&muteReqEvt));
	ASSERT_EQ(AudioDrv_Mute_fake.call_count,1);
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
