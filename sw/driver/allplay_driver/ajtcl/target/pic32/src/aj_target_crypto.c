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
#include "aj_crypto.h"
#include "aj_util.h"
//#include "em_rtc.h"   /* proably this is a gagco specific things, not exist in pic32 platform. */

static uint8_t seed[16];
static uint8_t key[16];

#ifdef TEMPERATURE_WORKING
static uint8_t RandBit(LDD_TDeviceData* tdd)
{
    uint8_t v = 0;
    int i;

    for (i = 0; i < 5; ++i) {
        uint16_t data;
        TemperatureSensor_StartSingleMeasurement(tdd);
        while (!TemperatureSensor_GetMeasurementCompleteStatus(tdd)) {
            /* Do nothing */
        }
        TemperatureSensor_GetMeasuredValues(tdd, &data);
        v += (data & 1);
    }
    return v >= 2;
}

int GatherBits(uint8_t* buffer, uint32_t len)
{
    int i;
    /*
     * We are going to use the onboard temperature sensor as an entropy source.
     */
    LDD_TDeviceData* tdd = TemperatureSensor_Init(NULL);

    /*
     * Use the default sample group
     */
    TemperatureSensor_SelectSampleGroup(tdd, 0);

    memset(buffer, 0, len);

    for (i = 0; i < len; ++i) {
        int j;
        uint8_t r = 0;
        for (j = 0; j < 8; ++j, r <<= 1) {
            r |= RandBit(tdd);
        }
        buffer[i] = r;
    }

    TemperatureSensor_Deinit(tdd);

#ifdef SHOW_RANDOM_BITS
    for (i = 0; i < len; ++i) {
        int j;
        int r = buffer[i];
        for (j = 0; j < 8; ++j) {
            printf("%c", '0' + (r & 1));
            r >>= 1;
        }
    }
    printf("\n");
#endif

    return 0;
}
#endif
void AJ_RandBytes(uint8_t* rand, uint32_t len)
{
    /*
     * On the first call we need to accumulate entropy
     * for the seed and the key.
     */
    if (seed[0] == 0) {
//        GatherBits(seed, sizeof(seed));
//        GatherBits(key, sizeof(key));
    }
    AJ_AES_Enable(key);
    /*
     * This follows the NIST guidelines for using AES as a PRF
     */
    while (len) {
        uint32_t tmp[4];
        uint32_t sz = min(16, len);

        //tmp[0] = RTC_CounterGet();
        tmp[0] = 0x12345678;
        tmp[1] += 1;
        AJ_AES_ECB_128_ENCRYPT(key, (uint8_t*)tmp, (uint8_t*)tmp);
        AJ_AES_CBC_128_ENCRYPT(key, seed, seed, 16, (uint8_t*)tmp);
        memcpy(rand, seed, sz);
        AJ_AES_CBC_128_ENCRYPT(key, seed, seed, 16, (uint8_t*)tmp);
        len -= sz;
        rand += sz;
    }
    AJ_AES_Disable();
}

