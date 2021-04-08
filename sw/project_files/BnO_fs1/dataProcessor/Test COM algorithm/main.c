#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "touch_data.h"


/* Buffers for the Channels, order*/
//  index   1	2	3	4	5	6	7	8
//  channel 11	10	4	5	8	7	1	2
//uint16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] = {{0}};	// history Buffer of all the channels
/*uint16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =1   {{75, 41, 22, 8, 7, 4, 0, -2, -5, -1, 2, -1, 6},
                                                                   2{95, 63, 40, 18, 10, 6, 4, 1, 1, 2, -3, -5, -3},
                                                                   3 {22, 25, 33, 46, 86, 250, 407, 295, 155, 80, 40, 15, 0},
                                                                   4 {3, 9, 13, 17, 36, 157, 275, 186, 92, 46, 22, 6, 0},
                                                                   5{47, 24, 13, 6, 1, -4, -1, -6, -8, -9, -7, -5, -10},
                                                                   6 {108, 62, 26, 15, 3, -4, -3, -13, -10, -11, -12, -10, -8},
                                                                   7 {17, 21, 26, 27, 40, 117, 250, 299, 192, 100, 56, 24, 12},
                                                                   8 {8, 10, 8, 9, 23, 69, 294, 348, 208, 122, 64, 31, 15}};
*/
    // the channels must be in order.
/*
 int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =   {{8, 10, 8, 9, 23, 69, 294, 348, 208, 122, 64, 31, 15},
                                                                    {47, 24, 13, 6, 1, -4, -1, -6, -8, -9, -7, -5, -10},
                                                                    {108, 62, 26, 15, 3, -4, -3, -13, -10, -11, -12, -10, -8},
                                                                    {17, 21, 26, 27, 40, 117, 250, 299, 192, 100, 56, 24, 12},
                                                                    {3, 9, 13, 17, 36, 157, 275, 186, 92, 46, 22, 6, 0},
                                                                    {75, 41, 22, 8, 7, 4, 0, -2, -5, -1, 2, -1, 6},
                                                                    {95, 63, 40, 18, 10, 6, 4, 1, 1, 2, -3, -5, -3},
                                                                    {22, 25, 33, 46, 86, 250, 407, 295, 155, 80, 40, 15, 0}};
                                                                    */
//uint8_t ChannelOrder[12] = {8,5,6,7,4,1,2,3};



 int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] = {};



// test 1
 /*int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =   {{647, 262, 68, 33, 22, 12, 6, 0, 0, 0, 0, 3, 0, 2, 0, 0, 0, 3},
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
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};*/
 //test 2
 /*int16_t History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES] =   {{55, 11, 3, 2, 9, 10, 4, 0, 0},
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
                                                                    {0, 0, 0, 0, 0, 0, 0, 0, 0}};*/


//uint8_t ChannelOrder[8] = {11,10,4,5,8,7,1,2};
const uint8_t ChannelOrder[8] = {10,9,3,4,6,7,0,1};
/* Math Functions -------------------------------------*/


typedef struct {
	float 	x;
	float 	y;

} Point_2D;

void generatePoints(Point_2D *t);
void translateTPpoint(Point_2D *point);
                        //(   A  ,    B  ,    C   ,   D  )
int16_t quadrents[4][2] = { {1,1}, {-1,1}, {-1,-1},{1,-1}};
uint16_t iqs360Deltas[8];



inline double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

inline double to_radians(double degrees){
    return (double)((degrees * M_PI) / 180);
}

void translateTPpoint(Point_2D *point)
{
    point->x = (float)(point->x - ORIGIN);
    point->y = (float)(point->y - ORIGIN);
}

void generatePoints(Point_2D *t)
{
//    t[0].x = 174;
//    t[0].y = 870;
//    t[1].x = 1177;
//    t[1].y = 904;

// test 1
    t[0].x = 102;
    t[0].y = 711;
    t[1].x = 944;
    t[1].y = 954;
// test 2
//    t[0].x = 127;
//   t[0].y = 553;
//    t[1].x = 984;
//    t[1].y = 760;
}

void scaleTrackPadPoints(Point_2D *t)
{
    t[0].x = TRACKPAD_WIDTH_MM_PER_PIXEL * t[0].x;
    t[0].y = TRACKPAD_HEIGHT_MM_PER_PIXEL * t[0].y;
    t[1].x = TRACKPAD_WIDTH_MM_PER_PIXEL * t[1].x;
    t[1].y = TRACKPAD_HEIGHT_MM_PER_PIXEL * t[1].y;

}

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


Point_2D calculateCOMBruteForce(uint8_t i)
{
    uint8_t j,k, l = 0;
    uint16_t sum = 0;
    int16_t A = 0;
    int16_t B = 0;
    float C = 0;
    Point_2D com = {0};

    //History_Buffer[NR_OF_CHANNELS][HISTORY_BUFFER_SAMPLES]
    for (j = 0; j < 4; j++)
    {
        l = ChannelOrder[j];
        A = A + History_Buffer[l][i]*quadrents[j][0];
        sum = sum +  History_Buffer[l][i];
       printf("x: A[%d,%d] %d(%d,%d), ",i,j,A,History_Buffer[l][i],quadrents[j][0]);
    }

    k = 0;
    for (j = 4; j < 8; j++)
    {
        l = ChannelOrder[j];
        B = B + History_Buffer[l][i]*quadrents[k][0];
        sum = sum +  History_Buffer[l][i];
        printf("B[%d,%d] %d(%d,%d), ",i,j,B,History_Buffer[l][i],quadrents[k][0]);
        k++;
    }
    C = A*inner_ring+B*outer_ring;
    printf("A = %d,B = %d,C = %f ",A,B,C);
    com.x = (float)(C / (float)sum);
    printf("sum = %d, com.x = %f ",sum,com.x);
    /* clamp the point */
    if (com.x < -outer_ring)
    {
        com.x = -outer_ring;
    }
    else if (com.x > outer_ring)
    {
        com.x = outer_ring;
    }
    printf("x: %f\n",com.x);
    A = 0;
    B = 0;
    C = 0;
    sum = 0;
    for (j = 0; j < 4; j++)
    {
        l = ChannelOrder[j];
        A = A + History_Buffer[l][i]*quadrents[j][1];
        sum = sum +  History_Buffer[l][i];
        printf("y: %d,%d, A = %d(%d,%d), ",i,j,A,History_Buffer[l][i],quadrents[j][1]);
    }
    k = 0;
    for (j = 4; j < 8; j++)
    {
        l = ChannelOrder[j];
        B = B + History_Buffer[l][i]*quadrents[k][1];
        sum = sum +  History_Buffer[l][i];
        printf("%d,%d, B = %d(%d,%d), ",i,j,B,History_Buffer[l][i],quadrents[k][1]);
        k++;
    }
    C = A*inner_ring+B*outer_ring;
    printf("A = %d,B = %d,C = %f ",A,B,C);
    com.y = (float)(C / (float)sum);
    printf("sum = %d, com.y = %f ",sum,com.y);
    /* clamp the point */
    if (com.y < -outer_ring)
    {
        com.y = -outer_ring;
    }
    else if (com.y > outer_ring)
    {
        com.y = outer_ring;
    }

    printf("y: %f\n",com.y);
    return com;
}

Point_2D determineCOM(void)
{
    // the IQS360 deltas are all stored in a buffer. a time stamp of the touch and release must be marked
    // by the IQS572 events.
    // t0 = touch on IQS572
    // ts = swipe event on IQS572 (used for axis calculations)
    // tn = n samples after swipe event: if performance is slow then this point can be x samples after the swipe event
    // tn = touch release on the IQS572
    //uint8_t t_0 = 0;
    //uint8_t t_swipe = 7;
//test 1
    uint8_t t_n = 17;
//test 2
//    uint8_t t_n = 9;
    uint8_t i = 0;
    Point_2D com = {0};
    Point_2D com_temp = {0};


    /* array of n samples then a COM is calculated again */
    for (i = 0; i < t_n; i++)
    {
        // elements 8, so 8 multiplication / 8 additions
        com_temp = calculateCOMBruteForce(i);
        printf("x: %f, y: %f :",com_temp.x,com_temp.y);
        com.x += com_temp.x;
        com.y += com_temp.y;
        printf(", x: %f, y: %f\n",com.x,com.y);
    }
    com.x = (float)(com.x / (float)t_n);
    com.y = (float)(com.y / (float)t_n);
    return com;
}



int8_t determineSign(Point_2D *t, Point_2D p)
{
    float determinant = 0;
    Point_2D cross_product[2] = {{0}};
/*
    | ax ay 1 |
    | bx by 1 |
    | cx cy 1 |
*/
    //  calculate the determinant of the two vectors
    cross_product[0].x = t[0].x - p.x;
    cross_product[0].y = t[0].y - p.y;
    cross_product[1].x = t[1].x - p.x;
    cross_product[1].y = t[1].y - p.y;

    // now calculate the determinant so that the sign can be evaluated
    determinant = (cross_product[0].x  * cross_product[1].y) - (cross_product[0].y * cross_product[1].x );

    printf("| %f   %f |\n",cross_product[0].x, cross_product[0].y);
    printf("| %f   %f |\n",cross_product[1].x, cross_product[1].y);
    printf("determinant = %f\n", determinant);

    // just clamp clamp the determinant
    if ((determinant < BOUNDRY) && (determinant > BOUNDRY))
    {
        determinant = 0;
    }

    // now determine the position of the point
    if (determinant < 0)        // the point it below the line, when FW is left to right
        return -1;
    if (determinant == 0)
        return 0;
    return 1;
}

Gesture_Direction evaluateSign(int8_t position, Gesture_Direction gesture)
{
    /* this function is called with the FW assumption and is either confirmed or changed */
    if (position > 0){
        gesture = BW;
    }
    else if (position < 0){
        gesture = FW;
    }
    else{
        gesture = UNDEFINED;
    }
    return gesture;
}

Gesture_Direction evaluateSwipe(Point_2D *trajectory)
{
    Point_2D point_com = {0};
    int8_t position = 0;
    Gesture_Direction gesture = UNDEFINED;
    // step 1 : determine point sign for each of the 3 line
    if (FUZZY_BOUNDRY)
    {
        // loop sign check 3 times around the main trajectory axis
    }
    else
    {
        point_com = determineCOM();
        printf("COM = (%f, %f)\n", point_com.x, point_com.y);
        // only check the sign on the main trajectory axis
        position = determineSign(trajectory, point_com);
        printf("sign = %d\n", position);

        // now check the direction of the swipe VS the sign and give FW or BW result
        gesture = evaluateSign(position, FW);

    }
    return gesture;
}

void testUpdateHistoryBuffer(void)
{
   // History_Buffer[][]


}

// just use the math.h library in the stm32 code
int main()
{
    Gesture_Direction gesture = UNDEFINED;  // default direction is undefined

    Point_2D trajectory[2] = {{0,0},{0,0}};

    TouchData_verifyDatabase();

    generatePoints(trajectory);

    translateTPpoint(&trajectory[0]);
    translateTPpoint(&trajectory[1]);

    printf("A(%f, %f), B(%f, %f)\n",trajectory[0].x,trajectory[0].y,trajectory[1].x,trajectory[1].y);

    scaleTrackPadPoints(trajectory);
    printf("A(%f, %f), B(%f, %f)\n",trajectory[0].x,trajectory[0].y,trajectory[1].x,trajectory[1].y);

    testUpdateHistoryBuffer();

    // the decision Algorithm start here
    gesture = evaluateSwipe(trajectory);

    printGesture(gesture);

    {
        /* test code for testing the touch data component */
        int rows = TouchData_get_numberOfDataRows();
        int start_x, start_y;
        int end_x, end_y;
        TouchData_get_raw_startPoint_coordinate(0, &start_x, &start_y);
        TouchData_get_raw_endPoint_coordinate(0, &end_x, &end_y);

        printf("number of data row is: %d\n\r", rows);
        printf("start_x is: %d, start_y is: %d\n\r", start_x, start_y);
        printf("end_x is: %d, end_y is: %d\n\r", end_x, end_y);


    }


    return 0;
}
