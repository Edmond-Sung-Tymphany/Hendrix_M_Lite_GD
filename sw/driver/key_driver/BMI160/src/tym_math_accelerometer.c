/**
*  @file      tym_math_accelerometer.c
*  @brief     convert g value to degree
*  @author    Paris.Chen
*  @date      2017/02/24
*  @copyright Tymphany Ltd.
*/
#include "commonTypes.h"
#include "trace.h"

typedef struct
{
    int16 degree;
    int32 milliG;
} arcsin_map_t;

int8 tym_math_abs_int8(int8 i)
{      /* compute absolute value of int argument */
    return (i < 0 ? -i : i);
}

int32 tym_math_abs_int32(int32 i)
{      /* compute absolute value of int argument */
    return (i < 0 ? -i : i);
}

static const arcsin_map_t arcsin_table[] =
{
    {0,0},
    {1,17},
    {2,35},
    {3,52},
    {4,70},
    {5,87},
    {6,105},
    {7,122},
    {8,139},
    {9,156},
    {10,174},
    {11,191},
    {12,208},
    {13,225},
    {14,242},
    {15,259},
    {16,276},
    {17,292},
    {18,309},
    {19,326},
    {20,342},
    {21,358},
    {22,375},
    {23,391},
    {24,407},
    {25,423},
    {30,500},
    {35,574},
    {40,643},
    {45,707},
    {50,766},
    {55,819},
    {60,866},
    {65,906},
    {70,939},
    {75,965},
    {80,984},
    {85,996},
    {90,1000},
};

#define MAX_ARCSIN_INDEX (sizeof(arcsin_table) / sizeof(arcsin_map_t))
#define MAX_DEGREE (arcsin_table[MAX_ARCSIN_INDEX-1].degree)
int8 tym_math_calculate_degree_from_miliG(int32 mG)
{
    uint8 ii;
    int8 degree;
    int32 tempValue = mG;
    bool fIsNeg = false;

    if(tempValue == 0)
    {
        degree = 0;
        return degree;
    }

    if(tempValue < 0)
    {
        fIsNeg = true;
        tempValue = tym_math_abs_int32(tempValue);
    }

    if(tempValue > arcsin_table[MAX_ARCSIN_INDEX-1].milliG)
    {
        if(fIsNeg == true)
        {
            degree = -MAX_DEGREE;
        }
        else
        {
            degree = MAX_DEGREE;
        }
        return degree;
    }

    ii = 0;
    while (ii < MAX_ARCSIN_INDEX)
    {
        if(tempValue > arcsin_table[ii].milliG)
        {
            ii++;
        }
        else if(tempValue < arcsin_table[ii].milliG)
        {
            if ((arcsin_table[ii].milliG - tempValue) <= (tempValue - arcsin_table[ii-1].milliG))
            {
                degree = arcsin_table[ii].degree;
                break;
            }
            else
            {
                degree = arcsin_table[ii-1].degree;
                break;
            }
        }
        else
        {
            degree = arcsin_table[ii].degree;
            break;
        }
    }

    if(fIsNeg == true)
    {
        degree = -degree;
    }
    return degree;
}


