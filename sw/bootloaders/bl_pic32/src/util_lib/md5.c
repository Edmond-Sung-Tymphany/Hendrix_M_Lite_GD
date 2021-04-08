/**
 * @file      md5_string.c
 * @brief     The main source file for hex file parsing
 * @author    Gavin Lee
 * @date      14-Aug-2015
 * @copyright Tymphany Ltd.
 */

/* See RFC 1321 for a description of the MD5 algorithm.
 */


/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <p32xxxx.h>
#include <plib.h>
#include <GenericTypeDefs.h>
#include "Bootloader.h"
#include "HardwareProfile.h"
#include "util.h" //DBG_PRINT
#include "hex.h"
# include <string.h>
#include "assert.h"
#include "dbgprint.h"
#include <md5.h>


/*****************************************************************************
 * Defination                                                                *
 *****************************************************************************/
//#define MD5_TEST

#define cpu_to_le32(x) (x)
#define le32_to_cpu(x) cpu_to_le32(x)

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits.
 */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x >> (32 - (n)))))



/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/
static uint32 initstate[4] =
{
  0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476
};

static char s1[4] = {  7, 12, 17, 22 };
static char s2[4] = {  5,  9, 14, 20 };
static char s3[4] = {  4, 11, 16, 23 };
static char s4[4] = {  6, 10, 15, 21 };

static uint32 T[64] =
{
  0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
  0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
  0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
  0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
  0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
  0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
  0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
  0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
  0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
  0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
  0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
  0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
  0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
  0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
  0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
  0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

static uint32 state[4];
static unsigned int length;
static unsigned char buffer[64];



/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
static void md5_transform (const unsigned char block[64]);



/*****************************************************************************
 * Public Function Implemenation                                             *
 *****************************************************************************/
char* md5_to_str(unsigned char* md5_buf)
{
    static char str[100];
    snprintf(str, sizeof(str), "%02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X %02X%02X%02X%02X",
                       md5_buf[0], md5_buf[1], md5_buf[2],  md5_buf[3],  md5_buf[4],  md5_buf[5],  md5_buf[6],  md5_buf[7],
                       md5_buf[8], md5_buf[9], md5_buf[10], md5_buf[11], md5_buf[12], md5_buf[13], md5_buf[14], md5_buf[15]);
    return str;
}

void md5_init(void)
{
  memcpy ((char *) state, (char *) initstate, sizeof (initstate));
  length = 0;
}

void md5_update (const char *input, int inputlen)
{
  int buflen = length & 63;
  length += inputlen;
  if (buflen + inputlen < 64)
    {
      memcpy (buffer + buflen, input, inputlen);
      buflen += inputlen;
      return;
    }

  memcpy (buffer + buflen, input, 64 - buflen);
  md5_transform (buffer);
  input += 64 - buflen;
  inputlen -= 64 - buflen;
  while (inputlen >= 64)
    {
      /* Tymphany: 
       *    Note the original code process data on input directly:
       *      md5_transform (input);
       *    but it have problem here. On PIC32 MCU, we must use a 4byte-align address to access uint32,
       *    but the input here is not 4bytes-align, and cause exception occurs. To avoid problem, we
       *    copy data to buffer then process it.
       */
      memcpy (buffer, input, 64);
      md5_transform (buffer);
      input += 64;
      inputlen -= 64;
    }
  memcpy (buffer, input, inputlen);
  buflen = inputlen;
}

unsigned char *md5_final()
{
  int i, buflen = length & 63;

  buffer[buflen++] = 0x80;
  memset (buffer+buflen, 0, 64 - buflen);
  if (buflen > 56)
    {
      md5_transform (buffer);
      memset (buffer, 0, 64);
      buflen = 0;
    }

  *(uint32 *) (buffer + 56) = cpu_to_le32 (8 * length);
  *(uint32 *) (buffer + 60) = 0;
  md5_transform (buffer);

  for (i = 0; i < 4; i++)
    state[i] = cpu_to_le32 (state[i]);
  return (unsigned char *) state;
}



/*****************************************************************************
 * Private Function Implemenation                                            *
 *****************************************************************************/
static void md5_transform (const unsigned char block[64])
{
  //On PIC32 MCU, a uint32 pointer must align to 4byte boundary
  assert( ((int)block)%4==0 );

  int i, j;
  uint32 a,b,c,d,tmp;
  const uint32 *x = (uint32 *) block;

  a = state[0];
  b = state[1];
  c = state[2];
  d = state[3];

  /* Round 1 */
  for (i = 0; i < 16; i++)
    {
      tmp = a + F (b, c, d) + le32_to_cpu (x[i]) + T[i];
      tmp = ROTATE_LEFT (tmp, s1[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 2 */
  for (i = 0, j = 1; i < 16; i++, j += 5)
    {
      tmp = a + G (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+16];
      tmp = ROTATE_LEFT (tmp, s2[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 3 */
  for (i = 0, j = 5; i < 16; i++, j += 3)
    {
      tmp = a + H (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+32];
      tmp = ROTATE_LEFT (tmp, s3[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }
  /* Round 4 */
  for (i = 0, j = 0; i < 16; i++, j += 7)
    {
      tmp = a + I (b, c, d) + le32_to_cpu (x[j & 15]) + T[i+48];
      tmp = ROTATE_LEFT (tmp, s4[i & 3]);
      tmp += b;
      a = d; d = c; c = b; b = tmp;
    }

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;
}


/*****************************************************************************
 * Test Function Implemenation                                               *
 *****************************************************************************/
#ifdef MD5_TEST
static char * md5_string (const char *input)
{
  md5_init();
  md5_update (input, strlen (input));
  return md5_final ();
}

static char * md5_binary (const unsigned char *buf, int len)
{
  md5_init();
  md5_update (buf, len);
  return md5_final ();
}

void md5_string_test (char *buffer, char *expected)
{
  char result[16 * 3 +1];
  unsigned char* digest = md5_string (buffer);
  int i;

  for (i=0; i < 16; i++)
    sprintf (result+2*i, "%02x", digest[i]);

  if (strcmp (result, expected))
    printf ("MD5(%s) failed: %s\n", buffer, result);
  else
    printf ("MD5(%s) OK\n", buffer);
}

void md5_binary_test (const unsigned char *buf, int len)
{
  char result[16 * 3 +1];
  unsigned char* digest = md5_binary (buf, len);
  int i;

  printf("md5_string: ");
  for (i=0; i < 16; i++)
    printf ("%02x ", digest[i]);

  printf("\n");
}

void md5_test (void){
  unsigned char buf[1]= { 0x10 };
  md5_binary_test(buf, 1);

  md5_string_test ("", "d41d8cd98f00b204e9800998ecf8427e");
  md5_string_test ("a", "0cc175b9c0f1b6a831c399e269772661");
  md5_string_test ("abc", "900150983cd24fb0d6963f7d28e17f72");
  md5_string_test ("message digest", "f96b697d7cb7938d525a2f31aaf161d0");
  md5_string_test ("abcdefghijklmnopqrstuvwxyz", "c3fcd3d76192e4007dfb496cca67e13b");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789","d174ab98d277d9f5a5611c2c9f419d9f");
  md5_string_test ("12345678901234567890123456789012345678901234567890123456789012345678901234567890", "57edf4a22be3c955ac49da2e2107b67a");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz3456",  "6831fa90115bb9a54fbcd4f9fee0b5c4");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz345", "bc40505cc94a43b7ff3e2ac027325233");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz34567", "fa94b73a6f072a0239b52acacfbcf9fa");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz345678901234", "bd201eae17f29568927414fa326f1267");
  md5_string_test ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz34567890123", "80063db1e6b70a2e91eac903f0e46b85");
}
#endif /* MD5_TEST */
