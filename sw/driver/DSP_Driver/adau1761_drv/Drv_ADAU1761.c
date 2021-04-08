/**
*  @file      Drv_ADAU1761.c
*  @brief     This file implements the driver for the Audio DSP ADAU1761.
*  @modified  Daniel.Duan
*  @date      2014-09-15
*  @copyright Tymphany Ltd.
*/

#include <stdio.h>
#include <math.h>
#include "trace.h"
#include "cplus.h"
#include "commonTypes.h"
#include "DspDrv.h"
#include "I2CDrv.h"
#include "Drv_ADAU1761.h"
#include "Dsp_Init_Tab.h"
#include "Dsp_Tunable_Tab.h"


#ifndef NULL
#define NULL                             (0)
#endif

#ifndef DSPDRV_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif


#define PI                                   (3.1415926536)



typedef struct
{
    const uint8* head;     /*pointer to the tabe head*/
    uint8        type;     /*controller type*/
    uint8          id;     /*in case there can be more than 1 controller address in one array, we need to find the correct address*/
    uint16       addr;
} tTunableInfo;



/* private functions */
static void DSPDrv_FormatFiltData(int32* in, uint8* out, uint8 num);
static void DSPDrv_GetCtrlAddr(tTunableInfo *p);
static void DSPDrv_ChkMagicNum(const uint8 *p);
static void DSPDrv_Delay(uint16 count);
static void DSPDrv_DispatchDspConfigData(void* p);
static void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data);
static void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes);
static void DSPDrv_InitSection0(void *p);
static void DSPDrv_InitSection1(void *p);
static void DSPDrv_SetAnaInput(tDspAnaInCfg *p);
static void DSPDrv_SetAnaOutput(tDspAnaOutCfg *p);
static void DSPDrv_SetDigAudItf(tDspDigAudCfg *p);
static void DspDrv_SafeLoadData(uint8 num, const uint8 *array, uint16 addr);
static void DSPDrv_SetGpio0Func(tDspGpio0Cfg *p);
static bool DSPDrv_GetGpio0In(void);
static void DSPDrv_SetGpio0Out(bool out);
static void DSPDrv_SetGpio1Func(tDspGpio1Cfg *p);
static bool DSPDrv_GetGpio1In(void);
static void DSPDrv_SetGpio1Out(bool out);
static void DSPDrv_SetGpio2Func(tDspGpio2Cfg *p);
static bool DSPDrv_GetGpio2In(void);
static void DSPDrv_SetGpio2Out(bool out);
static void DSPDrv_SetGpio3Func(tDspGpio3Cfg *p);
static bool DSPDrv_GetGpio3In(void);
static void DSPDrv_SetGpio3Out(bool out);
static void DSPDrv_CalFiltParam(tDspFiltRawParam in, uint8* out);




static cI2CDrv dspI2c;

/**
 * Format input int32 data array into array of uint8, according to the designated number of int32
 *
 * @param      int32*          start address of the input int32 array
 *
 * @param      uint8*          output data array pointer
 *
 * @param      uint8           number of input int32 data
 *
 * @return     void
 */
static void DSPDrv_FormatFiltData(int32* in, uint8* out, uint8 num)
{
    uint32        temp;
    uint8          cnt;
    int32*        ptr0;
    uint8*        ptr1;

    ptr0 = in;
    ptr1 = out;

    for(cnt = 0; cnt < num; cnt++)
    {
        temp  = (uint32)(*ptr0);
        *ptr1 = (uint8)((temp >> 24) & 0xFF);
        ptr1++;
        *ptr1 = (uint8)((temp >> 16) & 0xFF);
        ptr1++;
        *ptr1 = (uint8)((temp >> 8) & 0xFF);
        ptr1++;
        *ptr1 = (uint8)(temp & 0xFF);
        ptr1++;
        ptr0++;
    }
}

static void DSPDrv_Delay(uint16 count)
{
    while(count--)
    {
        asm("nop");
        asm("nop");
    }

}

/**
 * Write data to DSP by I2C.
 *
 * @param      uint8           device I2C address
 *
 * @param      uint8           data byte number
 *
 * @param      const uint8*    pointer to data array
 *
 * @return     void
 */
static void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = device_add,
        .regAddr = NULL,
        .length  = bytes,
        .pMsg    = (uint8*)data
    };

    I2CDrv_MasterWrite(&dspI2c, &i2cMsg);
}

/**
 * Read data from DSP by I2C.
 *
 * @param      uint8*          buffer pointer to store received data
 *
 * @param      uint8           device address
 *
 * @param      uint16          register address
 *
 * @param      uint8           data byte number
 *
 * @return     void
 */
static void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint16 reg_add, uint16 bytes)
{
    tI2CMsg i2cMsg =
    {
        .devAddr = device_add,
        .regAddr = reg_add,
        .length  = bytes,
        .pMsg    = bufptr
    };

    I2CDrv_MasterReadWith2ByteRegAddress(&dspI2c, &i2cMsg);
}

/**
 * Safeload process
 *
 * @param      uint8                data length in word(4 bytes)
 * @param      uint8*               data array
 * @param      uint16               target address
 * @return     void
 */
static void DspDrv_SafeLoadData(uint8 num, const uint8 *array, uint16 addr)
{
    uint8            temp[6];
    uint8          *curr_ptr;
    uint8 curr_idx, curr_cnt;

    ASSERT((num <= SAFE_LOAD_DATA_SIZE_MAX) && (num > 0));

	for(curr_cnt = 0; curr_cnt < sizeof(temp); curr_cnt++)
    {
        temp[curr_cnt] = 0;
    }
    /* step1: set modulo size */
    curr_ptr = SAFE_LOAD_MODULO_SIZE_ADD;
    temp[0] = (uint8)(curr_ptr >> 8);
    temp[1] = (uint8)curr_ptr;
    temp[5] = num;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    curr_ptr++;
    for(curr_cnt = 0; curr_cnt < sizeof(temp); curr_cnt++)
    {
        temp[curr_cnt] = 0;
    }
    curr_idx = 0;
    /* step2: write safeload data */
    while(curr_idx < num*4)
    {
        temp[0] = (uint8)(curr_ptr >> 8);
        temp[1] = (uint8)curr_ptr;
        for(curr_cnt = 0; curr_cnt < 4; curr_cnt++)
        {
            temp[curr_cnt + 2] = array[curr_idx];
            curr_idx++;
        }

        DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
        curr_ptr++;
    }

    curr_ptr = SAFE_LOAD_ADD_FOR_TARGET_ADD;
    for(curr_cnt = 0; curr_cnt < sizeof(temp); curr_cnt++)
    {
        temp[curr_cnt] = 0;
    }
    /* step3: write target addr */
    temp[0] = (uint8)(curr_ptr >> 8);
    temp[1] = (uint8)curr_ptr;
    temp[4] = (uint8)((addr - 1) >> 8);
    temp[5] = (uint8)(addr - 1);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    /* step4: trigger safeload */
    for(curr_cnt = 0; curr_cnt < sizeof(temp); curr_cnt++)
    {
        temp[curr_cnt] = 0;
    }
    curr_ptr++;
    temp[0] = (uint8)(curr_ptr >> 8);
    temp[1] = (uint8)curr_ptr;
    temp[5] = num;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Get the controller address(page + register number) from the
 * DSP_Tunable_Tab in ROM
 *
 * @param      tTunableInfo    caller needs to provide the tab start addr
 *                             and the controller type
 *
 * @return     void
 */
static void DSPDrv_GetCtrlAddr(tTunableInfo *p)
{
    const uint8* curr_ptr;
    uint16       array_num, curr_idx;
    uint8        array_len, curr_id, block_num;

    curr_ptr = p->head + TABLE_TYPE_INIT_OFF;
    /* go through the table to find the ctrl */
    ASSERT(*curr_ptr == TABLE_TYPE_TUNABLE_LSB);

    curr_ptr = p->head + CHIP_MODEL_NO_OFF;
    ASSERT(*curr_ptr == MODEL_ADAU1761);

    /* get the array number */
    curr_ptr = p->head + ARRAY_NUM_OFF;
    array_num =  *curr_ptr;
    curr_ptr++;
    array_num += (*curr_ptr << 8);

    ASSERT(array_num);

    /* go through all the arrays */
    curr_ptr = (p->head) + ARRAY_START_OFF;
    for(curr_idx = 0; curr_idx < array_num; curr_idx++)
    {
        array_len = *curr_ptr;
        curr_ptr++;
        if(*curr_ptr == (p->type))
        {
            /* get the number of the controller block in the same type in one array */
            curr_ptr++;
            block_num = *curr_ptr;
            ASSERT(p->id < block_num);

            /* get the address of the controller blok */
            /* the id0 address */
            curr_ptr++;
            p->addr = *(curr_ptr + 2*(p->id));
            p->addr = (p->addr) << 8;
            p->addr |= *(curr_ptr + 2*(p->id) + 1);

            return;
        }

        curr_ptr += array_len;
    }

    /* can't find the ctrl info */
    ASSERT(curr_idx < array_num);
}

/**
 * Check the magic number in head of the DSP_Init_Tab or DSP_Tunable_Tab
 *
 * @param      uint8*          pointer to the table head
 *
 * @return     void
 */
static void DSPDrv_ChkMagicNum(const uint8 *p)
{
    uint8 magic_num[MAGIC_NUMBER_NUM];

    memcpy(magic_num, p, MAGIC_NUMBER_NUM);

    ASSERT((magic_num[0] == MAGIC_NUMBER_EVEN) && (magic_num[2] == MAGIC_NUMBER_EVEN)
           && (magic_num[4] == MAGIC_NUMBER_EVEN) && (magic_num[6] == MAGIC_NUMBER_EVEN)
           && (magic_num[1] == MAGIC_NUMBER_ODD) && (magic_num[3] == MAGIC_NUMBER_ODD)
           && (magic_num[5] == MAGIC_NUMBER_ODD) && (magic_num[7] == MAGIC_NUMBER_ODD));
}

/**
 * Load the DSP_Init_Tab into DSP
 *
 * @param      void*           pointer to the table head
 *
 * @return     void
 */
static void DSPDrv_DispatchDspConfigData(void* p)
{
    const uint8* curr_ptr;
    uint16       slot_idx, slot_num;
    uint8        slot_data_num;
    uint8        dly_arr[8];

    /* check magic number in the head */
    DSPDrv_ChkMagicNum((uint8*)p);

    /* Got the number of slot from DSP data */
    curr_ptr = (uint8*)p + ARRAY_NUM_OFF;    // get the lower byte
    slot_num = *(curr_ptr);
    curr_ptr++;
    slot_num += ((*curr_ptr) << 8);

    curr_ptr = (uint8*)p + ARRAY_START_OFF;      /* Point to first byte of the data array */

    for (slot_idx = 0; slot_idx < slot_num; slot_idx++)
    {
        slot_data_num = *curr_ptr;
        curr_ptr++;

        /* check whether it is a slot of time delay */
        if ((*curr_ptr == DELAY_SLOT_DATA0) && (slot_data_num == DELAY_SLOT_NUM))
        {
            /* copy all the data */
            memcpy(dly_arr, curr_ptr, DELAY_SLOT_NUM);

            if((dly_arr[1] == DELAY_SLOT_DATA0) && (dly_arr[2] == DELAY_SLOT_DATA0)
               && (dly_arr[4] == DELAY_SLOT_DATA0) && (dly_arr[5] == DELAY_SLOT_DATA0)
               && (dly_arr[6] == DELAY_SLOT_DATA0) && (dly_arr[7] == DELAY_SLOT_DATA1))
            {
                DSPDrv_Delay(RESET_DELAY_PER_UNIT * dly_arr[3]);
            }
        }

        else
        {
            DSPDrv_I2cWrite(DSP_I2C_ADDR, slot_data_num, curr_ptr);
        }

        curr_ptr += slot_data_num;
    }

    dspstatus |= DSP_INIT_SEC2_COMPLETED;
}

static void DSPDrv_InitSection0(void *p)
{
    /* If the device type is I2C, create the DSP I2C object */
    if (((cDSPDrv*)p)->pConfig->deviceInfo.deviceType == I2C_DEV)
    {
        I2CDrv_Ctor(&dspI2c,((tI2CDevice*)getDevicebyId(DSP_ONE, NULL)));
    }
}

static void DSPDrv_InitSection1(void *p)
{
    DSPDrv_DispatchDspConfigData();
}


static tDspInitSection DspInitSection[] =
{
    {&DSPDrv_InitSection0, 50},
    {&DSPDrv_InitSection1, 100},

};

/**
 * Config the L/R_MIC_PGA P/M inputs according to input, caller should
 * make clear the input config inside DSP
 *
 * @param      tDspAnaInCfg*       pointer to the Ana Input config struct
 *
 * @return     void
 */
static void DSPDrv_SetAnaInput(tDspAnaInCfg *p)
{
    uint8 tmp[3] = {0};

    /* set 0x400a */
    tmp[0] = (uint8)(RECORD_MIXER_LEFT_CTRL_0_ADDR >> 8);
    tmp[1] = (uint8)RECORD_MIXER_LEFT_CTRL_0_ADDR;
    tmp[2] = (p->linp_2mx1_gain | p->linn_2mx1_gain | p->mix1_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x400b */
    tmp[0] = (uint8)(RECORD_MIXER_LEFT_CTRL_1_ADDR >> 8);
    tmp[1] = (uint8)RECORD_MIXER_LEFT_CTRL_1_ADDR;
    tmp[2] = (p->lpga_2mx1_gain | p->laux_2mx1_gain);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x400c */
    tmp[0] = (uint8)(RECORD_MIXER_RIGHT_CTRL_0_ADDR >> 8);
    tmp[1] = (uint8)RECORD_MIXER_RIGHT_CTRL_0_ADDR;
    tmp[2] = (p->rinp_2mx2_gain | p->rinn_2mx2_gain | p->mix2_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x400d */
    tmp[0] = (uint8)(RECORD_MIXER_RIGHT_CTRL_1_ADDR >> 8);
    tmp[1] = (uint8)RECORD_MIXER_RIGHT_CTRL_1_ADDR;
    tmp[2] = (p->rpga_2mx2_gain | p->raux_2mx2_gain);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x400e */
    tmp[0] = (uint8)(RECORD_VOLUME_CTRL_LEFT_ADDR >> 8);
    tmp[1] = (uint8)RECORD_VOLUME_CTRL_LEFT_ADDR;
    tmp[2] = (p->lpga_2mx1_vol | ((p->lpga_2mx1_in_en) << 1) | p->lpga_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x400f */
    tmp[0] = (uint8)(RECORD_VOLUME_CTRL_RIGHT_ADDR >> 8);
    tmp[1] = (uint8)RECORD_VOLUME_CTRL_RIGHT_ADDR;
    tmp[2] = (p->rpga_2mx2_vol | ((p->rpga_2mx2_in_en) << 1) | p->rpga_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);
}

/**
 * Config the LineOut and HeadphoneOut according to input, caller should
 * make clear the output config inside DSP
 *
 * @param      tDspAnaOutCfg*       pointer to the Ana Input config struct
 *
 * @return     void
 */
static void DSPDrv_SetAnaOutput(tDspAnaOutCfg *p)
{
    uint8 tmp[3] = {0};

    /* set 0x401c */
    tmp[0] = (uint8)(PLAYBACK_MIXER_LEFT_CONTROL_0_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_MIXER_LEFT_CONTROL_0_ADDR;
    tmp[2] = (((p->mx3_rdac_in) << 6) | ((p->mx3_ldac_in) << 5) | p->laux_2mx3_gain | p->mx3_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x401d */
    tmp[0] = (uint8)(PLAYBACK_MIXER_LEFT_CONTROL_1_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_MIXER_LEFT_CONTROL_1_ADDR;
    tmp[2] = (p->mx2_2mx3_gain | p->mx1_2mx3_gain);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x401e */
    tmp[0] = (uint8)(PLAYBACK_MIXER_RIGHT_CONTROL_0_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_MIXER_RIGHT_CONTROL_0_ADDR;
    tmp[2] = (((p->mx4_rdac_in) << 6) | ((p->mx4_ldac_in) << 5) | p->laux_2mx4_gain | p->mx4_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x401f */
    tmp[0] = (uint8)(PLAYBACK_MIXER_RIGHT_CONTROL_1_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_MIXER_RIGHT_CONTROL_1_ADDR;
    tmp[2] = (p->mx2_2mx4_gain | p->mx1_2mx4_gain);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4020 */
    tmp[0] = (uint8)(PLAYBACK_LR_LEFT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LR_LEFT_ADDR;
    tmp[2] = (p->mx4_2mx5_gain | p->mx3_2mx5_gain | p->mx5_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4021 */
    tmp[0] = (uint8)(PLAYBACK_LR_RIGHT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LR_RIGHT_ADDR;
    tmp[2] = (p->mx4_2mx6_gain | p->mx3_2mx6_gain | p->mx6_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4022 */
    tmp[0] = (uint8)(PLAYBACK_LR_MONO_CTRL_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LR_MONO_CTRL_ADDR;
    tmp[2] = (p->mx7_ctrl | p->mx7_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4023 */
    tmp[0] = (uint8)(PLAYBACK_HEADPHONE_LEFT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_HEADPHONE_LEFT_ADDR;
    tmp[2] = (((p->lhp_vol) << 2) | ((p->lhp_en) << 1) | p->hp_vol_ctrl_en);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4024 */
    tmp[0] = (uint8)(PLAYBACK_HEADPHONE_RIGHT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_HEADPHONE_RIGHT_ADDR;
    tmp[2] = (((p->rhp_vol) << 2) | ((p->rhp_en) << 1) | p->hpout_mode);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4025 */
    tmp[0] = (uint8)(PLAYBACK_LINE_OUT_LEFT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LINE_OUT_LEFT_ADDR;
    tmp[2] = (((p->lout_vol) << 2) | ((p->lout_en) << 1) | p->lout_mode);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4026 */
    tmp[0] = (uint8)(PLAYBACK_LINE_OUT_RIGHT_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LINE_OUT_RIGHT_ADDR;
    tmp[2] = (((p->rout_vol) << 2) | ((p->rout_en) << 1) | p->rout_mode);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* set 0x4027 */
    tmp[0] = (uint8)(PLAYBACK_LINE_OUT_MONO_ADDR >> 8);
    tmp[1] = (uint8)PLAYBACK_LINE_OUT_MONO_ADDR;
    tmp[2] = (((p->mono_vol) << 2) | ((p->mono_en) << 1) | p->mono_mode);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);
}

/**
 * Config the digital audio interface according to the input
 *
 * @param      tDspDigAudCfg*        pointer to the digital audio interface config struct
 *
 * @return     void
 */
static void DSPDrv_SetDigAudItf(tDspDigAudCfg *p)
{
    uint8 tmp[3] = {0};

    /* 0x4015 */
    tmp[0] = (uint8)(SERIAL_PORT_CONTROL_0_ADDR >> 8);
    tmp[1] = (uint8)SERIAL_PORT_CONTROL_0_ADDR;
    tmp[2] = (p->fs_src | p->lrclk_mode | p->bclk_pol | p->lrclk_pol | p->chl_mode | p->bus_mode);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);

    /* 0x4016 */
    tmp[0] = (uint8)(SERIAL_PORT_CONTROL_1_ADDR >> 8);
    tmp[1] = (uint8)SERIAL_PORT_CONTROL_1_ADDR;
    tmp[2] = (p->bit_per_frame | p->tdm_dout_pos | p->tdm_din_pos | p->msb_pos | p->data_dly);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);
}

/**
 * Configure GPIO0 function
 *
 * @param      tDspGpio0Cfg*        pointer
 * @return     void
 */
static void DSPDrv_SetGpio0Func(tDspGpio0Cfg *p)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, SERIAL_DATAGPIO_PIN_CONFIG_ADDR, 1);

    /* 0x40f4 */
    temp[0] = (uint8)(SERIAL_DATAGPIO_PIN_CONFIG_ADDR >> 8);
    temp[1] = (uint8)SERIAL_DATAGPIO_PIN_CONFIG_ADDR;
    temp[2] &= ~0x01;
    temp[2] |= (p->gpio0_type);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    /* 0x40c6 */
    temp[0] = (uint8)(GPIO_0_CONTROL_ADDR >> 8);
    temp[1] = (uint8)GPIO_0_CONTROL_ADDR;
    temp[2] = (p->gpio0_func);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Get GPIO0 input
 *
 * @param      void
 * @return     bool        TRUE - high, FALSE - low
 */
static bool DSPDrv_GetGpio0In(void)
{
    uint8 temp = 0;

    DSPDrv_I2cRead(&temp, DSP_I2C_ADDR, GPIO0_SET_REG, 1);

    return((bool)(temp & 0x01));
}

/**
 * Set GPIO0 output
 *
 * @param      bool        TRUE - high, FALSE - low
 * @return     void
 */
static void DSPDrv_SetGpio0Out(bool out)
{
    uint8 temp[3] = {0};

    temp[0] = (uint8)(GPIO0_SET_REG >> 8);
    temp[1] = (uint8)GPIO0_SET_REG;
    temp[2] = out;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Configure GPIO1 function
 *
 * @param      tDspGpio1Cfg*        pointer
 * @return     void
 */
static void DSPDrv_SetGpio1Func(tDspGpio1Cfg *p)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, SERIAL_DATAGPIO_PIN_CONFIG_ADDR, 1);

    /* 0x40f4 */
    temp[0] = (uint8)(SERIAL_DATAGPIO_PIN_CONFIG_ADDR >> 8);
    temp[1] = (uint8)SERIAL_DATAGPIO_PIN_CONFIG_ADDR;
    temp[2] &= ~0x02;
    temp[2] |= (p->gpio1_type);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    /* 0x40c7 */
    temp[0] = (uint8)(GPIO_1_CONTROL_ADDR >> 8);
    temp[1] = (uint8)GPIO_1_CONTROL_ADDR;
    temp[2] = (p->gpio1_func);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Get GPIO1 input
 *
 * @param      void
 * @return     bool        TRUE - high, FALSE - low
 */
static bool DSPDrv_GetGpio1In(void)
{
    uint8 temp = 0;

    DSPDrv_I2cRead(&temp, DSP_I2C_ADDR, GPIO1_SET_REG, 1);

    return((bool)(temp & 0x01));
}

/**
 * Set GPIO1 output
 *
 * @param      bool        TRUE - high, FALSE - low
 * @return     void
 */
static void DSPDrv_SetGpio1Out(bool out)
{
    uint8 temp[3] = {0};

    temp[0] = (uint8)(GPIO1_SET_REG >> 8);
    temp[1] = (uint8)GPIO1_SET_REG;
    temp[2] = out;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Configure GPIO2 function
 *
 * @param      tDspGpio2Cfg*        pointer
 * @return     void
 */
static void DSPDrv_SetGpio2Func(tDspGpio2Cfg *p)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, SERIAL_DATAGPIO_PIN_CONFIG_ADDR, 1);

    /* 0x40f4 */
    temp[0] = (uint8)(SERIAL_DATAGPIO_PIN_CONFIG_ADDR >> 8);
    temp[1] = (uint8)SERIAL_DATAGPIO_PIN_CONFIG_ADDR;
    temp[2] &= ~0x04;
    temp[2] |= (p->gpio2_type);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    /* 0x40c8 */
    temp[0] = (uint8)(GPIO_2_CONTROL_ADDR >> 8);
    temp[1] = (uint8)GPIO_2_CONTROL_ADDR;
    temp[2] = (p->gpio2_func);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Get GPIO2 input
 *
 * @param      void
 * @return     bool        TRUE - high, FALSE - low
 */
static bool DSPDrv_GetGpio2In(void)
{
    uint8 temp = 0;

    DSPDrv_I2cRead(&temp, DSP_I2C_ADDR, GPIO2_SET_REG, 1);

    return((bool)(temp & 0x01));
}

/**
 * Set GPIO2 output
 *
 * @param      bool        TRUE - high, FALSE - low
 * @return     void
 */
static void DSPDrv_SetGpio2Out(bool out)
{
    uint8 temp[3] = {0};

    temp[0] = (uint8)(GPIO2_SET_REG >> 8);
    temp[1] = (uint8)GPIO2_SET_REG;
    temp[2] = out;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Configure GPIO3 function
 *
 * @param      tDspGpio3Cfg*        pointer
 * @return     void
 */
static void DSPDrv_SetGpio3Func(tDspGpio3Cfg *p)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, SERIAL_DATAGPIO_PIN_CONFIG_ADDR, 1);

    /* 0x40f4 */
    temp[0] = (uint8)(SERIAL_DATAGPIO_PIN_CONFIG_ADDR >> 8);
    temp[1] = (uint8)SERIAL_DATAGPIO_PIN_CONFIG_ADDR;
    temp[2] &= ~0x08;
    temp[2] |= (p->gpio3_type);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);

    /* 0x40c9 */
    temp[0] = (uint8)(GPIO_3_CONTROL_ADDR >> 8);
    temp[1] = (uint8)GPIO_3_CONTROL_ADDR;
    temp[2] = (p->gpio3_func);
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Get GPIO3 input
 *
 * @param      void
 * @return     bool        TRUE - high, FALSE - low
 */
static bool DSPDrv_GetGpio3In(void)
{
    uint8 temp = 0;

    DSPDrv_I2cRead(&temp, DSP_I2C_ADDR, GPIO3_SET_REG, 1);

    return((bool)(temp & 0x01));
}

/**
 * Set GPIO3 output
 *
 * @param      bool        TRUE - high, FALSE - low
 * @return     void
 */
static void DSPDrv_SetGpio3Out(bool out)
{
    uint8 temp[3] = {0};

    temp[0] = (uint8)(GPIO3_SET_REG >> 8);
    temp[1] = (uint8)GPIO3_SET_REG;
    temp[2] = out;
    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}


/**
 * Activate/deactivate ADC out.
 *
 * @param      bool            enable/disable ADC
 *
 * @return     void
 */
void DSPDrv_ActivateADCOut(bool input)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, ADC_CONTROL_0_ADDR, 1)

    if(input)
    {
        temp[2] |= 0x03;
    }

    else
    {
        temp[2] &= ~0x03);
    }

    temp[0] = (uint8)(ADC_CONTROL_0_ADDR >> 8);
    temp[1] = (uint8)ADC_CONTROL_0_ADDR;

    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Activate/deactivate DAC out.
 *
 * @param      bool            enable/disable DAC
 *
 * @return     void
 */
void DSPDrv_ActivateDACOut(bool input)
{
    uint8 temp[3] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, DAC_CONTROL_0_ADDR, 1)

    if(input)
    {
        temp[2] |= 0x03;
    }

    else
    {
        temp[2] &= ~0x03);
    }

    temp[0] = (uint8)(DAC_CONTROL_0_ADDR >> 8);
    temp[1] = (uint8)DAC_CONTROL_0_ADDR;

    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Activate/deactivate HP out.
 *
 * @param      bool            enable/disable HP
 *
 * @return     void
 */
void DSPDrv_ActivateHPAOut(bool input)
{
    uint8 temp[4] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, PLAYBACK_HEADPHONE_LEFT_ADDR, 2);

    if(input)
    {
        temp[2] |= 0x02;
        temp[3] |= 0x02;
    }

    else
    {
        temp[1] &= ~0x02;
        temp[2] &= ~0x02;
    }

    temp[0] = (uint8)(PLAYBACK_HEADPHONE_LEFT_ADDR >> 8);
    temp[1] = (uint8)PLAYBACK_HEADPHONE_LEFT_ADDR;

    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), temp);
}

/**
 * Activate/deactivate line out.
 *
 * @param      bool            enable/disable lineout
 *
 * @return     void
 */
void DSPDrv_ActivateLoOut(bool input)
{
    uint8 temp[4] = {0};

    DSPDrv_I2cRead(&temp[2], DSP_I2C_ADDR, PLAYBACK_LINE_OUT_LEFT_ADDR, 2);

    if(input)
    {
        temp[2] |= 0x02;
        temp[3] |= 0x02;
    }

    else
    {
        temp[2] &= ~0x02;
        temp[3] |= ~0x02;
    }

    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(temp), (uint8*)(&temp));
}

/**
 * Config the PLL according to the input
 *
 * @param      tDspPllCfg*        pointer to the PLL config struct
 *
 * @return     void
 */
void DSPDrv_ConfigPLL(tDspPllCfg *p)
{
    uint8 tmp[8] = {0};

    tmp[0] = (uint8)(PLLCTRLREGISTER_ADDR >> 8);
    tmp[1] = (uint8)(PLLCTRLREGISTER_ADDR);

    /* check type, frac or integer */
    if(p->pll_type == PLL_TYPE_INT)
    {
        tmp[2] = 0;
        tmp[3] = 0;
        tmp[4] = 0;
        tmp[5] = 0;
    }

    else
    {
        tmp[2] = (uint8)((p->pll_denom_val) >> 8);
        tmp[3] = (uint8)(p->pll_denom_val);
        tmp[4] = (uint8)((p->pll_numer_val) >> 8);
        tmp[5] = (uint8)(p->pll_numer_val);
    }

    tmp[6] = (p->pll_r_val | p->pll_x_val | p->pll_type);
    tmp[7] = 0x03;

    DSPDrv_I2cWrite(DSP_I2C_ADDR, sizeof(tmp), tmp);
}

void DSPDrv_Ctor(cDSPDrv* me)
{
    tI2CDevice * conf = (tI2CDevice *) getDevicebyId(DSP_ONE, NULL);
    ASSERT(conf);
    me->pInitTable = DspInitSection;
    me->sectionSize = ArraySize(DspInitSection);
    me->initPhase = 0;
    me->pConfig = (tI2CDevice*)conf;
}

void DSPDrv_Xtor(cDSPDrv* me)
{
    me->pInitTable = NULL;
    me->sectionSize = 0;
    me->initPhase = 0;
    me->pConfig = NULL;
    I2CDrv_Xtor(&dspI2c);
}

/* PRODUCT-DEPENDENT FUNCTIONS */
/**
 * Swith the audio in path according to product HW
 *
 * @param      eDspAudInOption        target audio in option
 * @return     void
 */
void DSPDrv_SwitchAudioIn(eDspAudInOption in)
{
    /* first disable DAC out*/
    DSPDrv_ActivateDACOut(FALSE);

    switch(in)
    {
        /* L/R AUX in */
        case PROD_AUD_IN_OPTION0:
            /* the I2S needs to be cut */
            DSPDrv_SetGpio0Func(GPIO0_CFG1);
            DSPDrv_SetGpio1Func(GPIO1_CFG1);
            DSPDrv_SetGpio2Func(GPIO2_CFG1);
            DSPDrv_SetGpio3Func(GPIO3_CFG1);
            DSPDrv_SetAnaInput(&ANA_INPUT_CFG0);
            break;

        /* diff in */
        case PROD_AUD_IN_OPTION1:
            /* the I2S needs to be cut */
            DSPDrv_SetGpio0Func(GPIO0_CFG1);
            DSPDrv_SetGpio1Func(GPIO1_CFG1);
            DSPDrv_SetGpio2Func(GPIO2_CFG1);
            DSPDrv_SetGpio3Func(GPIO3_CFG1);
            DSPDrv_SetAnaInput(&ANA_INPUT_CFG1);
            break;

        /* I2S in */
        case PROD_AUD_IN_OPTION2:
            /* enable I2S pins */
            DSPDrv_SetGpio0Func(GPIO0_CFG0);
            DSPDrv_SetGpio1Func(GPIO1_CFG0);
            DSPDrv_SetGpio2Func(GPIO2_CFG0);
            DSPDrv_SetGpio3Func(GPIO3_CFG0);
            /* all analog input should be cut */
            DSPDrv_SetAnaInput(&ANA_INPUT_CFG2);
            DSPDrv_setDigAudItf(&DIG_AUD_CFG0);
            break;

        default:
            ASSERT(0);
            break;
    }

    /* enable DAC out */
    DSPDrv_ActivateDACOut(TRUE);
}

/**
 * Swith the audio out path according to product HW
 *
 * @param      eDspAudOutOption        target audio out option
 * @return     void
 */
void DSPDrv_SwitchAudioOut(eDspAudOutOption out)
{
    /* first disable DAC out*/
    DSPDrv_ActivateDACOut(FALSE);

    switch(out)
    {
        case PROD_AUD_OUT_OPTION0:
            DSPDrv_setAnaOutput(&ANA_OUTPUT_CFG0);
            break;

        defaut:
            ASSERT(0);
            break;
    }

    /* enable DAC out */
    DSPDrv_ActivateDACOut(TRUE);
}

/**
 * Set volume
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_SetVolume(uint8 vol)
{
    /* write DSP register */
    uint8        vol_iic[4] = {0};
    tTunableInfo tunable_info;
    uint16       reg;

    tunable_info.head = (uint8*)dsp_tunable_tab;
    tunable_info.type = ARRAY_TYPE_VOL_CTRL;
    tunable_info.id   = 0;    /* suppose there are only 1 volume controller that is changeable */
    DSPDrv_GetCtrlAddr(&tunable_info);

    if(vol > MAX_VOLUME)
    {
        vol = MAX_VOLUME;
    }

    reg = tunable_info.addr;
    memcpy(vol_iic, &(vol_tab[vol]), sizeof(vol_iic));

    DspDrv_SafeLoadData(1, vol_iic, reg);
}

/**
 * This function is used to convert filter Parameter (Q factor, center freq, etc) into 5 DSP parameters
 *
 * @param   tDspFiltRawParam                 the filter parameter raw input
 * @param   uint8*                           pointer to the output data
 * @return  void
 */
static void DSPDrv_CalFiltParam(tDspFiltRawParam in, uint8* out)
{
    /* calculate common param for all filter */
    double w0 = 2.0*PI*in.freq/SAMPLING_FREQ;
    double A = pow(10.0, in.boost/40.0);
    double gainLinear = pow(10.0, in.gain/20.0);
    double a0, a1, a2, b0, b1, b2, S, alpha;
    int32  temp[EQ_CTRL_RAM_SLOT_NUM];
    uint8  cnt;

    S = in.q;

    if(in.en == FALSE)
    {
        temp[0] = 0x00800000;
        for(cnt = 1; cnt < EQ_CTRL_RAM_SLOT_NUM; cnt++)
        {
            temp[cnt] = 0;
        }
        /* change the data from int32 to uint8 */
        DSPDrv_FormatFiltData(temp, out, EQ_CTRL_RAM_SLOT_NUM);

        return;
    }

    else if(in.q == 0)
    {
        S = 0.05;
    }

    switch(in.type)
    {
        case FILTER_TYPE_PEAKING:
        {
            alpha = sin(w0)/(2.0*S);
            a0 = 1.0+alpha/A;
            a1 = -2.0*cos(w0);
            a2 = 1.0-alpha/A;
            b0 = (1.0+alpha*A)*gainLinear;
            b1 = -(2.0*cos(w0))*gainLinear;
            b2 = (1.0-A*alpha)*gainLinear;

            /*  speical behavior when speical inputs
             *  (inspired by Sigma Studio outputs)
             * */
            if(in.boost == 0.00)
            {
                b1 = 0;
                b2 = 0;
                a1 = 0;
                a2 = 0;
            }

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_TONE_LOW_SHELF:
        {
            alpha = (sin(w0)/2)*sqrt((A+1/A )*(1/S-1)+2);
            a0 = (A+1)+(A-1)*cos(w0)+2*sqrt(A)*alpha;
            a1 = -2*((A-1)+(A+1)*cos(w0));
            a2 = (A+1)+(A-1)*cos(w0)-2*sqrt(A)*alpha;
            b0 = A*((A+1)-(A-1)*cos(w0)+2*sqrt(A)*alpha)*gainLinear;
            b1 = 2*A*((A-1)-(A+1)*cos(w0))*gainLinear;
            b2 = A*((A+1)-(A-1)*cos(w0)-2*sqrt(A)*alpha)*gainLinear;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_TONE_HIGH_SHELF:
        {
            alpha = (sin(w0)/2)*sqrt((A+1/A)*(1/S-1)+2);
            a0 = (A+1)-(A-1)*cos(w0)+2*sqrt(A)*alpha;
            a1 = 2*((A-1)-(A+1)*cos(w0));
            a2 = (A+1)-(A-1)*cos(w0)-2*sqrt(A)*alpha;
            b0 = A*((A+1)+(A-1)*cos(w0)+2*sqrt(A)*alpha)*gainLinear;
            b1 = -2*A*((A-1)+(A+1)*cos(w0))*gainLinear;
            b2 = A*((A+1)+(A-1)*cos(w0)-2*sqrt(A)*alpha)*gainLinear;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_GENERAL_LOW_PASS:
        {
            alpha = sin(w0)/(2.0*S);
            a0 = 1.0+alpha;
            a1 = -2*cos(w0);
            a2 = 1-alpha;
            b0 = (1-cos(w0))*gainLinear/2;
            b1 = (1-cos(w0))*gainLinear;
            b2 = (1-cos(w0))*gainLinear/2;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_GENERAL_HIGH_PASS:
        {
            alpha = sin(w0)/(2.0*S);
            a0 = 1.0+alpha;
            a1 = -2*cos(w0);
            a2 = 1-alpha;
            b0 = (1+cos(w0))*gainLinear/2;
            b1 = -1*(1+cos(w0))*gainLinear;
            b2 = (1+cos(w0))*gainLinear/2;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_BUTTWORTH_LOW_PASS:
        {
            alpha = sin(w0)/sqrt(2.0);
            a0 = 1.0+alpha;
            a1 = -2*cos(w0);
            a2 = 1-alpha;
            b0 = (1-cos(w0))*gainLinear/2;
            b1 = 1-cos(w0)*gainLinear;
            b2 = (1-cos(w0))*gainLinear/2;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_BUTTWORTH_HIGH_PASS:
        {
            alpha = sin(w0)/sqrt(2.0);
            a0 = 1.0+alpha;
            a1 = -2.0*cos(w0);
            a2 = 1.0-alpha;
            b0 = (1.0+cos(w0))*gainLinear/2.0;
            b1 = -(1.0+cos(w0))*gainLinear;
            b2 = (1.0+cos(w0))*gainLinear/2.0;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_BESSEL_LOW_PASS:
        {
            alpha = sin(w0)/(2.0/sqrt(3.0));
            a0 = 1.0+alpha;
            a1 = -2*cos(w0);
            a2 = 1-alpha;
            b0 = (1-cos(w0))*gainLinear/2;
            b1 = 1-cos(w0)*gainLinear;
            b2 = (1-cos(w0))*gainLinear/2;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        case FILTER_TYPE_BESSEL_HIGH_PASS:
        {
            alpha = sin(w0)/(2.0/sqrt(3.0));
            a0 = 1.0+alpha;
            a1 = -2*cos(w0);
            a2 = 1-alpha;
            b0 = (1+cos(w0))*gainLinear/2;
            b1 = -1*(1+cos(w0))*gainLinear;
            b2 = (1+cos(w0))*gainLinear/2;

            /* normalization */
            a1 /= a0;
            a2 /= a0;
            b0 /= a0;
            b1 /= a0;
            b2 /= a0;

            /* inverton on a1, a2 */
            a1 = -a1;
            a2 = -a2;
        }
        break;

        default:
        {
            /* wrong data in */
            ASSERT(0);
        }
        break;
    }

    /* check data */
    ASSERT(a1 > -16.0);
    ASSERT(a1 < 15.99);
    ASSERT(a2 > -16.0);
    ASSERT(a2 < 15.99);
    ASSERT(b0 > -16.0);
    ASSERT(b0 < 15.99);
    ASSERT(b1 > -16.0);
    ASSERT(b1 < 15.99);
    ASSERT(b2> -16.0);
    ASSERT(b2 < 15.99);

    /* feed into output structure */
    temp[0] = (int32)(b0*(1L << 23));
    temp[1] = (int32)(b1*(1L << 23));
    temp[2] = (int32)(b2*(1L << 23));
    temp[3] = (int32)(a1*(1L << 23));
    temp[4] = (int32)(a2*(1L << 23));

    /* change the data from int32 to uint8 */
    DSPDrv_FormatFiltData(temp, out, EQ_CTRL_RAM_SLOT_NUM);
}

/**
 * Set BGC
 *
 * @param      int8            boost
 * @param      bool            enable or disable
 * @return     void
 */
void DSPDrv_SetBGC(int8 boost, bool en)
{
    tDspFiltRawParam                          raw_data;
    uint8             final_data[EQ_CTRL_RAM_BYTE_NUM];
    tTunableInfo                              tun_data;
    uint8                                     curr_idx;

    tun_data.head         = dsp_tunable_tab;
    tun_data.type         = ARRAY_TYPE_EQ;
    raw_data.freq         = BGC_FREQ;
    raw_data.boost        = boost;
    raw_data.gain         = BGC_GAIN;
    raw_data.q            = BGC_QFACTOR;
    raw_data.en           = en;

    /* get the start addr */
    for(curr_idx = 0; curr_idx < EQ_CTRL_NUM; curr_idx++)
    {
        if(EQ_TRANS_TAB[curr_idx].name== EQ_CTRL_BGC)
        {
            raw_data.type = EQ_TRANS_TAB[curr_idx].type;
            tun_data.id   = EQ_TRANS_TAB[curr_idx].id;

            break;
        }
    }

    ASSERT(curr_idx < EQ_CTRL_NUM);
    DSPDrv_GetCtrlAddr(&tun_data);

    /* cal out the EQ data */
    DSPDrv_CalFiltParam(raw_data, final_data);
    DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, final_data, tun_data.addr);
}

/**
 * Set User LP
 *
 * @param      int8            boost
 * @param      eUserLpSlope    slope
 * @param      bool            en
 * @return     void
 */
void DSPDrv_SetUserLp(uint16 freq, eUserLpSlope slope, bool en)
{
    tDspFiltRawParam                              raw_data;
    uint8                 final_data[EQ_CTRL_RAM_BYTE_NUM];
    tTunableInfo                                  tun_data;
    uint8                                    curr_idx, cnt;

    tun_data.head         = dsp_tunable_tab;
    tun_data.type         = ARRAY_TYPE_EQ;
    raw_data.freq         = freq;
    raw_data.boost        = USER_LP_BOOST;
    raw_data.gain         = USER_LP_GAIN;
    raw_data.q            = USER_LP_QFACTOR;
    raw_data.en           = en;

    /* get the start addr */
    for(curr_idx = 0; curr_idx < EQ_CTRL_NUM; curr_idx++)
    {
        if(EQ_TRANS_TAB[curr_idx].name== EQ_CTRL_USER_LP)
        {
            raw_data.type = EQ_TRANS_TAB[curr_idx].type;
            tun_data.id   = EQ_TRANS_TAB[curr_idx].id;

            break;
        }
    }

    ASSERT(curr_idx < EQ_CTRL_NUM);
    DSPDrv_GetCtrlAddr(&tun_data);

    /* cal out the EQ data */
    DSPDrv_CalFiltParam(raw_data, final_data);

    /* check slope */
    switch(slope)
    {
        case USER_LP_SLOPE_6DB:
            curr_idx = 1;
            break;

        case USER_LP_SLOPE_12DB:
            curr_idx = 2;
            break;

        case USER_LP_SLOPE_18DB:
            curr_idx = 3;
            break;

        case USER_LP_SLOPE_24DB:
            curr_idx = 4;
            break;

        default:
            ASSERT(0);
            break;
    }

    for(cnt = 0; cnt < USER_LP_STAGE_NUM; cnt++)
    {
        if(cnt < curr_idx)
        {
            DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, final_data, tun_data.addr);
        }

        else
        {
            DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, direct_pass_data, tun_data.addr);
        }
        tun_data.addr += EQ_CTRL_RAM_SLOT_NUM;
    }
}

/**
 * Set User HP
 *
 * @param      int8            boost
 * @param      eUserHpSlope    slope
 * @param      bool            en
 * @return     void
 */
void DSPDrv_SetUserHp(uint16 freq, eUserHpSlope slope, bool en)
{
    tDspFiltRawParam                          raw_data;
    uint8             final_data[EQ_CTRL_RAM_BYTE_NUM];
    tTunableInfo                              tun_data;
    uint8                                curr_idx, cnt;

    tun_data.head         = dsp_tunable_tab;
    tun_data.type         = ARRAY_TYPE_EQ;
    raw_data.freq         = freq;
    raw_data.boost        = USER_HP_BOOST;
    raw_data.gain         = USER_HP_GAIN;
    raw_data.q            = USER_HP_QFACTOR;
    raw_data.en           = en;

    /* get the start addr */
    for(curr_idx = 0; curr_idx < EQ_CTRL_NUM; curr_idx++)
    {
        if(EQ_TRANS_TAB[curr_idx].name== EQ_CTRL_USER_HP)
        {
            raw_data.type = EQ_TRANS_TAB[curr_idx].type;
            tun_data.id   = EQ_TRANS_TAB[curr_idx].id;

            break;
        }
    }

    ASSERT(curr_idx < EQ_CTRL_NUM);
    DSPDrv_GetCtrlAddr(&tun_data);

    /* cal out the EQ data */
    DSPDrv_CalFiltParam(raw_data, final_data);

    /* check slope */
    switch(slope)
    {
        case USER_HP_SLOPE_6DB:
            curr_idx = 1;
            break;

        case USER_HP_SLOPE_12DB:
            curr_idx = 2;
            break;

        case USER_HP_SLOPE_18DB:
            curr_idx = 3;
            break;

        case USER_HP_SLOPE_24DB:
            curr_idx = 4;
            break;

        default:
            ASSERT(0);
            break;
    }

    for(cnt = 0; cnt < USER_HP_STAGE_NUM; cnt++)
    {
        if(cnt < curr_idx)
        {
            DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, final_data, tun_data.addr);
        }

        else
        {
            DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, direct_pass_data, tun_data.addr);
        }
        tun_data.addr += EQ_CTRL_RAM_SLOT_NUM;
    }
}

/**
 * Set Tuning
 *
 * @param      eTuningRange           range
 * @return     void
 */
void DSPDrv_SetTuning(eTuningRange range)
{
    tDspFiltRawParam                          raw_data;
    uint8             final_data[EQ_CTRL_RAM_BYTE_NUM];
    tTunableInfo                              tun_data;
    uint8                                     curr_idx;

    tun_data.head         = dsp_tunable_tab;
    tun_data.type         = ARRAY_TYPE_EQ;
    raw_data.freq         = TUNING_FREQ;
    raw_data.gain         = TUNING_GAIN;
    raw_data.q            = TUNING_QFACTOR;

    /* get the start addr */
    for(curr_idx = 0; curr_idx < EQ_CTRL_NUM; curr_idx++)
    {
        if(EQ_TRANS_TAB[curr_idx].name== EQ_CTRL_TUNING)
        {
            raw_data.type = EQ_TRANS_TAB[curr_idx].type;
            tun_data.id   = EQ_TRANS_TAB[curr_idx].id;

            break;
        }
    }

    ASSERT(curr_idx < EQ_CTRL_NUM);
    DSPDrv_GetCtrlAddr(&tun_data);

    /* check range */
    switch(range)
    {
        case TUNING_RANGE_20HZ:
            raw_data.boost = TUNING_BOOST0;
            raw_data.en    = TRUE;
            break;

        case TUNING_RANGE_16HZ:
            raw_data.boost = TUNING_BOOST1;
            raw_data.en    = TRUE;
            break;

        case TUNING_RANGE_SEALED:
            raw_data.en    = FALSE;
            break;

        default:
            ASSERT(0);
            break;
    }

    /* cal out the EQ data */
    DSPDrv_CalFiltParam(raw_data, final_data);
    DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, final_data, tun_data.addr);
}

/**
 * Set PEQ
 *
 * @param      ePresetEQ        EQ
 * @param      uint16           freq
 * @param      doubel           gain
 * @param      double           q
 * @return     void
 */
static void DSPDrv_SetPeq(tDspPeqParam *pDspPeqPara)
{
    tDspFiltRawParam                          rawData;
    uint8            final_data[EQ_CTRL_RAM_BYTE_NUM];
    tTunableInfo                             tun_data;
    uint8                               curr_idx, cnt;

    tun_data.head  = dsp_tunable_tab;
    tun_data.type  = ARRAY_TYPE_EQ;
    rawData.gain   = PARAM_EQ_GAIN;
    rawData.freq   = pDspPeqPara->freq;
    rawData.boost  = pDspPeqPara->boost;
    rawData.q      = pDspPeqPara->q;
    rawData.en     = pDspPeqPara->en;

    /* cal out the EQ data */
    DSPDrv_CalFiltParam(rawData, final_data);

    /* get the start addr */
    for(curr_idx = 0; curr_idx < EQ_CTRL_NUM; curr_idx++)
    {
        if(EQ_TRANS_TAB[curr_idx].name== EQ_CTRL_PEQ)
        {
            rawData.type = EQ_TRANS_TAB[curr_idx].type;
            tun_data.id  = EQ_TRANS_TAB[curr_idx].id;
            break;
        }
    }

    ASSERT(curr_idx < EQ_CTRL_NUM);
    DSPDrv_GetCtrlAddr(&tun_data);

    switch(pDspPeqPara->pEq)
    {
        case PARAM_EQ_0:
        {
            curr_idx = 0;
        }
        break;

        case PARAM_EQ_1:
        {
            curr_idx = 1;
        }
        break;

        case PARAM_EQ_2:
        {
            curr_idx = 2;
        }
        break;

        default:
        {
            ASSERT(0);
        }
        break;
    }

    while(cnt < PARAM_EQ_STAGE_NUM)
    {
        if(cnt == curr_idx)
        {
            /* set the right parametric EQ */
            DspDrv_SafeLoadData(EQ_CTRL_RAM_SLOT_NUM, final_data, tun_data.addr);
        }
        tun_data.addr += EQ_CTRL_RAM_SLOT_NUM;
        cnt++;
    }
}


/**
 * Set delay
 *
 * @param      uint8       phase shift 0-180
 * @return     void
 */
void DSPDrv_SetPol(uint8 shift)
{
    uint8    temp[4] = {0};
    uint16         tmp = 0;
    /* get addr */
    tTunableInfo    tun_info;

    tun_info.head = dsp_tunable_tab;
    tun_info.type = ARRAY_TYPE_DELAY;
    tun_info.id   = 0;
    DSPDrv_GetCtrlAddr(&tun_info);

    /* set data */
    tmp = shift*240;
    temp[0] = 0x00;
    temp[1] = 0x00;
    temp[2] = 0x00;
    temp[3] = (uint8)(tmp/180);

    DspDrv_SafeLoadData(1, temp, tun_info.addr);
}

/**
 * Set user-defined EQ
 *
 * @param      void*         pointer to input data structure
 * @return     void
 */
void DSPDrv_SetUserEq(void* ptr)
{
    eDspEqCtrl eq_name = *((eDspEqCtrl*)ptr);

    switch(eq_name)
    {
        case EQ_CTRL_BGC:
        {
            int8 boost = (tDspBgcInputParam*)ptr->boost;

            DSPDrv_SetBGC(boost);
        }
        break;

        case EQ_CTRL_USER_LP:
        {
            uint16         freq = (tDspUserLpParam*)ptr->freq;
            eUserLpSlope  slope = (tDspUserLpParam*)ptr->slope;
            bool             en = (tDspUserLpParam*)ptr->en;

            DSPDrv_SetUserLp(freq, slope, en);
        }
        break;

        case EQ_CTRL_USER_HP:
        {
            uint16         freq = (tDspUserHpParam*)ptr->freq;
            eUserLpSlope  slope = (tDspUserHpParam*)ptr->slope;
            bool             en = (tDspUserHpParam*)ptr->en;

            DSPDrv_SetUserHp(freq, slope, en);
        }
        break;

        case EQ_CTRL_TUNING:
        {
            eTuningRange   range = (tDspTuningParam*)ptr->range;

            DSPDrv_SetTuning(range);
        }
        break;

        case EQ_CTRL_PEQ:
        {
            DSPDrv_SetPeq((tDspPeqParam*)ptr);
        }
        break;

        default:
        {
            ASSERT(0);
        }
        break;
    }
}


