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


INCPATH = 	-I../../../include \
			-I../../include \
			-I../include \
			-I../../bsp/Win32/include \
			-I../../ui_layer/include \
			-I../../project_files/$(TP_PRODUCT) \
			-I../../project_files/$(TP_PRODUCT)/conf \
			-I../../project_files/$(TP_PRODUCT)/$(TP_MCU_FAMILY)/include \
			-I../../hardware_management/include \
			-I../../driver/include 


# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TEST_EXE = keysrv_ut
UNDER_TEST_SRCS = 	../../server/server.c \
					../../common/persistantObj.c \
                    ./KeySrv.c
                    


TEST_CASE_SRCS = ./keysrv_ut.cc

include $(TP_SW_TOP)/test_tools/MakeGTest.inc
