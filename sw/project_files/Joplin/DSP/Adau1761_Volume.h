#ifndef __ADAU1761_VOLUME_H__
#define __ADAU1761_VOLUME_H__

/* Please refer to Joplin_volume table.xlsx */
const static uint32_t DSP_Gain_Signed_Format_5_23_Table[MAX_VOLUME_STEPS] =
{
    0x0000003B, // step 0, -103dB
    0x00001A07,
    0x00001D34,
    0x000024C4,
    0x000033EF,
    0x0000495C,
    0x000061D3,
    0x00008274,
    0x0000ADF6,
    0x0000DB01,
    0x000113B5,
    0x00015B19,
    0x0001B4F8,
    0x0002261C,
    0x00028DCF,
    0x0003090D,
    0x00039B87,
    0x0004499D,
    0x00051884,
    0x00060E6C,
    0x000732AE,
    0x00081385,
    0x00090FCC,
    0x000AC515,
    0x000E5CA1,
    0x001326DD,
    0x00198A13,
    0x00220EAA,
    0x002D6A86,
    0x003C9038,
    0x0050C336,
    0x006BB2D6,
    0x00800000, // step 32, 0dB
};

#endif  // __ADAU1761_VOLUME_H__

