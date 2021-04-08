#ifndef __EpHDMI_DRV_H__
#define __EpHDMI_DRV_H__

#define EpHDMI_REG_POWER        0x10
#define EpHDMI_REG_RX_CH        0x11
#define EpHDMI_REG_CEC_VOL      0x13

#define EpHDMI_CEC_DEFAULT_VOLUME   0x10

/**
 * Construct the Ep9xAx HDMI driver instance.
 * @return : none
 */
void EpHDMIDrv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void EpHDMIDrv_Xtor(void);

/**
 * HDMI in port select, port0/port1/port2
 * @return : none
 */
void EpHDMIDrv_Port(uint8_t port);

/**
 * CEC on/off control : 0->off, 1/others->on
 * @return : none
 */
void EpHDMIDrv_CEC_Enable(uint32_t enable);

/**
 * ARC on/off control : 0->off, 1/others->on
 * @return : none
 */
void EpHDMIDrv_ARC_Enable(uint32_t enable);

/**
 * Get the CEC volume value.
 * @return : CEC volume value. range 0~100, ERROR:0xff
 */
uint8_t EpHDMIDrv_GetCEC_Volume(void);



#endif  // __EpHDMI_DRV_H__

