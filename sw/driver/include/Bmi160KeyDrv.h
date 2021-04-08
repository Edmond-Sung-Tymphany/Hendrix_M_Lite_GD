/**
 * @file        Bmi160KeyDrv.h
 * @brief       The key driver (accelerometer and gyroscope sensor) interfaces and implementation
 * @author      Paris.Chen
 * @date        2017-02-17
 * @copyright   Tymphany Ltd.
 */

#ifndef BMI160_KEY_DRV_H
#define BMI160_KEY_DRV_H

#include "KeyDrv.h"
#include "I2CDrv.h"
#include "tym_bmi160.h"
#include "seq.h"

#ifdef __cplusplus
extern "C" {
#endif

/********************************/
/**\name POWER MODES DEFINITION */
/*******************************/
#define ACCEL_MODE_NORMAL                       (0x11)
#define GYRO_MODE_NORMAL                        (0x15)
#define ACCEL_LOWPOWER                          (0X12)
#define MAG_SUSPEND_MODE                        (1)
#define BMI160_MODE_SWITCHING_DELAY             (30)

/********************************/
/**\name RETURN TYPE */
/*******************************/
/* return type of communication routine*/
#define BMI160_RETURN_FUNCTION_TYPE             int8

/********************************/
/**\name RUNNING MODE DEFINITIONS */
/*******************************/
#define STANDARD_UI_9DOF_FIFO                   (0)
#define STANDARD_UI_IMU_FIFO                    (1)
#define STANDARD_UI_IMU                         (2)
#define STANDARD_UI_ADVANCEPOWERSAVE            (3)
#define	ACCEL_PEDOMETER                         (4)
#define APPLICATION_HEAD_TRACKING               (5)
#define APPLICATION_NAVIGATION                  (6)
#define APPLICATION_REMOTE_CONTROL              (7)
#define APPLICATION_INDOOR_NAVIGATION           (8)

/********************************/
/**\name MAG INTERFACE */
/*******************************/
#define C_BMI160_BYTE_COUNT                     (2)
#define BMI160_SLEEP_STATE                      (0x00)
#define BMI160_WAKEUP_INTR                      (0x00)
#define BMI160_SLEEP_TRIGGER                    (0x04)
#define BMI160_WAKEUP_TRIGGER                   (0x02)
#define BMI160_ENABLE_FIFO_WM                   (0x02)
#define BMI160_MAG_INTERFACE_OFF_PRIMARY_ON     (0x00)
#define BMI160_MAG_INTERFACE_ON_PRIMARY_ON      (0x02)

typedef enum
{
    TILT_SENSOR_FLOOR_MODE,
    TILT_SENSOR_WALL_MODE,
}eTiltSensorMode;

/*!
 * @brief struct used for assign the value for
 * gyro sleep configuration
 */
struct gyro_sleep_setting {
    uint8 sleep_trigger;        /**< gyro sleep trigger configuration*/
    uint8 wakeup_trigger;       /**< gyro wakeup trigger configuration*/
    uint8 sleep_state;          /**< gyro sleep state configuration*/
    uint8 wakeup_int;           /**< gyro wakeup interrupt configuration*/
};

SUBCLASS(cBmi160KeyDrv,cKeyDrv)
    const tBmi160KeyboardDevice  *pBmi160KeyboardConfig;
    int8 prev_acc_kf_degree;
    int8 dynamic_x_offset;
    int8 dynamic_y_offset;
    int8 dynamic_prev_degree;
    uint32 dynamic_cal_current_time;
    uint32 dynamic_cal_timeout;

METHODS
    void Bmi160KeyDrv_Ctor(cBmi160KeyDrv *me, const tBmi160KeyboardDevice *pSensorBmi160KeyboardConfig);
    void Bmi160KeyDrv_Xtor(cBmi160KeyDrv *me);
END_CLASS

void Bmi160Keyboard_InitStart(void * me);
bool Bmi160Keyboard_InitProcess(void * me);
void Bmi160Keyboard_InitDone(void * me);

#ifdef __cplusplus
}
#endif

#endif /* BMI160_KEY_DRV_H */

