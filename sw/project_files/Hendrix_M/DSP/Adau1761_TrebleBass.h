#ifndef __ADAU1761_TREBLE_BASS_H__
#define __ADAU1761_TREBLE_BASS_H__


#ifdef DSP_TREBLE_BASS_TUNING
// 4 bytes for each parameter
#define TREBLE_BASS_PARAM_SIZE      4
// 5 'WORD' for each PEQ size
#define TREBLE_BASS_PARAM_LENGHT    5

#define TREBLE_BASS_PARAM_TOTAL_SIZES   (TREBLE_BASS_PARAM_SIZE*TREBLE_BASS_PARAM_LENGHT)

// filter mode : LSF, freq = 120Hz, Q=1.41
const static uint8_t Adau1761_BassTable[]=
{
//-6 dB
	0x00,	0x7F,	0x2E,	0x25,
	0xFF,	0x05,	0x99,	0xE5,
	0x00,	0x7B,	0x47,	0x84,
	0x00,	0xFA,	0x5E,	0x5D,
	0xFF,	0x85,	0x82,	0x99,


//-5 dB
	0x00,	0x7F,	0x51,	0x4A,
	0xFF,	0x05,	0x72,	0x56,
	0x00,	0x7B,	0x4C,	0xDC,
	0x00,	0xFA,	0x87,	0x40,
	0xFF,	0x85,	0x5B,	0x6F,



//-4 dB
	0x00,	0x7F,	0x74,	0x55,
	0xFF,	0x05,	0x4B,	0xEB,
	0x00,	0x7B,	0x51,	0x3A,
	0x00,	0xFA,	0xAE,	0xF9,
	0xFF,	0x85,	0x35,	0x55,



//-3 dB
	0x00,	0x7F,	0x97,	0x4B,
	0xFF,	0x05,	0x26,	0x9B,
	0x00,	0x7B,	0x54,	0x9F,
	0x00,	0xFA,	0xD5,	0x93,
	0xFF,	0x85,	0x10,	0x44,



//-1.5 dB
	0x00,	0x7F,	0xCB,	0xA7,
	0xFF,	0x04,	0xF0,	0xAA,
	0x00,	0x7B,	0x57,	0xE4,
	0x00,	0xFB,	0x0D,	0x6E,
	0xFF,	0x84,	0xDA,	0x8D,


// 0dB
    0x00,     0x80,     0x00,     0x00,
    0xFF,     0x04,     0xBD,     0x12,
    0x00,     0x7B,     0x58,     0xFB,
    0x00,     0xFB,     0x42,     0xEE,
    0xFF,     0x84,     0xA7,     0x05,


// +1.5dB
	0x00,	0x80,	0x34,	0x6E,
	0xFF,	0x04,	0x8B,	0xBC,
	0x00,	0x7B,	0x57,	0xE4,
	0x00,	0xFB,	0x76,	0x2C,
	0xFF,	0x84,	0x75,	0x96,


//+ 3 dB
	0x00,	0x80,	0x69,	0x0B,
	0xFF,	0x04,	0x5C,	0x95,
	0x00,	0x7B,	0x54,	0x9F,
	0x00,	0xFB,	0xA7,	0x41,
	0xFF,	0x84,	0x46,	0x2B,


//+ 4 dB
	0x00,	0x80,	0x8C,	0x44,
	0xFF,	0x04,	0x3E,	0x51,
	0x00,	0x7B,	0x51,	0x3B,
	0x00,	0xFB,	0xC6,	0xD1,
	0xFF,	0x84,	0x27,	0xA3,


//+ 5 dB,
	0x00,	0x80,	0xAF,	0xA5,
	0xFF,	0x04,	0x20,	0xF8,
	0x00,	0x7B,	0x4C,	0xDD,
	0x00,	0xFB,	0xE5,	0x7B,
	0xFF,	0x84,	0x09,	0xF1,


//+6 dB
	0x00,	0x80,	0xD3,	0x35,
	0xFF,	0x04,	0x04,	0x83,
	0x00,	0x7B,	0x47,	0x85,
	0x00,	0xFC,	0x03,	0x48,
	0xFF,	0x83,	0xED,	0x10,

};

// filter mode : HSF, freq = 6000Hz, Q=1.41
const static uint8_t Adau1761_TrebleTable[]=
{
//-6 dB
	0x00,	0x46,	0x84,	0xDE,
	0xFF,	0xA0,	0x38,	0x41,
	0x00,	0x24,	0x99,	0xE0,
	0x00,	0xC4,	0xBF,	0xF3,
	0xFF,	0xAF,	0xE9,	0x0E,



//-5 dB
	0x00,	0x4D,	0xE0,	0xC7,
	0xFF,	0x94,	0xE7,	0x19,
	0x00,	0x29,	0x26,	0x6F,
	0x00,	0xC3,	0x17,	0x77,
	0xFF,	0xB0,	0xFA,	0x3A,


//-4 dB
	0x00,	0x56,	0x02,	0x70,
	0xFF,	0x88,	0x4B,	0x64,
	0x00,	0x2E,	0x3F,	0x12,
	0x00,	0xC1,	0x64,	0x09,
	0xFF,	0xB2,	0x0F,	0x10,



//-3 dB
	0x00,	0x5E,	0xFE,	0x7A,
	0xFF,	0x7A,	0x40,	0x97,
	0x00,	0x33,	0xF4,	0x02,
	0x00,	0xBF,	0xA5,	0x73,
	0xFF,	0xB3,	0x27,	0x7A,



//-1.5 dB
	0x00,	0x6E,	0x44,	0x59,
	0xFF,	0x62,	0x26,	0x79,
	0x00,	0x3D,	0xD0,	0x69,
	0x00,	0xBC,	0xF2,	0x31,
	0xFF,	0xB4,	0xD2,	0x95,


//0 dB
    0x00,     0x80,     0x00,     0x00,
    0xFF,     0x45,     0xDB,     0x59,
    0x00,     0x49,     0x7A,     0xDC,
    0x00,     0xBA,     0x24,     0xA7,
    0xFF,     0xB6,     0x85,     0x24,

//+1.5 dB
	0x00,	0x94,	0x95,	0xB7,
	0xFF,	0x24,	0xAA,	0xF7,
	0x00,	0x57,	0x44,	0x72,
	0x00,	0xB7,	0x3C,	0x25,
	0xFF,	0xB8,	0x3E,	0xBA,


//+ 3 dB
	0x00,	0xAC,	0x79,	0x55,
	0xFE,	0xFD,	0xC4,	0x04,
	0x00,	0x67,	0x8B,	0xCC,
	0x00,	0xB4,	0x37,	0xFC,
	0xFF,	0xB9,	0xFE,	0xDE,

//+ 4 dB
	0x00,	0xBE,	0x7D,	0x93,
	0xFE,	0xE0,	0x31,	0xC9,
	0x00,	0x73,	0xFE,	0x1B,
	0x00,	0xB2,	0x25,	0x87,
	0xFF,	0xBB,	0x2D,	0x02,

//+ 5 dB
	0x00,	0xD2,	0x61,	0x58,
	0xFE,	0xBF,	0x59,	0x24,
	0x00,	0x81,	0xE1,	0x91,
	0x00,	0xB0,	0x06,	0x4C,
	0xFF,	0xBC,	0x5D,	0xA7,


//+6 dB
	0x00,	0xE8,	0x55,	0xA1,
	0xFE,	0x9A,	0xE0,	0x79,
	0x00,	0x91,	0x5F,	0x2A,
	0x00,	0xAD,	0xDA,	0x1E,
	0xFF,	0xBD,	0x90,	0x9E,


};

#endif

#endif  // __ADAU1761_TREBLE_BASS_H__

