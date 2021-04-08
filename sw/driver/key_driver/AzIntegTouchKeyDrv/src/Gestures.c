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
* @file 	Gestures.c								 					      *
* @brief 	Implementation of the Gestures for touch driver. All of 		  *
* 			the processing of the Gestures are done here					  *
* @author 	JD Loy - Azoteq (PTY) Ltd                            	 		  *
* @version 	V2.0.0                                                        	  *
* @date 	24/03/2016                                                     	  *
*******************************************************************************/

//#define SH_DEBUG

// C includes
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#ifdef TOUCH_OFFLINE_TEST
#include <stdint.h>
#include "Gestures.h"
#include "IQS360A.h"
#include "product.config"
#include "touch_data.h"

#define uint32 uint32_t
#define uint16 uint16_t

#define assert(x)       if(!(x)) {printf("asserted at: %s, %d\r\n",__FILE__, __LINE__); exit(-1);}

#else
// User includes
#include "Gestures.h"
#include "IQS_Commands.h"
#include "tch_defines.h"
#include "IQS360A.h"
#include "IQS5xx.h"

// Tymphany platform
#include "deviceTypes.h"
#include "KeySrv.h"
#include "../key_server/KeySrv_priv.h"
#include "timer.h"

#endif


//debug
extern void AseNgSrv_SendLog(const char * str, const char * file, uint32 line);




//static tTimerInfo timerInfo;
#ifndef TOUCH_OFFLINE_TEST
static uint16 timerId= TIMER_ID_INVALID;
static void tapTimerCallBack(void * pCbPara);
static void doubleTapAnalyse();
#endif

/* Private Defines */
#define NR_OF_CHANNELS 				(uint8_t)(NR_OF_ACTIVE_CHANNELS-1) 	// Used to determine the size, take into account Prox CH
#define LTA_OFFSET					(uint8_t)(NR_OF_ACTIVE_CHANNELS*2) 	// Get an LTA Buffer offset
#define PI							(float)3.14

#define FUZZY_BOUNDRY       		0

/* Tymphany:
 *   The redifinition of RESOLUTION is ignored by compiler.
 *   Thus we remove it to avoid compiler warning.
 */
//#define RESOLUTION          		1792

#define ORIGIN              		892
#define TP_SCALE            		1
#define TP_CHANNELS         		8
#define BOUNDRY             		0.10                 /* previously this was 0.3, however in Function determineSign(), we now added some
                                                           math, such that the determinant is normalized by the actual length of vector AB (the length of swipe),
                                                           so the determinant will go smaller and it will easily fail into BOUNDRY, so at the same time we
                                                           decrease the BOUNDRY value as well. */

#define TRACKPAD_WIDTH      		3.2050  			// mm
#define TRACKPAD_HEIGHT     		3.2550   			// mm	THE TRACKPAD IS NOT ROUND
#define	RESOLUTION					((uint16_t)256) 	// pixels / channel
#define TRACKPAD_WIDTH_PX			( ( TP_CHANNELS - 1 )*RESOLUTION )
#define TRACKPAD_HEIGHT_PX			( ( TP_CHANNELS - 1 )*RESOLUTION )
#define TRACKPAD_WIDTH_MM_PER_PIXEL     0.002552065
#define TRACKPAD_HEIGHT_MM_PER_PIXEL    0.002594866

#define inner_ring          (double)2.09
#define outer_ring          (double)2.79

char shlog[100];
uint16_t type;

Point_2D_raw convertPointToTPAxis(Point_2D p);

Gesture_Coordinates g_SwipeAxis = {{0}};


Channel_t Channel[NR_OF_ACTIVE_CHANNELS];		// We have an array of NR_OF_ACTIVE_CHANNELS (5 counting prox) to account for prox which is discarded


						  //( A  ,    B  ,    C   ,   D  )
int16_t quadrents[4][2] = { {1,1}, {-1,1}, {-1,-1},{1,-1}};
/*
 *  Channel Location			|Channel Order 			       |Actual Channel order from IQS360A
 * =============================|==============================|=================================
 * 			Rx0	Rx1	Rx2	Rx3 	|		Rx0 Rx1 Rx2 Rx3 	   |	    Rx0	Rx1	Rx2	Rx3
 * 	Tx0		BLI	BLO	TLO	TLI 	| Tx0	4	7	6	1 	       | Tx0	0	3	6	9
 * 	Tx1		BRI	BRO	TRO	TRI 	| Tx1	3	8	5	2          | Tx1	1	4	7	10
 * 	Tx2		n/a	n/a	n/a	n/a 	| Tx2	n/a	n/a	n/a	n/a        | Tx2	2	5	8	11
 * 	=============================================================================================
 *
 * 	 (6)TLO        |        TRO(5)
 *		 (2)TLI    |   TRI(1)
 *                 |
 *   --------------o--------------
 *                 |
 *		  (3)BLI   |   BRI(4)
 * 	 (7)BLO        |        BRO(8)

 */
//uint8_t ChannelOrder[8] = {10,9,3,4,6,7,0,1};
uint8_t ChannelOrder[8] = {9,10,1,0,6,7,4,3};
uint16_t iqs360Deltas[8];

/* Buffers for the Channels */
#ifdef TEST_ALGORITHM
#ifdef test_1
int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =   {{55, 11, 3, 2, 9, 10, 4, 0, 0},
                                                                   {3, 3, 0, 5, 5, 7, 7, 5, 10},
                                                                   {0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                   {22, 9, 7, 7, 5, 5, 6, 7, 3},
                                                                   {2, 3, 2, 3, 2, 4, 5, 6, 8},
                                                                   {0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                   {37, 24, 19, 16, 15, 13, 10, 12, 9},
                                                                   {7, 9, 12, 16, 22, 30, 42, 57, 73},
                                                                   {0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                   {110, 59, 40, 33, 26, 20, 15, 13, 10},
                                                                   {13, 18, 27, 41, 63, 98, 155, 232, 326},
                                                                   {0, 0, 0, 0, 0, 0, 0, 0, 0}};
#elif defined test_2
int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =   {{647, 262, 68, 33, 22, 12, 6, 0, 0, 0, 0, 3, 0, 2, 0, 0, 0, 3},
                                                                    {3, 5, 11, 13, 19, 25, 30, 34, 38, 42, 43, 47, 43, 47, 43, 40, 40, 33},
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                    {276, 95, 39, 19, 13, 7, 6, 4, 4, 5, 5, 5, 3, 3, 3, 2, 3, 4},
                                                                    {7, 5, 7, 8, 10, 15, 17, 21, 25, 29, 30, 33, 33, 34, 33, 32, 30, 29},
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                    {14, 9, 6, 4, 3, 3, 2, 2, 2, 2, 0, 2, 0, 0, 0, 0, 0, 0},
                                                                    {0, 0, 0, 0, 2, 3, 6, 9, 14, 22, 30, 36, 39, 41, 39, 39, 35, 28},
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
                                                                    {29, 25, 16, 10, 7, 5, 5, 5, 2, 3, 0, 0, 2, 2, 0, 0, 0, 0},
                                                                    {0, 0, 0, 2, 6, 12, 19, 38, 63, 104, 156, 206, 239, 246, 249, 230, 193, 139},
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
#endif
#else
uint16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] = {{0}};	// history Buffer of all the channels
#endif
uint16_t Delta_buffer[NR_OF_CHANNELS][INPUT_BUFFER_SAMPLES] = {{0}};		// Input Buffer of all the channel deltas

/* Set Hand Position State */


/* Function Pointers */
void (*forward)(void);
void (*backward)(void);
void (*tap)(void);

/* Private Function Prototypes */
void toggle_forward(void);
void toggle_backward(void);
void toggle_tap(void);


#ifndef TOUCH_OFFLINE_TEST
/* Tymphany add */
char* str_evt_num()
{
#ifdef HAS_AZOTEQ_INTEG_TOUCHKEY_DRV_EVT_MINOTOR
    I2C_Device_t *iqs360a_cmds = IQS360A_Get_Device();
    I2C_Device_t *iqs5xx_cmds = IQS5xx_Get_Device();

    static char buf[100];
    snprintf(buf, sizeof(buf), "- [360]%d/%d(%d%%), [572]%d/%d(%d%%)",
              iqs360a_cmds->miss_evt_num, iqs360a_cmds->total_evt_num, iqs360a_cmds->miss_evt_num*100/iqs360a_cmds->total_evt_num,
              iqs5xx_cmds->miss_evt_num, iqs5xx_cmds->total_evt_num, iqs5xx_cmds->miss_evt_num*100/iqs5xx_cmds->total_evt_num);
    return buf;
#else
    return "";
#endif
}


/**
 * @brief	Init the Gesture Engine, such as buffers etc
 * @param	None
 * @retval	None
 */
void GS_init(void)
{
	/* Get the Funtion Pointer to the functions that need executing at gestures */
	forward = &toggle_forward;
	backward = &toggle_backward;
	tap = &toggle_tap;
}


/**
 * @brief	Add and process data (CS and LTA) from IQS360
 * @param	[uint8_t*] Pointer to buffer where data is located
 * @retval	[uint8_t] Success or not
 */
uint8_t process_data(uint8_t* buffer)
{
	uint8_t status = RETURN_OK;
	uint8_t i = 0;
	uint16_t tempCS = 0;	// Keep the temp of the CS
	uint16_t tempLTA = 0;	// Keep the temp of the LTA

#ifdef ATOUCH_DUMP_IQS360A_DATA
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
#ifdef ATOUCH_DUMP_IQS360A_DATA
		ATOUCH_PRINTF("(%d,%d), ", Channel[i].CS, Channel[i].LTA);
#endif
	}


#ifdef ATOUCH_DUMP_IQS360A_DATA
	ATOUCH_PRINTF("\r\n");
#endif

	return status;
}

#endif/* TOUCH_OFFLINE_TEST */

/**
 * @brief	Calculate the average of the input buffer (delta buffer) and then
 * 			add this value to the history buffer for further processing
 * @param	None
 * @retval	[uint8_t] Success or not
 */
uint8_t calc_average(void)
{
	uint8_t status = RETURN_OK;
	uint8_t i,j;
	uint16_t cumSum = 0;	// create large variable to keep the total value of each channel

#ifdef ATOUCH_DUMP_IQS360A_DATA
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
			cumSum += Delta_buffer[j][i];	// CH sum
		}

		History_Buffer[j][0] = (uint16_t)(cumSum/INPUT_BUFFER_SAMPLES);	// get the average and add to first element of History Buffer

		/* If the Average value is lower than the Filter value, make it a Nan */
		if (History_Buffer[j][0] < FILTER_VALUE)
		{
			History_Buffer[j][0] = 0;//NAN;	// Replace this with a NaN
		}

		// TODO Debug
		if(History_Buffer[j][0] > 0)
		{
			//__asm__("NOP");
		}

        #ifdef ATOUCH_DUMP_IQS360A_DATA
        ATOUCH_PRINTF("(%d/%d/%d), ", Channel[j+1].CS, Channel[j+1].LTA, History_Buffer[j][0]);
        #endif
	}

    #ifdef ATOUCH_DUMP_IQS360A_DATA
    ATOUCH_PRINTF("\r\n");
    #endif
	return status;
}



uint8_t add_to_buffer(void)
{
	uint8_t status = RETURN_OK;
	uint8_t i = 0;
	uint16_t tempDelta = 0;

	/* Calculate the Delta's - ignore Prox channel [0] */
	for (i = 1; i < NR_OF_ACTIVE_CHANNELS; i++) // Need to iterate over all channels
	{
		/* Remove oldest item and set that value to 0 */
		move_out_queue(&Delta_buffer[i-1][0], INPUT_BUFFER_SAMPLES);

		// Delta > 0, so NAN
		if (Channel[i].LTA >= Channel[i].CS)				// Project capacitance so CS > LTA and CS increases with a proximity
		{
			tempDelta = 0;//NAN;
		}
		// Valid Delta
		else {
			tempDelta = (Channel[i].CS - Channel[i].LTA);	// Project capacitance so CS > LTA and CS increases with a proximity
		}

		/* Add to back of queue */
		add_to_queue(&Delta_buffer[i-1][0], tempDelta);		// add this delta to the first element

        //printf("%d\t", tempDelta);
	}
    //printf("\r\n");

	/* Now call the Average value */
	calc_average();

	/* increment the queue length */
	g_SwipeAxis.queue_length++;

	return status;
}


#ifndef TOUCH_OFFLINE_TEST
/**
 * @brief	Report the Position of the hand. This is a very important
 * 			Step
 * @param	the gesture
 * @retval	None
 */
void report_hand_position(Gesture_Direction T)
{

	// Announce the winner!!
	if (T == FW)
	{
		forward();
		/*ATOUCH_PRINTF("FW swipe %d : t = %x %x|%x, maxCh = %x, 2nd = %x, tot = %d, (a%d,a%d,a%d,a%d) = (%d,%d,%d,%d) (%d,%d,%d,%d)\r\n",
		T, type, forw_votes, back_votes, maxCh, secondHighAtMaxCh,
		Total_P, segments[0], segments[1], segments[2], segments[3], a0,
		a1, a2, a3, Maximums[0].Order, Maximums[1].Order,
		Maximums[2].Order, Maximums[3].Order);*/
	}
	else if (T == BW)
	{
		backward();
		/*TOUCH_PRINTF("BW swipe %d : t = %x %x|%x, maxCh = %x, 2nd = %x, tot = %d, (a%d,a%d,a%d,a%d) = (%d,%d,%d,%d) (%d,%d,%d,%d)\r\n",
		T, type, forw_votes, back_votes, maxCh, secondHighAtMaxCh,
		Total_P, segments[0], segments[1], segments[2], segments[3], a0,
		a1, a2, a3, Maximums[0].Order, Maximums[1].Order,
		Maximums[2].Order, Maximums[3].Order);*/
	}
	else
	{
		KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_SWIPE_UNKNOW); //let MainApp show LED for undefine key
		ATOUCH_PRINTF("(undefined) %s\r\n", str_evt_num());
	}
}
#endif


#ifndef TOUCH_OFFLINE_TEST
/**
 * @brief	Get the actual Gesture that occured - call the action functions (callbacks) as well
 * @param	[uint8_t*] Pointer to XY_Info read from IQS5xx
 * @retval	None
 */
void get_gesture(uint8_t* xy_info, bool noise)
{
	uint8_t gesture_byte = xy_info[1]; // This is where gesture byte is located

	/* Interpret the IQS5xx Gesture */
	switch(gesture_byte)
	{
		// Tap
		case TAP:
			/* Tymphany:
			 *   Do double tap analyze. Based on UI spec, send single-tap first,
 			 *   then send double-tap event if present.
 			 *   we move tap() from here to doubleTapAnalyse(), becuase double press should send:
 			 *   (SP,DP), not (SP,SP,DP)
			 */
			//tap();
                  
			if(!noise)
			{
				doubleTapAnalyse();
#ifdef SH_DEBUG
				sprintf(shlog,"TAP\n");
				SH_SendString(shlog);
#endif
			}
			break;

		// Tap and Hold - Not used at the moment */
		case TAP_HOLD:
                {
                    if(!noise)
                    {
                       /* Tymphany:
                        *    When hold time trigger, 572 repeated send HOLD event
                        *    util finger release. We need a filter, to report only
                        *    first HOLD event
                        */
                        static uint32 lastHoldTapTimeMs= 0;
                        uint32 currTimeMs= getSysTime();
                        if( (currTimeMs - lastHoldTapTimeMs) > HOLD_FILTER_TIME_MS )
                        {
                            ATOUCH_PRINTF("(TAP_HOLD) %s\r\n", str_evt_num());
                            KeySrv_SendKeyEvt_Direct(KEY_EVT_HOLD, TOUCH_TAP_KEY);
                        }
                        lastHoldTapTimeMs= currTimeMs;

#ifdef SH_DEBUG
                        sprintf(shlog,"TAP & HOLD\n");
                        SH_SendString(shlog);
#endif
                    }
                break;
                }

		/* Not valid gesture */
		default:
			break;

		}
}

/*****************************************
 * Touch event handler
 *****************************************/
// forward() point to here
void toggle_forward(void)
{
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_SWIPE_RIGHT_KEY);
    ATOUCH_PRINTF("(SWIPE_RIGHT) %s\r\n", str_evt_num());
}

// backward() point to here
void toggle_backward(void)
{
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_SWIPE_LEFT_KEY);
    ATOUCH_PRINTF("(SWIPE_LEFT) %s\r\n", str_evt_num());
}

// tap() point to here
void toggle_tap(void)
{
    ATOUCH_PRINTF("(TAP) %s\r\n", str_evt_num());
    KeySrv_SendKeyEvt_Direct(KEY_EVT_SHORT_PRESS, TOUCH_TAP_KEY);
}

#endif /* TOUCH_OFFLINE_TEST */

/* Helper Functions -------------------------------------*/

/**
 * @brief	Add channel data (CS and LTA) from IQS333 to delta buffer and
 * 			history buffer for further processing
 * @param	[uint16_t*] Pointer to destination buffer
 * @param	[uint16_t*] Pointer to source buffer
 * @param	[size_t] Size of copy
 * @retval	[uint8_t] Success or not
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
 * @brief	Remove the oldest item of the queue - this will move all of
 * 			the channels' data, which is fine, because we want a fixed
 * 			time reference
 * @param	[Channel_Data_t*] Pointer to buffer to shift and clear
 * @param	[uint8_t] Size of copy
 * @retval	[uint8_t] Success or not
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
		buffer[i] = buffer[i-1];	// replace each element with the element before it
	}

	// clear last element
	buffer[0] = 0;

	return status;
}

/**
 * @brief	Remove the oldest item of the queue - this will move all of
 * 			the channels' data, which is fine, because we want a fixed
 * 			time reference
 * @param	[uint16_t*] Pointer to destination buffer
 * @param	[uint16_t] Data to be added
 * @param	[uint8_t] where to add element
 * @retval	[uint8_t] Success or not
 */
uint8_t add_to_queue(uint16_t* buffer, uint16_t data)
{
	uint8_t status = RETURN_OK;

	*buffer = data;

	return status;
}

uint8_t calculateCOMBruteForce(Point_2D *point, uint8_t i)
{
    uint8_t j,k,l = 0;
    float sum = 0;
    float A = 0;
    float B = 0;
    float C = 0;
    //Point_2D com = {0};  /* Tymphany: remove unused variable to avoid compiler warning */
	uint8_t res = RETURN_OK;
    #define SQRT_FACTOR 1

    //History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES]
    for (j = 0; j < 4; j++)
    {
    	l = ChannelOrder[j];
        A = A + pow(History_Buffer[l][i], SQRT_FACTOR)*quadrents[j][0];
        sum = sum +  pow(History_Buffer[l][i], SQRT_FACTOR);
//        printf("x: %d,%d, A = %d(%d,%d), ",i,j,A,History_Buffer[j][i],quadrents[j][0]);
    }

    k = 0;
    for (j = 4; j < 8; j++)
    {
    	l = ChannelOrder[j];
        B = B + pow(History_Buffer[l][i],SQRT_FACTOR)*quadrents[k][0];
        sum = sum +  pow(History_Buffer[l][i],SQRT_FACTOR);
//        printf("%d,%d, B = %d(%d,%d), ",i,j,B,History_Buffer[j][i],quadrents[k][0]);
        k++;
    }
    C = ((float)A)*inner_ring+((float)B)*outer_ring;
//    printf("C = %d ",C);
    // needs a check here to see if C or sum is not ZERO, otherwise we will get an NaN...
    if ((C != 0) || (sum != 0))
    {
    	point->x = (float)(C / (float)sum);
    }
    else
    {
    	res++;
    }

//    printf("sum = %d",sum);
    /* clamp the point */
    if (point->x < -outer_ring)
    {
    	point->x = -outer_ring;
    }
    else if (point->x > outer_ring)
    {
    	point->x = outer_ring;
    }
//    printf("x: %f\n",com.x);
    A = 0;
    B = 0;
    C = 0;
    sum = 0;
    for (j = 0; j < 4; j++)
    {
    	l = ChannelOrder[j];
        A = A + pow(History_Buffer[l][i],SQRT_FACTOR)*quadrents[j][1];
        sum = sum +  pow(History_Buffer[l][i],SQRT_FACTOR);
//        printf("y: %d,%d, A = %d(%d,%d), ",i,j,A,History_Buffer[j][i],quadrents[j][1]);
    }
    k = 0;
    for (j = 4; j < 8; j++)
    {
    	l = ChannelOrder[j];
        B = B + pow(History_Buffer[l][i],SQRT_FACTOR)*quadrents[k][1];
        sum = sum +  pow(History_Buffer[l][i],SQRT_FACTOR);
//        printf("%d,%d, B = %d(%d,%d), ",i,j,B,History_Buffer[j][i],quadrents[k][1]);
        k++;
    }
    C = ((float)A)*inner_ring+((float)B)*outer_ring;
//    printf("C = %d ",C);
    if ((C != 0) || (sum != 0))
    {
    	point->y = (float)(C / (float)sum);
    }
    else
    {
    	res++;
    }
    point->y = (float)((float)C / (float)sum);
//    printf("sum = %d",sum);
    /* clamp the point */
    if (point->y < -outer_ring)
    {
    	point->y = -outer_ring;
    }
    else if (point->y > outer_ring)
    {
    	point->y = outer_ring;
    }

//    printf("y: %f\n",com.y);

    return res;
}

uint8_t determineCOM(void)
{
    // the IQS360 deltas are all stored in a buffer. a time stamp of the touch and release must be marked
    // by the IQS572 events.
    // t0 = touch on IQS572
    // ts = swipe event on IQS572 (used for axis calculations)
    // tn = n samples after swipe event: if performance is slow then this point can be x samples after the swipe event
    // tn = touch release on the IQS572
  //uint8_t t_0 = 0;  /* Tymphany: remove unused variable to avoid compiler warning */
    uint8_t t_swipe = 0;
    uint8_t i = 0;
    Point_2D com = {0};
    Point_2D com_temp = {0};
	uint8_t res = RETURN_OK;

    float total = 0;

    t_swipe = g_SwipeAxis.queue_length;
    if (t_swipe > 0)
    {
		/* array of n samples then a COM is calculated again */
		for (i = 0; i < t_swipe; i++)
		{
			// elements 8, so 8 multiplication / 8 additions
			res = calculateCOMBruteForce(&com_temp,i);
	//        printf("x: %f, y: %f :",com_temp.x,com_temp.y);
			if (!res)
			{
                int extraFactor = 1;
                {
                    /* calculate the extraFactor */
                    if(t_swipe >= 3)
                    {
                        if(i==0)
                            extraFactor = 1;
                        else if(i==(t_swipe-1))
                            extraFactor = 1;
                        else if( i == (t_swipe/2)) /* middle of the swipe */
                            extraFactor = 4;
                        else
                            extraFactor = 1;
                    }
                    if(t_swipe >= 5)
                    {
                        /* more weight for the samples at the time around center */
                        if(i== ((t_swipe/2)-1))
                            extraFactor = 300;
                        else if(i== ((t_swipe/2)+1))
                            extraFactor = 100;
                    }
                }
				com.x += (com_temp.x * extraFactor);
				com.y += (com_temp.y * extraFactor);
                total += extraFactor;
			}
            else
            {
                //printf("cannot do COM\r\n");
            }

	//        printf(", x: %f, y: %f\n",com.x,com.y);
		}

		if (!res)
		{
			//g_SwipeAxis.com.x = (float)(com.x / (float)t_swipe);
			//g_SwipeAxis.com.y = (float)(com.y / (float)t_swipe);
			g_SwipeAxis.com.x = (float)(com.x / (float)total);
			g_SwipeAxis.com.y = (float)(com.y / (float)total);
		}
    }
    else
    {
    	res++;	// error: t_swipe can not be zero
    }

    return res;
}

int isOffCenterSwipe(void)
{
    float x1, y1, x2, y2, d;
    int rtn = 0;
    x1 = g_SwipeAxis.start.x;
    y1 = g_SwipeAxis.start.y;
    x2 = g_SwipeAxis.end.x;
    y2 = g_SwipeAxis.end.y;
    d = ((x2*y1) - (y2*x1)) / sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)); //distance between origin to the line AB
    if(fabsf(d) > 0.9)
    {
        /* anything swipe with more than 9mm from the origin is considered off center */
        rtn = 1;
    }
    return rtn;
}

int8_t checkSwipeBoundry(void)
{
	float A = 0;
	float B = 0;
	float C = 0;
	float d = 0;
	float num = 0;
	float den = 0;


	A = g_SwipeAxis.end.y - g_SwipeAxis.start.y;
	B = g_SwipeAxis.end.x - g_SwipeAxis.start.x;
	C = (g_SwipeAxis.end.x * g_SwipeAxis.start.y) - (g_SwipeAxis.end.y * g_SwipeAxis.start.x);

	num = fabsf(C);
	den = sqrt((A*A)+(B*B));
	d = num / den;

	if (d > 1.1)
		return 0;		// trajectory outside of bounds
	else
		return 1;		// valid trajectory

}

float g_determinant = 0;

int8_t determineSign(void)
{
    float determinant = 0;
    //int8_t det = 0;
    Point_2D cross_product[2] = {{0}};
/*
    | ax ay 1 |
    | bx by 1 |
    | cx cy 1 |
*/
    //  calculate the determinant of the two vectors
    cross_product[0].x = g_SwipeAxis.start.x - g_SwipeAxis.com.x;
    cross_product[0].y = g_SwipeAxis.start.y - g_SwipeAxis.com.y;
    cross_product[1].x = g_SwipeAxis.end.x - g_SwipeAxis.com.x;
    cross_product[1].y = g_SwipeAxis.end.y - g_SwipeAxis.com.y;

    // now calculate the determinant so that the sign can be evaluated
    determinant = (cross_product[0].x  * cross_product[1].y) - (cross_product[0].y * cross_product[1].x );

    {
        /* considering starting point as A, end point as B, the centre of mass as C
         * cross product determinant means the parallelogram area formed by A, B, C.
         * http://i.stack.imgur.com/PlunZ.png
         * so determinant value is strongly influenced by the length of the vector A, B (the length of the swipe)
         * in our case, the length of the swipe is given by the IQS572 IC and such length depends on the actual sampling on 572.
         * From my measurement, the AB length, ie. swipe length could differ up to 50%. This gives a big random factor in this algor.
         * The following maths removes the variation factor on the vector AB magnitude.
         */
         #if 1
        float x1 = g_SwipeAxis.start.x;
        float y1 = g_SwipeAxis.start.y;
        float x2 = g_SwipeAxis.end.x;
        float y2 = g_SwipeAxis.end.y;
        float tmp = ((x1-x2) * (x1-x2)) + ((y1-y2) * (y1-y2));
        float mag = sqrt(tmp);
        determinant = determinant / mag;
        #endif
        g_determinant = determinant;
    }
    //printf("determinant = %f\r\n", determinant);
#ifndef TOUCH_OFFLINE_TEST
//    printf("| %f   %f |\n",cross_product[0].x, cross_product[0].y);
//    printf("| %f   %f |\n",cross_product[1].x, cross_product[1].y);
    ATOUCH_PRINTF("determinant = %f\r\n", determinant);
#endif

//#ifdef DEBUG_COMMS
			//sprintf(shlog,"determinant: %d,\t",((int32_t)(determinant*100)));
			//printf(shlog);
			#ifndef TOUCH_OFFLINE_TEST
			AseNgSrv_SendLog(shlog, "", 0);
			#endif /* TOUCH_OFFLINE_TEST */
			//SH_SendString(shlog);
//#endif

    // just clamp clamp the determinant
    if ((determinant > -BOUNDRY) && (determinant < BOUNDRY))
    {
        determinant = 0;		// the gesture is undefined if the centre of mass is too close to the trajectory.
    }


    /* Tymphany:
     *    checkSwipeBoundry() is to ignore "low confidence" swipe which passes through the middle part of the touch panel.
     *    The whole circular IQS572 track pad has a radius of 20mm, checkSwipeBoundry() only accepts swipes that has a perpendicular
     *    distance within 11mm to the circle center..
     *    To reduce "undefine rate", we try to disable checkSwipeBoundry().
     */
#ifdef ATOUCH_CHECK_SWIPE_BOUNDRY //Tymphany add, never define, means remove this feature
    // Check to see if the trajectory is inside the swipe band. If it is now push out UNDEFINED
    /*det = checkSwipeBoundry();
    if (det == 0)
    {
    	determinant = 0;
    }
    */
#endif

    if(isOffCenterSwipe())
    {
        /* ignore off center swipe */
        determinant = 0;
    }

    // now determine the position of the point
    if (determinant < 0)        // the point it below the line, when FW is left to right
    {   //Tymphany: FW
        return -1;
    }
    else if (determinant > 0)
    {   //Tymphany: BW
        return 1;
    }
    else
    {   //Tymphany: undefine
    	return 0;
    }
}

Gesture_Direction evaluateSign(int8_t position, Gesture_Direction gesture)
{
    /* this function is called with the FW assumption and is either confirmed or changed */
    if (position > 0)
    {
        gesture = BW;
//#ifdef DEBUG_COMMS

#ifndef TOUCH_OFFLINE_TEST
		//sprintf(shlog,"gesture = BW \t %d \t xs \t%d, \t y s \t %d\t xf \t %d, \t yf \t %d \t\r",g_SwipeAxis.queue_length,g_SwipeAxis.start_raw.x,g_SwipeAxis.start_raw.y,g_SwipeAxis.end_raw.x,g_SwipeAxis.end_raw.y);
		//printf(shlog);
    	AseNgSrv_SendLog(shlog, "", 0);
#endif /* TOUCH_OFFLINE_TEST */

		//SH_SendString(shlog);
//#endif
    }
    else if (position < 0)
    {
        gesture = FW;
//#ifdef DEBUG_COMMS
		//sprintf(shlog,"gesture = FW \t %d \t xs \t%d, \t y s \t %d\t xf \t %d, \t yf \t %d \t\r",g_SwipeAxis.queue_length,g_SwipeAxis.start_raw.x,g_SwipeAxis.start_raw.y,g_SwipeAxis.end_raw.x,g_SwipeAxis.end_raw.y);
		//printf(shlog);
		//printf(shlog);
		#ifndef TOUCH_OFFLINE_TEST
    	AseNgSrv_SendLog(shlog, "", 0);
    	#endif /* TOUCH_OFFLINE_TEST */

		//SH_SendString(shlog);
//#endif
    }
    else
    {
        gesture = UNDEFINED;
//#ifdef DEBUG_COMMS
		//sprintf(shlog,"gesture = n-a \t %d \t xs \t%d, \t y s \t %d\t xf \t %d, \t yf \t %d \t\r",g_SwipeAxis.queue_length,g_SwipeAxis.start_raw.x,g_SwipeAxis.start_raw.y,g_SwipeAxis.end_raw.x,g_SwipeAxis.end_raw.y );
		//printf(shlog);
		#ifndef TOUCH_OFFLINE_TEST
    	AseNgSrv_SendLog(shlog, "", 0);
    	#endif /* TOUCH_OFFLINE_TEST */
				//SH_SendString(shlog);
//#endif
    }
    return gesture;
}


#ifdef TOUCH_OFFLINE_TEST
char data[2000] = {0};
void doDataCollection(void)
{

    /* in this function, we print out a lot of data for data collection purpose */

    /* here are the data we are going to send out
     *  Offset Byte 0:          enum                                                                Ground truth (this is a space holder for the actual user to fill in)
     *                                                                                              (0 for unknown, 1 for backward, 2 for forward, )
     *  Offset Byte 1-2:        uint16                                                              Queue Length
     *  offset byte 3-801:      int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES]      History_Buffer data.(1200 bytes)
     *  Offset Byte 802-803:    Start X coordinate                                                  uint16_t
     *  Offset Byte 804-805:    Start Y coordinate                                                  uint16_t
     *  Offset Byte 806-807:    End   X coordinate                                                  uint16_t
     *  Offset Byte 808-809:    End   Y coordinate                                                  uint16_t
     *  Offset Byte 820:        Code version major                                                  uint8
     *  Offset Byte 821:        Code version minor1                                                 uint8
     *  Offset Byte 821:        Code version minor2                                                 uint8
     *  Offset Byte 821:        Code version minor3                                                 uint8
     *  Offset Byte 823-826:    checksum                                                            uint32_t
     * */
    /* for easy data parsing, the data will be encoded like a C array like the following:
     * {0xaa, 0xbb, 0xcc....};
     */

    int cumm_idx = 0;

    int idx_history_buffer = 0;
    int idx_q_length = 0;
    int idx_start_x_coor = 0;
    int idx_start_y_coor = 0;
    int idx_end_x_coor = 0;
    int idx_end_y_coor = 0;
    int idx_ver_major = 0;
    int idx_ver_minor1 = 0;
    int idx_ver_minor2 = 0;
    int idx_ver_minor3 = 0;
    int idx_checksum = 0;

    /* ground truth */
    data[cumm_idx++] = 0; /* 0 for unknown ground truth, as this is run on the actual speaker,
                    so it does not know the ground truth during data collection. */

    /* for queue_length (little endian) */
    idx_q_length = cumm_idx;
    data[cumm_idx++] = g_SwipeAxis.queue_length & 0xff;
    data[cumm_idx++] = (g_SwipeAxis.queue_length>>8) & 0xff;

    /* for History_Buffer */
    idx_history_buffer = cumm_idx;
    {
        int i, j;
        for(i=0 ; i< NR_OF_CHANNELS ; i++)
        {
            for(j=0 ; j< HISTORY_BUFFER_SAMPLES ; j++)
            {
                int idx = cumm_idx + (i*HISTORY_BUFFER_SAMPLES + j)*2;
                ASSERT(idx+1 < sizeof(data));
                data[idx]   =    History_Buffer[i][j]       & 0xff;
                data[idx+1] =   (History_Buffer[i][j] >> 8) & 0xff;
            }
        }
        cumm_idx += (NR_OF_CHANNELS*HISTORY_BUFFER_SAMPLES*2);
    }

    /* for start x */
    idx_start_x_coor = cumm_idx;
    data[cumm_idx++] = (g_SwipeAxis.start_raw.x    ) & 0xff;
    data[cumm_idx++] = (g_SwipeAxis.start_raw.x >>8) & 0xff;
    /* for start y */
    idx_start_y_coor = cumm_idx;
    data[cumm_idx++] = (g_SwipeAxis.start_raw.y    ) & 0xff;
    data[cumm_idx++] = (g_SwipeAxis.start_raw.y >>8) & 0xff;
    /* for end x */
    idx_end_x_coor = cumm_idx;
    data[cumm_idx++] = (g_SwipeAxis.end_raw.x    ) & 0xff;
    data[cumm_idx++] = (g_SwipeAxis.end_raw.x >>8) & 0xff;
    /* for end y */
    idx_end_y_coor = cumm_idx;
    data[cumm_idx++] = (g_SwipeAxis.end_raw.y    ) & 0xff;
    data[cumm_idx++] = (g_SwipeAxis.end_raw.y >>8) & 0xff;
    /* MCU major */
    idx_ver_major = cumm_idx;
    data[cumm_idx++] = SW_MAJOR_VERSION;
    idx_ver_minor1 = cumm_idx;
    data[cumm_idx++] = SW_MINOR_VERSION1;
    idx_ver_minor2 = cumm_idx;
    data[cumm_idx++] = SW_MINOR_VERSION2;
    idx_ver_minor3 = cumm_idx;
    data[cumm_idx++] = SW_MINOR_VERSION3;
    /* result determinant */



    /* checksum */
    {
        /* this does checksum for the bytes from offset 1, all the way till the end. */
        /* note that the first bytes does not count as it is a place holder for user to fill in the ground truth. */
        int i;
        int checksum = data[1];
        for(i=2 ; i<cumm_idx ; i++)
        {
            checksum ^= data[i];
        }
        idx_checksum = cumm_idx;
        data[cumm_idx++] = checksum;
        printf("cum_index:%d\n", cumm_idx);
    }

    /* print all the data */
    {
        int i;
        {
            /* print start */
            char tmp[50];
            int len = sprintf(tmp, "/* determinant: %.5f */", g_determinant);

            UartDrv_Write_Blocking(1, tmp, len);
            UartDrv_Write_Blocking(1, "{", 1);
        }
        for(i=0 ; i<cumm_idx ; i++)
        {
            {
                /* add some data labels here */
                if(i == idx_history_buffer)
                {
                    char tmp[] = "/* History_Buffer */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_q_length)
                {
                    char tmp[] = "/* q length */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_start_x_coor)
                {
                    char tmp[] = "/* start x coor */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_start_y_coor)
                {
                    char tmp[] = "/* start y coor */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_end_x_coor)
                {
                    char tmp[] = "/* end x coor */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_end_y_coor)
                {
                    char tmp[] = "/* end y coor */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_ver_major)
                {
                    char tmp[] = "/* ver major */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_ver_minor1)
                {
                    char tmp[] = "/* ver minor1 */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_ver_minor2)
                {
                    char tmp[] = "/* ver minor2 */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_ver_minor3)
                {
                    char tmp[] = "/* ver minor3 */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
                if(i == idx_checksum)
                {
                    char tmp[] = "/* checksum */";
                    UartDrv_Write_Blocking(1, tmp, strlen(tmp));
                }
            }
            {
                char tmp[20] = {0};
                int len = sprintf(tmp, "0x%02x", data[i]);
                //UartDrv_Write_Blocking(1, tmp, len);
                ATOUCH_PRINTF(tmp);
                if(i != cumm_idx-1)
                {
                    UartDrv_Write_Blocking(1, ",", 1);
                }
            }
        }
    }

    /* ending */
    UartDrv_Write_Blocking(1, "},", 2);
    UartDrv_Write_Blocking(1, "\r\n", 2);
    UartDrv_Write_Blocking(1, "\r\n", 2);
    UartDrv_Write_Blocking(1, "\r\n", 2);

}
#endif /* TOUCH_OFFLINE_TEST */


Gesture_Direction evaluateSwipe(bool noise)
{
    //Point_2D point_com = {{0}};  /* Tymphany: remove unused variable to avoid compiler warning */
    int8_t position = 0;
    Gesture_Direction gesture = UNDEFINED;
	uint8_t res = RETURN_OK;
#ifdef DEBUG_COMMS
	Point_2D_raw t;
#endif
#if 1
    {
        /* some data tuning here */
        int k,j = 0;
        // the decision Algorithm start here
        for(k=0 ; k<NR_OF_CHANNELS ; k++)
        {
            for(j=0 ; j<HISTORY_BUFFER_SAMPLES ; j++)
            {
                //History_Buffer[0][j] *=1.5; /* 0 means ch 1 on IQS360 */
                //History_Buffer[3][j] *=1.5; /* 1 means ch 2 on IQS360 */
                //History_Buffer[k][j] = pow(History_Buffer[k][j], 0.50);
                //History_Buffer[k][j] = sqrt(History_Buffer[k][j]);
            }
        }
    #if 0
        /* increase the weighting of the samples in the middle of the swipe */
        {
            if(g_SwipeAxis.queue_length >= 3)
            {
                int middle_index = g_SwipeAxis.queue_length / 2;
                for(k=0 ; k< NR_OF_CHANNELS ; k++)
                {
                    History_Buffer[k][middle_index] *= 2;
                }
            }
        }
#endif


        /* give some benefit to the channel 2, 5 as they seems weaker. */
        {
            for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
            {
                History_Buffer[0/*ch1*/][k] *= 1.75;
                History_Buffer[3/*ch4*/][k] *= 1.75;
                History_Buffer[1/*ch2*/][k] *= 1.2;
                History_Buffer[4/*ch5*/][k] *= 1.2;
                History_Buffer[6/*ch7*/][k] *= 1.0;
                History_Buffer[9/*ch10*/][k] *= 1.0;
                History_Buffer[7/*ch8*/][k] *= 1.4;
                History_Buffer[10/*ch11*/][k] *= 1.4;
            }
        }
#if 0
        {
            /* for neightborhood channel, make them all get the max weight. among them
             * say, 1 and 4 are neightborhood sensors, if sensor 1 get 40 and sensor#4 gets 50,
             * make them both 50
             * */
            int k,j;
            int sensorSets[][2] = { {0/*ch1*/,3/*ch4*/},
                                    {1/*ch2*/,4/*ch5*/},
                                    {6/*ch7*/,9/*ch10*/},
                                    {7/*ch8*/,10/*ch11*/}};
            for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
            {
                for(j=0 ; j< 4/*4 sets of sensors in total */ ; j++)
                {
                    int chA = sensorSets[j][0];
                    int chB = sensorSets[j][1];
                    if(History_Buffer[chA/*ch1*/][k] > History_Buffer[chB/*ch4*/][k])
                    {
                        History_Buffer[chB/*ch4*/][k] =  (History_Buffer[chA/*ch1*/][k] + History_Buffer[chB/*ch4*/][k])/2  ;
                    }
                    else
                    {
                        History_Buffer[chA/*ch4*/][k] =  (History_Buffer[chA/*ch1*/][k] + History_Buffer[chB/*ch4*/][k])/2  ;
                    }
                }
            }
        }
#endif


        {
            /* try to figure out conditions that 2 channels is weighed against 6 other channels */
            float x1 = g_SwipeAxis.start.x;
            float y1 = g_SwipeAxis.start.y;
            float x2 = g_SwipeAxis.end.x;
            float y2 = g_SwipeAxis.end.y;
            int i = 0;
            int oneSideCnt = 0; /* count how many sensors are on one side of the swipe */
            Point_2D sensorPoints[] = { {2.44, 2.44},   /* note that 2.44 is the sensor midpoint location */
                {-2.44, 2.44},
                {-2.44, -2.44},
                {2.44, -2.44},
            };
            int quadrantSensorSign[4] = {0}; /* array idx 0 for quadrant 1, idx 1 for quadrant 2, etc */
            for(i=0 ; i<4 ; i++)
            {
                float x0 = sensorPoints[i].x;
                float y0 = sensorPoints[i].y;
                float d = ((y2-y1)*x0 - (x2-x1)*y0 + x2*y1 - y2*x1) \
                          / sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1));
                if(d > 0)
                {
                    quadrantSensorSign[i] = 1;
                    oneSideCnt ++;
                }
                else
                {
                    quadrantSensorSign[i] = -1;
                }
            }
            if(oneSideCnt == 1 || oneSideCnt == 3)
            {
                const float ch_factor = 0.6;
                /* this is imbalance and we have to do sth,
                 * now find which side has 3 sets of sensors */
                if((quadrantSensorSign[0] == quadrantSensorSign[1]) == quadrantSensorSign[2])
                {
                    int k;
                    for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
                    {
                        History_Buffer[6/*ch7*/][k] *= ch_factor;
                        History_Buffer[9/*ch10*/][k] *= ch_factor;
                        History_Buffer[1/*ch2*/][k] *= ch_factor;
                        History_Buffer[4/*ch5*/][k] *= ch_factor;
                    }
                }
                else if((quadrantSensorSign[1] == quadrantSensorSign[2]) == quadrantSensorSign[3])
                {
                    int k;
                    for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
                    {
                        History_Buffer[7/*ch8*/][k] *= ch_factor;
                        History_Buffer[10/*ch11*/][k] *= ch_factor;
                        History_Buffer[0/*ch1*/][k] *= ch_factor;
                        History_Buffer[3/*ch4*/][k] *= ch_factor;
                    }
                }
                else if((quadrantSensorSign[2] == quadrantSensorSign[3]) == quadrantSensorSign[0])
                {
                    int k;
                    for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
                    {
                        History_Buffer[6/*ch7*/][k] *= ch_factor;
                        History_Buffer[9/*ch10*/][k] *= ch_factor;
                        History_Buffer[1/*ch2*/][k] *= ch_factor;
                        History_Buffer[4/*ch5*/][k] *= ch_factor;
                    }

                }
                else if((quadrantSensorSign[3] == quadrantSensorSign[0]) == quadrantSensorSign[1])
                {
                    int k;
                    for(k=0 ; k< HISTORY_BUFFER_SAMPLES ; k++)
                    {
                        History_Buffer[7/*ch8*/][k] *= ch_factor;
                        History_Buffer[10/*ch11*/][k] *= ch_factor;
                        History_Buffer[0/*ch1*/][k] *= ch_factor;
                        History_Buffer[3/*ch4*/][k] *= ch_factor;
                    }
                }
                else
                {
                    //assert(0);
                }
            }
            //assert(oneSideCnt != 0 && oneSideCnt != 4);
            //printf("oneSideCnt: %d\n", oneSideCnt);
        }




        {
            /* make a cap on the readings */
            const int capValue = 80;
            for(k=0 ; k<NR_OF_CHANNELS ; k++)
            {
                for(j=0 ; j<HISTORY_BUFFER_SAMPLES ; j++)
                {
                    if(History_Buffer[k][j] > capValue)
                    {
                        History_Buffer[k][j] = capValue;
                    }
                }
            }
        }

#if 0
        {
            /* remove beginning and end sample, if we have more than 3 samples. */
            if(g_SwipeAxis.queue_length >= 3)
            {
                for(k=0 ; k< NR_OF_CHANNELS ; k++)
                {
                    History_Buffer[k][0] *= 0.3;
                    History_Buffer[k][g_SwipeAxis.queue_length - 1] *= 0.3;
                }
            }
        }
#endif


                /* as the  */
#if 1
        {
            const float factor = 0.85;
            g_SwipeAxis.start.x *= factor;
            g_SwipeAxis.start.y *= factor;
            g_SwipeAxis.end.x *= factor;
            g_SwipeAxis.end.y *= factor;

        }
#endif
    }
#endif
    // step 1 : determine point sign for each of the 3 line
    if (FUZZY_BOUNDRY)
    {
        // loop sign check 3 times around the main trajectory axis
    }
    else
    {
    	/* determine the centre of mass of the IQS360 data */
        res = determineCOM();

        if (!res)
        {
        	// only check the sign on the main trajectory axis
        	position = determineSign();
        }
        else
        {
        	// force to an undefined state
        	position = 0;
        }

        // now check the direction of the swipe VS the sign and give FW or BW result, if FW, then -1 sign.
        gesture = evaluateSign(position, FW);

        #ifndef TOUCH_OFFLINE_TEST
        if(noise)
        {
            return UNDEFINED;
        }
        report_hand_position(gesture);
        #endif /* TOUCH_OFFLINE_TEST */
    }
    return gesture;
}

void resetGestureBuffers(void)
{
#ifndef TEST_ALGORITHM
	/* Clear buffers */
	memset((void *)History_Buffer, 0, sizeof(History_Buffer));
	memset((void *)Delta_buffer, 0, sizeof(Delta_buffer));
#endif
}


void resetGestureVariables(void)
{
	g_SwipeAxis.start.x = 0;
	g_SwipeAxis.start.y = 0;
	g_SwipeAxis.start_raw.x = 0;
	g_SwipeAxis.start_raw.y = 0;
	g_SwipeAxis.end.x = 0;
	g_SwipeAxis.end.y = 0;
	g_SwipeAxis.end_raw.x = 0;
	g_SwipeAxis.end_raw.y = 0;
	g_SwipeAxis.queue_length = 0;
	g_SwipeAxis.com.x = 0;
	g_SwipeAxis.com.y = 0;
}

void translateTouchPoint(Point_2D *point,uint16_t x, uint16_t y)
{
    point->x = (float)(x - ORIGIN);
    point->y = (float)(y - ORIGIN);
}

Point_2D_raw convertPointToTPAxis(Point_2D p)
{
	Point_2D_raw t = {0,0};
	Point_2D temp = {0,0};

	temp.x = (p.x / TRACKPAD_WIDTH_MM_PER_PIXEL);
	temp.y = -(p.y / TRACKPAD_HEIGHT_MM_PER_PIXEL);

	t.x = (uint16_t)(temp.x + ORIGIN);
	t.y = (uint16_t)(temp.y + ORIGIN);

	return t;
}

void scaleTrackPadPoints(Point_2D *t)
{
    t->x = TRACKPAD_WIDTH_MM_PER_PIXEL * t->x;
    t->y = -(TRACKPAD_HEIGHT_MM_PER_PIXEL * t->y);
}

void setFirstPoint(uint16_t x, uint16_t y)
{
	translateTouchPoint(&g_SwipeAxis.start, x, y);
	scaleTrackPadPoints(&g_SwipeAxis.start);
	g_SwipeAxis.start_raw.x = x;
	g_SwipeAxis.start_raw.y = y;
	// now get the index in the History Buffer
	g_SwipeAxis.queue_length = 0;
}

void setLastPoint(uint16_t dx, uint16_t dy)
{
	g_SwipeAxis.end_raw.x = g_SwipeAxis.start_raw.x + dx;
	g_SwipeAxis.end_raw.y = g_SwipeAxis.start_raw.y + dy;
	translateTouchPoint(&g_SwipeAxis.end, g_SwipeAxis.end_raw.x, g_SwipeAxis.end_raw.y);
	scaleTrackPadPoints(&g_SwipeAxis.end);
	// now get the index in the History Buffer
}

#ifndef TOUCH_OFFLINE_TEST
/**Tymphany added**/
static void tapTimerCallBack(void * pCbPara)
{
    //There is nothing happed during 300 ms, send a tap event and clear the timerID
    timerId = TIMER_ID_INVALID;
    //tap();
}
#endif /* TOUCH_OFFLINE_TEST */


#ifndef TOUCH_OFFLINE_TEST
static void doubleTapAnalyse()
{
    if(timerId != TIMER_ID_INVALID)
    {
        //This is a double tap, send a double tap event
        ATOUCH_PRINTF("(DOUBLE-TAP) %s\r\n", str_evt_num());
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
        tap();
        Timer_StartTimer(DOUBLE_CLICK_PERIOD_MS,&timerId,tapTimerCallBack,NULL);
    }
}
#endif /* TOUCH_OFFLINE_TEST */


#ifdef TOUCH_OFFLINE_TEST

void printGesture(Gesture_Direction g)
{
    char gesture[3] = "FW";
    if (g == FW)
    {
        gesture[0] = 'F';
        gesture[1] = 'W';
    }
    else if (g == BW)
    {
        gesture[0] = 'B';
    }
    else
    {
        gesture[0] = 'n';
        gesture[1] = 'a';
    }
    printf("gesture is %s\n",gesture);
}

void debug_printing()
{
    {
        /* test code for testing the touch data component */
        int rows = TouchData_get_numberOfDataRows();
        int start_x, start_y;
        int end_x, end_y;
        TouchData_get_raw_startPoint_coordinate(0, &start_x, &start_y);
        TouchData_get_raw_endPoint_coordinate(0, &end_x, &end_y);

        printf("number of data row is: %d\r\n", rows);
        printf("start_x is: %d, start_y is: %d\r\n", start_x, start_y);
        printf("end_x is: %d, end_y is: %d\r\n", end_x, end_y);

    }
        {
            /* print out the details calculated */
            /*
            printf("g_SwipeAxis.start.x: %f\r\n", g_SwipeAxis.start.x);
            printf("g_SwipeAxis.start.y: %f\r\n", g_SwipeAxis.start.y);
            printf("g_SwipeAxis.start_raw.x: %d\r\n", g_SwipeAxis.start_raw.x);
            printf("g_SwipeAxis.start_raw.y: %d\r\n", g_SwipeAxis.start_raw.y);
            printf("g_SwipeAxis.end.x: %f\r\n", g_SwipeAxis.end.x);
            printf("g_SwipeAxis.end.y: %f\r\n", g_SwipeAxis.end.y);
            printf("g_SwipeAxis.end_raw.x: %d\r\n", g_SwipeAxis.end_raw.x);
            printf("g_SwipeAxis.end_raw.y: %d\r\n", g_SwipeAxis.end_raw.y);
            */
        }

}



void touch_data_preparation(int i)
{
    {
            /* fill in the start and end point data */
            int x0, y0;
            int x1, y1;
            TouchData_get_raw_startPoint_coordinate(i, &x0, &y0);
            TouchData_get_raw_endPoint_coordinate(i, &x1, &y1);
            /* fill start point */
            setFirstPoint(x0, y0);
            /* fill end point */
            {
                g_SwipeAxis.end_raw.x = x1;
                g_SwipeAxis.end_raw.y = y1;
                translateTouchPoint(&g_SwipeAxis.end, g_SwipeAxis.end_raw.x, g_SwipeAxis.end_raw.y);
                scaleTrackPadPoints(&g_SwipeAxis.end);
            }
        }

        {
            /* fill in history data and q length */
            int q_length = 0;
            TouchData_get_HistoryBuffer(i, (uint16_t**)History_Buffer, &q_length);
            g_SwipeAxis.queue_length = q_length;

        }
}

const char* Gesture_touchType2string(Gesture_touchType touchType)
{
    const char* text = NULL;
    switch(touchType)
    {
        case TOUCH_TYPE_NEAR:
            text = "Near";
            break;
        case TOUCH_TYPE_CENTER:
            text = "Center";
            break;
        case TOUCH_TYPE_FAR:
            text = "Far";
            break;
        default:
            break;
    }
    return text;
}

int Gesture_UserAngle2int(Gesture_UserAngle angle)
{
    int rtn = 0;
    assert(angle < ANGLE_MAX);
    switch(angle)
    {
        case ANGLE_UNKNOWN:
            rtn = 999;
            break;
        case ANGLE_NEG135:
            rtn = -135;
            break;
        case ANGLE_NEG090:
            rtn = -90;
            break;
        case ANGLE_NEG045:
            rtn = -45;
            break;
        case ANGLE_POS000:
            rtn = 0;
            break;
        case ANGLE_POS045:
            rtn = 45;
            break;
        case ANGLE_POS090:
            rtn = 90;
            break;
        case ANGLE_POS135:
            rtn = 135;
            break;
        case ANGLE_POS180:
            rtn = 180;
            break;
        default:
            break;
    }
    return rtn;

}

Gesture_touchType getTouchType(int dataIdx)
{
    Gesture_touchType rtn;
    {
        /* get the ang.. */
        {
            /* get the distance from origin to mid-AB */
            float x1, y1, x2, y2;
            Point_2D A = {0};
            Point_2D B = {0};
            {
                /* fill in x1, y1 */
                int x, y;
                TouchData_get_raw_startPoint_coordinate(dataIdx, &x, &y);
                A.x = x;
                A.y = y;
                translateTouchPoint(&A, A.x, A.y);
                scaleTrackPadPoints(&A);
                x1 = A.x;
                y1 = A.y;
            }
            {
                /* fill in x2, y2 */
                int x, y;
                TouchData_get_raw_endPoint_coordinate(dataIdx, &x, &y);
                B.x = x;
                B.y = y;
                translateTouchPoint(&B, B.x, B.y);
                scaleTrackPadPoints(&B);
                x2 = B.x;
                y2 = B.y;
            }

            float d = ((x2*y1) - (y2*x1)) / sqrt((y2-y1)*(y2-y1) + (x2-x1)*(x2-x1)); //distance between origin to the line AB
            float x_m = (x1 + x2)/2;
            float y_m = (y1 + y2)/2;
            float mid_ang = atan2((float) y_m, (float)x_m)    *180.0/3.141516;;
            {
                /* convert it to the testing angle domain */
                mid_ang -= 90;
                if (mid_ang < -180)
                    mid_ang += 360;
            }
            if(fabsf(d) > 0.9)  /* distant enough from the origin or not */
            {
                rtn = TOUCH_TYPE_FAR;
                switch(TouchData_get_actualUserAngle(dataIdx))
                {
                    case ANGLE_NEG135:
                        if(mid_ang > 135 || (mid_ang >-179 && mid_ang < -45))
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_NEG090:
                        if(mid_ang >-180 && mid_ang < 0)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_NEG045:
                        if(mid_ang >-135 && mid_ang < 45)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_POS000:
                        if(mid_ang >-90 && mid_ang < 90)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_POS045:
                        if(mid_ang >-45 && mid_ang < 135)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_POS090:
                        if(mid_ang >0 && mid_ang < 180)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_POS135:
                        if(mid_ang < -135 || (mid_ang > 45))
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    case ANGLE_POS180:
                        if(mid_ang >90 || mid_ang < -90)
                            rtn = TOUCH_TYPE_NEAR;
                        break;
                    default:
                        assert(0);
                        break;
                }
            }
            else
            {
                rtn = TOUCH_TYPE_CENTER;
            }
        }

    }
    return rtn;
}

void printData2ExcelCsvFormat(int dataIdx)
{
    printf("[Excel CVS]\r\n");
    /* for the first 7 rows, they are:
     * 1. ground truth
     * 2. user's angle
     *    start_raw.x,
     * 2. start_raw.y,
     * 3. end_raw.x,
     * 4. end_raw.y,
     * 5. q_length
     * */
    {
        /* ground truth */
        Gesture_Direction gesture = UNDEFINED;  // default direction is undefined
        gesture = TouchData_get_groundTruth(dataIdx);
        printf("Ground Truth,");
        if(gesture == FW)
        {
            printf("FW\r\n");
        }
        else if(gesture == BW)
        {
            printf("BW\r\n");
        }
    }
    {
        /* ground truth swipe type */
        Gesture_touchType touchType = getTouchType(dataIdx);
        printf("Type,%s\r\n", Gesture_touchType2string(touchType));
    }
    {
        /* user angle */
        Gesture_UserAngle userAngle = TouchData_get_actualUserAngle(dataIdx);
        int angle_int = Gesture_UserAngle2int(userAngle);
        printf("User angle,");
        printf("%d\r\n", angle_int);
    }
    {
        printf("start_raw x,");
        printf("%d\r\n", g_SwipeAxis.start_raw.x);
        printf("start_raw y,");
        printf("%d\r\n", g_SwipeAxis.start_raw.y);
        printf("end_raw x,");
        printf("%d\r\n", g_SwipeAxis.end_raw.x);
        printf("end_raw y,");
        printf("%d\r\n", g_SwipeAxis.end_raw.y);
        printf("q_length,");
        printf("%d\r\n", g_SwipeAxis.queue_length);
        printf("determinant,");
        printf("%.3f\r\n", g_determinant);
    }
    {
        /* print the history buffer */
        int i,j;
        //for(i=0 ; i<HISTORY_BUFFER_SAMPLES ; i++)
        for(i=0 ; i<g_SwipeAxis.queue_length ; i++)
        {
            printf("data[%01d],", i);
            for(j=0 ; j<NR_OF_CHANNELS ; j++)
            {
                char tmp[]=",";
                if(j==(NR_OF_CHANNELS-1))
                {
                    tmp[0] = '\0';
                }
                printf("%03d%s", History_Buffer[j][i], tmp);
            }
            printf("\r\n");
        }
    }
    printf("[/Excel CVS]\r\n");
}

int main()
{
    Gesture_Direction gesture = UNDEFINED;  // default direction is undefined
    int i = 0;
    int correct_cnt = 0;
    int far_cnt = 0;
    int center_cnt = 0;
    int near_cnt = 0;
    int error_report_cnt = 0;

    typedef struct{
        int angle_total_cnt;
        int angle_correct_cnt;
    }Angle_entry;

    typedef struct{
        int total_cnt;
        int correct_cnt;
        int wrong_cnt;
        int unknown_cnt;
    }Analytics_entry;

    Analytics_entry analyticsData[TOUCH_TYPE_MAX][ANGLE_MAX] = {{0}};
    Angle_entry angle_entries[ANGLE_MAX] = {{0}};

    TouchData_verifyDatabase();

    for(i=0 ; i<TouchData_get_numberOfDataRows() ; i++)
    {
        /* run through all the test data */
        touch_data_preparation(i);

        gesture = evaluateSwipe();

        //printGesture(gesture);

        {
            /* reporting, update data */
            Gesture_UserAngle userAngle = TouchData_get_actualUserAngle(i);
            Gesture_touchType touchType = getTouchType(i);

            analyticsData[touchType][userAngle].total_cnt++;

            if(TouchData_get_groundTruth(i) == gesture)
            {
                correct_cnt++;
                angle_entries[userAngle].angle_correct_cnt++;
                analyticsData[touchType][userAngle].correct_cnt++;
            }
            else if(gesture == UNDEFINED)
            {
                analyticsData[touchType][userAngle].unknown_cnt++;
            }
            else
            {
                analyticsData[touchType][userAngle].wrong_cnt++;
                {
                    if(error_report_cnt < 5)
                    {
                        error_report_cnt ++;
                        printf("DataSet #%03d\r\n", i);
                        printf("[After data tuning]\r\n");
                        printData2ExcelCsvFormat(i);
                        touch_data_preparation(i);  /* reset all the data before printing to excel format */
                        printf("[Before data tuning]\r\n");
                        printData2ExcelCsvFormat(i);
                    }
                }
            }

            angle_entries[userAngle].angle_total_cnt++;

            switch(touchType)
            {
                case TOUCH_TYPE_CENTER:
                    center_cnt++;
                    break;
                case TOUCH_TYPE_NEAR:
                    near_cnt++;
                    break;
                case TOUCH_TYPE_FAR:
                    far_cnt++;
                    break;
                default:
                    break;
            }
        }
    }
    {
        /* print result */
        printf("=======================\r\n");
        printf("Result: Overall accurary: %.1f%%, %d/%d\r\n", (float)100.0 * correct_cnt / TouchData_get_numberOfDataRows(), correct_cnt, TouchData_get_numberOfDataRows());
        printf("=======================\r\n");

        {
            /* print the analysis against angle */
            int j = 0;
            printf("=======================\r\n");
            for(j=0 ; j<ANGLE_MAX ; j++)
            {
                printf("Result: At angle: %d, accurary: %.1f%%, %d/%d\r\n", Gesture_UserAngle2int(j),
                                                                            (float)100.0 * angle_entries[j].angle_correct_cnt / angle_entries[j].angle_total_cnt,
                                                                            angle_entries[j].angle_correct_cnt,
                                                                            angle_entries[j].angle_total_cnt
                                                                            );
            }
            printf("=======================\r\n");
        }
        {
            /* print out a 3d table , error cnt, direction, far/near */
            int i,j;
            printf("=======================\r\n");
            printf("far cnt:%d, center_cnt:%d, near_cnt:%d\n\r)", far_cnt, center_cnt, near_cnt);

            printf("\tnear\t\t\tcenter\t\t\tfar\r\n");
            for(i=0 ; i<ANGLE_MAX ; i++)
            {
                printf("%03d\t", Gesture_UserAngle2int(i));
                for(j=0 ; j<TOUCH_TYPE_MAX ; j++)
                {
                    printf("(%02d,%02d,%02d)/%d, (%.1f%%)\t",   analyticsData[j][i].correct_cnt,
                                                                analyticsData[j][i].unknown_cnt,
                                                                analyticsData[j][i].wrong_cnt,
                                        analyticsData[j][i].total_cnt,
                                        100.0*(float)analyticsData[j][i].correct_cnt / analyticsData[j][i].total_cnt
                                        );
                }
                printf("\r\n");

            }
            printf("=======================\r\n");
        }
    }
    return 0;
}
#endif
