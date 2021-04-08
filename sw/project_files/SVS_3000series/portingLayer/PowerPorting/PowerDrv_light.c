/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for B&O CA17)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in B&O CA17
@author      Daniel Qin
@date        2016-03-17
@copyright (c) Tymphany Ltd. All rights reserved.

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "PowerDrv_light.config"
#include "trace.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "controller.h"

/*GPIO or ADC object*/
cGpioDrv    powerGpioDrv;
cADCDrv     adcDrv;

static eDcInSta dcInStatus = DC_IN_STA_ON;
static bool jackInOutStatus = FALSE;    /* Trigger Jack In ->High->FALSE, out -> Low -> TRUE */
static bool jackLevelStatus = FALSE;    /* Trigger Jack level High -> FALSE, Low -> TRUE */
static bool jackInFirstFlag = FALSE;

static bool isDrvReady = FALSE;

hwVersionTag hwVersionArray[HW_TYPE_NUM] =
{
    {"MP",      0,    573},  //HW_TYPE_MP
    {"PVT",     573,  756 }, //HW_TYPE_PVT
    {"DVT2",    756,  1085}, //HW_TYPE_DVT2
    {"DVT1",    1085, 1456}, //HW_TYPE_DVT1
    {"EVT2",    1459, 1750}, //HW_TYPE_EVT2
    {"EVT1",    1750, 2150}, //HW_TYPE_EVT1
    {"ES3",     2150, 2725}, //HW_TYPE_ES3
    {"ES2",     2725, 3185}, //HW_TYPE_ES2
    {"ES1",     3185, 3541}, //HW_TYPE_ES1
    {"PRE-ES",  3541, 3096}, //HW_TYPE_PRE_ES
};

/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, 50},
};

static void PowerDrv_CheckJackLevelStatus(cPowerDrv *me);
static void PowerDrv_CheckJackInOutStatus(cPowerDrv *me);
static void PowerDrv_CheckDcSense(cPowerDrv *me);

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/

void PowerDrv_Ctor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
    isDrvReady = FALSE;

    /* ADC for reading battery and AC input status*/
    uint16 attached_device_index = 0;
    tDevice *pPowerDev = NULL;

    pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
    while(pPowerDev != NULL)
    {
        switch(pPowerDev->deviceType)
        {
        case GPIO_DEV_TYPE:
          {
            tGPIODevice *pPowerGPIOConf = (tGPIODevice*)pPowerDev;
            GpioDrv_Ctor(&powerGpioDrv,pPowerGPIOConf);
            break;
          }
        case ADC_DEV_TYPE:
          {
            tADCDevice *pPowerAdcConf = (tADCDevice*)pPowerDev;
            ADCDrv_Ctor(&adcDrv, pPowerAdcConf);
            ADCDrv_StartScanning(&adcDrv);
            break;
          }
        default:
          ASSERT(0);
          return;
        }
        attached_device_index++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
    }

}

void PowerDrv_Xtor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
    isDrvReady = FALSE;
    ADCDrv_Xtor(&adcDrv);
    PowerDrv_DeinitialPower(me);

}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/
uint16  PowerDrv_InitialPower(cPowerDrv *me)
{
    /*need to change uint16 to int32, but keep it for now as it's used in other project*/
    uint16 delayTime;
    if(me->step < Q_DIM(powerUpSequence))
    {
        /* call the power up function according to step*/
        powerUpSequence[me->step].powerUpFunction(me);
        /* get delay time*/
        delayTime = powerUpSequence[me->step].delaytime;
        me->step++;
    }
    else
    { /* if reset is completed, return 0 */
        delayTime = 0;
        me->step = 0;
        isDrvReady = TRUE;
    }
    return delayTime;
}

void PowerDrv_DeinitialPower(cPowerDrv *me)
{
    // turn off the power control
    GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_POWER_CTRL);
    isDrvReady = FALSE;
}

void PowerDrv_EnterSleepMode()
{
//    TP_PRINTF("sleep\r\n");
//    BSP_ExpBlockingDelayMs(5); //wait print finish, for debug
//    
//    //before sleep
//    IWDG_ReloadCounter();
//    ADCDrv_ResetAdcConversionStatus();
//    PowerDrv_DisableSystemTimerInt();
//    PowerDrv_DisableEXTI_Config();
//    PowerDrv_EnablelWakeUpSources();
//    
//    //sleep
//    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
//    
//    //wakeup
//    PowerDrv_EnableSystemTimerInt();  

//    /* It seems WFI set system clock to default,
//     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
//     */
//    BSP_init_clock();
//	 
//    TP_PRINTF("wakeup\r\n");
}

void PowerDrv_Update(cPowerDrv *me)
{
    if(isDrvReady)
    {
        PowerDrv_CheckJackInOutStatus(me);
        PowerDrv_CheckJackLevelStatus(me);
        PowerDrv_CheckDcSense(me);
    }
}

void PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable)
{
    switch(setId)
    {
        case POWER_SET_ID_CTRL:
        {
            if(enable)
            {
                GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_POWER_CTRL);
            }
            else
            {
                GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_POWER_CTRL);
            }

            break;
        }
#ifdef USING_BD2242G_CTRL
        case POWER_SET_ID_BD2242G:
        {
            if(enable)
            {
                GpioDrv_SetBit(&powerGpioDrv, GPIO_BD2242G_CTRL);
            }
            else
            {
                GpioDrv_ClearBit(&powerGpioDrv, GPIO_BD2242G_CTRL);
            }
            break;
        }
#endif
        default:
        {
            break;
        }
    }
}


void PowerDrv_Get(cPowerDrv *me, ePowerGetId getId)
{
}


/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void PowerDrv_PowerUpStage1(cPowerDrv *me)
{
    PowerDrv_Set(me, POWER_SET_ID_CTRL, TRUE);
}



/* When jack in:
 *   1 unmute auxin input, MCU may detect auxin signel, and let ASE trigger source change
 *   2 DSP wakeup to detect auxin signal
 *  
 * When jack not appear:
 *   1 mute auxin input
 *   2 dsp sleep when no audio output
 */

static void PowerDrv_CheckJackLevelStatus(cPowerDrv *me)
{
    int32 adcValue;
    static uint8 statusCount = 0;
    bool levelStatus;

    if(jackInOutStatus)
    {
        adcValue = ADCDrv_GetData(&adcDrv, ADC_JACK);
        
        if(adcValue <= POWER_DRV_JACK_HIGH_LEVEL)
        {
            levelStatus = TRUE;
        }
        else
        {
            levelStatus = FALSE;
        }
        
        if(levelStatus != jackLevelStatus || jackInFirstFlag)
        {
            statusCount++;
            if(statusCount >= POWER_DRV_JACK_LEVEL_DETECT_TIMES)
            {
                statusCount = 0;
                jackLevelStatus = levelStatus;
                jackInFirstFlag = FALSE;
                Setting_Set(SETID_JACKIN_LEVEL, &levelStatus);

                PowerSrvJackStateEvt *pEvt = Q_NEW(PowerSrvJackStateEvt, JACK_STATE_SIG);
                pEvt->type = JACK_DET_LEVEL;
                pEvt->param = levelStatus;
                SendToServer(MAIN_APP_ID, (QEvt*)pEvt);
            }
        }
        else
        {
            statusCount = 0;
        }
        
    }
    else
    {
        jackLevelStatus = FALSE;
        statusCount = 0;
    }

}

static void PowerDrv_CheckJackInOutStatus(cPowerDrv *me)
{
    static int32 jackDetTimer = 0;
    static uint8 statusCount = 0;

    if ((jackDetTimer -= POWER_SRV_TIMEOUT_IN_MS) <= 0)
    {
        bool jackIn = GpioDrv_ReadBit(&powerGpioDrv, GPIO_IN_JACK_DET);

        /* Check the jack cable is plug in or out */
        if (jackIn != jackInOutStatus)
        {
            statusCount++;
            if (statusCount >= POWER_DRV_JACKIN_DETECT_TIMES)
            {
                statusCount = 0;
                jackInOutStatus = jackIn;
                jackInFirstFlag = jackIn ? TRUE : FALSE;
                Setting_Set(SETID_JACKIN_STATUS, &jackIn);

                PowerSrvJackStateEvt *pEvt = Q_NEW(PowerSrvJackStateEvt, JACK_STATE_SIG);
                pEvt->type = JACK_DET_IN_OUT;
                pEvt->param = jackIn;
                SendToServer(MAIN_APP_ID, (QEvt*)pEvt);
            }
        }
        else
        {
            statusCount = 0;
        }
        
        jackDetTimer = POWER_DRV_JACKIN_DETECT_IN_MS;
    }
}

static void PowerDrv_CheckDcSense(cPowerDrv *me)
{
    int32 adcValue;
    static uint8 statusCount = 0;
    eDcInSta dcSense;

    adcValue = ADCDrv_GetData(&adcDrv, ADC_AC_SENSE);       
    
    if (adcValue > POWER_DRV_AC_SENSE_LOW_LEVEL)
    {
        dcSense = DC_IN_STA_ON;
    }
    else
    {
        dcSense = DC_IN_STA_OFF;
    }

    if (dcSense != dcInStatus)
    {
        statusCount ++;
        //TP_PRINTF("status=[%d,%d]\r\n", dcInStatus, dcSense);
        if (statusCount >= POWER_DRV_AC_SENSE_DETECT_TIMES)
        {
            statusCount = 0;
            dcInStatus = dcSense;

            /* if switch to OFF mode, mute the amplifier immediately, and then
             * send to signal to MAIN_APP_ID.
             *
             */
            if (DC_IN_STA_OFF == dcSense)
            {
                //GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_AMP_MUTE);
            }

            PowerSrvInfoEvt *pEvt = Q_NEW(PowerSrvInfoEvt, POWER_STATE_SIG);
            pEvt->dcInStatus = dcSense;
            SendToServer(MAIN_APP_ID, (QEvt*)pEvt);
        }
    }
    else
    {
        statusCount = 0;
    }
}

static eHwVer PowerDrv_GetHwVersion()
{
    eHwVer hwVerIndex= HW_TYPE_NUM;
    int16 rawResult = 0;
    uint8 ii;
    ASSERT( ArraySize(hwVersionArray) == HW_TYPE_NUM );

    if (adcDrv.isCreated)
    {
        rawResult = (int16)ADCDrv_GetData(&adcDrv, adcDrv.ADCConfig->pAdcPinIoAttr[0].adcPin);
        if (ADC_DATA_IS_NOT_READY != rawResult)
        {
            for (ii = 0; ii < ArraySize(hwVersionArray); ii++)
            {
                if (hwVersionArray[ii].minVol <= rawResult && rawResult <= hwVersionArray[ii].maxVol)
                {
                    hwVerIndex = ii;
                    break;
                }
            }
            ASSERT(ii<ArraySize(hwVersionArray));
        }
        ADCDrv_StartScanning(&adcDrv);
    }
    return hwVerIndex;
}

