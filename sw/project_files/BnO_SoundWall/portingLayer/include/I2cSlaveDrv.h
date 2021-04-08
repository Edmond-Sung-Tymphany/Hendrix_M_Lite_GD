#ifndef __I2C_SLAVE_DRV_H__
#define __I2C_SLAVE_DRV_H__

typedef enum tagI2cReadCmd
{
    I2C_READ_CMD_NONE=0,
    I2C_READ_CMD_ITEM_NO,     // Item No.
    I2C_READ_CMD_TYPE_NO,     // Type No.
    I2C_READ_CMD_SERIAL_NO,   // Serial No.
    I2C_READ_CMD_HW_VER,
    I2C_READ_CMD_BTL_VER,
    I2C_READ_CMD_APP_VER,
    I2C_READ_CMD_DSP_VER,
    I2C_READ_CMD_NTC,
    I2C_READ_CMD_DSP_DELAY,   // GAIN_AND_DELAY
    I2C_READ_CMD_DSP_GAIN,    // DRIVER_GAIN
    I2C_READ_CMD_DSP_ROOMEQ,  // BASS_AND_ROOMEQ
    I2C_READ_CMD_END
}I2cReadCmd_t;

typedef enum tagI2cWriteCmd
{
    I2C_WRITE_CMD_NONE=0,
    I2C_WRITE_CMD_ENTER_DFU,
    I2C_WRITE_CMD_STANDBY,
    I2C_WRITE_CMD_MUTE_AMP,
    I2C_WRITE_CMD_TEST_TONE,
    I2C_WRITE_CMD_DSP_DELAY,   // GAIN_AND_DELAY
    I2C_WRITE_CMD_DSP_GAIN,    // DRIVER_GAIN
    I2C_WRITE_CMD_DSP_ROOMEQ,  // BASS_AND_ROOMEQ
    I2C_WRITE_CMD_DSP_UPDATE,  // write the gain/delay/roomEQ parameter to FLASH
    I2C_WRITE_CMD_END
}I2cWriteCmd_t;

typedef struct tagI2cA2bMsg
{
    void        *p_value;   // point to the message data
}I2cA2bMsg_t;

typedef struct tagI2cSlaveRxEvt
{
    QEvt super;
    QActive * sender;
    int32_t cmd;
    uint32_t *p_data;
}I2cSlaveRxEvt;

void I2cSlaveDrv_Ctor(void);
void I2cSlaveDrv_Xtor(void);
void I2cSlaveDrv_Init(void);
void I2cSlaveDrv_Deinit(void);
void I2cSlaveDrv_Service(void);

#endif  // __I2C_SLAVE_DRV_H__

