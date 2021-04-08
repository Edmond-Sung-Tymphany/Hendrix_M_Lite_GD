#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H
#include "plib.h"

#define MICROCHIP_PIC_32

#if defined MICROCHIP_PIC_18
typedef unsigned char  uchar;  /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  bool;   /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  uint8;  /*<! unsigned char   8 bits [0,255] */
typedef signed   char  int8;   /*<! signed   char   8 bits [-128,127] */
typedef unsigned short uint16; /*<! unsigned short 16 bits [0,65535]*/
typedef signed   short int16;  /*<! signed   short 16 bits [-32768,32767]*/
typedef unsigned long  uint32; /*<! unsigned long  32 bits [0,4294967295]*/
typedef signed   long  int32;  /*<! signed   long  32 bits [-2147483648, 2147483647]*/

#define TRUE  ((bool)1)
#define FALSE ((bool)0)
#define NULL  ((void*)0)




#define BE_TO_LE_16_BITS(x) (((x<<8)&0xFF00)|(x&0x00FF))
#define GET_LSB_16_BITS(x)  (x&0x00FF)
#define GET_MSB_16_BITS(x)  ((x>>8)&0x00FF)

#elif defined MICROCHIP_PIC_24
#include <string.h>

typedef unsigned char  uchar;  /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  bool;   /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  uint8;  /*<! unsigned char   8 bits [0,255] */
typedef signed   char  int8;   /*<! signed   char   8 bits [-128,127] */
typedef unsigned short uint16; /*<! unsigned short 16 bits [0,65535]*/
typedef signed   short int16;  /*<! signed   short 16 bits [-32768,32767]*/
typedef unsigned long  uint32; /*<! unsigned long  32 bits [0,4294967295]*/
typedef signed   long  int32;  /*<! signed   long  32 bits [-2147483648, 2147483647]*/

#define TRUE  ((bool)1)
#define FALSE ((bool)0)

#define BE_TO_LE_16_BITS(x) (((x<<8)&0xFF00)|(x&0x00FF))
#define GET_LSB_16_BITS(x)  (x&0x00FF)
#define GET_MSB_16_BITS(x)  ((x>>8)&0x00FF)

#elif defined PC_PLATFORM

#include <stddef.h>
typedef unsigned char     uchar;
typedef unsigned char     bool;
typedef unsigned char     uint8;
typedef signed   char     int8;
typedef unsigned short    uint16;
typedef signed   short    int16;
typedef unsigned long     uint32;
typedef signed   long     int32;

#define TRUE  (1)
#define FALSE (0)

#define BE_TO_LE_16_BITS(x) (((x<<8)&0xFF00)|(x&0x00FF))
#define GET_LSB_16_BITS(x)  (x&0x00FF)
#define GET_MSB_16_BITS(x)  ((x>>8)&0x00FF)

#elif defined MICROCHIP_PIC_32
#include <string.h>

typedef unsigned char  uchar;  /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  bool;   /*<! unsigned char   8 bits [0,255] */
typedef unsigned char  uint8;  /*<! unsigned char   8 bits [0,255] */
typedef signed   char  int8;   /*<! signed   char   8 bits [-128,127] */
typedef unsigned short uint16; /*<! unsigned short 16 bits [0,65535]  */
typedef signed   short int16;  /*<! signed   short 16 bits [-32768,32767]*/
typedef unsigned long  uint32; /*<! unsigned long  32 bits [0,4294967295]*/
typedef signed   long  int32;  /*<! signed   long  32 bits [-2147483648, 2147483647]*/

//#define TRUE  ((bool)1)
//#define FALSE ((bool)0)

#define UP     1
#define DOWN   0

#define HIGH  (1)
#define LOW   (0)

//#define ON        1
//#define OFF       0

#define BE_TO_LE_16_BITS(x) (((x<<8)&0xFF00)|(x&0x00FF))
#define GET_LSB_16_BITS(x)  (x&0x00FF)
#define GET_MSB_16_BITS(x)  ((x>>8)&0x00FF)
#else
#error "Common types not defined for your platform. See commonTypes.h"
#endif

#endif
