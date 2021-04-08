# Compiler flags to enable all warnings & debug info
NANOPB_DIR = ../../../common/nanopb
FRAMING_DIR = ../../../common/framing/yahdlc/C/
PLATFORM_LIBRARY_INCLUDE = ../../../include
ASE_SRV_COMMON_SHARE = ../../../server/ase_tk_server/message_format

OUTPUT_DIR = ./output
EXEC	:= $(OUTPUT_DIR)/ase_tk

MKDIR	?= mkdir -p
RM		?= rm -f

# -Werror -Wall
CFLAGS =  -g -O0
CFLAGS += -I$(NANOPB_DIR)

# Put the flags here which needs to be the last 
POSTFLAGS = -lpthread 

# C source code files that are required
CSRC  = main.c                   # The main program
CSRC += $(NANOPB_DIR)/pb_encode.c  # The nanopb encoder
CSRC += $(NANOPB_DIR)/pb_decode.c  # The nanopb decoder
CSRC += $(NANOPB_DIR)/pb_common.c  # The nanopb common parts
CSRC += $(ASE_SRV_COMMON_SHARE)/ase_tk.pb.c   # The compiled protocol definition
CSRC += $(FRAMING_DIR)/*.c
CSRC += ./comms/error_handle_lib/*.c
CSRC += ./comms/protocol_buffer/*.c
CSRC += ./comms/framing/*.c
CSRC += ./comms/uart/*.c
CSRC += ./comms/*.c
CSRC += ./common/*.c
CSRC += ./user_interface/*.c
CSRC += ./user_interface/terminal_input/*.c
CSRC += ./user_interface/batch_input/*.c

INC = -I$(NANOPB_DIR) \
	  -I$(PLATFORM_LIBRARY_INCLUDE)\
	  -I$(ASE_SRV_COMMON_SHARE)\
	  -I./comms/includes  \
	  -I./user_interface/includes \
	  -I./common/includes \
	  -I./

# mkdir the output directory
create-output-directory := $(shell $(MKDIR) $(OUTPUT_DIR);)

# Build rule for the main program
ase_tk: $(CSRC)
	$(CC) $(CFLAGS) $(INC) -o $(EXEC) $(CSRC) $(POSTFLAGS)
	$(EXEC)

clean:
	@$(RM) -r $(OUTPUT_DIR)
