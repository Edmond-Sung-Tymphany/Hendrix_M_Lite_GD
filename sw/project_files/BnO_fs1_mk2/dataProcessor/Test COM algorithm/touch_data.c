#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "touch_data.h"
#include "Gestures.h"
#define assert(x)       if(!(x)) {printf("asserted at: %s, %d\n\r",__FILE__, __LINE__); exit(-1);}

//#include "touch_DB.c"
#include "touch_DB_allDirections_May9.c"
//#include "touch_DB_allDirections_testUser_May11.c"
//#include "touch_DB_one_near_swipe.c"
//#include "touch_DB_180_5far_5center_5near.c"
//#include "touch_DB_000_3far_3center_3near.c"
//#include "touch_DB_090_3far_3center_3near.c"
//#include "touch_DB_NEG090_3far_3center_3near.c"

/* this function tells you how much rows of data set there are.
 * note that one data row for one touch swipe */
int TouchData_get_numberOfDataRows()
{
    return (sizeof(touch_database) / DATA_ROW_LENGTH);
}

/* this function fill in the History_Buffer for you */
void TouchData_get_HistoryBuffer(int dataIdx, uint16_t** pHistoryBuffer, int* p_Q_length)
{
    assert(dataIdx < TouchData_get_numberOfDataRows());
    assert(pHistoryBuffer);
    assert(p_Q_length);
    {
        const char* tmp = &touch_database[dataIdx][3]; /* 3 for the offset of History_Buffer in the array */
        memcpy(pHistoryBuffer, tmp, 50*12*2);    /* NR_OF_SAMPLES * HISTORY_BUFFER_SAMPLES * 2 bytes for each sample */
    }
    {
        const char* tmp = &touch_database[dataIdx][0];
        *p_Q_length = (tmp[2]<<8) + tmp[1];
    }
    return;
}

void TouchData_get_raw_startPoint_coordinate(int dataIdx, int* p_x, int* p_y)
{
    const unsigned char* tmp = (const unsigned char*)(&touch_database[dataIdx][0]);
    assert(dataIdx < TouchData_get_numberOfDataRows());
    assert(p_x);
    assert(p_y);
    *p_x = tmp[1203] + (tmp[1204]<<8);     /* 1204 and 1205 are the offset to where the start point data is */
    *p_y = tmp[1205] + (tmp[1206]<<8);     /* 1206 and 1207 are the offset to where the start point data is */
    return;
}

void TouchData_get_raw_endPoint_coordinate(int dataIdx, int* p_x, int* p_y)
{
    const unsigned char* tmp = (const unsigned char*)&touch_database[dataIdx][0];
    assert(dataIdx < TouchData_get_numberOfDataRows());
    assert(p_x);
    assert(p_y);
    *p_x = tmp[1207] + (tmp[1208]<<8);     /* 1208 and 1209 are the offset to where the start point data is */
    *p_y = tmp[1209] + (tmp[1210]<<8);     /* 1210 and 1211 are the offset to where the start point data is */
    return;
}

Gesture_UserAngle TouchData_get_actualUserAngle(int dataIdx)
{
    const unsigned char* tmp = (const unsigned char*)(&touch_database[dataIdx][0]);
    assert(dataIdx < TouchData_get_numberOfDataRows());
    Gesture_UserAngle rtn = (tmp[0]>>4) & 0xff;
    if(rtn >= ANGLE_MAX)
    {
        printf("invalid ground truth user angle at data entry: %d, now assert\n\r", dataIdx);
        assert(0);
    }
    return rtn;
}

Gesture_Direction TouchData_get_groundTruth(int dataIdx)
{
    const unsigned char* tmp = (const unsigned char*)(&touch_database[dataIdx][0]);
    assert(dataIdx < TouchData_get_numberOfDataRows());
    Gesture_Direction rtn = 0;
    int value = tmp[0]&0x0f;
    if(value == 1)
        rtn = BW;
    else if(value == 2)
        rtn = FW;
    else
    {
        printf("invalid ground truth information in the data set #%d, it should be either 1 or 2 but i got %d, now assert\n\r", dataIdx, tmp[0]&0x0f);
        assert(0);
    }


    return rtn;
}

int TouchData_verifyDatabase()
{
    int i;
    for(i=0 ; i< TouchData_get_numberOfDataRows() ; i++)
    {
        /* this does checksum for the bytes from offset 1, all the way till the end. */
        /* note that the first bytes does not count as it is a place holder for user to fill in the ground truth. */
        const unsigned char* tmp = (const unsigned char*)(&touch_database[i][0]);
        int j;
        unsigned char checksum = tmp[1];
        int data_length_for_checksum = DATA_ROW_LENGTH - 1;
        for(j=2 ; j<data_length_for_checksum ; j++)
        {
            checksum ^= tmp[j];
        }
        if(checksum != tmp[DATA_ROW_LENGTH-1])
        {
            printf("checksum incorrect on the data row: %d (zero indexing)\n\r", i);
            assert(0);
        }

    }
    printf("Data verification ok\n\r");
    return 0;
}
