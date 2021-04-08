#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

/*!< Signed integer types  */
typedef   signed char     int8_t;
typedef   signed short    int16_t;
typedef   signed long     int32_t;

/*!< Unsigned integer types  */
typedef unsigned char     uint8_t;
typedef unsigned short    uint16_t;
typedef unsigned long     uint32_t;

typedef enum {
    FALSE = 0,
    TRUE = !FALSE
} bool;

typedef enum {
    FW = 1,
    UNDEFINED = 0,
    BW = -1
} Gesture_Direction;

#define PI					(float)3.14
#define FUZZY_BOUNDRY       FALSE
//#define RESOLUTION          1792
#define ORIGIN              892
#define TP_SCALE            1
#define TP_CHANNELS         8
#define BOUNDRY             0.5

#define TRACKPAD_WIDTH      3.2050   // mm
#define TRACKPAD_HEIGHT     3.2550   // mm	THE TRACKPAD IS NOT ROUND
#define	RESOLUTION			((uint16_t)256)
#define TRACKPAD_WIDTH_PX	( ( TP_CHANNELS - 1 )*RESOLUTION )
#define TRACKPAD_HEIGHT_PX	( ( TP_CHANNELS - 1 )*RESOLUTION )
#define TRACKPAD_WIDTH_MM_PER_PIXEL     0.002552065
#define TRACKPAD_HEIGHT_MM_PER_PIXEL    0.002594866

#define inner_ring          (double)2.09
#define outer_ring          (double)2.79

/* Private Defines */
#define NR_OF_ACTIVE_CHANNELS		13		// 13 active channels in this setup + Prox : 0,1,2,*3,4,5,*6,7,8,*9,10,11  (3, 6 and 9 are not actually enabled, but are kept in the array to keep continuity
#define NR_OF_CHANNELS 				(uint8_t)(NR_OF_ACTIVE_CHANNELS-1) 	// Used to determine the size, take into account Prox CH
#define LTA_OFFSET					(uint8_t)(NR_OF_ACTIVE_CHANNELS*2) 	// Get an LTA Buffer offset

                                                                                                               /* Private Defines -------------------------------------*/
#define	NR_OF_SAMPLES						8
#define HISTORY_BUFFER_SAMPLES				51//


#define assert(x)       if(!(x)) {printf("asserted at: %s, %d\n\r",__FILE__, __LINE__); exit(-1);}


#endif
