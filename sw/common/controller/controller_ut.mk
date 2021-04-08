			
# Flags passed to the GCC compiler.
CFLAGS +=

# Flags passed to the preprocessor.
# Set Google Test's header directory as a system directory, such that
# the compiler doesn't generate warnings in Google Test headers.
CPPFLAGS += -DGTEST_CONTROLLER

# Flags passed to the C++ compiler.
CXXFLAGS +=

TEST_WITH_QP = TRUE


INCPATH = 	-I../../../include \
			-I../../include \
			-I../../bsp/Win32/include \
			-I../include \
			-I../../hardware_management/include \
            -I../../server/include \
            -I../../test_tools \
            -I../../ui_layer/include



# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TEST_EXE = controller_ut
UNDER_TEST_SRCS = 	./controller.c



TEST_CASE_SRCS = ./controller_ut.cc

include $(TP_SW_TOP)/test_tools/MakeGTest.inc
