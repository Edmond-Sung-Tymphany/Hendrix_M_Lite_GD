#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Include project Makefile
ifeq "${IGNORE_LOCAL}" "TRUE"
# do not include local makefile. User is passing all local related variables already
else
include Makefile
# Include makefile containing local settings
ifeq "$(wildcard nbproject/Makefile-local-debug.mk)" "nbproject/Makefile-local-debug.mk"
include nbproject/Makefile-local-debug.mk
endif
endif

# Environment
MKDIR=gnumkdir -p
RM=rm -f 
MV=mv 
CP=cp 

# Macros
CND_CONF=debug
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IMAGE_TYPE=debug
OUTPUT_SUFFIX=elf
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
else
IMAGE_TYPE=production
OUTPUT_SUFFIX=hex
DEBUGGABLE_SUFFIX=elf
FINAL_IMAGE=dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
endif

# Object Directory
OBJECTDIR=build/${CND_CONF}/${IMAGE_TYPE}

# Distribution Directory
DISTDIR=dist/${CND_CONF}/${IMAGE_TYPE}

# Source Files Quoted if spaced
SOURCEFILES_QUOTED_IF_SPACED=../../src/hwsetup_lib/hwsetup.c ../../src/hwsetup_lib/interrupts.c ../../src/nvm_lib/NvmDrv_Pic32mx.c ../../src/nvm_lib/NVMem.c ../../src/nvm_lib/Triggers.c ../../src/QcAllplay/allplay_crc-ccitt.c ../../src/QcAllplay/allplay_protocol.c ../../src/QcAllplay/allplay_receiver.c ../../src/QcAllplay/allplay_receiver_simulator.c ../../src/QcAllplay/allplay_stream_pic32.c ../../src/uart_lib/tp_uart.c ../../src/uart_lib/uart_basic.c ../../src/util_lib/assert.c ../../src/util_lib/hex.c ../../src/util_lib/TimerUtil.c ../../src/util_lib/util.c ../../src/util_lib/dbgprint.c ../../src/util_lib/ui.c ../../src/util_lib/md5.c ../../src/BootLoader.c

# Object Files Quoted if spaced
OBJECTFILES_QUOTED_IF_SPACED=${OBJECTDIR}/_ext/538184631/hwsetup.o ${OBJECTDIR}/_ext/538184631/interrupts.o ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o ${OBJECTDIR}/_ext/4456416/NVMem.o ${OBJECTDIR}/_ext/4456416/Triggers.o ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o ${OBJECTDIR}/_ext/2118930945/tp_uart.o ${OBJECTDIR}/_ext/2118930945/uart_basic.o ${OBJECTDIR}/_ext/1593686579/assert.o ${OBJECTDIR}/_ext/1593686579/hex.o ${OBJECTDIR}/_ext/1593686579/TimerUtil.o ${OBJECTDIR}/_ext/1593686579/util.o ${OBJECTDIR}/_ext/1593686579/dbgprint.o ${OBJECTDIR}/_ext/1593686579/ui.o ${OBJECTDIR}/_ext/1593686579/md5.o ${OBJECTDIR}/_ext/1445274692/BootLoader.o
POSSIBLE_DEPFILES=${OBJECTDIR}/_ext/538184631/hwsetup.o.d ${OBJECTDIR}/_ext/538184631/interrupts.o.d ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d ${OBJECTDIR}/_ext/4456416/NVMem.o.d ${OBJECTDIR}/_ext/4456416/Triggers.o.d ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d ${OBJECTDIR}/_ext/2118930945/tp_uart.o.d ${OBJECTDIR}/_ext/2118930945/uart_basic.o.d ${OBJECTDIR}/_ext/1593686579/assert.o.d ${OBJECTDIR}/_ext/1593686579/hex.o.d ${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d ${OBJECTDIR}/_ext/1593686579/util.o.d ${OBJECTDIR}/_ext/1593686579/dbgprint.o.d ${OBJECTDIR}/_ext/1593686579/ui.o.d ${OBJECTDIR}/_ext/1593686579/md5.o.d ${OBJECTDIR}/_ext/1445274692/BootLoader.o.d

# Object Files
OBJECTFILES=${OBJECTDIR}/_ext/538184631/hwsetup.o ${OBJECTDIR}/_ext/538184631/interrupts.o ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o ${OBJECTDIR}/_ext/4456416/NVMem.o ${OBJECTDIR}/_ext/4456416/Triggers.o ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o ${OBJECTDIR}/_ext/2118930945/tp_uart.o ${OBJECTDIR}/_ext/2118930945/uart_basic.o ${OBJECTDIR}/_ext/1593686579/assert.o ${OBJECTDIR}/_ext/1593686579/hex.o ${OBJECTDIR}/_ext/1593686579/TimerUtil.o ${OBJECTDIR}/_ext/1593686579/util.o ${OBJECTDIR}/_ext/1593686579/dbgprint.o ${OBJECTDIR}/_ext/1593686579/ui.o ${OBJECTDIR}/_ext/1593686579/md5.o ${OBJECTDIR}/_ext/1445274692/BootLoader.o

# Source Files
SOURCEFILES=../../src/hwsetup_lib/hwsetup.c ../../src/hwsetup_lib/interrupts.c ../../src/nvm_lib/NvmDrv_Pic32mx.c ../../src/nvm_lib/NVMem.c ../../src/nvm_lib/Triggers.c ../../src/QcAllplay/allplay_crc-ccitt.c ../../src/QcAllplay/allplay_protocol.c ../../src/QcAllplay/allplay_receiver.c ../../src/QcAllplay/allplay_receiver_simulator.c ../../src/QcAllplay/allplay_stream_pic32.c ../../src/uart_lib/tp_uart.c ../../src/uart_lib/uart_basic.c ../../src/util_lib/assert.c ../../src/util_lib/hex.c ../../src/util_lib/TimerUtil.c ../../src/util_lib/util.c ../../src/util_lib/dbgprint.c ../../src/util_lib/ui.c ../../src/util_lib/md5.c ../../src/BootLoader.c


CFLAGS=
ASFLAGS=
LDLIBSOPTIONS=

############# Tool locations ##########################################
# If you copy a project from one host to another, the path where the  #
# compiler is installed may be different.                             #
# If you open this project with MPLAB X in the new host, this         #
# makefile will be regenerated and the paths will be corrected.       #
#######################################################################
# fixDeps replaces a bunch of sed/cat/printf statements that slow down the build
FIXDEPS=fixDeps

# The following macros may be used in the pre and post step lines
Device=PIC32MX450F256H
ProjectDir="D:\code\tymplat\mofa\tymphany_platform\sw\bootloaders\bl_pic32\mplab\bl_PolkAllplay"
ConfName=debug
ImagePath="dist\debug\${IMAGE_TYPE}\bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ImageDir="dist\debug\${IMAGE_TYPE}"
ImageName="bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}"
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
IsDebug="true"
else
IsDebug="false"
endif

.build-conf:  ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-debug.mk dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}
	@echo "--------------------------------------"
	@echo "User defined post-build step: [bl_post_build_actions.bat ${IMAGE_TYPE} ${ConfName} ${MP_CC_DIR}]"
	@bl_post_build_actions.bat ${IMAGE_TYPE} ${ConfName} ${MP_CC_DIR}
	@echo "--------------------------------------"

MP_PROCESSOR_OPTION=32MX450F256H
MP_LINKER_FILE_OPTION=,--script="..\..\src_proj_specific\bl_PolkAllplay\btl_32MX450F256H_generic.ld"
# ------------------------------------------------------------------------------------
# Rules for buildStep: assemble
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: assembleWithPreprocess
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compile
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
${OBJECTDIR}/_ext/538184631/hwsetup.o: ../../src/hwsetup_lib/hwsetup.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/538184631 
	@${RM} ${OBJECTDIR}/_ext/538184631/hwsetup.o.d 
	@${RM} ${OBJECTDIR}/_ext/538184631/hwsetup.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/538184631/hwsetup.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/538184631/hwsetup.o.d" -o ${OBJECTDIR}/_ext/538184631/hwsetup.o ../../src/hwsetup_lib/hwsetup.c   
	
${OBJECTDIR}/_ext/538184631/interrupts.o: ../../src/hwsetup_lib/interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/538184631 
	@${RM} ${OBJECTDIR}/_ext/538184631/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/538184631/interrupts.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/538184631/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/538184631/interrupts.o.d" -o ${OBJECTDIR}/_ext/538184631/interrupts.o ../../src/hwsetup_lib/interrupts.c   
	
${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o: ../../src/nvm_lib/NvmDrv_Pic32mx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d" -o ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o ../../src/nvm_lib/NvmDrv_Pic32mx.c   
	
${OBJECTDIR}/_ext/4456416/NVMem.o: ../../src/nvm_lib/NVMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/NVMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/NVMem.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/NVMem.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/NVMem.o.d" -o ${OBJECTDIR}/_ext/4456416/NVMem.o ../../src/nvm_lib/NVMem.c   
	
${OBJECTDIR}/_ext/4456416/Triggers.o: ../../src/nvm_lib/Triggers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/Triggers.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/Triggers.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/Triggers.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/Triggers.o.d" -o ${OBJECTDIR}/_ext/4456416/Triggers.o ../../src/nvm_lib/Triggers.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o: ../../src/QcAllplay/allplay_crc-ccitt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o ../../src/QcAllplay/allplay_crc-ccitt.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_protocol.o: ../../src/QcAllplay/allplay_protocol.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o ../../src/QcAllplay/allplay_protocol.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_receiver.o: ../../src/QcAllplay/allplay_receiver.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o ../../src/QcAllplay/allplay_receiver.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o: ../../src/QcAllplay/allplay_receiver_simulator.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o ../../src/QcAllplay/allplay_receiver_simulator.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o: ../../src/QcAllplay/allplay_stream_pic32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o ../../src/QcAllplay/allplay_stream_pic32.c   
	
${OBJECTDIR}/_ext/2118930945/tp_uart.o: ../../src/uart_lib/tp_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2118930945 
	@${RM} ${OBJECTDIR}/_ext/2118930945/tp_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/2118930945/tp_uart.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2118930945/tp_uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2118930945/tp_uart.o.d" -o ${OBJECTDIR}/_ext/2118930945/tp_uart.o ../../src/uart_lib/tp_uart.c   
	
${OBJECTDIR}/_ext/2118930945/uart_basic.o: ../../src/uart_lib/uart_basic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2118930945 
	@${RM} ${OBJECTDIR}/_ext/2118930945/uart_basic.o.d 
	@${RM} ${OBJECTDIR}/_ext/2118930945/uart_basic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2118930945/uart_basic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2118930945/uart_basic.o.d" -o ${OBJECTDIR}/_ext/2118930945/uart_basic.o ../../src/uart_lib/uart_basic.c   
	
${OBJECTDIR}/_ext/1593686579/assert.o: ../../src/util_lib/assert.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/assert.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/assert.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/assert.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/assert.o.d" -o ${OBJECTDIR}/_ext/1593686579/assert.o ../../src/util_lib/assert.c   
	
${OBJECTDIR}/_ext/1593686579/hex.o: ../../src/util_lib/hex.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/hex.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/hex.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/hex.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/hex.o.d" -o ${OBJECTDIR}/_ext/1593686579/hex.o ../../src/util_lib/hex.c   
	
${OBJECTDIR}/_ext/1593686579/TimerUtil.o: ../../src/util_lib/TimerUtil.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/TimerUtil.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d" -o ${OBJECTDIR}/_ext/1593686579/TimerUtil.o ../../src/util_lib/TimerUtil.c   
	
${OBJECTDIR}/_ext/1593686579/util.o: ../../src/util_lib/util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/util.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/util.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/util.o.d" -o ${OBJECTDIR}/_ext/1593686579/util.o ../../src/util_lib/util.c   
	
${OBJECTDIR}/_ext/1593686579/dbgprint.o: ../../src/util_lib/dbgprint.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/dbgprint.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/dbgprint.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/dbgprint.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/dbgprint.o.d" -o ${OBJECTDIR}/_ext/1593686579/dbgprint.o ../../src/util_lib/dbgprint.c   
	
${OBJECTDIR}/_ext/1593686579/ui.o: ../../src/util_lib/ui.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/ui.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/ui.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/ui.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/ui.o.d" -o ${OBJECTDIR}/_ext/1593686579/ui.o ../../src/util_lib/ui.c   
	
${OBJECTDIR}/_ext/1593686579/md5.o: ../../src/util_lib/md5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/md5.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/md5.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/md5.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/md5.o.d" -o ${OBJECTDIR}/_ext/1593686579/md5.o ../../src/util_lib/md5.c   
	
${OBJECTDIR}/_ext/1445274692/BootLoader.o: ../../src/BootLoader.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/BootLoader.o.d 
	@${RM} ${OBJECTDIR}/_ext/1445274692/BootLoader.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/BootLoader.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE) -g -D__DEBUG -D__MPLAB_DEBUGGER_ICD3=1 -fframe-base-loclist  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1445274692/BootLoader.o.d" -o ${OBJECTDIR}/_ext/1445274692/BootLoader.o ../../src/BootLoader.c   
	
else
${OBJECTDIR}/_ext/538184631/hwsetup.o: ../../src/hwsetup_lib/hwsetup.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/538184631 
	@${RM} ${OBJECTDIR}/_ext/538184631/hwsetup.o.d 
	@${RM} ${OBJECTDIR}/_ext/538184631/hwsetup.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/538184631/hwsetup.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/538184631/hwsetup.o.d" -o ${OBJECTDIR}/_ext/538184631/hwsetup.o ../../src/hwsetup_lib/hwsetup.c   
	
${OBJECTDIR}/_ext/538184631/interrupts.o: ../../src/hwsetup_lib/interrupts.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/538184631 
	@${RM} ${OBJECTDIR}/_ext/538184631/interrupts.o.d 
	@${RM} ${OBJECTDIR}/_ext/538184631/interrupts.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/538184631/interrupts.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/538184631/interrupts.o.d" -o ${OBJECTDIR}/_ext/538184631/interrupts.o ../../src/hwsetup_lib/interrupts.c   
	
${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o: ../../src/nvm_lib/NvmDrv_Pic32mx.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o.d" -o ${OBJECTDIR}/_ext/4456416/NvmDrv_Pic32mx.o ../../src/nvm_lib/NvmDrv_Pic32mx.c   
	
${OBJECTDIR}/_ext/4456416/NVMem.o: ../../src/nvm_lib/NVMem.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/NVMem.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/NVMem.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/NVMem.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/NVMem.o.d" -o ${OBJECTDIR}/_ext/4456416/NVMem.o ../../src/nvm_lib/NVMem.c   
	
${OBJECTDIR}/_ext/4456416/Triggers.o: ../../src/nvm_lib/Triggers.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/4456416 
	@${RM} ${OBJECTDIR}/_ext/4456416/Triggers.o.d 
	@${RM} ${OBJECTDIR}/_ext/4456416/Triggers.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/4456416/Triggers.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/4456416/Triggers.o.d" -o ${OBJECTDIR}/_ext/4456416/Triggers.o ../../src/nvm_lib/Triggers.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o: ../../src/QcAllplay/allplay_crc-ccitt.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_crc-ccitt.o ../../src/QcAllplay/allplay_crc-ccitt.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_protocol.o: ../../src/QcAllplay/allplay_protocol.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_protocol.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_protocol.o ../../src/QcAllplay/allplay_protocol.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_receiver.o: ../../src/QcAllplay/allplay_receiver.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_receiver.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_receiver.o ../../src/QcAllplay/allplay_receiver.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o: ../../src/QcAllplay/allplay_receiver_simulator.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_receiver_simulator.o ../../src/QcAllplay/allplay_receiver_simulator.c   
	
${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o: ../../src/QcAllplay/allplay_stream_pic32.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2112209336 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d 
	@${RM} ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o.d" -o ${OBJECTDIR}/_ext/2112209336/allplay_stream_pic32.o ../../src/QcAllplay/allplay_stream_pic32.c   
	
${OBJECTDIR}/_ext/2118930945/tp_uart.o: ../../src/uart_lib/tp_uart.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2118930945 
	@${RM} ${OBJECTDIR}/_ext/2118930945/tp_uart.o.d 
	@${RM} ${OBJECTDIR}/_ext/2118930945/tp_uart.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2118930945/tp_uart.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2118930945/tp_uart.o.d" -o ${OBJECTDIR}/_ext/2118930945/tp_uart.o ../../src/uart_lib/tp_uart.c   
	
${OBJECTDIR}/_ext/2118930945/uart_basic.o: ../../src/uart_lib/uart_basic.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/2118930945 
	@${RM} ${OBJECTDIR}/_ext/2118930945/uart_basic.o.d 
	@${RM} ${OBJECTDIR}/_ext/2118930945/uart_basic.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/2118930945/uart_basic.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/2118930945/uart_basic.o.d" -o ${OBJECTDIR}/_ext/2118930945/uart_basic.o ../../src/uart_lib/uart_basic.c   
	
${OBJECTDIR}/_ext/1593686579/assert.o: ../../src/util_lib/assert.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/assert.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/assert.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/assert.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/assert.o.d" -o ${OBJECTDIR}/_ext/1593686579/assert.o ../../src/util_lib/assert.c   
	
${OBJECTDIR}/_ext/1593686579/hex.o: ../../src/util_lib/hex.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/hex.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/hex.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/hex.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/hex.o.d" -o ${OBJECTDIR}/_ext/1593686579/hex.o ../../src/util_lib/hex.c   
	
${OBJECTDIR}/_ext/1593686579/TimerUtil.o: ../../src/util_lib/TimerUtil.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/TimerUtil.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/TimerUtil.o.d" -o ${OBJECTDIR}/_ext/1593686579/TimerUtil.o ../../src/util_lib/TimerUtil.c   
	
${OBJECTDIR}/_ext/1593686579/util.o: ../../src/util_lib/util.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/util.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/util.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/util.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/util.o.d" -o ${OBJECTDIR}/_ext/1593686579/util.o ../../src/util_lib/util.c   
	
${OBJECTDIR}/_ext/1593686579/dbgprint.o: ../../src/util_lib/dbgprint.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/dbgprint.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/dbgprint.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/dbgprint.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/dbgprint.o.d" -o ${OBJECTDIR}/_ext/1593686579/dbgprint.o ../../src/util_lib/dbgprint.c   
	
${OBJECTDIR}/_ext/1593686579/ui.o: ../../src/util_lib/ui.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/ui.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/ui.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/ui.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/ui.o.d" -o ${OBJECTDIR}/_ext/1593686579/ui.o ../../src/util_lib/ui.c   
	
${OBJECTDIR}/_ext/1593686579/md5.o: ../../src/util_lib/md5.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1593686579 
	@${RM} ${OBJECTDIR}/_ext/1593686579/md5.o.d 
	@${RM} ${OBJECTDIR}/_ext/1593686579/md5.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1593686579/md5.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1593686579/md5.o.d" -o ${OBJECTDIR}/_ext/1593686579/md5.o ../../src/util_lib/md5.c   
	
${OBJECTDIR}/_ext/1445274692/BootLoader.o: ../../src/BootLoader.c  nbproject/Makefile-${CND_CONF}.mk
	@${MKDIR} ${OBJECTDIR}/_ext/1445274692 
	@${RM} ${OBJECTDIR}/_ext/1445274692/BootLoader.o.d 
	@${RM} ${OBJECTDIR}/_ext/1445274692/BootLoader.o 
	@${FIXDEPS} "${OBJECTDIR}/_ext/1445274692/BootLoader.o.d" $(SILENT) -rsi ${MP_CC_DIR}../  -c ${MP_CC}  $(MP_EXTRA_CC_PRE)  -x c -c -mprocessor=$(MP_PROCESSOR_OPTION)  -ffunction-sections -mips16 -mno-float -Os -DPIC32_POLK_CAMDEN_SQUARE -DBOOTLOADER -I"../../src/Driver" -I"../../src/hwsetup_lib" -I"../../src/QcAllplay" -I"../../src/uart_lib" -I"../../src/util_lib" -I"../../src" -I"../../src/nvm_lib" -I"../../../../project_files/polk_allplay/pic32/include" -I"../../../../bsp/PIC32/include" -Werror -MMD -MF "${OBJECTDIR}/_ext/1445274692/BootLoader.o.d" -o ${OBJECTDIR}/_ext/1445274692/BootLoader.o ../../src/BootLoader.c   
	
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: compileCPP
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
else
endif

# ------------------------------------------------------------------------------------
# Rules for buildStep: link
ifeq ($(TYPE_IMAGE), DEBUG_RUN)
dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk    ../../src_proj_specific/bl_PolkAllplay/btl_32MX450F256H_generic.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mdebugger -D__MPLAB_DEBUGGER_ICD3=1 -mprocessor=$(MP_PROCESSOR_OPTION) -Os -mips16 -mno-float -o dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}           -mreserve=data@0x0:0x1FC -mreserve=boot@0x1FC02000:0x1FC02FEF -mreserve=boot@0x1FC02000:0x1FC0275F  -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--defsym=__MPLAB_DEBUG=1,--defsym=__DEBUG=1,--defsym=__MPLAB_DEBUGGER_ICD3=1,--gc-sections,-L"../../MPLAB_Workspace",-Map="${ImageDir}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem
	
else
dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${OUTPUT_SUFFIX}: ${OBJECTFILES}  nbproject/Makefile-${CND_CONF}.mk   ../../src_proj_specific/bl_PolkAllplay/btl_32MX450F256H_generic.ld
	@${MKDIR} dist/${CND_CONF}/${IMAGE_TYPE} 
	${MP_CC} $(MP_EXTRA_LD_PRE)  -mprocessor=$(MP_PROCESSOR_OPTION) -Os -mips16 -mno-float -o dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} ${OBJECTFILES_QUOTED_IF_SPACED}          -Wl,--defsym=__MPLAB_BUILD=1$(MP_EXTRA_LD_POST)$(MP_LINKER_FILE_OPTION),--gc-sections,-L"../../MPLAB_Workspace",-Map="${ImageDir}/${PROJECTNAME}.${IMAGE_TYPE}.map",--report-mem
	${MP_CC_DIR}\\xc32-bin2hex dist/${CND_CONF}/${IMAGE_TYPE}/bl_PolkAllplay.${IMAGE_TYPE}.${DEBUGGABLE_SUFFIX} 
endif


# Subprojects
.build-subprojects:


# Subprojects
.clean-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/debug
	${RM} -r dist/debug

# Enable dependency checking
.dep.inc: .depcheck-impl

DEPFILES=$(shell mplabwildcard ${POSSIBLE_DEPFILES})
ifneq (${DEPFILES},)
include ${DEPFILES}
endif
