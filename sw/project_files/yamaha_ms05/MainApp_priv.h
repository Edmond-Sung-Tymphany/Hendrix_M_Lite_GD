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

#include "keySrv.h"
#include "controller.h"
#include "modes.h"
#include "MainApp.h"
#include "audioSrv.h"
#include "BluetoothSrv.h"
#include "gpioDrv.h"

#ifdef HAS_MENU
#include "MenuDlg.h"
#endif
   
#define PHASE_DEGREE                0x1B /*ascii 0x1B used for phase degree symbol*/
#define ENABLED                     1
#define DISABLED                    0
#define PRESET_EMPTY                0
#define TEN_MS                      10
#define MINUTES_IN_MS(x)            ((x)*60*1000)
#define SECONDS_IN_MS(x)            ((x)*1000)
#define TWO_SECONDS                 2
#define FLOAT_TYPE(x)               x<10
#define STANDBY_TIME_THRESHOLD_MIN  20 /* 2O minites */
#define NUM_OF_PRESET               3

#define RGC_FREQ_25                 250
#define RGC_FREQ_31                 310
#define RGC_FREQ_40                 400

#define PEQ_FREQ_20                 200   
#define PEQ_FREQ_22                 220
#define PEQ_FREQ_25                 250
#define PEQ_FREQ_28                 280
#define PEQ_FREQ_30                 300

#define DISPLAY_VOL                 0
#define DISPLAY_OFF                 1
#define DISPLAY_LOGO                2


#define NO_OFFSET                   0
#define ONE_ITEM                    1
#define DEFAULT_TIMEOUT_SEC         10

#define POLARITY_POSITIVE           0
#define POLARITY_NEGATIVE           10


#ifdef __cplusplus
}
#endif
 
#endif /* MAIN_APP_PRIVATE_H */
