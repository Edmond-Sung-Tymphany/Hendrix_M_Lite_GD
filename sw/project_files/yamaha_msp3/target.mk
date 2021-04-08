
############################################################################
# TO BE GENEREATED
############################################################################
    TP_PRODUCT      :=SVS_14_ULTRA_SB
TP_MCU_FAMILY   :=stm32
TARGET      ?= rel






ICF_FILE    := project_files\\$(TP_PRODUCT)\\iar\\link.icf
CPPFLAGS    += -DUSE_STDPERIPH_DRIVER -DSTM32F0XX
TARGET_FLAGS+= --no_cse --no_unroll --no_inline --no_code_motion    \
    --no_tbaa --no_clustering --no_scheduling --debug --endian=little       \
    --cpu=Cortex-M0 --enum_is_int -e --fpu=None -On
LDFLAGS     += --redirect _Printf=_PrintfFull --redirect _Scanf=_ScanfFull         \
    --config $(ICF_FILE) --semihosting --entry __iar_program_start --vfe
LD_MAP      += --map $(MAP_FILE)
STRIPFLAGS  += --ihex


# Platform fundamental files


plat_file   := bsp/$(TP_MCU_FAMILY)/bsp.c                           \
    common/persistantObj.c                                          \
    common/controller/controller.c                                  \
    hardware_management/deviceTypes.c                               \
    project_files/$(TP_PRODUCT)/main.c                              \
    project_files/$(TP_PRODUCT)/MainApp.c                           \
    project_files/$(TP_PRODUCT)/$(TP_MCU_FAMILY)/attachedDevices.c  \
    project_files/$(TP_PRODUCT)/$(TP_MCU_FAMILY)/projBsp.c          \

# include path
include_dirs = include                                      \
    common/qp/include                                       \
    driver/include                                          \
    hardware_management/include                             \
    server/include                                          \
    ui_layer/include                                        \
    project_files/$(TP_PRODUCT)                             \
    project_files/$(TP_PRODUCT)/conf                        \
    project_files/$(TP_PRODUCT)/DSP                         \
    project_files/$(TP_PRODUCT)/$(TP_MCU_FAMILY)/include

ifeq "$(TP_MCU_FAMILY)" "stm32"
include_dirs += bsp/stm32/include                           \
    bsp/stm32/Libraries/CMSIS/Include                       \
    bsp/stm32/Libraries/CMSIS/ST/STM32F0xx/Include          \
    bsp/stm32/Libraries/STM32F0xx_StdPeriph_Driver/inc
endif

# Product Config 
TP_FEATURE  += HAS_AUDIO_CONTROL
TP_FEATURE  += HAS_KEYS
TP_FEATURE  += HAS_SETTING
TP_FEATURE  += HAS_DISPLAY
TP_FEATURE  += HAS_TM1629
TP_FEATURE  += HAS_DEBUG
TP_FEATURE  += HAS_ADC_KEY
TP_FEATURE  += HAS_GPIO_KEY
TP_FEATURE  += HAS_NEC_IR
TP_FEATURE  += HAS_ADAU1761
TP_FEATURE  += HAS_DELEGATES
TP_FEATURE  += EXTERNAL_HIGH_SPEED_CLOCK
TP_FEATURE  += REMEMBER_VOLUME
TP_FEATURE  += HAS_NVM
TP_FEATURE  += ENABLE_SETTING_UPDATE_PUBLISH
TP_FEATURE  += SETTING_HAS_ROM_DATA
TP_FEATURE  += HAS_MENU
TP_FEATURE  += HAS_SCREEN_DIM_CTRL
