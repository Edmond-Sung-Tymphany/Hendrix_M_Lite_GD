/******************************************************************************
*                                                                             *
*                                                                             *
*                                Copyright by                                 *
*                                                                             *
*                              Azoteq (Pty) Ltd                               *
*                          Republic of South Africa                           *
*                                                                             *
*                           Tel: +27(0)21 863 0033                            *
*                          E-mail: info@azoteq.com                            *
*                                                                             *
*=============================================================================*
* @file     State_Machine.c                                                   *
* @brief    Implementation of state machine for touch driver                  *
* @author   AJ van der Merwe - Azoteq (PTY) Ltd                               *
* @version  V1.0.0                                                            *
* @date     25/08/2015                                                        *
*******************************************************************************/

// C includes
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// User includes
#include "deviceTypes.h"
#include "IQS_Commands.h"
#include "State_Machine_priv.h"
#include "tch_defines.h"
#include "I2C_Comms.h"
#include "IQS333.h"
#include "IQS263.h"
// Tymphany includes
#include "controller.h"
#include "SettingSrv.h"
#include "attachedDevices.h"
#include "AzIntegTouchKeyDrv.config"

static void Atouch_Update_Device_Status(cAzIntegTouchKeyDrv *me, I2C_Device_t* iqs, const bool connected);
uint32_t touchEventTimerMs = 0;
EventFlag_t event_flag = EVENT_NONE;
uint16_t iqs333_pre_wheel_1 = 0;
uint16_t iqs333_pre_wheel_2 = 0;
uint8_t  iqs263_pre_wheel = 0;
bool     iqs333_w2_has_touch = 0;
bool     iqs333_w1_has_touch = 0;
bool     iqs263_has_touch = 0;



/* Private Defines -------------------------------------*/
#define SM_BUFFER_SIZE        (uint8_t)(IQS333_NR_OF_ACTIVE_CHANNELS*4) // This is the Max that the Buffer should be long

/* Create Space for the I2C Devices with reference to State_Machine */
I2C_Device_t* iqs333_sm = NULL;
I2C_Device_t* iqs263_sm = NULL;

/* State Machine Power */
uint8_t Power = OFF;

//Noise Detect
static bool noiseBit = 0;
static uint32 noiseDetectTime= 0;

/**
 * @brief    Initialise the state machine. This includes initializing the IQS (I2C) devices
 *             and getting the pointers to their location for use by the state machine
 * @param    None
 * @retval    [uint8_t] Status of this function
 */
uint8_t state_machine_init(cAzIntegTouchKeyDrv *me)
{
    uint8_t res;
    pTouchKeyDrv = me;
    iqs333_sm = NULL;
    iqs263_sm = NULL;
    
    noiseBit = 0;
    noiseDetectTime= 0;

    /* Create the IQS devices */
    res = State_machine_Setup_Devices();//IQS_Setup_Devices();
    
    if (res)
    {
        // TODO should handle the error here
        return ERR_NO_DEVICE;
    }

    /* Switch on IQS Power */
    //res = IQS_Power(ON);

    /* Wait for devices to settle */
    Delay_ms(10);

    /* Did memory allocation fail? */
    /* Get device reference */
    iqs333_sm = IQS333_Get_Device();

    if (iqs333_sm == NULL)
    {
        return ERR_NO_DEVICE;
    }

    /* Did memory allocation fail? */
    /* Get device reference */
    iqs263_sm = IQS263_Get_Device();
    
    iqs333_sm->gpioObj = &(me->gpioDrv);
    iqs263_sm->gpioObj = &(me->gpioDrv);
    
    if (iqs263_sm == NULL)
    {
        return ERR_NO_DEVICE;
    }
    
    /* Switch the State Machine Power On */
    state_machine_power_on(me);

    return res; // return result
}

/**
 * @brief    Switch Power on to the state machine. This is just setting things up
 * @param    None
 * @retval   None
 */
void state_machine_power_on(cAzIntegTouchKeyDrv *me)
{
    /* Set the starting state for the IQS devices */
    iqs333_sm->State = Set_Active_Channels;    // Init state for IQS360A
    iqs263_sm->State = Set_Active_Channels;  // Init state for 572

    /* For new touch board (MCU control 572 by 333-rst),
     * Always enter bootlaoder and consider upgrade firmware on bootup.
     */
    
    /* Init Gesture Engine */
    //GS_init();

    /* Start Timer 2 as general Purpose timer */
    //timerEnable(TIM2, ENABLE);


    /* To reset power, we must pull down all pins
     */
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_POWER);
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_IOEXP_RST);
    I2C1_LowLevel_ForcePullLow(); //force pull low SCL/SDA
        
    Delay_ms(200);
    
    I2C1_LowLevel_Init(); //Set back open drain mode  for I2C
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_IOEXP_RST);
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_POWER);
    ATOUCH_PRINTF("\r\n\r\n\r\n*** Touch power reset done ***\r\n\r\n\r\n");
 
    Delay_ms(100);

    //extern void IoExpanderDrv_ReCtor_aw9110b();
    //IoExpanderDrv_ReCtor_aw9110b();
    
                
    //Reset 360a
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    //Delay_ms(100);
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    
    //GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    //Delay_ms(1);
    //GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
    IQS_Event_Init(iqs333_sm);
    IQS_Event_Init(iqs263_sm);
#endif
    
    Power = ON;    //switch Power On
}

/**
 * @brief    Switch Power off to the state machine. Remove unwanted memory
 * @param    None
 * @retval    None
 */
void state_machine_power_off(cAzIntegTouchKeyDrv *me)
{
    /* Switch off EXTI */
    //EXTI_Configuration((uint32_t)IQS360A_EXTI_IRQ, (uint8_t)IQS360A_RDY_PORT_SOURCE,
    //            IQS360A_RDY_PIN_SOURCE, (uint32_t)iqs360_sm->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)DISABLE);

    //EXTI_Configuration((uint32_t)IQS5XX_EXTI_IRQ, (uint8_t)IQS5XX_RDY_PORT_SOURCE,
    //                IQS5XX_RDY_PIN_SOURCE, (uint32_t)iqs5xx_sm->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)DISABLE);
    iqs333_sm = NULL;
    iqs263_sm = NULL;
    Power = OFF;    // switch Power Off
}



/* Check if we are in noise period
 */
bool noise_period()
{
    uint32 noise_period_ms= getSysTime() - noiseDetectTime;
    if( noiseBit )
    {
        if( noise_period_ms <= NOISE_EXTEND_PERIOD_MS )
        {
            return TRUE;
        }
        else
        {
            ATOUCH_PRINTF("\r\n\r\n ***** Leave touch noise period *********** \r\n\r\n");
            noiseBit= FALSE;
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }
}



/**
 * @brief    Process the state machine for the 2 IQS (I2C) devices
 * @detail    Different state machines are implemented for the 2
 *             devices. This can be changed in any manner.
 * @param    None
 * @retval    None
 */

void state_machine_process(cAzIntegTouchKeyDrv *me)
{
    uint8_t res = RETURN_OK;
    uint8_t buffer[SM_BUFFER_SIZE] = {0};
    uint8_t iqs333_touch = 0;
    uint8_t iqs263_touch = 0;
    uint8_t iqs263_wheel = 0;
    uint16_t iqs333_wheel_1 = 0;
    uint16_t iqs333_wheel_2 = 0;
    uint32  touch_timer = 0;

    if(iqs333_sm->RDY_Window && (getSysTime()-iqs333_sm->RDY_StartTime) < 17)
    {
        GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TOUCH_I2C_SW);
        switch (iqs333_sm->State)
        {
            case Set_Active_Channels:
                ATOUCH_PRINTF("\r\nIQS333: Set_Active_Channels\r\n");
                res = IQS333_Set_Active_Channels();
                break;
            case Init:
                ATOUCH_PRINTF("\r\nIQS333: Init\r\n");
                res = IQS333_Init();
                break;
            case Redo_ATI:
                ATOUCH_PRINTF("\r\nIQS333: Redo_ATI\r\n");
                res = IQS333_Redo_ATI();
                break;
            case Check_ATI:
                ATOUCH_PRINTF("\r\nIQS333: Check_ATI\r\n");
                res = IQS333_ATI_Status();
                break;
            case Run:
                res = IQS333_Read_Data(buffer);
                if(RETURN_OK == res)
                {
                    iqs333_touch = buffer[0];
                    if(iqs333_touch!= 0)
                    {
                        ATOUCH_PRINTF("\r\nIQS333: TOUCH_BYTES: 0x%x\r\n",iqs333_touch);
                    }
                    iqs333_wheel_1 = (uint16_t)(buffer[1] | buffer[2] << 8);      
                    iqs333_wheel_2 = (uint16_t)(buffer[3] | buffer[4] << 8);
                    if((iqs333_pre_wheel_1 != iqs333_wheel_1) || (iqs333_pre_wheel_2 != iqs333_wheel_2))
                    {
                        ATOUCH_PRINTF("\r\nIQS333: WHEEL_COORDS wheel_1: %d\r\n",iqs333_wheel_1);
                        ATOUCH_PRINTF("\r\nIQS333: WHEEL_COORDS wheel_2: %d\r\n",iqs333_wheel_2);
                    }
                }
                if(iqs333_touch & IQS333_CH_WHEEL_1)
                {
                    if(FALSE == iqs333_w1_has_touch)
                    {
                        iqs333_w1_has_touch = TRUE;
                        iqs333_pre_wheel_1 = iqs333_wheel_1;
                        if(EVENT_NONE == event_flag)
                        {
                            event_flag = EVENT_PAUSE_PLAY;
                            touchEventTimerMs = getSysTime();
                        }
                    }

                    if((iqs333_wheel_1 - iqs333_pre_wheel_1) > IQS333_JUMP)
                    {
                        /*filter, if jump from 0 to max, abandon 0*/
                        iqs333_pre_wheel_1 = iqs333_wheel_1;
                    }
                    else if((iqs333_pre_wheel_1 - iqs333_wheel_1) > IQS333_JUMP)
                    {
                        /*filter, if jump from max to 0, abandon 0*/
                    }
                    else if((iqs333_wheel_1 - iqs333_pre_wheel_1) > IQS333_DETA)
                    {
                        /*IQS333_DETA used to distinguish between touch and swipe,
                          IQS333_VOL_THRE used to control the volume change speed*/
                        event_flag = EVENT_VOL_UP;
                        if((iqs333_wheel_1 - iqs333_pre_wheel_1) > IQS333_VOL_THRE)
                        {
                            iqs333_pre_wheel_1 = iqs333_wheel_1;
                            KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_UP_KEY);
                            ATOUCH_PRINTF("\r\nIQS333: IQS333_CH_WHEEL_1 send EVENT_VOL_UP\r\n");
                        }
                    }
                    else if((iqs333_pre_wheel_1 - iqs333_wheel_1) > IQS333_DETA)
                    {
                        event_flag = EVENT_VOL_DOWN;
                        if((iqs333_pre_wheel_1 - iqs333_wheel_1) > IQS333_VOL_THRE)
                        {
                            iqs333_pre_wheel_1 = iqs333_wheel_1;
                            KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_DOWN_KEY);
                            ATOUCH_PRINTF("\r\nIQS333: IQS333_CH_WHEEL_1 send EVENT_VOL_DOWN\r\n");
                        }
                    }
                }
                else if(iqs333_touch & IQS333_CH_WHEEL_2)
                {
                    if(FALSE == iqs333_w2_has_touch)
                    {
                        iqs333_w2_has_touch = TRUE;
                        iqs333_pre_wheel_2 = iqs333_wheel_2;
                        if(EVENT_NONE == event_flag)
                        {
                            event_flag = EVENT_PRE;
                            touchEventTimerMs = getSysTime();
                        }
                    }
                    if((iqs333_wheel_2 - iqs333_pre_wheel_2) > IQS333_JUMP)
                    {
                        /*if jump from 0 to max, abandon max*/
                    }
                    else if((iqs333_pre_wheel_2 - iqs333_wheel_2) > IQS333_JUMP)
                    {
                        /*if jump from max to 0, abandon max*/
                        iqs333_pre_wheel_2 = iqs333_wheel_2;
                    }
                    else if((iqs333_wheel_2 - iqs333_pre_wheel_2) > IQS333_DETA)
                    {
                        event_flag = EVENT_VOL_UP;
                        if((iqs333_wheel_2 - iqs333_pre_wheel_2) > IQS333_VOL_THRE)
                        {
                            iqs333_pre_wheel_2 = iqs333_wheel_2;
                            KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_UP_KEY);
                            ATOUCH_PRINTF("\r\nIQS333: IQS333_CH_WHEEL_2 send EVENT_VOL_UP\r\n");
                        }
                    }
                    else if((iqs333_pre_wheel_2 - iqs333_wheel_2) > IQS333_DETA)
                    {
                        event_flag = EVENT_VOL_DOWN;
                        if((iqs333_pre_wheel_2 - iqs333_wheel_2) > IQS333_VOL_THRE)
                        {
                            iqs333_pre_wheel_2 = iqs333_wheel_2;
                            KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_DOWN_KEY);
                            ATOUCH_PRINTF("\r\nIQS333: IQS333_CH_WHEEL_2 send EVENT_VOL_DOWN\r\n");
                        }
                    }
                }
                else
                {
                    iqs333_w1_has_touch = FALSE;
                    iqs333_w2_has_touch = FALSE;
                }
                break;
            case Error:
                ATOUCH_PRINTF("\r\n***IQS333: Error***\r\n\r\n");
                break;
            default:
                iqs333_sm->State = Set_Active_Channels;
                break;
        }
        // Close the window
        GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TOUCH_I2C_SW);
        iqs333_sm->RDY_Window = 0;
    }

    if(iqs263_sm->RDY_Window && (getSysTime()-iqs263_sm->RDY_StartTime) < 4)
    {
        switch(iqs263_sm->State)
        {
            case Set_Active_Channels:
                ATOUCH_PRINTF("\r\nIQS263: Set_Active_Channels\r\n");
                res = IQS263_Set_Active_Channels();
                break;
            case Init:
                ATOUCH_PRINTF("\r\nIQS263: Init\r\n");
                res = IQS263_Init();
                break;
            case Redo_ATI:
                ATOUCH_PRINTF("\r\nIQS263: Redo_ATI\r\n");
                res = IQS263_Redo_ATI();
                break;
            case Check_ATI:
                ATOUCH_PRINTF("\r\nIQS263: Check_ATI\r\n");
                res = IQS263_ATI_Status();
                break;
            case Run:
                //res = IQS263_Read_Data(buffer);               
                res = IQS263_Read_wheel(buffer);
                if(RETURN_OK == res)
                {
                    iqs263_touch = buffer[0];
                    iqs263_wheel = buffer[1];
                    if(iqs263_touch != 0)
                    {
                        ATOUCH_PRINTF("\r\nIQS263: iqs263_touch: 0x%x\r\n",iqs263_touch);
                    }
                    if(iqs263_wheel != iqs263_pre_wheel)
                    {
                        ATOUCH_PRINTF("\r\nIQS263: iqs263_wheel: %d\r\n",iqs263_wheel);
                    }
                    if(iqs263_touch & IQS263_CH_WHEEL)
                    {
                        if(FALSE == iqs263_has_touch)
                        {
                            iqs263_has_touch = TRUE;
                            iqs263_pre_wheel = iqs263_wheel;
                            if(event_flag == EVENT_NONE)
                            {
                                event_flag = EVENT_NEXT;
                            }
                        }

                        if((iqs263_wheel - iqs263_pre_wheel) > IQS263_JUMP)
                        {
                            /*jump from 0 to 256, ignore*/
                        }
                        else if((iqs263_wheel - iqs263_pre_wheel) > IQS263_DETA)
                        {
                            event_flag = EVENT_VOL_DOWN;
                            if((iqs263_wheel - iqs263_pre_wheel) > IQS263_VOL_THRE)
                            {
                                iqs263_pre_wheel = iqs263_wheel;
                                KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_DOWN_KEY);
                                ATOUCH_PRINTF("\r\nIQS263: send EVENT_VOL_DOWN\r\n");
                            }
                        }
                        else if((iqs263_pre_wheel - iqs263_wheel) > IQS263_JUMP)
                        {
                            /*jump from 256 to 0, adopt 0*/
                            iqs263_pre_wheel = iqs263_wheel;
                        }
                        else if((iqs263_pre_wheel - iqs263_wheel) > IQS263_DETA)
                        {
                            event_flag = EVENT_VOL_UP;
                            if((iqs263_pre_wheel - iqs263_wheel) > IQS263_VOL_THRE)
                            {
                                iqs263_pre_wheel = iqs263_wheel;
                                KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, VOLUME_UP_KEY);
                                ATOUCH_PRINTF("\r\nIQS263: send EVENT_VOL_UP\r\n");
                            }
                        }
                    }
                    else
                    {
                        iqs263_has_touch = FALSE;
                    }
                }
                break;
            case Error:

                break;
            default:

                break;
        }
        // Close the window
        iqs263_sm->RDY_Window = 0;    
    }

    touch_timer = getSysTime() - touchEventTimerMs;

    if(iqs333_w1_has_touch == FALSE && iqs333_w2_has_touch == FALSE && iqs263_has_touch == FALSE)
    {
        switch(event_flag)
        {
            case EVENT_PRE:
                KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, PREV_KEY);
                ATOUCH_PRINTF("\r\nIQS333: send EVENT_PRE iqs333_touch:0x%x\r\n",iqs333_touch);
                break;
            case EVENT_PAUSE_PLAY:
                ATOUCH_PRINTF("\r\nIQS333: touch_timer=%d\r\n",touch_timer);
                if(touch_timer <= LONG_PRESS_TIME)
                {
                    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, CONNECT_KEY);
                    ATOUCH_PRINTF("\r\nIQS333: send short press EVENT_PAUSE_PLAY iqs333_touch:0x%x\r\n",iqs333_touch);
                } 
                break;
            case EVENT_NEXT:
                KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, NEXT_KEY);
                ATOUCH_PRINTF("\r\nIQS263: send EVENT_NEXT iqs263_touch:0x%x\r\n",iqs263_touch);
                break;
            default:
                break;
        }
        event_flag = EVENT_NONE;
        touchEventTimerMs = 0;
        ATOUCH_PRINTF("\r\nIQS333: reset event_flag as EVENT_NONE\r\n");
    }
    else if(touch_timer > LONG_PRESS_TIME)
    {
        /*send long press key there, and release the event_flag so that it will not be sent again*/
        switch(event_flag)
        {
            case EVENT_PAUSE_PLAY:
                KeySrv_SendKeyEvt_Direct(KEY_EVT_LONG_PRESS, CONNECT_KEY);
                event_flag = EVENT_NONE;
                ATOUCH_PRINTF("\r\nIQS333: send long press EVENT_PAUSE_PLAY iqs333_touch:0x%x\r\n",iqs333_touch);
                break;
            default:
                break;
        }
    }
    return;
}

/**
 * @brief    Clear the Data Buffer
 * @param    None
 * @retval    None
 */
void Clear_SM_Buffers(uint8_t* buffer, uint8_t size)
{
    memset((void *)(buffer), 0, size);    // Clear the buffer to 0
}

/* Tymphany:
 *  1. For production test, reocrd IQS572/IQS360A connection status to setting server 
 *  2. For MainApps, show error LED
 */
static void Atouch_Update_Device_Status(cAzIntegTouchKeyDrv *me, I2C_Device_t* iqs, const bool connected)
{
    ATOUCH_PRINTF("IQS%s: connected=%d\r\n", ((iqs==iqs333_sm)?"333":"263"), connected);
    Setting_Set(iqs->settingId, &connected);
}


/* Added by Tymphany */
/**
 * @brief    This function gets called from the EXTI IRQ to determine if this is one
 *             of the IQS devices that caused the RDY interrupt
 * @param    None
 * @retval    None
 */
void IQS_Check_EXTI(void)
{
    if (EXTI_GetITStatus(EXTI_LINE_TCH_333_RDY))
    {
        if(pTouchKeyDrv && pTouchKeyDrv->super_.isCreated && iqs333_sm)
        {
            IQS_RDY_window(iqs333_sm);    // IQS333 is RDY
            if(iqs333_sm->RDY_Window == 1)
            {
                iqs333_sm->RDY_StartTime = getSysTime();
            } 
        }
        EXTI_ClearITPendingBit(EXTI_LINE_TCH_333_RDY); // clear the IPB
    }
    if (EXTI_GetITStatus(EXTI_LINE_TCH_263_RDY))
    {
        if(pTouchKeyDrv && pTouchKeyDrv->super_.isCreated && iqs263_sm)
        {
            IQS_RDY_window(iqs263_sm);    // IQS263 is RDY
            if(iqs263_sm->RDY_Window == 1)
            {
                iqs263_sm->RDY_StartTime = getSysTime();
            }                        
        }
        EXTI_ClearITPendingBit(EXTI_LINE_TCH_263_RDY); // clear the IPB
    }
}

uint8_t State_machine_Setup_Devices(void)
{
	uint8_t status = RETURN_OK;

	I2C_Device_t *iqs333_cmds = NULL;
	I2C_Device_t *iqs263_cmds = NULL;

	/* Get IQS333 */
	iqs333_cmds = IQS333_Get_Device();

	/* No memory allocated - this is a major error */
	if (iqs333_cmds == NULL)
	{
		return ERR_NO_DEVICE;
	}

	/* Get IQS263 */
	iqs263_cmds = IQS263_Get_Device();

	/* No memory allocated - this is a major error */
	if (iqs263_cmds == NULL)
	{
		return ERR_NO_DEVICE;
	}

	/* Now Setup devices */

	//EXTI_DeInit();		// Insure Fresh start

	/* iqs333_cmds */
	iqs333_cmds->Poll = Use_RDY;					// We should use the RDY
	iqs333_cmds->Type = RDY_Active_Low;				// RDY Active Low
	iqs333_cmds->Poll_Speed = Fast;					// Poll Fast
    iqs333_cmds->Stop = I2C_Repeat_Start;			// At the moment, do not send stop after transfer
	iqs333_cmds->Event_Mode = Active;				// Not in Event mode
	iqs333_cmds->State = Set_Active_Channels;		// Set Active Channels State - this must be done first for IQS333
	/* Attach the Line to the IQS device */
    
    /* Tymphany add */
    iqs333_cmds->pI2cDrv = &pTouchKeyDrv->i2c333Drv;
    iqs333_cmds->RDY_GpioPin = GPIO_IN_TCH_333_RDY;
    //iqs333_cmds->settingId = SETID_IQS333_CONNECTED;
    
    ASSERT(IQS333_ADDR==(iqs333_cmds->pI2cDrv->pConfig->devAddress>>1));
	//EXTI_Configuration((uint32_t)IQS333_EXTI_IRQ, (uint8_t)IQS333_RDY_PORT_SOURCE,
	//		IQS333_RDY_PIN_SOURCE, (uint32_t)iqs333_cmds->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)ENABLE);

	/* IQS263 */
	iqs263_cmds->Poll = Use_RDY;					// We should use the Poll
	iqs263_cmds->Type = RDY_Active_Low;			// RDY Active High
	iqs263_cmds->Poll_Speed = Fast;					// Poll Fast
	iqs263_cmds->Stop = I2C_Repeat_Start;			// At the moment, do not send stop after transfer
	iqs263_cmds->Event_Mode = Active;				// Event mode active
	iqs263_cmds->State = Set_Active_Channels;						// Init State

    /* Tymphany add */
    iqs263_cmds->pI2cDrv = &pTouchKeyDrv->i2c263Drv;
    iqs263_cmds->RDY_GpioPin = GPIO_IN_TCH_263_RDY;
    //iqs263_cmds->settingId = SETID_IQS572_CONNECTED;
    
    ASSERT(IQS263_ADDR==(iqs263_cmds->pI2cDrv->pConfig->devAddress>>1));
    //Tymphany: Interrupt is enabled on AzIntegTouchKeyDrv_Ctor()
    /*
    EXTI_Configuration((uint32_t)IQS263_EXTI_IRQ, (uint8_t)IQS263_RDY_PORT_SOURCE,
                IQS263_RDY_PIN_SOURCE, (uint32_t)iqs263_cmds->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)ENABLE);*/
    return status;
}

