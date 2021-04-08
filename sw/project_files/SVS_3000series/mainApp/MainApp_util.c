/**
*  @file      MainApp_util.c
*  @brief     util function for BnO mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "trace.h"
#include "controller.h"
#include "Setting_id.h"
#include "LedSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_bleEvtHandler.h"
#include "piu_common.h"
#include "MainApp.Config"
#include "GpioDrv.h"

/*****************************************************************
 * Definition
 *****************************************************************/
static cGpioDrv gpioObj;
Color ledBrightnessList[LED_MAX];

/*****************************************************************
 * Function Implemenation
 *****************************************************************/
void MainApp_SwitchMode(cMainApp* me, uint16 modeId)
{
    SwitchModeReqEvt* reqEvt = Q_NEW(SwitchModeReqEvt, SYSTEM_MODE_REQ_SIG);
    reqEvt->sender = (QActive*)me;
    reqEvt->modeId = modeId;
    SendToController((QEvt*)reqEvt);
}

#ifdef NDEBUG
/* IAR project optimize is set high, will ignore bellow const.
*  so set a low optimize before the function, so that this function only will be a low optimze
*/
#pragma optimize=low
#endif
void MainApp_ValidateData(cMainApp* me)
{
    uint8 i;
    int16 *pData = me->pMenuData;

    /* because bellow data will not be called, but we have to put into ROM */
    (void)(menuData);
    (void)(preset1Data);
    (void)(preset2Data);
    (void)(preset3Data);
    (void)(preset1Name);
    (void)(preset2Name);
    (void)(preset3Name);
    (void)(BOOTLOADER_MODE);

    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        if(pData[i] < menuDataAttr[i].minVal || pData[i] > menuDataAttr[i].maxVal)
        {
            pData[i] = menuDataAttr[i].defaultVal;
        }
    }
}

bool MainApp_ValidateBleData(QEvt const * const e)
{
    BleWriteDataReq *pBleData = (BleWriteDataReq *)e;
    uint8 settIndex = pBleData->offset / sizeof(uint16);
    uint8 i;
    int16 value;

    ASSERT(settIndex < NUM_OF_MENU_DATA);
    ASSERT(pBleData->size <= (SIZE_OF_LARGE_EVENTS - sizeof(eSettingId) - sizeof(uint16) - sizeof(uint16)));

    if(SETID_MENU_DATA == pBleData->setting_id)
    {
        for(i = 0; i < pBleData->size; i += sizeof(uint16))
        {
            memcpy(&value, pBleData->data, sizeof(uint16));
            if(value < menuDataAttr[settIndex].minVal || value > menuDataAttr[settIndex].maxVal)
            {
                return FALSE;
            }
            settIndex++;
        }
    }
    else
    {
        ASSERT(pBleData->size <= 8);
    }

    return TRUE;
}


void MainApp_InitBrightnessList(void)
{
    eLed i;
    for (i = LED_MIN; i < LED_MAX; i++)
    {
        ledBrightnessList[i] = LED_BRIGHTNESS_0;
    }
}

void MainApp_SendLedReq(cMainApp* me, ledMask mask, Color c)
{
    eLed i;
    ledMask m;
    ledMask actual_mask = 0x00;
    //TP_PRINTF("MainApp_SendLedReq: led=%d, c=5%d\r\n", led, c);

    /* this is a LED filter, if previous brightness is the same as current,
    *  then remove it
    */
    for (i = LED_MIN; i < LED_MAX; i++)
    {
        m = 1 << i;
        if (mask & m)
        {
            /* change it to higher brightness of Amber LED when to it enter to dim brightness
             */
            if(LED_MASK_LED_STBY_AMBER == m)
            {
                if(LED_BRIGHTNESS_25 == c)
                {
                    c = LED_BRIGHTNESS_50;
                }
            }

            /* store the value into the list */
            if(ledBrightnessList[i] != c)
            {
                actual_mask |= m;
                ledBrightnessList[i] = c;
            }
        }
    }

    LedSrv_SetEvtOn((QActive*)me, actual_mask, c);
}

void MainApp_UpdateLeds(cMainApp* me)
{
    ledMask on_mask, dim_mask, off_mask;
    uint8 steps;        // current step of "Volume" or "LPF" or "PHASE"
    uint8 num1, num2;   // calculate
    uint8 i;

    /* Clear mask */
    on_mask = dim_mask = off_mask = 0;

    /* 1. "PLUS" and "MINUS" LED*/
    on_mask |= LED_MASK_LED_PLUS_MINUS;

    /* 2. "Auto/On" LED */
    if(STANDBY_MODE_ON == me->standbyMode)
    {
        on_mask     |= LED_MASK_LED_STBY_BLUE;
        off_mask    |= LED_MASK_LED_STBY_AMBER;
    }
    else
    {
        on_mask     |= LED_MASK_LED_STBY_AMBER;
        off_mask    |= LED_MASK_LED_STBY_BLUE;
    }

    /* 3. "VOL", "LPF", "PHASE" LED*/
    if(PAGE_SETTING_VOL == me->pageSetting)
    {
        on_mask     |= LED_MASK_LED_VOL;

        MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
        MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
    }
    else if(PAGE_SETTING_LP_FRE == me->pageSetting)
    {
        MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, TRUE);
        MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
        off_mask    |= LED_MASK_LED_VOL;
    }
    else if(PAGE_SETTING_PHASE == me->pageSetting)
    {
        MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, TRUE);
        MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
        off_mask    |= LED_MASK_LED_VOL;
    }
    else
    {
        ASSERT(0);
    }    

    /* 4. LED Bar */
    steps = MainApp_CalcStep(me);
    num1 = steps / LED_BRIGHTNESS_TOTAL_STEPS;
    num2 = steps % LED_BRIGHTNESS_TOTAL_STEPS;


    /* handle the special case */
    if(PAGE_SETTING_LP_FRE == me->pageSetting)
    {
        uint8 lpSettIndex = MainApp_GetSettIndex(PAGE_SETTING_LP_STATUS);
        int16 lpStatus = me->pMenuData[lpSettIndex];

        if(LP_STATUS_OFF == lpStatus)
        {
            num1 = LED_BAR_MAX + 1; // because LED_BAR_MAX is 10, but led-bar has 11 leds
            num2 = 0;
        }
    }

    /* full on led mask */
    for(i = 0; i < num1; i++)
    {
        on_mask |= (1 << (LED_BAR_MIN + i));
    }

    /* dim led mask */
    if(num2 <= LED_BRIGHTNESS_TOTAL_STEPS)
    {
        dim_mask |= (1 << (LED_BAR_MIN + num1));
    }

    /* off led mask */
    for(i += 1; i <= LED_BAR_MAX; i++)
    {
        off_mask |= (1 << (LED_BAR_MIN + i));
    }

    /* send led request */
    MainApp_SendLedReq(me, off_mask, LED_BRIGHTNESS_0);

    if(num2 <= LED_BRIGHTNESS_TOTAL_STEPS)
    {
        MainApp_SendLedReq(me, dim_mask, ledBrightnessArray[num2]);
    }

    MainApp_SendLedReq(me, on_mask,  LED_BRIGHTNESS_100);
}

void MainApp_CleanLeds(cMainApp* me)
{
    MainApp_SendLedReq(me, LED_MASK_ALL_LEDS, LED_BRIGHTNESS_0);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
}

void MainApp_DimLeds(cMainApp* me)
{
    ledMask dim_mask = 0x00;
    ledMask off_mask = 0x00;

    MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);

    if(STANDBY_MODE_ON == me->standbyMode)
    {
        dim_mask |= LED_MASK_LED_STBY_BLUE;
        off_mask |= LED_MASK_LED_STBY_AMBER;
    }
    else
    {
        dim_mask |= LED_MASK_LED_STBY_AMBER;
        off_mask |= LED_MASK_LED_STBY_BLUE;
    }

    // turn off led-bar
    off_mask |= LED_MASK_LED_BAR;

    dim_mask |= (LED_MASK_LED_VOL | LED_MASK_LED_PLUS_MINUS);

    /* turn off the led first */
    MainApp_SendLedReq(me, off_mask, LED_BRIGHTNESS_0);
    MainApp_SendLedReq(me, dim_mask, LED_BRIGHTNESS_25);
}

void MainApp_UpgradingLeds(cMainApp* me)
{
    ledMask on_mask = 0x00;
    ledMask off_mask = 0x00;

    off_mask  = LED_MASK_LED_PLUS_MINUS | LED_MASK_LED_STBY | LED_MASK_LED_VOL;
    off_mask |= LED_MASK_LED_BAR_4 | LED_MASK_LED_BAR_5 | LED_MASK_LED_BAR_6 | LED_MASK_LED_BAR_7 | LED_MASK_LED_BAR_8;

    on_mask = LED_MASK_LED_UPGRADING;

    MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
    /* turn off the led first */
    MainApp_SendLedReq(me, off_mask, LED_BRIGHTNESS_0);
    MainApp_SendLedReq(me, on_mask,  LED_BRIGHTNESS_100);
}

void MainApp_StandbyLeds(cMainApp* me)
{
    ledMask dim_mask = 0x00;
    ledMask off_mask = 0x00;

    // turn off
    off_mask |= (LED_MASK_LED_STBY_BLUE | LED_MASK_LED_BAR | LED_MASK_LED_VOL);

    dim_mask |= (LED_MASK_LED_STBY_AMBER | LED_MASK_LED_PLUS_MINUS);

    /* turn off the led first */
    MainApp_SendLedReq(me, off_mask, LED_BRIGHTNESS_0);
    MainApp_SendLedReq(me, dim_mask, LED_BRIGHTNESS_25);

    MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, FALSE);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, FALSE);
}

void MainApp_PoweringUpLeds(cMainApp* me)
{
    ledMask on_mask = 0x00;
    ledMask off_mask = 0x00;

    if(STANDBY_MODE_ON == me->standbyMode)
    {
        on_mask  |= LED_MASK_LED_STBY_BLUE;
        off_mask |= LED_MASK_LED_STBY_AMBER;
    }
    else
    {
        on_mask  |= LED_MASK_LED_STBY_AMBER;
        off_mask |= LED_MASK_LED_STBY_BLUE;
    }

    on_mask |= LED_MASK_LED_BAR | LED_MASK_LED_PLUS_MINUS | LED_MASK_LED_VOL;
    
    MainApp_SendLedReq(me, off_mask, LED_BRIGHTNESS_0);
    MainApp_SendLedReq(me, on_mask,  LED_BRIGHTNESS_100);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_LPF, TRUE);
    MainApp_SetGpioLeds(me, GPIO_OUT_LED_PHASE, TRUE);
}

void MainApp_CalcAndSet(cMainApp * const me, bool plus_minus)
{
    uint8 settIndex;
    uint8 num;
    int16 *pTable;
    uint8 step;
    eAudioSettId dspSettId;

    settIndex = MainApp_GetSettIndex(me->pageSetting);
    dspSettId = MainApp_GetDspSettId(me, me->pageSetting);

    if(PAGE_SETTING_VOL == me->pageSetting)
    {
        pTable = (int16 *)volStepTable;
        num = ArraySize(volStepTable);
    }
    else if(PAGE_SETTING_LP_FRE == me->pageSetting)
    {
        pTable = (int16 *)lpFreqStepTable;
        num = ArraySize(lpFreqStepTable);

        /* special case*/
        int16 lpStatus = MainApp_GetMenuData(me, PAGE_SETTING_LP_STATUS);
        if(LP_STATUS_OFF == lpStatus)
        {
            if(plus_minus)
            {
                /* still off, do nothing */
                return;
            }
            else
            {
                /* release the LP status */
                MainApp_SetMenuData(me, PAGE_SETTING_LP_STATUS, LP_STATUS_ON);
                AudioSrv_SetEq(dspSettId, TRUE);
                MainApp_BleReadDataResp(&(me->pMenuData[settIndex - 1]), SETID_MENU_DATA, sizeof(uint16), (settIndex -1) * sizeof(uint16)); // status
                MainApp_BleReadDataResp(&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16)); // frequency
                return;
            }
        }
        else
        {
            /*LP status is ON, do bellow*/
        }
    }
    else if(PAGE_SETTING_PHASE == me->pageSetting)
    {
        pTable = (int16 *)phaseStepTable;
        num = ArraySize(phaseStepTable);
    }
    else
    {
        ASSERT(0);
    }

    step = MainApp_CalcStep(me);
    num = num - 1;

    if(plus_minus)
    {
        if(step < num)
        {
            me->pMenuData[settIndex] = pTable[step + 1];
        }
    }
    else
    {
        if(step > 0)
        {
            me->pMenuData[settIndex] = pTable[step - 1];
        }
    }

    /* update dsp and app display */
    {
        AudioSrv_SetEq(dspSettId, TRUE);
        MainApp_BleReadDataResp(&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
    }
}

uint8 MainApp_CalcStep(cMainApp * const me)
{
    uint8 settIndex, findIndex;
    uint8 num;
    int16 *pTable;
    int16 value;

    if(PAGE_SETTING_VOL == me->pageSetting)
    {
        pTable = (int16 *)volStepTable;
        num = ArraySize(volStepTable);
    }
    else if(PAGE_SETTING_LP_FRE == me->pageSetting)
    {
        pTable = (int16 *)lpFreqStepTable;
        num = ArraySize(lpFreqStepTable);
    }
    else if(PAGE_SETTING_PHASE == me->pageSetting)
    {
        pTable = (int16 *)phaseStepTable;
        num = ArraySize(phaseStepTable);
    }
    else
    {
        ASSERT(0);
    }

    settIndex = MainApp_GetSettIndex(me->pageSetting);
    value = me->pMenuData[settIndex];

    for(findIndex = 0; findIndex < num; findIndex++)
    {
        if(pTable[findIndex] >= value)
        {
            break;
        }
    }

    return findIndex;
}

eAudioSettId MainApp_GetAudioSettId(ePageSettingId pageId)
{
    uint8 i;

    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        if(pageId == menuDataAttr[i].settingId)
        {
            break;
        }
    }

    ASSERT(i != NUM_OF_MENU_DATA);
    return menuDataAttr[i].dspSettId;
}

void MainApp_ResetSettings(cMainApp * const me)
{
    uint8 i;

    /* 1. Reset menu data */
    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        me->pMenuData[i] = menuDataAttr[i].defaultVal;
    }
    Setting_Set(SETID_MENU_DATA, me->pMenuData);

    /* 2. Reset preset data */
    Setting_Set(SETID_PRESET_1, (void*)&preset1OriginalData);
    Setting_Set(SETID_PRESET_2, (void*)&preset2OriginalData);
    Setting_Set(SETID_PRESET_3, (void*)&preset3OriginalData);

    /* 3. Reset preset name */
    Setting_Set(SETID_PRESET_1_NAME, PRESET_1_ORIGIN_NAME);
    Setting_Set(SETID_PRESET_2_NAME, PRESET_2_ORIGIN_NAME);
    Setting_Set(SETID_PRESET_3_NAME, PRESET_3_ORIGIN_NAME);

    bool screenStatus = TRUE;
    Setting_Set(SETID_SCREEN_STATUS, &screenStatus);
}

bool MainApp_ResetPage(cMainApp * const me, ePageSettingId pageSettingId)
{
    uint8 i = 0;

    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        if(pageSettingId == menuDataAttr[i].settingId && me->pMenuData[i] != menuDataAttr[i].defaultVal)
        {
            me->pMenuData[i] = menuDataAttr[i].defaultVal;
            return TRUE;
        }
    }

    return FALSE;
}

uint8 MainApp_GetSettIndex(ePageSettingId pageSettId)
{
    uint8 i;

    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        if(pageSettId == menuDataAttr[i].settingId)
        {
           break;
        }
    }

    ASSERT(i != NUM_OF_MENU_DATA);
    return i;
}

void MainApp_SetMenuData(cMainApp * const me, ePageSettingId pageSettId, int16 value)
{
    uint8 index;

    index = MainApp_GetSettIndex(pageSettId);

    me->pMenuData[index]  = value;
}

int16 MainApp_GetMenuData(cMainApp * const me, ePageSettingId pageSettId)
{
    uint8 index;

    index = MainApp_GetSettIndex(pageSettId);

    return me->pMenuData[index];
}

int16 MainApp_GetMenuDefault(uint8 index)
{
    ASSERT(index < NUM_OF_MENU_DATA);
    return menuDataAttr[index].defaultVal;
}

ePageSettingId MainApp_GetSettPage(uint8 settIndex)
{
    ASSERT(settIndex < NUM_OF_MENU_DATA);
    return menuDataAttr[settIndex].settingId;
}

eAudioSettId MainApp_GetDspSettId(cMainApp * const me, ePageSettingId pageSettingId)
{
    uint8 i = 0;
    eAudioSettId dspSettId = DSP_SETT_ID_MAX;

    for(i = 0; i < NUM_OF_MENU_DATA; i++)
    {
        if(menuDataAttr[i].settingId == pageSettingId)
        {
            dspSettId = menuDataAttr[i].dspSettId;
            break;
        }
    }

    return dspSettId;
}

void MainApp_LoadPreset(cMainApp * const me)
{
    uint8_t i, j;
    bool enabled;
    ePageSettingId mandatoryArr[] ={
    PAGE_SETTING_VOL, PAGE_SETTING_PHASE, PAGE_SETTING_POLARITY,
    PAGE_SETTING_LP_STATUS, PAGE_SETTING_PEQ1_STATUS, PAGE_SETTING_PEQ2_STATUS,
    PAGE_SETTING_PEQ3_STATUS, PAGE_SETTING_RGC_STATUS};

    for(i = 0; i < ArraySize(mandatoryArr); i++)
    {
        for(j = 0; j < NUM_OF_MENU_DATA; j++)
        {
            if(mandatoryArr[i] == menuDataAttr[j].settingId)
            {
                enabled = me->pMenuData[j] ? TRUE : FALSE;
                AudioSrv_SetEq(menuDataAttr[j].dspSettId, enabled);
                break;
            }
        }
    }
}

uint8* MainApp_GetFeatures(uint8 *size)
{
    *size = ArraySize(projFeatureSet);
    return (uint8*)projFeatureSet;
}

void MainApp_InitGpioLeds(void)
{
    tGPIODevice *pConfig = (tGPIODevice*)getDevicebyIdAndType(LED_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(pConfig);
    GpioDrv_Ctor(&gpioObj, pConfig);
}

// this is special leds using GPIO to control
void MainApp_SetGpioLeds(cMainApp * const me, eGPIOId gpioId, bool on)
{
    if(on)
    {
        GpioDrv_SetBit(&gpioObj, gpioId);
    }
    else
    {
        GpioDrv_ClearBit(&gpioObj, gpioId);
    }
}

int32 MainApp_GetIdleTimeout(cMainApp * const me)
{
    uint8 settIndex;
    int32 ms = 0;

    settIndex = MainApp_GetSettIndex(PAGE_SETTING_TIMEOUT);

    if(me->pMenuData[settIndex])
    {
        ms = 100 * me->pMenuData[settIndex];
    }
    else
    {
        ms = INVALID_VALUE;
    }

    return ms;
}

int32 MainApp_GetStandbyTimeout(cMainApp * const me)
{
    int32 ms = 0;

    if(STANDBY_MODE_ON == me->standbyMode)
    {
        ms = INVALID_VALUE;
    }
    else if(STANDBY_MODE_AUTO == me->standbyMode)
    {
        ms = TIMER_ID_ENTER_STANDBY_TIMEOUT_IN_MS;
    }
    else if(STANDBY_MODE_TRIGGER == me->standbyMode)
    {
        ms = INVALID_VALUE;
    }
    else
    {
        ASSERT(0);
    }

    return ms;
}

int32 MainApp_GetJackLowTimeout(cMainApp * const me)
{
    int32 ms = 0;

    if(JACK_LEVEL_LOW == me->jackLevelStatus)
    {
        ms = TIMER_ID_JACK_LOW_TIMEOUT_IN_MS;
    }
    else
    {
        ms = INVALID_VALUE;
    }

    return ms;
}

QState MainApp_JackHandler(cMainApp * const me, eJackDetType type, int32 param)
{
    QState trans_state = Q_HANDLED();
    uint8 settIndex;
    
    if (JACK_DET_IN_OUT == type)
    {
        bool isJackIn = param ? TRUE : FALSE;
        me->jackLevelStatus = JACK_LEVEL_NONE;
        TP_PRINTF("Jack %s \r\n", isJackIn ? "In" : "Out");
        if (isJackIn)
        {
            /* store stanby mode */
            me->prevStandbyMode = me->standbyMode;
            me->standbyMode = STANDBY_MODE_TRIGGER;
        }
        else
        {
            /* restore the standby mode */
            me->standbyMode = me->prevStandbyMode;
        }

        settIndex = MainApp_GetSettIndex(PAGE_SETTING_STANDBY);

        MainApp_BleReadDataResp(&(me->standbyMode), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
    }
    else if (JACK_DET_LEVEL == type)
    {
        bool isJackHigh = param ? TRUE : FALSE;
        if(JACK_LEVEL_HIGH == isJackHigh)
        {
            TP_PRINTF("Jack in High\r\n");
            me->jackLevelStatus = JACK_LEVEL_HIGH;
            if(SYSTEM_STA_STANDBY == me->systemStatus)
            {
                trans_state = Q_TRAN(&MainApp_Active);
            }
        }
        else
        {
            TP_PRINTF("Jack in Low\r\n");
            me->jackLevelStatus = JACK_LEVEL_LOW;
        }
    }
    else
    {
        ASSERT(0);
    }

    return trans_state;
}

QState MainApp_PowerHandler(cMainApp * const me, eDcInSta status)
{
    QState ret = Q_HANDLED();

    if(me->dcSense != status)
    {
        if (DC_IN_STA_ON == status)
        {
            TP_PRINTF("Power switch ON\r\n");
            /* Previous is OFF stage, now switch to ON stage
            *  soft reset to clear the unstable status
            *
            */
            //BSP_SoftReboot();  do it at MainApp_DeActive
        }
        else if (DC_IN_STA_OFF ==  status)
        {
            TP_PRINTF("Power switch OFF\r\n");
            /* Previous is ON stage, now switch to OFF stage
            *  save the settings
            *  Close the amplifier
            *  Enter DeActive
            *  if this duration is very short, this step will be unstable to save
            */
            //SettingSrv_BookkeepingEx(me);
            ret = Q_TRAN(&MainApp_DeActive);
        }
        else
        {
            ASSERT(0);
        }

        me->dcSense = status;
    }

    return ret;
}

void MainApp_SetVolume(cMainApp * const me, int16 value)
{
    uint8 settIndex;
    eAudioSettId dspSettId;
    int16 volume = (-VALUE_MAGNIFICATION) * value;

    me->pageSetting = PAGE_SETTING_VOL;
    settIndex = MainApp_GetSettIndex(PAGE_SETTING_VOL);
    dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_VOL);

    if(volume >= menuDataAttr[settIndex].minVal && volume <= menuDataAttr[settIndex].maxVal)
    {
        me->pMenuData[settIndex] = volume;
        AudioSrv_SetEq(dspSettId, TRUE);
        MainApp_BleReadDataResp(&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
    }
}

void MainApp_GetVolume(cMainApp * const me)
{
    int16 value;
    MainAppRespEvt *pDataResponse;

    value = MainApp_GetMenuData(me, PAGE_SETTING_VOL);
    value = value / (-VALUE_MAGNIFICATION);

    pDataResponse = Q_NEW(MainAppRespEvt, MAINAPP_GET_VOL_RESP_SIG);
    pDataResponse->evtReturn = RET_SUCCESS;
    pDataResponse->value = value;
    SendToServer(USB_SRV_ID, (QEvt*)pDataResponse);
}

