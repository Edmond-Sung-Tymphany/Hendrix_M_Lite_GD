# TP_SW_TOP (...tymphany_platform\sw) and 
# TP_PRODUCT (project name define) have to defined before compilation		
# Flags passed to the GCC compiler.
CFLAGS +=

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS +=

# Flags passed to the C++ compiler.
CXXFLAGS +=

TEST_WITH_QP = TRUE

INCPATH = 	-I../../../include -I../../include


# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TEST_EXE = ringbuf_ut
UNDER_TEST_SRCS = ringbuf.c


TEST_CASE_SRCS = ./ringbuf_ut.cc

include $(TP_SW_TOP)/test_tools/MakeGTest.inc
