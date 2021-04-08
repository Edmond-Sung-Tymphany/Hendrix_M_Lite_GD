#ifndef __SYSTEM_DRV_PRIV_H__
#define __SYSTEM_DRV_PRIV_H__

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/
#define     SYS_PWR_ENABLE(x)         do{ \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_MAIN_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_5V_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_DSP_POWER); \
                                      }while(0)
#define     SYS_PWR_DISABLE(x)        do{ \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_MAIN_POWER); \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_5V_POWER); \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_DSP_POWER); \
                                      }while(0)

#define     A2B_PWR_ENABLE(x)         do{ \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)
#define     A2B_PWR_DISABLE(x)        do{ \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)

#define MAIN_POWER_ENABLE(x)        GpioDrv_SetBit(&(x), GPIO_OUT_MAIN_POWER)
#define MAIN_POWER_DISABLE(x)       GpioDrv_ClearBit(&(x), GPIO_OUT_MAIN_POWER)

#define OP_SHUTDOWN_ENABLE(x)       GpioDrv_ClearBit(&(x), GPIO_OUT_OP_MUTE)
#define OP_SHUTDOWN_DISABLE(x)      GpioDrv_SetBit(&(x), GPIO_OUT_OP_MUTE)

#define AMP_SHUTDOWN_ENABLE(x)      GpioDrv_ClearBit(&(x), GPIO_OUT_AMP_RESET)
#define AMP_SHUTDOWN_DISABLE(x)     GpioDrv_SetBit(&(x), GPIO_OUT_AMP_RESET)

// add the reset for the chip which need to reset when system power up
#define POWER_UP_RESET_CHIP_ENABLE(x)     do{ \
                                            GpioDrv_ClearBit(&(x), GPIO_OUT_DSP_RESET); \
                                            GpioDrv_ClearBit(&(x), GPIO_OUT_CODEC_RESET); \
                                          }while(0)
#define POWER_UP_RESET_CHIP_DISABLE(x)    do{ \
                                            GpioDrv_SetBit(&(x), GPIO_OUT_DSP_RESET); \
                                            GpioDrv_SetBit(&(x), GPIO_OUT_CODEC_RESET); \
                                          }while(0)

#define SYSTEM_INIT_STEP    0

static void SystemDrv_PowerOnStage(void);
static void SystemDrv_ResetFinishStage(void);
static void SystemDrv_PowerReadyStage(void);


#endif  // __SYSTEM_DRV_PRIV_H__

