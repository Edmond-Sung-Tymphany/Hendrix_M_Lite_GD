#ifndef __ADAU1761_DRV_PRIV_H__
#define __ADAU1761_DRV_PRIV_H__

#define ADI_REG_TYPE    static const uint8_t

#define ADAU1761_MAX_TX_BYTES   80

/* DSP chip address */
#define SAFE_LOAD_MODULO_SIZE_ADD             (0x0000)
#define SAFE_LOAD_DATA_START_ADD              (0x0001)
#define SAFE_LOAD_ADD_FOR_TARGET_ADD          (0x0006)
#define SAFE_LOAD_SET_DATA_SIZE_ADD           (0x0007)
#define SAFE_LOAD_DATA_SIZE_MAX               (5)


typedef struct tagAdau1761Drv
{
    bool        isReady;
    bool        i2cEnable;
    cI2CDrv     i2cDrv;
}Adau1761Drv_t;

/* 
 * control register define
 */
/* ClkCtrlRegister  - Registers (IC 1) */
#define CLKCTRLREGISTER_ADDR                  (0x4000)

/* RegPowCtrlRegister  - Registers (IC 1) */
#define REGPOWCTRLREGISTER_ADDR               (0x4001)

/* PLLCrlRegister  - Registers (IC 1) */
#define PLLCTRLREGISTER_ADDR                  (0x4002)

/* MicCtrlRegister  - Registers (IC 1) */
#define MICCTRLREGISTER_ADDR                  (0x4008)

/* Record Pwr Management  - Registers (IC 1) */
#define RECORD_PWR_MANAGEMENT_ADDR            (0x4009)

/* Record Mixer Left Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_0_ADDR         (0x400A)

/* Record Mixer Left Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_LEFT_CTRL_1_ADDR         (0x400B)

/* Record Mixer Right Ctrl 0  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_0_ADDR        (0x400C)

/* Record Mixer Right Ctrl 1  - Registers (IC 1) */
#define RECORD_MIXER_RIGHT_CTRL_1_ADDR        (0x400D)

/* Record Volume Ctrl Left  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_LEFT_ADDR          (0x400E)

/* Record Volume Ctrl Right  - Registers (IC 1) */
#define RECORD_VOLUME_CTRL_RIGHT_ADDR         (0x400F)

/* Record Mic Bias Control  - Registers (IC 1) */
#define RECORD_MIC_BIAS_CONTROL_ADDR          (0x4010)

/* ALC Control 0  - Registers (IC 1) */
#define ALC_CONTROL_0_ADDR                    (0x4011)

/* ALC Control 1  - Registers (IC 1) */
#define ALC_CONTROL_1_ADDR                    (0x4012)

/* ALC Control 2  - Registers (IC 1) */
#define ALC_CONTROL_2_ADDR                    (0x4013)

/* ALC Control 3  - Registers (IC 1) */
#define ALC_CONTROL_3_ADDR                    (0x4014)

/* Serial Port Control 0  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_0_ADDR            (0x4015)

/* Serail Port Control 1  - Registers (IC 1) */
#define SERIAL_PORT_CONTROL_1_ADDR            (0x4016)

/* Converter Ctrl 0  - Registers (IC 1) */
#define CONVERTER_CTRL_0_ADDR                 (0x4017)

/* Converter Ctrl 1  - Registers (IC 1) */
#define CONVERTER_CTRL_1_ADDR                 (0x4018)

/* ADC Control 0  - Registers (IC 1) */
#define ADC_CONTROL_0_ADDR                    (0x4019)

/* ADC Control 1  - Registers (IC 1) */
#define ADC_CONTROL_1_ADDR                    (0x401A)

/* ADC Control 2  - Registers (IC 1) */
#define ADC_CONTROL_2_ADDR                    (0x401B)

/* Playback Mixer Left Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_0_ADDR    (0x401C)

/* Plaback Mixer Left Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_LEFT_CONTROL_1_ADDR    (0x401D)

/* Plaback Mixer Right Control 0  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_0_ADDR   (0x401E)

/* Playback Mixer Right Control 1  - Registers (IC 1) */
#define PLAYBACK_MIXER_RIGHT_CONTROL_1_ADDR   (0x401F)

/* Playback Mixer LR Left  - Registers (IC 1)  Page 71 */
#define PLAYBACK_LR_LEFT_ADDR                 (0x4020)

/* Playback Mixer LR Right  - Registers (IC 1) Page 71 */
#define PLAYBACK_LR_RIGHT_ADDR                (0x4021)

/* Playback LR Mono Ctrl  - Registers (IC 1) */
#define PLAYBACK_LR_MONO_CTRL_ADDR            (0x4022)

/* Playback Headphone Left  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_LEFT_ADDR          (0x4023)

/* Playback Headphone Right  - Registers (IC 1) */
#define PLAYBACK_HEADPHONE_RIGHT_ADDR         (0x4024)

/* Playback Line Out Left  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_LEFT_ADDR           (0x4025)

/* Playback Line Out Right  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_RIGHT_ADDR          (0x4026)

/* Playback Line Out Mono  - Registers (IC 1) */
#define PLAYBACK_LINE_OUT_MONO_ADDR           (0x4027)

/* Playback Control  - Registers (IC 1) */
#define PLAYBACK_CONTROL_ADDR                 (0x4028)

/* Playback Power Management  - Registers (IC 1) */
#define PLAYBACK_POWER_MANAGEMENT_ADDR        (0x4029)

/* DAC Control 0  - Registers (IC 1) */
#define DAC_CONTROL_0_ADDR                    (0x402A)

/* DAC Control 1  - Registers (IC 1) */
#define DAC_CONTROL_1_ADDR                    (0x402B)

/* DAC Control 2  - Registers (IC 1) */
#define DAC_CONTROL_2_ADDR                    (0x402C)

/* Serial Port Pad Control 0  - Registers (IC 1) */
#define SERIAL_PORT_PAD_CONTROL_0_ADDR        (0x402D)

/* Comm Port Pad Ctrl 0  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_0_ADDR             (0x402F)

/* Comm Port Pad Ctrl 1  - Registers (IC 1) */
#define COMM_PORT_PAD_CTRL_1_ADDR             (0x4030)

/* JackRegister  - Registers (IC 1) */
#define JACKREGISTER_ADDR                     (0x4031)

/* Dejitter Register Control  - Registers (IC 1) */
#define DEJITTER_REGISTER_CONTROL_ADDR        (0x4036)

/* CRC Ideal_1  - Registers (IC 1) */
#define CRC_IDEAL_1_ADDR                      (0x40C0)

/* CRC Ideal_2  - Registers (IC 1) */
#define CRC_IDEAL_2_ADDR                      (0x40C1)

/* CRC Ideal_3  - Registers (IC 1) */
#define CRC_IDEAL_3_ADDR                      (0x40C2)

/* CRC Ideal_4  - Registers (IC 1) */
#define CRC_IDEAL_4_ADDR                      (0x40C3)

/* CRC Enable  - Registers (IC 1) */
#define CRC_ENABLE_ADDR                       (0x40C4)

/* GPIO 0 Control  - Registers (IC 1) */
#define GPIO_0_CONTROL_ADDR                   (0x40C6)

/* GPIO 1 Control  - Registers (IC 1) */
#define GPIO_1_CONTROL_ADDR                   (0x40C7)

/* GPIO 2 Control  - Registers (IC 1) */
#define GPIO_2_CONTROL_ADDR                   (0x40C8)

/* GPIO 3 Control  - Registers (IC 1) */
#define GPIO_3_CONTROL_ADDR                   (0x40C9)

/* Watchdog_Enable  - Registers (IC 1) */
#define WATCHDOG_ENABLE_ADDR                  (0x40D0)

/* Watchdog Register Value 1  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_1_ADDR        (0x40D1)

/* Watchdog Register Value 2  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_2_ADDR        (0x40D2)

/* Watchdog Register Value 3  - Registers (IC 1) */
#define WATCHDOG_REGISTER_VALUE_3_ADDR        (0x40D3)

/* Watchdog Error  - Registers (IC 1) */
#define WATCHDOG_ERROR_ADDR                   (0x40D4)

/* Non Modulo RAM 1  - Registers (IC 1) */
#define NON_MODULO_RAM_1_ADDR                 (0x40E9)

/* Non Modulo RAM 2  - Registers (IC 1) */
#define NON_MODULO_RAM_2_ADDR                 (0x40EA)

/* Sample Rate Setting  - Registers (IC 1) */
#define SAMPLE_RATE_SETTING_ADDR              (0x40EB)

/* Routing Matrix Inputs  - Registers (IC 1) */
#define ROUTING_MATRIX_INPUTS_ADDR            (0x40F2)

/* Routing Matrix Outputs  - Registers (IC 1) */
#define ROUTING_MATRIX_OUTPUTS_ADDR           (0x40F3)

/* Serial Data/GPIO Pin Config  - Registers (IC 1) */
#define SERIAL_DATAGPIO_PIN_CONFIG_ADDR       (0x40F4)

/* DSP Enable Register  - Registers (IC 1) */
#define DSP_ENABLE_REGISTER_ADDR              (0x40F5)

/* DSP Run Register  - Registers (IC 1) */
#define DSP_RUN_REGISTER_ADDR                 (0x40F6)

/* DSP Slew Modes  - Registers (IC 1) */
#define DSP_SLEW_MODES_ADDR                   (0x40F7)

/* Serial Port Sample Rate Setting  - Registers (IC 1) */
#define SERIAL_PORT_SAMPLE_RATE_SETTING_ADDR  (0x40F8)

/* Clock Enable Reg 0  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_0_ADDR               (0x40F9)

/* Clock Enable Reg 1  - Registers (IC 1) */
#define CLOCK_ENABLE_REG_1_ADDR               (0x40FA)

static void Adau1761Drv_InitStage0(void);
static void Adau1761Drv_InitStage1(void);
static void Adau1761Drv_InitStage2(void);

// sigma studio function
static void SIGMA_WRITE_REGISTER_BLOCK(uint8_t device_add, uint16_t reg_add, uint16_t bytes, const uint8_t* data);

#endif  // __ADAU1761_DRV_PRIV_H__

