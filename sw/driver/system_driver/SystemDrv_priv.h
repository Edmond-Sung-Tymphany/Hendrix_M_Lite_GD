#ifndef __SYSTEM_DRV_PRIV_H__
#define __SYSTEM_DRV_PRIV_H__

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/
#define     SYS_PWR_ON                GPIO_OUT_DSP_3V3
#define     SYS_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),SYS_PWR_ON)
#define     SYS_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),SYS_PWR_ON)

#define AMP_SHUTDOWN_ENABLE(x)      do{GpioDrv_ClearBit(&(x), GPIO_OUT_AMP_W_PDN); GpioDrv_ClearBit(&(x), GPIO_OUT_AMP_T_PDN); }while(0);
#define AMP_SHUTDOWN_DISABLE(x)     do{GpioDrv_SetBit(&(x), GPIO_OUT_AMP_W_PDN); GpioDrv_SetBit(&(x), GPIO_OUT_AMP_T_PDN); }while(0); 

#define SYSTEM_INIT_STEP    0

#define ADC_REFERENCE_mVOLT     3300    // mcu voltage 3.3V
#define ADC_PRECISION_BTIS      12      // adc precision is 12 bits

#ifdef HAS_HW_VERSION_TAG
#define HW_VERSION_PIN      ADC_PIN9
// ES1 : pull-up:22K, pull-down:100k, Voltage=2.7V, dac-value=3357
#define HW_ES1_HIGH_THRESHOLD       3437
#define HW_ES1_LOW_THRESHOLD        3277
// ES2 : pull-up:36K, pull-down:100k, Voltage=2.4V, dac-value=3012
#define HW_ES2_HIGH_THRESHOLD       3082
#define HW_ES2_LOW_THRESHOLD        2932
// ES3 : pull-up:68K, pull-down:100k, Voltage=2.0V, dac-value=2438
#define HW_ES3_HIGH_THRESHOLD       2498
#define HW_ES3_LOW_THRESHOLD        2378
// EVT1 : pull-up:120K, pull-down:100k, Voltage=1.5V, dac-value=1862
#define HW_EVT1_HIGH_THRESHOLD      1912
#define HW_EVT1_LOW_THRESHOLD       1812
// EVT2 : pull-up:150K, pull-down:100k, Voltage=1.3V, dac-value=1638
#define HW_EVT2_HIGH_THRESHOLD      1678
#define HW_EVT2_LOW_THRESHOLD       1598
// DVT1 : pull-up:220K, pull-down:100k, Voltage=1.0V, dac-value=1280
#define HW_DVT1_HIGH_THRESHOLD      1312
#define HW_DVT1_LOW_THRESHOLD       1258
// DVT2 : pull-up:360K, pull-down:100k, Voltage=0.7V, dac-value=890
#define HW_DVT2_HIGH_THRESHOLD      915
#define HW_DVT2_LOW_THRESHOLD       865
// PVT : pull-up:560K, pull-down:100k, Voltage=0.5V, dac-value=621
#define HW_PVT_HIGH_THRESHOLD       650
#define HW_PVT_LOW_THRESHOLD        590
// MP1 : pull-up:0K, pull-down:NC, Voltage=3.3V, dac-value=4095
#define HW_MP1_HIGH_THRESHOLD      4095
#define HW_MP1_LOW_THRESHOLD       3900
// MP2 : pull-up:NC, pull-down:0K, Voltage=0V, dac-value=0
#define HW_MP2_HIGH_THRESHOLD      50
#define HW_MP2_LOW_THRESHOLD       0
#endif

static void SystemDrv_ResetStage(void);
static void SystemDrv_PoweringUpStage(void);
static void SystemDrv_PowerReadyStage(void);


#endif  // __SYSTEM_DRV_PRIV_H__

