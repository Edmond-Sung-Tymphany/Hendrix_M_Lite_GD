#ifndef __SPI_FLASH_PRIV_H__
#define __SPI_FLASH_PRIV_H__

// write enable
#define FLASH_CMD_WREN          0x06
// write disable
#define FLASH_CMD_WRDI          0x04
// write status register
#define FLASH_CMD_WRSR          0x01
// read identification
#define FLASH_CMD_RDID          0x9f
// read status register
#define FLASH_CMD_RDSR          0x05
// read data
#define FLASH_CMD_READ          0x03
// fast read data
#define FLASH_CMD_FAST_READ     0x0b
// read SFDP
#define FLASH_CMD_RDSFDP        0x5a
// read electronic ID
#define FLASH_CMD_RES           0xab
// read electronic manufacturer & device ID
#define FLASH_CMD_REMS          0x90
// double output mode command
#define FLASH_CMD_DREAD         0x3b
// sector erase
#define FLASH_CMD_SE            0x20
// block erase, 0x52 or 0xd8
#define FLASH_CMD_BE            0x52
// chip erase, 0x60 or 0xc7
#define FLASH_CMD_CE            0x60
// page program
#define FLASH_CMD_PP            0x02
// read security register
#define FLASH_CMD_RDSCUR        0x2b
// write security register
#define FLASH_CMD_WRSCUR        0x2f
// enter secured OTP
#define FLASH_CMD_ENSO          0xb1
// exit secured OTP
#define FLASH_CMD_EXSO          0xc1
// deep power down
#define FLASH_CMD_DP            0xb9
// release deep power down
#define FLASH_CMD_RDP           0xab

/*!< Write In Progress (WIP) flag */
#define FLASH_FLAG_WIP          0x01


#endif  //__SPI_FLASH_PRIV_H__

