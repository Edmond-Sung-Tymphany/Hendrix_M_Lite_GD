/*****************************************************************************
*  @file      MainApp.h
*  @brief     Public header file for main app. This is implemented for the product in question
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*****************************************************************************/

#ifndef MAINAPP_PRIV_H
#define	MAINAPP_PRIV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "MainApp.h"
#include "KeySrv.h"

/*****************************************************************
 * Type Definition
 *****************************************************************/
#ifdef HAS_SHOP_MODE
typedef enum
{
    SHOP_MODE_DISABLE= 0, //normal mode
    SHOP_MODE_ENABLE,
    SHOP_MODE_MAX
} eShopMode;
#endif


typedef enum
{
    REBOOT_CODE_INVALID = 0,
    REBOOT_CODE_ASE_BOOTING_TIMEOUT,
    REBOOT_CODE_ASE_COM_WDG_TIMEOUT,
    REBOOT_CODE_UPGRADE_TIMEOUT,
    REBOOT_CODE_PREACTIVE_TIMEOUT,
    REBOOT_CODE_POWERING_UP_TIMEOUT,
    REBOOT_CODE_POWERING_DOWN_TIMEOUT,
} eRebootCode;

enum MainAppPriSignals /* main appp private signals */
{
    MAINAPP_TIMEOUT_SIG = MAX_SIG,

    MAINAPP_SET_VOL_REQ_SIG      = 0xE0,
    MAINAPP_GET_VOL_REQ_SIG      = 0xE1,
    MAINAPP_GET_VOL_RESP_SIG     = 0xE2,
    BLE_WRITE_DATA_REQ_SIG       = 0xF0,//240
    BLE_READ_DATA_REQ_SIG        = 0xF1,//241
    BLE_READ_DATA_RESP_SIG       = 0xF2,//242
    BLE_RESET_ITEM_REQ_SIG       = 0xF3,//243
    BLE_MENU_FEATURE_REQ_SIG     = 0xF4,//244 what feature this product has
    BLE_MENU_FEATURE_RESP_SIG    = 0xF5,//245
    BLE_UART_VERIFY_REQ_SIG      = 0xF6,//246
    BLE_UART_VERIFY_RESP_SIG     = 0xF7,//247
    BLE_PRODUCTION_TEST_RQE_SIG  = 0xF8,//248 production test signal event only defined in sdf file
    BLE_PRODUCTION_TEST_RESP_SIG = 0xF9,//249
    MAINAPP_DFU_REQ_SIG          = 0xFA,//250
    MAINAPP_DFU_RESP_SIG         = 0xFB,//251
    MAINAPP_VERSION_REQ_SIG      = 0xFC,//252
    MAINAPP_VERSION_RESP_SIG     = 0xFD,//253
    MAINAPP_PRODUCT_NAME_REQ_SIG = 0xFE, //254
    MAINAPP_PRODUCT_NAME_RESP_SIG= 0xFF, //255
};

typedef enum
{
    TIMER_ID_IDLE_CHECK_TIMEOUT = 0,
    TIMER_ID_ENTER_STANDBY_TIMEOUT,
    TIMER_ID_SYS_TEMP_CHECK_TIMEOUT,
    TIMER_ID_STACK_CHECK_TIMEOUT,
    TIMER_ID_JACK_LOW_TIMEOUT,
    TIMER_ID_STARTUP_AMP_TIMEOUT,
    TIMER_ID_SHUTDOWN_AMP_TIMEOUT,
    TIMER_ID_DSPMUTE_TIMEOUT,

//    TIMER_ID_DELAYED_ERROR_REBOOT,
//    TIMER_ID_DELAYED_ENTER_SHOP_MODE,
//    TIMER_ID_DELAYED_POWER_INIT,
//    TIMER_ID_UPGRADE_TIMEOUT,
//    TIMER_ID_FACTORY_RESET_TIMEOUT,
//    TIMER_ID_POWER_DOWN_TIMEOUT,

    TIMER_ID_MAX
}eTimerID;

#define FEATURE_ID_DISPLAY              (0x00)
#define FEATURE_ID_SYS_TIMEOUT          (0x01)
#define FEATURE_ID_STANDBY              (0x02)
#define FEATURE_ID_LP                   (0x03)
#define FEATURE_ID_HP                   (0x04)
#define FEATURE_ID_PEQ1                 (0x05)
#define FEATURE_ID_PEQ2                 (0x06)
#define FEATURE_ID_PEQ3                 (0x07)
#define FEATURE_ID_RGC                  (0x08)
#define FEATURE_ID_PHASE                (0x09)
#define FEATURE_ID_POLARITY             (0x0A)
#define FEATURE_ID_TUNNING              (0x0B)
#define FEATURE_ID_VOLUME               (0x0C)
#define FEATURE_ID_PRESET_NAME          (0x0D)
#define FEATURE_ID_BRIGHTNESS           (0x0E)
#define FEATURE_ID_OTA                  (0x0F)
#define FEATURE_ID_REMOVE_LOCK          (0x80)
#define FEATURE_ID_REMOVE_DISPLAY_ON    (0x81)
#define FEATURE_ID_REMOVE_TUNING_EXT    (0x82)

#ifdef MAINAPP_DEBUG
    const static char *mainApp_debug = "[MainApp_Debug] ";
    #define MAINAPP_DEBUG_MSG TP_PRINTF("\n%s", mainApp_debug); TP_PRINTF
    #define TYMQP_DUMP_QUEUE_WITH_LOG(me, ...) TymQP_DumpQueue_WithLog((QActive *)(me), __func__, __VA_ARGS__)
#else
    #define MAINAPP_DEBUG_MSG(...)
    #define TYMQP_DUMP_QUEUE_WITH_LOG(...)
#endif

#define MAINAPP_POWERING_UP_IN_MS                (1000) /*1sec*/
#define MAINAPP_TIMEOUT_IN_MS                    (100)  /*100 ms */

#define MAINAPP_SYS_TEMP_CHECK_TIMEOUT_IN_MS     (1000) /* 1s */
#define MAINAPP_FACTORY_RESET_TIMEOUT_IN_MS      (500)  /* should be the same as MAINAPP_POWERING_UP_IN_MS */
#define MAINAPP_UPGRADE_TIMEOUT_IN_MS            (500)
#define MAINAPP_STACK_CHECK_TIMEOUT_IN_MS        (1000*120) /* 120s */
#define TIMER_ID_JACK_LOW_TIMEOUT_IN_MS          (10*1000)
#define TIMER_ID_STARTUP_AMP_TIMEOUT_IN_MS       (2000)
#define TIMER_ID_SHUTDOWN_AMP_TIMEOUT_IN_MS      (100)
#define TIMER_ID_DSPMUTE_TIMEOUT_IN_MS           (1000)

#define TIMER_ID_ENTER_STANDBY_TIMEOUT_IN_MS     (20*60*1000)   /*20min*/

#define AMP_STANDBY_ENABLE
#define AMP_STANDBY_DISABLE

/************************************************************************************/
/* State function defintions */
static QState MainApp_Initial(cMainApp * const me, QEvt const * const e);
static QState MainApp_PreActive(cMainApp * const me, QEvt const * const e);
/************************************************************************************/
/* Other Functions */

END_CLASS

#ifdef	__cplusplus
}
#endif

#endif	/* MAINAPP_PRIV_H */
