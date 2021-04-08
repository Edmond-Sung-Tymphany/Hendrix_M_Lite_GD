/**
 *  @file      CS48L11_Driver.c
 *  @brief     This file contains the driver for Cirrus Logic DSP with Dolby decode function
 *  @author    Viking Wang
 *  @date      17-Oct-2017
 *  @copyright Tymphany Ltd.
 */

#include "product.config"

#ifdef HAS_CS48L11_DSP

#include "commontypes.h"
#include "stm32f0xx.h"
#include "trace.h"
#include "bsp.h"
#include "attachedDevices.h"
#include "GPIODrv.h"
#include "HWspiDrv.h"

#include "CS48L11_priv.h"
#include "CS48L11_Driver.h"

// include dsp firmware
#include "./Cs48L11FW/OS.h"
#include "./Cs48L11FW/cfg0_48L11_60_0100.h"
#include "./Cs48L11FW/APP.h"
#include "./Cs48L11FW/preKickStart.h"
#include "./Cs48L11FW/initial.h"
#include "./Cs48L11FW/KickStart.h"

//#define CS48L11_ONLINE_TUNING

static cGpioDrv cs48L11_gpio;
static cHWspiDrv cs48L11_drv;
static cHWspiDrv *p_cs48L11_drv;

/* read buffer */
//#define CS48L11_READ_BUF_SIZE   8
#define CS48L11_READ_BUF_SIZE   4
static uint8_t cs48L11_read_buffer[CS48L11_READ_BUF_SIZE];
static bool cs48l11_restart_needed=FALSE;
static uint32_t cs48L11_boot_status;

/* slave boot & soft reset command */
const static uint8_t cs48L11_cmd_slave_boot[] = { 0x80, 0x00, 0x00, 0x00 };
const static uint8_t cs48L11_cmd_soft_reset[] = { 0x40, 0x00, 0x00, 0x00 };

#define CS48L11_SPI_CS_DISABLE  GpioDrv_SetBit(&cs48L11_gpio, GPIO_OUT_CS48L11_SPI_CS)
#define CS48L11_SPI_CS_ENABLE   GpioDrv_ClearBit(&cs48L11_gpio, GPIO_OUT_CS48L11_SPI_CS)

/* waiting for the BUSY pin to high(ready) */
static bool Cs48L11_WaitBusyHigh(uint32_t timeout)
{
    uint32_t tickStart;
    bool success=FALSE;
    if( cs48l11_restart_needed )
        return FALSE;

    tickStart = getSysTime() + timeout;
    
    do
    {
        if( GpioDrv_ReadBit(&cs48L11_gpio, GPIO_IN_CS48L11_BUSY) )
        {
            success = TRUE;
            break;
        }
    }while(tickStart > getSysTime());

    return success;
}

static bool Cs48L11_WriteBytes(uint16_t len, uint8_t *p_data)
{
    uint8_t write_cmd=0x80;
    bool success=FALSE;
    
    if( Cs48L11_WaitBusyHigh(10) )
    {
        CS48L11_SPI_CS_ENABLE;

        HWspiDrv_WriteRead(p_cs48L11_drv, &write_cmd, NULL, 1, 10);
        HWspiDrv_WriteRead(p_cs48L11_drv, p_data, NULL, len, 10);

        CS48L11_SPI_CS_DISABLE;
        success = TRUE;
    }
    else
    {
        success = FALSE;
        ALWAYS_printf("\n\rError : Busy pin is low.");
    }

    return success;
}

/* write a mount of data */
static bool Cs48L11_LoadFirmware(uint16_t len, uint8_t *p_data)
{
    uint8_t write_cmd=0x80;
    bool success=FALSE;
    uint32_t i, wait_cnt;
    
    if( Cs48L11_WaitBusyHigh(10) )
    {
        success = TRUE;
        CS48L11_SPI_CS_ENABLE;

        HWspiDrv_WriteRead(p_cs48L11_drv, &write_cmd, NULL, 1, 10);

        for(i=0; i<len; i+=4)
        {
            HWspiDrv_WriteRead(p_cs48L11_drv, p_data, NULL, 4, 10);
            p_data += 4;
            wait_cnt = 0;
            // hard code the GPIO read for speed up
            while( IS_CS48L11_BUSY_PIN_LOW )
            {
                wait_cnt ++;
                if( wait_cnt > 100 )
                {
                    i = len;    // exit for(...) loop
                    success = FALSE;
                    ALWAYS_printf("\n\rLoad Firmware ERROR.");
                    break;
                }
            }
        }

        CS48L11_SPI_CS_DISABLE;
    }
    else
    {
        success = FALSE;
        ALWAYS_printf("\n\rError : Busy pin is low.");
    }

    return success;
}

static void CS48L11_ReadOneFrame()
{
    uint8_t read_cmd=0x81;
    
    CS48L11_SPI_CS_ENABLE;

    // send read command
    HWspiDrv_WriteRead(p_cs48L11_drv, &read_cmd, NULL, 1, 10);
    // read data from dsp
    HWspiDrv_WriteRead(p_cs48L11_drv, NULL, cs48L11_read_buffer, CS48L11_READ_BUF_SIZE, 10);
    
    CS48L11_SPI_CS_DISABLE;
}

static bool CS48L11_ReadBootStaus(uint32_t timeout)
{
    uint32_t tickStart;
    bool success=FALSE;
    
    tickStart = getSysTime() + timeout;
    do
    {
        if( GpioDrv_ReadBit(&cs48L11_gpio, GPIO_IN_CS48L11_INT) == RESET )
        {
            success = TRUE;
            break;
        }
    }while(tickStart > getSysTime());

    if( success )
    {
        CS48L11_ReadOneFrame();
        cs48L11_boot_status = (((uint32_t)cs48L11_read_buffer[0]) << 24) | (((uint32_t)cs48L11_read_buffer[1]) << 16) | \
                              (((uint32_t)cs48L11_read_buffer[2]) << 8) | ((uint32_t)cs48L11_read_buffer[3]);
    }
    else
    {   // clear the read buffer
        memset(cs48L11_read_buffer, 0xee, CS48L11_READ_BUF_SIZE);
        cs48L11_boot_status = 0xeeeeeeee;
        ALWAYS_printf("\n\rError : INT pin is high.");
    }

    return success;
}

/**
 * Construct the DSP driver instance.
 * @return : none
 */
void Cs48L11Drv_Ctor(void)
{
    tSpiDevice *p_device;

    p_cs48L11_drv = &cs48L11_drv;

    ASSERT( ! p_cs48L11_drv->isReady );

    cs48L11_gpio.gpioConfig = (tGPIODevice *) getDevicebyIdAndType(DSP_DECODER_DEV_ID, GPIO_DEV_TYPE, NULL);
    ASSERT(cs48L11_gpio.gpioConfig);
    GpioDrv_Ctor(&cs48L11_gpio, cs48L11_gpio.gpioConfig);

    p_device = (tSpiDevice *) getDevicebyIdAndType(DSP_DECODER_DEV_ID, SPI_DEV_TYPE, NULL);
    ASSERT(p_device);

#ifndef CS48L11_ONLINE_TUNING
    HWspiDrv_Ctor(p_cs48L11_drv, p_device);
#endif

    p_cs48L11_drv->isReady = TRUE;
    p_cs48L11_drv->isBusy = FALSE;
}

/**
 * Exit & clean up the driver.
 */
void Cs48L11Drv_Xtor(void)
{
    GpioDrv_Xtor(&cs48L11_gpio);
#ifndef CS48L11_ONLINE_TUNING
    HWspiDrv_Xtor(p_cs48L11_drv);
#endif
    p_cs48L11_drv->isReady = FALSE;
    p_cs48L11_drv->isBusy = TRUE;
    p_cs48L11_drv = NULL;
}


bool CS48L11Drv_LoadULD(UldID_t id)
{
    bool success=FALSE;

    ASSERT(id < ULD_ID_MAX);

    // send slave boot command
    if( ! Cs48L11_WriteBytes(4, (uint8_t *)cs48L11_cmd_slave_boot) )
        return success;

    // check boot message
    CS48L11_ReadBootStaus(10);
    if( cs48L11_boot_status == DSP_MSG_BOOT_START )
    {
        success = TRUE;
        switch( id )
        {
        case ULD_ID_OS:
            Cs48L11_LoadFirmware(Bytes_of_OS_uld, (uint8_t *)OS_ULD);
            break;
        case ULD_ID_AC3:
            Cs48L11_LoadFirmware(Bytes_of_CA22_AC3_CustomPPM_cfg0_48L11_60_0100_uld, (uint8_t *)CA22_AC3_CUSTOMPPM_CFG0_48L11_60_0100_ULD);
            break;
        case ULD_ID_CROSSBAR:
            break;
        case ULD_ID_APP:
            Cs48L11_LoadFirmware(Bytes_of_APP_uld, (uint8_t *)APP_ULD);
            break;
        case ULD_ID_PPM:
            break;
        default :
            success = FALSE;
            ALWAYS_printf("\n\rUnknow ULD id = %d.", id);
            break;
        }
    }
    else
    {
        ALWAYS_printf("\n\rLoading UldID=%d error. boot status = 0x%x.", id, cs48L11_boot_status);
        return success;
    }

    CS48L11_ReadBootStaus(10);
    if( cs48L11_boot_status == DSP_MSG_BOOT_SUCCESS )
    {
        success = TRUE;
    }
    else
    {
        success = FALSE;
        ALWAYS_printf("\n\rLoading UldID=%d error. boot failure.", id);
    }

    return success;
}

/**
 * Write the dsp program/parameter data.
 */

bool Cs48L11Drv_Init(void)
{
    bool success=FALSE;
    uint32_t retry_cnt=0;

#ifdef CS48L11_ONLINE_TUNING
    return TRUE;
#endif

    while( retry_cnt < 3 )
    {
        retry_cnt ++;

        // check "BOOT_READY"
        CS48L11_ReadBootStaus(10);
        if( cs48L11_boot_status != DSP_MSG_BOOT_READY )
        {
            ALWAYS_printf("\n\rDSP boot not ready : %d retry.", retry_cnt);
            continue;
        }
        // load ULD files
        if( ! CS48L11Drv_LoadULD(ULD_ID_OS) )
        {
            ALWAYS_printf("\n\rDSP can not load OS : %d retry.", retry_cnt);
            continue;
        }
        if( ! CS48L11Drv_LoadULD(ULD_ID_APP) )
        {
            ALWAYS_printf("\n\rDSP can not load APP : %d retry.", retry_cnt);
            continue;
        }
        if( ! CS48L11Drv_LoadULD(ULD_ID_AC3) )
        {
            ALWAYS_printf("\n\rDSP can not load AC3 : %d retry.", retry_cnt);
            continue;
        }

        // soft reset
        if( ! Cs48L11_WriteBytes(4, (uint8_t *)cs48L11_cmd_soft_reset) )
        {
            ALWAYS_printf("\n\rDSP can not soft reset : %d retry.", retry_cnt);
            continue;
        }
        // check "APP_START"
        CS48L11_ReadBootStaus(10);
        if( cs48L11_boot_status != DSP_MSG_APP_START )
        {
            ALWAYS_printf("\n\rDSP APP START not ready : %d retry.", retry_cnt);
            continue;
        }
//        BSP_BlockingDelayMs(20);
        // send hardware configuration
        if( ! Cs48L11_LoadFirmware(Bytes_of_preKickStart_cfg, (uint8_t *)PREKICKSTART_CFG) )
        {
            ALWAYS_printf("\n\rDSP can not load preKichStart.");
            continue;
        }
//        BSP_BlockingDelayMs(20);
        // send firmware configuration
        if( ! Cs48L11_LoadFirmware(Bytes_of_initial_cfg, (uint8_t *)INITIAL_CFG) )
        {
            ALWAYS_printf("\n\rDSP can not load initial.h");
            continue;
        }
//        BSP_BlockingDelayMs(20);
        // send KICKSTART
        if( ! Cs48L11_LoadFirmware(Bytes_of_KickStart_cfg, (uint8_t *)KICKSTART_CFG) )
        {
            ALWAYS_printf("\n\rDSP can not load KickStart.");
            continue;
        }

        //CS48L11_ReadBootStaus(10);
        //printf("\n\rBoot status = 0x%x", cs48L11_boot_status);

        success = TRUE;
        if( success )
            break;
    }
        
    return success;
}

#endif  // HAS_CS48L11_DSP
