#ifndef __I2C_SERVER_H__
#define __I2C_SERVER_H__


#define I2C_SRV_TIMEOUT_IN_MS      10


typedef struct tagI2cSrv
{
	cServer super_; 
}cI2cSrv;

void I2cSrv_Enable(bool enable);
void I2cSrv_StartUp(cPersistantObj *me);
void I2cSrv_ShutDown(cPersistantObj *me);

#endif  // __I2C_SERVER_H__

