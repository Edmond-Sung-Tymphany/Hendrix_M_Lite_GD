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
* @file     IQS_Commands.c                                                    *
* @brief    Commands specific to the IQS devices                              *
* @author   AJ van der Merwe - Azoteq PTY Ltd                                 *
* @version  V1.0.0                                                            *
* @date     24/08/2015                                                        *
* @note     This implementation is not using dynamic memory allocation,       *
*           but it is possible to do so, should that be a requirement         *
*******************************************************************************/


// C includes
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// User includes
#include "deviceTypes.h"
#include "IQS_Commands.h"
#include "tch_defines.h"
#include "I2C_Comms.h"
#include "IQS360A.h"
#include "IQS5xx.h"


/* Create Space for the I2C Device pointers with reference to IQS_Commands */
static I2C_Device_t *iqs360a_cmds = NULL;	// Pointer to the IQS360A object
static I2C_Device_t *iqs5xx_cmds = NULL;	// Pointer to the IQS5xx object

/**
 * @brief    Create the IQS devices as I2C devices and give them Default values
 * @param    None
 * @retval    [uint8_t] Status of this action
 */
uint8_t IQS_Setup_Devices(void)
{
	uint8_t status = RETURN_OK;

	iqs360a_cmds = NULL;
	iqs5xx_cmds = NULL;

	/* Get IQS360A */
	iqs360a_cmds = IQS360A_Get_Device();

	/* No memory allocated - this is a major error */
	if (iqs360a_cmds == NULL)
	{
		return ERR_NO_DEVICE;
	}

	/* Get IQS5xx */
	iqs5xx_cmds = IQS5xx_Get_Device();

	/* No memory allocated - this is a major error */
	if (iqs5xx_cmds == NULL)
	{
		return ERR_NO_DEVICE;
	}

	/* Now Setup devices */

	//EXTI_DeInit();		// Insure Fresh start

	/* iqs360a_cmds */
	iqs360a_cmds->I2C_Address = IQS360A_ADDR;			// I2C Address of the IQS360A
	iqs360a_cmds->Poll = Use_RDY;					// We should use the RDY
	iqs360a_cmds->Type = RDY_Active_Low;				// RDY Active Low
	iqs360a_cmds->Poll_Speed = Fast;					// Poll Fast
	iqs360a_cmds->RDY_Pin = IQS360A_RDY_PIN;			// RDY Pin for the IQS360A
	iqs360a_cmds->RDY_Port = IQS360A_RDY_PORT;		// RDY Port for the IQS360A
	iqs360a_cmds->Stop = I2C_Repeat_Start;			// At the moment, do not send stop after transfer
	iqs360a_cmds->Event_Mode = Inactive;				// Not in Event mode
	iqs360a_cmds->State = Set_Active_Channels;		// Set Active Channels State - this must be done first for IQS360A
	/* Attach the Line to the IQS device */
	iqs360a_cmds->Exti_Line = IQS360A_EXTI_LINE;
	//EXTI_Configuration((uint32_t)IQS360A_EXTI_IRQ, (uint8_t)IQS360A_RDY_PORT_SOURCE,
	//		IQS360A_RDY_PIN_SOURCE, (uint32_t)iqs360a_cmds->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)ENABLE);

	/* IQS5xx */
	iqs5xx_cmds->I2C_Address = IQS5XX_ADDR;			// I2C Address of the IQS5xx
	iqs5xx_cmds->Poll = Polling;					// We should use the Poll
	iqs5xx_cmds->Type = RDY_Active_High;			// RDY Active High
	iqs5xx_cmds->Poll_Speed = Fast;					// Poll Fast
	iqs5xx_cmds->RDY_Pin = IQS5XX_RDY_PIN;			// RDY Pin for the IQS5xx
	iqs5xx_cmds->RDY_Port = IQS5XX_RDY_PORT;		// RDY Port for the IQS5xx
	iqs5xx_cmds->Stop = I2C_Repeat_Start;			// At the moment, do not send stop after transfer
	iqs5xx_cmds->Event_Mode = Inactive;				// Event mode active
	iqs5xx_cmds->State = Init;						// Init State
	/* Attach the Line to the IQS device */
	iqs5xx_cmds->Exti_Line = IQS5XX_EXTI_LINE;

    ASSERT(IQS5XX_ADDR==(iqs5xx_cmds->pI2cDrv->pConfig->devAddress>>1));

    //Tymphany: Interrupt is enabled on AzIntegTouchKeyDrv_Ctor()
    /*
    EXTI_Configuration((uint32_t)IQS5XX_EXTI_IRQ, (uint8_t)IQS5XX_RDY_PORT_SOURCE,
                IQS5XX_RDY_PIN_SOURCE, (uint32_t)iqs5xx_cmds->Exti_Line, (EXTITrigger_TypeDef)EXTI_Trigger_Rising_Falling, (FunctionalState)ENABLE);*/
    return status;
}




/**
 * @brief    This is a callback function for the EXTI. This will be called
 *             if an interrupt happens. The RDY window will then be indicated
 *             for the IQS device
 * @param    [I2C_Device_t*] iqs - pointer to the IQS device that interrupted
 * @retval    None
 */
void IQS_RDY_window(I2C_Device_t* iqs)
{
    /* IQS Device is RDY for Comms */
    if( IQS_Get_RDY_WindowsOpen(iqs) )
        iqs->RDY_Window = 1;    // open window
    else
        iqs->RDY_Window = 0;    // close window

    /* Tymphany add */
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
    if(iqs->RDY_Window && iqs->State==Run)
    {
        IQS_Event_Generator(iqs);
    }
#endif    
}
    


/**
 * @brief    Get the status of the RDY line of the received I2C_Device
 * @param    [I2C_Device_t*] iqs - pointer to I2C Device to check
 * @retval    [uint8_t] RDY pin status
 */
uint8_t IQS_Get_RDY_Status(I2C_Device_t* iqs)
{
    int8 value= GpioDrv_ReadBit(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
    return value;
}
    


/**
 * Tymphany:
 *    Note this function is different with IQS_Get_RDY_Status()
 *    it compare low RDY GPIO value with ActiveLow/ActiveHigh,
 *    and return windows is open or close
 */
bool IQS_Get_RDY_WindowsOpen(I2C_Device_t* iqs)
{
    int8 value= GpioDrv_ReadBit(&pTouchKeyDrv->gpioDrv, iqs->RDY_GpioPin);
    
    if(iqs->Type == RDY_Active_High) //IQS572
        return (value)?1:0;
    else if(iqs->Type == RDY_Active_Low) //IQS333
        return (value)?0:1;
    else {
        ASSERT(0);
        return TRUE;
    }
}

/* Tymphany add */
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
void IQS_Event_Init(I2C_Device_t* iqs)
{
    iqs->miss_evt_num= 0;
    iqs->total_evt_num= 0;
}


void IQS_Event_Consumer(I2C_Device_t* iqs, const int32 total_evt_start_check, const int32 miss_evt_precent_limit, const int32 total_evt_max)
{
    /* Before init, event missing is resonable, thus only check on Run state 
     */
    ASSERT(iqs->State==Run);
    
    uint32 miss_evt_limit= 0;
    
    //Consume one event
    iqs->miss_evt_num--;

    // Check if miss too many events   
    // Ignore checking if miss_evt_limit==-1
    if(miss_evt_precent_limit!=-1 && iqs->total_evt_num > total_evt_start_check) {
        ASSERT(total_evt_max >= total_evt_start_check);
        miss_evt_limit= iqs->total_evt_num * miss_evt_precent_limit / 100;
        ASSERT(miss_evt_limit > iqs->miss_evt_num);
    }

    // Get only newer data, to know latest event missing information
    // Ignore reset if total_evt_max==-1
    if(total_evt_max!=-1 && iqs->total_evt_num > total_evt_max ) {
        iqs->miss_evt_num= 0;
        iqs->total_evt_num= 0;
    }
}


void IQS_Event_Generator(I2C_Device_t* iqs)
{
    /* Before init, event missing is resonable, thus only check on Run state 
     */
    ASSERT(iqs->State==Run);
    
    //ASSERT(iqs->miss_evt_num>=0);
    iqs->miss_evt_num++;
    iqs->total_evt_num++;
}
#endif /* HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR */



/**
 * @brief    This function gets called from the EXTI IRQ to determine if this is one
 *             of the IQS devices that caused the RDY interrupt
 * @param    None
 * @retval    None
 */
void IQS_Check_EXTI(void)
{
    if (EXTI_GetITStatus((uint32_t)iqs360a_cmds->Exti_Line)) //PC11
    {
        IQS_RDY_window(iqs360a_cmds);	// IQS360A is RDY
        EXTI_ClearITPendingBit((uint32_t)iqs360a_cmds->Exti_Line); // clear the IPB
    }
    if (EXTI_GetITStatus((uint32_t)iqs5xx_cmds->Exti_Line)) //PC12 
    {
        IQS_RDY_window(iqs5xx_cmds);    // IQS5xx is RDY
        EXTI_ClearITPendingBit((uint32_t)iqs5xx_cmds->Exti_Line); // clear the IPB
    }
}


/* Tymphany: this is power on/off code from sample code, we do not need */
#if 0
/**
 * @brief    Switch Power to IQS Devices ON or OFF
 * @param    [uint8_t] onOff - power ON or power OFF
 * @retval    [uint8_t] Status - error = !success = 0
 */
uint8_t IQS_Power(uint8_t onOff)
{
    uint8_t status = RETURN_OK;

    /* Switch Power on */
    if (onOff == ON)
    {
        status = IQS_Power_On();
    }
    /* Switch Power off */
    else {
        status = IQS_Power_Off();
    }

    /* Return the Status */
    return status;
}


/**
 * @brief    Switch Power to IQS Devices ON
 * @param    None
 * @retval    [uint8_t] Status - error = !success = 0
 */
uint8_t IQS_Power_On(void)
{
    uint8_t status = RETURN_OK;
    uint8_t pin_status;

    /* Set VDDHI pin High */
    GPIO_SetBits(VDDHI_PORT, VDDHI_PIN);

    /* Just give the Pin a little time */
    //__asm__("NOP");

    /* Now read the Status to check whether the Status has changed */
    pin_status = GPIO_ReadOutputDataBit(VDDHI_PORT, VDDHI_PIN);

    /* Power Successfully Off */
    if (pin_status == LOW)
    {
        status = ERR_VDDHI_NOT_ON;
    }
    /* Failed to switch off */
    else {
        status = RETURN_OK;
    }

    return status;
}

/**
 * @brief    Switch Power to IQS Devices OFF
 * @param    None
 * @retval    [uint8_t] Status - error = !success = 0
 */
uint8_t IQS_Power_Off(void)
{
    uint8_t status = RETURN_OK;
    uint8_t pin_status;

    /* Set VDDHI pin low */
    GPIO_ResetBits(VDDHI_PORT, VDDHI_PIN);

    /* Just give the Pin a little time */
    //__asm__("NOP");

    /* Now read the Status to check whether the Status has changed */
    pin_status = GPIO_ReadOutputDataBit(VDDHI_PORT, VDDHI_PIN);

    /* Power Successfully Off */
    if (pin_status == LOW)
    {
        status = RETURN_OK;
    }
    /* Failed to switch off */
    else {
        status = ERR_VDDHI_NOT_OFF;
    }

    return status;
}
#endif
