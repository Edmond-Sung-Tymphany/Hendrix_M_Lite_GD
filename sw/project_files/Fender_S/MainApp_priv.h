/**
 * @file        MainApp_priv.h
 * @brief       Main application for iBT150
 * @author      Christopher 
 * @date        2014-04-24
 * @copyright   Tymphany Ltd.
 */
 

#ifndef MAIN_APP_PRIVATE_H
#define MAIN_APP_PRIVATE_H
 
#ifdef __cplusplus
 extern "C" {
#endif

#include "product.config"
#include "application.h"

#include "controller.h"
#include "modes.h"
#include "MainApp.h"
#include "gpioDrv.h"

#define DEFAULT_ADC_MIN                       (0)
#define DEFAULT_ADC_MAX                       (4095)

/* audio source related */
#define MAINAPP_SOURCE_AUX      AUDIO_CHANNEL_AUXIN
#define MAINAPP_SOURCE_BT       AUDIO_CHANNEL_BT
#define MAINAPP_SOURCE_MAX          2
#define MAINAPP_SOURCE_DEFAULT      MAINAPP_SOURCE_AUX

/* battery capacity value */
#define MAINAPP_BATTERY_URGENT_LEVEL        (5)
#define MAINAPP_BATTERY_CRITICAL_LEVEL      (10)
#define MAINAPP_BATTERY_LOW_LEVEL              (20)
#define MAINAPP_BATTERY_NORMAL_LEVEL        (90)
#define MAINAPP_BATTERY_FULL_LEVEL              (92)    /*(95)*/    /* (95) nearly fully charged */


/* combo key handler */
#define GPIO_KEY_RELEASED           ((uint8_t)0x00)
#define GPIO_KEY_TALK_PRESSED     ((uint8_t)0x01)
#define GPIO_KEY_BT_PRESSED         ((uint8_t)0x02)

// add signal control
REQ_EVT(MainAppCtlEvt)
    uint32_t value;
END_REQ_EVT(MainAppCtlEvt)

#ifdef __cplusplus
}
#endif
 
#endif /* MAIN_APP_PRIVATE_H */
