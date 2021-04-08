/**
 * @file        IoExpanderDrv_priv.h
 * @brief       This file implements the driver for AW9523B
 * @author      Wesley Lee
 * @date        2014-06-06
 * @copyright   Tymphany Ltd.
 */
#ifndef IOEXPANDERDRV_PRIV_H
#define IOEXPANDERDRV_PRIV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "IoExpanderDrv.h"

/* AW9523B register */
#define AW9523B_INPUT_PORT0_REG     (0x00)  /**< reading the Port0 GPIO input status */
#define AW9523B_INPUT_PORT1_REG     (0x01)  /**< reading the Port1 GPIO input status */
#define AW9523B_OUTPUT_PORT0_REG    (0x02)  /**< R/W the Port0 GPIO output status */
#define AW9523B_OUTPUT_PORT1_REG    (0x03)  /**< R/W the Port1 GPIO output status */
#define AW9523B_CONFIG_PORT0_REG    (0x04)  /**< setting the IO mode for Port0 */
#define AW9523B_CONFIG_PORT1_REG    (0x05)  /**< setting the IO mode for Port1 */
#define AW9523B_INT_EN_PORT0_REG    (0x06)  /**< setting the interrupt enable for Port0 */
#define AW9523B_INT_EN_PORT1_REG    (0x07)  /**< setting the interrupt enable for Port1 */

#define AW9523B_ID_REG              (0x10)  /**< reading the ID, i.e. 0x23 */
#define AW9523B_CTL_REG             (0x11)  /**< control register */
#define AW9523B_LED_MODE_PORT0_REG  (0x12)  /**< LED Mode Switch for Port0*/
#define AW9523B_LED_MODE_PORT1_REG  (0x13)  /**< LED Mode Switch for Port1*/

/* Register for setting the brightness of individual pin*/
#define AW9523B_P0_0_DIM_REG        (0x24)
#define AW9523B_P0_1_DIM_REG        (0x25)
#define AW9523B_P0_2_DIM_REG        (0x26)
#define AW9523B_P0_3_DIM_REG        (0x27)
#define AW9523B_P0_4_DIM_REG        (0x28)
#define AW9523B_P0_5_DIM_REG        (0x29)
#define AW9523B_P0_6_DIM_REG        (0x2A)
#define AW9523B_P0_7_DIM_REG        (0x2B)
#define AW9523B_P1_0_DIM_REG        (0x20)
#define AW9523B_P1_1_DIM_REG        (0x21)
#define AW9523B_P1_2_DIM_REG        (0x22)
#define AW9523B_P1_3_DIM_REG        (0x23)
#define AW9523B_P1_4_DIM_REG        (0x2C)
#define AW9523B_P1_5_DIM_REG        (0x2D)
#define AW9523B_P1_6_DIM_REG        (0x2E)
#define AW9523B_P1_7_DIM_REG        (0x2F)

#define AW9523B_DIM_MAX_LEVEL       (0xFF)

#define AW9523B_SW_RSTN_REG         (0x7F)  /**< Software Reset Register */

/* AW9523B register reference value */
#define AW9523B_ID                  (0x23)  /**< @sa AW9523B_ID_REG */

/* @sa AW9523B_CTL_REG */
#define AW9523B_CTL_GPOMD           (0x10)
#define AW9523B_CTL_GPOMD_OPEN_DRAIN (0x00)
#define AW9523B_CTL_GPOMD_PUSH_PULL (0x10)
#define AW9523B_CTL_ISEL            (0x03)
#define AW9523B_CTL_ISEL_MAX        (0x00)
#define AW9523B_CTL_ISEL_3_QUARTERS (0x01)
#define AW9523B_CTL_ISEL_HALF       (0x02)
#define AW9523B_CTL_ISEL_QUARTER    (0x03)

/* @sa AW9523B_LED_MODE_PORT0_REG, AW9523B_LED_MODE_PORT1_REG */
#define AW9523B_LED_MODE_GPIO       1
#define AW9523B_LED_MODE_LED        0

/* @sa AW9523B_SW_RSTN_REG */
#define AW9523B_SW_RSTN_VAULE       (0x00)

/* private functions / data */
static void IoExpanderDrv_I2cWrite_aw9523(cIoExpanderDrv *me, uint8 *data, uint8 length);
#ifdef AW9523B_USE_GPIO_SETTING
static void IoExpanderDrv_I2cRead_aw9523(cIoExpanderDrv *me, uint8 reg, uint8* value);
#endif
#ifdef __cplusplus
}
#endif

#endif /* IOEXPANDERDRV_PRIV_H */
