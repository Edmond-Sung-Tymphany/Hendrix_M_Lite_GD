#ifndef __I2C_H
#define __I2C_H

void I2C_Configuration(void);
void I2C_ReadS_24C(uint8_t addr ,uint8_t* pBuffer,uint16_t no);
void I2C_Standby_24C(void);
void I2C_ByteWrite_24C(uint8_t addr,uint8_t dat);
void I2C_PageWrite_24C(uint8_t addr,uint8_t* pBuffer, uint8_t no);
void I2C_WriteS_24C(uint8_t addr,uint8_t* pBuffer,  uint16_t no);
void I2C_Test(void);
void Delay(uint32_t nCount);

#endif
