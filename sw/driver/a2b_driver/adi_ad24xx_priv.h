#ifndef ADI_AD2410_PRIV_H
#define ADI_AD2410_PRIV_H

static uint32 adi_a2b_slave_I2C_Read(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nRegAddress,uint8* pData);
static uint32 adi_a2b_slave_I2C_Write(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nRegAddress,uint8 nData);


#endif
