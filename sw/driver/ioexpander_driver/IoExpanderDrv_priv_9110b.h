/**
 * @file        IoExpanderDrv_priv.h
 * @brief       This file implements the driver for AW9110B
 * @author      Wesley Lee
 * @date        2015-11-20
 * @copyright   Tymphany Ltd.
 */
#ifndef IOEXPANDERDRV_PRIV_H
#define IOEXPANDERDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "IoExpanderDrv.h"
#define IO_EXPANDER_PORT_A      0
#define IO_EXPANDER_PORT_B      1
#define IO_EXPANDER_BIT_0       0
#define IO_EXPANDER_BIT_1       1
#define IO_EXPANDER_BIT_2       2
#define IO_EXPANDER_BIT_3       3
#define IO_EXPANDER_BIT_4       4
#define IO_EXPANDER_BIT_5       5
#define IO_EXPANDER_BIT_6       6
#define IO_EXPANDER_BIT_7       7

#define LED_MODE                0
#define GPIO_MODE               1
/* AW9110B I2C address */
/* --------------------------------------------------------------------- */
#define AW9110B_ADDR                (0xB0)
#define AW9110B_ADDR_A1_MASK        (0x04)
#define AW9110B_ADDR_A0_MASK        (0x02)

/* AW9110B register */
/* --------------------------------------------------------------------- */
#define AW9110B_GPIO_INPUT_A_REG    (0x00)  /**< reading OUT4 ~ OUT9 status */
#define AW9110B_GPIO_INPUT_B_REG    (0x01)  /**< reading OUT0 ~ OUT3 status */
#define AW9110B_GPIO_OUTPUT_A_REG   (0x02)  /**< R/W the OUT4 ~ OUT9 output status */
                                            /**< SMART-FADE: OUT4 ~ OUT5 could be fade-in/fade-out */
#define AW9110B_GPIO_OUTPUT_B_REG   (0x03)  /**< R/W the OUT0 ~ OUT3 output status */
                                            /**< SMART-FADE: OUT0 ~ OUT3 could be fade-in/fade-out */
#define AW9110B_GPIO_CFG_A_REG      (0x04)  /**< setting the IO mode for OUT4 ~ OUT9 */
                                            /**< With Breath enabled, OUT4 ~ OUT5 could be BLINK or SMART-FADE*/
#define AW9110B_GPIO_CFG_B_REG      (0x05)  /**< setting the IO mode for OUT0 ~ OUT3 */
                                            /**< With Breath enabled, OUT0 ~ OUT3 could be BLINK or SMART-FADE*/
#define AW9110B_GPIO_INTN_A_REG     (0x06)  /**< interrupt enable for OUT4 ~ OUT9 */
#define AW9110B_GPIO_INTN_B_REG     (0x07)  /**< interrupt enable for OUT0 ~ OUT3 */

#define AW9110B_CTL_REG             (0x11)  /**< control register */
#define AW9110B_GPMD_A_REG          (0x12)  /**< LED/GPIO Mode Switch for OUT4 ~ OUT9 */
#define AW9110B_GPMD_B_REG          (0x13)  /**< LED/GPIO Mode Switch for OUT0 ~ OUT3 */

#define AW9110B_EN_BRE_REG          (0x14)  /**< Enable Breathing for OUT0 ~ OUT5 */
#define AW9110B_FADE_TMR_REG        (0x15)  /**< Fade in/out timer setting under BLINK or SMART-FADE */
#define AW9110B_FULL_TMR_REG        (0x16)  /**< Full on/off timer setting under BLINK or SMART-FADE */

#define AW9110B_DLY0_BRE_REG        (0x17)  /**< Delay timer for OUT0 under BLINK mode */
#define AW9110B_DLY1_BRE_REG        (0x18)  /**< Delay timer for OUT1 under BLINK mode */
#define AW9110B_DLY2_BRE_REG        (0x19)  /**< Delay timer for OUT2 under BLINK mode */
#define AW9110B_DLY3_BRE_REG        (0x1A)  /**< Delay timer for OUT3 under BLINK mode */
#define AW9110B_DLY4_BRE_REG        (0x1B)  /**< Delay timer for OUT4 under BLINK mode */
#define AW9110B_DLY5_BRE_REG        (0x1C)  /**< Delay timer for OUT5 under BLINK mode */

/* Register for setting the brightness of individual pin*/
#define AW9110B_DIM0_REG            (0x20)
#define AW9110B_DIM1_REG            (0x21)
#define AW9110B_DIM2_REG            (0x22)
#define AW9110B_DIM3_REG            (0x23)
#define AW9110B_DIM4_REG            (0x24)
#define AW9110B_DIM5_REG            (0x25)
#define AW9110B_DIM6_REG            (0x26)
#define AW9110B_DIM7_REG            (0x27)
#define AW9110B_DIM8_REG            (0x28)
#define AW9110B_DIM9_REG            (0x29)

#define AW9110B_RESET_REG           (0x7F)  /**< Software Reset Register with writing 0x00 */

/* AW9110B register value */
/* --------------------------------------------------------------------- */
/* @sa  AW9110B_GPIO_INPUT_A_REG */
#define AW9110B_GPIO_INPUT_A5       (0x20)  /**< input state of OUT9 */
#define AW9110B_GPIO_INPUT_A4       (0x10)  /**< input state of OUT8 */
#define AW9110B_GPIO_INPUT_A3       (0x08)  /**< input state of OUT7 */
#define AW9110B_GPIO_INPUT_A2       (0x04)  /**< input state of OUT6 */
#define AW9110B_GPIO_INPUT_A1       (0x02)  /**< input state of OUT5 */
#define AW9110B_GPIO_INPUT_A0       (0x01)  /**< input state of OUT4 */

/* @sa  AW9110B_GPIO_INPUT_B_REG */
#define AW9110B_GPIO_INPUT_B3       (0x08)  /**< input state of OUT3 */
#define AW9110B_GPIO_INPUT_B2       (0x04)  /**< input state of OUT2 */
#define AW9110B_GPIO_INPUT_B1       (0x02)  /**< input state of OUT1 */
#define AW9110B_GPIO_INPUT_B0       (0x01)  /**< input state of OUT0 */

/* @sa  AW9110B_GPIO_OUTPUT_A_REG */
#define AW9110B_GPIO_OUTPUT_A5      (0x20)  /**< output state of OUT9 */
#define AW9110B_GPIO_OUTPUT_A4      (0x10)  /**< output state of OUT8 */
#define AW9110B_GPIO_OUTPUT_A3      (0x08)  /**< output state of OUT7 */
#define AW9110B_GPIO_OUTPUT_A2      (0x04)  /**< output state of OUT6 */
#define AW9110B_GPIO_OUTPUT_A1      (0x02)  /**< output state of OUT5
                                                 if (GPMD_A1 == 0) && (0EN_BRE5 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */
#define AW9110B_GPIO_OUTPUT_A0      (0x01)  /**< output state of OUT4
                                                 if (GPMD_A0 == 0) && (0EN_BRE4 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */

/* @sa  AW9110B_GPIO_OUTPUT_B_REG */
#define AW9110B_GPIO_OUTPUT_B3      (0x08)  /**< output state of OUT3
                                                 if (GPMD_B3 == 0) && (0EN_BRE3 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */
#define AW9110B_GPIO_OUTPUT_B2      (0x04)  /**< output state of OUT2
                                                 if (GPMD_B2 == 0) && (0EN_BRE2 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */
#define AW9110B_GPIO_OUTPUT_B1      (0x02)  /**< output state of OUT1
                                                 if (GPMD_B1 == 0) && (0EN_BRE1 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */
#define AW9110B_GPIO_OUTPUT_B0      (0x01)  /**< output state of OUT0
                                                 if (GPMD_B0 == 0) && (0EN_BRE0 == 1), SMART-FADE mode
                                                 0 -> 1: Fade-in 
                                                 1 -> 0: Fade-out */

/* @sa  AW9110B_GPIO_CFG_A_REG, AW9110B_GPIO_CFG_B_REG */
#define AW9110B_GPIO_CFG_INPUT      (1)
#define AW9110B_GPIO_CFG_OUTPUT     (0)
#define AW9110B_GPIO_CFG_A_OUT4_OUT5_MASK   (0x03)
#define AW9110B_GPIO_CFG_B_OUT0_OUT3_MASK   (0x0F)

// with GPMD == 0 and EN_BRE == 1
#define AW9110B_GPIO_CFG_SMART_FADE (0)
#define AW9110B_GPIO_CFG_BLINK      (1)

/* @sa  AW9110B_GPIO_INTN_A_REG, AW9110B_GPIO_INTN_B_REG */
#define AW9110B_GPIO_INT_ENABLE     (0)
#define AW9110B_GPIO_INT_DISABLE    (1)


/* @sa  AW9110B_CTL_REG */
#define AW9110B_CTL_GO              (0x80)  /**< Under BLINK mode, write 1 as breath enable */
#define AW9110B_CTL_GPOMD_MASK      (0x10)
#define AW9110B_CTL_GPOMD_OPEN_DRAIN (0x00)
#define AW9110B_CTL_GPOMD_PUSH_PULL (0x10)
#define AW9110B_CTL_ISEL_MASK       (0x03)
#define AW9110B_CTL_ISEL_MAX        (0x00)
#define AW9110B_CTL_ISEL_3_QUARTERS (0x01)
#define AW9110B_CTL_ISEL_HALF       (0x02)
#define AW9110B_CTL_ISEL_QUARTER    (0x03)

/* @sa  AW9110B_GPMD_A_REG, AW9110B_GPMD_B_REG */
#define AW9110B_GPMD_GPIO           (1)
#define AW9110B_GPMD_LED            (0)

/* @sa  AW9110B_EN_BRE_REG */
#define AW9110B_BREATH_ENABLE       (1)
#define AW9110B_BREATH_DISABLE      (0)

/* @sa  AW9110B_FADE_TMR_REG */
#define AW9110B_FDOFF_TMR_MASK      (0x38)
#define AW9110B_FDOFF_TMR_0MS       (0x00)
#define AW9110B_FDOFF_TMR_315MS     (0x08)
#define AW9110B_FDOFF_TMR_630MS     (0x10)
#define AW9110B_FDOFF_TMR_1260MS    (0x18)
#define AW9110B_FDOFF_TMR_2520MS    (0x20)
#define AW9110B_FDOFF_TMR_5040MS    (0x28)

#define AW9110B_FDON_TMR_MASK       (0x07)
#define AW9110B_FDON_TMR_0MS        (0x00)
#define AW9110B_FDON_TMR_315MS      (0x01)
#define AW9110B_FDON_TMR_630MS      (0x02)
#define AW9110B_FDON_TMR_1260MS     (0x03)
#define AW9110B_FDON_TMR_2520MS     (0x04)
#define AW9110B_FDON_TMR_5040MS     (0x05)

/* @sa  AW9110B_FULL_TMR_REG */
#define AW9110B_FLOFF_TMR_MASK      (0x38)
#define AW9110B_FLOFF_TMR_0MS       (0x00)
#define AW9110B_FLOFF_TMR_315MS     (0x08)
#define AW9110B_FLOFF_TMR_630MS     (0x10)
#define AW9110B_FLOFF_TMR_1260MS    (0x18)
#define AW9110B_FLOFF_TMR_2520MS    (0x20)
#define AW9110B_FLOFF_TMR_5040MS    (0x28)
#define AW9110B_FLOFF_TMR_10080MS   (0x30)
#define AW9110B_FLOFF_TMR_20160MS   (0x38)

#define AW9110B_FLON_TMR_MASK       (0x07)
#define AW9110B_FLON_TMR_0MS        (0x00)
#define AW9110B_FLON_TMR_315MS      (0x01)
#define AW9110B_FLON_TMR_630MS      (0x02)
#define AW9110B_FLON_TMR_1260MS     (0x03)
#define AW9110B_FLON_TMR_2520MS     (0x04)
#define AW9110B_FLON_TMR_5040MS     (0x05)
#define AW9110B_FLON_TMR_10080MS    (0x06)
#define AW9110B_FLON_TMR_20160MS    (0x07)

/* @sa  AW9110B_DLY0_BRE_REG, AW9110B_DLY1_BRE_REG, AW9110B_DLY2_BRE_REG,
        AW9110B_DLY3_BRE_REG, AW9110B_DLY4_BRE_REG, AW9110B_DLY5_BRE_REG */
#define AW9110B_DLY_TMR_0MS         (0x00)
#define AW9110B_DLY_TMR_256MS       (0x01)
#define AW9110B_DLY_TMR_65280MS     (0xFF)

/* @sa  AW9110B_DIM0_REG, AW9110B_DIM1_REG, AW9110B_DIM2_REG, W9110B_DIM3_REG, 
        AW9110B_DIM4_REG, AW9110B_DIM5_REG, AW9110B_DIM6_REG, W9110B_DIM7_REG, 
        AW9110B_DIM8_REG, AW9110B_DIM9_REG */
#define AW9110B_DIM_MAX_LEVEL       (0xFF)

/* @sa  AW9110B_RESET_REG */
#define AW9110B_SW_RESET_VAULE      (0x00)

/* --------------------------------------------------------------------- */
/* private functions / data */
#define AW9110B_NUM_BLINK           (6)     // Number of Blink channel available

#ifdef __cplusplus
}
#endif

#endif /* IOEXPANDERDRV_PRIV_H */
