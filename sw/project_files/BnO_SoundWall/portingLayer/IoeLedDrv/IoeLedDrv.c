/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  Ioexpandor 9110B driver
                  -------------------------
@file        IoeLedDrv.c
@brief       This file implements the drivers for AW9110 ioexpendor 
@author      Viking WANG
@date        2016-11-15
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"

#if defined(HAS_IOE_LED) 

#include "commontypes.h"
#include "stm32f0xx.h"
#include "trace.h"
#include "bsp.h"
#include "attacheddevices.h"
#include "GpioDrv.h"
#include "I2CDrv.h"
#ifdef HAS_I2C_BUS_DETECT
#include "SystemDrv.h"        
#endif
#ifdef IOE_LED_VIA_SW_I2C
#include "SWi2c_Drv.h"
#endif
#include "IoeLedDrv.h"
#include "IoeLedDrv_priv.h"

static cGpioDrv led_gpio_drv;
static IoeLedDrv_t ioe_led_drv;
static IoeLedMode_t led_mode[LED_ID_MAX];
static IoeLedMode_t ioe_led_mode_shadow[LED_ID_MAX];
static uint8_t en_bre_reg=0x00;
static uint8_t en_bre_reg_shadow=0x00;

static bool IoeLed_WriteByte(uint8_t addr, uint8_t data)
{
    bool ret=TRUE;
    
    ASSERT( ioe_led_drv.isReady );
    if( ! ioe_led_drv.i2cEnable )
        return FALSE;


    // hardware I2C interface
#ifdef IOE_LED_VIA_HW_I2C
    uint8_t reg_data[2];
    
    reg_data[0] = addr;
    reg_data[1] = data;
    
    tI2CMsg i2cMsg=
    {
        .devAddr = ioe_led_drv.i2cDrv.pConfig->devAddress,
        .regAddr = NULL,
        .length = 2,
        .pMsg = (uint8_t *)reg_data,
    };

    if( TP_SUCCESS == I2CDrv_MasterWrite(&(ioe_led_drv.i2cDrv), &i2cMsg) )
        ret = TRUE;
    else
        ret = FALSE;
#endif

    // software I2C interface
#ifdef IOE_LED_VIA_SW_I2C
    cSWi2cDrv_t *p_swi2cDrv;
    p_swi2cDrv = (cSWi2cDrv_t *)(& ioe_led_drv.i2cDrv);
    if( ! p_swi2cDrv->devReady )
        ret = FALSE;
    else
    {
        if( TP_SUCCESS == SWi2cDrv_WriteByte(p_swi2cDrv, addr, data) )
            ret = TRUE;
        else
            ret = FALSE;
    }
#endif

    if( ret == FALSE )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_LED);
#endif
        ioe_led_drv.i2cEnable = FALSE;  // disable it
    }

    return ret;
}

static bool IoeLed_ReadByte(uint8_t addr, uint8_t *p_byte)
{
    bool ret=TRUE;
    
    ASSERT( ioe_led_drv.isReady );
    if( ! ioe_led_drv.i2cEnable )
        return FALSE;

    // hardware I2C interface
#ifdef IOE_LED_VIA_HW_I2C
    tI2CMsg i2cMsg=
    {
        .devAddr = ioe_led_drv.i2cDrv.pConfig->devAddress,
        .regAddr = addr,
        .length = 1,
        .pMsg = p_byte,
    };
    if( TP_SUCCESS == I2CDrv_MasterRead(&(ioe_led_drv.i2cDrv), &i2cMsg) )
        ret = TRUE;
    else
        ret = FALSE;
#endif

    // software I2C interface
#ifdef IOE_LED_VIA_SW_I2C
    cSWi2cDrv_t *p_swi2cDrv;
    p_swi2cDrv = (cSWi2cDrv_t *)(& ioe_led_drv.i2cDrv);
    if( ! p_swi2cDrv->devReady )
        ret = FALSE;
    else
    {
        if( TP_SUCCESS == SWi2cDrv_ReadByte(p_swi2cDrv, addr, p_byte) )
            ret = TRUE;
        else
            ret = FALSE;
    }
#endif

    if( ret == FALSE )
    {
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_LED);
#endif
        ioe_led_drv.i2cEnable = FALSE;  // disable it
    }
    
    return ret;
}

static void IoeLed_BlinkGo(bool enable)
{
    uint8_t ctl_reg_data;

    IoeLed_ReadByte(IOE_REG_CONTROL, &ctl_reg_data);

    if( enable )
        ctl_reg_data |= 0x80;
    else
        ctl_reg_data &= 0x7f;

    IoeLed_WriteByte(IOE_REG_CONTROL, ctl_reg_data);
}

void IoeLed_Ctor(void)
{
    uint16 attached_device_index = 0;
    tDevice *p_led_dev = NULL;

    p_led_dev = (tDevice*)getDevicebyId(LED_DEV_ID, &attached_device_index);
    while( p_led_dev != NULL )
    {
        switch(p_led_dev->deviceType)
        {
            case GPIO_DEV_TYPE:
            {
                tGPIODevice *pLedGPIOConf = (tGPIODevice*)p_led_dev;
                GpioDrv_Ctor(&led_gpio_drv, pLedGPIOConf);
                break;
            }
#ifdef IOE_LED_VIA_HW_I2C
            case I2C_DEV_TYPE:
            {
                tI2CDevice *p_i2c_dev = (tI2CDevice *)p_led_dev;
                I2CDrv_Ctor(&(ioe_led_drv.i2cDrv), p_i2c_dev);
                ioe_led_drv.isReady = TRUE;
                ioe_led_drv.i2cEnable = TRUE;
#if defined(TUNING_ON_ST_EVK_BOARD) // || defined(BRINGUP_DEBUG)
                ioe_led_drv.i2cEnable = FALSE;
#endif
                break;
            }
#endif
#ifdef IOE_LED_VIA_SW_I2C
            case SWI2C_DEV_TYPE:
            {
                cSWi2cDrv_t *p_swi2cDrv;
                p_swi2cDrv = (cSWi2cDrv_t *)(& ioe_led_drv.i2cDrv);
                p_swi2cDrv->pConfig = (stSWi2cDevice_t *)p_led_dev;
                SWi2cDrv_Ctor(p_swi2cDrv, p_swi2cDrv->pConfig);
                ioe_led_drv.isReady = TRUE;
                ioe_led_drv.i2cEnable = TRUE;
                break;
            }
#endif
            default:
                ASSERT(0);
                break;
        }
        attached_device_index ++;
        p_led_dev = (tDevice*)getDevicebyId(LED_DEV_ID, &attached_device_index);
    }
}

void IoeLed_I2cEnable(bool enable)
{
    ioe_led_drv.i2cEnable = enable;
}

void IoeLed_Reset(void)
{
    // reset the chip
    GpioDrv_ClearBit(&led_gpio_drv, GPIO_OUT_LED_RESET);
    BSP_BlockingDelayMs(20);    // 
    GpioDrv_SetBit(&led_gpio_drv, GPIO_OUT_LED_RESET);
    BSP_BlockingDelayMs(6); // wait for a while for i2c writing
}

void IoeLed_Init(void)
{
#ifdef IOE_LED_VIA_SW_I2C
    cSWi2cDrv_t *p_swi2cDrv;
    p_swi2cDrv = (cSWi2cDrv_t *)(& ioe_led_drv.i2cDrv);
    if( ! SWi2cDrv_DeviceAvailable(p_swi2cDrv, 3) )
    {
        TP_PRINTF("\n\rError : Ioexpandor is not ready\n\r");
#ifdef HAS_I2C_BUS_DETECT
        SystemDrv_SetI2cBusStatus(I2C_ERROR_LED);
#endif
        ioe_led_drv.i2cEnable = FALSE;  // disable it
    }
#endif

    // write the initial register.
    IoeLed_WriteByte(IOE_REG_GPIO_CFG_A, 0x3f);     // out4-9 NC on board, set to input
    IoeLed_WriteByte(IOE_REG_GPIO_CFG_B, 0x0f);     // out0->out3 : blink mode/input
    IoeLed_WriteByte(IOE_REG_GPIO_INT_A, 0x3f);     // disable out4-9 interrupt
    IoeLed_WriteByte(IOE_REG_GPIO_INT_B, 0x0f);     // disable out0-3 interrupt
    IoeLed_WriteByte(IOE_REG_CONTROL, (0x00|IOE_CONTROL_ISEL_QUARTER));     // max output current select
    IoeLed_WriteByte(IOE_REG_GPMD_A, 0x3f);     // out4-9 NC on board, set to GPIO
    IoeLed_WriteByte(IOE_REG_GPMD_B, 0x08);     // out0-2 set to LED mode
    IoeLed_WriteByte(IOE_REG_EN_BREATH, 0x00);     // disable all breath
    en_bre_reg = 0;
    en_bre_reg_shadow = en_bre_reg;

    // initial the LED mode
    IoeLedIndex_t led_id;
    uint32_t *pU32_cur, *pU32_shadow;
    pU32_cur = (uint32_t *)led_mode;
    pU32_shadow = (uint32_t *)ioe_led_mode_shadow;
    for(led_id=LED_ID_START; led_id<LED_ID_MAX; led_id++)
    {
        led_mode[led_id].mode = (uint8_t)((IOE_LED_MODE_NORMAL<<4)|LED_BLINK_MODE_NONE);
        led_mode[led_id].outX = led_id;     // depend on the layout
        led_mode[led_id].dim_reg = (uint8_t)(led_mode[led_id].outX + IOE_REG_DIM0);
        led_mode[led_id].dim_value = 0x00;    // off
        // save to shadow 
        *pU32_shadow = *pU32_cur;
        pU32_cur ++;
        pU32_shadow ++;
    }
}

void IoeLed_Refresh(void)
{
    IoeLedIndex_t led_id;
    uint32_t *pU32_cur, *pU32_shadow;
    uint32_t update_breath=0;
    uint8_t mode, blink_mode, out_x;
    IoeLedMode_t *p_led_mode;

#ifdef LED_ONLY_IN_ACTIVE_STATUS
    if( SYSTEM_STATUS_WORKING != SystemDrv_GetSystemStatus() )
    {
//        GpioDrv_ClearBit(&led_gpio_drv, GPIO_OUT_LED_RESET);
        return ;
    }
#endif
    
    pU32_cur = (uint32_t *)led_mode;
    pU32_shadow = (uint32_t *)ioe_led_mode_shadow;

    // check whether we need to update
    for(led_id=LED_ID_START; led_id<LED_ID_MAX; led_id++)
    {
        if( *pU32_cur ==  *pU32_shadow )
        {
            pU32_cur ++;
            pU32_shadow ++;
            continue;
        }

        if( en_bre_reg_shadow )
        {
            update_breath = 1;
            IoeLed_WriteByte(IOE_REG_EN_BREATH, 0x00);
        }
        
        p_led_mode = (IoeLedMode_t *)pU32_cur;

        mode = (p_led_mode->mode & 0xf0) >> 4;
        blink_mode = p_led_mode->mode & 0x0f;

        switch( mode )
        {
        case IOE_LED_MODE_NORMAL:
        {
            out_x = p_led_mode->outX;
            if( out_x < 6 ) // only out0-5 support blink mode, check it
            {
                en_bre_reg &= (uint8_t)(~(1<<out_x));
            }
            IoeLed_WriteByte(p_led_mode->dim_reg, p_led_mode->dim_value);
            break;
        }
        case IOE_LED_MODE_BLINK:
        {
            uint8_t fade_on_off, full_on_off;
            out_x = p_led_mode->outX;
            if( out_x < 6 ) // only out0-5 support blink mode, check it
            {
                en_bre_reg |= (uint8_t)(1<<out_x);
            }
            else
            {
                ASSERT(0);
            }
            switch( blink_mode )
            {
            case LED_BLINK_MODE_FAST:
            {
                fade_on_off = FADE_OFF_TIMER_512ms | FADE_ON_TIMER_512ms;
                full_on_off = FULL_OFF_TIMER_256ms | FULL_ON_TIMER_256ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            case LED_BLINK_MODE_MEDIUM:
            {
                fade_on_off = FADE_OFF_TIMER_1024ms | FADE_ON_TIMER_1024ms;
                full_on_off = FULL_OFF_TIMER_256ms | FULL_ON_TIMER_256ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            case LED_BLINK_MODE_SLOW:
            {
                fade_on_off = FADE_OFF_TIMER_2048ms | FADE_ON_TIMER_2048ms;
                full_on_off = FULL_OFF_TIMER_512ms | FULL_ON_TIMER_256ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            case LED_BLINK_MODE_APPEND:
                break;
            case LED_FLASH_MODE_FAST:
            {
                fade_on_off = FADE_OFF_TIMER_0ms | FADE_ON_TIMER_0ms;
                full_on_off = FULL_OFF_TIMER_256ms | FULL_ON_TIMER_256ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            case LED_FLASH_MODE_MEDIUM:
            {
                fade_on_off = FADE_OFF_TIMER_0ms | FADE_ON_TIMER_0ms;
                full_on_off = FULL_OFF_TIMER_256ms | FULL_ON_TIMER_512ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            case LED_FLASH_MODE_SLOW:
            {
                fade_on_off = FADE_OFF_TIMER_0ms | FADE_ON_TIMER_0ms;
                full_on_off = FULL_OFF_TIMER_256ms | FULL_ON_TIMER_1024ms;
                IoeLed_WriteByte(IOE_REG_FADE_TIMER, fade_on_off);
                IoeLed_WriteByte(IOE_REG_FULL_TIMER, full_on_off);
                break;
            }
            default :
                ASSERT(0);
                break;
            }
            break;
        }
        default:
            ASSERT(0);
            break;
        }

        *pU32_shadow = *pU32_cur;
        pU32_cur ++;
        pU32_shadow ++;
    }

    if( (en_bre_reg_shadow != en_bre_reg) || update_breath )
    {
        IoeLed_WriteByte(IOE_REG_EN_BREATH, en_bre_reg);
    }
    if( en_bre_reg )
    {
        IoeLed_BlinkGo(TRUE);
    }
    if( en_bre_reg_shadow != en_bre_reg )
    {
        p_led_mode = led_mode;
        for(led_id=LED_ID_START; led_id<LED_ID_MAX; led_id++)
        {
            IoeLed_WriteByte(p_led_mode->dim_reg, p_led_mode->dim_value);
            p_led_mode ++;
        }
    }
    en_bre_reg_shadow = en_bre_reg;
}

void IoeLed_SetupMode(IoeLedIndex_t led_id, uint8_t mode, uint8_t blink_mode, uint8_t dim_value)
{
    ASSERT( (mode < IOE_LED_MODE_MAX) && (blink_mode < LED_BLINK_MODE_MAX) );

    if( mode == IOE_LED_MODE_NORMAL )
        blink_mode = LED_BLINK_MODE_NONE;
    led_mode[led_id].mode = (mode << 4) | blink_mode;
    led_mode[led_id].dim_value = dim_value;    
}

void IoeLed_Standby(void)
{
#ifdef LED_ONLY_IN_ACTIVE_STATUS
    SystemDrv_SetSystemStatus(SYSTEM_STATUS_WORKING);
#endif
    // turn on the RED LED first
    IoeLed_SetupMode(LED_ID_BLUE, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
    IoeLed_SetupMode(LED_ID_RED, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, 0x20);
    IoeLed_SetupMode(LED_ID_GREEN, IOE_LED_MODE_NORMAL, LED_BLINK_MODE_NONE, IOE_DIM_VALUE_OFF);
#ifdef LED_ONLY_IN_ACTIVE_STATUS
    IoeLed_Refresh();
#endif
}

#endif  // HAS_IOE_LED

