#ifndef __CS8422_DRIVER_H__
#define __CS8422_DRIVER_H__

typedef enum tagCS8422Source
{
    CS8422_SOURCE_I2S_IN,
    CS8422_SOURCE_RX0,
    CS8422_SOURCE_RX1,
    CS8422_SOURCE_RX2,
    CS8422_SOURCE_RX3,
    CS8422_SOURCE_MAX
}CS8422Source_t;

/**
 * Construct the CS8422 driver instance.
 * @return : none
 */
void CS8422Drv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void CS8422Drv_Xtor(void);

/**
 * Enable/Disalbe the I2C bus access
 */
void CS8422Drv_I2cEnable(bool enable);


/**
 * brief : CS8422 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void CS8422Drv_Init(void);

/**
 * brief : CS8422 initial, write the necessary config register data to the chip.
 * @param me - instance of the driver
 * @return : none
 */
void CS8422Drv_SourceSelect(CS8422Source_t source);

#endif  // __CS8422_DRIVER_H__

