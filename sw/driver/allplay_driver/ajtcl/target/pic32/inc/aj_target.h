#ifndef _AJ_TARGET_H
#define _AJ_TARGET_H
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
#include "product.config"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

/*____________________________________________________________________________*/



#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

#ifndef min
#define min(x, y) ((x) < (y) ? (x) : (y))
#endif

#define WORD_ALIGN(x) ((x & 0x3) ? ((x >> 2) + 1) << 2 : x)
#define HOST_IS_LITTLE_ENDIAN  1
#define HOST_IS_BIG_ENDIAN     0

void Assert_func(const char* pFile, int line);
#define tp_assert(exp) {\
    {if (exp) {;} else { int line = __LINE__; const char* pFile= __FILE__;\
        while(1){ printf("%s:%d\n", pFile, line); }\
        Assert_func(pFile, line); \
    }\
    }\
}\
\

/* back to printf for demo*/
#ifndef Q_SPY
#if (!defined NDEBUG) && (defined ALLPLAYSRV_DEBUG)
#define AJ_Printf printf
#else
#define AJ_Printf(...)
#endif
#else
#define AJ_Printf(...)
#endif
#define AJ_ASSERT(x) tp_assert(x)

/**
 * Reboot the MCU
 */
void AJ_Reboot(void);

#define AJ_EXPORT

#ifdef AJ_SERIAL_CONNECTION
#define AJ_SERIAL_USART           USART1
#define AJ_SERIAL_USART_CLK       cmuClock_USART1
#define AJ_SERIAL_USART_LOCATION  USART_ROUTE_LOCATION_LOC2
#define AJ_SERIAL_USART_TXPORT    gpioPortD
#define AJ_SERIAL_USART_TXPIN     7
#define AJ_SERIAL_USART_RXPORT    gpioPortD
#define AJ_SERIAL_USART_RXPIN     6
#endif

#define AJ_CreateNewGUID AJ_RandBytes

#define AJ_GetDebugTime(x) AJ_ERR_RESOURCES

typedef struct _AJ_Semaphore {
    int32_t count;
    char* name;
} AJ_Semaphore;

void AJ_TimerTargetInit(void);

void AJ_TargetPlatformInit(int bRebootSAM);

/* add the following prototypes for easier console functionality */
void initConsole(void);
void resetConsole(void);
int kbhitConsole(void);
int getchConsole(void);

#endif
