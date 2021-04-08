// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

extern "C"{
#include "commonTypes.h"
#include "controller.h"
#include "deviceTypes.h"
#include "persistantObj.h"

#include "debugSSrv.h"

#include "qf_port.c"
}
#include "gtest/gtest.h"
#include <string.h>

/*****************************************************************************************************************
 * Fake functions
 *****************************************************************************************************************/
FAKE_VOID_FUNC(UartDrv_Ctor, cUartDrv*, const tUARTDevice*, cRingBuf*, cRingBuf*);
FAKE_VOID_FUNC(UartDrv_Xtor, cUartDrv*);
FAKE_VALUE_FUNC(uint32, UartDrv_RegisterRxCallback, cUartDrv*, uartRxCb);
FAKE_VALUE_FUNC(uint32, UartDrv_Write, cUartDrv*, const uint8*, uint32);

FAKE_VOID_FUNC(RingBuf_Ctor, cRingBuf*, data*, uint32);
FAKE_VOID_FUNC(RingBuf_Xtor, cRingBuf*);
FAKE_VOID_FUNC(RingBuf_Reset, cRingBuf*);

FAKE_VALUE_FUNC(const void*, Setting_Get, eSettingId);
FAKE_VOID_FUNC(Setting_Set, eSettingId, const void*);
#ifdef ENABLE_WAKEUP_BY_UART
FAKE_VALUE_FUNC(uint32, UartDrv_RegisterWakeUpCallback, cUartDrv*, uartWakeUpCb);
FAKE_VOID_FUNC(UartDrv_EnableWakeUp, cUartDrv*);
#endif
FAKE_VOID_FUNC(GpioDrv_DirectPinAccessSet, eIoPort, eIoBit, eGPIOInitAttr);
FAKE_VALUE_FUNC(uint8, GpioDrv_DirectPinAccessRead, eIoPort, eIoBit, eGPIODrection);

/* List of fakes used by this unit tester */
#define FFF_FAKES_LIST(FAKE)            \
    FAKE(QF_publish)					\
    FAKE(SendToServer)					\
    FAKE(UartDrv_Ctor)					\
    FAKE(UartDrv_Xtor)					\
    FAKE(UartDrv_RegisterRxCallback)	\
    FAKE(RingBuf_Ctor)					\
    FAKE(RingBuf_Xtor)					\
    FAKE(RingBuf_Reset)                 \
    FAKE(GpioDrv_DirectPinAccessSet)    \
    FAKE(GpioDrv_DirectPinAccessRead)

/*****************************************************************************************************************
 * Local variables
 *****************************************************************************************************************/
static cDebugSSrv me;       // test object
static uint8 fakeTxBuf[SIZE_OF_LARGE_EVENTS + DMSG_MIN_SIZE];    // fake buffer for UART Tx

static const tUARTDevice UartDebugConfig =
{
    {DEBUG_DEV_ID, UART_DEV_TYPE},  // tDevice
    115200,                         // baudrate
    TP_UART_DEV_1,                  // uartId
    TP_UART_BYTE_SIZE_8_BITS,       // byteSize
    TP_UART_PARITY_NONE,            // parity
    TP_UART_STOP_BIT_1,             // stopBits
    FALSE,                          // dmaEnable
    {0, 0},                         // interrupt
    0,                              // bBatchProcessInIRQ
};

/* @brief       Fake getDevicebyId()
 */
static const tDevice* my_getDevicebyId(eDeviceID deviceID, uint16 *index)
{
    (void) deviceID;
    (void) index;
    return (tDevice*)&UartDebugConfig;
}

/* @brief       Fake Uart Write by copying all the Tx data into fakeTxBuf
 */
static uint32 my_UartDrv_Write(cUartDrv* me, const uint8 *p, uint32 size)
{
    (void) me;
    (void) size;
    uint32 ret = SUCCESS;

    memcpy(fakeTxBuf, p, sizeof(fakeTxBuf));
    return ret;
}

#ifdef EXAMINE_BUFFER
/* @brief       print buffer out for debugging
 */
static void printBuf(uint8 *p, uint32 msgSize)
{
    const uint32 itemPerLine = 16;
    uint32 i = 0;

    printf("size = %lu\r\n", msgSize);
    for ( ; i < msgSize / itemPerLine + 1; ++i)
    {
        uint32 j = 0;
        for ( ; j < itemPerLine; ++j)
        {
            uint32 index = i * itemPerLine + j;
            if (index < msgSize)
            {
                printf(" %02X ", p[index]);
            }
        }
        printf("\r\n");
    }
}
#endif

/*****************************************************************************************************************
 * Local variables
 *****************************************************************************************************************/
class DebugSSrvTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        getDevicebyId_fake.custom_fake = my_getDevicebyId;
        UartDrv_Write_fake.custom_fake = my_UartDrv_Write;
        memset(fakeTxBuf, 0, sizeof(fakeTxBuf));

        DebugSrv_StartUp((cPersistantObj *)&me);

        /* List of fakes we use and need to reset */
        FFF_FAKES_LIST(RESET_FAKE);
        FFF_RESET_HISTORY();
    }

    virtual void TearDown()
    {
        DebugSSrv_ShutDown((cPersistantObj *)&me);
    }

public:
};

/*****************************************************************************************************************
 * Test cases
 *****************************************************************************************************************/

/**
 * Test:           Positive test if the initial state is entering to the correct initial state
 * Assumptions:    Clean server (that is that set-up and tear-down should have cleared all data
 * Expectations:   The state registered function is in deactive mode
 */
TEST_F(DebugSSrvTest, Initial_Entry_OK)
{
    DebugSSrv_Initial(&me, NULL);
    ASSERT_EQ(((QMsm *)&me)->temp.fun, Q_STATE_CAST(DebugSSrv_DeActive));
}

/**
 * Test:           Positive test if the ready state could pass the data out
 * Assumptions:    Clean server (that is that set-up and tear-down should have cleared all data
 * Expectations:   The first byte of the Tx is START_SIGN, the signal should be in the correct position
 */
TEST_F(DebugSSrvTest, Active_KeyTx_OK)
{
	QEvt evt;
	evt.sig = KEY_STATE_SIG;
    DebugSSrv_Ready(&me, &evt);

    ASSERT_EQ(START_SIGN,       fakeTxBuf[START_SIGN_IDX]);
    ASSERT_EQ(KEY_STATE_SIG,    fakeTxBuf[SIG_IDX]);
}

/**
 * Test:           Positive test if the ready state could handle the incoming data
 * Assumptions:    Clean server (that is that set-up and tear-down should have cleared all data
 * Expectations:   The signal will be published by QF_publish()
 */
TEST_F(DebugSSrvTest, Active_KeyRx_OK)
{
    // sample data of KeyStateEvt KEY_DOWN from BLE
    const uint8 sampleData[] =
    {
        0xAA, // START_SIGN_IDX
        0x07, // SIG_IDX
        0x03, // SRVID_IDX
        0x13, 0x00, // SIZELSB_IDX, SIZEMSB_IDX
        // DATASTART_IDX,
        0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0xF8, 0x4D, // CRC
    };

	QEvt evt;
	evt.sig = CMD_READY_CALLBACK_SIG;
    memcpy(me.uartRxBuf, sampleData, sizeof(sampleData));
    me.dmsg.msg_size = sizeof(sampleData);

    DebugSSrv_Ready(&me, &evt);

    ASSERT_EQ(1,    QF_publish_fake.call_count);
    ASSERT_EQ(0,    SendToServer_fake.call_count);
}

/**
 * Test:           Negative test if the ready state could handle the incorrect incoming data
 * Assumptions:    Clean server (that is that set-up and tear-down should have cleared all data
 * Expectations:   The error signal will be feedback through SendToServer()
 */
TEST_F(DebugSSrvTest, Active_ErrorCRC_OK)
{
    // sample data of KeyStateEvt KEY_DOWN from BLE
    const uint8 sampleData[] =
    {
        0xAA, // START_SIGN_IDX
        0x07, // SIG_IDX
        0x03, // SRVID_IDX
        0x13, 0x00, // SIZELSB_IDX, SIZEMSB_IDX
        // DATASTART_IDX,
        0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0xEC, 0xEC, // incorrect CRC
    };

	QEvt evt;
	evt.sig = CMD_READY_CALLBACK_SIG;
    memcpy(me.uartRxBuf, sampleData, sizeof(sampleData));
    me.dmsg.msg_size = sizeof(sampleData);

    DebugSSrv_Ready(&me, &evt);

    ASSERT_EQ(0,    QF_publish_fake.call_count);
    ASSERT_EQ(1,    SendToServer_fake.call_count);
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
