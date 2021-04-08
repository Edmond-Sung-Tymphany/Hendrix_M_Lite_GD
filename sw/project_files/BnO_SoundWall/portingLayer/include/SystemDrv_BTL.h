#ifndef __SYSTEM_DRV_BTL_H__
#define __SYSTEM_DRV_BTL_H__

/****************************************************************/
/*** **************MACRO DEFINITION********************************/
/***************************************************************/
#define     BTL_SYS_PWR_ENABLE(x)     do{ \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_MAIN_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_5V_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_DSP_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)
#define     BTL_SYS_PWR_DISABLE(x)    do{ \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_MAIN_POWER); \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_5V_POWER); \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_DSP_POWER); \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)

#define     BTL_A2B_PWR_ENABLE(x)     do{ \
                                        GpioDrv_SetBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)
#define     BTL_A2B_PWR_DISABLE(x)    do{ \
                                        GpioDrv_ClearBit(&(x), GPIO_OUT_A2B_POWER); \
                                      }while(0)

#define BTL_AMP_SHUTDOWN_ENABLE(x)      GpioDrv_ClearBit(&(x), GPIO_OUT_AMP_RESET)
#define BTL_AMP_SHUTDOWN_DISABLE(x)     GpioDrv_SetBit(&(x), GPIO_OUT_AMP_RESET)

void BTL_SystemDrv_ShutDownAmp(bool enable);
void BTL_SystemDrv_PowerOnA2B(bool enable);
void BTL_SystemDrv_PowerOff(void);
void BTL_SystemDrv_PowerOn(void);
void BTL_SystemDrv_Init(void);

#endif  // __SYSTEM_DRV_BTL_H__

