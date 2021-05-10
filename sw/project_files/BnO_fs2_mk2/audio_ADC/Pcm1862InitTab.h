/**
* @file Pcm1862InitTab.h
* @brief The devices attached to the product.
* @author Daniel Qin
* @date 26-Mar-2015
* @copyright Tymphany Ltd.
*/

#include "AdcDrv_pcm1862_priv.h" //PCM1862_AUTO_GAIN_MAPPING



typedef unsigned char cfg_u8;
typedef union {
    struct {
        cfg_u8 offset;
        cfg_u8 value;
    };
    struct {
        cfg_u8 command;
        cfg_u8 param;
    };
} cfg_reg;

/* Example C code */
/*
    // Externally implemented function that can write n-bytes to the device
    // PCM51xx and TAS5766 targets require the high bit (0x80) of the I2C register to be set on multiple writes.
    // Refer to the device data sheet for more information.
    extern int i2c_write(unsigned char *data, int n);
    // Externally implemented function that delays execution by n milliseconds
    extern int delay(int n);
    // Example implementation.  Call like:
    //     transmit_registers(registers, sizeof(registers)/sizeof(registers[0]));
    void transmit_registers(cfg_reg *r, int n)
    {
        int i = 0;
        while (i < n) {
            switch (r[i].command) {
            case CFG_META_SWITCH:
                // Used in legacy applications.  Ignored here.
                break;
            case CFG_META_DELAY:
                delay(r[i].param);
                break;
            case CFG_META_BURST:
                i2c_write((unsigned char *)&r[i+1], r[i].param);
                i += r[i].param/2 +1;
                break;
            default:
                i2c_write((unsigned char *)&r[i], 2);
                break;
            }
            i++;
        }
    }
 */
/*volume steps product related, please fill in volume steps you need*/
const uint8 PCM1862_MIXER_GAIN_TABLE[][3] = 
{
  {0x10, 0x00, 0x00},           /*0dB*/
  {0x0E, 0x42, 0x90},           /*-1dB*/
  {0x0C, 0xB5, 0x91},           /*-2dB*/
  {0x0B, 0x53, 0xBE},           /*-3dB*/
  {0x0A, 0x18, 0x66},           /*-4dB*/
  {0x08, 0xFF, 0x59},           /*-5dB*/
  {0x08, 0x04, 0xDC},           /*-6dB*/
  {0x07, 0x25, 0x9D},           /*-7dB*/
  {0x06, 0x5E, 0xA5},           /*-8dB*/
  {0x05, 0xAD, 0x50},           /*-9dB*/
  {0x05, 0x0F, 0x44},           /*-10dB*/
  {0x04, 0x82, 0x68},           /*-11dB*/
  {0x04, 0x04, 0xDE},           /*-12dB*/
  {0x03, 0x94, 0xFA},           /*-13dB*/
  {0x03, 0x31, 0x42},           /*-14dB*/
  {0x02, 0xD8, 0x62},           /*-15dB*/
  {0x02, 0x89, 0x2C},           /*-16dB*/
  {0x02, 0x42, 0x93},           /*-17dB*/
  {0x02, 0x03, 0xA7},           /*-18dB*/
  {0x01, 0xCB, 0x94},           /*-19dB*/
  {0x01, 0x99, 0x99},           /*-20dB*/
  {0x01, 0x6D, 0x0E},           /*-21dB*/
  {0x01, 0x45, 0x5B},           /*-22dB*/
  {0x01, 0x21, 0xF9},           /*-23dB*/
  {0x01, 0x02, 0x70},           /*-24dB*/
  {0x00, 0xE6, 0x55},           /*-25dB*/
  {0x00, 0xCD, 0x49},           /*-26dB*/
  {0x00, 0xB6, 0xF6},           /*-27dB*/
  {0x00, 0xA3, 0x10},           /*-28dB*/
  {0x00, 0x00, 0x68},           /*-80dB*/
};

cfg_reg audioAdcInitTab[] = {
    { 0x00, 0x00 }, //page 0
    { 0x01, 0xe8 }, //-12.0dB (Min), remember to adjust volume after init
    { 0x02, 0xe8 },  
    { 0x03, 0x00 },
    { 0x04, 0x00 },
    { 0x05, 0xde }, //AGC disable
    { 0x06, 0x40 },
    { 0x07, 0x40 },
    { 0x08, 0x42 },
    { 0x09, 0x42 },
    { 0x0a, 0x00 },
    { 0x0b, 0xcc },
    { 0x0c, 0x00 },
    { 0x0d, 0x00 },
    { 0x0e, 0x00 },
    { 0x0f, 0xe8 },
    { 0x10, 0x01 },
    { 0x11, 0x20 },
    { 0x12, 0x00 },
    { 0x13, 0x00 },
    { 0x14, 0x00 },
    { 0x15, 0x00 },
    { 0x16, 0xe8 },
    { 0x17, 0x00 },
    { 0x18, 0x00 },
#ifdef PCM1862_AUTO_GAIN_MAPPING
    { 0x19, 0x00 }, //auto gain mapping0, use 0x10 to control global gain
#else    
    { 0x19, 0xFF }, //manually gain mapping0
#endif    
    { 0x1a, 0x00 },
    { 0x1b, 0x00 },
    { 0x1c, 0x00 },
    { 0x1d, 0x00 },
    { 0x1e, 0x00 },
    { 0x1f, 0x00 },
    { 0x20, 0x41 }, //master mode, clock from MCLK
    { 0x21, 0x00 },
    { 0x22, 0x00 },
    { 0x23, 0x01 },
    { 0x24, 0x50 },
    { 0x25, 0x07 },
    { 0x26, 0x03 },
    { 0x27, 0x3f },
    { 0x28, 0x11 },
    { 0x29, 0x01 },
    { 0x2a, 0x01 },
    { 0x2b, 0x08 },
    { 0x2c, 0x00 },
    { 0x2d, 0x00 },
    { 0x2e, 0x00 },
    { 0x2f, 0x00 },
    { 0x30, 0x00 },
    { 0x31, 0x00 },
    { 0x32, 0x00 },
    { 0x33, 0x01 },
    { 0x34, 0x00 },
    { 0x35, 0x00 },
    { 0x36, 0x01 },
    { 0x37, 0x00 },
    { 0x38, 0x00 },
    { 0x39, 0x00 },
    { 0x3a, 0x00 },
    { 0x3b, 0x00 },
    { 0x3c, 0x00 },
    { 0x3d, 0x00 },
    { 0x3e, 0x00 },
    { 0x3f, 0x00 },
    { 0x40, 0x80 },
    { 0x41, 0x7f },
    { 0x42, 0x00 },
    { 0x43, 0x80 },
    { 0x44, 0x7f },
    { 0x45, 0x00 },
    { 0x46, 0x80 },
    { 0x47, 0x7f },
    { 0x48, 0x00 },
    { 0x49, 0x80 },
    { 0x4a, 0x7f },
    { 0x4b, 0x00 },
    { 0x4c, 0x80 },
    { 0x4d, 0x7f },
    { 0x4e, 0x00 },
    { 0x4f, 0x80 },
    { 0x50, 0x7f },
    { 0x51, 0x00 },
    { 0x52, 0x80 },
    { 0x53, 0x7f },
    { 0x54, 0x00 },
    { 0x55, 0x80 },
    { 0x56, 0x7f },
    { 0x57, 0x00 },
    { 0x58, 0x00 },
    { 0x59, 0x00 },
    { 0x5a, 0x00 },
    { 0x5b, 0x00 },
    { 0x5c, 0x00 },
    { 0x5d, 0x00 },
    { 0x5e, 0x00 },
    { 0x5f, 0x00 },
    { 0x60, 0x01 },
    { 0x61, 0x00 },
    { 0x62, 0x10 },
    { 0x63, 0x00 },
    { 0x64, 0x00 },
    { 0x65, 0x00 },
    { 0x66, 0x00 },
    { 0x67, 0x00 },
    { 0x68, 0x00 },
    { 0x69, 0x00 },
    { 0x6a, 0x00 },
    { 0x6b, 0x00 },
    { 0x6c, 0x00 },
    { 0x6d, 0x00 },
    { 0x6e, 0x00 },
    { 0x6f, 0x00 },
    { 0x70, 0x70 },
    { 0x71, 0x10 },
    { 0x72, 0x0f },
    { 0x73, 0x03 },
    { 0x74, 0x32 },
    { 0x75, 0x00 },
    { 0x76, 0x01 },
    { 0x77, 0xcc },
    { 0x78, 0x07 },
    { 0x79, 0x00 },
    { 0x7a, 0x00 },
    { 0x7b, 0x00 },
    { 0x7c, 0x00 },
    { 0x7d, 0x00 },
    { 0x7e, 0x00 },
    { 0x7f, 0x00 },
    { 0x00, 0x01 },
    { 0x01, 0x00 },
    { 0x02, 0x17 },
    { 0x03, 0x00 },
    { 0x04, 0x00 },
    { 0x05, 0x00 },
    { 0x06, 0x00 },
    { 0x07, 0x00 },
    { 0x08, 0x00 },
    { 0x09, 0x00 },
    { 0x0a, 0x00 },
    { 0x0b, 0x00 },
    { 0x0c, 0x00 },
    { 0x0d, 0x00 },
    { 0x0e, 0x00 },
    { 0x0f, 0x00 },
};
