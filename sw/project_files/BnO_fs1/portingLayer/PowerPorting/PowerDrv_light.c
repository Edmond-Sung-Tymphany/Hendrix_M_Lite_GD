/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Power Driver Light Edition (for B&O FS1)
                  -------------------------

                  SW Module Document




@file        PowerDrv.c
@brief       It's the power driver for STM32F0xx, used in B&O FS1
@author      Daniel Qin
@date        2015-01-23
@copyright (c) Tymphany Ltd. All rights reserved.

Power sequence is have to be updated.
-------------------------------------------------------------------------------
*/

#include "PowerDrv_light_priv.h"
#include "trace.h"
#include "PowerSrv.h"
#include "PowerDrv_v2.h"
#include "SettingSrv.h"
#include "controller.h"
#include "I2CDrv.h"
#include "Timer.h"
#include "BatteryDrvRAJ240045.h"


/***********************************************
 * Definition
 ***********************************************/
#define POWERDRV_DEBUG

#ifndef POWERDRV_DEBUG
    // printf() silence if not debugging
    #undef TP_PRINTF
    #define TP_PRINTF
#endif

#define BATT_ACTUAL_MAX_RSOC_PERCENT (93)
#define HUNDRED_PERCENT (100)

/***********************************************
 * Global Variable
 ***********************************************/
/*GPIO object*/
cGpioDrv powerGpioDrv;

/*ADC object*/
cADCDrv adcDrv;

/* Variables */
static eDcInSta      dcInStatus        = DC_IN_STA_MAX;
static const char *  hwVersion         = NULL;
static uint16        _5vSen            = 0;
static uint16        pvddSen           = 0;
static eChargerState chargeStatus      = CHARGER_STA_MAX;
static int16         tempBatt          = 0;
static eTempLevel    batteryTempLevel  = TL_NORMAL;
static int32   batteryHavingRestPeriod = TIME_IS_UP;
static int32   dc_debouncer_start_ms         = 0;


/* I2C object */
cI2CDrv      batteryI2cObj;
cBatteryDrv  batteryDrv;

struct
{
    bool battNoResponse;
    bool ifBattCharging;
    bool ifBattCriticalSoh;
    bool constRegionInit;
} LocalBatteryFlags;

/***********************************************
 * Array
 ***********************************************/
/* Private functions / variables. Declare and drivers here */
static tpowerUpSequence powerUpSequence[] =
{
    {&PowerDrv_PowerUpStage1, GET_TICKS_IN_MS(POWER_STAGE1_DELAY)},
};

/* Note:  the member order of the table should be same as eBatteryInfoId */
tBattRegData battInfo[BATTERY_INFO_ID_MAX] = 
{
   // regAddr                    valueLength  settingId                  regValue  valid
    {RAJ240045_FW_VER_ADDR,             2,    SETID_BATTERY_SW,               0,   0}, // BATTERY_INFO_ID_FW_VER
    {RAJ240045_HW_VER_ADDR,             2,    SETID_BATTERY_HW,               0,   0}, // BATTERY_INFO_ID_HW_VER
    {RAJ240045_DESIGN_CAP_ADDR,         2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_DESIGN_CAP
    {RAJ240045_DESIGN_VOL_ADDR,         2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_DESIGN_VOL
    {RAJ240045_SERIAL_NUMBER_ADDR,      2,    SETID_BATTERY_SN,               0,   0}, // BATTERY_INFO_ID_SERIAL
    {RAJ240045_TOTAL_VOL_ADDR,          2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_TOTAL_VOL
    {RAJ240045_CURRENT_ADDR,            2,    SETID_BATTERY_CURRENT,          0,   0}, // BATTERY_INFO_ID_CURRENT
    {RAJ240045_ABSOLUTE_SOC_ADDR,       2,    SETID_BATTERY_CAPACITY_ASOC,    0,   0}, // BATTERY_INFO_ID_ASOC
    {RAJ240045_REMAINING_CAP_ADDR,      2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_REMAINING_CAP
    {RAJ240045_FULL_CHARGE_CAP_ADDR,    2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_FULL_CH_CAP
    {RAJ240045_CHARGE_CURRENT_ADDR,     2,    SETID_BATTERY_CHARGING_CURRENT, 0,   0}, // BATTERY_INFO_ID_CHARGE_CURRENT
    {RAJ240045_CHARGE_VOL_ADDR,         2,    SETID_BATTERY_CHARGING_VOLTAGE, 0,   0}, // BATTERY_INFO_ID_CHARGE_VOL
    {RAJ240045_BATT_STATUS_ADDR,        2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_STATUS
    {RAJ240045_CYCLE_COUNT_ADDR,        2,    SETID_BATTERY_CYCLE,            0,   0}, // BATTERY_INFO_ID_CYCLE
    //{RAJ240045_CELL_VOLT_1_ADDR,      2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_CELL_VOLT_1
    //{RAJ240045_CELL_VOLT_2_ADDR,      2,    SETID_MAX,                      0,   0}, // BATTERY_INFO_ID_CELL_VOLT_2
    {RAJ240045_AVG_TEMP_ADDR,           2,    SETID_BATTERY_TEMP,             0,   0}, // BATTERY_INFO_ID_AVG_TEMP
    {RAJ240045_CELL_TEMP_1_ADDR,        2,    SETID_BATTERY_TEMP_CELL1,       0,   0}, // BATTERY_INFO_ID_CELL_TEMP_1
    {RAJ240045_CELL_TEMP_2_ADDR,        2,    SETID_BATTERY_TEMP_CELL2,       0,   0}, // BATTERY_INFO_ID_CELL_TEMP_2
    {RAJ240045_AVG_CURRENT_ADDR,        2,    SETID_BATTERY_AVG_CURRENT,      0,   0}, // BATTERY_INFO_ID_AVG_CURRENT
    {RAJ240045_RELATIVE_SOC_ADDR,       2,    SETID_BATTERY_CAPACITY_RSOC,    0,   0}, // BATTERY_INFO_ID_RSOC, battery capacity 0~100
    {RAJ240045_SAFETY_STATUS_ADDR,      2,    SETID_BATTERY_SAFETY_STATUS,    0,   0}, // BATTERY_INFO_ID_SAFETY
    {RAJ240045_SOH_ADDR,                1,    SETID_BATTERY_HEALTH_SOH,       0,   0}, // BATTERY_INFO_ID_SOH
};


hwVersionTag hwVersionArray[] =
{
    {"MP2",     0,    400 },
    {"PVT",     400,  750 },
    {"DVT2",    750,  1100},
    {"DVT1",    1100, 1450},
    {"EVT2",    1450, 1750},
    {"EVT1",    1750, 2200},
    {"ES3",     2200, 2700},
    {"ES2",     2700, 3150},
    {"ES1",     3150, 3500},
    {"PRE-ES",  3500, 3900},
    {"MP",      3900, 4095},
};




/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/
void PowerDrv_Ctor(cPowerDrv *me)
{
    me->step = INITIAL_STEP;
    
    /* ADC for reading battery and AC input status*/
    uint16 attached_device_index = 0;
    tDevice *pPowerDev = NULL;
    
    PowerDrv_InitVariables(me);

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
            {
                ASSERT(0);
                return;
            }
        }
        attached_device_index++;
        pPowerDev = (tDevice*)getDevicebyId(POWER_DEV_ID,&attached_device_index);
    }

    PowerDrv_Set(me, POWER_SET_ID_CHARGER_ON, TRUE);
    
    /* Init Battery */
    batteryI2cObj.pConfig = (tI2CDevice*)getDevicebyIdAndType(BATT_DEV_ID, I2C_DEV_TYPE, NULL);
    BatteryDrv_Ctor(&batteryDrv, &batteryI2cObj);
    
    bool battExist= FALSE;
    Setting_Set(SETID_BATTERY_EXIST, &battExist);
    
    CHARGER_FAST(powerGpioDrv);

    PowerDrv_EnablelWakeUpSources();
}

void PowerDrv_Xtor(cPowerDrv *me)
{
    /* Fill me in! */
    me->step = INITIAL_STEP;
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
    }
    return delayTime;
}


void PowerDrv_DeinitialPower(cPowerDrv *me)
{
}


void PowerDrv_EnterSleepMode(cPowerDrv *me)
{
    //before sleep
    //Reset battery capacity level, in order to be able
    //to detect and process it next boot up
    uint8 batteryLevel = (uint8)BatteryStatus_NO_BATTERY;
    Setting_Set(SETID_BATTERY_CAPACITY_LEVEL, &batteryLevel);

    ADCDrv_ResetAdcConversionStatus(&adcDrv);
    PowerDrv_DisableSystemTimerInt();
    PowerDrv_DisableEXTI_Config();
    PowerDrv_InitVariablesBeforeSleep(me);

    GpioDrv_ClearBit(&powerGpioDrv, GPIO_OUT_TCH_360_RST);
    //sleep
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

    /*
     We do not enable 360 again because when boot up,
     MainApp transit state from LED-Booting (no KeySrv) to ASE-TK-Booting (have KeySrv),
     KeySrv and touch driver will do initialization.
     When enable 360, MCU try to access I2C and cause IQS572 firmware (v5.8.13.13) have unexpected RDY high.
     To workaround this problem and let MCU do not wakeup by repetaed 572-RDY interrupt,
     we disable all I2C device (include IQS360) on OFF mode except of IQS572.
    */
    //GpioDrv_SetBit(&powerGpioDrv, GPIO_OUT_TCH_360_RST);
    
    //after wakeup
    GpioDrv_EnableExtInterrupt(&powerGpioDrv, GPIO_IN_TCH_572_RDY, ExtiIntTri_Rising_Falling);
    GpioDrv_EnableExtInterrupt(&powerGpioDrv, GPIO_IN_TCH_360_RDY, ExtiIntTri_Rising_Falling);
    PowerDrv_EnableSystemTimerInt();
    
    /* It seems WFI set system clock to default,
     * cause system tick increase from 1ms to 8ms. We need to re-init clock here.
     */
    BSP_init_clock();
}

void PowerDrv_SetBattNoResponse(cPowerDrv *me, bool noResponse)
{
    /* battNOResponse occurs: stop charging
     * recover from battNoResponse: PowerDrv_Update() will enable charging later
     */
    if(LocalBatteryFlags.battNoResponse!=noResponse && noResponse)
    {
        TP_PRINTF("\r\n\r\n\r\n\r\n*** batt_exist: %d ***\r\n\r\n\r\n\r\n", !noResponse);
    }
    LocalBatteryFlags.battNoResponse = noResponse;

    bool battExist= !noResponse;
    Setting_Set(SETID_BATTERY_EXIST, &battExist);
}

void PowerDrv_UpdateOne(cPowerDrv *me, bool sleepUpdate)
{
    static uint32 batteryNoResponseTimer = 0;

    /*--------------------------------------------------------------------
     *  per tick 50ms (POWER_SRV_TIMEOUT_IN_MS)
     *--------------------------------------------------------------------
     */
    /* update DC in status */
    eDcInSta dcInStatus_now = (eDcInSta)IS_DC_IN(powerGpioDrv);

    if(dcInStatus_now != dcInStatus)
    {
        if (0 == dc_debouncer_start_ms)
        {
            dc_debouncer_start_ms = getSysTime();
        }

        if ((getSysTime() - dc_debouncer_start_ms) >= DC_DEBOUNCE_TIME_MS)
        {
            TP_PRINTF("PowerDrv: report change, dc=%d\r\n", dcInStatus_now);

            dcInStatus = dcInStatus_now;

            Setting_Set(SETID_IS_DC_PLUG_IN, &dcInStatus); // DC Plug in

            PowerDrv_ReportStateChange(me);

            if( dcInStatus_now && Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP) )
            {
                PowerSrvWakeUpEvent *pe = Q_NEW(PowerSrvWakeUpEvent, POWER_WAKE_UP_SIG);
                pe->powerSrvWakeUpType = AC_PLUG_IN_WAKE_UP_TYPE;
                QF_PUBLISH(&pe->super, NULL);
            }
        }
    }
    else
    {
        dc_debouncer_start_ms = 0;
    }
    /*--------------------------------------------------------------------
     *  per after cycle is done - resting period (BATTERY_HAVING_REST_PERIOD_MS)
     *--------------------------------------------------------------------
     */
    /* update charger status */
    eChargerState chargeStatusNow = PowerDrv_GetChargerStatus();

    if (Setting_IsReady(SETID_BATTERY_SAFETY_STATUS))
    {
        uint16 safetyStatus = *(uint16*)Setting_Get(SETID_BATTERY_SAFETY_STATUS);

        if (safetyStatus & (  RAJ240045_SAFETY_STATUS_COT_CUT
                            | RAJ240045_SAFETY_STATUS_SW_OC
                            | RAJ240045_SAFETY_STATUS_OVER_VOLT
                            | RAJ240045_SAFETY_STATUS_SHORT
                            | RAJ240045_SAFETY_STATUS_PF
                            | RAJ240045_SAFETY_STATUS_DOT_DUT
                            | RAJ240045_SAFETY_STATUS_SW_OD
                            | RAJ240045_SAFETY_STATUS_HW_OD))
        {
            chargeStatusNow = CHARGER_STA_ERROR;
        }
    }

    if (CHARGER_STA_MAX != chargeStatusNow)
    {
        Setting_Set(SETID_CHARGER_STATUS, &chargeStatusNow);  // Charger Status
    }

    if ( chargeStatusNow != chargeStatus )
    {
        chargeStatus = chargeStatusNow;
        if( dcInStatus != DC_IN_STA_MAX )
        {
            PowerDrv_ReportStateChange(me);
        }
    }

    /*
    If enable battery readin in sleep mode MCU try to access I2C and cause IQS572
    firmware (v5.8.13.13) have unexpected RDY high.
    To workaround this problem and let MCU do not wakeup by repetaed 572-RDY interrupt,
    we disable all I2C device (include battery) on OFF mode except for IQS572
    */
    if (TRUE == sleepUpdate)
    {
        return;
    }

    if (TIME_IS_UP >= batteryHavingRestPeriod)
    {
        eTpRet ret = BatteryDrv_readRegValue(&batteryDrv, &battInfo[me->curId]);

        if (TP_SUCCESS == ret)
        {
            PowerDrv_BattHandler(me, me->curId);
            ++me->curId;

            if (FALSE == LocalBatteryFlags.constRegionInit)
            {
                me->curId = BATTERY_INFO_ID_FW_VER;
                LocalBatteryFlags.constRegionInit = TRUE;
            }

            if (me->curId >= BATTERY_INFO_ID_MAX)
            {
                me->curId = BATTERY_IFNO_ID_CONST_REGION_END;

                if (Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP))
                {
                    batteryHavingRestPeriod = TIME_IS_UP;
                }
                else
                {
                    if (TRUE == LocalBatteryFlags.ifBattCharging)
                    {
                        batteryHavingRestPeriod = TIME_IS_UP;
                    }
                    else
                    {
                        batteryHavingRestPeriod = BATTERY_HAVING_REST_PERIOD_MS;
                    }
                }
            }

            PowerDrv_SetBattNoResponse(me, FALSE);
            batteryNoResponseTimer = 0;
        }
        else
        {
            batteryNoResponseTimer += me->timetick;
            if (batteryNoResponseTimer > POWER_DRIVER_BATTERY_NO_RESPONSE_CRITICAL_MS)
            {
                PowerDrv_SetBattNoResponse(me, TRUE);
                PowerDrv_InitBattVariables(me);
            }
        }
    }
    else
    {
        if (Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP))
        {
            batteryHavingRestPeriod = TIME_IS_UP;
        }
        else
        {
            batteryHavingRestPeriod -= me->timetick;
        }
    }

    if (battInfo[BATTERY_INFO_ID_STATUS].valid)
    {
        uint32 batt_status = battInfo[BATTERY_INFO_ID_STATUS].regValue;
        bool full_charge = ((batt_status & RAJ240045_STATUS_BIT_FULL_CHARGE) ? TRUE : FALSE);

        bool sleep = *(bool*)Setting_Get(SETID_SYSTEM_SLEEP);
        uint16 SpeakerTemperature = TL_NUM;

        SpeakerTemperature = *(uint16*)Setting_GetEx(SETID_TEMP_LEVEL_AUDIO, &SpeakerTemperature);
        bool speakerOverHeat = (SpeakerTemperature <= TL_WARN);

        if (FALSE == LocalBatteryFlags.ifBattCharging)
        {
            if ((FALSE == LocalBatteryFlags.battNoResponse)
            &&  ((FALSE == speakerOverHeat) || (TRUE == sleep))
            &&  (FALSE == full_charge)
            &&  (CHARGER_STA_ERROR != chargeStatus)
            &&  (DC_IN_STA_ON == dcInStatus)
            &&  (batteryTempLevel == TL_NORMAL) )
            {
                PowerDrv_Set(me, POWER_SET_ID_CHARGER_ON, TRUE);
            }
        }
        else
        {
            if ((TRUE == LocalBatteryFlags.battNoResponse)
            ||  ((TRUE == speakerOverHeat) && (FALSE == sleep))
            ||  (TRUE == full_charge)
            ||  (CHARGER_STA_ERROR == chargeStatus)
            ||  (DC_IN_STA_OFF == dcInStatus)
            ||  (batteryTempLevel != TL_NORMAL) )
            {
               PowerDrv_Set(me, POWER_SET_ID_CHARGER_ON, FALSE);
            }
        }
    }

    /*--------------------------------------------------------------------
     *  per 10sec (POWER_DRIVER_UPDATE_INTERVAL_MS)
     *-------------------------------------------------------------------- */ 
    if (!(Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP)))
    {
        me->timer += me->timetick;
        if(me->timer < me->period)
        {
            return;
        }
        me->timer = 0;
        PowerDrv_UpdateAdcValues();
        PowerDrv_PrintInfo(me);
    }
}


void PowerDrv_Update(cPowerDrv *me)
{
    if (Setting_IsReady(SETID_SYSTEM_SLEEP) && *(bool*)Setting_Get(SETID_SYSTEM_SLEEP))
    {
        PowerDrv_UpdateOne(me, TRUE);
    }
    else
    {
        PowerDrv_UpdateOne(me, FALSE);
    }
}


void PowerDrv_Set(cPowerDrv *me, ePowerSetId setId, bool enable)
{
    switch (setId)
    {
        case POWER_SET_ID_FUEL_GAUGE_SHUT_DOWN:
        {
            //ASE-TK send STORAGE only if AC=ON, thus FEP do not check AC here
            TP_PRINTF("\r\n\r\n *** PowerDrv_Set: shutdown battery *** \r\n\r\n");

            /* After shutdown battery, because of capacity still output current, MCU do not power off immediately.
             * At this moment, if press key to impact current, battery may wakeup again. Thus we need to disable charging.
             */
            CHARGER_OFF(powerGpioDrv);
            BSP_ExpBlockingDelayMs(500); 

            //Shutdown battery
            BatteryDrv_shutDown(&batteryDrv);

            /* Wait until DC insert or voltage down to zero
             */
            int i;
            for(i=0 ; i<60*60 ; i++)
            {
                BSP_ExpBlockingDelayMs(1000);                
                if( IS_DC_IN(powerGpioDrv) ) {
                    BSP_SoftReboot();
                }
                TP_PRINTF("Wait battery shutdown: %d/%d\r\n", i, 60*60);
            }

            /* If can not shutdown battery after 1 hour, reboot or assert */
            ASSERT(0);
            BSP_SoftReboot();

            break;
        }
        case POWER_SET_ID_OVERHEAT:
        {            
            break;
        }
        case POWER_SET_ID_CHARGER_ON:
        {
            if (enable)
            {
                CHARGER_ON(powerGpioDrv);
                LocalBatteryFlags.ifBattCharging = TRUE;
            }
            else
            {
                CHARGER_OFF(powerGpioDrv);
                LocalBatteryFlags.ifBattCharging = FALSE;
            }
            break;
        }
        case POWER_SET_ID_SLOW_CHARGER:
        {
            TP_PRINTF("Set charger speed = %s\r\n", (enable?"SLOW":"FAST") );
            if (enable)
            {
                CHARGER_SLOW(powerGpioDrv);
            }
            else
            {
                CHARGER_FAST(powerGpioDrv);
            }
            break;
        }
        default:
            break;
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
    /* Initialize DSP gpios, to disable dynamic boost on very begining stage
     * Note DSP driver will initialize again
     */
    tGPIODevice* pDspGPIOConf = NULL;
    pDspGPIOConf= (tGPIODevice*)getDevicebyIdAndType(DSP_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pDspGPIOConf);
    cGpioDrv gpioDsp;
    GpioDrv_Ctor(&gpioDsp,pDspGPIOConf);
}


static void PowerDrv_DisableEXTI_Config()
{
  EXTI_InitTypeDef   EXTI_InitStructure;

  EXTI_InitStructure.EXTI_Line = EXTI_Line1  | EXTI_Line2  | EXTI_Line3  | EXTI_Line4  |
                                 EXTI_Line5  | EXTI_Line6  | EXTI_Line7  | EXTI_Line8  |
                                 EXTI_Line9  | EXTI_Line10 | EXTI_Line11 |
                                 EXTI_Line13 | EXTI_Line16 |
                                 EXTI_Line19 | EXTI_Line21 | EXTI_Line22 | EXTI_Line23 |
                                 EXTI_Line27 ;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
  EXTI_InitStructure.EXTI_LineCmd = DISABLE;
  EXTI_Init(&EXTI_InitStructure);
}

static void PowerDrv_SetExtInterrupt(uint8 port, uint8 pin, uint32 extiLine,
                                                  EXTITrigger_TypeDef type, IRQn_Type irq, FunctionalState state)
{
    /* Enable the interrupts */
    EXTI_InitTypeDef   EXTI_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;

    /* Connect EXTI Line to gpio pin */
    SYSCFG_EXTILineConfig(port, pin);
    /* Configure EXTI line */
    EXTI_InitStructure.EXTI_Line = extiLine;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = type;
    EXTI_InitStructure.EXTI_LineCmd = state;
    EXTI_Init(&EXTI_InitStructure);

    /* Enable and set EXTI Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = irq;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd = state;
    NVIC_Init(&NVIC_InitStructure);
}

static void PowerDrv_EnablelWakeUpSources()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    /* Initial GPIO for AC interrupt as it's used for ADC in running time*/
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);  // for PC12, PC14
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE); // for PB15

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    //PC12 GPIO_IN_TCH_572_RDY,  PC14, GPIO_IN_POWER_KEY
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_14;
    GPIO_Init(GPIOC, &GPIO_InitStructure);    

    //PB15 GPIO_IN_DC_IN
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);    
    
    //PC12 GPIO_IN_TCH_572_RDY
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOC, EXTI_PinSource12,
                   EXTI_Line12, EXTI_Trigger_Rising_Falling, EXTI4_15_IRQn, ENABLE);
    
    //PC14 GPIO_IN_POWER_KEY
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOC, EXTI_PinSource14,
                   EXTI_Line14, EXTI_Trigger_Rising_Falling, EXTI4_15_IRQn, ENABLE);
    
    //PB15 GPIO_IN_DC_IN
    PowerDrv_SetExtInterrupt(EXTI_PortSourceGPIOB, EXTI_PinSource15,
                   EXTI_Line15, EXTI_Trigger_Rising_Falling, EXTI4_15_IRQn, ENABLE);
}

static void PowerDrv_DisableSystemTimerInt()
{
    uint32 tmp = SysTick->CTRL;
    tmp = tmp & ~SysTick_CTRL_TICKINT_Msk;
    tmp = tmp & ~SysTick_CTRL_ENABLE_Msk;
    SysTick->CTRL  = tmp;
}


static void PowerDrv_InitVariables(cPowerDrv *me)
{
    _5vSen       = 0;
    pvddSen      = 0;
    tempBatt     = 0;
    dc_debouncer_start_ms = 0;
    dcInStatus          = DC_IN_STA_MAX;
    Setting_Reset(SETID_IS_DC_PLUG_IN); // DC Plug in 
    
    PowerDrv_InitVariablesBeforeSleep(me);
}

static void PowerDrv_InitVariablesBeforeSleep(cPowerDrv *me)
{    
    tempBatt     = 0;
    batteryTempLevel = TL_NORMAL;    
    dc_debouncer_start_ms = 0;
    //dcInStatus        = DC_IN_STA_MAX;

    PowerDrv_SetBattNoResponse(me, FALSE); //LocalBatteryFlags.battNoResponse
    batteryHavingRestPeriod = 0;
    
    dc_debouncer_start_ms = 0;
    //dcInStatus        = DC_IN_STA_MAX;
    chargeStatus= CHARGER_STA_MAX;
    
    PowerDrv_InitBattVariables(me);
}

/* Init battery variable when
 * 1) Bootup
 * 2) Battery I2C communicate fail
 * 3) After sleep
 */
static void PowerDrv_InitBattVariables(cPowerDrv *me)
{
    /* When wakeup or remove/re-insert battery, read again for every fileds
     */

    //After wakeup, battery/charger status need to update
    int ii;
    for(ii = 0 ; ii < ArraySize(battInfo); ii++)
    {
        battInfo[ii].valid    = FALSE;
        battInfo[ii].regValue = -1;
        if(battInfo[ii].settingId!=SETID_MAX)
        {
            Setting_Reset(battInfo[ii].settingId);
        }
    }
    
    tempBatt     = 0;
    batteryTempLevel = TL_NORMAL;    

    LocalBatteryFlags.ifBattCharging    = FALSE;
    LocalBatteryFlags.ifBattCriticalSoh = FALSE;
    LocalBatteryFlags.constRegionInit   = FALSE;
}

static void PowerDrv_EnableSystemTimerInt()
{
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk |
                   SysTick_CTRL_TICKINT_Msk   |
                   SysTick_CTRL_ENABLE_Msk;                    /* Enable SysTick IRQ and SysTick Timer */
}

static eChargerState PowerDrv_GetChargerStatus()
{
    bool chValue = Get_CHARGER_STAT(powerGpioDrv);

           eChargerState newState  = CHARGER_STA_MAX;
    static eChargerState stabState = CHARGER_STA_MAX;
    static eChargerState prevState = CHARGER_STA_MAX;

    static uint8 switchCount                = 0;
    static uint32 startDebounceTime         = 0;
    static eChargerDebouncingState debState = CHARGER_STATE_STABLE;

    if(chValue)
    {
        newState = CHARGER_STA_CHARGING_DONE;
    }
    else
    {
        newState = CHARGER_STA_CHARGING;
    }

    switch(debState)
    {
        case CHARGER_STATE_STABLE:
        {
            if (newState != prevState)
            {
                prevState = newState;
                switchCount = 0;
                debState = CHARGER_STATE_DEBOUNCING;
                startDebounceTime = getSysTime();
            }
            break;
        }
        case CHARGER_STATE_DEBOUNCING:
        {
            if (newState != prevState)
            {
                prevState = newState;
                switchCount++;
            }

            if ((getSysTime() - startDebounceTime) >= CHARGER_ERROR_DEBOUNCING_LENGTH_MS )
            {
                if (0 == switchCount)
                {
                    stabState = newState;
                    debState = CHARGER_STATE_STABLE;
                }
                else if (switchCount >= CHARGER_ERROR_PERIOD_COUNT)
                {
                    stabState = CHARGER_STA_ERROR;
                    debState = CHARGER_STATE_ERROR;
                    startDebounceTime = getSysTime();
                }
                else
                {
                    switchCount = 0;
                    startDebounceTime = getSysTime();
                }
            }
            break;
        }
        case CHARGER_STATE_ERROR:
        {
            if ((getSysTime() - startDebounceTime) >= CHARGER_ERROR_PERIOD_LENGTH_WITH_TOLERANCE_MS )
            {
                switchCount = 0;
                debState = CHARGER_STATE_DEBOUNCING;
                startDebounceTime = getSysTime();
            }
            else if (newState != prevState)
            {
                prevState = newState;
                startDebounceTime = getSysTime();
            }
            break;
        }
        default:
            break;

    }

    return stabState;
}


static void PowerDrv_BattHandler(cPowerDrv *me, eBatteryInfoId currId)
{    
    if( !battInfo[currId].valid ) {
        return;
    }
    
    switch(currId)
    {            
        case BATTERY_INFO_ID_AVG_TEMP:
        {
            //eTempLevel tempLevelBattPrev= batteryTempLevel;
            int16 tempBattNew = battInfo[BATTERY_INFO_ID_AVG_TEMP].regValue;
            PowerDrv_UpdateTempLevel(tempBattNew,  &tempBatt,  &batteryTempLevel,  battTempLevels, ArraySize(battTempLevels));
            //PowerDrv_CheckTempLevel(me, batteryTempLevel, tempLevelBattPrev);
            Setting_Set(SETID_BATTERY_TEMP,       &tempBattNew);
            Setting_Set(SETID_BATTERY_TEMP_LEVEL, &batteryTempLevel);
            break;
        }            
        case BATTERY_INFO_ID_RSOC:
        {
            uint16 batteryRsocUser = 0;

            batteryRsocUser = *(uint16*)Setting_Get(SETID_BATTERY_CAPACITY_RSOC);
            batteryRsocUser = (batteryRsocUser * HUNDRED_PERCENT) / BATT_ACTUAL_MAX_RSOC_PERCENT;
            batteryRsocUser = batteryRsocUser > HUNDRED_PERCENT ? HUNDRED_PERCENT : batteryRsocUser;

            Setting_Set(SETID_BATTERY_CAPACITY_RSOC_USER, &batteryRsocUser);

            uint8 batteryCapacityLevel = (uint8)PowerDrv_GetBatteryStatus( batteryRsocUser );
            /*
            Two additional conditions to declare Ctritical state of charge:
            ASOC <= Critical value (6% ASOC)
            SOH < Critical SOH

            ASOC will always hit a critical barrier before RSOC.
            */

            if (TRUE == battInfo[BATTERY_INFO_ID_ASOC].valid)
            {
                if (battInfo[BATTERY_INFO_ID_ASOC].regValue < ASOC_CRITICAL)
                {
                    batteryCapacityLevel = BatteryStatus_LEVEL_CRITICAL;
                }
            }

            //Get previous battery level2
            uint8 batteryLevelPrev= (uint8)BatteryStatus_NO_BATTERY;
            if( Setting_IsReady(SETID_BATTERY_CAPACITY_LEVEL) )
            {
                batteryLevelPrev= *(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_LEVEL);
            }
            //Report change, if battery level is changed
            if( batteryLevelPrev != batteryCapacityLevel )
            {
                TP_PRINTF("PowerDrv: report change, batt-level=%d -> %d\r\n", batteryLevelPrev, batteryCapacityLevel); 
                Setting_Set(SETID_BATTERY_CAPACITY_LEVEL, &batteryCapacityLevel);
                if( dcInStatus != DC_IN_STA_MAX )
                {
                    PowerDrv_ReportStateChange(me);
                }
            }
            break;
        }
        case BATTERY_INFO_ID_SOH:      
        {
            uint8 batteryHealthLevel = (uint8)PowerDrv_GetHealthStatus();
            Setting_Set(SETID_BATTERY_HEALTH_LEVEL, &batteryHealthLevel);
            Setting_Set(SETID_BATTERY_HEALTH_SOH,   &battInfo[BATTERY_INFO_ID_SOH].regValue);
            break;
        }       
        default:
            break;
    }
}


static void PowerDrv_UpdateAdcValues()
{
    int32 rawResult = 0;
    uint16 ii;
    ASSERT(adcDrv.isCreated);
    
    //HW Version
    if( !Setting_IsReady(SETID_HW_VER) )
    {
        rawResult = ADCDrv_GetData(&adcDrv, ADC_HW_VER);
        if (ADC_DATA_IS_NOT_READY != rawResult)
        {
            for (ii = 0; ii < ArraySize(hwVersionArray); ii++)
            {
                if (hwVersionArray[ii].minVol <= rawResult && rawResult <= hwVersionArray[ii].maxVol)
                {
                    hwVersion = hwVersionArray[ii].hwVersionStr;
                    break;
                }
            }
            ASSERT(ii<ArraySize(hwVersionArray));
            Setting_Set(SETID_HW_VER, hwVersion);
            TP_PRINTF("\r\n\r\nHW_VERSION: %s \r\n\r\n\r\n", hwVersion);
        }
    }
        
    
    //5V
    rawResult = ADCDrv_GetData(&adcDrv, ADC_5V_SEN);
    if (ADC_DATA_IS_NOT_READY != rawResult)
    {
        _5vSen = _5V_TO_mVOLT(rawResult);
        Setting_Set(SETID_5V_SEN, &_5vSen);
    }
   
    
    //PVDD
    rawResult = (int16)ADCDrv_GetData(&adcDrv, ADC_PVDD_SEN);
    if (ADC_DATA_IS_NOT_READY != rawResult)
    {
        pvddSen = PVDD_TO_mVOLT(rawResult);
        Setting_Set(SETID_PVDD_SEN, &pvddSen);
    }
    
    
    //Continus ADC scanning
    ADCDrv_StartScanning(&adcDrv);
}


static BatteryStatus PowerDrv_GetBatteryStatus(uint16 battCapacity)
{
    BatteryStatus battLevel= BatteryStatus_NO_BATTERY;
    
    if (battCapacity <= batt_capacity_critical)
    {
        battLevel = BatteryStatus_LEVEL_CRITICAL;
    }
    else if (battCapacity <= batt_capacity_low)
    {
        battLevel = BatteryStatus_LEVEL_LOW;
    }
    else if (battCapacity <= batt_capacity_mid)
    {
        battLevel = BatteryStatus_LEVEL_MID;
    }
    else if (battCapacity <= batt_capacity_extra)
    {
        battLevel = BatteryStatus_LEVEL_EXTRA;
    }
    else if (battCapacity <= batt_capacity_high)
    {
        battLevel = BatteryStatus_LEVEL_HIGH;
    }
    else
    {
        ASSERT(0);
    }

    return battLevel;
}


static eBattHealth PowerDrv_GetHealthStatus()
{
    eBattHealth battHealth = BATT_HEALTH_UNKNOWN;
    
    if(battInfo[BATTERY_INFO_ID_SOH].valid)
    {
        uint32 healthLevel = battInfo[BATTERY_INFO_ID_SOH].regValue;

        if (healthLevel <= healthLevelCritical)
        {
            battHealth = BATT_HEALTH_CRITICAL;
        }
        else if (healthLevel <= healthLevelPoor)
        {
            battHealth = BATT_HEALTH_POOR;
        }

        else if (healthLevel <= healthLevelGood)
        {
            battHealth = BATT_HEALTH_GOOD;
        }
        else
        {
            ASSERT(0);
        }
    }

    return battHealth;
}


static void PowerDrv_ReportStateChange(cPowerDrv *me)
{
    PowerSrvInfoEvt *evt = Q_NEW(PowerSrvInfoEvt, POWER_BATT_STATE_SIG);
    ASSERT(evt);
        
    evt->dcInStatus= dcInStatus;
    evt->batteryInfo.chargerState = chargeStatus;
    evt->batteryInfo.battStatus = BatteryStatus_NO_BATTERY;
    
    if( Setting_IsReady(SETID_BATTERY_CAPACITY_LEVEL) )
    {
        evt->batteryInfo.battStatus= (BatteryStatus)*(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_LEVEL);
    }
    QF_PUBLISH(&evt->super, NULL);    
}


static void PowerDrv_PrintInfo(cPowerDrv *me)
{ 
#ifdef POWERDRV_DEBUG
    uint16 batt_status= battInfo[BATTERY_INFO_ID_STATUS].regValue;
    bool ch_slow = GpioDrv_isBitSet(&powerGpioDrv, GPIO_OUT_SLOW_CH);

    uint8 batteryLevel= BatteryStatus_NO_BATTERY;
    if( Setting_IsReady(SETID_BATTERY_CAPACITY_LEVEL) )
    {
        batteryLevel= *(uint8*)Setting_Get(SETID_BATTERY_CAPACITY_LEVEL);
    }
    
    uint8 battHealthLevel= BATT_HEALTH_UNKNOWN;
    if( Setting_IsReady(SETID_BATTERY_HEALTH_LEVEL) )
    {
        battHealthLevel= *(uint8*)Setting_Get(SETID_BATTERY_HEALTH_LEVEL);
    }
    
    uint16 tempBatt=-1, tempBattCell1=-1, tempBattCell2=-1;
    uint16 batteryTempLevel= -1;
    tempBatt= *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP, &tempBatt);
    batteryTempLevel= *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP_LEVEL, &batteryTempLevel);
    tempBattCell1= *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP_CELL1, &tempBattCell1);
    tempBattCell2= *(uint16*)Setting_GetEx(SETID_BATTERY_TEMP_CELL2, &tempBattCell2);  
            
    TP_PRINTF("\r\nPower: DC=%s, ch-enable:%d, ch-status=%d, ch-speed=%s, PVDD=%d.%dmV, 5V=%d.%dmV\r\n", 
                  (dcInStatus?"ON":"OFF"), LocalBatteryFlags.ifBattCharging, chargeStatus, (ch_slow?"SLOW":"FAST"),
                  pvddSen/1000, (pvddSen/100)%10, _5vSen/1000, (_5vSen/100)%10);
    
    if( LocalBatteryFlags.battNoResponse )
    {
        TP_PRINTF("Batt: exist=0\r\n");
    }
    else
    {
        TP_PRINTF("Batt: fwv= 0x%04x, hwv= 0x%04x, serial= 0x%04x, cycle= %dC\r\n",
                      battInfo[BATTERY_INFO_ID_FW_VER].regValue,
                      battInfo[BATTERY_INFO_ID_HW_VER].regValue,
                      battInfo[BATTERY_INFO_ID_SERIAL].regValue,
                      battInfo[BATTERY_INFO_ID_CYCLE].regValue);
        TP_PRINTF("Batt: V=total:%dmV / charge:%dmV / design:%dmV \r\n",
                      battInfo[BATTERY_INFO_ID_TOTAL_VOL].regValue,
                      battInfo[BATTERY_INFO_ID_CHARGE_VOL].regValue,
                      battInfo[BATTERY_INFO_ID_DESIGN_VOL].regValue);
        TP_PRINTF("Batt: I=cur:%dmA / avg:%dmA / charge:%dmA\r\n",
                      (int16) battInfo[BATTERY_INFO_ID_CURRENT].regValue,
                      (int16) battInfo[BATTERY_INFO_ID_AVG_CURRENT].regValue,
                      battInfo[BATTERY_INFO_ID_CHARGE_CURRENT].regValue);
        TP_PRINTF("Batt: cap=remain:%dmAh / design:%dmAh / full:%dmAh\r\n",
                      battInfo[BATTERY_INFO_ID_REMAINING_CAP].regValue,
                      battInfo[BATTERY_INFO_ID_DESIGN_CAP].regValue,
                      battInfo[BATTERY_INFO_ID_FULL_CH_CAP].regValue);
        TP_PRINTF("Batt: status 0x%04x ==> init=%d, discharge=%d, full-charge=%d\r\n",
                      batt_status,
                      ((batt_status & RAJ240045_STATUS_BIT_INIT) ? 1 : 0),
                      ((batt_status & RAJ240045_STATUS_BIT_DISCHARGE) ? 1 : 0),
                      ((batt_status & RAJ240045_STATUS_BIT_FULL_CHARGE) ? 1 : 0));
        TP_PRINTF("Batt: safety=0x%04x; RSOC=%d%%|%d%%[L%d], ASOC=%d%%, SOH=%d%%[L%d]\r\n",
                      battInfo[BATTERY_INFO_ID_SAFETY].regValue,
                      battInfo[BATTERY_INFO_ID_RSOC].regValue,
                      (battInfo[BATTERY_INFO_ID_RSOC].regValue * HUNDRED_PERCENT) / BATT_ACTUAL_MAX_RSOC_PERCENT,
                      batteryLevel,
                      battInfo[BATTERY_INFO_ID_ASOC].regValue,
                      battInfo[BATTERY_INFO_ID_SOH].regValue,
                      battHealthLevel);
        TP_PRINTF("Batt temp: %d.%dC(L%d) = avg(%d.%dC,%d.%dC)\r\n\r\n",
                      tempBatt/10, tempBatt%10,    batteryTempLevel, 
                      tempBattCell1/10, tempBattCell1%10,
                      tempBattCell2/10, tempBattCell2%10);
    }
#endif
}


void PowerDrv_UpdateTempLevel(int16 tempNew, int16 *pTemp, eTempLevel *pLevel, const sRange *levels, uint32 numLevel)
{
    eTempLevel tempLevelNew;
    int16 i;
    
    //config check
    ASSERT( numLevel==TL_NUM );

    if( *pTemp < tempNew  ) //raise curve
    {
        //Check if new curve cross over "lower bound"
        for (i = 0; i < numLevel; i++)
        {
            if ( *pTemp<=levels[i].lower && levels[i].lower<=tempNew  )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i >= numLevel)
        {
            tempLevelNew= *pLevel;
        }
    }
    else //down curve (include equal)
    {
        //Check if new curve cross over "lower bound"
        for (i = numLevel-1; i >=0; i--)
        {
            if ( *pTemp>=levels[i].upper && levels[i].upper >= tempNew )
            {
                tempLevelNew= (eTempLevel)i;
                break;
            }
        }
        //Do not cross over, keep previous level
        if(i < 0)
        {
            tempLevelNew= *pLevel;
        }
    }
    
    //debug
    if(tempNew<=levels[TL_SUBNORMAL].upper)
    {
        ASSERT(TL_SUBNORMAL == tempLevelNew);
    }
    if(tempNew>=levels[TL_CRITICAL].lower)
    {
        ASSERT(TL_CRITICAL == tempLevelNew);
    }
    ASSERT(tempLevelNew<TL_NUM);
    
    //Store result
    *pTemp= tempNew;
    *pLevel= tempLevelNew;
}


//static void PowerDrv_CheckTempLevel(cPowerDrv *me, eTempLevel tempLevelNew, eTempLevel tempLevelPrev)
//{
//    if(tempLevelNew!=tempLevelPrev)
//    {
//        switch(tempLevelNew)
//        {
//        case TL_CRITICAL:
//            TP_PRINTF("\r\nERROR: PowerDrv_CheckTempLevel: temp level %d -> %d (CRITICAL) ***\r\n\r\n", tempLevelPrev, tempLevelNew);
//            PowerDrv_SetOverheat(me, TRUE);
//            ASSERT(0);  //should never trigger currently
//            break;
//        
//        case TL_SERIOUS:
//            TP_PRINTF("\r\nERROR: PowerDrv_CheckTempLevel: temp level %d -> %d (SERIOUS) ***\r\n\r\n", tempLevelPrev, tempLevelNew);
//            PowerDrv_SetOverheat(me, TRUE);
//            ASSERT(0);  //should never trigger currently
//            break;
//        
//        case TL_WARN:
//            TP_PRINTF("\r\nERROR: PowerDrv_CheckTempLevel: temp level %d -> %d (WARN) ***\r\n\r\n", tempLevelPrev, tempLevelNew);
//            PowerDrv_SetOverheat(me, TRUE);
//            break;
//       
//        case TL_NORMAL:
//            TP_PRINTF("\r\nERROR: PowerDrv_CheckTempLevel: temp level %d -> %d (NORMAL) ***\r\n\r\n", tempLevelPrev, tempLevelNew);            
//            PowerDrv_SetOverheat(me, FALSE);
//            break;
//             
//        default:
//            ASSERT(0);
//            break;
//        }
//    }
//      
//}
