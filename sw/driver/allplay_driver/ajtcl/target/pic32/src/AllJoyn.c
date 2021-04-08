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
#include "aj_util.h"

extern void AJ_UART_Initialize(void);

extern void AJ_Main();

void AllJoyn_Start(unsigned long arg)
{
    AJ_Status status = AJ_OK;

    AJ_UART_Initialize();

    AJ_EnablePrintfOverSWO();

    AJ_Printf("\n******************************************************");
    AJ_Printf("\n                AllJoyn Thin-Client");
    AJ_Printf("\n******************************************************\n");

    if (status == AJ_OK) {
        AJ_Main();
    }
    AJ_Printf("Quitting\n");
    while (TRUE) {
    }
}


