/**
 * @file        BluetoothDrv.h
 * @brief       BT Driver header file
 * @author      Edmond Sung
 * @date        2014-02-10
 * @copyright   Tymphany Ltd.

 
 Change History:
 VERSION    : 1    DRAFT      2014-02-10     Edmond Sung
 DESCRIPTION: First Draft.
 SCO/ERROR  :
 
 VERSION    : 2    DRAFT      2014-05-13     Johnny Fan
 DESCRIPTION: second Draft. To implement the driver for ROM based CSR module
 SCO/ERROR  :


 */

#ifndef BLUETOOTH_DRIVER_H
#define BLUETOOTH_DRIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "qp_port.h"
#include "signals.h"
#include "BluetoothSrv.h"
#include "GpioDrv.h"

/*The type of time request that driver ask for*/
typedef enum
{
    BT_START_TIME,  /* start calculating*/
    BT_END_TIME,    /* end calculating*/
    BT_SET_TIME,    /* set a time */
    BT_MAX_TIME,
}eTimeType;

/* BT status event*/
IND_EVT(BtDrvStatusEvt)
    uint8 btStatus;
END_IND_EVT(BtDrvStatusEvt)

/* BT cmd done event*/
IND_EVT(BtDrvCmdEvt)
    eBtCmd cmd;
END_IND_EVT(BtDrvCmdEvt)

/* BT codec status*/
IND_EVT(BtDrvCodecStatusEvt)
    uint8 codecStatus;
END_IND_EVT(BtDrvCodecStatusEvt)

/* call back function to calcute the time*/
typedef uint32(*timeReqCb)(eTimeType reqTimeType, int32 timeInMs);


CLASS(cBluetoothDrv)   
    /* private data */
    uint8 step;
#ifdef HAS_BT_SEQ_CONTROL
    uint8_t startup_step;
    uint8_t poweroff_step;
#endif
    eBtCmd cmd;
    QActive* pRequester;
    timeReqCb pTimeReq;
METHODS
    /* public functions */
void BluetoothDrv_Ctor(cBluetoothDrv *me);
void BluetoothDrv_Xtor(cBluetoothDrv *me);


void BluetoothDrv_EnableCodecStatus(bool bEnable );
bool BluetoothDrv_CodecStatus(void);
void BluetoothDrv_TurnOnBT(cBluetoothDrv *me);
void BluetoothDrv_TurnOffBT(cBluetoothDrv *me);
void BluetoothDrv_PowerEnable(bool enable);
void BluetoothDrv_DisconnectBT(void);
#ifdef HAS_FIX_VOL_AUDIO_CUE
void BluetoothDrv_AudioCueHold(uint16 delay_time);
bool BluetoothDrv_CheckAudioCuePlaying(uint16 interval);
void BluetoothDrv_SetDelayStartAudioCue(uint16 delay_time);
bool BluetoothDrv_DelayStartAudioCue(uint16 interval);
#endif
/**
* send the BT command to BT module 
* @param[in]      me         bluetooth driver object
* @param[in]      cmd       the BT command 
*/
void BluetoothDrv_ExecuteCmd(cBluetoothDrv *me, eBtCmd cmd);


/**
* inform the BT driver that time is up 
* @param[in]      me         bluetooth driver object
*/
void BluetoothDrv_TimeIsUp(cBluetoothDrv *me);


/**
* register the timeReq call back function
* clent can reset the call back function by calling this function
* @param[in]      me                 bluetooth driver object
* @param[in]      fCb               the time call back function pointer            
*/
void BluetoothDrv_RegisterTimeReqCb(cBluetoothDrv* me, timeReqCb fCb);

/**
* register to get the BT module status signal from driver 
* clent can reset the receiver by calling this function
* @param[in]      me                 bluetooth driver object
* @param[in]      req                the object (BT server) to receive the signal     
*/
void BluetoothDrv_RegisterDriverSig(cBluetoothDrv* me, QActive* req);

/**
* un-register to get the BT module status signal from driver 
* @param[in]      me                 bluetooth driver object
*/
void BluetoothDrv_UnRegisterDriverSig(cBluetoothDrv* me);

/* current : LED0 used as status respond, LED1 used as event respond */
void BluetoothDrv_Led0_IRQ_Handler(void);
void BluetoothDrv_Led1_IRQ_Handler(void);

#ifdef HAS_BT_SEQ_CONTROL
int32_t BluetoothDrv_StartupInit(cBluetoothDrv* me);
int32_t BluetoothDrv_PowerOffInit(cBluetoothDrv* me);
#endif

END_CLASS


#endif /* BLUETOOTH_DRIVER_H */

