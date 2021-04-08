/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Boot-Loader common functions
                  -------------------------

                  SW Module Document




@file        bl_common.c
@brief       Implemented a boot-loader which will NOT be updated through OTA
@author      Wesley Lee
@date        30-Nov-2015
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-11     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "fep_addr.h"
#include "stm32f0xx.h"
#include "bl_common.h"
#include "stdio.h"
#include "string.h"

typedef void (*pFunction)(void);
static uint32_t SystemCoreClock = 8000000;
volatile static uint32 currTime = 0;

__weak void SysTick_Handler(void) {                       /* system clock tick ISR */
    ++currTime;
}

/**
* @brief Get the tick elapsed
* @return   uint32  the tick elapsed
*/
__weak uint32 getSysTime()
{
    return currTime;
}

void bl_BSP_init(void)
{
    /* update the clock value "SystemCoreClock" */
    uint32 tmp = 0;
    __I uint8 AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

    SystemCoreClock = HSI_VALUE;

    /* Compute HCLK clock frequency ----------------*/
    /* Get HCLK prescaler */
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> 4)];
    /* HCLK clock frequency */
    SystemCoreClock >>= tmp;

    if (SysTick_Config(SystemCoreClock / 1000))
    {
        /* Capture error */
        while (1);
    }
}

void bl_jumpAddr(uint32 address)
{
    pFunction Jump_To_Application;
    uint32_t JumpAddress;

    // Shut down any tasks running
    RCC_DeInit();
    SysTick->CTRL   = 0;  // reset the Systick Timer
    SysTick->LOAD   = 0;
    SysTick->VAL    = 0;
    // RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);     // Select HSI as system clock source

    __set_PRIMASK(1);   // Disable interrupts

    /* Jump to user application */
    JumpAddress = *(__IO uint32_t*) (address + 4);
    Jump_To_Application = (pFunction) JumpAddress;

    /* Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) address);
    
    /* Jump to application */
    Jump_To_Application();
}

uint32 bl_calcChecksum(uint32* pStart, uint32* pEnd)
{
    uint32* pAddr = pStart;
    uint32 checksum = 0xFFFFFFFF;

    /* Enable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, ENABLE);
    CRC_ResetDR();

    while (pAddr < pEnd)
    {
        checksum = CRC_CalcCRC(*pAddr);
        pAddr++;
    }

    /* Disable CRC clock */
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_CRC, DISABLE);

    return checksum;
}

static void bl_setFepStatus(uint32* addr, uint32 value)
{
    /* To increase flash life cycle, only write when value is difference */
    if( *addr == value )
    {
        return;
    }
    
    /* Read data from flash first */
    eFepStblStatus stbl_status = (eFepStblStatus) *(uint32 *) FEP_ADDR_STBL_STATUS;
    uint32 ubl_checksum       = *(uint32*)FEP_ADDR_UBL_CHECKSUM;
    uint32 firmware_checksum  = *(uint32*)FEP_ADDR_FIRMWARE_CHECKSUM;
    uint32 firmware_status    = *(uint32*)FEP_ADDR_FIRMWARE_STATUS;
    uint32 piu_ver            = *(uint32*)FEP_ADDR_PIU_VER;
    uint32 ubl_ver            = *(uint32*)FEP_ADDR_UBL_VER;
    uint32 firmware_ver       = *(uint32*)FEP_ADDR_FIRMWARE_VER;

    /* Change data accordingly */
    switch ((uint32)addr)
    {
        case FEP_ADDR_STBL_STATUS:
            stbl_status = (eFepStblStatus)value;
            break;
        case FEP_ADDR_FIRMWARE_STATUS:
            firmware_status = (eFepStblStatus)value;
            break;
        case FEP_ADDR_PIU_VER:
            piu_ver = value;
            break;
        case FEP_ADDR_UBL_VER:
            ubl_ver = value;
            break;
        case FEP_ADDR_FIRMWARE_VER:
            firmware_ver = value;
            break;
        default:
            break;
    }

    /* Write new value to flash */
    FLASH_Unlock();
    // Erase Flash
    FLASH_Status s = FLASH_ErasePage(FEP_ADDR_STBL_STATUS);
    // Write to Flash
    s = FLASH_ProgramWord(FEP_ADDR_STBL_STATUS, stbl_status);
    s = FLASH_ProgramWord(FEP_ADDR_UBL_CHECKSUM, ubl_checksum);
    s = FLASH_ProgramWord(FEP_ADDR_FIRMWARE_CHECKSUM, firmware_checksum);
    s = FLASH_ProgramWord(FEP_ADDR_FIRMWARE_STATUS, firmware_status);
    s = FLASH_ProgramWord(FEP_ADDR_PIU_VER, piu_ver);
    s = FLASH_ProgramWord(FEP_ADDR_UBL_VER, ubl_ver);
    s = FLASH_ProgramWord(FEP_ADDR_FIRMWARE_VER, firmware_ver);
    FLASH_Lock();
}

void bl_setStblStatus(uint32 stbl_status)
{
    bl_setFepStatus((uint32 *)FEP_ADDR_STBL_STATUS, stbl_status);
}

eFepStblStatus bl_getStblStatus(void)
{
    eFepStblStatus stbl_Status = (eFepStblStatus) *(uint32 *) FEP_ADDR_STBL_STATUS;
    return stbl_Status;
}

void bl_setFirmwareStatus(uint32 fw_status)
{
    bl_setFepStatus((uint32 *)FEP_ADDR_FIRMWARE_STATUS, fw_status);
}


eFepFirmwareStatus bl_getFirmwareStatus(void)
{
    eFepFirmwareStatus fw_status = (eFepFirmwareStatus) *(uint32 *) FEP_ADDR_FIRMWARE_STATUS;
    return fw_status;
}


//NOTE: this function only write version when stored version is different
void bl_writeVersion(void *addr, uint8 ver1, uint8 ver2, uint8 ver3, uint8 ver4)
{
    uint32 ver= (ver1<<24) | (ver2<<16) | (ver3<<8) | ver4;
    bl_setFepStatus(addr, ver);
}


char* bl_readVersion(void *addr)
{
    static char buf[20];
    uint8 *addr2= (uint8*)addr;
    snprintf(buf, sizeof(buf), "%d.%d.%d.%d", addr2[3], addr2[2], addr2[1], addr2[0]);
    return buf;
}


