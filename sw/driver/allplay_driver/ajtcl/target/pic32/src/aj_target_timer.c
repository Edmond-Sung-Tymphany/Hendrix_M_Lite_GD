/**
 * @file
 */
/******************************************************************************
 * Copyright 2013, Qualcomm Innovation Center, Inc.
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

#include "aj_target.h"
#include "aj_status.h"
#include "aj_util.h"
#include <signal.h>
#include <time.h>

void _AJ_DebugCheckTimerList(AJ_Timer* list)
{
}


void _AJ_DumpTimerList(AJ_Timer* list)
{
}


extern void AJ_GlobalTimerHandler(void)
{
}


AJ_Timer* AJ_TimerInit(uint32_t timeout,
                       AJ_TimerCallback timerCallback,
                       void* context,
                       uint32_t timerId)
{
    return AJ_OK;
}



void AJ_TimerInsertInList(AJ_Timer** list, AJ_Timer* newNode)
{

}

AJ_Timer* AJ_TimerRemoveFromList(AJ_Timer** list, uint32_t timerId)
{
    return NULL;
}


AJ_Status AJ_TimerRegister(uint32_t timeout,
                           AJ_TimerCallback timerCallback,
                           void* context,
                           uint32_t* timerId)
{
    return AJ_OK;
}

AJ_Status AJ_TimerRefresh(uint32_t timerId,
                          uint32_t timeout)
{
    return AJ_OK;
}


void AJ_TimerCancel(uint32_t timerId, uint8_t keep)
{
}
