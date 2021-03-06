/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Sensor Driver(accelerometer and gyroscope tpe)
                  -------------------------

                  SW Module Document




@file        Bmi160KeyDrv.c
@brief       The key driver(accelerometer and gyroscope type) interfaces and implementation
@author      Paris Chen
@date        2017-02-17
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2017-02-17     Paris.Chen
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  :
-------------------------------------------------------------------------------
*/
#include "Bmi160KeyDrv.h"
#include "trace.h"
#include "Bsp.h"
#include "tym_math_accelerometer.h"
#include "Bmi160Drv.Config"
#include "setting_id.h"
#include "SettingSrv.h"
#include "KalmanFilter.Config"

//#define DEBUG_DRIVER_BMI160
#ifdef DEBUG_DRIVER_BMI160
#define PRINT_ONCE_EVERY_10MS 250
#define DRV_BMI160_DBG(x) printf x
#else
#define DRV_BMI160_DBG(x)
#endif

#define BMI160_RET_SUCCESS (0)
#define BMI160_RET_FAIL    (-1)

#define BMI160_I2C_ADDRESS 0xD0

#define KALMAN_FILTER_EST_ERROR_LOOP 100
#define SENSOR_INIT_SEQ_DELAY_TIME 1

/*******************************************
 * Private Function Prototype              *
 *******************************************/
static void Bmi160KeyDrv_UpdateKeyStatus(cKeyDrv *me);
static int32 Bmi160KeyDrv_GetKeyDegree(cKeyDrv *me);

/*******************************************
 * bmi160 Variable Declaration            *
 *******************************************/
static cI2CDrv bmi160I2c;
/* Mapping the structure*/
static struct bmi160_t configbmi160;
/* Read the sensor data of accel, gyro and mag*/
static cSeq seq;

static BMI160_RETURN_FUNCTION_TYPE initResult = ERROR;

/*******************************************
 * bmi160 Function Prototype              *
 *******************************************/
static BMI160_RETURN_FUNCTION_TYPE bmi160_initialize_sensor(struct bmi160_t * s_bmi160);
static int8 i2c_routine(struct bmi160_t * s_bmi160);

static int8 bmi160_i2c_bus_read(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt);
static int8 bmi160_i2c_bus_read_burst(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint32 cnt);
static int8 bmi160_i2c_bus_write(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt);

static void Bmi160KeyDrv_InitSection0(void *me);
static void Bmi160KeyDrv_InitSection1(void *me);
static void Bmi160KeyDrv_InitSection2(void *me);
static void Bmi160KeyDrv_InitSection3(void *me);
static void Bmi160KeyDrv_InitSection4(void *me);
static void Bmi160KeyDrv_InitSection5(void *me);
static void Bmi160KeyDrv_InitSection6(void *me);
static void Bmi160KeyDrv_InitSection7(void *me);

static tSeqSection Bmi160KeyDrvInitSection[] =
{
    {&Bmi160KeyDrv_InitSection0,    0},
    {&Bmi160KeyDrv_InitSection1,    BMI160_GEN_READ_WRITE_DELAY},
    {&Bmi160KeyDrv_InitSection2,    BMI160_MODE_SWITCHING_DELAY},
    {&Bmi160KeyDrv_InitSection3,    BMI160_GEN_READ_WRITE_DELAY},
    {&Bmi160KeyDrv_InitSection4,    0},
    {&Bmi160KeyDrv_InitSection5,    BMI160_GEN_READ_WRITE_DELAY},
    {&Bmi160KeyDrv_InitSection6,    BMI160_GEN_READ_WRITE_DELAY},
    {&Bmi160KeyDrv_InitSection7,    0},
};

/**
* Key Driver(i2c type) object constructor
* @param[in]    me              the Key Driver object
* @param[in]    pConfig         configuration of the Key instance
* @param[in]    keyID           the keyID
*/
void Bmi160KeyDrv_Ctor(cBmi160KeyDrv *me, const tBmi160KeyboardDevice *pSensorBmi160KeyboardConfig)
{
    /* check parameters */
    ASSERT(me && pSensorBmi160KeyboardConfig);
    me->pBmi160KeyboardConfig = pSensorBmi160KeyboardConfig;
    me->dynamic_prev_degree = 0;
    me->dynamic_cal_current_time = 0;
    me->dynamic_cal_timeout = 0;

    me->super_.KeyGetRawDataCb = Bmi160KeyDrv_GetKeyDegree;
    me->super_.KeyStartScanCb = NULL;
    me->super_.KeyUpdateStatusCb = Bmi160KeyDrv_UpdateKeyStatus;
    me->super_.keySimulationState = KEY_INVALIDE_STATE;
    me->super_.keyState = KEY_UP;
    me->super_.keyID = INVALID_KEY;
    me->super_.isCreated = TRUE;
}

/**
* Key Driver(i2c type) object destructor
* @param[in]    me              the Key Driver object
*/
void Bmi160KeyDrv_Xtor(cBmi160KeyDrv *me)
{
    ASSERT(me);

    me->super_.KeyGetRawDataCb = NULL;
    me->super_.KeyStartScanCb = NULL;
    me->super_.KeyUpdateStatusCb = NULL;

    me->super_.keySimulationState = KEY_INVALIDE_STATE;
    me->super_.keyState = KEY_INVALIDE_STATE;
    me->super_.keyID = INVALID_KEY;
    me->super_.isCreated = FALSE;

    me->dynamic_prev_degree = 0;
    me->dynamic_cal_current_time = 0;
    me->dynamic_cal_timeout = 0;
    me->pBmi160KeyboardConfig = NULL;
}

static eTiltSensorMode Bmi160KeyDrv_CheckSensorMode(int32 acc_g_x)
{
    if((acc_g_x > BMI160_MODE_SWITCH_G_X_POS_VALUE) || (  acc_g_x < BMI160_MODE_SWITCH_G_X_NEG_VALUE))
    {
        return TILT_SENSOR_WALL_MODE;
    }
    else
    {
        return TILT_SENSOR_FLOOR_MODE;
    }
}

static bool Bmi160KeyDrv_CheckIsSensorLyingMode(int32 acc_g_z)
{
    if((acc_g_z > BMI160_LYING_MODE_G_Z_POS_VALUE) || (  acc_g_z < BMI160_LYING_MODE_G_Z_NEG_VALUE))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static bool Bmi160KeyDrv_CalDynamicOffset(cKeyDrv *me, int8 current_degree)
{
    cBmi160KeyDrv * pBmi160KeyObj = (cBmi160KeyDrv*)me;
    bool result = FALSE;

    if(pBmi160KeyObj->dynamic_prev_degree == current_degree)
    {
        pBmi160KeyObj->dynamic_cal_current_time = getSysTime();
        if(pBmi160KeyObj->dynamic_cal_timeout == 0)
        {
            pBmi160KeyObj->dynamic_cal_timeout = pBmi160KeyObj->dynamic_cal_current_time + BMI160_DYNAMIC_CAL_SYSTEM_RUNNING_MS_TIMEOUT;
        }
    }
    else
    {
        pBmi160KeyObj->dynamic_cal_timeout = 0;
    }

    if((pBmi160KeyObj->dynamic_cal_current_time >= pBmi160KeyObj->dynamic_cal_timeout)&&(pBmi160KeyObj->dynamic_cal_timeout != 0))
    {
        result = TRUE;
        pBmi160KeyObj->dynamic_cal_timeout = 0;
        DRV_BMI160_DBG(("***************** Enable Dynamic Offset Change , new offset = %d *************** \r\n", current_degree));
    }
    pBmi160KeyObj->dynamic_prev_degree = current_degree;
    return result;
}

static void Bmi160KeyDrv_UpdateKeyStatus(cKeyDrv *me)
{
    int16 acc_raw_x, acc_raw_y, acc_raw_z;
    int32 acc_g_x, acc_g_y, acc_g_z, acc_g_kf;
    int8 acc_degree, acc_kf_degree;
    cBmi160KeyDrv * pBmi160KeyObj;
    eTiltSensorMode TiltSensorWorkingMode;
#ifdef DEBUG_DRIVER_BMI160
    static uint8 print_count;;
    int8 ori_degree;
#endif

#ifdef HAS_I2C_ERROR_RECOVERY
    uint8 i2cState = I2C_STATE_OK;

    i2cState = *(uint8*)Setting_GetEx(SETID_LED_I2C_STATE, &i2cState);

    if (I2C_STATE_OK != i2cState)
    {
        return;
    }
#endif
    bmi160_read_accel_z(&acc_raw_z);
    acc_g_z = ( acc_raw_z * 1000)/((0x7fff + 1)/2);

    if(Bmi160KeyDrv_CheckIsSensorLyingMode(acc_g_z))
    {
        me->keyState = KEY_UP;
        return;
    }
    pBmi160KeyObj = (cBmi160KeyDrv*)me;

    bmi160_read_accel_x(&acc_raw_x);
    bmi160_read_accel_y(&acc_raw_y);

    acc_g_x = ( acc_raw_x * 1000)/((0x7fff + 1)/2);
    acc_g_y = ( acc_raw_y * 1000)/((0x7fff + 1)/2);

    TiltSensorWorkingMode = Bmi160KeyDrv_CheckSensorMode(acc_g_x);

    if(TILT_SENSOR_FLOOR_MODE == TiltSensorWorkingMode)
    {
        acc_g_kf = acc_g_x;
#ifdef DEBUG_DRIVER_BMI160
        print_count++;
        if(print_count > PRINT_ONCE_EVERY_10MS)
        {
            printf("Sensor Mode : SENSOR_FLOOR_MODE \r\n");
        }
#endif
    }
    else
    {
        acc_g_kf = acc_g_y;
#ifdef DEBUG_DRIVER_BMI160
        print_count++;
        if(print_count > PRINT_ONCE_EVERY_10MS)
        {
            printf("Sensor Mode : SENSOR_WALL_MODE \r\n");
        }
#endif
    }

    acc_degree = tym_math_calculate_degree_from_miliG(acc_g_kf);
    acc_g_kf = KalmanFilter_Update(acc_g_kf, TILT_FILTER_ID);
    acc_kf_degree = tym_math_calculate_degree_from_miliG(acc_g_kf);

#ifdef DEBUG_DRIVER_BMI160
    if(print_count > PRINT_ONCE_EVERY_10MS)
    {
        printf("Before Offset:\r\n");
        printf("Accel Raw Data  = %d, y = %d \r\n" , acc_g_x , acc_g_y);
        printf("Raw data Degree x = %d, y = %d \r\n" ,tym_math_calculate_degree_from_miliG(acc_g_x), tym_math_calculate_degree_from_miliG(acc_g_y));
        printf("kalman data Degree x = %d \r\n" ,acc_kf_degree);
    }
#endif

#ifdef DEBUG_DRIVER_BMI160
    ori_degree = acc_kf_degree;
#endif

    if(TILT_SENSOR_FLOOR_MODE == TiltSensorWorkingMode)
    {
        if((acc_kf_degree <= BMI160_MAX_DYNAMIC_CALIBRATION_FLOOR_MODE_OFFSET) \
            && (acc_kf_degree >= BMI160_MIN_DYNAMIC_CALIBRATION_FLOOR_MODE_OFFSET) \
            && ((acc_kf_degree + pBmi160KeyObj->dynamic_x_offset)!=0))
        {
            if(TRUE == Bmi160KeyDrv_CalDynamicOffset(me, acc_kf_degree))
            {
                pBmi160KeyObj->dynamic_x_offset = -1 * acc_kf_degree;
            }
        }
    }
    else
    {
        if((acc_kf_degree <= BMI160_MAX_DYNAMIC_CALIBRATION_WALL_MODE_OFFSET) \
            && (acc_kf_degree >= BMI160_MIN_DYNAMIC_CALIBRATION_WALL_MODE_OFFSET) \
            && ((acc_kf_degree + pBmi160KeyObj->dynamic_y_offset)!=0))
        {
            if(TRUE == Bmi160KeyDrv_CalDynamicOffset(me, acc_kf_degree))
            {
                pBmi160KeyObj->dynamic_y_offset = -1 * acc_kf_degree;
            }
        }
    }

    if(TILT_SENSOR_FLOOR_MODE == TiltSensorWorkingMode)
    {
        acc_degree = acc_degree + pBmi160KeyObj->dynamic_x_offset;
        acc_kf_degree = acc_kf_degree + pBmi160KeyObj->dynamic_x_offset;

        if(acc_degree > MAX_FLOOR_DEGREE)
        {
            acc_degree = MAX_FLOOR_DEGREE;
        }
        else if(acc_degree < MIN_FLOOR_DEGREE)
        {
            acc_degree = MIN_FLOOR_DEGREE;
        }

        if(acc_kf_degree > MAX_FLOOR_DEGREE)
        {
            acc_kf_degree = MAX_FLOOR_DEGREE;
        }
        else if(acc_kf_degree < MIN_FLOOR_DEGREE)
        {
            acc_kf_degree = MIN_FLOOR_DEGREE;
        }
    }
    else
    {
        acc_degree = acc_degree + pBmi160KeyObj->dynamic_y_offset;
        acc_kf_degree = acc_kf_degree + pBmi160KeyObj->dynamic_y_offset;

        if(acc_degree > MAX_WALL_DEGREE)
        {
            acc_degree = MAX_WALL_DEGREE;
        }
        else if(acc_degree < MIN_WALL_DEGREE)
        {
            acc_degree = MIN_WALL_DEGREE;
        }

        if(acc_kf_degree > MAX_WALL_DEGREE)
        {
            acc_kf_degree = MAX_WALL_DEGREE;
        }
        else if(acc_kf_degree < MIN_WALL_DEGREE)
        {
            acc_kf_degree = MIN_WALL_DEGREE;
        }
    }

#ifdef DEBUG_DRIVER_BMI160
    if(print_count > PRINT_ONCE_EVERY_10MS)
    {
        printf("After Offset:\r\n");
        if(TILT_SENSOR_FLOOR_MODE == TiltSensorWorkingMode)
            printf("Raw value - KF Value = %d - %d \r\n", acc_g_x, acc_g_kf);
        else
           printf("Raw value - KF Value = %d - %d \r\n", acc_g_y, acc_g_kf);
        printf("Raw Degree - KF Degree = %d - %d \r\n" ,acc_degree , acc_kf_degree);
        printf("origin Degree - offset Degree = %d - %d ,[%d] \r\n" ,ori_degree , acc_kf_degree, pBmi160KeyObj->dynamic_x_offset);
        print_count = 0;
    }
#endif

    if(TILT_SENSOR_FLOOR_MODE == TiltSensorWorkingMode)
    {
        me->keyID = TILT_SENSOR_FLOOR_KEY;
    }
    else
    {
        me->keyID = TILT_SENSOR_WALL_KEY;
    }

    me->keyState = KEY_UP;
    me->param = acc_kf_degree;

    if(acc_kf_degree == pBmi160KeyObj->prev_acc_kf_degree)
    {
        me->keyState = KEY_DOWN;
    }

    pBmi160KeyObj->prev_acc_kf_degree = acc_kf_degree;
}

static int32 Bmi160KeyDrv_GetKeyDegree(cKeyDrv *me)
{
    int16 raw_data;
    int32 g_data;
    eTiltSensorMode TiltSensorWorkingMode;

    bmi160_read_accel_z(&raw_data);
    g_data = ( raw_data * 1000)/((0x7fff + 1)/2);
    if(Bmi160KeyDrv_CheckIsSensorLyingMode(g_data))
        return BMI160_INVALID_DEGREE;

    bmi160_read_accel_x(&raw_data);
    g_data = ( raw_data * 1000)/((0x7fff + 1)/2);
    TiltSensorWorkingMode = Bmi160KeyDrv_CheckSensorMode(g_data);

    if(TILT_SENSOR_WALL_MODE == TiltSensorWorkingMode)
    {
        bmi160_read_accel_y(&raw_data);
        g_data = ( raw_data * 1000)/((0x7fff + 1)/2);
    }

    return (int32)tym_math_calculate_degree_from_miliG(g_data);
}

/*!
 * @brief This function used for initialize the sensor
 *
 *
 * @return results of bus communication function
 * @retval 0 -> Success
 * @retval 1 -> Error
 *
 *
 */
static BMI160_RETURN_FUNCTION_TYPE bmi160_initialize_sensor(struct bmi160_t * s_bmi160)
{
    BMI160_RETURN_FUNCTION_TYPE com_rslt = BMI160_INIT_VALUE;
    /*
    * Based on the user need configure I2C or SPI interface.
    * It is sample code to explain how to use the bmi160 API
    */
    com_rslt = i2c_routine(s_bmi160);

    /*
    * This function used to assign the value/reference of
    * the following parameters
    * I2C address
    * Bus Write
    * Bus read
    * company_id
    */
    com_rslt += bmi160_init_no_wait(s_bmi160);
    return com_rslt;
}

static void Bmi160KeyDrv_InitSection0(void *me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig;
    pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)Seq_GetOwner(&seq);
    I2CDrv_Ctor(&bmi160I2c,(tI2CDevice*)(pBmi160KeyboardConfig->attachedDeviceObjConfig));
    initResult = bmi160_initialize_sensor(&configbmi160);
    if(TRUE == bmi160_need_wait())
    {
        pBmi160KeyboardConfig->DelayTimeOut = BMI160_GEN_READ_WRITE_DELAY;
    }
}

static void Bmi160KeyDrv_InitSection1(void *me)
{
    initResult += bmi160_set_command_register_no_wait_1(ACCEL_MODE_NORMAL);
}

static void Bmi160KeyDrv_InitSection2(void *me)
{
    initResult += bmi160_set_command_register_no_wait_2(ACCEL_MODE_NORMAL);
}

static void Bmi160KeyDrv_InitSection3(void *me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig;
    pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)Seq_GetOwner(&seq);
    initResult += bmi160_set_accel_bw_no_wait(BMI160_ACCEL_NORMAL_AVG4);
    if(TRUE == bmi160_need_wait())
    {
        pBmi160KeyboardConfig->DelayTimeOut = BMI160_GEN_READ_WRITE_DELAY;
    }
}

static void Bmi160KeyDrv_InitSection4(void *me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig;
    pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)Seq_GetOwner(&seq);
    /* set accel data rate as 100Hz*/
    initResult += bmi160_set_accel_output_data_rate_no_wait_1(
        BMI160_ACCEL_OUTPUT_DATA_RATE_100HZ,
        BMI160_ACCEL_OSR4_AVG1);
    if(TRUE == bmi160_need_wait())
    {
        pBmi160KeyboardConfig->DelayTimeOut = BMI160_GEN_READ_WRITE_DELAY;
    }
}

static void Bmi160KeyDrv_InitSection5(void *me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig;
    pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)Seq_GetOwner(&seq);
    /* set accel data rate as 100Hz*/
    initResult += bmi160_set_accel_output_data_rate_no_wait_2(
        BMI160_ACCEL_OUTPUT_DATA_RATE_100HZ,
        BMI160_ACCEL_OSR4_AVG1);
    if(TRUE == bmi160_need_wait())
    {
        pBmi160KeyboardConfig->DelayTimeOut = BMI160_GEN_READ_WRITE_DELAY;
    }
}

static void Bmi160KeyDrv_InitSection6(void *me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig;
    pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)Seq_GetOwner(&seq);
    initResult +=  bmi160_set_accel_range_no_wait(BMI160_ACCEL_RANGE_2G);
    if(TRUE == bmi160_need_wait())
    {
        pBmi160KeyboardConfig->DelayTimeOut = BMI160_GEN_READ_WRITE_DELAY;
    }
}

static void Bmi160KeyDrv_InitSection7(void *me)
{
    int i;
    int16 acc_raw;
    int32 acc_g;

    /* Process Kalmanfilter estimation error time*/
    float p = KALMAN_FILTER_P;
    float q = KALMAN_FILTER_Q;
    float r = KALMAN_FILTER_R;
    uint16 iterNum = KALMAN_FILTER_INIT_ITER;

    initResult += bmi160_read_accel_x(&acc_raw);
    acc_g = ( acc_raw * 1000)/((0x7fff + 1)/2);
    KalmanFilter_Register(TILT_FILTER_ID, &p, &q, &r, iterNum, (int32)acc_g);

    if (BMI160_RET_SUCCESS != initResult)
    {
        ASSERT(BMI160_RET_SUCCESS == initResult);
    }
    initResult = ERROR;
}

/*!
 * @brief Used for I2C initialization
 * @note
 * The following function is used to map the
 * I2C bus read, write, bmi160_delay_ms and
 * device address with global structure bmi160_t
*/
static int8 i2c_routine(struct bmi160_t *s_bmi160)
{
    /*--------------------------------------------------------------------------*
    * By using bmi160 the following structure parameter can be accessed
    * Bus write function pointer: BMI160_WR_FUNC_PTR
    * Bus read function pointer: BMI160_RD_FUNC_PTR
    * bmi160_delay_ms function pointer: bmi160_delay_ms_msec
    * I2C address: dev_addr
    *--------------------------------------------------------------------------*/
    s_bmi160->bus_write = bmi160_i2c_bus_write;
    s_bmi160->bus_read = bmi160_i2c_bus_read;
    s_bmi160->burst_read = bmi160_i2c_bus_read_burst;
    s_bmi160->dev_addr = BMI160_I2C_ADDRESS;

    return BMI160_INIT_VALUE;
}

/**************************************************************/
/**\name I2C/SPI read write function */
/**************************************************************/
/*-------------------------------------------------------------------*
*
*    This is a sample code for read and write the data by using I2C/SPI
*    Use either I2C or SPI based on your need
*    Configure the below code to your SPI or I2C driver
*
*-----------------------------------------------------------------------*/
 /*!
 * @brief : The function is used as I2C bus read
 * @return : Status of the I2C read
 * @param dev_addr : The device address of the sensor
 * @param reg_addr : Address of the first register, will data is going to be read
 * @param reg_data : This data read from the sensor, which is hold in an array
 * @param cnt : The no of byte of data to be read
 */
static int8 bmi160_i2c_bus_read(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt)
{
    tI2CMsg i2cMsg  =
    {
        .devAddr = dev_addr,
        .regAddr = reg_addr,
        .length  = cnt,
        .pMsg    = (uint8*)reg_data,
    };

    if(BMI160_RET_SUCCESS != I2CDrv_MasterRead(&bmi160I2c, &i2cMsg))
    {
        DRV_BMI160_DBG(("bmi160_i2c_bus_read:I2C read error\n"));
        return BMI160_RET_FAIL;
    }

    return BMI160_RET_SUCCESS;
}

static int8 bmi160_i2c_bus_read_burst(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint32 cnt)
{
    tI2CMsg i2cMsg  =
    {
        .devAddr = dev_addr,
        .regAddr = reg_addr,
        .length  = cnt,
        .pMsg    = (uint8*)reg_data,
    };

    if(BMI160_RET_SUCCESS != I2CDrv_MasterRead(&bmi160I2c, &i2cMsg))
    {
        DRV_BMI160_DBG(("bmi160_i2c_bus_read_burst:I2C read error\n"));
        return BMI160_RET_FAIL;
    }

    return BMI160_RET_SUCCESS;
}

static int8 bmi160_i2c_bus_write(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt)
{
    uint8 temp[256] = {0};

    temp[0] = reg_addr;
    memcpy(&temp[1], reg_data, cnt);

    tI2CMsg i2cMsg  =
    {
        .devAddr = dev_addr,
        .regAddr = 0,
        .length  = cnt+1,
        .pMsg    = (uint8*)temp,
    };

    if(BMI160_RET_SUCCESS != I2CDrv_MasterWrite(&bmi160I2c, &i2cMsg))
    {
        DRV_BMI160_DBG(("bmi160_i2c_bus_write:I2C read error\n"));
        return BMI160_RET_FAIL;
    }

    return BMI160_RET_SUCCESS;
}

void Bmi160Keyboard_InitStart(void * me)
{
    tKeyboardDevice * pKeyboardConfig  = (tKeyboardDevice*)me;
    Seq_Ctor(&seq, pKeyboardConfig, Bmi160KeyDrvInitSection, ArraySize(Bmi160KeyDrvInitSection));
}

bool Bmi160Keyboard_InitProcess(void * me)
{
    tBmi160KeyboardDevice * pBmi160KeyboardConfig = (tBmi160KeyboardDevice *)me;
    if(pBmi160KeyboardConfig->DelayTimeOut > 0)
    {
        pBmi160KeyboardConfig->DelayTimeOut = pBmi160KeyboardConfig->DelayTimeOut - SENSOR_INIT_SEQ_DELAY_TIME;
        return FALSE;
    }
    if (!Seq_isSeqFinished(&seq))
    {
        Seq_Refresh(&seq, SENSOR_INIT_SEQ_DELAY_TIME);
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void Bmi160Keyboard_InitDone(void * me)
{
    tKeyboardDevice * pKeyboardConfig  = (tKeyboardDevice*)me;
    pKeyboardConfig->isInitSeqDone = FALSE;
    I2CDrv_Xtor(&bmi160I2c);
}
