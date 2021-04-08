#ifndef __IoeLedDrv_PRIV_H__
#define __IoeLedDrv_PRIV_H__

#define IOE_REG_GPIO_OUTPUT_B   0x03
#define IOE_REG_GPIO_CFG_A      0x04    // out4->out9, input/output or smart-fade/blink select
#define IOE_REG_GPIO_CFG_B      0x05    // out0->out3, input/output or smart-fade/blink select
#define IOE_REG_GPIO_INT_A      0x06    // out4->out9, interrupt 0:enable, 1:disable
#define IOE_REG_GPIO_INT_B      0x07    // out0->out3, interrupt 0:enable, 1:disable
#define IOE_REG_CONTROL         0x11    // global control, blink mode start(GO)
    #define IOE_CONTROL_ISEL_MAX        0x00
    #define IOE_CONTROL_ISEL_3_QUARTERS 0x01
    #define IOE_CONTROL_ISEL_HALF       0x02
    #define IOE_CONTROL_ISEL_QUARTER    0x03
#define IOE_REG_GPMD_A          0x12    // out4->out9 Led/Gpio mode select, 0:led mode, 1:gpio mode
#define IOE_REG_GPMD_B          0x13    // out0->out3 Led/Gpio mode select, 0:led mode, 1:gpio mode
#define IOE_REG_EN_BREATH       0x14    // out0->out5 breath mode enable/disable
#define IOE_REG_FADE_TIMER      0x15    // fade on/off timer setup
#define IOE_REG_FULL_TIMER      0x16    // full on/off timer setup
#define IOE_REG_DIM0            0x20

// fade in/out timer for blink mode
#define FADE_ON_TIMER_0ms        0x00
#define FADE_ON_TIMER_256ms      0x01
#define FADE_ON_TIMER_512ms      0x02
#define FADE_ON_TIMER_1024ms     0x03
#define FADE_ON_TIMER_2048ms     0x04
#define FADE_ON_TIMER_4096ms     0x05
#define FADE_OFF_TIMER_0ms        0x00
#define FADE_OFF_TIMER_256ms      0x08
#define FADE_OFF_TIMER_512ms      0x10
#define FADE_OFF_TIMER_1024ms     0x18
#define FADE_OFF_TIMER_2048ms     0x20
#define FADE_OFF_TIMER_4096ms     0x28
// full on/off timer for blink mode
#define FULL_OFF_TIMER_0ms        0x00
#define FULL_OFF_TIMER_256ms      0x01
#define FULL_OFF_TIMER_512ms      0x02
#define FULL_OFF_TIMER_1024ms     0x03
#define FULL_OFF_TIMER_2048ms     0x04
#define FULL_OFF_TIMER_4096ms     0x05
#define FULL_OFF_TIMER_8192ms     0x06
#define FULL_OFF_TIMER_16384ms    0x07
#define FULL_ON_TIMER_0ms        0x00
#define FULL_ON_TIMER_256ms      0x08
#define FULL_ON_TIMER_512ms      0x10
#define FULL_ON_TIMER_1024ms     0x18
#define FULL_ON_TIMER_2048ms     0x20
#define FULL_ON_TIMER_4096ms     0x28
#define FULL_ON_TIMER_8192ms     0x30
#define FULL_ON_TIMER_16384ms    0x38

typedef struct tagIoeLedDrv
{
    bool        isReady;
    bool        i2cEnable;
#ifdef IOE_LED_VIA_HW_I2C    
    cI2CDrv     i2cDrv;
#endif
#ifdef IOE_LED_VIA_SW_I2C
    cSWi2cDrv_t i2cDrv;
#endif
}IoeLedDrv_t;

typedef struct tagIoeLedMode
{
    uint8_t     mode;   // MSB 4bits for master mode, LSB 4bits for blink mode
    uint8_t     outX;   // point to the outX
    uint8_t     dim_reg;
    uint8_t     dim_value;
}IoeLedMode_t;

#endif  // __IoeLedDrv_PRIV_H__
