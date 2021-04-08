#include "deviceTypes_v2.h"
#include "KeySrv.h"
#include "ldc2114.h"
#include "I2CDrv.h"
#include "Ldc2114.Config"
#include "setting_id.h"

#define DRV_LDC2114_DBG(x)// printf x
#define DRV_LDC2114_ERR(x) printf x

#define TOUCH_DATA_OUT_DISABLED (0)
#define TOUCH_DATA_OUT_ENABLED  (1)
#define TOUCH_DATA_OUT_SIZE     (18)
#define TOUCH_DATA_OUT_HEADER   ((TOUCH_DATA_OUT_SIZE << 8) | 16 )//(0x1210), tunnel command Id 16, size 18

int8 ldc2114_i2c_bus_read(cI2CDrv* ldc2114I2cDrv, uint8 reg_addr, uint8 *reg_data, uint32 cnt)
{
    if ((NULL == ldc2114I2cDrv) || (NULL == reg_data) || (0 == cnt))
    {
        DRV_LDC2114_DBG(("ldc2114_i2c_bus_read:input illegal\n"));
        return -1;
    }

    tI2CMsg i2cMsg  =
    {
        .devAddr = ldc2114I2cDrv->pConfig->devAddress,
        .regAddr = reg_addr,
        .length  = cnt,
        .pMsg    = (uint8*)reg_data,
    };


    if(TP_SUCCESS != I2CDrv_MasterRead(ldc2114I2cDrv, &i2cMsg))
    {
        DRV_LDC2114_DBG(("ldc2114_i2c_bus_read:I2C read error\n"));
        return -1;
    }

    return TP_SUCCESS;
}

int8 ldc2114_i2c_bus_write(cI2CDrv* ldc2114I2cDrv, uint8 reg_addr, uint8 *reg_data, uint8 cnt)
{
    uint8 temp[256] = {0};
    if ((NULL == ldc2114I2cDrv) || (NULL == reg_data) || (0 == cnt))
    {
        DRV_LDC2114_DBG(("ldc2114_i2c_bus_read:input illegal\n"));
        return -1;
    }

    temp[0] = reg_addr;
    memcpy(&temp[1], reg_data, cnt);


    tI2CMsg i2cMsg  =
    {
        .devAddr = ldc2114I2cDrv->pConfig->devAddress,
        .regAddr = 0,
        .length  = cnt+1,
        .pMsg    = (uint8*)temp,
    };

    if(TP_SUCCESS != I2CDrv_MasterWrite(ldc2114I2cDrv, &i2cMsg))
    {
        DRV_LDC2114_DBG(("ldc2114_i2c_bus_write:I2C read error\n"));
        return -1;
    }

    return TP_SUCCESS;

}

int8 ldc2114_write_register(cI2CDrv* ldc2114I2cDrv, uint8 reg_addr, uint8 reg_value)
{
    uint8 value;

    value = reg_value;
    ldc2114_i2c_bus_write(ldc2114I2cDrv, reg_addr, &value, REGISTER_DATA_SIZE);
}

int8 ldc2114_read_register(cI2CDrv* ldc2114I2cDrv, uint8 reg_addr, uint8* reg_value)
{
    ldc2114_i2c_bus_read(ldc2114I2cDrv, reg_addr, reg_value, REGISTER_DATA_SIZE);
}

int32 Ldc2114TouchKeyDrv_GetRawData(cKeyDrv *me)
{
    int16 rawData;
    uint8 data[3];

    bool isDown;

    cLdc2114TouchKeyDrv *toucKeyDrv = (cLdc2114TouchKeyDrv *) me;

    ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, OUT_ADDR, &data[2], REGISTER_DATA_SIZE);

    ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, DATA0_LSB_ADDR + (2 * toucKeyDrv->seqNo), &data[0], REGISTER_DATA_SIZE);
    ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, DATA0_MSB_ADDR + (2 * toucKeyDrv->seqNo), &data[1], REGISTER_DATA_SIZE);

    rawData = (data[1] << 8) | data[0];
    if ((rawData & 0x800) != 0)
    {
        rawData = rawData & ~0x800;
        rawData = rawData ^ 0x7FF;
        rawData = rawData * (-1);
    }

    isDown = (data[2] & (1 << toucKeyDrv->seqNo)) ? TRUE : FALSE;

    return isDown;
    //return rawData;
}

void Ldc2114TouchKeyDrv_Update(cKeyDrv *me)
{
    int16 rawData;
    uint8 data[3] = {0};
    bool isDown;
    uint8 status;
    static uint32 kCounter[8] = {0};
    static int16 rawSensorData[9] = {0};
    uint8 dataOutControl = TOUCH_DATA_OUT_DISABLED;
    rawSensorData[0] = TOUCH_DATA_OUT_HEADER;

    uint8 i2cState = I2C_STATE_OK;

#ifdef HAS_I2C_ERROR_RECOVERY
    i2cState = *(uint8*)Setting_GetEx(SETID_LED_I2C_STATE, &i2cState);
#endif
    cLdc2114TouchKeyDrv *toucKeyDrv = (cLdc2114TouchKeyDrv *) me;

    if (toucKeyDrv->i2cDrv.isReady && (I2C_STATE_OK == i2cState))
    {
        kCounter[me->keyID - TI_TOUCH_KEY_0]++;

        uint32 timeStart = getSysTime();

        if (TP_SUCCESS != ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, STATUS_ADDR, &status, REGISTER_DATA_SIZE))
        {
            DRV_LDC2114_ERR(("\nTouch read fail[%d]", toucKeyDrv->seqNo));

#ifdef HAS_I2C_ERROR_RECOVERY
            toucKeyDrv->i2cDrv.isReady = FALSE;

            uint8 i2cState = I2C_STATE_RECOVER_PHASE_1;
            Setting_Set(SETID_LED_I2C_STATE, &i2cState);
#endif
        }

        DRV_LDC2114_DBG(("\nStatus:[%d]", status));

        ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, OUT_ADDR, &data[2], REGISTER_DATA_SIZE);

        ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, DATA0_LSB_ADDR + (2 * toucKeyDrv->seqNo), &data[0], REGISTER_DATA_SIZE);
        ldc2114_i2c_bus_read(&toucKeyDrv->i2cDrv, DATA0_MSB_ADDR + (2 * toucKeyDrv->seqNo), &data[1], REGISTER_DATA_SIZE);

        rawData = (data[1] << 8) | data[0];
        if ((rawData & 0x800) != 0)
        {
            rawData = rawData & ~0x800;
            rawData = rawData ^ 0x7FF;
            rawData = rawData * (-1);
        }

        isDown = (data[2] & (1 << toucKeyDrv->seqNo)) ? TRUE : FALSE;

        if (0 == kCounter[me->keyID - TI_TOUCH_KEY_0] % 10)
        {
            DRV_LDC2114_DBG(("\nB[%d]:[%d][%d]", me->keyID - TI_TOUCH_KEY_0, rawData,(isDown) ? 1 : 0));

            rawSensorData[me->keyID - TI_TOUCH_KEY_0 + 1] = rawData;

            if (TI_TOUCH_KEY_7 == me->keyID)
            {
                DRV_LDC2114_DBG(("\n[%d][%d][%d][%d][%d][%d][%d][%d]\n", rawSensorData[1],
                                                                         rawSensorData[2],
                                                                         rawSensorData[3],
                                                                         rawSensorData[4],
                                                                         rawSensorData[5],
                                                                         rawSensorData[6],
                                                                         rawSensorData[7],
                                                                         rawSensorData[8]));

                dataOutControl = *(uint8*) Setting_Get(SETTID_TOUCH_RAW_DATA_TO_TUNNEL);

                if (TOUCH_DATA_OUT_ENABLED == dataOutControl)
                {
                    AseNgSrv_SendTunnel(rawSensorData, TOUCH_DATA_OUT_SIZE);
                }
            }
        }

        if (TRUE == isDown)
        {
            me->keyState = KEY_DOWN;
        }
        else
        {
            me->keyState = KEY_UP;
        }
    }
}

void Ldc2114TouchKeyDrv_Ctor(cLdc2114TouchKeyDrv *me, tTouchKeyboardDevice* cLdc2114TouchKeyDeviceDrv, eKeyID keyID, uint8 seqNo, uint8 gainSettId)
{
    DRV_LDC2114_DBG(("\nInit Ldc2114"));

    uint8 data = 0;
    uint8 dataOutControl = TOUCH_DATA_OUT_DISABLED;

    uint8 ldcRegDefault[LDC2114_CONFIG_LENGTH]={MAGIC_WORD_BYTE_0,    MAGIC_WORD_BYTE_1,    MAGIC_WORD_BYTE_2,    MAGIC_WORD_BYTE_3,
                                                EN_VALUE,             NP_SCAN_RATE_VALUE,   LP_SCAN_RATE_VALUE,   INTPOL_VALUE,
                                                LP_BASE_INC_VALUE,    NP_BASE_INC_VALUE,    MAXWIN_VALUE,         LC_DIVIDER_VALUE,
                                                HYST_VALUE,           TWIST_VALUE,          COMMON_DEFORM_VALUE,  OPOL_VALUE,
                                                CNTSC_VALUE,          SENSOR0_CONFIG_VALUE, SENSOR1_CONFIG_VALUE, SENSOR2_CONFIG_VALUE,
                                                SENSOR3_CONFIG_VALUE, FTF0_VALUE,           FTF1_2_VALUE,         FTF3_VALUE,
                                                MANUFACTURER_ID_LSB_VALUE, MANUFACTURER_ID_MSB_VALUE, DEVICE_ID_LSB_VALUE, DEVICE_ID_MSB_VALUE,
                                                GAIN0_DEFAULT_VALUE,  GAIN1_DEFAULT_VALUE,  GAIN2_DEFAULT_VALUE,  GAIN3_DEFAULT_VALUE};


    uint8 ldcAddr[LDC2114_DATA_LENGTH]={EN_ADDR,                  NP_SCAN_RATE_ADDR,        LP_SCAN_RATE_ADDR,   INTPOL_ADDR,
                                        LP_BASE_INC_ADDR,         NP_BASE_INC_ADDR,         MAXWIN_ADDR,         LC_DIVIDER_ADDR,
                                        HYST_ADDR,                TWIST_ADDR,               COMMON_DEFORM_ADDR,  OPOL_ADDR,
                                        CNTSC_ADDR,               SENSOR0_CONFIG_ADDR,      SENSOR1_CONFIG_ADDR, SENSOR2_CONFIG_ADDR,
                                        SENSOR3_CONFIG_ADDR,      FTF0_ADDR,                FTF1_2_ADDR,         FTF3_ADDR,
                                        MANUFACTURER_ID_LSB_ADDR, MANUFACTURER_ID_MSB_ADDR, DEVICE_ID_LSB_ADDR,  DEVICE_ID_MSB_ADDR,
                                        GAIN0_ADDR,               GAIN1_ADDR,               GAIN2_ADDR,          GAIN3_ADDR};

    me->super_.KeyGetRawDataCb = Ldc2114TouchKeyDrv_GetRawData;
    me->super_.KeyStartScanCb = NULL;
    me->super_.KeyUpdateStatusCb = Ldc2114TouchKeyDrv_Update;
    me->super_.keySimulationState = KEY_INVALIDE_STATE;
    me->super_.keyState = KEY_UP;
    me->super_.keyID = keyID;
    me->super_.isCreated = TRUE;

    me->seqNo = seqNo;

    I2CDrv_Ctor(&me->i2cDrv, cLdc2114TouchKeyDeviceDrv->i2cConfig);

    if (MASTER_KEY_ID == me->seqNo)
    {
        Setting_Set(SETTID_TOUCH_RAW_DATA_TO_TUNNEL, &dataOutControl);

        ldc2114_write_register(&me->i2cDrv,RESET_ADDR,RESET_FULL_RESET_VALUE);

        uint32 timeStart = getSysTime();

        while(((data & STATUS_CHIP_READY_VALUE) == 0) && (getSysTime() - timeStart < 100))
        {
            ldc2114_read_register(&me->i2cDrv,STATUS_ADDR, &data);
        }

        ldc2114_write_register(&me->i2cDrv,RESET_ADDR,RESET_RESET_REG_CONFIG_START_VALUE);

        timeStart = getSysTime();

        while(((data & STATUS_READY_TO_WRITE) == 0) && (getSysTime() - timeStart < 100))
        {
            ldc2114_read_register(&me->i2cDrv,STATUS_ADDR, &data);
        }

        uint8 *ldcRegFlash = (uint8*)Setting_Get(gainSettId);
        uint32 ldcHeader = ldcRegFlash[0] | (ldcRegFlash[1]<<8) | (ldcRegFlash[2]<<16) | (ldcRegFlash[3]<<24);
        uint8 *ldcReg = NULL;

        if(VALID_LDC_REG_VALUE != ldcHeader)
        {
            DRV_LDC2114_DBG(("\nVALID_LDC_REG_VALUE != ldcReg[0],   write the default register value\n"));
            ldcReg = ldcRegDefault;
            Setting_Set(gainSettId, ldcRegDefault);
        }
        else
        {
            DRV_LDC2114_DBG(("\nLDC2114 write the register data from flash\n"));
            ldcReg = ldcRegFlash;
        }

        for(uint8 i=0; i<LDC2114_DATA_LENGTH; i++)
        {
            ldc2114_write_register(&me->i2cDrv,ldcAddr[i], ldcReg[LDC2114_HEADER_LENGTH + i]);
        }

        ldc2114_write_register(&me->i2cDrv,RESET_ADDR,               RESET_RESET_REG_CONFIG_DONE_VALUE);
    }

#ifdef HAS_I2C_ERROR_RECOVERY
    uint8 i2cState = I2C_STATE_OK;
    Setting_Set(SETID_LED_I2C_STATE, &i2cState);
#endif
}

void Ldc2114TouchKeyDrv_Xtor(cLdc2114TouchKeyDrv *me)
{
    I2CDrv_Xtor(&me->i2cDrv);
    me->super_.isCreated = FALSE;
}

