			
# Flags passed to the GCC compiler.
INCPATH =
CFLAGS +=-DPC_PLATFORM $(INCPATH)

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -isystem $(GTEST_DIR)/include -I$(GMOCK_DIR)/include -DPC_PLATFORM -DGTEST

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wall -Wextra -pthread -DPC_PLATFORM -DGTEST

TEST_WITH_QP = TRUE


HEADER_LOCATIONS = 	-I../../../include \
					-I../../include \
					-I../../bsp/Win32/include \
					-I../include \
					-I../../ui_layer/include \
					-I../../project_files/iBT150 \
					-I../../project_files/iBT150/stm32/include \
					-I../../hardware_management/include \
					-I../../driver/include


# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TEST_EXE = debugsrv_ut
UNDER_TEST_SRCS = 	../../bsp/Win32/bsp.c \
					../../server/server.c \
					../../common/datatype/ringbuf/ringbuf.c \
					../../common/crc16_lib/crc16.c \
					./DebugSrv.c



TEST_CASE_SRCS = ./debugsrv_ut.cc

include $(TP_SW_TOP)/test_tools/MakeGTest.inc
