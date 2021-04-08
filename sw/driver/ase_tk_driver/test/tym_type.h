/**
*  @file      tym_type.h
*  @brief    Common type define
*  @author    Bill Wu
*  @date      11 -2013
*  @copyright Tymphany Ltd.
*/

#ifndef __TYM_TYPE_H__
#define __TYM_TYPE_H__

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned int  uint32;
typedef unsigned short uint16;
typedef long           int32;
typedef short          int16;
typedef signed char    int8;
typedef unsigned char  uint8;

#define ARRAY_LENGTH(x)  (sizeof(x) / sizeof(x[0]))
#define min(X,Y) ((X) < (Y) ? (X) : (Y))
#define max(X,Y) ((X) > (Y) ? (X) : (Y))

#define DEBUG_LOG(x, ...)                       \
                                do {            \
                                    printf("%4d: " #x "\n", __LINE__, ##__VA_ARGS__);   \
                                }while(0);      \

#endif /* __TYM_TYPE_H__ */

