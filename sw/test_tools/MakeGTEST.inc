# Points to the root of Google Mock, relative to where this file is.
GTEST_DIR = $(TP_SW_TOP)/test_tools/gtest

# include path
GTEST_INCPATH = -g -isystem $(GTEST_DIR)/include

# Flags passed to the GCC compiler.
CFLAGS += \

# Flags passed to the preprocessor.
CPPFLAGS += \
	-DPC_PLATFORM -DGTEST -D$(TP_PRODUCT)

# Flags passed to the C++ compiler.
CXXFLAGS += \
	-Wall -Wextra -pthread -Wunused-local-typedefs

# All Google Test headers.  Usually you shouldn't change this
# definition.
GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
				$(GTEST_DIR)/include/gtest/internal/*.h


# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
##################### IF YOU WANT THE WHOLE QP LIB THEN DEFINE TEST_WITH_QP... but i just removed this and will totally remove once i know it can be             ####################
ifeq ($(TEST_WITH_QP),TRUE) # basic check

INCPATH += -I$(TP_SW_TOP)/common/qp/include \
			-I$(TP_SW_TOP)/common/qp/ports/gtest \
			-I$(TP_SW_TOP)/test_tools

endif


# Extract source
TEST_C_SRCS = $(filter %.c,$(UNDER_TEST_SRCS))
TEST_CPP_SRCS = $(filter %.cc,$(TEST_CASE_SRCS))

# Extract object file names from source file names
TEST_C_OBJS = $(TEST_C_SRCS:%.c=%.o)
TEST_CPP_OBJS = $(TEST_CPP_SRCS:%.cc=%.o)

############################################################################
# Builds gtest + your UUT

all : $(TEST_EXE)

run : $(TEST_EXE)
	./$(TEST_EXE)

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(GTEST_DIR) $(GTEST_INCPATH) -c \
            $(GTEST_DIR)/src/gtest-all.cc

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -I$(GTEST_DIR) $(GTEST_INCPATH) -c \
            $(GTEST_DIR)/src/gtest_main.cc

gtest.a : gtest-all.o
	$(AR) $(ARFLAGS) $@ $^

gtest_main.a : gtest-all.o gtest_main.o
	$(AR) $(ARFLAGS) $@ $^
    
ifneq ($(TEST_EXE),) # basic check

$(TEST_EXE) : gtest_main.a $(TEST_CPP_OBJS) $(TEST_C_OBJS) 
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
#
#
# Clean Rule
clean :
	rm -f $(TEST_EXE) gtest-all.o gtest_main.a gtest_main.o $(TEST_C_OBJS) $(TEST_CPP_OBJS)
	
$(TEST_C_OBJS): %.o:%.c
	gcc $(CPPFLAGS) $(CFLAGS) $(INCPATH) -c -o $@ $<

$(TEST_CPP_OBJS): %.o:%.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(GTEST_INCPATH) $(INCPATH) -c -o $@ $<
	
else
all :
	$(error TEST_EXE not defined)
endif  # TEST_EXE
