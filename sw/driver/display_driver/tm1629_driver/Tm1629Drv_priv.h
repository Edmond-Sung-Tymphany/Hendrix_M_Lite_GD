/**
 * @file        Tm1629Drv_priv.h
 * @brief       Nothing
 * @author      Johnny Fan 
 * @date        2014-03-10
 * @copyright   Tymphany Ltd.
 */
#ifndef TM1629_DRIVER_PRIVATE_H
#define TM1629_DRIVER_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "DisplayDrv.h"

#define MAX_BRIGHTNESS_LEVEL           7
#define W_REG_ADDR_AUTO_INCREASE_CMD   0x40  /* Write register in address auto increate mode */
#define W_REG_FIX_ADD_CMD              0x44  /* Write register in fix address increate mode */
#define SET_DISPLAY_ADDR_CMD           0xC0
#define SET_DISPLAY_CMD                0x80
#define DISPLAY_ON_CMD                 0x08
#define MAX_DISPLAY_ADDR               0x0F

/*
TM1629_PIN_STB = me->gpioDrv->gpioConfig.tGPIOPins[0].GPIO_10
TM1629_PIN_CLK = me->gpioDrv->gpioConfig.tGPIOPins[1].GPIO_11
TM1629_PIN_DIO = me->gpioDrv->gpioConfig.tGPIOPins[2].GPIO_12
*/
#define TM1629_PIN_STB  me->gpioDrv->gpioConfig->pGPIOPinSet[0].gpioId
#define TM1629_PIN_CLK  me->gpioDrv->gpioConfig->pGPIOPinSet[1].gpioId
#define TM1629_PIN_DIO  me->gpioDrv->gpioConfig->pGPIOPinSet[2].gpioId

  
#define delay_1us()             asm("nop"); asm("nop")
#define _CODE_DOT_H             0x00
#define _CODE_DOT_L             0x40
#define CHAR_TABLE_MIN_ENTRY    0x1B    // ASCII : RS ( Record separator )
#define CHAR_TABLE_MAX_ENTRY    0x7A    // '''
#define MAX_DISPLAY_DIGITS      0x08

#define CAPITAL_SMALL_LETTER_DIFFERENCE (uint8)('a'-'A')
#define CHAR_TABLE_SIZE         (uint8)((CHAR_TABLE_MAX_ENTRY - CHAR_TABLE_MIN_ENTRY) + 1)
#define NOT_SUPPORT_CHAR_MAP_ADDR   5
#define NUM_OF_SEGMENT          17
  
#define SEGMENT_MAX_ID          18

/*******************************segments control**************************
  
          (0x0100)
          _______
         |   |   |             \           |          /
(0x2000) |\  |  /|(0x0200)      \          |         /
         | \ | / |               \         |        /
         |__\|/__|             (0x4000) (0x8000)  (0x0001)
         |  /|\  |              -------           -------
(0x1000) | / | \ |(0x0400)     (0x0020)           (0x0002)
         |/  |  \|               /         |        \
         |_______|              /          |         \
           (0x0800)            /           |          \          .
                            (0x0010)   (0x0008)   (0x0004)   (0x0040)
************************************************************************/

static const uint8 s_CharTable[CHAR_TABLE_SIZE][2]=
{
//   Byte       Byte
//   Low        High


    {0x23,      0x22},    // ascii 0x1B , used as the phase degree symbol
    {0x1C,      0x22},    // ascii 0x1C,  , -> 'o'     //
    {0x1E,      0x22},    // ascii 0x1D,  , -> 'd'     //

    {0x36,      0x00},    // ascii 0x1E,  , ->'11'     // fm stereo display
    {0x80,      0x08},    // ascii 0x1F,  , -> 'l'     // fm mono display
    {0x00,      0x00},    // ascii 0x20, ' ' [space]
    {0x00,0x00},    // ascii 0x21 , '!' not supported
    {0x00,0x00},    // ascii 0x22 , '"' not supported
    {0x00,0x00},    // ascii 0x23 , '#' not supported
    {0x00,0x00},    // ascii 0x24 , '$' not supported
    {0x00,0x00},    // ascii 0x25 , '%' not supported
    {0x00,0x00},    // ascii 0x26 , '&' not supported
    {0x00,0x00},    // ascii 0x27 , ''' not supported
    {0x30,      0x05},    // ascii 0x28 ,'('  // |<
    {0x46,      0x10},    // ascii 0x29 ,')'  // >|
    {0xC0,      0x3F},    // ascii 0x2A, '*'
    {0x80,      0x2A},    // ascii 0x2B, '+'
    {0x00,      0x10},    // ascii 0x2C, ','
    {0x00,      0x22},    // ascii 0x2D, '-'
    {0x00,      0x40},    // ascii 0x2E, '.'
    {0x00,      0x11},    // ascii 0x2F, '/'

    {0x3f,      0x11},    // ascii  '0'
    {0x80,      0x08},    // ascii  '1'
    {0x1b,      0x22},    // ascii  '2'
    {0x0f,      0x22},    // ascii  '3'
    {0x26,      0x22},    // ascii  '4'
    {0x2d,      0x22},    // ascii  '5', change to 'S' {0x29,0x24},
    {0x3d,      0x22},    // ascii  '6'
    {0x07,      0x00},    // ascii  '7'
    {0x3f,      0x22},    // ascii  '8'
    {0x2f,      0x22},    // ascii  '9'

    {0x00,0x00},    // ascii 0x3A, ':' , not supported
    {0x00,0x00},    // ascii 0x3B, ';' , not supported
    {0x00,      0x05},    // ascii 0x3C, '<'
    {0x00,0x00},    // ascii 0x3D, '=' , not supported
    {0x40,      0x10},    // ascii 0x3E, '>'
    {0x00,0x00},    // ascii 0x3F, '?' , not supported
    {0x00,0x00},    // ascii 0x40, '@' , not supported

    {0x37,      0x22},    // ascii  A
    {0x8f,      0x0a},    // ascii  B
    {0x39,      0x00},    // ascii  C
    {0x8f,      0x08},    // ascii  D
    {0x39,      0x22},    // ascii  E
    {0x31,      0x22},    // ascii  F
    {0x3D,      0x02},    // ascii  G
    {0x36,      0x22},    // ascii  H
    {0x89,      0x08},    // ascii  I
    {0x0E,      0x00},    // ascii  J
    {0x30,      0x25},    // ascii  K
    {0x38,      0x00},    // ascii  L
    {0x76,      0x01},    // ascii  M
    {0x76,      0x04},    // ascii  N
    {0x3F,      0x00},    // ascii  O
    {0x33,      0x22},    // ascii  P
    {0x3F,      0x04},    // ascii  Q
    {0x33,      0x26},    // ascii  R
    {0x29,      0x24},    // ascii  S
    {0x81,      0x08},    // ascii  T
    {0x3E,      0x00},    // ascii  U
    {0x30,      0x11},    // ascii  V
    {0x36,      0x14},    // ascii  W
    {0x40,      0x15},    // ascii  X
    {0x40,      0x09},    // ascii  Y
    {0x09,      0x11},    // ascii  Z

    {0x00,0x00},    // ascii 0x5B, '[' , not supported
    {0x40,      0x04},    // ascii 0x5C, '\'
    {0x00,0x00},    // ascii 0x5D, ']' , not supported
    {0x00,0x00},    // ascii 0x5E, '^' not supported
    {0x00,0x00},    // ascii 0x5F,  not supported
    {0x40,      0x00},    // ascii 0x60, '''

    {0x37,      0x22},    // ascii  a
    {0x3c,      0x22},    // ascii  b
    {0x18,      0x22},    // ascii  c
    {0x1E,      0x22},    // ascii  d
    {0x39,      0x22},    // ascii  e
    {0x31,      0x22},    // ascii  f
    {0x3D,      0x02},    // ascii  g
    {0x36,      0x22},    // ascii  h
    {0x89,      0x08},    // ascii  i
    {0x0E,      0x00},    // ascii  j
    {0x30,      0x25},    // ascii  k
    {0x38,      0x00},    // ascii  l
    {0x76,      0x01},    // ascii  m
    {0x76,      0x04},    // ascii  n
    {0x3F,      0x00},    // ascii  o
    {0x33,      0x22},    // ascii  p
    {0x3F,      0x04},    // ascii  q
    {0x33,      0x26},    // ascii  r
    {0x29,      0x24},    // ascii  s
    {0x81,      0x08},    // ascii  t
    {0x3E,      0x00},    // ascii  u
    {0x30,      0x11},    // ascii  v
    {0x36,      0x14},    // ascii  w
    {0x40,      0x15},    // ascii  x
    {0x40,      0x09},    // ascii  y
    {0x09,      0x11},    // ascii  z
};
 
static const uint8 segmentTable[NUM_OF_SEGMENT][2]=
{
  {0x20,0x00},
  {0x01,0x00},
  {0x02,0x00},
  {0x04,0x00},
  {0x08,0x00},
  {0x10,0x00},
  {0x40,0x00},
  {0x80,0x00},
  {0x00,0x01},
  {0x00,0x02},
  {0x00,0x04},
  {0x00,0x08},
  {0x00,0x10},
  {0x00,0x20},
  {0x00,0x40},
  {0xff,0x7f},//all segments on
  {0x00,0x00},//all segments off
};

/* Private functions / data */

static void Tm1629Drv_SendData(cDisplayDrv *me, uint8 segdata);
static uint8 Tm1629Drv_GetCharTableIndex(cDisplayDrv *me, uchar ch);
static void Tm16290Drv_SendDisplayCmd(cDisplayDrv *me, uint8 screenPos, const uint8 *pMapArray);

#ifdef __cplusplus
}
#endif

#endif /* TM1629_DRIVER_PRIVATE_H */
