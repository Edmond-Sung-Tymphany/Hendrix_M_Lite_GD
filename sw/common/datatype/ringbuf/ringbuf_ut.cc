// This sample shows how to write a simple unit test for a function,
// using Google C++ testing framework.

// Step 1. Include necessary header files such that the stuff your
// test logic needs is declared.
//
// Don't forget gtest.h, which declares the testing framework.

#include "ringbuf.h"
#include "gtest/gtest.h"


// Step 2. Use the TEST macro to define your tests.
//
// TEST has two parameters: the test case name and the test name.
// After using the macro, you should define your test logic between a
// pair of braces.  You can use a bunch of macros to indicate the
// success or failure of a test.  EXPECT_TRUE and EXPECT_EQ are
// examples of such macros.  For a complete list, see gtest.h.
//
// <TechnicalDetails>
//
// In Google Test, tests are grouped into test cases.  This is how we
// keep test code organized.  You should put logically related tests
// into the same test case.
//
// The test case name and the test name should both be valid C++
// identifiers.  And you should not use underscore (_) in the names.
//
// Google Test guarantees that each test you define is run exactly
// once, but it makes no guarantee on the order the tests are
// executed.  Therefore, you should write your tests in such a way
// that their results don't depend on their order.
//
// </TechnicalDetails>


// Tests RingBuf

// Tests constructing RingBuf
TEST(ringbufTest, Contrustion) {
    const int ARRAY_SIZE = 1024;
    cRingBuf ringBuf;           // Ring buffer object
    data arr[ARRAY_SIZE];

    memset(arr, 0, sizeof(data) * ARRAY_SIZE);
    RingBuf_Ctor(&ringBuf, arr, ARRAY_SIZE);

    EXPECT_TRUE(RingBuf_IsEmpty(&ringBuf));
    EXPECT_EQ(0, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 1, RingBuf_GetFreeSize(&ringBuf));
}

// Tests push/pop
TEST(ringbufTest, PushPop) {
    const int ARRAY_SIZE = 1024;
    const data item = 0xA5;     // random number
    cRingBuf ringBuf;           // Ring buffer object
    data arr[ARRAY_SIZE];
    data out[ARRAY_SIZE];
    uint16 i;

    memset(arr, 0, sizeof(data) * ARRAY_SIZE);
    RingBuf_Ctor(&ringBuf, arr, ARRAY_SIZE);

    // Single Push
    EXPECT_EQ(RET_SUCCESS, RingBuf_PushData(&ringBuf, item));
    EXPECT_EQ(1, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 2, RingBuf_GetFreeSize(&ringBuf));

    // Single Pop
    EXPECT_EQ(item, RingBuf_PopData(&ringBuf));
    EXPECT_EQ(0, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 1, RingBuf_GetFreeSize(&ringBuf));

    // Fill-up all entries
    for ( i = 0; i < ARRAY_SIZE - 1; i++ )
    {
        out[i] = i & 0xFF;
    }

    // Push
    EXPECT_EQ(RET_SUCCESS, RingBuf_Push(&ringBuf, out, ARRAY_SIZE - 1));
    EXPECT_EQ(ARRAY_SIZE - 1, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(0 , RingBuf_GetFreeSize(&ringBuf));

    // Pop
    EXPECT_EQ(RET_SUCCESS, RingBuf_Pop(&ringBuf, out, ARRAY_SIZE - 1));
    EXPECT_EQ(0, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 1, RingBuf_GetFreeSize(&ringBuf));
}

// Tests overflow
TEST(ringbufTest, Overflow) {
    const int ARRAY_SIZE = 1024;
    cRingBuf ringBuf;           // Ring buffer object
    data arr[ARRAY_SIZE];
    data out[ARRAY_SIZE];
    uint16 i;

    memset(arr, 0, sizeof(data) * ARRAY_SIZE);
    RingBuf_Ctor(&ringBuf, arr, ARRAY_SIZE);

    // Fill-up all entries
    for ( i = 0; i < ARRAY_SIZE - 1; i++ )
    {
        RingBuf_PushData(&ringBuf, i & 0xFF);
    }
    EXPECT_EQ(ARRAY_SIZE - 1, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(0 , RingBuf_GetFreeSize(&ringBuf));

    // Overflow Push
    EXPECT_EQ(RET_FAIL, RingBuf_PushData(&ringBuf, 0));
    EXPECT_EQ(ARRAY_SIZE - 1, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(0 , RingBuf_GetFreeSize(&ringBuf));

    // Pop all entries
    EXPECT_EQ(RET_SUCCESS, RingBuf_Pop(&ringBuf, out, ARRAY_SIZE - 1));
    EXPECT_EQ(0, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 1, RingBuf_GetFreeSize(&ringBuf));

    // Overflow Pop
    EXPECT_EQ(RET_FAIL, RingBuf_Pop(&ringBuf, out, 1));
    EXPECT_EQ(0, RingBuf_GetUsedSize(&ringBuf));
    EXPECT_EQ(ArraySize(arr) - 1, RingBuf_GetFreeSize(&ringBuf));
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
