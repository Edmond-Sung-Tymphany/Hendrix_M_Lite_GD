#ifndef __SPI_FLASH_DRV_H__
#define __SPI_FLASH_DRV_H__

/**
 * Construct the driver instance.
 * @return : none
 */
void SpiFlashDrv_Ctor(void);

/**
 * Exit & clean up the driver.
 */
void SpiFlashDrv_Xtor(void);

/**
 * get the manufacturer & device ID
 * @return : combine the ID
 */
uint32_t SpiFlashDrv_GetDeviceID();

/**
 * erase one sector
 * @return : TRUE if success, FALSE if failure
 */
bool SpiFlashDrv_EraseSector(uint32_t addr);

/**
 * read n bytes from the flash
 * @return : NONE
 */
void SpiFlashDrv_ReadBytes(uint32_t addr, uint32_t len, uint8_t *p_buf);

/**
 * write n bytes to the flash
 * @NOTE : do NOT cross the page
 * @return : NONE
 */
void SpiFlashDrv_WriteBytes(uint32_t addr, uint32_t len, uint8_t *p_buf);

#endif  //__SPI_FLASH_DRV_H__

