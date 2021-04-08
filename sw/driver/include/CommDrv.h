/**
 * @file        CommDrv.h
 * @brief       It's the driver to communicate with SoC
 * @author      Eason Huang 
 * @date        2016-06-02
 * @copyright   Tymphany Ltd.
 */

#ifndef COMM_DRV_H
#define COMM_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"
#include "trace.h"
#include "signals.h"
#include "bsp.h"
#include "attachedDevices.h"

#define MAX_COMM_CMD_LEN    24

/* MCU to SOC Cmd */
#define MCU_CMD_START_CODE                  0xa0
#define MCU_CMD_END_CODE                    0xef
#define MCU_CMD_ACK                         0xee

#define MCU_CMD_VERSION_RETURN              0x00
#define MCU_CMD_SW_RESET                    0x01
#define MCU_CMD_SOC_ACTIVE                  0x02
#define MCU_CMD_TOUCH_CHECK                 0x03
#define MCU_CMD_TOGGLE_UART                 0x04
#define MCU_CMD_POWER_OFF                   0x10 // BTN_POWER
#define MCU_CMD_POWER_ON                    0x11 // BTN_POWER
#define MCU_CMD_BATTERY_NOT_CHARGING        0x12
#define MCU_CMD_BATTERY_CHARGING            0x13
#define MCU_CMD_BATTERY_LEVEL               0x14
#define MCU_CMD_LEAVE_SUSPEND               0x15 //for T8032 only
#define MCU_CMD_GO_SUSPEND                  0x16
#define MCU_CMD_LEAVE_SUSPEND_TO_OFF        0x17
#define MCU_CMD_BATTERY_LOW                 0x18
#define MCU_CMD_BATTERY_FULL                0x19

#define MCU_CMD_KEY_PLAY                    0x20 // BTN_PLAY --> BTN_PLAY_PAUSE
#define MCU_CMD_KEY_PREVIOUS                0x21 // BTN_PREV
#define MCU_CMD_KEY_NEXT                    0x22 // BTN_NEXT
#define MCU_CMD_KEY_BT_PAIRING              0x23 // BTN_BT_PAIR
#define MCU_CMD_KEY_VOLUME_UP               0x60 // BTN_VOL_UP
#define MCU_CMD_KEY_VOLUME_DOWN             0x61 // BTN_VOL_DOWN
#define MCU_CMD_KEY_PLAY_PAUSE              0X62 // BTN_PLAY_PAUSE
#define MCU_CMD_KEY_PAUSE                   0x63 // BTN_PAUSE
#define MCU_CMD_KEY_MUTE                    0x64 // BTN_MUTE
#define MCU_CMD_KEY_SWITCH_SOURCE           0x65 // BTN_SOURCE
#define MCU_CMD_KEY_SWITCH_AUXIN            0x66 // BTN_LINE_IN
#define MCU_CMD_KEY_SWITCH_COAXIAL          0x67 // BTN_COXIAL_IN
#define MCU_CMD_KEY_SWITCH_OPTICAL          0x68 // BTN_OPTICAL_IN
#define MCU_CMD_KEY_SWITCH_HDMI             0x69 // BTN_HDMI_IN_1
#define MCU_CMD_KEY_SWITCH_HDMI_ARC         0x70 // BTN_HDMI_ARC
#define MCU_CMD_KEY_SWITCH_BT               0x71 // BTN_BLUETOOTH
#define MCU_CMD_KEY_VIZIO_TV_REMOTE         0x80
#define MCU_CMD_KEY_ENERGY_STAR             0x81
#define MCU_CMD_KEY_LAUNCH_SOFT_AP          0x82
#define MCU_CMD_KEY_DEMO_MODE_1             0x83
#define MCU_CMD_KEY_DEMO_MODE_2             0x84
#define MCU_CMD_KEY_DEMO_MODE_3             0x85
#define MCU_CMD_KEY_DEMO_POP                0x88
                                            
/* SOC to MCU Cmd */                        
#define SOC_CMD_START_CODE                  0xb0
#define SOC_CMD_END_CODE                    0xef
#define SOC_CMD_ACK                         0xee

#define SOC_CMD_GET_MCU_VERSION             0x00
#define SOC_CMD_SEND_SOC_VERSION            0x01
#define SOC_CMD_RESET_MCU_TO_UPDATE         0x02
#define SOC_CMD_WAKE_UP                     0x10
#define SOC_CMD_GET_BETTERY_LEVEL           0x11
#define SOC_CMD_SUSPEND                     0x12 //NOTE: for T8032
#define SOC_CMD_LEAVE_SUSPEND               0x13 //NOTE: for T8032
#define SOC_CMD_POWER_CUT                   0x14
#define SOC_CMD_PRE_SUSPEND                 0x15
#define SOC_CMD_SLEEP_OFF_TIME              0x16
#define SOC_CMD_ENTER_FAKE_SUSPEND          0x17
#define SOC_CMD_LEAVE_FAKE_SUSPEND          0x18
#define SOC_CMD_DSP_UPGRADE                 0x19

#define SOC_CMD_LED_BT_DISCOVERABLE         0x20
#define SOC_CMD_LED_BT_CONNECTABLE          0x21
#define SOC_CMD_LED_BT_CONNECTED            0x22
#define SOC_CMD_LED_BT_LINK_LOST            0x23
#define SOC_CMD_LED_BT_PAIRING_TIMEOUT      0x24
#define SOC_CMD_LED_WIFI_DISCONNEDTED       0x25
#define SOC_CMD_LED_WIFI_CONNEDTED          0x26
#define SOC_CMD_LED_ETHERNET_DISCONNEDTED   0x27
#define SOC_CMD_LED_ETHERNET_CONNEDTED      0x28
#define SOC_CMD_LED_USB_DISCONNECTED        0x29
#define SOC_CMD_LED_USB_CONNECTED           0x2a
#define SOC_CMD_LED_FW_UPGRADE              0x2b //This is used for Portable in T8032
#define SOC_CMD_LED_FACTORY_RESET           0x2c
#define SOC_CMD_LED_POWER_ON_COMPLETE       0x2d
#define SOC_CMD_LED_ULI_FW_UPGRADE          0x2e
#define SOC_CMD_LED_SYS_REBOOT              0x2f
#define SOC_CMD_LED_VOL_LEVEL               0x30
#define SOC_CMD_LED_TEST                    0x90
#define SOC_CMD_LED_TEST_END                0x91

/* Other */
#define COMMAND_STANDBY_MODE                "STANDBY_MODE"
#define COMMAND_SET_MUTE                    "SET_MUTE"
#define COMMAND_SET_VOLUME                  "SET_VOL"


typedef enum
{
    SOC2MCU,
    MCU2SOC,
} cmdDir;

typedef enum
{
    OFF,
    ON,
} socPower;

typedef enum
{
    USB2USB,
    USB2UART,
} usbFunc;

REQ_EVT(CommCmdEvt)
    cmdDir dir;
    char cmd[MAX_COMM_CMD_LEN+1];
END_REQ_EVT(CommCmdEvt)

CLASS(cCommDrv)
    /* private data */
    bool isCreated;
METHODS
    /* public functions */
void CommDrv_Ctor(cCommDrv *me);
void CommDrv_Xtor(cCommDrv *me);
void CommDrv_WriteCommand(uint8 *cmd);
void CommDrv_SetSOCPower(socPower flag);
void CommDrv_SelSOCUsbFunc(usbFunc func); 
END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* COMM_DRV_H */

