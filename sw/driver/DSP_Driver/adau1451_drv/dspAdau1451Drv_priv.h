/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  dsp 1451 driver
                  -------------------------

                  SW Module Document




@file        dsp_adau1451_driver.h
@brief       This file declares the structures and macros for adau1451
@author      Daniel Qin
@date        2015-12-15
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/
#ifndef __ADAU1451_DRV_H__
#define __ADAU1451_DRV_H__
#include "DspDrv1451.h"

#define DB_INDEX_OF_LP     0
#define DB_INDEX_OF_RGC    12
#define DB_INDEX_OF_PEQ1   3
#define DB_INDEX_OF_PEQ2   6
#define DB_INDEX_OF_PEQ3   9
#define DB_INDEX_OF_PHASE  2
#define DB_INDEX_OF_VOL    19

#define ADDR_OFFSET_0      0
#define ADDR_OFFSET_1      1
#define ADDR_OFFSET_2      2
#define ADDR_OFFSET_3      3
#define ADDR_OFFSET_4      4

#define CTR_ADDR_GROUP_1   1
#define CTR_ADDR_GROUP_2   2
#define CTR_ADDR_GROUP_3   3
#define CTR_ADDR_GROUP_4   4

#define EQ_CTRL_RAM_SLOT_NUM    (5)
#define PARAM_RAM_WORD_LEN      (4)
#define EQ_CTRL_RAM_BYTE_NUM    (PARAM_RAM_WORD_LEN*EQ_CTRL_RAM_SLOT_NUM)

#define PI                      (3.1415926536)
#define NUM_OF_FILTER_ADD       (4)
#define NUM_OF_RGC_SET_ADD      (2)
#define PARAM_EQ_GAIN           (0)

#define NO_PHASE_SHIFT          (1)
#define MAX_DELAY_SAMPLES       480

#define PROGRAM_RAM_ADDRESS_LOW_BOUNDARY        (0x0400)
#define PROGRAM_RAM_ADDRESS_HIGH_BOUNDARY       (0x07FF)
#define PARAM_RAM_ADDRESS_LOW_BOUNDARY          (0x0000)
#define PARAM_RAM_ADDRESS_HIGH_BOUNDARY         (0x03FF)
#define REG_ADDRESS_LEN                         (2)
#define PROGRAM_RAM_DATA_LEN                    (5)
#define PARAM_RAM_PAGE_SIZE                     (32)
#define PARAM_RAM_DATA_LEN                      (4)
#define SHIFT_EIGHT_BIT                         (8)
#define MASK_LOW_BYTE                           (0xFF)
//#define AUDIO_DETECT_REGISTER                   0x0801

#define POS_DATA_BYTE_LEN                       (4)

#define REG_ADDR_HIBERNATE                      (0xF400)
#define DATA_HIBERNATE_ENABLE                   (0x0001)
#define DATA_HIBERNATE_DISABLE                  (0x0000)

#define REG_ADDR_SPDIF_AUTO_RESTART             (0xF604)
#define DATA_SPDIF_AUTO_RESTART_ENABLE          (0x0001)
#define DATA_SPDIF_AUTO_RESTART_DISABLE         (0x0000)

/* ASRC Conversion Rateg */
#define ADAU1451_ASRC_RATIO_LEN                 (2)
#define REG_ADDR_ASRC0_RATIO                    (0xF582) //ASRC0~ASRC7: 0xF582~0xF589

typedef struct tCtrIdEQIdMap
{
    eAudioSettId dspSettid;
    uint8        dbIndex;  /* setting database index */
}tCtrIdEQIdMap;

typedef struct tCtrlInputIdMap
{
    eAudioChannel InputId; /* the input id requedted by audio server. */
    uint8                 controllerId; /*in case there can be more than 1 controller address in one array, we need to find the correct address*/
    uint8                 sourceIdx; /* the index of the source on the controller*/
}tCtrlInputIdMap;

typedef struct
{
    const uint8* head;     /*pointer to the tabe head*/
    uint8        type;     /*controller type*/
    uint8          id;     /*in case there can be more than 1 controller address in one array, we need to find the correct address*/
    uint16       addr;
} tTunableInfo;

typedef enum
{
    INIT_DATA_SEGMENT_1,
    INIT_DATA_SEGMENT_2,
    INIT_DATA_SEGMENT_3,
    INIT_DATA_SEGMENT_4,
    INIT_DATA_SEGMENT_5
}eInitDataSegment;


typedef struct tInitTableTXCtrl
{
    const uint8*        pOffset;
    uint32              NumOfDataInRow;
    uint16              tableRowNum;
    eInitDataSegment    dataSegment;
}tInitTableTXCtrl;

/*
 * The filter types we can support
 */
typedef enum
{
    FILTER_TYPE_PEAKING,
    FILTER_TYPE_TONE_HIGH_SHELF,
    FILTER_TYPE_TONE_LOW_SHELF,
    FILTER_TYPE_GENERAL_HIGH_PASS,
    FILTER_TYPE_GENERAL_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_LOW_PASS,
    FILTER_TYPE_BUTTWORTH_HIGH_PASS,
    FILTER_TYPE_BESSEL_LOW_PASS,
    FILTER_TYPE_BESSEL_HIGH_PASS,
    FILTER_TYPE_FIRST_ORDER_HIGH_PASS,
    FILTER_TYPE_FIRST_ORDER_LOW_PASS,
} eDspFiltTyp;

/* necessary constants for each EQ part */
/* THE FOLLOWING ENUM IS PRODUCT-RELATED, USER CAN DEFINE THE AUDIO CFG CHOICES HERE */
/* Tihs is just an example, user can modify it according the HW design */
typedef enum
{
    PROD_AUD_IN_OPTION0,
    PROD_AUD_IN_OPTION1,
    PROD_AUD_IN_OPTION2
} eDspAudInOption;

typedef enum
{
    PROD_AUD_OUT_OPTION0
} eDspAudOutOption;


#define AUDIO_DETECTED                        (0x00)
#define AUDIO_NOT_DETECTED                    (0x80)

static void DSPDrv1451_Reset1(void *p);
static void DSPDrv1451_Reset2(void *p);
static void DSPDrv1451_Reset3(void *p);
static void DSPDrv1451_InitI2c(void *p);
static void DSPDrv1451_InitSection1(void *p);
static void DSPDrv1451_InitSection2(void *p);
static void DSPDrv1451_InitSection3(void *p);
static void DSPDrv1451_InitSection4(void *p);
static void DSPDrv1451_InitSection5(void *p);
static void DSPDrv1451_InitSection6(void *p);
static void DSPDrv1451_InitSection7(void *p);
static void DSPDrv1451_InitSection8(void *p);
static void DSPDrv1451_InitSection9(void *p);
static void DSPDrv1451_InitSection10(void *p);
static void DSPDrv1451_InitSection11(void *p);
static void DSPDrv1451_InitSection12(void *p);
static void DSPDrv1451_Xtor_Cust(cDSPDrv1451* me);
static void DSPDrv1451_SetPower(bool state);
static void DSPDrv1451_I2cWrite(cDSPDrv1451 *me, uint16 length, const uint8 *data);
static void DSPDrv1451_I2cRead(cDSPDrv1451 *me, uint32 regAddr, uint16 length, const uint8 *data);
static void DSPDrv1451_GetCtrlAddr(tTunableInfo *p);
static void DSPDrv1451_Writer_Register(cDSPDrv1451* me, uint16 devAddr, uint32 regAddr, uint16 size, const uint8 *pData);
static float DSPDrv1451_DataToFloat(cDSPDrv1451* me, uint32 dsp_float, uint8 fmt_int, uint8 fmt_frac);
static uint32 DSPDrv1451_FloatTo8_24Data(cDSPDrv1451 *me, float fGainDb);



#endif
