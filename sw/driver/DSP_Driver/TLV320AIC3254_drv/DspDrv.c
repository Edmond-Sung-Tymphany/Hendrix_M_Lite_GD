/**
*  @file      DrvTLV320AIC3254.c
*  @brief     This file implements the driver for the Audio DSP TLV320AIC3254.
*  @modified  Donald Leung/Edmond Sung
*  @date      04-2013
*  @copyright Tymphany Ltd.
*/

#include "product.config"
#include "trace.h"
#include "DspDrv.h"
#include "DspDrv_priv.h"
#include "dsp_TLV320AIC3254_driver_config.h"
#include "dsp_TLV320AIC3254_driver_config.inc"

#include "I2CDrv.h"
#include "signals.h"
#include "Dsp_Init_Tab.h"
#include "Dsp_Tunable_Tab.h"
#include "bsp.h"



#ifndef NULL
#define NULL                             (0)
#endif

#ifdef DEBUG_DSP
#define DSP_DEBUG(x) DEBUG(x)
#else
#define DSP_DEBUG(x)
#endif

/* GPIO check times */
#define PIO_CHK_TIMES                                      (10)
#define PIO_CHK_CFM_TIMES                                  (3)

/* Private Variables */
static eDspStatus dspstatus = DSP_INIT_NONE;

static cI2CDrv dspI2c;

#define BUFFER_AB_OFFSET                    (0x12)
#define DSP_OPTIMIZING
#define PAGE_SETTING_SLOT_LEN               (2)
#define MINI_A_BUFFER_A_START_PAGE          (8)
#define MINI_A_BUFFER_A_END_PAGE            (16)
#define MINI_D_BUFFER_A_START_PAGE          (44)
#define MINI_D_BUFFER_A_END_PAGE            (52)
#define MINI_D_INSTRUCTION_END              (186)
#define MINI_AD_DATA_LEN                    (3)
#define MINI_AD_DATA_NUM_PER_PAGE           (30)
#define MINI_AD_DATA_BYTE_LEN               (4)
#define POS_DATA_BYTE_LEN                   (4)
#define MINI_AD_INSTRUCTION_END_BYTE_0      (0x88)
#define MINI_AD_INSTRUCTION_END_BYTE_1      (0x03)
#define MINI_AD_INSTRUCTION_END_BYTE_2      (0xFF)

static tCtrIdEQIdMap ctrIdEQIdMap[] =
{
    /* DSP setting ID  index of setting db*/
    {DSP_VOLUME_SETT_ID,  SETID_VOLUME},
};


/* Private Functions */
static bool DSPDrv_GetTunableInfo(tTunableInfo *p);
static bool DSPDrv_ChkMagicNum(const uint8 *p);
static void DSPDrv_InitTask(void *p);
static void DSPDrv_InitSection(void *p);
static void DSPDrv_InitPins(void *p);
static void DSPDrv_DispatchDspConfigData(void *p);
/* unzip the data slot into and sends it out */
static void DSPDrv_UnZipCoefData(void* p, const uint8* start, uint8 len);
/* unzip the instruction slot and sends it out */
static void DSPDrv_UnZipInstructionData(void* p, const uint8* start, uint8 len);
static void DSPDrv_DispatchCoefData(void* p, const uint8* start, const uint8* end);
static void DSPDrv_DispatchOrdData(void* p, const uint8* start, const uint8* end);
static void DSPDrv_WriteRegister(cDSPDrv *me, eTlv320ac3254BufferRegion BuffRegion, uint8 page, uint8* data, uint8 data_size);


static bool DSPDrv_GetTunableInfo(tTunableInfo *p)
{
    const uint8* curr_ptr;
    uint16       array_num, curr_idx;
    uint8        array_len;

    curr_ptr = (p->head) + TABLE_TYPE_INIT_OFF;

    /* go through the table to find the ctrl */
    if(*curr_ptr != TABLE_TYPE_TUNABLE_LSB)
    {
        return FALSE;
    }

    curr_ptr = (p->head) + CHIP_MODEL_NO_OFF;
    if(*curr_ptr != MODEL_AIC3254)
    {
        return FALSE;
    }

    /* get the array number */
    curr_ptr = (p->head) + ARRAY_NUM_OFF;
    array_num =  *curr_ptr;
    curr_ptr++;
    array_num += (*curr_ptr << 8);

    if(!array_num)
    {
        return FALSE;
    }

    /* go through all the arrays */
    curr_ptr = (p->head) + ARRAY_START_OFF;
    for(curr_idx = 0; curr_idx < array_num; curr_idx++)
    {
        array_len = *curr_ptr;
        curr_ptr++;
        if((*curr_ptr) == (p->type))
        {
            /* get the number of the controller block in the same type in one array */
            curr_ptr++;
            ASSERT((p->id) < (*curr_ptr));

            /* get the address of the controller blok */
            /* the id0 address */
            curr_ptr++;
            p->page = *(curr_ptr + 2*(p->id));
            p->reg  = *(curr_ptr + 2*(p->id) + 1);
            break;
        }

        curr_ptr += array_len;
    }

    /* can't find vol ctrl */
    if(curr_idx >= array_num)
    {
        return FALSE;
    }

    else
    {
        return TRUE;
    }
}

static bool DSPDrv_ChkMagicNum(const uint8 *p)
{
    uint8 magic_num[MAGIC_NUMBER_NUM];

    memcpy(magic_num, p, MAGIC_NUMBER_NUM);
    if((magic_num[0] == MAGIC_NUMBER_EVEN) && (magic_num[2] == MAGIC_NUMBER_EVEN)
       && (magic_num[4] == MAGIC_NUMBER_EVEN) && (magic_num[6] == MAGIC_NUMBER_EVEN)
       && (magic_num[1] == MAGIC_NUMBER_ODD) && (magic_num[3] == MAGIC_NUMBER_ODD)
       && (magic_num[5] == MAGIC_NUMBER_ODD) && (magic_num[7] == MAGIC_NUMBER_ODD))
    {
        return TRUE;
    }

    else
    {
        return FALSE;
    }
}

/* unzip the coef slot and sends it out */
static void DSPDrv_UnZipCoefData(void* p, const uint8* start, uint8 len)
{
    const uint8* curr_ptr;
    uint32 pos_data = 0;
    uint8 curr_array[MINI_AD_DATA_BYTE_LEN * MINI_AD_DATA_NUM_PER_PAGE + 1] = {0, 0};
    uint8 cnt;
    uint8 tail_zero_num;

    /* get the first byte as the start address */
    curr_ptr = start;
    cnt = *curr_ptr;
    curr_array[0] = (cnt & 0x0F);
    tail_zero_num = ((cnt & 0xF0) >> 4);

    /* get the pos data */
    curr_ptr++;
    memcpy((uint8*)(&pos_data), curr_ptr, POS_DATA_BYTE_LEN);
    curr_ptr += POS_DATA_BYTE_LEN;

    if(!pos_data)
    {
        /* There is something wrong with the coef data!!*/
        ASSERT(0);
        return;
    }

    /* go throught the pos_data, copy the data according to the pos_data */
    cnt = 1;
    while((uint32)curr_ptr - (uint32)start < len)
    {
        if(!pos_data)
        {
            /* There is something wrong with the coef data!!*/
            ASSERT(0);
            break;
        }
        if(pos_data & 0x01)
        {
            /* if this bit is 1 */
            memcpy(&(curr_array[cnt]), curr_ptr, MINI_AD_DATA_LEN);
            curr_ptr += MINI_AD_DATA_LEN;
        }

        pos_data = (pos_data >> 1);
        cnt += MINI_AD_DATA_BYTE_LEN;
    }

    /* sends out by I2C */
    cnt += tail_zero_num * MINI_AD_DATA_BYTE_LEN;
    DSPDrv_I2cWrite(((cDSPDrv*)p)->pI2CConfig->address, cnt, curr_array);
}

/* unzip the instruction slot and sends it out */
static void DSPDrv_UnZipInstructionData(void* p, const uint8* start, uint8 len)
{
    const uint8* curr_ptr;
    uint32 pos_data = 0;
    uint8 curr_array[MINI_AD_DATA_BYTE_LEN * MINI_AD_DATA_NUM_PER_PAGE + 1] = {0, 0};
    uint8 cnt;

    /* get the first byte as the start address */
    curr_ptr = start;
    curr_array[0] = ((*curr_ptr) & 0x0F);

    /* get the pos data */
    curr_ptr++;
    memcpy((uint8*)(&pos_data), curr_ptr, POS_DATA_BYTE_LEN);
    curr_ptr += POS_DATA_BYTE_LEN;

    /* go throught the pos_data, copy the data according to the pos_data */
    cnt = 1;
    while(cnt < MINI_AD_DATA_BYTE_LEN * MINI_AD_DATA_NUM_PER_PAGE)
    {
        if(pos_data & 0x01)
        {
            /* if this bit is 1 */
            memcpy(&(curr_array[cnt]), curr_ptr, MINI_AD_DATA_LEN);
            curr_ptr += MINI_AD_DATA_LEN;
            if((curr_array[cnt] == MINI_AD_INSTRUCTION_END_BYTE_0) && (curr_array[cnt+1] == MINI_AD_INSTRUCTION_END_BYTE_1) \
               && (curr_array[cnt+2] == MINI_AD_INSTRUCTION_END_BYTE_2))
            {
                cnt += MINI_AD_DATA_BYTE_LEN;
                break;
            }
        }

        pos_data = (pos_data >> 1);
        cnt += MINI_AD_DATA_BYTE_LEN;
    }

    /* sends out by I2C */
    DSPDrv_I2cWrite(((cDSPDrv*)p)->pI2CConfig->address, cnt, curr_array);
}

/* cope with the mini_A nad mini_D coef data */
static void DSPDrv_DispatchCoefData(void* p, const uint8* start, const uint8* end)
{
    const uint8* curr_ptr;
    uint8 array[PAGE_SETTING_SLOT_LEN];
    uint8 cnt;
    uint8 len;

    for(cnt = 0; cnt < 2; cnt++)
    {
        curr_ptr = start;
        /* go through all the slots */
        while((uint32)curr_ptr < (uint32)end)
        {
            len = *curr_ptr;
            curr_ptr++;
            /* when this is a page setting */
            if(len == PAGE_SETTING_SLOT_LEN)
            {
                /* copy the data */
                memcpy(array, curr_ptr, PAGE_SETTING_SLOT_LEN);
                if(cnt)
                {
                    array[1] += BUFFER_AB_OFFSET;
                }
                DSPDrv_I2cWrite(((cDSPDrv*)p)->pI2CConfig->address, len, array);
            }

            /* else, zip and sends out data */
            else
            {
                DSPDrv_UnZipCoefData(p, curr_ptr, len);
            }

            curr_ptr += len;
        }
    }
}

/* cope with the mini_A nad mini_D instruction data */
static void DSPDrv_DispatchInstructionData(void* p, const uint8* start, const uint8* end)
{
    const uint8* curr_ptr;
    uint8 array[PAGE_SETTING_SLOT_LEN];
    uint8 len;

    curr_ptr = start;
    /* go through all the slots */
    while((uint32)curr_ptr < (uint32)end)
    {
        len = *curr_ptr;
        curr_ptr++;
        /* when this is a page setting */
        if(len == PAGE_SETTING_SLOT_LEN)
        {
            /* copy the data */
            memcpy(array, curr_ptr, PAGE_SETTING_SLOT_LEN);
            DSPDrv_I2cWrite(((cDSPDrv*)p)->pI2CConfig->address, len, array);
        }

        /* else, zip and sends out data */
        else
        {
            DSPDrv_UnZipInstructionData(p, curr_ptr, len);
        }

        curr_ptr += len;
    }
}

static void DSPDrv_DispatchOrdData(void* p, const uint8* start, const uint8* end)
{
    const  uint8* curr_ptr;
    uint8  dly_arr[DELAY_SLOT_NUM] = {0, 0};
    uint16 len;

    /* go through the table slots again, reads in the data and sends out */
    curr_ptr = start;
    while((uint32)curr_ptr < (uint32)end)
    {
        len = *curr_ptr;
        curr_ptr++;

        /* check whether it is a slot of time delay */
        if((*curr_ptr == DELAY_SLOT_DATA0) && (len == DELAY_SLOT_NUM))
        {
            /* copy all the data */
            memcpy(dly_arr, curr_ptr, DELAY_SLOT_NUM);

            if((dly_arr[1] == DELAY_SLOT_DATA0) && (dly_arr[2] == DELAY_SLOT_DATA0)
               && (dly_arr[4] == DELAY_SLOT_DATA0) && (dly_arr[5] == DELAY_SLOT_DATA0)
               && (dly_arr[6] == DELAY_SLOT_DATA0) && (dly_arr[7] == DELAY_SLOT_DATA1))
            {
                BSP_BlockingDelayUs(RESET_DELAY_PER_UNIT * dly_arr[3]);
            }
        }

        else if(len)
        {
            DSPDrv_I2cWrite(((cDSPDrv*)p)->pI2CConfig->address, len, curr_ptr);
        }

        else
        {
            break;
        }

        curr_ptr += len;
    }
}

static void DSPDrv_DispatchDspConfigData(void* p)
{
    // the pointer points to the DSP global array in flash, which might be updated, so need to use "volatile"
    const uint8* volatile curr_ptr;
    /* start_add0 -- the start addr of mini_A coef in table,
     * end_add0   -- the end addr of mini_A coef in table,
     * start_add1 -- the start addr of mini_D coef in table,
     * end_add1   -- the end addr of mini_D coef in table
     */
    const uint8* add0 = NULL;
    const uint8* add1 = NULL;
    const uint8* add2 = NULL;
    const uint8* add3 = NULL;
    const uint8* add4 = NULL;
    const uint8* add5 = NULL;
    const uint8* add6 = NULL;
    uint16 len;

    /* check magic number in the head */
    if(DSPDrv_ChkMagicNum((uint8*)dsp_init_tab) == FALSE)
    {
        ASSERT(0);
    }

    /* start addr of  whole data */
    add0 = dsp_init_tab + ARRAY_START_OFF;

    /* the end addr of the table */
    curr_ptr = dsp_init_tab + TABLE_SIZE_OFF;    // get the lower byte
    len      = *curr_ptr;
    curr_ptr++;
    len   += ((*curr_ptr) << 8);
    add6   = dsp_init_tab + len;

    /* go through the table, get start and end addr for mini_A and mini_D coef */
    curr_ptr = add0;
    while((uint32)curr_ptr < (uint32)add6)
    {
        len = *curr_ptr;
        curr_ptr++;

        /* get start_add0, start_add1, end_add0 and end_add1 which stores the start addr of buffer_A in both mini_A and mini_D */
        if((len == PAGE_SETTING_SLOT_LEN) && (*curr_ptr == 0))
        {
            if(*(curr_ptr + 1) == MINI_A_BUFFER_A_START_PAGE)
            {
                if(add1 == NULL)
                {
                    add1 = (const uint8*)((uint32)curr_ptr - 1);
                }
            }

            if(*(curr_ptr + 1) > MINI_A_BUFFER_A_END_PAGE)
            {
                if((add2 == NULL) && (add1 != NULL))
                {
                    add2 = (const uint8*)((uint32)curr_ptr - 1);
                }
            }

            if(*(curr_ptr + 1) == MINI_D_BUFFER_A_START_PAGE)
            {
                if(add3 == NULL)
                {
                    add3 = (const uint8*)((uint32)curr_ptr - 1);
                }
            }

            if(*(curr_ptr + 1) > MINI_D_BUFFER_A_END_PAGE)
            {
                if((add4 == NULL) && (add3 != NULL))
                {
                    add4 = (const uint8*)((uint32)curr_ptr - 1);
                }
            }

            if(*(curr_ptr + 1) < MINI_A_BUFFER_A_START_PAGE)
            {
                if((add5 == NULL) && (add1 != NULL) && (add2 != NULL) && (add3 != NULL) && (add4 != NULL))
                {
                    add5 = (const uint8*)((uint32)curr_ptr - 1);
                }
            }
        }

        curr_ptr += len;
    }

    DSPDrv_DispatchOrdData(p, add0, add1);
    /* mini_A coef */
    DSPDrv_DispatchCoefData(p, add1, add2);
    /* mini_A instruction */
    DSPDrv_DispatchInstructionData(p, add2, add3);
    /* mini_D coef */
    DSPDrv_DispatchCoefData(p, add3, add4);
    /* mini_D instruction */
    DSPDrv_DispatchInstructionData(p, add4, add5);
    DSPDrv_DispatchOrdData(p, add5, add6);
}

void DSPDrv_I2cWrite(uint8 device_add, uint8 bytes, const uint8 *data)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    I2CDrv_MasterWrite(&dspI2c, &i2cMsg);
}

void DSPDrv_I2cRead(uint8 * bufptr, uint8 device_add, uint8 reg_add, uint16 bytes)
{
    tI2CMsg i2cMsg=
    {
        .devAddr = device_add,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    I2CDrv_MasterRead(&dspI2c, &i2cMsg);
}




/**
 * This set of process are necessary for the DSP to bootup and need to be handle by the DSP/ Audio server.
 * Important Note that the delay time should never be zero
 */

static tDspInitSection DspInitSection[]=
{
    {&DSPDrv_InitTask, 1},
    {&DSPDrv_DispatchDspConfigData, 1},
    {&DSPDrv_InitSection, 1}
};

void DSPDrv_Ctor(cDSPDrv* me)
{
    tI2CDevice * conf = (tI2CDevice *) getDevicebyId(DSP_DEV_ID, NULL);
    ASSERT(conf && (conf->address));
    me->pInitTable = DspInitSection;
    me->sectionSize = ArraySize(DspInitSection);
    me->initPhase = 0;
    me->max_vol = MAX_VOLUME;
    me->default_vol = DEFAULT_VOLUME;
    me->pI2CConfig = conf;
    me->isCreated = TRUE;
}

void DSPDrv_Xtor(cDSPDrv* me)
{
    me->pInitTable = NULL;
    me->sectionSize = 0;
    me->initPhase = 0;
    I2CDrv_Xtor(&dspI2c);
}


static void DSPDrv_InitTask(void *p)
{
    cDSPDrv *me = (cDSPDrv*)p;
    ASSERT(me->pI2CConfig);
    /* If the device type is I2C, create the DSP I2C object */
    if (me->pI2CConfig->deviceInfo.deviceType==I2C_DEV_TYPE)
    {
        I2CDrv_Ctor(&dspI2c, me->pI2CConfig);
    }
}


static void DSPDrv_InitSection(void *p)
{
    uint8 i;
    cDSPDrv *me = (cDSPDrv*)p;
    ASSERT(me->pI2CConfig);
    for(i=0; i<ArraySize(dsp_init_cmd_sec2); i++)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, sizeof(tDspData), (uint8*)(&(dsp_init_cmd_sec2[i])));
    }

    dspstatus |= DSP_INIT_SEC2_COMPLETED;
}

void DSPDrv_SetRegisterPage(cDSPDrv *me, uint8 page)
{
    uint8 tmp[2];
    tmp[0]=PAGE_SEL_REG;
    tmp[1]=page;
    DSPDrv_I2cWrite(me->pI2CConfig->address, sizeof(tmp), (uint8*)(&tmp));
}

void DSPDrv_SwapAdpFilt(cDSPDrv *me,eTlv320ac3254BufferRegion buf)
{
    uint8 adpt_filter_state = 0;
    uint8 TLV320AC_Adaptive_Filter_Swap[] = {0x01,0x00};
    if (buf == TLV320AIC_BUFFER_D)
    {
        DSPDrv_SetRegisterPage(me, 0x2c);
    }
    else //if (buf == TLV320AIC_BUFFER_A)
    {
        DSPDrv_SetRegisterPage(me, 0x08);
    }
    DSPDrv_I2cRead(&adpt_filter_state, me->pI2CConfig->address, ADC_ADP_FILT_CFG_REG, 1);
    TLV320AC_Adaptive_Filter_Swap[1] =  (adpt_filter_state  | 0x01);
    DSPDrv_I2cWrite(me->pI2CConfig->address, sizeof(TLV320AC_Adaptive_Filter_Swap), TLV320AC_Adaptive_Filter_Swap);
}

void DSPDrv_MuteDACOut(cDSPDrv *me)
{
}

void DSPDrv_UnMuteDACOut(cDSPDrv *me)
{
}

void DSPDrv_DAC_Mute(cDSPDrv *me,bool bmute)
{
    uint8 tmp[2];
    DSPDrv_SetRegisterPage(me, 0x00);
    if(bmute)
    {
        tmp[0]=0x40;
        tmp[1]=0x0d;
    }
    else
    {
        tmp[0]=0x40;
        tmp[1]=0x00;
    }
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&tmp));
}


void DSPDrv_MuteHPAOut(cDSPDrv *me)
{
    uint8 temp[3] = {HPL_DRV_GAIN_SET_REG, 0, 0};

    DSPDrv_SetRegisterPage(me, 0x01);
    DSPDrv_I2cRead(&temp[1],me->pI2CConfig->address,HPL_DRV_GAIN_SET_REG,2 );

    temp[1] |= HPL_DRV_MUTE;
    temp[2] |= HPR_DRV_MUTE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 3, (uint8*)(&temp));
}

void DSPDrv_UnMuteHPAOut(cDSPDrv *me)
{
    uint8 temp[3] = {HPL_DRV_GAIN_SET_REG, 0, 0};

    DSPDrv_SetRegisterPage(me, 0x01);
    DSPDrv_I2cRead(&temp[1],me->pI2CConfig->address,HPL_DRV_GAIN_SET_REG,2 );

    temp[1] &= ~HPL_DRV_MUTE;
    temp[2] &= ~HPR_DRV_MUTE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 3, (uint8*)(&temp));
}

void DSPDrv_MuteLoOut(cDSPDrv *me)
{
    uint8 temp[3] = {LOL_DRV_GAIN_SET_REG, 0, 0};

    DSPDrv_SetRegisterPage(me, 0x01);
    DSPDrv_I2cRead(&temp[1],me->pI2CConfig->address,LOL_DRV_GAIN_SET_REG,2 );

    temp[1] |= LOL_DRV_MUTE;
    temp[2] |= LOR_DRV_MUTE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 3, (uint8*)(&temp));
}

void DSPDrv_UnMuteLoOut(cDSPDrv *me)
{
    uint8 temp[3] = {LOL_DRV_GAIN_SET_REG, 0, 0};

    if(DSPDrv_GetLineoutStatus(me) == LINEOUT_UNPLUGGED)
    {
        DSPDrv_SetRegisterPage(me, 0x01);
        DSPDrv_I2cRead(&temp[1],me->pI2CConfig->address,LOL_DRV_GAIN_SET_REG,2 );
        temp[1] &= ~LOL_DRV_MUTE;
        temp[2] &= ~LOR_DRV_MUTE;
        DSPDrv_I2cWrite(me->pI2CConfig->address, 3, (uint8*)(&temp));
    }
}

void DSPDrv_MuteAmp(cDSPDrv *me)
{
    tDspData temp = {MFP4_FUNC_CTRL_REG, 0};

    DSPDrv_SetRegisterPage(me, 0x00);
    DSPDrv_I2cRead((uint8*)(&(temp.value)),me->pI2CConfig->address,MFP4_FUNC_CTRL_REG,1 );

    temp.value |= TRUE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&temp));
}

void DSPDrv_UnMuteAmp(cDSPDrv *me)
{
    tDspData temp = {MFP4_FUNC_CTRL_REG, 0};

    DSPDrv_SetRegisterPage(me, 0x00);
    DSPDrv_I2cRead((uint8*)(&(temp.value)),me->pI2CConfig->address,MFP4_FUNC_CTRL_REG,1 );
    temp.value |= FALSE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&temp));
}

void DSPDrv_ActivateAmp(cDSPDrv *me)
{
    tDspData temp = {MFP5_FUNC_CTRL_REG, 0};

    DSPDrv_SetRegisterPage(me, 0x00);
    DSPDrv_I2cRead((uint8*)(&(temp.value)),me->pI2CConfig->address,MFP5_FUNC_CTRL_REG,1 );

    temp.value |= TRUE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&temp));
}

void DSPDrv_DeActivateAmp(cDSPDrv *me)
{
    tDspData temp = {MFP5_FUNC_CTRL_REG, 0};

    DSPDrv_SetRegisterPage(me, 0x00);
    DSPDrv_I2cRead((uint8*)(&(temp.value)),me->pI2CConfig->address,MFP5_FUNC_CTRL_REG,1 );

    temp.value |= FALSE;
    DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&temp));
}

eLineoutStatus DSPDrv_GetLineoutStatus(cDSPDrv *me)
{
    uint8 i, j, k;
    tDspData data = {MFP1_FUNC_CTRL_REG, 0};

#ifdef NDEFFINE
    if(!(dspstatus & DSP_INIT_SEC3_COMPLETED))
    {
        return LINEOUT_UNKNOWN;   /* Init not completed, so we don't know the status */
    }
#endif
    i = 0;    /* Count times of high */
    j = 0;    /* Count times of low */
    k = PIO_CHK_TIMES;

    DSPDrv_SetRegisterPage(me, 0x00);

    while(k--)
    {
        DSPDrv_I2cRead((uint8*)(&data.value),me->pI2CConfig->address,MFP1_FUNC_CTRL_REG,1 );
        if((data.value) & MFP1_VAL)
        {
            i++;
            j = 0;
        }

        else
        {
            i = 0;
            j++;
        }

        if(i >= PIO_CHK_CFM_TIMES)
        {
            return LINEOUT_UNPLUGGED;
        }

        else if(j >= PIO_CHK_CFM_TIMES)
        {
            return LINEOUT_PLUGGED;
        }
    }

    return LINEOUT_UNKNOWN;
}

BOOL DSPDrv_IsAuxin(cDSPDrv *me)
{
    /* change the name "data" to "dspData", because with the name "data", it can't
    show in the debugging*/
    tDspData dspData = {MFP3_FUNC_CTRL_REG, 0};
#ifdef NDEFINE
    if(!(dspstatus & DSP_INIT_SEC3_COMPLETED))
    {
        return AUXIN_UNKNOWN;   /* Init not completed, so we don't know the status */
    }
#endif

    DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cRead(&(dspData.value), (me->pI2CConfig->address), MFP3_FUNC_CTRL_REG,1);
    if((dspData.value) & MFP3_VAL)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
    //return AUXIN_UNKNOWN;
}

void DSPDrv_setDACgain(cDSPDrv *me, int dB_onetenth)
{
    /* the DAC is at 0.5dB a step */
    uint8 tmp[2];
    uint8 data;
    uint8 reg[2] = {LDAC_CH_DIG_VOL_CTRL_REG, RDAC_CH_DIG_VOL_CTRL_REG};
    int i;
    if(dB_onetenth >= 0)
    {
        data = (256 - (-dB_onetenth / 5)) & 0xff;
    }
    else
    {
        data = (dB_onetenth / 5) & 0xff;
    }

    /* this is done twice for both left and right channel */
    for(i=0 ; i<2 ; i++)
    {
        tmp[0] = reg[i];
        tmp[1] = data;
        /* Select Page 0 */
        DSPDrv_SetRegisterPage(me, 0);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)(&tmp));
    }

}



uint16 DSPDrv_Init(cDSPDrv* me)
{
    //static uint16 phase=0;
    uint16 delaytime;
    ASSERT(me &&(me->pInitTable));
    me->pInitTable[me->initPhase].initSectionFunc(me);
    delaytime=me->pInitTable[me->initPhase].delaytime;
    me->initPhase++;
    if (me->initPhase == me->sectionSize)
    {
        me->initPhase = 0;
        delaytime = 0;
    }
    return delaytime;
}

#ifdef HAS_I2S_INPUT
static int DSPDrv_isPLLPowerOn(cDSPDrv *me)
{
    uint8 reg5 = 0;
    DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cRead(&reg5, me->pConfig->address, CLK_SET2_REG, 1);
    return (reg5>>7) & 0x1; //return the bit 7
}


static void DSPDrv_setPllparam(cDSPDrv *me, unsigned int R, unsigned int J, unsigned int D, unsigned int P)
{
    {
        // update J value
        uint8 tmp[2] = {CLK_SET3_REG, 0x00};
        DSPDrv_SetRegisterPage(me, 0x00);
        ASSERT(J>=4 && J<=63);
        tmp[1] = J;
        DSPDrv_I2cWrite(me->pConfig->address, 2, (uint8*)(&tmp));
    }
    {
        //update D value
        /*
         * PLL divider D value (MSB) = Page 0, Register 7, D(5:0)
         * PLL divider D value (LSB) = Page 0, Register 8, D(7:0)
         */

        // set Page 0, Register 7, D(5:0)
        {
            uint8 tmp[2] = {CLK_SET4_REG, 0x00};
            DSPDrv_SetRegisterPage(me, 0x00);
            ASSERT(D>=0 && D<=9999);
            tmp[1] = (D>>8) & 0xff;
            DSPDrv_I2cWrite(me->pConfig->address, 2, (uint8*)(&tmp));
        }
        // set Page 0, Register 8, D(7:0)
        {
            uint8 tmp[2] = {CLK_SET5_REG, 0x00};
            DSPDrv_SetRegisterPage(me, 0x00);
            ASSERT(D>=0 && D<=9999);
            tmp[1] = D & 0xff;
            DSPDrv_I2cWrite(me->pConfig->address, 2, (uint8*)(&tmp));
        }
    }
    {
        //update P and R value
        uint8 tmp[2] = {CLK_SET2_REG, 0x00};
        int bIsPLLon = DSPDrv_isPLLPowerOn(me);
        DSPDrv_SetRegisterPage(me, 0x00);
        tmp[1] |= bIsPLLon<<7;              // to preserve the status of the PLL power flag
        ASSERT(P>=1 && P<=8);
        tmp[1] |= (P & 0x07) << 4;   // the P value lies in bit4-6
        ASSERT(R>=1 && R<=4);
        tmp[1] |= (R & 0x07);        // the R value lies in bits0-3
        DSPDrv_I2cWrite(me->pConfig->address, 2, (uint8*)(&tmp));
    }
}


static void DSPDrv_setDacPower(cDSPDrv *me, eDspPowerSetting setting)
{
    uint8 currentValue = 0;
    {
        /* read existing value */
        DSPDrv_SetRegisterPage(me, 0);
        DSPDrv_I2cRead(&currentValue, me->pConfig->address, DAC_CH_SET1_REG, 1);
    }
    uint8 tmp[2] = {DAC_CH_SET1_REG, 0};
    if(setting == DSP_POWER_SETTINGS_ON)
    {
        tmp[1] = currentValue | (1<<6) | (1<<7);    //turn on bit6 and 7
    }
    else if(setting == DSP_POWER_SETTINGS_OFF)
    {
        tmp[1] = currentValue & (~(1<<6)) & (~(1<<7));  //turn off bit 6 and 7
    }
    else
    {
        ASSERT(0);
    }
    //DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    return;
}

static void DSPDrv_setAdcPower(cDSPDrv *me, eDspPowerSetting setting)
{

    uint8 currentValue = 0;
    {
        /* read existing value */
        DSPDrv_SetRegisterPage(me, 0);
        DSPDrv_I2cRead(&currentValue, me->pConfig->address, ADC_CH_SET_REG, 1);
    }

    uint8 tmp[2] = {ADC_CH_SET_REG, 0};
    if(setting == DSP_POWER_SETTINGS_ON)
    {
        tmp[1] = currentValue | (1<<6) | (1<<7);    //turn on bit6 and 7
    }
    else if(setting == DSP_POWER_SETTINGS_OFF)
    {
        tmp[1] = currentValue & (~(1<<6)) & (~(1<<7));  //turn off bit 6 and 7
    }
    else
    {
        ASSERT(0);
    }
    //DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    return;

}


static void DSPDrv_set_M_N_ADC_DAC_Clk_divider_power(cDSPDrv *me, eDspPowerSetting setting)
{
    /* go through all the 4 registers for:
     * ->  NADC:    reg 18
     * ->  MADC:    reg 19
     * ->  NDAC:    reg 11
     * ->  MDAC:    reg 12
     */
    uint8 i;
    uint8 reg[] = {11, 12, 18, 19};
    DSPDrv_SetRegisterPage(me, 0);
    for(i=0 ; i<ArraySize(reg); i++)
    {
        uint8 currentValue = 0;
        {
            /* read existing value */

            DSPDrv_I2cRead(&currentValue, me->pConfig->address, reg[i], 1);
        }

        uint8 tmp[2] = {0};
        tmp[0] = reg[i];
        if(setting == DSP_POWER_SETTINGS_ON)
        {
            tmp[1] = currentValue | (1<<7);    //turn on bit6 and 7
        }
        else if(setting == DSP_POWER_SETTINGS_OFF)
        {
            tmp[1] = currentValue & (~(1<<7));  //turn off bit 6 and 7
        }
        else
        {
            ASSERT(0);
        }
        //DSPDrv_SetRegisterPage(me, 0);
        DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    }
    return;

}



static int DSPDrv_switchAudioInterfaceTo(cDSPDrv *me, eTlv320ac3254AudioInterface interface)
{
    uint8 tmp[2] = {AUD_ITF_SET5_REG, 0};
    switch(interface)
    {
        case TLV320AC3254_AUDIO_INTERFACE_PRIMARY:
            tmp[1]=0x00;
            break;
        case TLV320AC3254_AUDIO_INTERFACE_SECONDARY:
            tmp[1] = 0x0F;
            break;
        default:
            ASSERT(0);
    }
    DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    return 0;
}



static void DSPDrv_setPLLPower(cDSPDrv *me, eDspPowerSetting setting)
{
    uint8 tmp[2] = {CLK_SET2_REG, 0};
    DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cRead(&(tmp[1]), me->pConfig->address, CLK_SET2_REG, 1);

    if(setting == DSP_POWER_SETTINGS_ON)
    {
        tmp[1] |= (1<<7); //turn on bit7 as it is the PLL power bit
    }
    else if(setting == DSP_POWER_SETTINGS_OFF)
    {
        tmp[1] &= (~(1<<7)); //turn off bit7 as it is the PLL power bit
    }
    else
    {
        ASSERT(0);
    }

    //DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);

}

static void DSPDrv_setPllClockSource(cDSPDrv *me, eTlv320ac3254ClockSource clk_src)
{
    uint8 tmp[2] = {CLK_SET1_REG, 3};
    switch(clk_src)
    {
        case TLV320AC3254_CLK_MCLK:
            tmp[1] |= 0x00<<2;
            break;
        case TLV320AC3254_CLK_BCLK:
            tmp[1] |= 0x01<<2;
            break;
        case TLV320AC3254_CLK_GPIO:
            tmp[1] |= 0x02<<2;
            break;
        case TLV320AC3254_CLK_MPF1:
            tmp[1] |= 0x03<<2;
            break;
    };
    DSPDrv_SetRegisterPage(me, 0);
    DSPDrv_I2cWrite(me->pConfig->address, 2, (uint8*)(&tmp));

}

#endif

#ifdef AUDIO_MULTI_SOURCE
void DSPDrv_SetInputChannelDifferential(cDSPDrv *me)
{
    uint8 tmp_val[5] = {0x2C,0,0,1,0};
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, 0x0A, tmp_val, 5);
}
void DSPDrv_SetInputSingleEnded(cDSPDrv *me)
{
    uint8 tmp_val[5] = {0x2C,0,0,2,0};
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, 0x0A, tmp_val, 5);
}

//TODO: this function is not generic enough and need to be re-write in generic form so it will be easy to re-config.
void DSPDrv_AnalogInputChannelSelect(cDSPDrv *me, eTlv320ac3254AnalogInputChannel audio_input)
{
    DSPDrv_SetRegisterPage(me, 0x01);
    if(audio_input == TLV320AC3254_IN_NONE)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_LINE_BT_MIX_52);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_LINE_BT_MIX_54);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_LINE_BT_MIX_55);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_LINE_BT_MIX_57);
    }
    else if(audio_input == TLV320AC3254_IN1)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1R_52);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1L_54);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1R_55);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1L_57);
    }
    else if(audio_input == TLV320AC3254_IN2)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN2R_52);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN2L_54);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN2R_55);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN2L_57);
    }
    else if (audio_input == TLV320AC3254_IN3)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN3R_52);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN3L_54);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN3R_55);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN3L_57);
    }
    else if (audio_input == TLV320AC3254_IN1_IN2_10K)
    {
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1L_IN2L_52);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1L_IN2L_54);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1R_IN2R_57);
        DSPDrv_I2cWrite(me->pI2CConfig->address, 2, (uint8*)TLV320AC_IN1R_IN2R_55);
    }

#ifdef HAS_DIFFERENTIAL_CHANNEL
    switch(audio_input)
    {
        case TLV320AC3254_IN3:
        {
#ifdef CHANNEL_3_DIFFERENTIAL
            DSPDrv_SetInputChannelDifferential(me);
#else
            DSPDrv_SetInputSingleEnded(me);
#endif
            break;
        }
        case TLV320AC3254_IN1_IN2_10K:
        {
#ifdef CHANNEL_1_2_DIFFERENTIAL
            DSPDrv_SetInputChannelDifferential(me);
#else
            DSPDrv_SetInputSingleEnded(me);
#endif
            break;
        }
        default:
            break;
    }
#endif
}
#endif // AUDIO_MULTI_SOURCE

#ifndef HAS_I2S_INPUT
void DSPDrv_set_Input(cDSPDrv *me, eAudioCtrlDriverInput input)
{
    ASSERT(input < AUDIOCTRL_DRIVER_INPUT_MAX);
    switch(input)
    {

        case AUDIOCTRL_DRIVER_INPUT_ANALOG_3:
            DSPDrv_AnalogInputChannelSelect(me, TLV320AC3254_IN3);
            break;
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1_2:
            DSPDrv_AnalogInputChannelSelect(me, TLV320AC3254_IN1_IN2_10K);
            break;
        default:
            break;

    }

}

#else
void DSPDrv_set_Input(cDSPDrv *me, eAudioCtrlDriverInput input)
{
    ASSERT(input < AUDIOCTRL_DRIVER_INPUT_MAX);
    switch(input)
    {
        case AUDIOCTRL_DRIVER_INPUT_I2S_1:
            /* update the clock source and PLL */
            DSPDrv_setDacPower(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_setAdcPower(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_set_M_N_ADC_DAC_Clk_divider_power(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_setPLLPower(me, DSP_POWER_SETTINGS_OFF);

            DSPDrv_setPllClockSource(me, TLV320AC3254_CLK_BCLK);
            DSPDrv_setPllparam(me, 2, 32, 0, 1);

            /* BCLK, LRCLK have to be present for DSP operation.
             * For I2S input, the DSP is provided with those clocks, so we set our DSP BCLK and LRCLK as inputs,
             * and direct them to the primary audio interface
             * */
            DSPDrv_switchAudioInterfaceTo(me, TLV320AC3254_AUDIO_INTERFACE_PRIMARY);
            DSPDrv_setPLLPower(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_set_M_N_ADC_DAC_Clk_divider_power(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_setAdcPower(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_setDacPower(me, DSP_POWER_SETTINGS_ON);
            break;

        case AUDIOCTRL_DRIVER_INPUT_ANALOG_3:
            /* update the clock source and PLL */
            DSPDrv_setDacPower(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_setAdcPower(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_set_M_N_ADC_DAC_Clk_divider_power(me, DSP_POWER_SETTINGS_OFF);
            DSPDrv_setPLLPower(me, DSP_POWER_SETTINGS_OFF);

            DSPDrv_setPllClockSource(me, TLV320AC3254_CLK_MCLK);
            DSPDrv_setPllparam(me, 1, 7, 5264, 1);

            /* BCLK, LRCLK have to be present for DSP operation.
            * unlike I2S input, BCLK and LRCLK are not provided so we basically generate them from DSP and output them.
            * we output to the secondary audio interface so we wont have signal conflict
            * with the connected I2S device on the primary audio interface
            * */
            DSPDrv_switchAudioInterfaceTo(me, TLV320AC3254_AUDIO_INTERFACE_SECONDARY);
            DSPDrv_setPLLPower(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_set_M_N_ADC_DAC_Clk_divider_power(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_setAdcPower(me, DSP_POWER_SETTINGS_ON);
            DSPDrv_setDacPower(me, DSP_POWER_SETTINGS_ON);
#ifdef AUDIO_MULTI_SOURCE
            DSPDrv_AnalogInputChannelSelect(me, TLV320AC3254_IN3);
#endif // AUDIO_MULTI_SOURCE
            break;
        case AUDIOCTRL_DRIVER_INPUT_ANALOG_1_2:
            DSPDrv_AnalogInputChannelSelect(me, TLV320AC3254_IN1_IN2_10K);
            break;
        default:
            break;

    }

}
#endif

static void DSPDrv_WriteRegister(cDSPDrv *me, eTlv320ac3254BufferRegion BuffRegion, uint8 page, uint8* data, uint8 data_size)
{
    if (BuffRegion == TLV320AIC_BUFFER_A)
    {
        DSPDrv_SetRegisterPage(me, page);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);

        DSPDrv_SetRegisterPage(me, page + BUFFER_AB_OFFSET);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);

        DSPDrv_SwapAdpFilt(me, BuffRegion);

        DSPDrv_SetRegisterPage(me, page);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);

        DSPDrv_SetRegisterPage(me, page + BUFFER_AB_OFFSET);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);
    }
    else if (BuffRegion == TLV320AIC_BUFFER_D)
    {
        DSPDrv_SetRegisterPage(me, page);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);
        DSPDrv_SwapAdpFilt(me, BuffRegion);
        DSPDrv_SetRegisterPage(me, page);
        DSPDrv_I2cWrite(me->pI2CConfig->address, data_size, (uint8*)data);

    }
}

#ifdef GAIN_ADJUSTMENT_FOR_DIFF_BATTLEVEL
void DSPDrv_SetGain(cDSPDrv *me, uint8 level)
{
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, GAIN_PAGE0, gain_coefs[level], 4);

    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, GAIN_PAGE1, gain_coefs[level], 4);
}
#endif


void DSPDrv_SetVol(cDSPDrv *me, uint8 vol)
{
    uint8        volume_iic[4] = {0, 0, 0, 0};
    tTunableInfo tunable_info;

    tunable_info.head = (uint8*)dsp_tunable_tab;
    tunable_info.type = ARRAY_TYPE_VOL_CTRL;
    tunable_info.id   = 0;

    if(DSPDrv_GetTunableInfo(&tunable_info) == FALSE)
    {
        ASSERT(0);
    }

    if(vol > MAX_VOLUME)
    {
        vol = MAX_VOLUME;
    }

    volume_iic[0] = tunable_info.reg;
    memcpy(&volume_iic[1],  &MASTER_VOL[MAX_VOLUME - vol][0], 3);

#ifdef DSP_OPTIMIZING
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, tunable_info.page, volume_iic, 4);
#else
    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SwapAdpFilt(me, TLV320AIC_BUFFER_A);

    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);
#endif
}

#ifdef REDUCE_TOTAL_GAIN_WHILE_LOW_POWER
/**
 * Refresh volume when battery is low, reduce it by 6dB
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_SetVolForLowPower(cDSPDrv *me, uint8 vol)
{
    uint8        volume_iic[4] = {0, 0, 0, 0};
    tTunableInfo tunable_info;
    uint32       temp = 0;
    uint8        idx  = 0;

    tunable_info.head = (uint8*)dsp_tunable_tab;
    tunable_info.type = ARRAY_TYPE_VOL_CTRL;
    tunable_info.id   = 0;

    if(DSPDrv_GetTunableInfo(&tunable_info) == FALSE)
    {
        ASSERT(0);
    }

    if(vol > MAX_VOLUME)
    {
        vol = MAX_VOLUME;
    }

    temp = 0;
    for(idx = 0; idx < 3; idx++)
    {
        temp |= (MASTER_VOL[MAX_VOLUME - vol][idx] << (16 - idx*8));
    }

    /* right-shift it 1 byte */
    if(temp > 0x01)
    {
        temp = (temp >> 1);
    }

    for(idx = 1; idx < 4; idx++)
    {
        volume_iic[idx] |= (uint8)(temp >> (24 - idx*8));
    }
    volume_iic[0] = tunable_info.reg;

#ifdef DSP_OPTIMIZING
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, tunable_info.page, volume_iic, 4);

#else
    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SwapAdpFilt(me, TLV320AIC_BUFFER_A);

    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);

    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, 4, (uint8*)volume_iic);
#endif
}
#endif

#ifdef AUDIO_LIMITER_FOR_LOW_POWER
/**
 * Refresh DRCs for normal power
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_SetDrcForNormalPower(cDSPDrv *me)
{
    /* MSB ADC COEFF No. 14 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, DRC_PAGE0, (uint8*)drc_coefs_batt_mode_p0, 3);
    /* MSB ADC COEFF No. 270 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, DRC_PAGE1, (uint8*)drc_coefs_batt_mode_p1, 3);

    /* MSB DAC COEFF No. 67*/
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE2, (uint8*)drc_coefs_batt_mode_p2_1, 4);
    /* MSB DAC COEFF No. 70 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE2, (uint8*)drc_coefs_batt_mode_p2_2, 2);

    /* MSB DAC COEFF No. 323 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE3, (uint8*)drc_coefs_batt_mode_p3_1, 4);
    /* MSB DAC COEFF No. 326 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE3, (uint8*)drc_coefs_batt_mode_p3_2, 2);
}

/**
 * Refresh DRCs when power is low
 *
 * @param      uint8        target volume
 * @return     void
 */
void DSPDrv_SetDrcForLowPower(cDSPDrv *me)
{
}

/**
 * Refresh DRCs for adaptor mode
 *
 * @param      void
 * @return     void
 */
void DSPDrv_SetDrcForAdaptorMode(cDSPDrv *me)
{
    /* MSB ADC COEFF No. 14 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, DRC_PAGE0, (uint8*)drc_coefs_adaptor_mode_p0, 3);

    /* MSB ADC COEFF No. 270 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, DRC_PAGE1, (uint8*)drc_coefs_adaptor_mode_p1, 3);

    /* MSB DAC COEFF No. 67*/
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE2, (uint8*)drc_coefs_adaptor_mode_p2_1, 4);
    /* MSB DAC COEFF No. 70 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE2, (uint8*)drc_coefs_adaptor_mode_p2_2, 2);

    /* MSB DAC COEFF No. 323 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE3, (uint8*)drc_coefs_adaptor_mode_p3_1, 4);
    /* MSB DAC COEFF No. 326 */
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_D, DRC_PAGE3, (uint8*)drc_coefs_adaptor_mode_p3_2, 2);

}
#endif

int DSPDrv_setStereoMux(cDSPDrv *me, eDspMuxChannel ch)
{
    uint8        tmp[5];
    tTunableInfo tunable_info;

    tunable_info.head = (uint8*)dsp_tunable_tab;
    tunable_info.type = ARRAY_TYPE_SRC_SW;
    tunable_info.id   = 0;
    if(DSPDrv_GetTunableInfo(&tunable_info) == FALSE)
    {
        ASSERT(0);
    }

    switch(ch)
    {
        case DSP_MUX_CH1:
            // 00000100 should be 1, which means CH1 on the
            // Stereo_Mux_TwoToOne
            tmp[0] = tunable_info.reg;
            tmp[1] = 0x00;
            tmp[2] = 0x00;
            tmp[3] = 0x01;
            tmp[4] = 0x00;
            break;
        case DSP_MUX_CH2:
            // ffffff00 is -1, which means CH2 on the
            // Stereo_Mux_TwoToOne
            tmp[0] = tunable_info.reg;
            tmp[1] = 0xff;
            tmp[2] = 0xff;
            tmp[3] = 0xff;
            tmp[4] = 0x00;
            break;
        default:
            TP_PRINTF("invalid param at %s\n", __func__);
            ASSERT(0);
            return -1;
    }

#ifdef DSP_OPTIMIZING
    DSPDrv_WriteRegister(me, TLV320AIC_BUFFER_A, tunable_info.page, tmp, sizeof(tmp));
#else
    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);

    DSPDrv_SwapAdpFilt(me, TLV320AIC_BUFFER_A);

    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
    DSPDrv_SetRegisterPage(me, tunable_info.page + BUFFER_AB_OFFSET);
    DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
#endif

    return 0;
}

BOOL DSPDrv_HasMusicStream(cDSPDrv *me)
{
    uint8 tmp_val = 0x0;
    tTunableInfo tunable_info;

    tunable_info.head = (uint8*)dsp_tunable_tab;
    tunable_info.type = ARRAY_TYPE_MUS_DET;
    tunable_info.id   = 0;
    if(DSPDrv_GetTunableInfo(&tunable_info) == FALSE)
    {
        ASSERT(0);
    }

    DSPDrv_SwapAdpFilt(me, TLV320AIC_BUFFER_A);
    DSPDrv_SetRegisterPage(me, tunable_info.page);
    DSPDrv_I2cRead(&tmp_val, (me->pI2CConfig->address), tunable_info.reg, 1);

    if((tmp_val == 0x80) || (tmp_val == 0x0))
    {
        return FALSE;
    }

    else
    {
        return TRUE;
    }
}

static void DSPDrv_SetStaticEQ(cDSPDrv *me, eDspSettId dspSettId, BOOL eqStatus)
{
    if(DSP_PLAINEQ_SETT_ID == dspSettId)
    {
#if \
        defined(PLAIN_EQ_MUX_0_PAGE)    && \
        defined(PLAIN_EQ_MUX_0_REG)     && \
        defined(PLAIN_EQ_MUX_1_PAGE)    && \
        defined(PLAIN_EQ_MUX_1_REG)

#define NUM_MUX_FOR_EQ  2
        uint8 tmp[5];
        uint8 i = 0;
        uint8 StereoMuxPage[NUM_MUX_FOR_EQ] = {0};
        uint8 StereoMuxAddr[NUM_MUX_FOR_EQ] = {0};
        uint8 StereoMuxBuffer[NUM_MUX_FOR_EQ] = {0};
        StereoMuxPage[0] = PLAIN_EQ_MUX_0_PAGE;
        StereoMuxAddr[0] = PLAIN_EQ_MUX_0_REG;
        StereoMuxBuffer[0] = PLAIN_EQ_MUX_0_BUFFER;
        StereoMuxPage[1] = PLAIN_EQ_MUX_1_PAGE;
        StereoMuxAddr[1] = PLAIN_EQ_MUX_1_REG;
        StereoMuxBuffer[1] = PLAIN_EQ_MUX_1_BUFFER;

        for(i=0 ; i<NUM_MUX_FOR_EQ ; i++)
        {
            switch(eqStatus)
            {
                case FALSE:
                {
                    // 00000100 should be 1, which means CH1 on the
                    // Stereo_Mux_TwoToOne
                    //
                    // in our design, CH1 refers to the EQ-enabled path
                    tmp[0]=StereoMuxAddr[i];
                    tmp[1]=0x00;
                    tmp[2]=0x00;
                    tmp[3]=0x01;
                    tmp[4]=0x00;
                }
                break;
                case TRUE:
                {
                    // ffffff00 is -1, which means CH2 on the
                    // Stereo_Mux_TwoToOne
                    //
                    // in our design, CH2 refers to the plain EQ path
                    tmp[0]=StereoMuxAddr[i];
                    tmp[1]=0xff;
                    tmp[2]=0xff;
                    tmp[3]=0xff;
                    tmp[4]=0x00;
                }
                break;
                default:
                {
                    TP_PRINTF("invalid param at %s\n", __func__);
                    ASSERT(0);
                }
            }
#ifdef DSP_OPTIMIZING
            DSPDrv_WriteRegister(me, StereoMuxBuffer[i], StereoMuxPage[i], tmp, sizeof(tmp));
#else
            DSPDrv_SetRegisterPage(me, StereoMuxPage[i]);
            DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
            DSPDrv_SetRegisterPage(me, StereoMuxPage[i]+BUFFER_AB_OFFSET);
            DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);

            DSPDrv_SwapAdpFilt(me, StereoMuxBuffer[i]);

            DSPDrv_SetRegisterPage(me, StereoMuxPage[i]);
            DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
            DSPDrv_SetRegisterPage(me, StereoMuxPage[i]+BUFFER_AB_OFFSET);
            DSPDrv_I2cWrite(me->pConfig->address, sizeof(tmp), tmp);
#endif
        }
#else
        /* the needed address and page are not defined by the user.
         * so the user of this driver cannot use this plain EQ function
         * we assert the program if the user calls this function */
        TP_PRINTF("invalid function call to: DSPDrv_setPlainEQ(), assert now\n");
        ASSERT(0);
#endif
    }
    else
    {
        /* user passed a wrong EQ ID, something is wrong */
        ASSERT(0);
    }
}

/*
 * Below are the public functions which are not used in this project but have to
 * be declared to supress compiling errors
 */
void DSPDrv_SetAudio(cDSPDrv *me, eDspSettId dspSettId, BOOL enable)
{
    uint8 i;
    for(i = 0; i < ArraySize(ctrIdEQIdMap); i++)
    {
        if(dspSettId == ctrIdEQIdMap[i].dspSettid)
        {
            break;
        }
    }
    ASSERT(dspSettId < DSP_SETT_ID_MAX);
    switch(dspSettId)
    {
        case DSP_VOLUME_SETT_ID:
        {
            if(enable)
            {
                int8 volumeLevel = 0;
                volumeLevel =  *(int8*)Setting_Get(ctrIdEQIdMap[i].settingId);
                if(volumeLevel <= MAX_VOLUME && volumeLevel >= 0)
                {
                    DSPDrv_SetVol(me,volumeLevel);
                }
            }
            else
            {
                DSPDrv_SetVol(me,0);
            }
        }
        break;
        case DSP_PLAINEQ_SETT_ID:
        {
            DSPDrv_SetStaticEQ(me, dspSettId, enable);
        }
        break;
        default:
            break;
    }
}
