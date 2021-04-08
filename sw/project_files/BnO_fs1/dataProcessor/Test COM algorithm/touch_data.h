#ifndef TOUCH_DATA_H
#define TOUCH_DATA_H

#define DATA_ROW_LENGTH 1216

#include <stdint.h>
#include "Gestures.h"
int TouchData_get_numberOfDataRows();
void TouchData_get_HistoryBuffer(int dataIdx, uint16_t** pHistoryBuffer, int* p_Q_length);
void TouchData_get_raw_startPoint_coordinate(int dataIdx, int* p_x, int* p_y);
void TouchData_get_raw_endPoint_coordinate(int dataIdx, int* p_x, int* p_y);
int TouchData_get_groundTruth(int dataIdx);
int TouchData_verifyDatabase();
Gesture_UserAngle TouchData_get_actualUserAngle(int dataIdx);
#endif
