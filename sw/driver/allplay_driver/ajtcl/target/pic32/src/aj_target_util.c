/**
 * @file
 */
/******************************************************************************
 * Copyright 2012-2013, Qualcomm Innovation Center, Inc.
 *
 *    All rights reserved.
 *    This file is licensed under the 3-clause BSD license in the NOTICE.txt
 *    file for this project. A copy of the 3-clause BSD license is found at:
 *
 *        http://opensource.org/licenses/BSD-3-Clause.
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the license is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the license for the specific language governing permissions and
 *    limitations under the license.
 ******************************************************************************/
#include "plib.h"
#include <stdlib.h>
#include "commonTypes.h"
#include "bsp.h"
#include "byteSwapLib.h"

#include "aj_target.h"
#include "aj_util.h"
#include "aj_serio.h"

void AJ_Sleep(uint32_t time)
{
    uint32 start = 0;
    uint32 current = 0;
    start = getSysTime();

    while ((current - start) < time) {
        // g_msTicks is being updated by SysTick_Handler, so just keep spinning.
        // (it might be nicer to be woken up by an alarm instead)
        current = getSysTime();
    }
}

//return millionseconds
uint32_t AJ_GetElapsedTime(AJ_Time* timer, uint8_t cumulative)
{
    uint32_t elapsed;
    uint32_t ticks_per_sec = BSP_TICKS_PER_SEC;
    AJ_Time now;

    uint32_t ticks;
    ticks = getSysTime();
    now.seconds = ticks / ticks_per_sec;
    now.milliseconds = (ticks - (now.seconds * ticks_per_sec)) * 1000 / ticks_per_sec;

    elapsed = (1000 * (now.seconds - timer->seconds)) + (now.milliseconds - timer->milliseconds);
    if (!cumulative) {
        timer->seconds = now.seconds;
        timer->milliseconds = now.milliseconds;
    }
    return elapsed;
}

int32_t AJ_GetTimeDifference(AJ_Time* timerA, AJ_Time* timerB)
{
    int32_t diff;

    diff = (1000 * (timerA->seconds - timerB->seconds)) + (timerA->milliseconds - timerB->milliseconds);
    return diff;
}

void AJ_TimeAddOffset(AJ_Time* timerA, uint32_t msec)
{
    uint32_t msecNew;
    if (msec == -1) {
        timerA->seconds = -1;
        timerA->milliseconds = -1;
    } else {
        msecNew = (timerA->milliseconds + msec);
        timerA->seconds = timerA->seconds + (msecNew / 1000);
        timerA->milliseconds = msecNew % 1000;
    }
}

int8_t AJ_CompareTime(AJ_Time timerA, AJ_Time timerB)
{
    if (timerA.seconds == timerB.seconds) {
        if (timerA.milliseconds == timerB.milliseconds) {
            return 0;
        } else if (timerA.milliseconds > timerB.milliseconds) {
            return 1;
        } else {
            return -1;
        }
    } else if (timerA.seconds > timerB.seconds) {
        return 1;
    } else {
        return -1;
    }
}
int mallocedSize = 0;
void* AJ_Malloc(size_t sz)
{
    char* mem = malloc(sz+12);
    
    if (!mem) {
        AJ_Printf("!!!Failed to malloc %d bytes\n", (int)sz);
        AJ_ASSERT(mem != NULL);
    }
    char* isOccupied = (char*)mem + 3;
    *isOccupied = 1;

    char* sizeNum = (char*)mem + 4;
    sizeNum[0] = (sz >> 0) & 0xff;
    sizeNum[1] = (sz >> 8) & 0xff;
    sizeNum[2] = (sz >> 16) & 0xff;
    sizeNum[3] = (sz >> 24) & 0xff;

    char* magicNum = (char*)mem + 8 + sz;
    magicNum[0] = 0x12;
    magicNum[1] = 0x34;
    magicNum[2] = 0x56;
    magicNum[3] = 0x78;

#ifdef DEBUG_MALLOC
    {
        int i = 0;
        for(i=0 ; i<sz+12 ; i++)
        {
            printf("%x, ", mem[i]);
        }
        printf("\n");
    }
    printf("m:%d\n", mallocedSize);
#endif
    mallocedSize += (sz+12);
    //printf("m:%d,%x,%d\n", sz, mem, mallocedSize);
    
    return mem + 8;
}

void* AJ_Realloc(void* ptr, size_t size)
{
    char* mem = AJ_Malloc(size);

    if(ptr)
    {
        size_t prevSize = *(size_t *)(ptr-4);
        size_t copySize = (size > prevSize) ? prevSize : size;
        memcpy(mem, ptr, copySize);
        AJ_Free(ptr);
    }
    return mem;
}

void AJ_Free(void* mem)
{
    AJ_ASSERT(mem);
    mem -= 8;
    #ifdef DEBUG_MALLOC
    printf("f:%x\n", mem);
    #endif
    char* isOccupied = (char*)mem + 3;
    AJ_ASSERT(*isOccupied == 1);   /* what we free must be not free-ed before */
    *isOccupied = 0;

    unsigned char* sizeNum = (unsigned char*)mem + 4;
    unsigned int sz = 0;
    sz |= sizeNum[0] << 0;
    sz |= sizeNum[1] << 8;
    sz |= sizeNum[2] << 16;
    sz |= sizeNum[3] << 24;

    unsigned char* magicNum = (unsigned char*)mem + 8 + sz;

    if(!(magicNum[0] == 0x12 && magicNum[1] == 0x34 && magicNum[2] == 0x56 && magicNum[3] == 0x78))
    {
        printf("size:%d, magic: %h %h %h %h\n", sz, magicNum[0], magicNum[1], magicNum[2], magicNum[3]);
        AJ_ASSERT(0);
    }
    /*
    AJ_ASSERT(magicNum[0] == 0x12);
    AJ_ASSERT(magicNum[1] == 0x34);
    AJ_ASSERT(magicNum[2] == 0x56);
    AJ_ASSERT(magicNum[3] == 0x78);
    */
    
    if (mem) {
        free(mem);
    }
    mallocedSize -= (sz+12);
}


uint8_t AJ_StartReadFromStdIn()
{
    /* Not implemented */
    return FALSE;
}

uint8_t AJ_StopReadFromStdIn()
{
    /* Not implemented */
    return FALSE;
}

char* AJ_GetCmdLine(char* buf, size_t num)
{
    (void) buf;
    (void) num;
    /* Not implemented */
    return NULL;
}

void AJ_UART_Initialize(void)
{
#ifdef AJ_SERIAL_CONNECTION
    AJ_SerIOConfig config;
    config.bitrate = 115200;
    config.bits = 8;
    config.parity = 0;
    config.stopBits = 1;

    AJ_SerialIOInit(&config);
#endif
}

void AJ_Reboot(void)
{
/*
    A_UINT32 read_value;

    read_value = SCB_AIRCR;
    read_value &= ~SCB_AIRCR_VECTKEY_MASK;
    read_value |= SCB_AIRCR_VECTKEY(0x05FA);
    read_value |= SCB_AIRCR_SYSRESETREQ_MASK;

    _int_disable();
    SCB_AIRCR = read_value;
 */
    while (1) ;  /* Ensure completion of memory access */
}

/*
 * Turn on Serial Wire Output, so AJ_Printf can output to a terminal window
 */
void AJ_EnablePrintfOverSWO(void)
{
    /*
    if (DBG_Connected()) {
        DBG_SWOEnable(GPIO_ROUTE_SWLOCATION_LOC0);
    }
     */
}

void AJ_TimerTargetInit(void)
{
    // Skip: All-Play share the timer with QP BSP
}

void AJ_InitTimer(AJ_Time* timer)
{
    uint32 t = getSysTime();
    timer->seconds = (uint32_t)(t / 1000);
    timer->milliseconds = (uint16_t)(t % 1000);
}


void AJ_TargetPlatformInit(int bRebootSAM)
{
    (void) bRebootSAM;
}

uint16_t AJ_ByteSwap16(uint16_t x)
{
    return _byteswap_ushort(x);
}

uint32_t AJ_ByteSwap32(uint32_t x)
{
    return _byteswap_ulong(x);
}

uint64_t AJ_ByteSwap64(uint64_t x)
{
    return _byteswap_uint64(x);
}

AJ_Status AJ_IntToString(int32_t val, char* buf, size_t buflen)
{
    AJ_Status status = AJ_OK;
    int c = _snprintf(buf, buflen, "%d", val);
    if (c <= 0 || c > buflen) {
        status = AJ_ERR_RESOURCES;
    }
    return status;
}

AJ_Status AJ_InetToString(uint32_t addr, char* buf, size_t buflen)
{
    AJ_Status status = AJ_OK;
    int c = _snprintf(buf, buflen, "%u.%u.%u.%u", (addr & 0xFF000000) >> 24, (addr & 0x00FF0000) >> 16, (addr & 0x0000FF00) >> 8, (addr & 0x000000FF));
    if (c <= 0 || c > buflen) {
        status = AJ_ERR_RESOURCES;
    }
    return status;
}
