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
//#include "aj_semaphore.h"

//#include "em_device.h" /* not exist in pic32 target */

AJ_Semaphore* AJ_SemaphoreCreate(char* name,
                                 int32_t count)
{
    if (name) {
        AJ_Printf("AJ_SemaphoreCreate(%s,%d)\n", name, count);
    }
    AJ_Semaphore* ret = (AJ_Semaphore*) AJ_Malloc(sizeof(AJ_Semaphore));
    if (ret) {
        ret->count = count;
        ret->name = NULL;
    }
    return ret;
}

void AJ_SemaphoreDestroy(AJ_Semaphore* sem)
{
    if (sem && sem->name) {
        AJ_Printf("AJ_SemaphoreDestroy(%s)\n", sem->name);
    }

    AJ_Free(sem);
}


AJ_Status AJ_SemaphoreWait(AJ_Semaphore* sem)
{
    //INT_Disable();
    return AJ_OK;
}


AJ_Status AJ_SemaphoreWaitTimed(AJ_Semaphore* sem,
                                uint32_t timeout)
{
    return AJ_ERR_UNEXPECTED;
}

AJ_Status AJ_SemaphoreUnlock(AJ_Semaphore* sem)
{
    //INT_Enable();
    return AJ_OK;
}

