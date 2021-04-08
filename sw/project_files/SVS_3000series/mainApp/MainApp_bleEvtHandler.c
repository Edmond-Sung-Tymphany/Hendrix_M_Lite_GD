/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of mainApp
*  @author    Daniel Qin
*  @date      15-July-2015
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "tym_qp_lib.h"

#include "AudioSrv.h"
#include "SettingSrv.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"
#include "MainApp_bleEvtHandler.h"


/*****************************************************************
 * Global Variable
 *****************************************************************/



/*****************************************************************
 * Function Implemenation
 *****************************************************************/
void MainApp_BleReadDataResp(void * pDataToSend, eSettingId settId, uint16 size, uint16 offset)
{
    //ASSERT((size % 2) == 0);
    if(size <= SMALL_DATA_SIZE)
    {
        BleReadDataSmallResp * pDataResponse =  Q_NEW(BleReadDataSmallResp, BLE_READ_DATA_RESP_SIG);
        pDataResponse->setting_id = settId;
        pDataResponse->size = size;
        pDataResponse->offset = offset;
        memcpy((void*)(&(pDataResponse->data[0])),(void*)pDataToSend,size);
        SendToServer(BLE_CTRL_SRV_ID, (QEvt*)pDataResponse);
        //SendToServer(USB_SRV_ID, (QEvt*)pDataResponse);
    }
    else if(size <= MEDIUM_DATA_SIZE)
    {
        BleReadDataMediumResp * pDataResponse =  Q_NEW(BleReadDataMediumResp, BLE_READ_DATA_RESP_SIG);
        pDataResponse->setting_id = settId;
        pDataResponse->size = size;
        pDataResponse->offset = offset;
        memcpy((void*)(&(pDataResponse->data[0])),(void*)pDataToSend,size);
        SendToServer(BLE_CTRL_SRV_ID,(QEvt*)pDataResponse);
        //SendToServer(USB_SRV_ID, (QEvt*)pDataResponse);
    }
    else
    {
        BleReadDataLargeResp * pDataResponse =  Q_NEW(BleReadDataLargeResp, BLE_READ_DATA_RESP_SIG);
        pDataResponse->setting_id = settId;
        pDataResponse->size = size;
        pDataResponse->offset = offset;
        memcpy((void*)(&(pDataResponse->data[0])),(void*)pDataToSend,size);
        SendToServer(BLE_CTRL_SRV_ID,(QEvt*)pDataResponse);
        //SendToServer(USB_SRV_ID, (QEvt*)pDataResponse);
    }
}

static void MainApp_BleSyncDsp(cMainApp * const me, uint8 settIndex)
{
    eAudioSettId dspSettId;
    ePageSettingId settingId = MainApp_GetSettPage(settIndex);

    switch(settingId)
    {
        case PAGE_SETTING_PRE1_LOAD:
        case PAGE_SETTING_PRE2_LOAD:
        case PAGE_SETTING_PRE3_LOAD:
        case PAGE_SETTING_PRE4_LOAD:
        case PAGE_SETTING_PRE1_SAVE:
        case PAGE_SETTING_PRE2_SAVE:
        case PAGE_SETTING_PRE3_SAVE:
        case PAGE_SETTING_PRE4_SAVE:
        {
            TP_PRINTF("preset base=%d,target=%d\r\n", PAGE_SETTING_PRE1_LOAD, settingId);
            break;
        }
        case PAGE_SETTING_LP_STATUS:
        case PAGE_SETTING_LP_FRE:
        case PAGE_SETTING_LP_SLO:
        {
            bool enabled = MainApp_GetMenuData(me, PAGE_SETTING_LP_STATUS)? TRUE : FALSE;
            me->pageSetting = PAGE_SETTING_LP_FRE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, enabled);
            break;
        }
        case PAGE_SETTING_PHASE:
        {
            me->pageSetting = PAGE_SETTING_PHASE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, TRUE);
            break;
        }
        case PAGE_SETTING_VOL:
        {
            me->pageSetting = PAGE_SETTING_VOL;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, TRUE);
            break;
        }
        case PAGE_SETTING_PEQ1_STATUS:
        case PAGE_SETTING_PEQ1_FRE:
        case PAGE_SETTING_PEQ1_BOOST:
        case PAGE_SETTING_PEQ1_Q:
        {
            bool enabled = MainApp_GetMenuData(me, PAGE_SETTING_PEQ1_STATUS)? TRUE : FALSE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, enabled);
            break;
        }
        case PAGE_SETTING_PEQ2_STATUS:
        case PAGE_SETTING_PEQ2_FRE:
        case PAGE_SETTING_PEQ2_BOOST:
        case PAGE_SETTING_PEQ2_Q:
        {
            bool enabled = MainApp_GetMenuData(me, PAGE_SETTING_PEQ2_STATUS)? TRUE : FALSE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, enabled);
            break;
        }
        case PAGE_SETTING_PEQ3_STATUS:
        case PAGE_SETTING_PEQ3_FRE:
        case PAGE_SETTING_PEQ3_BOOST:
        case PAGE_SETTING_PEQ3_Q:
        {
            bool enabled = MainApp_GetMenuData(me, PAGE_SETTING_PEQ3_STATUS)? TRUE : FALSE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, enabled);
            break;
        }
        case PAGE_SETTING_RGC_STATUS:
        case PAGE_SETTING_RGC_FREQ:
        case PAGE_SETTING_RGC_SLOPE:
        {
            bool enabled = MainApp_GetMenuData(me, PAGE_SETTING_RGC_STATUS)? TRUE : FALSE;
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, enabled);
            break;
        }
        case PAGE_SETTING_POLARITY:
        {
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, TRUE);
            break;
        }
        case PAGE_SETTING_TUNNING:
        {
            dspSettId = MainApp_GetDspSettId(me, settingId);
            AudioSrv_SetEq(dspSettId, TRUE);
            break;
        }
        case PAGE_SETTING_STANDBY:
        {
            if(STANDBY_MODE_TRIGGER == me->standbyMode)
            {
                /* during Trigger standby mode, the trigger has the highest priotity.
                *  can't overwrite the trigger standby mode
                */
                if(STANDBY_MODE_TRIGGER != me->pMenuData[settIndex])
                {
                    me->pMenuData[settIndex] = me->prevStandbyMode;
                    MainApp_BleReadDataResp(&(me->standbyMode), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                }
            }
            else
            {
                /* click to trigger mode, return to previous mode*/
                if(STANDBY_MODE_TRIGGER == me->pMenuData[settIndex])
                {
                    me->pMenuData[settIndex] = me->standbyMode;
                    MainApp_BleReadDataResp(&(me->standbyMode), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                }
                else
                {
                    me->standbyMode = me->pMenuData[settIndex];
                    me->prevStandbyMode = me->standbyMode;
                }
            }

            break;
        }
        default:
        {
            break;
        }
    }

}

static QState MainApp_BleWriteData(cMainApp * const me, QEvt const * const e)
{
    BleWriteDataReq *pBleData = (BleWriteDataReq *)e;
    uint8 settIndex;

    //TP_PRINTF("write sett=%d\r\n", pBleData->setting_id);
    if(MainApp_ValidateBleData(e))
    {
        if(SETID_MENU_DATA == pBleData->setting_id)
        {
            settIndex = (pBleData->offset) / sizeof(uint16);
            memcpy(&(me->pMenuData[settIndex]), pBleData->data, pBleData->size);
            MainApp_BleSyncDsp(me, settIndex);
        }
        else
        {
            Setting_Set(pBleData->setting_id, pBleData->data);
        }
    }
    else
    {
        ASSERT(0);
        return Q_UNHANDLED();
    }

    return Q_HANDLED();
}

static QState MainApp_BleReadData(cMainApp * const me, QEvt const * const e)
{
    BleReadDataReq *pBleData = (BleReadDataReq *)e;
    uint16 *pSrc = (uint16*)Setting_Get(pBleData->setting_id);
    uint8 settIndex;

    TP_PRINTF("read sett=%d, %d, %d\r\n", pBleData->setting_id, pBleData->size, pBleData->offset);
    TP_PRINTF("read standby=%d,%d,%d,%d\r\n", pSrc[2], me->pMenuData[2], me->standbyMode, me->prevStandbyMode);

    if(pBleData->setting_id == SETID_MENU_DATA)
    {
        MainApp_BleReadDataResp(&pSrc[(pBleData->offset/2)], pBleData->setting_id, pBleData->size, pBleData->offset);

        /* some settings need to reflesh again */
        settIndex = MainApp_GetSettIndex(PAGE_SETTING_STANDBY);
        MainApp_BleReadDataResp(&(me->standbyMode), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
    }else
    {
        MainApp_BleReadDataResp(&pSrc[(pBleData->offset)], pBleData->setting_id, pBleData->size, pBleData->offset);
    }

    return Q_HANDLED();
}

static QState MainApp_BleResetItem(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_HANDLED();
    BleResetItemReq *pBleData = (BleResetItemReq*)e;
    bool isDataReseted = FALSE;
    uint8 settIndex;
    bool isEqOn = FALSE;
    eAudioSettId dspSettId;

    switch(pBleData->featureId)
    {
        case FEATURE_ID_DISPLAY:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_DISPLAY);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_DISPLAY);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
            }
            break;
        }
        case FEATURE_ID_SYS_TIMEOUT:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_TIMEOUT);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_TIMEOUT);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
            }
            break;
        }
        case FEATURE_ID_STANDBY:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_STANDBY);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_STANDBY);

                /*if Trigger Cable is connected, do not the standbymode, otherwise the led will be set wrong */
                if(STANDBY_MODE_TRIGGER == me->standbyMode)
                {
                    //MainApp_SetMenuData(me, PAGE_SETTING_STANDBY, me->standbyMode);
                }
                else
                {
                    // update the variable
                    me->standbyMode = me->prevStandbyMode = me->pMenuData[settIndex];
                }

                MainApp_BleReadDataResp((void*)&(me->standbyMode), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
            }
            break;
        }
        case FEATURE_ID_LP:
        {
            isDataReseted  = MainApp_ResetPage(me, PAGE_SETTING_LP_STATUS);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_LP_FRE);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_LP_SLO);

            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_LP_STATUS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, 3 * sizeof(uint16), settIndex * sizeof(uint16));
                isEqOn = me->pMenuData[settIndex] ?  TRUE : FALSE;
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_LP_STATUS);
                AudioSrv_SetEq(dspSettId, isEqOn);
            }
            break;
        }
        case FEATURE_ID_PEQ1:
        {
            isDataReseted  = MainApp_ResetPage(me, PAGE_SETTING_PEQ1_STATUS);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ1_FRE);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ1_BOOST);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ1_Q);

            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_PEQ1_STATUS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, 4 * sizeof(uint16), settIndex * sizeof(uint16));
                isEqOn = me->pMenuData[settIndex] ?  TRUE : FALSE;
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_PEQ1_STATUS);
                AudioSrv_SetEq(dspSettId, isEqOn);
            }
            break;
        }
        case FEATURE_ID_PEQ2:
        {
            isDataReseted  = MainApp_ResetPage(me, PAGE_SETTING_PEQ2_STATUS);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ2_FRE);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ2_BOOST);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ2_Q);

            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_PEQ2_STATUS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, 4 * sizeof(uint16), settIndex * sizeof(uint16));
                isEqOn = me->pMenuData[settIndex] ?  TRUE : FALSE;
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_PEQ2_STATUS);
                AudioSrv_SetEq(dspSettId, isEqOn);
            }
            break;
        }
        case FEATURE_ID_PEQ3:
        {
            isDataReseted  = MainApp_ResetPage(me, PAGE_SETTING_PEQ3_STATUS);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ3_FRE);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ3_BOOST);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_PEQ3_Q);

            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_PEQ3_STATUS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, 4 * sizeof(uint16), settIndex * sizeof(uint16));
                isEqOn = me->pMenuData[settIndex] ?  TRUE : FALSE;
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_PEQ3_STATUS);
                AudioSrv_SetEq(dspSettId, isEqOn);
            }
            break;
        }
        case FEATURE_ID_RGC:
        {
            isDataReseted  = MainApp_ResetPage(me, PAGE_SETTING_RGC_STATUS);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_RGC_FREQ);
            isDataReseted |= MainApp_ResetPage(me, PAGE_SETTING_RGC_SLOPE);

            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_RGC_STATUS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, 3 * sizeof(uint16), settIndex * sizeof(uint16));
                isEqOn = me->pMenuData[settIndex] ?  TRUE : FALSE;
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_RGC_STATUS);
                AudioSrv_SetEq(dspSettId, isEqOn);
            }
            break;
        }
        case FEATURE_ID_PHASE:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_PHASE);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_PHASE);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_PHASE);
                AudioSrv_SetEq(dspSettId, TRUE);
            }
            break;
        }
        case FEATURE_ID_POLARITY:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_POLARITY);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_POLARITY);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_POLARITY);
                AudioSrv_SetEq(dspSettId, TRUE);
            }
            break;
        }
        case FEATURE_ID_TUNNING:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_TUNNING);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_TUNNING);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_TUNNING);
                AudioSrv_SetEq(dspSettId, TRUE);
            }
            break;

        }
        case FEATURE_ID_VOLUME:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_VOL);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_VOL);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
                dspSettId = MainApp_GetDspSettId(me, PAGE_SETTING_VOL);
                AudioSrv_SetEq(dspSettId, TRUE);
            }
            break;

        }
        case FEATURE_ID_PRESET_NAME:
        {
            MainApp_BleResetPresetName();
            break;
        }
        case FEATURE_ID_BRIGHTNESS:
        {
            isDataReseted = MainApp_ResetPage(me, PAGE_SETTING_BRIGHTNESS);
            if(isDataReseted)
            {
                settIndex = MainApp_GetSettIndex(PAGE_SETTING_BRIGHTNESS);
                MainApp_BleReadDataResp((void*)&(me->pMenuData[settIndex]), SETID_MENU_DATA, sizeof(uint16), settIndex * sizeof(uint16));
            }
            break;

        }
        default:
        {
            ret = Q_HANDLED();
            break;
        }
    }

    return ret;
}

static QState MainApp_BleFeatureReq(cMainApp * const me, QEvt const * const e)
{
    BleFeatureResp * pFeatureResp = Q_NEW(BleFeatureResp, BLE_MENU_FEATURE_RESP_SIG);
    uint8 *pFeatures = NULL;
    uint8 size;

    pFeatures = MainApp_GetFeatures(&size);
    ASSERT(pFeatures);

    memcpy(pFeatureResp->data, pFeatures, size);
    pFeatureResp->numberOfFeature = size;
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)pFeatureResp);

    return Q_HANDLED();
}

static void MainApp_BleResetPresetName(void)
{
    uint8 *pPreset;

    Setting_Set(SETID_PRESET_1_NAME, PRESET_1_ORIGIN_NAME);
    pPreset = (uint8*)Setting_Get(SETID_PRESET_1_NAME);
    MainApp_BleReadDataResp((void*)pPreset, SETID_PRESET_1_NAME, sizeof(PRESET_1_ORIGIN_NAME), 0);

    Setting_Set(SETID_PRESET_2_NAME, PRESET_2_ORIGIN_NAME);
    pPreset = (uint8*)Setting_Get(SETID_PRESET_2_NAME);
    MainApp_BleReadDataResp((void*)pPreset, SETID_PRESET_2_NAME, sizeof(PRESET_2_ORIGIN_NAME), 0);

    Setting_Set(SETID_PRESET_3_NAME, PRESET_3_ORIGIN_NAME);
    pPreset = (uint8*)Setting_Get(SETID_PRESET_3_NAME);
    MainApp_BleReadDataResp((void*)pPreset, SETID_PRESET_3_NAME, sizeof(PRESET_3_ORIGIN_NAME), 0);
}

static QState MainApp_BleDfuReq(cMainApp * const me, QEvt const * const e)
{
    BleDfuResp *pResp = Q_NEW(BleDfuResp, MAINAPP_DFU_RESP_SIG);
    ASSERT(pResp);
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)pResp);

    me->upgradeMethod = UPGRADE_BY_APP;
    return Q_TRAN(&MainApp_EnterUpgrading);
}

static QState MainApp_BleVersionReq(cMainApp * const me, QEvt const * const e)
{
    char *pData;
    uint8_t length;
    BleVersionResp *pResp = Q_NEW(BleVersionResp, MAINAPP_VERSION_RESP_SIG);
    ASSERT(pResp);

    // copy the version and name of this product
    length = sizeof(pResp->mcuVer);
    memset((uint8*)&pResp->mcuVer[0], 0, length);

    pData = MCU_VERSION;
    pResp->lenOfVer = MIN(length, strlen(pData));
    memcpy((uint8*)&pResp->mcuVer[0], pData, pResp->lenOfVer);
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)pResp);

    return Q_HANDLED();
}

static QState MainApp_BleProductNameReq(cMainApp * const me, QEvt const * const e)
{
    char *pData;
    uint8_t length;
    BleProductNameResp *pResp = Q_NEW(BleProductNameResp, MAINAPP_PRODUCT_NAME_RESP_SIG);
    ASSERT(pResp);

    // copy the version and name of this product
    length = sizeof(pResp->productName);
    memset((uint8*)&pResp->productName[0], 0, length);

    pData = TP_PRODUCT;
    pResp->lenOfName = MIN(length, strlen(pData));
    memcpy((uint8*)&pResp->productName[0], pData, pResp->lenOfName);
    SendToServer(BLE_CTRL_SRV_ID, (QEvt*)pResp);

    return Q_HANDLED();
}

QState MainApp_BleEvtHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    TP_PRINTF("MainApp_BleEvtHandler sig=%d\r\n", e->sig);
    switch(e->sig)
    {
        case BLE_WRITE_DATA_REQ_SIG:
            ret = MainApp_BleWriteData(me, e);
            break;
        case BLE_READ_DATA_REQ_SIG:
            ret = MainApp_BleReadData(me, e);
            break;
        case BLE_RESET_ITEM_REQ_SIG:
            ret = MainApp_BleResetItem(me, e);
            break;
        case BLE_MENU_FEATURE_REQ_SIG:
            ret= MainApp_BleFeatureReq(me, e);
            break;
        case MAINAPP_DFU_REQ_SIG:
            ret = MainApp_BleDfuReq(me, e);
            break;
        case MAINAPP_VERSION_REQ_SIG:
            ret = MainApp_BleVersionReq(me, e);
            break;
        case MAINAPP_PRODUCT_NAME_REQ_SIG:
            ret = MainApp_BleProductNameReq(me, e);
            break;
        default:
            TP_PRINTF("unknown ble event\r\n");
            break;
    }

    return ret;
}

