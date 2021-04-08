#ifndef __SYSTEM_DRV_PRIV_H__
#define __SYSTEM_DRV_PRIV_H__

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/
#define     SYS_PWR_ON                GPIO_OUT_PWR_EN   /*GPIO_OUT_DSP_3V3*/
#define     SYS_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),SYS_PWR_ON)
#define     SYS_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),SYS_PWR_ON)

/* To do: move to other place */
#ifdef  HAS_DSP_EN
#define     DSP_PWR_ON                        GPIO_OUT_DSP_3V3
#define     DSP_PWR_ENABLE(x)         GpioDrv_SetBit(&(x),DSP_PWR_ON)
#define     DSP_PWR_DISABLE(x)        GpioDrv_ClearBit(&(x),DSP_PWR_ON)
#endif

#ifdef HAS_EXT_CHARGE_CTRL
#ifdef HAS_PWR_IO_EXPANDER
#define     EX_CHG_CTRL             IOE_OUT_EX_CHG_CTL
#define     EX_CHG_CTRL_ENABLE(x)         SystemDrv_Ioe_Set_by_ID(&(x),EX_CHG_CTRL)
#define     EX_CHG_CTRL_DISABLE(x)        SystemDrv_Ioe_Clear_by_ID(&(x),EX_CHG_CTRL)
#else
#define     EX_CHG_CTRL             GPIO_OUT_EX_CHG_CTL
#define     EX_CHG_CTRL_ENABLE(x)         GpioDrv_SetBit(&(x),EX_CHG_CTRL)
#define     EX_CHG_CTRL_DISABLE(x)         GpioDrv_ClearBit(&(x),EX_CHG_CTRL)
#endif
#endif

#ifdef HAS_BAT_CHG_CURRENT_SELECT
#ifdef HAS_PWR_IO_EXPANDER              // ioe pin high --->  1.5A    ioe pin low ---> 2.5A
#define     BAT_CHG_CUR_SEL              IOE_OUT_ICHG_SEL
#define     BAT_CHG_CUR_LOW(x)         SystemDrv_Ioe_Set_by_ID(&(x),BAT_CHG_CUR_SEL)
#define     BAT_CHG_CUR_HIGH(x)         SystemDrv_Ioe_Clear_by_ID(&(x),BAT_CHG_CUR_SEL)
#endif
#endif


#ifdef HAS_BAT_CHARGE
#ifdef HAS_PWR_IO_EXPANDER
#define     BAT_CHG_ON              IOE_OUT_BAT_CHG_EN
#define     BAT_CHG_ENABLE(x)         SystemDrv_Ioe_Set_by_ID(&(x),BAT_CHG_ON)
#define     BAT_CHG_DISABLE(x)         SystemDrv_Ioe_Clear_by_ID(&(x),BAT_CHG_ON)
#else
#define     BAT_CHG_ON              GPIO_OUT_BAT_CHG_EN
#define     BAT_CHG_ENABLE(x)         GpioDrv_SetBit(&(x),BAT_CHG_ON)
#define     BAT_CHG_DISABLE(x)         GpioDrv_ClearBit(&(x),BAT_CHG_ON)
#endif
#endif

#ifdef HAS_BOOST_ENABLE
#ifdef HAS_PWR_IO_EXPANDER
#define     BOOST_ON                IOE_OUT_BOOST_EN
#define     BOOST_ENABLE(x)         SystemDrv_Ioe_Set_by_ID(&(x),BOOST_ON)
#define     BOOST_DISABLE(x)         SystemDrv_Ioe_Clear_by_ID(&(x),BOOST_ON)
#else
#define     BOOST_ON                GPIO_OUT_BOOST_EN
#define     BOOST_ENABLE(x)         GpioDrv_SetBit(&(x),BOOST_ON)
#define     BOOST_DISABLE(x)         GpioDrv_ClearBit(&(x),BOOST_ON)
#endif
#endif

#ifdef HAS_AMP_CTRL
#ifdef HAS_PWR_IO_EXPANDER
#define     AMP_ON                  IOE_OUT_AMP_ON
#define     AMP_ENABLE(x)           SystemDrv_Ioe_Set_by_ID(&(x),AMP_ON)
#define     AMP_DISABLE(x)           SystemDrv_Ioe_Clear_by_ID(&(x),AMP_ON)
#endif
#endif

#ifdef HAS_DC_IN
#define DC_IN_ADC_PIN         ADC_PIN8
#define DC_IN_ADC_THRESHOLD     (2172)          /* 1.7V, DC=11V */
#endif

#ifdef HAS_BATTERY_NTC
#define BATTERY_NTC_PIN         ADC_PIN7

#ifdef HENDRIX_M_ES1
#define NTC_MINUS_10          (3187)
#define NTC_MINUS_9           (3170)
#define NTC_MINUS_8           (3152)
#define NTC_0                 (2995)
#define NTC_4                 (2905)
#define NTC_25                (2319)
#define NTC_31                (2131)
#define NTC_35                (2005)
#define NTC_41                (1818)
#define NTC_45                (1696)
#define NTC_48                (1606)
#define NTC_50                (1548)
#define NTC_55                (1403)
#define NTC_58                (1328)
#define NTC_60                (1276)
#else
#define NTC_MINUS_10                    (3189)
#define NTC_MINUS_9                     (3175)
#define NTC_MINUS_8                     (3161)
#define NTC_0                           (3014)
#define NTC_4                           (2936)
#define NTC_25                          (2414)
#define NTC_31                          (2234)
#define NTC_35                          (2114)
#define NTC_41                          (1938)
#define NTC_45                          (1819)
#define NTC_48                          (1729)
#define NTC_50                          (1669)
#define NTC_55                          (1538)
#define NTC_58                          (1448)
#define NTC_60                          (1395)
#endif



#define NTC_CHARGING_OVERHEAT_THRESHOLD          (NTC_45)
#define NTC_CHARGING_SUPERCOOL_THRESHOLD         (NTC_0)
#define NTC_OVERHEAT_THRESHOLD                   (NTC_60)
#define NTC_OVERHEAT_WARNING                     (NTC_58)
#define NTC_OVERHEAT_RETURN_NORMAL               (NTC_55)
#define NTC_SUPERCOOL_THRESHOLD                  (NTC_MINUS_8)
#define NTC_NORMAL_LOWER_THRESHOLD               (NTC_4)
#define NTC_NORMAL_HIGHER_THRESHOLD              (NTC_41)
#define NTC_ROOM_TEMPERATURE                     (NTC_25)

#define SYS_NTC_SAMPLE_NUM      (3)        /* (3)*1500ms = 4.5 sec window*/
#define SYS_NTC_AVERAGE_NUM     (3)        /* 4.5 sec window */
#endif

#ifdef HAS_BATTERY_DETECT
#ifdef HENDRIX_M_ES1
#define ADC_CAPACITY_CRITICAL             (872)//(703)      /* ~5.5V */
#define ADC_CAPACITY_5                    (1031)//(831)    /* 6.5V */
#define ADC_CAPACITY_10                   (1064)//(857)    /* 6.7V */  /*(3122)*/    /* 6.6V */
#define ADC_CAPACITY_20                   (1080)//(870)    /* 6.8V */ /*(3170)*/    /* 6.7V */
#define ADC_CAPACITY_50                   (1167)//(940)    /* 7.35V*/
#define ADC_CAPACITY_80                   (1270)//(1023)    /*8.0V*/
#define ADC_CAPACITY_90                   (1301)//(1048)    /*8.2V*/
#define ADC_CAPACITY_100                  (1333)//(1074)     /*8.4V*/
#else
#define ADC_CAPACITY_CRITICAL             (2590)      /* ~5.5V */
#define ADC_CAPACITY_5                    (3122)   /* 6.6V */ /*(3076)*/   /* 6.5V */
#define ADC_CAPACITY_10                   (3170)    /* 6.7V */  /*(3122)*/    /* 6.6V */
#define ADC_CAPACITY_20                   (3260)    /* 6.8V */ /*(3170)*/    /* 6.7V */
#define ADC_CAPACITY_50                   (3470)    /* 7.35V*/
#define ADC_CAPACITY_80                   (3770)    /*8.0V*/
#define ADC_CAPACITY_90                   (3874)    /*8.2V*/
#define ADC_CAPACITY_100                  (3974)     /*8.4V*/
#endif

#define CAPACITY_4                          (4)
#define CAPACITY_5                          (5)
#define CAPACITY_10                         (10)
#define CAPACITY_20                         (20)
#define CAPACITY_80                         (80)
#define CAPACITY_90                         (90)
#define CAPACITY_100                        (100)

#define SYS_BATTERY_SAMPLE_NUM      (20)        /* (20)*50 = 1 sec window*/
#define SYS_BATTERY_AVERAGE_NUM     (20)        /* 1 sec window */
#define BATTERY_ADC_PIN     ADC_PIN6

#define INSTANT_CRITICAL_COUNTER_THRESHOLD      (1)
#endif

#ifdef HAS_HW_VERSION_TAG
/*
#define HW_VERSION_PIN      ADC_PIN9
#define HW_VERSION_ES1_HIGH_THRESHOLD       (3850)
#define HW_VERSION_ES1_LOW_THRESHOLD        (3650)
#define HW_VERSION_ES2_HIGH_THRESHOLD       (1900)
#define HW_VERSION_ES2_LOW_THRESHOLD        (1800)
*/
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
#define HW_MP1_HIGH_THRESHOLD      4096
#define HW_MP1_LOW_THRESHOLD       3900
// MP2 : pull-up:NC, pull-down:0K, Voltage=0V, dac-value=0
#define HW_MP2_HIGH_THRESHOLD      50
#define HW_MP2_LOW_THRESHOLD       0
#endif


#define SYSTEM_INIT_STEP    0

#define ADC_REFERENCE_mVOLT     3300    // mcu voltage 3.3V
#define ADC_PRECISION_BTIS      12      // adc precision is 12 bits

static void SystemDrv_ResetStage(void);
static void SystemDrv_PoweringUpStage(void);
static void SystemDrv_PowerReadyStage(void);


#endif  // __SYSTEM_DRV_PRIV_H__

