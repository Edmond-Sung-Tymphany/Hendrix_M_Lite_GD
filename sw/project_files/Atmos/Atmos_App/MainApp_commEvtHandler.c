/**
*  @file      MainApp_KeyEvtHandler.c
*  @brief     Key event handler of mainApp
*  @author    Viking Wang
*  @date      26-May-2016
*  @copyright Tymphany Ltd.
*/

/*****************************************************************
 * Include
 *****************************************************************/
#include "product.config"
#include "controller.h"
#include "bsp.h"
#include "trace.h"
#include "AudioSrv.h"
#include "KeySrv.h"
#include "LedSrv.h"
#include "PowerSrv.h"
#include "SettingSrv.h"
#include "CommSrv.h"
#include "NvmDrv.h" //NvmDrv_WriteWord()
#include "tym_qp_lib.h"
#include "MainApp_priv.h"
#include "MainApp_util.h"

#define  SOURCE_TO_LED_ID(_x)  ((_x) - AUDIO_SOURCE_AUXIN + LED_IND_ID_INPUT_AUXIN)
/*****************************************************************
 * Global Variable
 *****************************************************************/


/*****************************************************************
 * Function Implemenation
 *****************************************************************/

QState MainApp_commHandler(cMainApp * const me, QEvt const * const e)
{
    QState ret = Q_UNHANDLED();
    CommStateEvt *evt = (CommStateEvt*)e;
    switch(evt->evtId)
    {
        case COMM_EVT_DSPUPGRADE:
            TP_PRINTF("%s: COMM_EVT_DSPUPGRADE\n\r", __func__);
            MainApp_SwitchAudioSource(me, AUDIO_SOURCE_BLUETOOTH);
            MainApp_SendLedReq(me, LED_IND_ID_ALL_OFF);  
            MainApp_SendLedReq(me,SOURCE_TO_LED_ID(AUDIO_SOURCE_COAXIAL));  //led #4    
            ret = Q_HANDLED();
            break;    
        case COMM_EVT_STANDBY:
            TP_PRINTF("%s: COMM_EVT_STANDBY\n\r", __func__);
            ret = Q_HANDLED();
            break;
        case COMM_EVT_VOLUME:
            TP_PRINTF("%s: COMM_EVT_VOLUME\n\r", __func__);
            ret = Q_HANDLED();
            break;
        case KEY_EVT_MUTE:
            TP_PRINTF("%s: KEY_EVT_MUTE\n\r", __func__);
            ret = Q_HANDLED();
            break;
        default:
            break;
    }
      
    return ret;

}