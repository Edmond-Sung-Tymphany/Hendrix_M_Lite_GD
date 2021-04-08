#include "product.config"

#ifdef HAS_GPIO_LED

#include "commontypes.h"
#include "stm32f0xx.h"
#include "stm32f0xx_rcc.h"
#include "stm32f0xx_gpio.h"
#include "trace.h"
#include "GpioLed.h"

typedef struct tagLedGpioConfig
{
    uint32_t    gpio_AHB_clock;     // GPIO group AHB clock
    GPIO_TypeDef *gpio_group;       // GPIOA/B/...
    uint16_t    gpio_pin_index;     // GPIO_Pin_x
}GLED_CONFIG_t;

const static GLED_CONFIG_t gled_config[GLED_ID_END]=
{
    { RCC_AHBPeriph_GPIOA, GPIOA, GPIO_Pin_11 },    // LED4 : AUX/Mini
    { RCC_AHBPeriph_GPIOB, GPIOB, GPIO_Pin_13 },    // LED2 : POWER
    { RCC_AHBPeriph_GPIOA, GPIOA, GPIO_Pin_8 },     // LED3 : Charging(portalbe) / RCA(large)
    { RCC_AHBPeriph_GPIOB, GPIOB, GPIO_Pin_12 },    // LED1 : AUX-in
};

static GLED_CONTROLLER_t gled_controller[GLED_ID_END];

static void GLED_OnOff(GLED_ID_t gled_id, uint8_t on)
{
    if( on )
    {
        GPIO_ResetBits(gled_config[gled_id].gpio_group, gled_config[gled_id].gpio_pin_index);
    }
    else
    {
        GPIO_SetBits(gled_config[gled_id].gpio_group, gled_config[gled_id].gpio_pin_index);
    }
    gled_controller[gled_id].cur_on = on;
}

void GLED_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    GLED_ID_t gled_id;

// the default is set to off, if you want to turn on, init here.
//    gled_controller[GLED_ID_BT].cur_on = 1;

    for(gled_id=GLED_ID_START; gled_id<GLED_ID_END; gled_id++)
    {
        // enable GPIO clock
        RCC_AHBPeriphClockCmd(gled_config[gled_id].gpio_AHB_clock, ENABLE);
        GPIO_InitStructure.GPIO_Pin = gled_config[gled_id].gpio_pin_index;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;

        GLED_OnOff(gled_id, 0/*gled_controller[gled_id].cur_on*/);

        GPIO_Init((GPIO_TypeDef *)(gled_config[gled_id].gpio_group), &GPIO_InitStructure);
        
    }
}

void GLED_SetupMode(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt)
{
    ASSERT( mode < GLED_MODE_LAST_MODE );
    ASSERT( gled_id < GLED_ID_END );
    
    gled_controller[gled_id].gled_mode = mode;
    gled_controller[gled_id].cur_tick = 0;
    gled_controller[gled_id].repeat = repeat_cnt;
    gled_controller[gled_id].finally_on = gled_controller[gled_id].cur_on;
}

void GLED_SetupModeWithCheck(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt)
{
    ASSERT( mode < GLED_MODE_LAST_MODE );
    ASSERT( gled_id < GLED_ID_END );

    if( gled_controller[gled_id].gled_mode == mode )
        return ;
    
    gled_controller[gled_id].gled_mode = mode;
    gled_controller[gled_id].cur_tick = 0;
    gled_controller[gled_id].repeat = repeat_cnt;
    gled_controller[gled_id].finally_on = gled_controller[gled_id].cur_on;
}

void GLED_SetupMode_2(GLED_ID_t gled_id, GLED_MODE_t mode, uint8_t repeat_cnt, uint8_t finally_on)
{
    ASSERT( mode < GLED_MODE_LAST_MODE );
    ASSERT( gled_id < GLED_ID_END );
    
    gled_controller[gled_id].gled_mode = mode;
    gled_controller[gled_id].cur_tick = 0;
    gled_controller[gled_id].repeat = repeat_cnt;
    gled_controller[gled_id].finally_on = finally_on;
}

void GLED_SetupOnOffMode(GLED_ID_t gled_id, GLED_MODE_t mode)
{
    uint8_t on;
    
    ASSERT( mode <= GLED_MODE_ON );
    ASSERT( gled_id < GLED_ID_END );

    on = (uint8_t)mode;
    gled_controller[gled_id].gled_mode = mode;
    gled_controller[gled_id].cur_tick = 0;
    gled_controller[gled_id].repeat = 0;
    gled_controller[gled_id].finally_on = on;

    GLED_OnOff(gled_id, on);
}

void GLED_PeriodicRefresh(void)
{
    uint32_t refresh_tick_cnt;
    GLED_ID_t gled_id;
    GLED_CONTROLLER_t *p_gled_ctrl;

    p_gled_ctrl = gled_controller;
    
    for(gled_id=GLED_ID_START; gled_id<GLED_ID_END; gled_id++)
    {
        p_gled_ctrl->cur_tick ++;
        refresh_tick_cnt = ((uint32_t)p_gled_ctrl->cur_tick);
        switch( p_gled_ctrl->gled_mode )
        {
        case GLED_MODE_OFF:
            GLED_OnOff(gled_id, 0);
            break;
        case GLED_MODE_ON:
            GLED_OnOff(gled_id, 1);
            break;
        case GLED_MODE_SLOW_FLASH:
            if( refresh_tick_cnt < 21 )
                GLED_OnOff(gled_id, 1);
            else
                GLED_OnOff(gled_id, 0);
            if( refresh_tick_cnt >= 24 )
            {
                if( p_gled_ctrl->repeat & 0x80 )    // always repeat
                {
                    p_gled_ctrl->cur_tick = 0;
                }
                else
                {
                    ASSERT( p_gled_ctrl->repeat );
                    p_gled_ctrl->repeat --;
                    if( p_gled_ctrl->repeat )
                    {
                        p_gled_ctrl->cur_tick = 0;
                    }
                    else
                    {
                        GLED_MODE_t mode;
                        mode = (GLED_MODE_t)(p_gled_ctrl->finally_on);
                        GLED_SetupMode(gled_id, mode, 0);
                    }
                }
            }
            break;
        case GLED_MODE_FLASH:
            if( refresh_tick_cnt < 9 )
                GLED_OnOff(gled_id, 1);
            else
                GLED_OnOff(gled_id, 0);
            if( refresh_tick_cnt >= 12 )
            {
                if( p_gled_ctrl->repeat & 0x80 )    // always repeat
                {
                    p_gled_ctrl->cur_tick = 0;
                }
                else
                {
                    ASSERT( p_gled_ctrl->repeat );
                    p_gled_ctrl->repeat --;
                    if( p_gled_ctrl->repeat )
                    {
                        p_gled_ctrl->cur_tick = 0;
                    }
                    else
                    {
                        GLED_MODE_t mode;
                        mode = (GLED_MODE_t)(p_gled_ctrl->finally_on);
                        GLED_SetupMode(gled_id, mode, 0);
                    }
                }
            }
            break;
        case GLED_MODE_DOUBLE_FLASH:
            if( (refresh_tick_cnt < 5) || (8<refresh_tick_cnt)&&(refresh_tick_cnt<13) )
                GLED_OnOff(gled_id, 1);
            else
                GLED_OnOff(gled_id, 0);
            if( refresh_tick_cnt >= 32 )
            {
                if( p_gled_ctrl->repeat & 0x80 )    // always repeat
                {
                    p_gled_ctrl->cur_tick = 0;
                }
                else
                {
                    ASSERT( p_gled_ctrl->repeat );
                    p_gled_ctrl->repeat --;
                    if( p_gled_ctrl->repeat )
                    {
                        p_gled_ctrl->cur_tick = 0;
                    }
                    else
                    {
                        GLED_MODE_t mode;
                        mode = (GLED_MODE_t)(p_gled_ctrl->finally_on);
                        GLED_SetupMode(gled_id, mode, 0);
                    }
                }
            }    
            break;
        case GLED_MODE_QUICK_FLASH:
            if( refresh_tick_cnt < 5 )
                GLED_OnOff(gled_id, 1);
            else
                GLED_OnOff(gled_id, 0);
            if( refresh_tick_cnt >= 8 )
            {
                if( p_gled_ctrl->repeat & 0x80 )    // always repeat
                {
                    p_gled_ctrl->cur_tick = 0;
                }
                else
                {
                    ASSERT( p_gled_ctrl->repeat );
                    p_gled_ctrl->repeat --;
                    if( p_gled_ctrl->repeat )
                    {
                        p_gled_ctrl->cur_tick = 0;
                    }
                    else
                    {
                        GLED_MODE_t mode;
                        mode = (GLED_MODE_t)(p_gled_ctrl->finally_on);
                        GLED_SetupMode(gled_id, mode, 0);
                    }
                }
            }            
            break;
        default :   // unknown mode
            ASSERT(0);
            break;
        }

        p_gled_ctrl ++;
    }
}


#endif  // HAS_GPIO_LED

