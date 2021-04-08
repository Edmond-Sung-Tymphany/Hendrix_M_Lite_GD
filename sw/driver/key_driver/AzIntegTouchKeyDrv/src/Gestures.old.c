/******************************************************************************
 *                                                                             *
 *                                                                             *
 *                                Copyright by                                 *
 *                                                                             *
 *                              Azoteq (Pty) Ltd                               *
 *                          Republic of South Africa                           *
 *                                                                             *
 *                           Tel: +27(0)21 863 0033                            *
 *                          E-mail: info@azoteq.com                            *
 *                                                                             *
 *=============================================================================*
 * @file     Gestures.c                                                        *
 * @brief    Implementation of the Gestures for touch driver. All of           *
 *           the processing of the Gestures are done here                      *
 * @author   AJ van der Merwe - Azoteq (PTY) Ltd                               *
 * @version  V1.0.0                                                            *
 * @date     01/09/2015                                                        *
 *******************************************************************************/

// C includes
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// User includes
#include "deviceTypes.h"
#include "Gestures.h"
#include "IQS_Commands.h"
#include "tch_defines.h"
#include "IQS333.h"
#include "IQS5xx.h"

// Tymphany platform
#include "KeySrv.h"
#include "timer.h"
static tTimerInfo timerInfo;
static uint16 timerId;
static void tapTimerCallBack(void * pCbPara);
static void doubleTapAnalyse();

/* Private Defines */
#define NR_OF_CHANNELS  (uint8_t)(NR_OF_ACTIVE_CHANNELS-1)     // Used to determine the size, take into account Prox CH
#define LTA_OFFSET      (uint8_t)(NR_OF_ACTIVE_CHANNELS*2)     // Get an LTA Buffer offset
#define PI              (float)3.14

uint16_t type;

Channel_t Channel[NR_OF_ACTIVE_CHANNELS]; // We have an array of NR_OF_ACTIVE_CHANNELS (5 counting prox) to account for prox which is discarded

/* Buffers for the Channels */
uint16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] = { { 0 } }; // history Buffer of all the channels
uint16_t Delta_buffer[NR_OF_CHANNELS][INPUT_BUFFER_SAMPLES] = { { 0 } }; // Input Buffer of all the channel deltas

/* Remapped data */
uint16_t Position[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] = { { 0 } }; // History is assigned to the Position (remapped) of the Channels

/* Set Hand Position State */
//Hand_Position_t hand_position;
Quarts_t hand_position;

/* Function Pointers */
void (*forward)(void);
void (*backward)(void);
void (*tap)(void);

/* Private Function Prototypes */
void toggle_forward(void);
void toggle_backward(void);
void toggle_tap(void);

/**
 * @brief    Init the Gesture Engine, such as buffers etc
 * @param    None
 * @retval    None
 */
void GS_init(void)
{
    /* Get the Funtion Pointer to the functions that need executing at gestures */
    forward = &toggle_forward;
    backward = &toggle_backward;
    tap = &toggle_tap;
}

/**
 * @brief    Add channel data (CS and LTA) from IQS333 to delta buffer and
 *             history buffer for further processing
 * @param    [uint8_t*] Pointer to buffer where data is located
 * @retval    [uint8_t] Success or not
 */
uint8_t add_channel_data(uint8_t* buffer)
{
    uint8_t status = RETURN_OK;
    uint8_t i = 0;
    uint16_t tempCS = 0; // Keep the temp of the CS
    uint16_t tempLTA = 0; // Keep the temp of the LTA

#ifdef ATOUCH_DUMP_IQS333_DATA
    ATOUCH_PRINTF("IQS333 ChA: ");
#endif    

    // loop through all the channels
    for(i = 0; i < NR_OF_ACTIVE_CHANNELS; i++)
    {
        // Set channel Counts data
        tempCS = (uint16_t)((uint16_t)(0xFF00&((uint16_t)(buffer[(i*2) + 1])<<8)) + buffer[(i*2)]); // Build the 16 bit number (LSB then MSB)
        Channel[i].CS = tempCS;

        // Set channel Counts data
        tempLTA = (uint16_t)((uint16_t)(0xFF00&((uint16_t)(buffer[(i*2) + LTA_OFFSET + 1])<<8)) + buffer[(i*2) + LTA_OFFSET]); // Build the 16 bit number (LSB then MSB)
        Channel[i].LTA = tempLTA;
        
#ifdef ATOUCH_DUMP_IQS333_DATA
        ATOUCH_PRINTF("(%d,%d), ", Channel[i].CS, Channel[i].LTA);
#endif        
    }

#ifdef ATOUCH_DUMP_IQS333_DATA
    ATOUCH_PRINTF("\r\n");
#endif    
    return status;
}

/**
 * @brief    Calculate Delta (LTA - CS) for each channel and add to
 *             the delta buffer. If Delta < 0, add NAN to the list
 * @param    None
 * @retval    [uint8_t] Success or not
 */
uint8_t calc_delta(void)
{
    uint8_t status = RETURN_OK;
    uint8_t i = 0;
    uint16_t tempDelta = 0;

    /* Calculate the Delta's - ignore Prox channel [0] */
    for (i = 1; i < NR_OF_ACTIVE_CHANNELS; i++) // Need to iterate over all channels
            {
        /* Remove oldest item and set that value to 0 */
        move_out_queue(&Delta_buffer[i - 1][0], INPUT_BUFFER_SAMPLES);

        // Delta > 0, so NAN
        if (Channel[i].LTA <= Channel[i].CS)
        {
            tempDelta = NAN;
        }
        // Valid Delta
        else {
            tempDelta = (Channel[i].LTA - Channel[i].CS);
        }

        /* Add to back of queue */
        add_to_queue(&Delta_buffer[i - 1][0], tempDelta); // add this delta to the first element
    }

    /* Now call the Average value */
    calc_average();

    return status;
}

/**
 * @brief    Calculate the average of the input buffer (delta buffer) and then
 *             add this value to the history buffer for further processing
 * @param    None
 * @retval    [uint8_t] Success or not
 */
uint8_t calc_average(void)
{
    uint8_t status = RETURN_OK;
    uint8_t i, j;
    uint16_t cumSum = 0; // create large variable to keep the total value of each channel

#ifdef ATOUCH_DUMP_IQS333_DATA
    ATOUCH_PRINTF("IQS333 AVG: ");
#endif    
    /* Iterate over all channels and calculate the average of each channel delta buffer */
    for (j = 0; j < NR_OF_CHANNELS; j++)
    {
        /* Remove oldest item and set that value to 0 */
        move_out_queue(&History_Buffer[j][0], HISTORY_BUFFER_SAMPLES);

        cumSum = 0; // clear cumSum

        /* Now Iterate over the whole delta buffer for each channel */
        for (i = 0; i < INPUT_BUFFER_SAMPLES; i++)
        {
            cumSum += Delta_buffer[j][i]; // CH sum
        }

        History_Buffer[j][0] = (uint16_t)(cumSum / INPUT_BUFFER_SAMPLES); // get the average and add to first element of History Buffer

        /* If the Average value is lower than the Filter value, make it a Nan */
        if (History_Buffer[j][0] < FILTER_VALUE)
        {
            History_Buffer[j][0] = NAN; // Replace this with a NaN
        }

        //Tymphany: History_Buffer[j][0] is a culcumated delta value
        // TODO Debug
        if (History_Buffer[j][0] > 0) {
            //__asm__("NOP");
        }

        #ifdef ATOUCH_DUMP_IQS333_DATA
        ATOUCH_PRINTF("(%d/%d/%d), ", Channel[j+1].CS, Channel[j+1].LTA, History_Buffer[j][0]);
        #endif

    }

    #ifdef ATOUCH_DUMP_IQS333_DATA
    ATOUCH_PRINTF("\r\n");
    #endif

    return status;
}

/**
 * @brief    Remap the Channels to the "Software" channels declared in the
 *             algorithm description document. This involves copying the History_Buffer
 *             to the correct Position
 * @param    None
 * @retval    [uint8_t] Success or not
 */
uint8_t remap_data(void)
{
    uint8_t status = RETURN_OK;

    // Copy (remap) History Buffers - from here on, 'Position' will be used instead of History Buffer
    // We have to this manually
//    copy_data(&Position[0][0], &History_Buffer[0][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[1][0], &History_Buffer[1][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[2][0], &History_Buffer[3][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[3][0], &History_Buffer[2][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));

//    copy_data(&Position[0][0], &History_Buffer[3][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[1][0], &History_Buffer[2][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[2][0], &History_Buffer[0][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
//    copy_data(&Position[3][0], &History_Buffer[1][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));

    copy_data(&Position[0][0], &History_Buffer[0][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
    copy_data(&Position[1][0], &History_Buffer[1][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
    copy_data(&Position[2][0], &History_Buffer[2][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));
    copy_data(&Position[3][0], &History_Buffer[3][0], (sizeof(uint16_t)*HISTORY_BUFFER_SAMPLES));

    // Done remapping
    return status;
}

/**
 * @brief    Calculate the Position of the hand. This is a very important
 *             Step
 * @param    None
 * @retval    None
 */
void calc_hand_position(uint8_t T) 
{
    int8_t j;
    //uint16_t maxV = 0;
    int8_t maxCh, secondHighAtMaxCh;
    uint16_t Position_Sum[NR_OF_CHANNELS] = { 0 }; // This is the sum of the channel delta history - this used to determine hand position
//    uint16_t Copy_Pos_Sum[NR_OF_CHANNELS] = {0};    // This is a copy of Position_sum, but we do not want to change position sum
//    uint8_t qIndex[NR_OF_CHANNELS] = {0, 1, 2, 3};    // This contains the channels - will be ordered from large to small, or small to large
//    Semi_t Semi[NR_OF_CHANNELS];    // This contains the halves (channel Pairs) - will be ordered from large to small, or small to large
    Key_Value_t Maximums[NR_OF_CHANNELS] = { 0 };
    uint8_t segments[NR_OF_CHANNELS] = { 0, 1, 2, 3 };

    uint16_t Total_P = 0;
    uint16_t tempMax = 0;

    uint16_t L0 = 0;
    uint16_t L1 = 0;
    uint16_t L2 = 0;
    uint16_t L3 = 0;

    uint16_t a0 = 0;
    uint16_t a1 = 0;
    uint16_t a2 = 0;
    uint16_t a3 = 0;
    uint8_t aH = 0;
    uint8_t aL = 0;
    //uint8_t a_win = 0;  //unuse variable on original sample code

    uint16_t b0 = 0;
    uint16_t b1 = 0;
    uint16_t b2 = 0;
    uint16_t b3 = 0;
    //uint8_t bb = 0;  //unuse variable on original sample code
    //uint8_t b_win = 0;  //unuse variable on original sample code

    //uint16_t c0 = 0;  //unuse variable on original sample code
    //uint16_t c1 = 0;  //unuse variable on original sample code
    //uint16_t c2 = 0;  //unuse variable on original sample code
    //uint16_t c3 = 0;  //unuse variable on original sample code
    //uint8_t cc = 0;  //unuse variable on original sample code
    //uint8_t c_win = 0;  //unuse variable on original sample code

    uint8_t back_votes = 0;
    uint8_t forw_votes = 0;

    //uint8_t small = 10; // what is clasified small
    uint8_t v_small = 3;

    type = 0;

    // debug
//    uint16_t sum = 0;

    maxCh = 0;
    /* Loop over the channels and calculate the sum */
    for (j = 0; j < NR_OF_CHANNELS; j++)
    {
        Position_Sum[j] = sum_buffer(&Position[j][0], HISTORY_BUFFER_SAMPLES);
//        sum += Position_Sum[j];    // add
        Maximums[j] = max_value_of_channel(&Position[j][0], HISTORY_BUFFER_SAMPLES);    // not efficient running thru the same loop twice, but, this is just testing to see if the concept works
        Maximums[j].Order = j;
        Maximums[j].Channel = j;
        // the maximum for the current channel is now found so compare it to the previous to see if it is the peak channel
        if (Maximums[j].Value > Maximums[maxCh].Value )
        {
            maxCh = j;
        }
    }
    
    //
    // now just update the orders with the first peak at 0 then the last peak at 3
    // send array and just update the rank parameters
    order_maximums(&Maximums[0]);
    resort_by_channel(&Maximums[0]);
    

// John's method
    // since we have the max value, now find the next highest value at the index of the max channel
    /* Loop over the channels and calculate the sum */
    secondHighAtMaxCh = 0;
    tempMax = 0;
    for (j = 0; j < NR_OF_CHANNELS; j++)
    {
        if (j == maxCh) // just bounce over the max channel because we are not intrested in this one
            continue;
        // find the channel which is the next highest where the peak is
        if ((Position[j][Maximums[maxCh].Key] > tempMax))
        {
            secondHighAtMaxCh = j;
            tempMax = Position[j][Maximums[maxCh].Key];
        }
    }
    

    // now calculate the weights of all the channels - will be used to see what channels are small or large and rotate to the swipe directions axis
    switch(T)
    {
    case 0: //SWIPE_UP
        segments[0] = 0;
        segments[1] = 1;
        segments[2] = 2;
        segments[3] = 3;

        L0 = Position_Sum[0];
        L1 = Position_Sum[1];
        L2 = Position_Sum[2];
        L3 = Position_Sum[3];
        Total_P = L0 + L1 + L2 + L3;

        L0 = L0 * 100;
        L1 = L1 * 100;
        L2 = L2 * 100;
        L3 = L3 * 100;

        L0 = L0 / Total_P;
        L1 = L1 / Total_P;
        L2 = L2 / Total_P;
        L3 = L3 / Total_P;

        a0 = L0;
        a1 = L1;
        a2 = L2;
        a3 = L3;
        aH = get_max_value(a0, a1, a2, a3);
        aL = get_min_value(a0, a1, a2, a3);
        b0 = L0 + L1;
        b1 = L1 + L2;
        b2 = L2 + L3;
        b3 = L3 + L0;
        break;
    case 3: //SWIPE_RIGHT
        segments[0] = 3;
        segments[1] = 0;
        segments[2] = 1;
        segments[3] = 2;
        L0 = Position_Sum[3];
        L1 = Position_Sum[0];
        L2 = Position_Sum[1];
        L3 = Position_Sum[2];
        Total_P = L0 + L1 + L2 + L3;

        L0 = L0 * 100;
        L1 = L1 * 100;
        L2 = L2 * 100;
        L3 = L3 * 100;

        L0 = L0 / Total_P;
        L1 = L1 / Total_P;
        L2 = L2 / Total_P;
        L3 = L3 / Total_P;

        a0 = L0;
        a1 = L1;
        a2 = L2;
        a3 = L3;
        aH = get_max_value(a0, a1, a2, a3);
        aL = get_min_value(a0, a1, a2, a3);
        b0 = L1 + L2;
        b1 = L2 + L3;
        b2 = L3 + L0;
        b3 = L0 + L1;
        break;
    case 2: //SWIPE_DOWN
        segments[0] = 2;
        segments[1] = 3;
        segments[2] = 0;
        segments[3] = 1;
        L0 = Position_Sum[2];
        L1 = Position_Sum[3];
        L2 = Position_Sum[0];
        L3 = Position_Sum[1];
        Total_P = L0 + L1 + L2 + L3;

        L0 = L0 * 100;
        L1 = L1 * 100;
        L2 = L2 * 100;
        L3 = L3 * 100;

        L0 = L0 / Total_P;
        L1 = L1 / Total_P;
        L2 = L2 / Total_P;
        L3 = L3 / Total_P;

        a0 = L0;
        a1 = L1;
        a2 = L2;
        a3 = L3;
        aH = get_max_value(a0, a1, a2, a3);
        aL = get_min_value(a0, a1, a2, a3);
        b0 = L2 + L3;
        b1 = L3 + L0;
        b2 = L0 + L1;
        b3 = L1 + L2;
        break;
    case 1: //SWIPE_LEFT
        segments[0] = 1;
        segments[1] = 2;
        segments[2] = 3;
        segments[3] = 0;
        L0 = Position_Sum[1];
        L1 = Position_Sum[2];
        L2 = Position_Sum[3];
        L3 = Position_Sum[0];
        Total_P = L0 + L1 + L2 + L3;

        L0 = L0 * 100;
        L1 = L1 * 100;
        L2 = L2 * 100;
        L3 = L3 * 100;

        L0 = L0 / Total_P;
        L1 = L1 / Total_P;
        L2 = L2 / Total_P;
        L3 = L3 / Total_P;

        a0 = L0;
        a1 = L1;
        a2 = L2;
        a3 = L3;
        aH = get_max_value(a0, a1, a2, a3);
        aL = get_min_value(a0, a1, a2, a3);
        b0 = L3 + L0;
        b1 = L0 + L1;
        b2 = L1 + L2;
        b3 = L2 + L3;
        break;
    }    

    // all the state logic is defendant on the swipe gesture received from the IQS572
    /* Interpret the IQS5xx Gesture */
    // type 1 1 dominate channel with 2 channels at very small if not zero values
    // this is a primary level  since it is the most clear cut desision
    if ((a0 == 0) && (a1 == 0) && (a2 == 0) && (a3 == 0))
    {
        forw_votes++;
        type = 0x0F;
    }
    else if (((maxCh == segments[2]) || (maxCh == segments[3]) ) && (a0 < v_small) && (a1 < v_small))// and p1 and p0 are small
    {
        forw_votes++;
        type = 0x1F;
    }
    else if (((maxCh == segments[1]) || (maxCh == segments[0]) ) && (a2 < v_small) && (a3 < v_small))// and p1 and p0 are small
    {
        back_votes++;
        type = 0x1B;
    }
    else
    {
        // type 2 swipe --> has to be clear cut
        if ((maxCh == segments[2]) && (secondHighAtMaxCh == segments[3]))
        {
            forw_votes++;
            type = 0x2;
        }
        else if ((maxCh == segments[3]) && (secondHighAtMaxCh == segments[2]))
        {
            forw_votes++;
            type = 0x2;
        }
        else if ((maxCh == segments[0]) && (secondHighAtMaxCh == segments[1]))
        {
            back_votes++;
            type = 0x2;
        }
        else if ((maxCh == segments[1]) && (secondHighAtMaxCh == segments[0]))
        {
            back_votes++;
            type = 0x2;
        }

        // type 3 / 6 / 5LH swipe
        if (((maxCh == segments[1]) && (secondHighAtMaxCh == segments[2])) || ((maxCh == segments[2]) && (secondHighAtMaxCh == segments[1])))
        {
            if (a3 > a0) // type a
                    {
                // now decide if it was an edge case bw/fw ie where is the highest weight
                if (a0 == 0)
                {
                    forw_votes++;
                    type = 0x6A;
                }
                else
                {    // what if a1 is huge? the B weights are all biased to wards A1 side
//                        forw_votes++;
//                    }
//                    // type 3 :
//                    else if (b0 > b2) {
//                        back_votes++;
//                    } else {
                        forw_votes++;
//                    }
                    type = 0x3A;
                }
            }
            else if (a0 > a3)     // type b
                    {
                // now decide if it was an edge case bw/fw ie where is the highest weight
                if (a3 < v_small)
                {
                    if (aH == segments[2])
                    {
                        back_votes++;
                        type = 0x6B;
                    }
                    else
                    {
                        forw_votes++;
                        type = 0x6A;
                    }
                }
                else if (a0 > a3) // is it a left hand?
                        {
                    back_votes++;
                    type = 0x3B;
                }
                else
                {
                    if (b2 > b0)
                    {
                        forw_votes++;
                    }
                    // type 3 :
                    else if (b0 > b2)
                    {
                        back_votes++;
                    }
                    else
                    {
                        back_votes++;
                    }
                    type = 0x3;
                }
            }
//            else {
//                // ok a0 and a3 are now equal -> look more deeply
//                type = 0x3;
//            }
        }
        
        // maxch 2 2nd max 3 = type 4/6 ::: maxCh = 2, 2nd = 0, tot = 1094, (a0,a1,a2,a3) = (18,1,59,21)
//            if (((maxCh == 2) && (secondHighAtMaxCh == 1)) || ((maxCh == 3) && (secondHighAtMaxCh == 2)))
//            {
//
//            }
        // BW swipe : t = 0 0, maxCh = 0, 2nd = 0, tot = 688, (a0,a1,a2,a3) = (50,0,40,8)
        // FW swipe : t = 40 1, maxCh = 0, 2nd = 0, tot = 1628, (a0,a1,a2,a3) = (40,21,3,34)
        if (maxCh == segments[0])
        {
            // look which is smaller a0,a1,a2,a3
            if (aL == segments[1]) // RH, type 4a
                    {
                forw_votes++;
                type = 0x40;
            }
            else if (aL == segments[3])                // LH, type 4b
                    {
                back_votes++;
                type = 0x40;
            }

            if (a1 < a2) // type 4a
                    {
                // now look to see which si bigger a0 or a3
                if (a1 > a3)
                {
                    back_votes++;
                    type = 0x40;
                }
                else if (a3 > a1)
                {
                    forw_votes++;
                    type = 0x40;
                }
                else
                {
                    // look which was max when a0 was max
                    if (secondHighAtMaxCh == segments[0])
                    {
                        back_votes++;
                        type = 0x40;
                    }
                    else if (secondHighAtMaxCh == segments[3])
                    {
                        forw_votes++;
                        type = 0x40;
                    }
                    else
                    {
                        // unknown assume forware
                        forw_votes++;
                        type = 0x40;
                    }
                }
            }
            else    // type 4b
            {
                // a2 is now the smallest
                if (aL == segments[2]) // LH, type 5b
                        {
                    back_votes++;
                    type = 0x5B0;
                }
                else if (aL == segments[0])                // LH, type 4b
                        {
                    back_votes++;
                    type = 0x5B0;
                }
            }
        }

//4a
        //BW swipe : t = 0 0, maxCh = 3, 2nd = 0, tot = 1627, (a0,a1,a2,a3) = (7,0,14,37)
        if ((maxCh == segments[3]) && (secondHighAtMaxCh == segments[0]))
        {
            if (a2 > a1)
            {
                forw_votes++;
                type = 0x4A3;
            }
            else if (a1 > a2)
            {
                back_votes++;
                type = 0x4A3;
            }
            else    // look at the 1/2 weights
            {
                forw_votes++;
                type = 0x4A9;
            }
        }
        
//4b
        //BW swipe : t = 0 0, maxCh = 3, 2nd = 0, tot = 1627, (a0,a1,a2,a3) = (7,0,14,37)
        if ((maxCh == segments[1]) && (secondHighAtMaxCh == segments[0]))
        {
            if (a1 > a2)
            {
                back_votes++;
                type = 0x4B3;
            }
            else if (a2 > a1)
            {
                forw_votes++;
                type = 0x4B3;
            }
            else    // look at the 1/2 weights
            {
                back_votes++;
                type = 0x4B9;
            }
        }

        // type 6
        if (maxCh == segments[3])
        {
            // look which is smaller a0,a1,a2,a3
            if (aL == segments[0]) // RH, type 5a
                    {
                forw_votes++;
                type = 0x63;
            }
            else if (aL == segments[2])                // LH, type 5b
                    {
                back_votes++;
                type = 0x63;
            } 
        }
    }

    if (type == 0)
    {
        type = 0x9E;
    }

    // Announce the winner!!
    if (forw_votes >= back_votes) 
    {
        forward();
        ATOUCH_PRINTF("FW swipe %d : t = %x %x|%x, maxCh = %x, 2nd = %x, tot = %d, (a%d,a%d,a%d,a%d) = (%d,%d,%d,%d) (%d,%d,%d,%d)\r\n",
                T, type, forw_votes, back_votes, maxCh, secondHighAtMaxCh,
                Total_P, segments[0], segments[1], segments[2], segments[3], a0,
                a1, a2, a3, Maximums[0].Order, Maximums[1].Order,
                Maximums[2].Order, Maximums[3].Order);
    } 
    else 
    {
        backward();
        ATOUCH_PRINTF("BW swipe %d : t = %x %x|%x, maxCh = %x, 2nd = %x, tot = %d, (a%d,a%d,a%d,a%d) = (%d,%d,%d,%d) (%d,%d,%d,%d)\r\n",
                T, type, forw_votes, back_votes, maxCh, secondHighAtMaxCh,
                Total_P, segments[0], segments[1], segments[2], segments[3], a0,
                a1, a2, a3, Maximums[0].Order, Maximums[1].Order,
                Maximums[2].Order, Maximums[3].Order);
    }
////////////////////////////////////

    /*  original method
     L0 = Position_Sum[0];
     L1 = Position_Sum[1];
     L2 = Position_Sum[2];
     L3 = Position_Sum[3];

     Total_P = L0 + L1 + L2 + L3;

     L0 = L0 * 100;
     L1 = L1 * 100;
     L2 = L2 * 100;
     L3 = L3 * 100;

     L0 = L0 / Total_P;
     L1 = L1 / Total_P;
     L2 = L2 / Total_P;
     L3 = L3 / Total_P;

     a0 = L0;
     a1 = L1;
     a2 = L2;
     a3 = L3;

     b0 = L0 + L1;
     b1 = L1 + L2;
     b2 = L2 + L3;
     b3 = L3 + L0;

     c0 = L1 + L2 + L3;
     c1 = L2 + L3 + L0;
     c2 = L3 + L0 + L1;
     c3 = L0 + L1 + L2;

     aa = get_max_value(a0, a1, a2, a3);
     idxa = get_idx_at_max_value(&Position[aa][0], HISTORY_BUFFER_SAMPLES);                    // aa is the channel which has the max value
     bb = get_max_value(b0, b1, b2, b3);
     cc = get_max_value(c0, c1, c2, c3);

     // Group A

     if (aa == 0) {
     if ((T == 0) || (T == 3)) {
     a_win = 0; // backwards
     }
     else if ((T == 1) || (T == 2)) {
     a_win = 1; // forwards
     }
     }
     if (aa == 1) {
     if ((T == 0) || (T == 1)) {
     a_win = 0; // backwards
     }
     else if ((T == 2) || (T == 3)) {
     a_win = 1; // forwards
     }
     }
     if (aa == 2) {
     if ((T == 1) || (T == 2)) {
     a_win = 0; // backwards
     }
     else if ((T == 0) || (T = 3)) {
     a_win = 1; // forwards
     }
     }
     if (aa == 3) {
     if ((T == 2) || (T == 3)) {
     a_win = 0; // backwards
     }
     else if ((T == 1) || (T == 0)) {
     a_win = 1; // forwards
     }
     }


     // Group B
     if (bb == 0) {
     if ((T == 0)) {
     b_win = 0; // backwards
     }
     else if ((T == 2)) {
     b_win = 1; // forwards
     }
     }
     if (bb == 1) {
     if ((T == 1)) {
     b_win = 0; // backwards
     }
     else if ((T == 3)) {
     b_win = 1; // forwards
     }
     }
     if (bb == 2) {
     if ((T == 2)) {
     b_win = 0; // backwards
     }
     else if ((T == 0)) {
     b_win = 1; // forwards
     }
     }
     if (bb == 3) {
     if ((T == 3)) {
     b_win = 0; // backwards
     }
     else if ((T == 1)) {
     b_win = 1; // forwards
     }
     }

     // Group C
     if (cc == 0) {
     if ((T == 0) || (T == 3)) {
     c_win = 0; // backwards
     }
     else if ((T == 1) || (T == 2)) {
     c_win = 1; // forwards
     }
     }
     if (cc == 1) {
     if ((T == 0) || (T == 1)) {
     c_win = 0; // backwards
     }
     else if ((T == 2) || (T == 3)) {
     c_win = 1; // forwards
     }
     }
     if (cc == 2) {
     if ((T == 1) || (T == 2)) {
     c_win = 0; // backwards
     }
     else if ((T == 0) || (T = 3)) {
     c_win = 1; // forwards
     }
     }
     if (cc == 3) {
     if ((T == 2) || (T == 3)) {
     c_win = 0; // backwards
     }
     else if ((T == 1) || (T == 3)) {
     c_win = 1; // forwards
     }
     }


     // count votes!

     if (a_win){
     forw_votes++;
     }
     else {
     back_votes++;
     }

     //    if (b_win) {
     //        forw_votes++;
     //    }
     //    else {
     //        back_votes++;
     //    }

     //    if (c_win) {
     //        forw_votes++;
     //    }
     //    else {
     //        back_votes++;
     //    }


     // Announce the winner!!
     if (forw_votes >= back_votes){
     forward();
     }
     else{
     backward();
     }
     */

    /* Copy Position sum, so that we can arrange a none important array,
     * then we can still use the original Position_Sum
     */
//    copy_data(&Copy_Pos_Sum[0], &Position_Sum[0], (sizeof(uint16_t)*NR_OF_CHANNELS));
//
//    /* Now Check the largest Channel */
//    arrange_large_small(Copy_Pos_Sum, qIndex);
////    arrange_small_large(Copy_Pos_Sum, qIndex);
//
//    // TODO everything is 0
//    if (sum == 0)
//    {
//        return;
//    }
//
//    // TODO Need to implement channel pairs
//
//    /* Check Largest channel */
//    switch (qIndex[0])
//    {
//        // Hand over P2
//        case 0:
//            __asm__("NOP");// TODO Debug
//            hand_position = P0;
//            break;
//
//        // Hand over P3
//        case 1:
//            __asm__("NOP");// TODO Debug
//            hand_position = P1;
//            break;
//
//        // Hand over P0
//        case 2:
//            __asm__("NOP");// TODO Debug
//            hand_position = P2;
//            break;
//
//        // Hand over P1
//        case 3:
//            __asm__("NOP");// TODO Debug
//            hand_position = P3;
//            break;
//
//        default:
//            break;
//    }
    /* Check smallest channel */
//    switch (qIndex[0])
//    {
//        // Hand over P2
//        case 0:
//            __asm__("NOP");// TODO Debug
//            break;
//
//        // Hand over P3
//        case 1:
//            __asm__("NOP");// TODO Debug
//            break;
//
//        // Hand over P0
//        case 2:
//            __asm__("NOP");// TODO Debug
//            break;
//
//        // Hand over P1
//        case 3:
//             __asm__("NOP");// TODO Debug
//            break;
//
//        default:
//            break;
//    }
}

void resort_by_channel(Key_Value_t* buffer)
{
    uint8_t i, j, n, swap;
    n = 4;
    for (i = 0 ; i < ( n - 1 ); i++)
    {
        for (j = 0 ; j < n - i - 1; j++)
        {
            if (buffer[j].Channel > buffer[j + 1].Channel) /* For decreasing order use < */
            {
                swap = buffer[j].Channel;
                buffer[j].Channel = buffer[j + 1].Channel;
                buffer[j + 1].Channel = swap;
                swap = buffer[j].Key;
                buffer[j].Key = buffer[j + 1].Key;
                buffer[j + 1].Key = swap;
                swap = buffer[j].Value;
                buffer[j].Value = buffer[j + 1].Value;
                buffer[j + 1].Value = swap;
                swap = buffer[j].Order;
                buffer[j].Order = buffer[j + 1].Order;
                buffer[j + 1].Order = swap;
            }
        }
    }
}

void order_maximums(Key_Value_t* buffer)
{
    uint8_t i, j, n, swap;
    n = 4;
    for (i = 0 ; i < ( n - 1 ); i++)
    {
        for (j = 0 ; j < n - i - 1; j++)
        {
            if (buffer[j].Key < buffer[j + 1].Key) /* For decreasing order use < */
            {
                swap = buffer[j].Channel;
                buffer[j].Channel = buffer[j + 1].Channel;
                buffer[j + 1].Channel = swap;
                swap = buffer[j].Key;
                buffer[j].Key = buffer[j + 1].Key;
                buffer[j + 1].Key = swap;
                swap = buffer[j].Order;
                buffer[j].Value = buffer[j + 1].Value;
                buffer[j + 1].Value = swap;
                swap = buffer[j].Value;
            }
        }
    }
    // populate Order now - inefficient but dont have time to think of a nice algorithm and this is an stm32 btw...
    buffer[0].Order = 0;
    buffer[1].Order = 1;
    buffer[2].Order = 2;
    buffer[3].Order = 3;
}

/* Tymphany add */
char* str_evt_num()
{
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR 
    I2C_Device_t *iqs333_cmds = IQS333_Get_Device();
    I2C_Device_t *iqs5xx_cmds = IQS5xx_Get_Device();

    static char buf[100];
    snprintf(buf, sizeof(buf), "- [333]%d/%d(%d%%), [572]%d/%d(%d%%)", 
        iqs333_cmds->miss_evt_num, iqs333_cmds->total_evt_num, iqs333_cmds->miss_evt_num*100/iqs333_cmds->total_evt_num, 
        iqs5xx_cmds->miss_evt_num, iqs5xx_cmds->total_evt_num, iqs5xx_cmds->miss_evt_num*100/iqs5xx_cmds->total_evt_num);
    return buf;
#else
    return "";
#endif
}

int touch_num= 0;
/**
 * @brief    Get the actual Gesture that occured - call the action functions (callbacks) as well
 * @param    [uint8_t*] Pointer to XY_Info read from IQS5xx
 * @retval    None
 */
void get_gesture(uint8_t* xy_info) 
{
    uint8_t gesture_byte = xy_info[1]; // This is where gesture byte is located
    uint8_t T = 5; // Default gesture

    /* Interpret the IQS5xx Gesture */
    switch(gesture_byte)
    {
    // Tap
    case TAP:
        T = 4; //TAP        
        ATOUCH_PRINTF("(TAP) %s\r\n", str_evt_num());
        doubleTapAnalyse();
        break;

        // Tap and Hold - Not used at the moment */
    case TAP_HOLD:
        /* Continue */
        ATOUCH_PRINTF("(TAP_HOLD) %s\r\n", str_evt_num());
        KeySrv_SendKeyEvt_Direct(KEY_EVT_LONG_PRESS, TOUCH_TAP_KEY);
        T = 5; // nothing
        break;
        
     /* Tymphany add */
    //case DOUBLE_TAP:
        //TODO: support double tap
        //KeySrv_SendKeyEvt_Direct(KEY_EVT_DOUBLE_PRESS, TOUCH_TAP_KEY);
    //    break;
        
        // Swipe Left
    case SWIPE_LEFT:
        ATOUCH_PRINTF("(SWIPE_LEFT) %s\r\n", str_evt_num());
        T = 1;
//            // Toggle Backward
//            if((hand_position == P1) || (hand_position == P2))
//            {
//                backward();
//            }
//
//            // Toggle Forward
//            if((hand_position == P0) || (hand_position == P3))
//            {
//                forward();
//            }

        break;

        // Swipe Right
    case SWIPE_RIGHT:
        ATOUCH_PRINTF("(SWIPE_RIGHT) %s\r\n", str_evt_num());

//            // Toggle Backward
//            if((hand_position == P0) || (hand_position == P3))
//            {
//                backward();
//            }
//
//            // Toggle Forward
//            if((hand_position == P1) || (hand_position == P2))
//            {
//                forward();
//            }

        T = 3;
        break;

        // Swipe Up
    case SWIPE_UP:
      ATOUCH_PRINTF("(SWIPE_UP) %s\r\n", str_evt_num());
//            // Toggle Backward
//            if((hand_position == P2) || (hand_position == P3))
//            {
//                backward();
//            }
//
//            // Toggle Forward
//            if((hand_position == P0) || (hand_position == P1))
//            {
//                forward();
//            }

        // Toggle Backward
//            if((hand_position == P0) || (hand_position == P1))
//            {
//                backward();
//            }
//
//            // Toggle Forward
//            if((hand_position == P0) || (hand_position == P3))
//            {
//                forward();
//            }

        T = 0;
        break;

        // Swipe Down
    case SWIPE_DOWN:
        ATOUCH_PRINTF("(SWIPE_DOWN) %s\r\n", str_evt_num());
        // Toggle Backward
//            if((hand_position == P2) || (hand_position == P3))
//            {
//                backward();
//            }
//
//            // Toggle Forward
//            if((hand_position == P0) || (hand_position == P1))
//            {
//                forward();
//            }
        T = 2;
        break;

        /* Not valid gesture */
    default:
        ATOUCH_PRINTF("(default <==ERROR?)\r\n");
        //ASSERT(0);
        //forward();
        T = 3; // default forward
        break;
    }

    /* Now get the hand position */
    /* Now Calculate the Hand Position */
    if (T <= 3) {
        calc_hand_position(T);
    }

    /* Clear buffers */
    memset((void *) History_Buffer, 0, sizeof(History_Buffer));
    memset((void *) Delta_buffer, 0, sizeof(Delta_buffer));
    memset((void *) Position, 0, sizeof(Position));

    /* Now do gesture decision */
//        do_gesture_decision(T)
    // Could not yet implement
//    int16_t deltaX = 0;
//    int16_t deltaY = 0;
//    float angle = 0.0f;
//
//    /* Get the delta Values */
//    deltaX = (int16_t)((0xFF00&(int16_t)((int16_t)(0xFF&xy_info[2])<<8)) | xy_info[3]);
//    deltaY = (int16_t)((0xFF00&(int16_t)((int16_t)(0xFF&xy_info[4])<<8)) | xy_info[5]);
//
//    // let's swop delta Y to be the negative of what we get - this is the wrong way around
//    deltaY = -deltaY;
//
//    /* Now, calulate the gesture angle */
//    angle = calc_angle(deltaX, deltaY);
//
//
//    /* Implement our Own small gesture engine */
//    switch(gesture_byte)
//        {
//            // Tap
//            case TAP:
//                tap();
//                break;
//
//            // Tap and Hold - Not used at the moment */
//            case TAP_HOLD:
//                /* Continue */
//                break;
//
//            // Swipes
//            case SWIPE_RIGHT:
//            case SWIPE_UP:
//            case SWIPE_LEFT:
//            case SWIPE_DOWN:
//
//                /* Implement rotation according to hand position */
//
//
//                // Toggle Backward
//                if(hand_position == P0)
//                {
//                    angle += 135;
//                }
//
//                if(hand_position == P1)
//                {
//                    angle += 45;
//                }
//
//                // Toggle Backward
//                if(hand_position == P2)
//                {
//                    angle -= 45;
//                }
//
//                // Toggle Forward
//                if(hand_position == P3)
//                {
//                    angle -= 135;
//                }
//
//                if(angle < 0)
//                {
//                    angle = 360 + angle;
//                }
//                if(angle > 360)
//                {
//                    angle = angle - 360;
//                }
//
//                /* Now calculate what to do */
//                if (((angle >= 0) && (angle < 90)) || ((angle >= 270) && (angle < 360)))
//                {
//                    forward();
//                }
//                else {
//                    backward();
//                }
//
//                break;
//
//            /* Not valid gesture */
//            default:
//                forward();
//                break;
//
//        }
}


/*****************************************
 * Touch event handler
 *****************************************/
// forward() point to here
void toggle_forward(void) 
{
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_SWIPE_RIGHT_KEY);
}

// backward() point to here
void toggle_backward(void) 
{
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_SWIPE_LEFT_KEY);
}

// tap() point to here
void toggle_tap(void) 
{
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_TAP_KEY);
}


/* Helper Functions -------------------------------------*/

/**
 * @brief    Sum all the values of the buffer, up to a 'NAN' - if a NaN was
 *             found, exit the summation and return the values thus far
 * @param    [uint16_t*] Pointer to the buffer to sum
 * @param    [uint16_t] Size of buffer to sum
 * @retval    [uint16_t] Sum of all buffer elements
 */
uint16_t sum_buffer(uint16_t* buffer, uint8_t size)
{
    uint16_t i;
    uint16_t sum = 0;

    // Loop over buffer and sum
    for (i = 0; i < size; i++)
    {
        // If a NAN is found, break out
        if (buffer[i] == NAN)
        {
            continue;
            //break;    // breaking out sometime ignores the actual peak is swiped slowly
        }

        sum += buffer[i];
    }

    // return the 32 bit sum value
    return sum;
}

/**
 * @brief returns the maximum value for the given channel
 */
Key_Value_t max_value_of_channel(uint16_t* buffer, uint8_t size)
{
    uint16_t i, idx;
    uint16_t max = 0;
    Key_Value_t key_value = { 0 };

    key_value.Key = 0;
    key_value.Value = 0;

    // Loop over buffer and sum
    for (i = 0; i < size; i++)
    {
        if (buffer[i] > max)
        {
            max = buffer[i];
            idx = i;
        }
    }
    key_value.Key = idx;
    key_value.Value = max;

    // return the 32 bit max value
    return key_value;
}

/**
 * @brief    Rearrange the Names of the sectors from largest
 *             to smallest cumulative sum
 * @param    [uint16_t*] Pointer to the array to arrange
 * @param    [uint8_t*] Pointer to the array with channel 'names'
 * @retval    None
 */
void arrange_large_small(uint16_t* channels, uint8_t* names)
{
    uint16_t tempName, tempChannel;
    uint8_t i, j, n = NR_OF_CHANNELS;

    /* Use Bubble sort as elements are a few */
    for (i = 0; i < n-1; i++)
    {
        for (j = 0; j < n-1; j++)
        {
            // We test on channels and swop names
            if (channels[j] < channels[j + 1]) /* For decreasing order use < */
            {
                tempChannel = channels[j];
                channels[j] = channels[j + 1];
                channels[j + 1] = tempChannel;

                tempName = names[j];
                names[j] = names[j + 1];
                names[j + 1] = tempName;
            }
        }
    }

}

/**
 * @brief    Rearrange the Names of the sectors from smallest
 *             to largest cumulative sum
 * @param    [uint16_t*] Pointer to the array to arrange
 * @param    [uint8_t*] Pointer to the array with channel 'names'
 * @retval    None
 */
void arrange_small_large(uint16_t* channels, uint8_t* names)
{
    uint16_t tempName, tempChannel;
    uint8_t i, j, n = NR_OF_CHANNELS;

    /* Use Bubble sort as elements are a few */
    for (i = 0; i < n-1; i++)
    {
        for (j = 0; j < n-1; j++)
        {
            // We test on channels and swop names
            if (channels[j] > channels[j + 1]) /* For decreasing order use < */
            {
                tempChannel = channels[j];
                channels[j] = channels[j + 1];
                channels[j + 1] = tempChannel;

                tempName = names[j];
                names[j] = names[j + 1];
                names[j + 1] = tempName;
            }
        }
    }

}

/**
 * @brief    Add channel data (CS and LTA) from IQS333 to delta buffer and
 *             history buffer for further processing
 * @param    [uint16_t*] Pointer to destination buffer
 * @param    [uint16_t*] Pointer to source buffer
 * @param    [size_t] Size of copy
 * @retval    [uint8_t] Success or not
 */
uint8_t copy_data(uint16_t* dest, uint16_t* source, size_t size)
{
    uint8_t status = RETURN_OK;

    if (memcpy((void *)dest, (const void *)source, size) != NULL)
    {
        status = RETURN_OK;
    }
    else {
        status = ERR_COPYING_DATA;
    }

    return status;
}

/**
 * @brief    Remove the oldest item of the queue - this will move all of
 *             the channels' data, which is fine, because we want a fixed
 *             time reference
 * @param    [Channel_Data_t*] Pointer to buffer to shift and clear
 * @param    [uint8_t] Size of copy
 * @retval    [uint8_t] Success or not
 */
uint8_t move_out_queue(uint16_t* buffer, uint8_t size)
{
    uint8_t status = RETURN_OK;
    uint8_t i;

    /* Iterate Backwards through the array to move elements and remove the oldest
     * element. First in, First out
     */
    for (i = (size-1); i > 0; i--)
    {
        buffer[i] = buffer[i - 1]; // replace each element with the element before it
    }

    // clear last element
    buffer[0] = 0;

    return status;
}

/**
 * @brief    Remove the oldest item of the queue - this will move all of
 *             the channels' data, which is fine, because we want a fixed
 *             time reference
 * @param    [uint16_t*] Pointer to destination buffer
 * @param    [uint16_t] Data to be added
 * @param    [uint8_t] where to add element
 * @retval    [uint8_t] Success or not
 */
uint8_t add_to_queue(uint16_t* buffer, uint16_t data)
{
    uint8_t status = RETURN_OK;

    *buffer = data;

    return status;
}

/**
 * @brief    Get the largest value and return the index
 */
uint8_t get_max_value (uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3)
{
    if ((x0 >= x1) && (x0 >= x2) && (x0 >= x3))
    {
        return 0;
    }
    if ((x1 >= x0) && (x1 >= x2) && (x1 >= x3))
    {
        return 1;
    }
    if ((x2 >= x0) && (x2 >= x1) && (x2 >= x3))
    {
        return 2;
    }
    if ((x3 >= x0) && (x3 >= x1) && (x3 >= x2))
    {
        return 3;
    }

    return 0;
}

/**
 * @brief    Get the largest value and return the index
 */
uint8_t get_min_value (uint16_t x0, uint16_t x1, uint16_t x2, uint16_t x3)
{
    if ((x0 <= x1) && (x0 <= x2) && (x0 <= x3))
    {
        return 0;
    }
    if ((x1 <= x0) && (x1 <= x2) && (x1 <= x3))
    {
        return 1;
    }
    if ((x2 <= x0) && (x2 <= x1) && (x2 <= x3))
    {
        return 2;
    }
    if ((x3 <= x0) && (x3 <= x1) && (x3 <= x2))
    {
        return 3;
    }

    return 0;
}

/**
 *     @brief    Get the index of the maximum value for the given set of data
 */
uint8_t get_idx_at_max_value(uint16_t* buffer, uint8_t size)
{
    uint8_t i;
    uint16_t max = 0;
    uint8_t idx = 0;

    // Loop over buffer and find the max index
    for (i = 0; i < size; i++)
    {
        if (buffer[i] > max)
        {
            max = buffer[i];
            idx = i;
        }
    }

    // return the 32 bit sum value
    return idx;
}

/****************        Math Functions for Calculating Gestures            *********************/

/**
 * @brief    Calculate arctan with series expansion and return the
 *             answer in degrees
 * @param    [float] floating point of deltaX/deltaY or deltaY/deltaX
 * @retval    [float] floating point number that is the Angle in degrees
 */
float arctan(float x)
{
    float answer = 0.0f;

    answer = x / (1 + 0.28125 * (x * x));

    answer = answer * (180 / PI);

    return answer;
}

/**
 * @brief    Calculate the angle of the swipe and return the degrees from 0-360
 *             as a floating point number
 * @param    [int16_t] signed 16-bit delta X
 * @param    [int16_t] signed 16-bit delta X
 * @retval    [float] floating point number that is the Angle in degrees
 */
float calc_angle(int16_t x, int16_t y)
{
    float angle = 0.0f;
    int16_t absX = 0;
    int16_t absY = 0;
    float theta = 0.0f;

    /* Check all the necessary conditions and convert arctan ans */

    /* Get abs value to compare */
    absX = gAbs(x);
    absY = gAbs(y);

    /* If abs of x larger or equal to abs y, then use atan(y/x) */
    if(absX >= absY)
    {

        theta = (float) ((float) y / (float) x);
        // Get the angle in degrees
        angle = arctan(theta);

        /* Angle is negative of x and y negative */
        if((angle <= 0) || ((x < 0) && (y < 0)))
        {
            if(x < 0)
            {
                angle = 180 + angle;
            }
            else if(y < 0)
            {
                angle = 360 + angle;
            }
            else
            {
                angle = 180 + angle;
            }
        }
    }

    /* If abs of x is smaller than y, use atan(x/y) */
    else if(absX < absY)
    {

        theta = (float) ((float) x / (float) y);

        // Get the angle in degrees
        angle = arctan(theta);

        /* We have a negative angle, check the quadrant */
        if(angle <= 0)
        {
            // Y is negative
            if(y < 0)
            {
                angle = 270 - angle;
            }
            // Y is positive
            else if (y > 0)
            {
                angle = 180 - (90 + angle);
            }
        }
        // Angle is positive
        else if(angle > 0)
        {
            // X and Y is negative
            if((x < 0) && (y < 0))
            {
                angle = 270 - angle;
            }
            else
            {
                angle = 90 - angle;
            }
        }
    }

    return angle;
}

/**
 * @brief    Get the abs value of the number (not perfect implementation
 * @param    [int16_t] signed 16 bit number
 * @retval    [int16_t] return the absolute value (16-bit)
 */
int16_t gAbs(int16_t x)
{
    if (x < 0)
        x = -x;

    return x;
}

/**Tymphany added**/
static void tapTimerCallBack(void * pCbPara)
{
    //There is nothing happed during 300 ms, send a tap event and clear the timerID
    timerId = TIMER_ID_INVALID;
    tap();
}

static void doubleTapAnalyse()
{
    if(timerId != TIMER_ID_INVALID)
    {
        //This is a double tap, send a double tap event
        KeySrv_SendKeyEvt_Direct(KEY_EVT_DOUBLE_PRESS, TOUCH_TAP_KEY);
        /* 
         * if in DOUBLE_CLICK_PERIOD_MS, 300 ms, there are three tap event coming, the second one the reset the timer
         * the third one will retriger the timer.
        */
        Timer_StopTimer(timerId);
        timerId = TIMER_ID_INVALID;
    }
    else
    {
        Timer_StartTimer(DOUBLE_CLICK_PERIOD_MS,&timerId,tapTimerCallBack,NULL);
    }
}