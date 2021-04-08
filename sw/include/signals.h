/******************************************************************************

Copyright (c) 2015, Tymphany HK Ltd. All rights reserved.

Confidential information of Tymphany HK Ltd.

The Software is provided "AS IS" and "WITH ALL FAULTS," without warranty of any
kind, including without limitation the warranties of merchantability, fitness
for a particular purpose and non-infringement. Tymphany HK LTD. makes no
warranty that the Software is free of defects or is suitable for any particular
purpose.

******************************************************************************/

/**
*  @file      signals.h
*  @brief     The file that contains the enumarated signal id's.
*  @author    Christopher Alexander
*  @date      26-Oct-2013
*  @copyright Tymphany Ltd.
*/

#ifndef SIGNALS_H
#define	SIGNALS_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "product.config"  // for the macro
#include "qep.h"

/** \brief Signals that can be passed around in QEvt events
* These should be devided into published (MAX_PUB_SIG) and direct signals
*/
typedef enum
{
    KEY_SIG = Q_USER_SIG, //4
    PAUSE_SIG,            /* published by BSP to pause the application */
    TERMINATE_SIG,        /* published by BSP to terminate the application */
    KEY_STATE_SIG,         /* published by key server */
    ASE_TK_STATE_SIG,     /* published by ASE-TK server */
    ALLPLAY_STATE_SIG,    /* published by allplay server */
    ALLPLAY_EQ_SIG,    /* published by allplay server */
    ALLPLAY_LED_SIG,    /* published by allplay server */
    COMM_STATE_SIG,       /* published by comm server */
    HDMI_STATE_SIG,     /* published by HDMI server */
    USB_STATE_SIG,      /* published by USB server */


    /* Tymphany platform Power Server related Signals */
    POWER_STATE_SIG,    /* PowerSrv => ALL: boardcast an update in Power State*/    
    POWER_BATT_STATE_SIG, /* PowerSrv => ALL: boardcast an update for battery state*/
    POWER_WAKE_UP_SIG,

    /*System Status Signal*/
    SYSTEM_STATE_SIG,
    
    /* AUDIO SERVER PUBLISHED SIGNALS */
    AUDIO_STATE_SIG,
    AUDIO_MUSIC_STREAM_STATE_SIG, /* AudioSrv => ALL: boardcast an update for music detection state */

    LED_STATE_SIG,
    BT_STATE_SIG,
    SETTING_UPDATE_SIG,     /* published when DSS server have finished writing setting into flash */
    POWER_MCU_SLEEP_SIG,    /* KeySrv publish signal for power manager that MCU have to be put into sleep mode, and halt the MCU*/

    MAX_PUB_SIG,          /* the last published signal */

        ALLPLAY_RESP_SIG,     /* AllplaySrv => UI: response the UI with allplay server info*/
        I2C_RESP_SIG,
        LED_RESP_SIG,
        DEBUG_RESP_SIG,
        KEY_DEBUG_RESP_SIG,     /* KeySrv=>debugSrv */
        POWER_RESP_SIG,         /* PowerSrv => UI: response the UI with power server info*/
        POWER_DEBUG_RESP_SIG,   /* PowerSrv => DebugSrv: response with raw value*/
        AUDIO_SET_RESP_SIG,
        AUDIO_MUTE_RESP_SIG,
        BT_RESP_SIG,
        BLE_RESP_SIG,
        SETTING_RESP_SIG,
        SETTING_START_RESP_SIG,
        SETTING_DATA_RESP_SIG,
        SETTING_END_RESP_SIG,
        SYSTEM_ACTIVE_RESP_SIG,
        SYSTEM_SLEEP_RESP_SIG,  /* server => UI : response sleep request */
        SYSTEM_MODE_RESP_SIG,
        AUDIO_SWITCH_CHANNEL_RESP_SIG,

        DEBUG_GPIO_RESP_SIG,  /* to recive gpio status */
        SETTING_READ_OFFSET_RESP_SIG,
        
        ASE_TK_RESP_SIG,
        
        DEBUG_I2C_RESP_SIG,
        DEBUG_ADC_RESP_SIG,
      

        MAX_RESP_SIG,        /*all sig above : 4 bytes sys offset; all sig bellow : 8 bytes sys offset */
    ALLPLAY_RESET_SIG,
    ALLPLAY_CMD_SIG,   
    ALLPLAY_REQ_SIG,      /* UI => AllplaySrv: to query the allplay server info*/

    I2C_WRITE_SIG,
    I2C_READ_SIG,

    /* LED SERVER POSTED SIGNALS */
    LED_REQ_SIG,
    LED_TIMEOUT_SIG,

    /* HDMI SERVER SIGNALS */
    HDMI_REQ_SIG,           /* signals/messages send from MCU server to HDMI chip */
    HDMI_INT_REQ_SIG,       /* interrupt signal get from MCU interrupt pin */
    HDMI_TIMEOUT_SIG,       /* server timeout */

    /* DEBUG SERVER POSTED SIGNALS */   
    DEBUG_SIG,
    DEBUG_GPIO_REQ_SIG, /* debug signal to control gpio  */
    DEBUG_RECV_FRAME_SIG,

    /* KEY SERVER POSTED SIGNALS */
    KEY_DEBUG_REQ_SIG,      /* debugSrv=>KeySrv */ /* TODO: Fix these up */
    KEY_TIMEOUT_SIG,        //0x1F=31
   
    /* POWER SERVER POSTED SIGNALS */
    POWER_REQ_SIG,          /* UI => PowerSrv: to query the power status, battery level info*/

    /* POWER SERVER PRODUCTION TEST SIGNALS */
    POWER_DEBUG_REQ_SIG,    /* PT => PowerSrv: to get the raw value of battery and ac voltage*/

    POWER_STANDBY_SIG,      /* UI => PowerSrv: ask powerSrv to standby, turn off unnessary part*/
    POWER_ACTIVE_SIG,       /* UI => PowerSrv: ask powerSrv to wake up */
    

    POWER_MCU_RESET_SIG,    /* UI => PowerSrv: ask powerSrv to reboot MCU*/

    WAKE_UP_KEY_INTERRUPT_SIG,
    REMOTE_KEY_SIG,

    AC_IN_INTERRUPT_SIG,
    AC_OUT_INTERRUPT_SIG,
    CHARGER_STAT_INTERRUPT_SIG,

    /* Set the battery source and input source */
    POWER_SET_SIG,
    POWER_SET_RESP_SIG,
    POWER_GET_SIG,
    POWER_GET_RESP_SIG,

    POWER_EMERGENCY_MODE_REQ_SIG,
    POWER_EMERGENCY_MODE_RESP_SIG,
    /* Power Dlg => MainApp, indicate the system that battery is in shutdown mode */
    /* or shall we publish power state instead? */
    POWER_OFF_IND_SIG,
    
    EJECT_EXT_BATTERY_REQ_SIG,
    EJECT_EXT_BATTERY_RESP_SIG,
    EXT_BATTERY_IN_INTERRUPT_SIG,

    /* AUDIO SERVER SIG */
    AUDIO_MUTE_SIG,
    AUDIO_SWITCH_CHANNEL_SIG,
    AUDIO_SET_SIG,

    /*Set different power level DRC signals*/
    AUDIO_SET_POWER_LEVEL_DRC_SIG,
    /*Gain adjustment for different battery levels */
    AUDIO_SET_POWER_LEVEL_GAIN_SIG,

    /* DISPLAY SERVER SIG */
    DISPLAY_REQ_SIG,
    DISPLAY_SET_DIM_TIME_SIG,

    /*BT SERVER/DRIVER SIG*/
    BT_STATUS_SIG,
    BT_CMD_DONE_SIG,
    BT_REQ_SIG,
    //For NFC detecting and production test
    BT_NFC_IND_SIG,
    BT_CODEC_STATUS_SIG,

    /* BLE SERVER SIGNAL*/
    BLE_REQ_SIG,
    BLE_RECV_TIMEOUT_SIG,
    BLE_RECV_FRAME_SIG,

    /* Setting Server Signal */
    SETTING_REQ_SIG,
    SETTING_START_REQ_SIG,      /* Start writing data to SettingSrv */
    SETTING_DATA_REQ_SIG,       /* Data sending, should have SETTING_START_REQ_SIG first */
    SETTING_END_REQ_SIG,        /* Last truck of data to be written */
    SETTING_WRITE_OFFSET_REQ_SIG, /* light transaction writng data inside setting data entry with offset */
    SETTING_READ_OFFSET_REQ_SIG,
    SETTING_FLASH_REQ_SIG,      /* Force SettingSrv to load/save data from/into flash right away */

    /* ASE_TK server signal*/
    ASE_TK_REQ_SIG,         /* Send the request signal to ASE_TK server */
    ASETK_RECEIVE_COMMAND,
    
    /* Jack in/out stage and level at plugin */
    JACK_STATE_SIG,

    /* USB server signal */
    USB_REQ_SIG,
    USB_SEND_DATA_SIG,
    USB_RECV_DATA_SIG,

    /* Display server debug signal*/
    DISPLAY_DEBUG_REQ_SIG,
    DISPLAY_SET_BRIGHTNESS_SIG,

    /*Safety monitor signals*/
    SAFETY_ALARM_OVERTEMPERATURE,

    /*Timer*/
    TIMER_FEED_WD_SIG,

    /* comm server signal*/
    COMM_REQ_SIG,         /* Send the request signal to comm server */
    COMM_RECEIVE_COMMAND,
/****************** UI LAYER POSTED SIGNALS ****************/
    IDLE_TIMEOUT_SIG,   /*Idle delegate => its owner: indicate the idle timer is timeout. */
    SYSTEM_ACTIVE_REQ_SIG,
    SYSTEM_SLEEP_REQ_SIG,   /* UI=> server : request sleep  */
    SYSTEM_MODE_REQ_SIG,
    PT_ASE_DISABLE_UART_REQ_SIG,
    /* Added all the dlg signals here */
    DLG_SIG_START,
    VIEW_DLG_REQ_SIG = DLG_SIG_START,
    MAINAPP_START_DLG_SIG,
    MAINAPP_STOP_DLG_SIG,


    DEBUG_I2C_REQ_SIG,
    DEBUG_ADC_REQ_SIG,
    
    AUDIO_MUTE_CHANNEL_REQ_SIG,

    I2C_SLAVE_REQ_SIG,

    MAX_SIG               /* the last signal */
}eSignal;


/** \brief Indication definition
*/
#define IND_EVT(event)    \
typedef struct event {  \
    QEvt super;

#define END_IND_EVT(event)\
} event;

/** \brief Request event
 */
#define REQ_EVT(req_id)    \
typedef struct req_id {  \
    QEvt super;         \
    QActive * sender;
#define END_REQ_EVT(req_id)\
} req_id;


/** \brief Response event
 */
#define RESP_EVT(resp_id) IND_EVT(resp_id) \
    eEvtReturn  evtReturn;
#define END_RESP_EVT(resp_id) END_IND_EVT(resp_id)


#ifdef	__cplusplus
}
#endif

#endif	/* SIGNALS_H */

