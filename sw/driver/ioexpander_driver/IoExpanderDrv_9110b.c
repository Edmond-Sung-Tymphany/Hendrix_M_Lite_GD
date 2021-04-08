/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  IoExpander Driver
                  -------------------------

                  SW Module Document




@file        IoExpanderDrv.c
@brief       This file implements the driver for AW9110B
@author      Wesley Lee
@date        2015-11-24
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-11-24     Wesley Lee
DESCRIPTION: First Draft. Copied from AW9110B implementation
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "./IoExpanderDrv_priv_9110b.h"
#include "IoExpanderDrv.config"
#include "gpioDrv.h"
#include "trace.h"
#include "bsp.h" //BSP_InExp()

#ifdef IOEXPANDERDRV_RST_CONTROL
static cGpioDrv gpioResetIoExpanderDrv;
#endif

/* Private functions / variables. Declare and drivers here */

static void IoExpanderDrv_I2cRead_aw9110b(cIoExpanderDrv *me, uint8 reg, uint8* value);
static void IoExpanderDrv_I2cWrite_aw9110b(cIoExpanderDrv *me, uint8 reg, uint8 value);

/* for debug purpose only */
#ifdef HAS_IOEXPANDER_LED_TEST
static cIoExpanderDrv * g_pme = NULL;

void IoExpanderDrv_TestOn()
{
    if (g_pme != NULL)
    {
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 0, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 1, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 2, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 3, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 0, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 1, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 2, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 3, 255);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 4, 255);
    }
}
void IoExpanderDrv_TestOff()
{
    if (g_pme != NULL)
    {
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 0, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 1, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 2, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 1, 3, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 0, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 1, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 2, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 3, 0);
        IoExpanderDrv_SetBrightness_aw9110b(g_pme, 0, 4, 0);
    }
}

#endif
/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
/* @brief       IO Expander driver constructor
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   pConfig     device configuration (AW9110B is I2C device)
 */

void IoExpanderDrv_Ctor_aw9110b(cIoExpanderDrv *me, tIoeLedDevice *pIoeLedConfig)
{
    if(!me->i2cDrv.isReady)
    {
        /*
        * All project ioexpander config should be pre-defined in IoExpanderDrv.config
        * These io configurations normally will not be changed, thus ioexpander only Ctor once,
	*/

#ifdef IOEXPANDERDRV_RST_CONTROL      
        /* Ctor gpio for ioexpander reset control */
        tGPIODevice *ioexpanderRstCtrl = NULL;
        ASSERT(pIoeLedConfig);
        ioexpanderRstCtrl = pIoeLedConfig->pResetGpioDevice;
        ASSERT(ioexpanderRstCtrl);
        GpioDrv_Ctor(&gpioResetIoExpanderDrv,ioexpanderRstCtrl);
      
        /* Reset ioexpander before init started */
        EXPANDER_RST_LOW(gpioResetIoExpanderDrv);
        /* ca17 exeption handler will ctor IO-Expender, but interrupt already disable on exception,
         * to let ctor() work on both normal and exception mode, let it support two blocking delay.
         */
        if( BSP_InExp() )
            BSP_ExpBlockingDelayMs(50);
        else
            BSP_BlockingDelayMs(50);//Delay 50 ms
        EXPANDER_RST_HIGH(gpioResetIoExpanderDrv);
        if( BSP_InExp() )
            BSP_ExpBlockingDelayMs(50);
        else
        BSP_BlockingDelayMs(50);//Delay 50 ms
#endif

        I2CDrv_Ctor(&me->i2cDrv,(tI2CDevice*)pIoeLedConfig->i2cDeviceConf);

        // software reset

        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_RESET_REG,         pIoeLedConfig->pIoExpanderConfig->swResetValue);

        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPMD_A_REG,        pIoeLedConfig->pIoExpanderConfig->ledModePortA);
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPMD_B_REG,        pIoeLedConfig->pIoExpanderConfig->ledModePortB);

        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_A_REG, pIoeLedConfig->pIoExpanderConfig->outPutPortA);
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_B_REG, pIoeLedConfig->pIoExpanderConfig->outPutPortB);

        // Control register
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_CTL_REG, pIoeLedConfig->pIoExpanderConfig->controlValue);

#ifdef HAS_IOEXPANDER_LED_TEST
    g_pme = me;
    IoExpanderDrv_TestOn();
#endif

    }
}

/* @brief       IO Expander driver destructor
 * @param[in]   me          pointer to IO-Expander driver object
 */
void IoExpanderDrv_Xtor_aw9110b(cIoExpanderDrv *me)
{
    if(me->i2cDrv.isReady)
    {
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_RESET_REG, AW9110B_SW_RESET_VAULE);
        I2CDrv_Xtor(&me->i2cDrv);
        me->i2cDrv.isReady = FALSE;
    }
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
/* @brief       Set the corresponding Port-Pin to High
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 */
void IoExpanderDrv_SetGpio_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin)
{   
    uint8 tempData;

    switch(port)
    {
        case IO_EXPANDER_PORT_A:
        {
            IoExpanderDrv_I2cRead_aw9110b(me, AW9110B_GPIO_OUTPUT_A_REG, &tempData);
            tempData |= (1 << pin);
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_A_REG, tempData);
            break;
        }
        
        case IO_EXPANDER_PORT_B:
        {
            IoExpanderDrv_I2cRead_aw9110b(me, AW9110B_GPIO_OUTPUT_B_REG, &tempData);
            tempData |= (1 << pin);
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_B_REG, tempData);
            break;
        }
        default:
            ASSERT(0);
    }
}

/* @brief       Clear the corresponding Port-Pin to Ground
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 */
void IoExpanderDrv_ClearGpio_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin)
{
    uint8 tempData;

    switch(port)
    {
        case IO_EXPANDER_PORT_A:
        {
            IoExpanderDrv_I2cRead_aw9110b(me, AW9110B_GPIO_OUTPUT_A_REG, &tempData);
            tempData &= (~(1 << pin));
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_A_REG, tempData);
            break;
        }
        
        case IO_EXPANDER_PORT_B:
        {
            IoExpanderDrv_I2cRead_aw9110b(me, AW9110B_GPIO_OUTPUT_B_REG, &tempData);
            tempData &= (~(1 << pin));
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_OUTPUT_B_REG, tempData);
            break;
        }
        default:
            ASSERT(0);
    }    
}

/* @brief       Set the corresponding Port-Pin brightness
 * @param[in]   me          pointer to IO-Expander driver object
 * @param[in]   port        0 or 1
 * @param[in]   pin         0 to 7
 * @param[in]   value       0 to 255 brightness, 0 means off
 */
void IoExpanderDrv_SetBrightness_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin, uint8 value)
{
    switch(port)
    {
        case IO_EXPANDER_PORT_A:
        {
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_DIM0_REG + pin + 4, value);
            break;
        }
        case IO_EXPANDER_PORT_B:
        {
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_DIM0_REG + pin, value);
            break;
        }
        default:
            ASSERT(0);
    }
}

void IoExpanderDrv_AutoBlink_aw9110b(cIoExpanderDrv *me, eIoeAutoPatt patt)
{
    // Breathing Blink for AW9110B is available for OUT0 ~ OUT5
    // Detail step from Reference Manual:
    // 1. Configure OUTx as LED,    AW9110B_GPMD_A_REG, AW9110B_GPMD_B_REG
    // 2. Configure EN_BRE,         AW9110B_EN_BRE_REG
    // 3. Configure BLINK,          AW9110B_GPIO_CFG_A_REG, AW9110B_GPIO_CFG_B_REG
    // 4. Configure timing,         AW9110B_FADE_TMR_REG, AW9110B_FULL_TMR_REG, 
    //                              AW9110B_DLY0_BRE_REG, AW9110B_DLY1_BRE_REG, AW9110B_DLY2_BRE_REG
    //                              AW9110B_DLY3_BRE_REG, AW9110B_DLY4_BRE_REG, AW9110B_DLY5_BRE_REG
    // 5. Set GO,                   AW9110B_CTL_REG

    tIoePattern *pPatt = &autoBlinkPatt[patt];
    uint8 led_mask;

    if(!pPatt->led_mask)
    {
        return;
    }

    // Step 1, implemented in ctor
    // Step 2, EN_BRE
    IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_EN_BRE_REG, pPatt->led_mask);

    // Step 3, BNINK mode
    led_mask = (pPatt->led_mask >> 4) & AW9110B_GPIO_CFG_A_OUT4_OUT5_MASK;
    if(led_mask)
    {
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_A_REG, led_mask);
    }

    led_mask = pPatt->led_mask & AW9110B_GPIO_CFG_B_OUT0_OUT3_MASK;
    if(led_mask)
    {
        IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_B_REG, led_mask);
    }

    // Step 4, timing
    for (uint32 i = AW9110B_NUM_BLINK - 1; i; --i)
    {
        uint8 delay = pPatt->delay[i];
        if (delay != AW9110B_DLY_TMR_0MS)
        {
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_DLY0_BRE_REG + i, delay);
        }
    }

    IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_FADE_TMR_REG, pPatt->fade_time);
    IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_FULL_TMR_REG, pPatt->full_time);

    // Step 5, GO
    IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_CTL_REG, IO_EXPANDER_CONTROL_VALUE | AW9110B_CTL_GO);
}

void IoExpanderDrv_SetIoInput_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin)
{
    uint8 value = 0;
    switch(port)
    {
        case IO_EXPANDER_PORT_A:
        {
            value = IO_EXPANDER_DIRECTIOIN_PORTA | (1 << pin);
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_A_REG, value);
            break;
        }
        case IO_EXPANDER_PORT_B:
        {
            value = IO_EXPANDER_DIRECTIOIN_PORTB & (1 << pin);
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_B_REG, value);
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
}

void IoExpanderDrv_SetIoOutput_aw9110b(cIoExpanderDrv *me, uint8 port, uint8 pin)
{
    uint8 value = 0;
    switch(port)
    {
        case IO_EXPANDER_PORT_A:
        {
            value = IO_EXPANDER_DIRECTIOIN_PORTA & (~(1 << pin));
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_A_REG, value);
            break;
        }
        case IO_EXPANDER_PORT_B:
        {
            value = IO_EXPANDER_DIRECTIOIN_PORTB & (~(1 << pin));
            IoExpanderDrv_I2cWrite_aw9110b(me, AW9110B_GPIO_CFG_B_REG, value);
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
}

static void IoExpanderDrv_I2cRead_aw9110b(cIoExpanderDrv *me, uint8 reg, uint8* value)
{
    cI2CDrv* pi2cObj= &me->i2cDrv;
    tI2CMsg i2cMsg=
    {
        .devAddr    = pi2cObj->pConfig->devAddress,
        .regAddr    = reg,
        .length     = 2,
        .pMsg       = value,
    };

    if (TRUE == me->i2cDrv.isReady)
    {
        if (TP_SUCCESS != I2CDrv_MasterRead(&me->i2cDrv, &i2cMsg))
        {
            me->i2cDrv.isReady = FALSE;
        }
    }
}

static void IoExpanderDrv_I2cWrite_aw9110b(cIoExpanderDrv *me, uint8 reg, uint8 value)
{
    cI2CDrv* pi2cObj= &me->i2cDrv;
    uint8 data[2] = {reg, value};
    tI2CMsg i2cMsg=
    {
        .devAddr    = pi2cObj->pConfig->devAddress,
        .regAddr    = NULL,
        .length     = 2,
        .pMsg       = data,
    };

    if (TRUE == me->i2cDrv.isReady)
    {
        if (TP_SUCCESS != I2CDrv_MasterWrite(&me->i2cDrv, &i2cMsg))
        {
            me->i2cDrv.isReady = FALSE;
        }
    }
}


void IoExpanderDrv_ReCtor_aw9110b()
{
    EXPANDER_RST_LOW(gpioResetIoExpanderDrv);
    BSP_BlockingDelayMs(50);
    EXPANDER_RST_HIGH(gpioResetIoExpanderDrv);
    BSP_BlockingDelayMs(50);
    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    cIoExpanderDrv ioeDrv= {0};	

    //turn off all LED 	
    IoExpanderDrv_Ctor_aw9110b(&ioeDrv, pIoeLedConfig);
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_RESET_REG,           AW9110B_SW_RESET_VAULE);

    // Default output level
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_GPIO_OUTPUT_A_REG,   IO_EXPANDER_DEFAULT_OUTPUT_PORTA);
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_GPIO_OUTPUT_B_REG,   IO_EXPANDER_DEFAULT_OUTPUT_PORTB);

    // LED mode switch 
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_GPMD_A_REG,          IO_EXPANDER_DEFAULT_LED_MODE_PORTA);
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_GPMD_B_REG,          IO_EXPANDER_DEFAULT_LED_MODE_PORTB);

    // Control register
    IoExpanderDrv_I2cWrite_aw9110b(&ioeDrv, AW9110B_CTL_REG,             IO_EXPANDER_CONTROL_VALUE);
}
