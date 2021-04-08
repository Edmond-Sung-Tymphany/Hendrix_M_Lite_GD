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
#include "State_Machine.h"
#include "tch_defines.h"
#include "I2C_Comms.h"
#include "IQS360A.h"
#include "IQS5xx.h"
#include "Gestures.h"

// Tymphany includes
#include "controller.h"
#include "SettingSrv.h"
#include "attachedDevices.h"
#include "../key_server/KeySrv_priv.h"


static void Atouch_Update_Device_Status(cAzIntegTouchKeyDrv *me, I2C_Device_t* iqs, const bool connected);
uint16 i2cReadFailCounter_360 = 0;
uint16 i2cReadFailCounter_572 = 0;
uint16 i2cReadFailCounter_ioe = 0;
bool   touchIsFailWaiting= 0; //TRUE means fail
uint32 touchFailTriggerTimeMs= 0;
int32  iqs360LastEvtTimeMs= 0;


/* Private Defines -------------------------------------*/
#define SM_BUFFER_SIZE        (uint8_t)(NR_OF_ACTIVE_CHANNELS*4) // This is the Max that the Buffer should be long

/* Create Space for the I2C Devices with reference to State_Machine */
I2C_Device_t* iqs360_sm = NULL;
I2C_Device_t* iqs5xx_sm = NULL;

Algorithm_State_t gesture_algorithm_sm = Idle;
/* State Machine Power */
uint8_t Power = OFF;

//static cAzIntegTouchKeyDrv * pAzKeyObj = NULL;


//Noise Detect
static bool noiseBit = 0;
static uint32 noiseDetectTime= 0;

/* Timeout Flag */
//extern volatile u8 Timer2Expired;

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
    iqs360_sm = NULL;
    iqs5xx_sm = NULL;
    gesture_algorithm_sm = Idle;
    
    i2cReadFailCounter_360 = 0;
    i2cReadFailCounter_572 = 0;
    i2cReadFailCounter_ioe = 0;
    iqs360LastEvtTimeMs= getSysTime();
    touchFailTriggerTimeMs= getSysTime();
    touchIsFailWaiting= FALSE;
    
    noiseBit = 0;
    noiseDetectTime= 0;

    /* Create the IQS devices */
    res = IQS_Setup_Devices();
    
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
    iqs360_sm = IQS360A_Get_Device();

    if (iqs360_sm == NULL)
    {
        return ERR_NO_DEVICE;
    }

    /* Did memory allocation fail? */
    /* Get device reference */
    iqs5xx_sm = IQS5xx_Get_Device();
    
    iqs360_sm->gpioObj = &(me->gpioDrv);
    iqs5xx_sm->gpioObj = &(me->gpioDrv);
    
    if (iqs5xx_sm == NULL)
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
    iqs360_sm->State = Set_Active_Channels;    // Init state for IQS360A
    iqs5xx_sm->State = BL_FirmwareUpgrade;  // Init state for 572
    gesture_algorithm_sm = Wait_For_Touch;

    /* For new touch board (MCU control 572 by 333-rst),
     * Always enter bootlaoder and consider upgrade firmware on bootup.
     */
    
    /* Init Gesture Engine */
    GS_init();

    /* Start Timer 2 as general Purpose timer */
    //timerEnable(TIM2, ENABLE);


    /* To reset power, we must pull down all pins
     */
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_POWER);
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_IOEXP_RST);
    I2C1_LowLevel_ForcePullLow(); //force pull low SCL/SDA
        
    Delay_ms(200);
    
    I2C1_LowLevel_Init(); //Set back open drain mode  for I2C
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_IOEXP_RST);
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_POWER);
    ATOUCH_PRINTF("\r\n\r\n\r\n*** Touch power reset done ***\r\n\r\n\r\n");
 
    Delay_ms(100);

    tIoeLedDevice *pIoeLedConfig = (tIoeLedDevice*) getDevicebyIdAndType(LED_DEV_ID, IO_EXPANDER_DEV_TYPE, NULL);
    ASSERT(pIoeLedConfig);
    if(((const tDevice*)pIoeLedConfig)->deviceSubType == AW9110B)
    {
        extern void IoExpanderDrv_ReCtor_aw9110b();
        IoExpanderDrv_ReCtor_aw9110b();
    }
    else if(((const tDevice*)pIoeLedConfig)->deviceSubType == AW9120)
    {
        extern void IoExpanderDrv_ReCtor_aw9120();
        IoExpanderDrv_ReCtor_aw9120();
    }
    else
    {
        ASSERT(0);
    }
                 
    //Reset 360a
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    Delay_ms(100);
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_360_RST);
    
    GpioDrv_ClearBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    Delay_ms(1);
    GpioDrv_SetBit(&me->gpioDrv, GPIO_OUT_TCH_572_RST);
    
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
    IQS_Event_Init(iqs360_sm);
    IQS_Event_Init(iqs5xx_sm);
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
    iqs360_sm = NULL;
    iqs5xx_sm = NULL;
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
    uint8_t buffer[SM_BUFFER_SIZE] = {0};
    uint8_t res = RETURN_OK;
    //I2C_Stop_t tempStop= (I2C_Stop_t)(-1);    // Temp storage of the Stop bit value
    //uint16_t timeout = 2000;      // 2 Sec Timeout
    //uint16_t timeout_psc = 7199;  // 100 us ticks
    uint8_t xyInfo = 0;             // IQS5xx Info Byte
    uint8_t gestureByte = 0;        // IQS5xx Gesture Byte

    uint8_t touch = 0;
    uint16_t deltaX = 0;
    uint16_t deltaY = 0;
    uint16_t X1 = 0;
    uint16_t Y1 = 0;
    //timerInit(TIM2, timeout, timeout_psc);     // set timer for 100 us ticks
    //Timer2Expired = 0;                        // Clear Timeout Flag

    //Print log if leave noise period
    noise_period();
	
    /* Check if we continuously detected that I2C read fail. if yes, it means touch is probably dead, 
     * we reset the power of the touch pannel
     */
    bool sys_sleep= FALSE;
    sys_sleep= *(bool*)Setting_GetEx(SETID_SYSTEM_SLEEP, &sys_sleep);
    if(touchIsFailWaiting && !sys_sleep)
    {
        int32 touchFailWaitMs= getSysTime() - touchFailTriggerTimeMs;
        if( touchFailWaitMs >= TOUCH_PAUSE_AFTER_FAIL_READ_MS )
        {
            //fail wait finish
            touchIsFailWaiting= FALSE;
            Power= OFF; //trigger reset
            ATOUCH_PRINTF("\r\n\r\n\r\nTouch fail wait finish after %dms\r\n\r\n\r\n", TOUCH_PAUSE_AFTER_FAIL_READ_MS);
        }
        else
        {   //still fail wait
            //static int tmp=0;
            //if( (tmp++)%1000==0 )
            //{                
            //    printf("touch recovery still need %ds\r\n", (TOUCH_PAUSE_AFTER_FAIL_READ_MS-touchFailWaitMs)/1000);
            //}
            return; 
        }
    }

    
    int32 iqs360IdleMs=  getSysTime() - iqs360LastEvtTimeMs;
    if(iqs360IdleMs >= TOUCH_360_IDLE_TIMEOUT_MS)
    {
        iqs360LastEvtTimeMs= getSysTime();
        Power= OFF; //trigger reset
        ATOUCH_PRINTF("\r\n\r\n\r\nTouch 360 idle timeout after %dms\r\n\r\n\r\n", TOUCH_360_IDLE_TIMEOUT_MS);
    }

    
    /* If the State Machine is off, first Initialise and then run */
    if (Power == OFF)
    {
        state_machine_init(me); // re-init state machine
            
        /* Record connection status for (1)Error LED and (2)production test */
        Atouch_Update_Device_Status(me, iqs360_sm, /*connected:*/FALSE);
        Atouch_Update_Device_Status(me, iqs5xx_sm, /*connected:*/FALSE);
    }

    
    /* If both Windows are open, handle the bus contention by
     * disabling the Stop bit
     */
    if (iqs360_sm->RDY_Window && iqs5xx_sm->RDY_Window)
    {
        //tempStop = iqs360_sm->Stop;
        iqs360_sm->Stop = I2C_Repeat_Start;
    }
    
    /* First chat with the IQS572, since as soon as there is a touch we need to know.
     * Then chat with the IQS360A after the IQS572
     */
    /* IQS5xx RDY to Chat */
    if (iqs5xx_sm->RDY_Window || iqs5xx_sm->State==BL_FirmwareUpgrade)    
    {
        /* Process the State Machine for the IQS5xx */
        switch (iqs5xx_sm->State)
        {
            case BL_FirmwareUpgrade:                
                ATOUCH_PRINTF("IQS572_FirmwareUpgrade\r\n");
                res = IQS5xx_BL_FirmwareUpgrade(); 
                
                //We must loss many IQS333 events while upgrading, thus re-start monitor
                #ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
                IQS_Event_Init(iqs5xx_sm); 
                IQS_Event_Init(iqs360_sm);
                #endif
                break;  
            case Init:
                
                ATOUCH_PRINTF("\r\nIQS572: Initial\r\n");
                iqs5xx_sm->Event_Mode = Inactive;    // by default in streaming mode
                res = IQS5xx_Init();                 // Initialize the IQS5xx for this application
                //ATOUCH_PRINTF("ized \r\n");
                /* Tymphany: if next status is run, means 572 is ready, report status change
                 */
                if( iqs5xx_sm->State == Run )
                {
                    Atouch_Update_Device_Status(me, iqs5xx_sm, /*connected:*/TRUE);
                }
                
                break;

            case Redo_ATI:
                ATOUCH_PRINTF("IQS572: RedoATI \r\n");
                
                /* Tymphany: not used by 572, add assert() to double confirm */
                ASSERT(0);
                
                gesture_algorithm_sm = Wait_For_Touch;    // just reset the state machine to waiting for a touch, the IQS572 auto calibrates internally
                break;

            case Run:
                /* Tymphany add */
                //ATOUCH_PRINTF("IQS572_Read_Data\r\n"); 
                res = IQS5xx_Read_Data(buffer);     // Read the IQS5xx to get the gesture info
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR
                if(res==RETURN_OK) {
                    IQS_Event_Consumer (iqs5xx_sm, IQS5XX_TOTAL_EVT_START_CHECK, IQS5XX_MISS_EVT_PERCENT_LIMIT, IQS5XX_TOTAL_EVT_MAX);
                }
#endif

                // Get the Info from the IQS5xx and pack correctly into the corresponding variables
                xyInfo = buffer[0];
                gestureByte = buffer[1];
                deltaX = buffer[2]<<8 | buffer[3];
                deltaY = buffer[4]<<8 | buffer[5];
//                touchID = buffer[6];
                X1 = buffer[7]<<8 | buffer[8];
                Y1 = buffer[9]<<8 | buffer[10];

                /* Noise detect. Only IQS572 firmware [5.8.13.14] support noise detect
                 */
#ifdef TOUCH_NOISE_DETECTION				
                uint16 noiseBitNow = xyInfo & 0x08;
                if( noiseBitNow )
                {
                    noiseBit = TRUE;
                    noiseDetectTime= getSysTime();
                    ATOUCH_PRINTF("\r\n\r\n ***** Enter touch noise period *********** \r\n\r\n");
#ifdef DEBUG_DEMO_FEATURE
                    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_NOISE_KEY);
#endif
                }
#endif /* TOUCH_NOISE_DETECTION */

                /* Start looking at the IQS360A as soon as a touch is seen on the IQS5xx - read the touch bits
                 * note 0x07 = Palm reject, otherwise normal touches. Plus only single touch application
                 */
                touch = xyInfo & 0x07;
                /* We are now waiting for a touch to be detected */
                if (gesture_algorithm_sm == Wait_For_Touch)
                {
                    if (touch == 0x01)
                    {
                        //ATOUCH_PRINTF("gestureByte, %d, X1: %d, Y1: %d\r",gestureByte,X1,Y1);

                        iqs360_sm->State = Run;                    // Start logging IQS360A data, or continue is it is running
                        /* set the mode to the next state */
                        gesture_algorithm_sm = Wait_For_Swipe;

                        /* Reset the timer, because we are still busy */
                        //timerInit(TIM2, timeout, timeout_psc);     // set timer for 100 us ticks
                        //Timer2Expired = 0;                        // Clear Timeout Flag

                        /* log the coordinates of the first touch location */
                        setFirstPoint(X1,Y1);
                    }
                }
                /* We are now waiting for a swipe to be detected */
                else if (gesture_algorithm_sm == Wait_For_Swipe)
                {
                    /* Check whether a gesture was seen */
                    if (gestureByte)
                    {
                        //ATOUCH_PRINTF("IQS572_Read: xy=%02x,g=%02x",xyInfo,gestureByte);
                        /* Check if there was a swipe,
                         *     Bit definitions - GestureByte
                            TAP                Bit0
                            TAP_N_HOLD        Bit1
                            LEFT_SWIPE        Bit2
                            RIGHT_SWIPE        Bit3
                            UP_SWIPE        Bit4
                            DOWN_SWIPE        Bit5
                        */
                        if ((gestureByte & 0x3C) != 0x00)
                        {
                            //ATOUCH_PRINTF("gestureByte, %d, X1: %d, Y1: %d\r",gestureByte,X1,Y1);
                            /* A swipe was detected, now determine the fand location */
                            gesture_algorithm_sm = Evaluate_Swipe;
                            /* log the coordinates of the touch location when the swipe was detected
                             * NOTE: client requested that swipe event be sent ONLY on touch release, hence deltaX and deltaY
                             * being used here...*/
                            setLastPoint(deltaX,deltaY);
                        }
                        else
                        {
                            /* A Tap or a Tap&Hold was detected, reset the state machine */
                            gesture_algorithm_sm = Reset_SM;
                            /* Now Get other gesture gesture */
                            get_gesture(buffer, noise_period());
                        }
                    }
                    else
                    {
                        /* if the touch is lost then reset the algorithm state machine */
                        if (touch != 0x01)
                        {
                            gesture_algorithm_sm = Reset_SM;
                        }
                    }
                }
                break;

            case Error:
                ATOUCH_PRINTF("IQS572: Error \r\n");
                break;

            default:
                iqs5xx_sm->State = Init;
                break;
        }
        /* Tymphany: Because every sub function set STOP bit, we do not need to set here
         */
        //iqs5xx_sm->Stop = I2C_Stop;

        /* Error Handling */
        if(res)
        {
            // If any read/write error occurred, setup the IQS5xx for this application again
            if(iqs5xx_sm->State!=BL_FirmwareUpgrade)
            {
                ATOUCH_PRINTF("IQS572: Error, back to Init\r\n");
                iqs5xx_sm->State = Init;
            }
            else
            {
                ATOUCH_PRINTF("IQS572: Firmware upgrade fail, stay on BL_FirmwareUpgrade\r\n");
            }
                
            /* Tymphany: reocrd connection status */
            Atouch_Update_Device_Status(me, iqs5xx_sm, /*connected:*/FALSE);
            i2cReadFailCounter_572++;
            res = RETURN_OK; // re-init since 360 may use it. 
        }
        else
        {
            //reset touch rest counter
            i2cReadFailCounter_572 = 0;
        }

        // Close the window
        iqs5xx_sm->RDY_Window = 0;
    }

    /* IQS360A RDY to Chat */
    if (iqs360_sm->RDY_Window)
    {
        /* Process the State Machine for the IQS360A */
        switch (iqs360_sm->State)
        {
            // This is the first state for the IQS360A, otherwise settings might not set
            case Set_Active_Channels:
                ATOUCH_PRINTF("\r\nIQS360A: Set_Active_Channels\r\n");
                                
                /* Tymphany: reocrd connection status */
                Atouch_Update_Device_Status(me, iqs360_sm, /*connected:*/FALSE);
                
                iqs360_sm->Event_Mode = Inactive;    // by default in streaming mode
                res = IQS360A_Set_Active_Channels();    // Set the active channels before setting up IQS360A
                break;

            case Init:
                ATOUCH_PRINTF("IQS360A: Initial\r\n");
                                
                /* Tymphany: reocrd connection status */
                Atouch_Update_Device_Status(me, iqs360_sm, /*connected:*/FALSE);

                res = IQS360A_Init();    // initialise the IQS360A
                //ATOUCH_PRINTF("ized\r\n");
                break;

            case Redo_ATI:                // Redo ATI
                                
                gesture_algorithm_sm = Reset_SM;
                ATOUCH_PRINTF("IQS360A: Redo_ATI\r\n");
                res = IQS360A_Redo_ATI();

                break;

            case Check_ATI:                // Check for ATI error
                res = IQS360A_ATI_Status();
                //ATOUCH_PRINTF("IQS360A: Check_ATI = %d \r\n",res);

                /* Tymphany: if next status is run, means 360a is ready, report status change
                 */
                if( iqs360_sm->State == Run )
                {
                    Atouch_Update_Device_Status(me, iqs360_sm, /*connected:*/TRUE);
                }

                break;

            case Run:                    // Run Loop
                res = IQS360A_Read_Data(buffer);    // Send buffer address with, to save the data in

#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR
                if(res==RETURN_OK) {
                    IQS_Event_Consumer (iqs360_sm, IQS360A_TOTAL_EVT_START_CHECK, IQS360a_MISS_EVT_PERCENT_LIMIT, IQS360a_TOTAL_EVT_MAX);
                }
#endif

#ifndef TEST_ALGORITHM
                /* Add data to the Buffers, ONLY if there is a touch on the IQS572 */
                if ((gesture_algorithm_sm == Wait_For_Touch) || (gesture_algorithm_sm == Wait_For_Swipe))
                {

                    /* Process data */
                    process_data(buffer);
                    /* Add Channel to Buffer */
                    add_to_buffer();
                }
#else
                gesture_algorithm_sm = Evaluate_Swipe;
#endif
                //__asm__("NOP");    // Debug
                break;

            /* No need to read the IQS360 data since there is no touch on the track pad. We are only interested in data during the swipe*/
            case Block_Update:
                /* Continue */
                break;

            case Error:                    // Error
                ATOUCH_PRINTF("\r\n*** IQS360A: Error ***\r\n\r\n");
                break;

            default:
                iqs360_sm->State = Set_Active_Channels;
                break;
        }

        /* Error Handling */
        if(res)
        {
            ATOUCH_PRINTF("\r\n*** IQS360A: Comms Error ***\r\n\r\n");
            iqs360_sm->State = Set_Active_Channels;
            res = 0;
                        
            /* Tymphany: 
             *    When 360 fail to init, it still fail many times even original code
             *    set state back to  Set_Active_Channels. Thus we reset chip when fail occurs.
             */
            //state_machine_power_off(me);                       
                                
            /* Tymphany: reocrd connection status */
            Atouch_Update_Device_Status(me, iqs360_sm, /*connected:*/FALSE);
            i2cReadFailCounter_360++;
        }
        else
        {
            i2cReadFailCounter_360 = 0;
            iqs360LastEvtTimeMs= getSysTime();
        }

        // Close the window
        iqs360_sm->RDY_Window = 0;

    }

             

    if( i2cReadFailCounter_572 >= TOUCH_I2C_FAIL_READ_TIMES_LIMIT || 
        i2cReadFailCounter_360 >= TOUCH_I2C_FAIL_READ_TIMES_LIMIT || 
        i2cReadFailCounter_ioe >= TOUCH_I2C_FAIL_READ_TIMES_LIMIT)
    {
        touchIsFailWaiting= TRUE; //means fail
        touchFailTriggerTimeMs= getSysTime();
        ATOUCH_PRINTF("\r\n\r\n\r\nTouch fail wait start, 572_fail=%d/%d, 360_fail=%d/%d, ioe=%d/%d \r\n\r\n\r\n", 
            i2cReadFailCounter_572,
            TOUCH_I2C_FAIL_READ_TIMES_LIMIT,
            i2cReadFailCounter_360,
            TOUCH_I2C_FAIL_READ_TIMES_LIMIT,
            i2cReadFailCounter_ioe,
            TOUCH_I2C_FAIL_READ_TIMES_LIMIT);        
    }
    
    /* Tymphany: Because every sub function set STOP bit, we do not need to set here
     */
    /* Restore Stop Bit of IQS360A */
    //iqs360_sm->Stop = tempStop;

    /* Now run the gesture state machine part after there has been comms with the IQS572 and IQS360 */
    switch (gesture_algorithm_sm)
    {
        case     Idle:
            /* keep in Idle until the IQS572 and the IQS360 are actually set up and streaming */
            if ((iqs5xx_sm->State == Run) && (iqs360_sm->State == Run))
            {
                gesture_algorithm_sm = Wait_For_Touch;
            }
            break;
        case     Wait_For_Touch:
            /* nothing done at this point, only during IQS572 comms */
            break;
        case     Wait_For_Swipe:
            /* nothing done at this point, only during IQS572 comms */
            break;
        case     Evaluate_Swipe:
            /* calls the gesture algorithm main function. uses the start and end points in the algorithm */
            evaluateSwipe( noise_period() );
            //doDataCollection();
            gesture_algorithm_sm = Reset_SM;
            break;
        case     Reset_SM:
            /* reset all variable */
            resetGestureVariables();
            /* reset all buffers */
            resetGestureBuffers();
            gesture_algorithm_sm = Idle;
            break;
        default:
            gesture_algorithm_sm = Idle;
            break;
    }

    //if (Timer2Expired)
    //{
        //state_machine_power_off();    // switch state machine power off
    //}
    //if(iqs360_sm->State != Block_Update)
    //{
        //timerEnable(TIM2, DISABLE);
    //}
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
    ATOUCH_PRINTF("IQS%s: connected=%d\r\n", ((iqs==iqs360_sm)?"360A":"572"), connected);
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
    if (EXTI_GetITStatus(EXTI_LINE_TCH_360_RDY))
    {
        if(pTouchKeyDrv && pTouchKeyDrv->super_.isCreated && iqs360_sm)
        {
            IQS_RDY_window(iqs360_sm);    // IQS360A is RDY
        }
        EXTI_ClearITPendingBit(EXTI_LINE_TCH_360_RDY); // clear the IPB
    }
    if (EXTI_GetITStatus(EXTI_LINE_TCH_572_RDY))
    {
        if(pTouchKeyDrv && pTouchKeyDrv->super_.isCreated && iqs360_sm)
        {
            IQS_RDY_window(iqs5xx_sm);    // IQS5xx is RDY
        }
        EXTI_ClearITPendingBit(EXTI_LINE_TCH_572_RDY); // clear the IPB
    }
}
