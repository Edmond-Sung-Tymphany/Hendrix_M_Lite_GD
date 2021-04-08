#ifndef __ADAU1452_DRV_PRIV_H__
#define __ADAU1452_DRV_PRIV_H__

#define ADI_REG_TYPE    static const uint8_t

#define DSP_MAX_DELAY_SAMPLES       9600

/* DSP chip address */
#define SAFE_LOAD_DATA_START_ADD              MOD_SAFELOADMODULE_DATA_SAFELOAD0_ADDR
#define SAFE_LOAD_ADD_FOR_TARGET_ADD          MOD_SAFELOADMODULE_ADDRESS_SAFELOAD_ADDR
#define SAFE_LOAD_SET_DATA_SIZE_ADD           MOD_SAFELOADMODULE_NUM_SAFELOAD_ADDR
#define SAFE_LOAD_DATA_SIZE_MAX               5


typedef struct tagAdau1452Drv
{
    bool        isReady;
    bool        i2cEnable;
    cI2CDrv     i2cDrv;
}Adau1452Drv_t;

// sigma studio function
static void SIGMA_WRITE_REGISTER_BLOCK(uint8_t device_add, uint16_t reg_add, uint16_t bytes, const uint8_t* data);
static void Adau1452Drv_I2CWrite(uint16_t reg_add, uint16_t bytes, const uint8_t* data);

#endif  // __ADAU1452_DRV_PRIV_H__


