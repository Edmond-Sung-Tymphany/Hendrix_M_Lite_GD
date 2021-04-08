#ifndef __PCM1690_DRIVER_H__
#define __PCM1690_DRIVER_H__

/**
 * Construct the PCM1690 driver instance.
 * @return : none
 */
void PCM1690Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void PCM1690Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void PCM1690Drv_I2cEnable(bool enable);


/**
 * brief : PCM1690 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void PCM1690Drv_Init(void);

#endif  // __PCM1690_DRIVER_H__

