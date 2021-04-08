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
* @file 	Gestures.h									 				      *
* @brief 	Header for Gestures - this is the file of the processing of		  *
* 			all of the Gestures										          *
* @author 	AJ van der Merwe - Azoteq (PTY) Ltd                            	  *
* @version 	V1.0.0                                                        	  *
* @date 	01/09/2015                                                     	  *
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GESTURES_H
#define __GESTURES_H


#include "tch_defines.h"
#include <stdbool.h>
#include <stdio.h>

//#include <math.h>

// User includes
//#define TEST_ALGORITHM
//#define test_1
//#define test_2
//#define TEST_determineCOM


/* Private Defines -------------------------------------*/
//#define	NR_OF_SAMPLES						5
#define INPUT_BUFFER_SAMPLES				1//13
#define HISTORY_BUFFER_SAMPLES				50//81 10*12

/* Create a NAN number for values smaller than the Filter Value */
#define FILTER_VALUE						2
#define NAN									0	// Corresponds to the Algorithm

/* Define Gestures */
#define TAP									0x01
#define TAP_HOLD                            0x02  // T = 5
//#define SWIPE_LEFT                          0x04  // T = 1
//#define SWIPE_RIGHT                         0x08  // T = 3
//#define SWIPE_UP                            0x10  // T = 0
//#define SWIPE_DOWN                          0x20  // T = 2

/* Tymphany add */
#define DOUBLE_TAP                          0x40




/**
 * Struct to keep the incoming channel data in - LTA and CS values
 * This will be an array of channels
 */
typedef struct {
	uint16_t CS;
	uint16_t LTA;
}Channel_t;

/**
 * Struct to keep the channel data in - we code that value
 * of the number of channels in. We have to know about the
 * channels that are present. No dynamic allocation here
 */
typedef struct {
	// Start Counting at CH1 as CH0 is Prox
	uint16_t CH1;
	uint16_t CH2;
	uint16_t CH4;
	uint16_t CH5;
	uint16_t CH7;
	uint16_t CH8;
	uint16_t CH10;
	uint16_t CH11;
}Channel_Data_t;



typedef struct {
	float 	x;
	float 	y;
} Point_2D;

typedef struct {
	uint16_t 	x;
	uint16_t 	y;
} Point_2D_raw;

typedef struct {
	Point_2D start;							// scaled, and translated XY coordinate of the start point in MM
	Point_2D_raw start_raw;					// raw XY coordinate of the start point in UNITS
	Point_2D end;
	Point_2D_raw end_raw;
	uint8_t queue_length;					// the length of the queue. t0 = lastest point, tn = first point : note the queue has newest data first
	Point_2D com;							// centre of mass
} Gesture_Coordinates;

typedef struct {
	uint16_t start;
	uint16_t end;
} Gesture_Coordinates_Raw;

typedef enum {
    FW = 1,
    UNDEFINED = 0,
    BW = -1
} Gesture_Direction;

typedef enum {
    ANGLE_UNKNOWN = 0,
    ANGLE_NEG135 = 1,
    ANGLE_NEG090 = 2,
    ANGLE_NEG045 = 3,
    ANGLE_POS000 = 4,
    ANGLE_POS045 = 5,
    ANGLE_POS090 = 6,
    ANGLE_POS135 = 7,
    ANGLE_POS180 = 8,
    ANGLE_MAX,
} Gesture_UserAngle;

typedef enum{
    TOUCH_TYPE_NEAR,
    TOUCH_TYPE_CENTER,
    TOUCH_TYPE_FAR,
    TOUCH_TYPE_MAX,
}Gesture_touchType;

/* Function Prototypes -------------------------------------*/
void GS_init(void);
uint8_t process_data(uint8_t* buffer);
uint8_t add_to_buffer(void);


uint8_t remap_data(void);
void calc_hand_position(uint8_t T);
void get_gesture(uint8_t* xy_info, bool noise);

Gesture_Direction evaluateSwipe(bool noise);
void resetGestureVariables(void);
void resetGestureBuffers(void);
void setFirstPoint(uint16_t x, uint16_t y);
void setLastPoint(uint16_t x, uint16_t y);

/* Helper Functions -------------------------------------*/
uint16_t sum_buffer(uint16_t* buffer, uint8_t size);
void arrange_large_small(uint16_t* channels, uint8_t* names);
void arrange_small_large(uint16_t* channels, uint8_t* names);
uint8_t copy_data(uint16_t* dest, uint16_t* source, size_t size);
uint8_t move_out_queue(uint16_t* buffer, uint8_t size);
uint8_t add_to_queue(uint16_t* buffer, uint16_t data);


/* Math Functions -------------------------------------*/
float arctan(float x);
float calc_angle(int16_t x, int16_t y);
int16_t gAbs(int16_t x);

#endif /* __GESTURES_H */
