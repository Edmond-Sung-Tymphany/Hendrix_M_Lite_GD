/**
* @file attachedDevices.h
* @brief The devices attached to the product.
* @author Christopher Alexander
* @date 26-Nov-2013
* @copyright Tymphany Ltd.
*/

/* TODO: Auto generate this data */
#ifndef ATTACHEDDEVICES_H
#define ATTACHEDDEVICES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "deviceTypes_v2.h"

/*******************************************************************************
 *  General settings configs go here
 ******************************************************************************/
// SAM will check PRODUCT_VERSION_NUM to determine upgrade or not.
#define PRODUCT_VERSION "BnO SoundWall"

/*******************************************************************************
 *  DSP configs go here
 ******************************************************************************/
#define DSP_VERSION     "N/A "
/*******************************************************************************
 *  Key configs go here
 ******************************************************************************/
/* For ADC PIN */
#define NUM_OF_ADC_NTC_PIN              3
#define NUM_OF_ADC_HW_VER_PIN           1
#define NUM_OF_ALL_ENABLED_ADC_PIN      (NUM_OF_ADC_NTC_PIN + NUM_OF_ADC_HW_VER_PIN)

// power control
#define GPIO_OUT_MAIN_POWER         GPIO_0
#define GPIO_OUT_DSP_POWER          GPIO_1
#define GPIO_OUT_A2B_POWER          GPIO_2
#define GPIO_OUT_5V_POWER           GPIO_3
#define GPIO_IN_POWER_LOST          GPIO_31

// dsp control
#define GPIO_OUT_DSP_RESET          GPIO_4
#define GPIO_OUT_DSP_SELFBOOT       GPIO_5
#define GPIO_IN_DSP_SELFBOOT        GPIO_OUT_DSP_SELFBOOT
// reserve GPIO_6

// amplifier control
#define GPIO_OUT_AMP_RESET          GPIO_7
#define GPIO_IN_AMP_CLP             GPIO_8
#define GPIO_IN_AMP_FAULT           GPIO_9
// reserve GPIO_10/11

// codec control
#define GPIO_OUT_CODEC_RESET        GPIO_12
#define GPIO_IN_CODEC_INT           GPIO_13
// reserve GPIO_14

// music/audio/jack detect
#define GPIO_IN_DET_AUX             GPIO_15
#define GPIO_IN_DET_SPDIF           GPIO_16
#define GPIO_IN_PL_INSERT           GPIO_17
#define GPIO_IN_DSP_TUNING          GPIO_18
// OP mute control
#define GPIO_OUT_OP_MUTE            GPIO_19
// reserve GPIO_20/21

#define GPIO_OUT_LED_RESET          GPIO_22

#ifdef MCU_COMM_VIA_GPIO
#define GPIO_OUT_A2B_OUTPUT_1       GPIO_23
#define GPIO_OUT_A2B_OUTPUT_2       GPIO_24
#define GPIO_IN_A2B_INPUT_1         GPIO_25
#define GPIO_IN_A2B_INPUT_2         GPIO_26
#endif

extern const tDevice * const devices[];
extern const uint8 NUM_OF_ATTACHED_DEVICES;

/* temp place for version line */
#define SOFTWARE_VERSION_STRING     PRODUCT_VERSION

/**
* UART Driver hardware pin initialization
* @param[in]    id              UART Driver ID (ref: enum eUartDevice)
*/
void UartDrv_Init(eTpUartDevice id);

/**
* UART Driver hardware pin de-initialization
* @param[in]    id              UART Driver ID (ref: enum eUartDevice)
*/
void UartDrv_Deinit(eTpUartDevice id);

/*
* I2C GPIO configuration (calls by I2CDrv_Ctor() from I2CDrv.c)
* init SDL and SDA pins according project
*/
void I2C1_LowLevel_Init();
void I2C2_LowLevel_Init(void);
#ifdef HAS_I2C_SLAVE
void I2C1_Slave_LowLevel_Init(void);
void I2C2_Slave_LowLevel_Init(void);
#endif

void I2C2_GPIO_Deinit(void);
void I2C2_GPIO_ReInit(void);

#ifdef __cplusplus
}
#endif

#endif /* ATTACHEDDEVICES_H */
