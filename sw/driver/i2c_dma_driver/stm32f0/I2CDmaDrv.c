/**
*  @file      I2CDmaDrv.c
*  @brief     This file contains functions for I2C master read or write in DMA mode
*  @waring    DO NOT USE I2CDrv.c AND THIS FILE TOGETHER
*  @waring    Message memory should not be freed until DMA complete callback.Suggest free it in callback function
*  @waring    This Driver need to use Software Timer.Timer number should +2 if 2 I2C channel is used
*  @version   v0.2
*  @author    Gavin Lee , Alexey Abdulov , Alex.Li
*  @date      2017/7/13
*  @copyright Tymphany Ltd.
*/

/*WARNING:
* DO NOT USE I2CDrv.c AND THIS FILE TOGETHER
* Message memory should not be freed until transmit completed
* This Driver need to use Software Timer.Timer number should +2 if 2 I2C channel is used
* If I2C2 is needed, Set I2C_NUM to 2
*/

#include "I2CDmaDrv.h"
#include "attachedDevices.h"
#include "MsgRingBuf.h"
#include "./I2CDmaDrv_priv.h"
#include "stm32f0xx_i2c.h"
#include "trace.h"
#include "commonTypes.h"
#include "stm32f0xx_rcc.h"
#include "timer.h"
#include "bl_common.h"


#define SCL_400     (400)
#define PRESC_400   (0x00 << 28)   // timing prescalr, referring to datasheet RM0360 in page 584
#define SCLDEL_400  (0x01 << 20)   // data setup time.
#define SDADEL_400  (0x00 << 16)   // Data hold time.
#define SCLH_400    (0x03 << 8)    //SCL high period
#define SCLL_400    (0x08)         //SCL low period

#define SCL_350     (350)          //practical test for near 348.2khz
#define PRESC_350   (0x00 << 28)   // timing prescalr,referring to datasheet RM0360 in page 584
#define SCLDEL_350  (0x01 << 20)   // data setup time.Practical test for near 1.12us
#define SDADEL_350  (0x00 << 16)   // Data hold time.Practical test for near 500ns
#define SCLH_350    (0x05 << 8)    //SCL high period.Practical test for near 1.1us
#define SCLL_350    (0x09)         //SCL low period.Practical test for near 1.6us

#define SCL_100     (100)
#define PRESC_100   (0x01 << 28)   // timing prescalr, referring to datasheet RM0360 in page 584
#define SCLDEL_100  (0x04 << 20)   // data setup time.
#define SDADEL_100  (0x02 << 16)   // Data hold time.
#define SCLH_100    (0x0f << 8)    //SCL high period
#define SCLL_100    (0x13)         //SCL low period

const static tI2CSpeedMap i2cSpeedMap[] =
{
    {SCL_400, PRESC_400 | SCLDEL_400 | SDADEL_400 | SCLH_400 | SCLL_400}, //400KHz for 8MHz CPU frequency
    {SCL_350, PRESC_350 | SCLDEL_350 | SDADEL_350 | SCLH_350 | SCLL_350}, //350KHz for 8MHz CPU frequency
    {SCL_100, PRESC_100 | SCLDEL_100 | SDADEL_100 | SCLH_100 | SCLL_100}  //100KHz for 8MHz CPU frequency
};

static bool bIsChReady[I2C_NUM] = {0};
volatile bool bIsChBusy[I2C_NUM] = {0};
static uint32 chBaudRate[I2C_NUM] = {0};
static uint16 chUserCount[I2C_NUM] = {0};
#ifndef NDEBUG
volatile int16 waitForCallbackNum[I2C_NUM] = {0};//this num should equal to i2cRingBuf used size
#endif

static cMsgRingBuf i2cRingBuf[I2C_NUM];
static void* i2cBuff[I2C_NUM][I2C_BUF_SIZE];
static tI2CDmaMsg* i2cTopMsg[I2C_NUM];
static tI2CDmaMsg addrMsg[I2C_NUM];//this msg use to decode addr apart
static uint8 tempaddr[I2C_NUM][3];//this msg use to decode addr apart
static uint8 timeoutNum[I2C_NUM] = {0, 1};

static uint16 i2cTimeoutTimerId[I2C_NUM] = {0, 0};
static bool bIsTimerRunning[I2C_NUM] = {0, 0};

/**
* Construct the i2c_dma driver instance with ringbuf
* @param me - instance of the driver
* @param pConfig - pointer to the config structure
* @param successCallBack - successcallback function pointer
* @param errorCallBack - errorcallback function pointer
*/
void I2CDmaDrv_Ctor(cI2CDmaDrv * me, tI2CDmaDevice * pConfig, pdI2CDma_CALLBACK callback)
{
    ASSERT(pConfig);
    ASSERT(me && pConfig->deviceInfo.deviceType == I2C_DEV_TYPE);

    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
    * chXUserCount is never 0, and never xtor I2C
    */
    ASSERT(!me->isReady);

    me->pConfig = pConfig;
    me->pvI2CDmaCallback = callback;

    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx = I2C1;

    i2c_channel = I2CDmaDrv_SwitchChannel(me, I2Cx);

    ASSERT(I2C_NUM > i2c_channel);
    ASSERT(chUserCount[i2c_channel] >= 0);

    if(chUserCount[i2c_channel] == 0)  //if nobody used it before, we need to ctor ringbuf
    {
        MsgRingBuf_Ctor(&i2cRingBuf[i2c_channel], i2cBuff[i2c_channel], I2C_BUF_SIZE);
    }

    chUserCount[i2c_channel]++;

    if(!bIsChReady[i2c_channel]) //not init yet
    {
        I2CDmaDrv_LowLevelInit(me);
        chBaudRate[i2c_channel] = me->pConfig->baudRate;
        bIsChReady[i2c_channel] = TRUE;
    }

    /* All configuration on a I2C bus should have the same baut rate.
    * But to avoid wrong setting, we double check here.
    */
    ASSERT(chBaudRate[i2c_channel] == me->pConfig->baudRate);

    me->isReady = TRUE;
}

/**
* Exit & clean up the driver.
* @param me - instance of the driver
*/
void I2CDmaDrv_Xtor(cI2CDmaDrv * me)
{
    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
    * chXUserCount is never 0, and never xtor I2C
    */
    me->pvI2CDmaCallback = NULL;

    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx = I2C1;

    i2c_channel = I2CDmaDrv_SwitchChannel(me, I2Cx);

    if(chUserCount[i2c_channel] == 1)  //if only one used it, we need to xtor ringbuf
    {
        MsgRingBuf_Xtor(&i2cRingBuf[i2c_channel]);

        I2C_DeInit(I2Cx);
        I2C_Cmd(I2Cx, DISABLE);
        I2C_ITConfig(I2Cx, I2C_IT_NACKI | I2C_IT_ERRI, DISABLE);
        I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);

        if(I2Cx == I2C1)
        {
            DMA_Cmd(DMA1_Channel2, DISABLE);
            DMA_Cmd(DMA1_Channel3, DISABLE);
        }

        else if(I2Cx == I2C2)
        {
            DMA_Cmd(DMA2_Channel1, DISABLE);
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }

        bIsChReady[i2c_channel] = false;
        bIsChBusy[i2c_channel] = false;
        chBaudRate[i2c_channel] = 0;
        me->isReady = FALSE;
    }

    ASSERT(chUserCount[i2c_channel] > 0);
    chUserCount[i2c_channel]--;
}

/**
* Write a message through DMA in master mode,message memory should not be freed until transmit completed
* @param me - instance of the driver
* @param msg - pointer to the message
*/
eTpRet I2CDmaDrv_MasterWrite(cI2CDmaDrv * me, tI2CDmaMsg const * const  msg)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx;
    uint32_t ReloadEndMode = I2C_AutoEnd_Mode;


    //check if MsgRingBuf full or get wrong CHANNEL
    ASSERT(msg->regAddr == NULL); //MasterWrite should not have regaddr

    if(me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        i2c_channel = 0;
        I2Cx = I2C1;
    }

    else if(me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        i2c_channel = 1;
        I2Cx = I2C2;
    }

    else
    {
        me->pvI2CDmaCallback((void *)msg, I2cCbRet_Err);
        return TP_FAIL;
    }

    if(MsgRingBuf_GetFreeSize(&i2cRingBuf[i2c_channel]) <= 0)//check if buffer full
    {
        me->pvI2CDmaCallback((void *)msg, I2cCbRet_BuffFullError);
        return TP_FAIL;
    }

    __disable_irq();
    MsgRingBuf_PushData(&i2cRingBuf[i2c_channel], (void *)msg);
#ifndef NDEBUG
    waitForCallbackNum[i2c_channel]++;
#endif
    __enable_irq();

    if(!bIsChBusy[i2c_channel])
    {
        bIsChBusy[i2c_channel] = true;
        I2CDmaDrv_StartDmaWrite(I2Cx, ReloadEndMode);
    }

    return TP_SUCCESS;
}

/**
* Read a message through DMA in master mode,message memory should not be freed until transmit completed
* @param me - instance of the driver
* @param msg - pointer to the message
*/
eTpRet I2CDmaDrv_MasterRead(cI2CDmaDrv * const me, tI2CDmaMsg *  msg)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx;
    uint32_t ReloadEndMode = I2C_SoftEnd_Mode;

    //check if MsgRingBuf full or get wrong CHANNEL
    if(me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        i2c_channel = 0;
        I2Cx = I2C1;
    }

    else if(me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        i2c_channel = 1;
        I2Cx = I2C2;
    }

    else
    {
        me->pvI2CDmaCallback((void *)msg, I2cCbRet_Err);
        return TP_FAIL;
    }

    if(MsgRingBuf_GetFreeSize(&i2cRingBuf[i2c_channel]) <= 0)//check if buffer full
    {
        me->pvI2CDmaCallback((void *)msg, I2cCbRet_BuffFullError);
        return TP_FAIL;
    }

    msg->regAddrLen = me->pConfig->regAddrLen;
    __disable_irq();
    MsgRingBuf_PushData(&i2cRingBuf[i2c_channel], (void *)msg);
#ifndef NDEBUG
    waitForCallbackNum[i2c_channel]++;
#endif
    __enable_irq();

    if(!bIsChBusy[i2c_channel])
    {
        bIsChBusy[i2c_channel] = true;
        I2CDmaDrv_StartDmaWrite(I2Cx, ReloadEndMode);
    }

    return TP_SUCCESS;
}

/**
* Put it in callback function to check if waitForCallbackNum is equal to RingBuf used size.
* @param me - instance of the driver
* @param msg - pointer to the message
* @retval - The value of RingBuf used size
*/
int16 I2CDmaDrv_CheckRingBuf(cI2CDmaDrv * const me)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx = I2C1;
    int16 res;

    i2c_channel = I2CDmaDrv_SwitchChannel(me, I2Cx);

    res = MsgRingBuf_GetUsedSize(&i2cRingBuf[i2c_channel]);
#ifndef NDEBUG
    waitForCallbackNum[i2c_channel]--;
#endif

    ASSERT(waitForCallbackNum[i2c_channel] == res);

    return res;
}


/* PRIVATE FUNCTIONS */
static void I2CDmaDrv_LowLevelInit(cI2CDmaDrv * me)
{
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_TypeDef* I2Cx;
    NVIC_InitTypeDef NVIC_InitStructure;

    /* GPIO initialization */
    if(me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
        I2C1_LowLevel_Init();
        I2Cx = I2C1;
        NVIC_InitStructure.NVIC_IRQChannel = I2C1_IRQn;
    }

    else if(me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
        I2C2_LowLevel_Init();
        I2Cx = I2C2;
        NVIC_InitStructure.NVIC_IRQChannel = I2C2_IRQn;
    }

    /* I2C configuration */
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStructure.I2C_DigitalFilter = 0x00;
    I2C_InitStructure.I2C_OwnAddress1 = 0x00;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    uint8 i = 0;

    for(i = 0; i < ArraySize(i2cSpeedMap); i++)
    {
        if(me->pConfig->baudRate == i2cSpeedMap[i].speedInKHz)
        {
            I2C_InitStructure.I2C_Timing = i2cSpeedMap[i].timing;
            break;
        }
    }

    ASSERT(i < ArraySize(i2cSpeedMap));
    I2C_Init(I2Cx, &I2C_InitStructure);

    I2C_ITConfig(I2Cx, I2C_IT_NACKI | I2C_IT_ERRI, ENABLE);
    //I2C1->CR1|=0x7F<<1;

    NVIC_InitStructure.NVIC_IRQChannelPriority  = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd       = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    I2C_Cmd(I2Cx, ENABLE);
}

/**
* Start DMA write transmit
* @param I2Cx - where x can be 1 or 2 to select the I2C peripheral
* @param  StartStopMode - new state of the I2C START condition generation.
*/
static eTpRet I2CDmaDrv_StartDmaWrite(I2C_TypeDef* I2Cx, uint32_t ReloadEndMode)
{
    //DMA1 CH2->I2C1 TX
    //DMA2 CH1->I2C2 TX
    uint8 i2c_channel = 0;
    DMA_InitTypeDef  DMA_InitStructure;
    DMA_Channel_TypeDef * DMAx_Channely;

    if(I2Cx == I2C1)
    {
        i2c_channel = 0;
        DMAx_Channely = DMA1_Channel2;
    }

    else if(I2Cx == I2C2)
    {
        i2c_channel = 1;
        DMAx_Channely = DMA2_Channel1;
    }

    else
    {
        ASSERT(0);
    }

    __disable_irq();
    i2cTopMsg[i2c_channel] = (tI2CDmaMsg*)MsgRingBuf_PopData(&i2cRingBuf[i2c_channel]);
    __enable_irq();

    if(!bIsTimerRunning[i2c_channel])
    {
        __disable_irq();
        Timer_StartTimer(I2C_TIMEOUT_MS, &i2cTimeoutTimerId[i2c_channel], timeoutCallback, &timeoutNum[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = true;
    }

    else
    {
        ASSERT(0);
    }

    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2cTopMsg[i2c_channel]->pMsg;
    DMA_InitStructure.DMA_BufferSize = i2cTopMsg[i2c_channel]->length;

    if(i2cTopMsg[i2c_channel]->regAddr != NULL)//means need to read
    {
        addrMsg[i2c_channel] = *i2cTopMsg[i2c_channel];

        switch(addrMsg[i2c_channel].regAddrLen)
        {
        case REG_LEN_8BITS:
            //Send device address with W bit
            tempaddr[i2c_channel][0] = (uint8)addrMsg[i2c_channel].regAddr;
            addrMsg[i2c_channel].pMsg = tempaddr[i2c_channel];
            addrMsg[i2c_channel].length = 1;
            break;

        case REG_LEN_16BITS:
            //Send device address with W bit
            tempaddr[i2c_channel][0] = (uint8)((addrMsg[i2c_channel].regAddr & 0xff00) >> 8);
            tempaddr[i2c_channel][1] = (uint8)(addrMsg[i2c_channel].regAddr & 0x00ff);
            addrMsg[i2c_channel].pMsg = tempaddr[i2c_channel];
            addrMsg[i2c_channel].length = 2;
            break;

        case REG_LEN_24BITS:
            //Send device address with W bit
            tempaddr[i2c_channel][0] = (uint8)((addrMsg[i2c_channel].regAddr & 0xff0000) >> 16);
            tempaddr[i2c_channel][1] = (uint8)((addrMsg[i2c_channel].regAddr & 0xff00) >> 8);
            tempaddr[i2c_channel][2] = (uint8)(addrMsg[i2c_channel].regAddr & 0x00ff);
            addrMsg[i2c_channel].pMsg = tempaddr[i2c_channel];
            addrMsg[i2c_channel].length = 3;
            break;

        default:
            ASSERT(0); //wrong register length setting
            break;
        }

        DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)addrMsg[i2c_channel].pMsg;
        DMA_InitStructure.DMA_BufferSize = addrMsg[i2c_channel].length;
        ReloadEndMode = I2C_SoftEnd_Mode;
    }

    DMA_DeInit(DMAx_Channely);//DeInit first
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2Cx->TXDR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//Memory to Peripheral
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMAx_Channely, &DMA_InitStructure);

    if(i2c_channel == 0)
    {
        DMA_RemapConfig(DMA1, DMA1_CH2_I2C1_TX);
        DMA_RemapConfig(DMA1, DMA1_CH3_I2C1_RX);
    }

    if(i2c_channel == 1)
    {
        DMA_RemapConfig(DMA2, DMA2_CH1_I2C2_TX);
        DMA_RemapConfig(DMA2, DMA2_CH2_I2C2_RX);
    }

    DMA_ITConfig(DMAx_Channely, DMA_IT_TC | DMA_IT_TE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Ch2_3_DMA2_Ch1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(DMAx_Channely, ENABLE);
    I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, ENABLE);

    if(i2cTopMsg[i2c_channel]->regAddr != NULL)
    {
        I2C_TransferHandling(I2Cx, addrMsg[i2c_channel].devAddr, addrMsg[i2c_channel].length,
                             ReloadEndMode, I2C_Generate_Start_Write);
    }

    else
    {
        I2C_TransferHandling(I2Cx, i2cTopMsg[i2c_channel]->devAddr, i2cTopMsg[i2c_channel]->length,
                             ReloadEndMode, I2C_Generate_Start_Write);
    }

    return TP_SUCCESS;
}

/**
* Start DMA read transmit
* @param I2Cx - where x can be 1 or 2 to select the I2C peripheral
* @param  StartStopMode - new state of the I2C START condition generation.
*/
static eTpRet I2CDmaDrv_StartDmaRead(I2C_TypeDef* I2Cx, uint32_t ReloadEndMode)
{
    //DMA1 CH3->I2C1 RX
    //DMA2 CH2->I2C2 RX
    uint8 i2c_channel = 0;
    DMA_InitTypeDef  DMA_InitStructure;
    DMA_Channel_TypeDef * DMAx_Channely;

    if(I2Cx == I2C1)
    {
        i2c_channel = 0;
        DMAx_Channely = DMA1_Channel3;
    }

    else if(I2Cx == I2C2)
    {
        i2c_channel = 1;
        DMAx_Channely = DMA2_Channel2;
    }

    else
    {
        ASSERT(0);
    }

    if(!bIsTimerRunning[i2c_channel])
    {
        __disable_irq();
        Timer_StartTimer(I2C_TIMEOUT_MS, &i2cTimeoutTimerId[i2c_channel], timeoutCallback, &timeoutNum[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = true;
    }

    else
    {
        ASSERT(0);
    }

    //msg is already pop to i2cTopMsg when send regaddr
    DMA_DeInit(DMAx_Channely);//DeInit first
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)i2cTopMsg[i2c_channel]->pMsg;
    DMA_InitStructure.DMA_BufferSize = i2cTopMsg[i2c_channel]->length;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&I2Cx->RXDR;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//Peripheral to Memory
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    DMA_Init(DMAx_Channely, &DMA_InitStructure);

    if(I2Cx == I2C2)
    {
        DMA_RemapConfig(DMA2, DMA2_CH2_I2C2_RX);
    }

    DMA_ITConfig(DMAx_Channely, DMA_IT_TC | DMA_IT_TE, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Ch2_3_DMA2_Ch1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_Cmd(DMAx_Channely, ENABLE);
    I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, ENABLE);

    I2C_TransferHandling(I2Cx, i2cTopMsg[i2c_channel]->devAddr, i2cTopMsg[i2c_channel]->length,
                         ReloadEndMode, I2C_Generate_Start_Read);
    return TP_SUCCESS;
}

static uint8 I2CDmaDrv_SwitchChannel(cI2CDmaDrv * const me, I2C_TypeDef* I2Cx)
{
    uint8 i2c_channel = 0;
    I2Cx = I2C1;
    (void)I2Cx;

    if(me->pConfig->channel == I2C_CHANNEL_ONE)
    {
        i2c_channel = 0;
        I2Cx = I2C1;
    }

    else if(me->pConfig->channel == I2C_CHANNEL_TWO)
    {
        i2c_channel = 1;
        I2Cx = I2C2;
    }

    else
    {
        ASSERT(0);
    }

    return i2c_channel;
}

static void timeoutCallback(void *pCbPara)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx;
    i2c_channel = *(uint8*)pCbPara;

    ASSERT(bIsTimerRunning[i2c_channel]);

    if(bIsTimerRunning[i2c_channel])
    {
        bIsTimerRunning[i2c_channel] = false;

        if(i2c_channel == 0)//I2C1 TIMEOUT
        {
            I2Cx = I2C1;
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA1_Channel2, DISABLE);
            DMA_Cmd(DMA1_Channel3, DISABLE);

        }

        else if(i2c_channel == 1)//I2C2 TIMEOUT
        {
            I2Cx = I2C2;
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA2_Channel1, DISABLE);
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }

        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_Timeout);
#ifdef I2C_ERROR_RECOVER
        I2C_Cmd(I2Cx, DISABLE);
        I2C_Cmd(I2Cx, ENABLE);
#endif

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }
}

/*============================================================================*/
/* IRQ functions */
/*============================================================================*/
void DMA1_Ch2_3_DMA2_Ch1_2_IRQHandler(void)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx;

    if(DMA_GetFlagStatus(DMA1_FLAG_TC2) == SET || DMA_GetFlagStatus(DMA2_FLAG_TC1) == SET)
    {
        //I2C TX DMA Completed
        if(DMA_GetFlagStatus(DMA1_FLAG_TC2) == SET)//I2C1
        {
            i2c_channel = 0;
            I2Cx = I2C1;
            DMA_ClearFlag(DMA1_FLAG_TC2);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx, DISABLE);
            DMA_Cmd(DMA1_Channel2, DISABLE);
        }

        else if(DMA_GetFlagStatus(DMA2_FLAG_TC1) == SET)//I2C2
        {
            i2c_channel = 1;
            I2Cx = I2C2;
            DMA_ClearFlag(DMA2_FLAG_TC1);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx, DISABLE);
            DMA_Cmd(DMA2_Channel1, DISABLE);
        }

        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;

        if(i2cTopMsg[i2c_channel]->regAddr != NULL)//read msg
        {
            //means this msg is sending addr don't callback and need to read
            I2CDmaDrv_StartDmaRead(I2Cx, I2C_AutoEnd_Mode);
        }

        else//means normal msg no need to read
        {
            i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_WriteSuccess);

            if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
            {
                I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
            }

            else
            {
                bIsChBusy[i2c_channel] = 0;
            }
        }
    }

    if(DMA_GetFlagStatus(DMA1_FLAG_TC3) == SET || DMA_GetFlagStatus(DMA2_FLAG_TC2) == SET)
    {
        //I2C RX DMA Completed
        if(DMA_GetFlagStatus(DMA1_FLAG_TC3) == SET)//I2C1
        {
            i2c_channel = 0;
            I2Cx = I2C1;
            DMA_ClearFlag(DMA1_FLAG_TC3);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA1_Channel3, DISABLE);
        }

        else if(DMA_GetFlagStatus(DMA2_FLAG_TC2) == SET)//I2C2
        {
            i2c_channel = 1;
            I2Cx = I2C2;
            DMA_ClearFlag(DMA2_FLAG_TC2);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }

        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_ReadSuccess);

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }

    if(DMA_GetFlagStatus(DMA1_FLAG_TE2) == SET || DMA_GetFlagStatus(DMA2_FLAG_TE1) == SET)
    {
        //I2C TX DMA Error
        if(DMA_GetFlagStatus(DMA1_FLAG_TE2) == SET)
        {
            i2c_channel = 0;
            I2Cx = I2C1;
            DMA_ClearFlag(DMA1_FLAG_TE2);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx, DISABLE);
            DMA_Cmd(DMA1_Channel2, DISABLE);
        }

        else if(DMA_GetFlagStatus(DMA2_FLAG_TE1) == SET)
        {
            i2c_channel = 1;
            I2Cx = I2C2;
            DMA_ClearFlag(DMA2_FLAG_TE1);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Tx, DISABLE);
            DMA_Cmd(DMA2_Channel1, DISABLE);
        }

        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_WriteDmaError);

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }

    if(DMA_GetFlagStatus(DMA1_FLAG_TE3) == SET || DMA_GetFlagStatus(DMA2_FLAG_TE2) == SET)
    {
        //I2C RX DMA Error
        if(DMA_GetFlagStatus(DMA1_FLAG_TE3) == SET)
        {
            i2c_channel = 0;
            I2Cx = I2C1;
            DMA_ClearFlag(DMA1_FLAG_TE3);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA1_Channel2, DISABLE);
        }

        else if(DMA_GetFlagStatus(DMA2_FLAG_TE2) == SET)
        {
            i2c_channel = 1;
            I2Cx = I2C2;
            DMA_ClearFlag(DMA2_FLAG_TE2);
            I2C_DMACmd(I2Cx, I2C_DMAReq_Rx, DISABLE);
            DMA_Cmd(DMA2_Channel2, DISABLE);
        }

        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_ReadDmaError);

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }
}

void I2C1_IRQHandler(void)
{
    uint8 i2c_channel = 0;
    I2C_TypeDef* I2Cx = I2C1;

    if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_NACKF) == SET)//NACK
    {
        I2C_ClearFlag(I2Cx, I2C_FLAG_NACKF);
        I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA1_Channel2, DISABLE);
        DMA_Cmd(DMA1_Channel3, DISABLE);
        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_NACK);

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }

    else //Red Alert Error I2C BUS NEED TO RESET
    {
        I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA1_Channel2, DISABLE);
        DMA_Cmd(DMA1_Channel3, DISABLE);
        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_Err);
#ifdef I2C_ERROR_RECOVER
        I2C_Cmd(I2Cx, DISABLE);
        I2C_Cmd(I2Cx, ENABLE);
#endif

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }
}

void I2C2_IRQHandler(void)
{
    uint8 i2c_channel = 1;
    I2C_TypeDef* I2Cx = I2C2;

    if(I2C_GetFlagStatus(I2Cx, I2C_FLAG_NACKF) == SET)//NACK
    {
        I2C_ClearFlag(I2Cx, I2C_FLAG_NACKF);
        I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA2_Channel1, DISABLE);
        DMA_Cmd(DMA2_Channel2, DISABLE);
        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_NACK);

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }

    else //Red Alert Error I2C BUS NEED TO RESET
    {
        I2C_DMACmd(I2Cx, I2C_DMAReq_Tx | I2C_DMAReq_Rx, DISABLE);
        DMA_Cmd(DMA2_Channel1, DISABLE);
        DMA_Cmd(DMA2_Channel2, DISABLE);
        __disable_irq();
        Timer_StopTimer(i2cTimeoutTimerId[i2c_channel]);
        __enable_irq();
        bIsTimerRunning[i2c_channel] = false;
        i2cTopMsg[i2c_channel]->owner->pvI2CDmaCallback(i2cTopMsg[i2c_channel], I2cCbRet_Err);
#ifdef I2C_ERROR_RECOVER
        I2C_Cmd(I2Cx, DISABLE);
        I2C_Cmd(I2Cx, ENABLE);
#endif

        if(!MsgRingBuf_IsEmpty(&i2cRingBuf[i2c_channel]))
        {
            I2CDmaDrv_StartDmaWrite(I2Cx, I2C_AutoEnd_Mode);
        }

        else
        {
            bIsChBusy[i2c_channel] = 0;
        }
    }
}
