/*
-------------------------------------------------------------------------------
TYMPHANY LTD
                  I2C Slave Driver
                  -------------------------
                  SW Module Document
@file        I2cSlaveDrv.c
@brief       This file implements a i2c slave driver.
@author      Viking Wang
@date        2016-11-22
@copyright (c) Tymphany Ltd. All rights reserved.
-------------------------------------------------------------------------------
*/

#include "product.config"
#include "stm32f0xx.h"

#include "commonTypes.h"
#include "trace.h"
#include "assert.h"
#include "controller.h"
#include "SettingSrv.h"
#include "attacheddevices.h"

#include "I2cSlaveDrv.h"

#ifdef SYSCLK_RUN_AT_32M
#define I2C_SLAVE_TIMING_100KHz     0x00B00000
#define I2C_SLAVE_TIMING_400KHz     0x00600000
#else
// this is for 8MHz paramter
#define I2C_SLAVE_TIMING_100KHz     0x00200000
#define I2C_SLAVE_TIMING_400KHz     0x00100000
#endif

static bool i2c_slave_ready=FALSE;
static bool is_i2c_read=FALSE;
static uint16_t i2c_rx_count=0;
static uint16_t i2c_tx_count=0;
static uint8_t *p_i2c_tx_buf=NULL;
static uint8_t *p_i2c_rx_buf=NULL;
static I2cA2bMsg_t i2c_a2b_msg[I2C_READ_CMD_END];
static int32_t i2c_slave_cmd=NULL;

static void I2cSlave_LowLevelInit(tI2CSlaveDevice *p_dev)
{
    IRQn_Type irq_num;
    I2C_InitTypeDef  I2C_InitStructure;
    I2C_TypeDef* I2Cx;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    if (p_dev->channel == I2C_CHANNEL_ONE)
    {
        I2Cx = I2C1;
        irq_num = I2C1_IRQn;
        I2C_Cmd(I2Cx, DISABLE); // disable PE
        I2C1_Slave_LowLevel_Init();
    }
    else if (p_dev->channel == I2C_CHANNEL_TWO)
    {
        I2Cx = I2C2;
        irq_num = I2C2_IRQn;
        I2C_Cmd(I2Cx, DISABLE); // disable PE
        I2C2_Slave_LowLevel_Init();
    }
    else
    {
        ASSERT(0);
    }
    
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
    I2C_InitStructure.I2C_DigitalFilter = 0x00;
    I2C_InitStructure.I2C_OwnAddress1 = p_dev->dev_addr;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Disable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    if( p_dev->speed == I2C_SPEED_100K )
    {
        I2C_InitStructure.I2C_Timing = I2C_SLAVE_TIMING_100KHz;
    }
    else
    {
        I2C_InitStructure.I2C_Timing = I2C_SLAVE_TIMING_400KHz;
    }
    /* Apply sEE_I2C configuration after enabling it */
    I2C_Init(I2Cx, &I2C_InitStructure);

    // enable the i2c interrupt
    I2C_ITConfig(I2Cx, (I2C_CR1_ADDRIE | I2C_CR1_STOPIE | I2C_CR1_TXIE | I2C_CR1_RXIE | I2C_CR1_NACKIE), ENABLE);
    /* sEE_I2C Peripheral Enable */
    I2C_Cmd(I2Cx, ENABLE);

    // enable vector table IRQ
    NVIC_InitStructure.NVIC_IRQChannel          = irq_num;
    NVIC_InitStructure.NVIC_IRQChannelPriority  = 0x00;
    NVIC_InitStructure.NVIC_IRQChannelCmd       = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
       
}

static void I2cSlaveDrv_TxAddrInit()
{
    i2c_a2b_msg[I2C_READ_CMD_ITEM_NO].p_value = (void *)Setting_GetAddr(SETID_ITEM_NO);
    i2c_a2b_msg[I2C_READ_CMD_TYPE_NO].p_value = (void *)Setting_GetAddr(SETID_TYPE_NO);
    i2c_a2b_msg[I2C_READ_CMD_SERIAL_NO].p_value = (void *)Setting_GetAddr(SETID_SERIAL_NO);
    i2c_a2b_msg[I2C_READ_CMD_HW_VER].p_value = (void *)Setting_GetAddr(SETID_HW_VER);
    i2c_a2b_msg[I2C_READ_CMD_BTL_VER].p_value = (void *)Setting_GetAddr(SETID_BTL_VER);
    i2c_a2b_msg[I2C_READ_CMD_APP_VER].p_value = (void *)Setting_GetAddr(SETID_APP_VER);
    i2c_a2b_msg[I2C_READ_CMD_DSP_VER].p_value = (void *)Setting_GetAddr(SETID_DSP_VER);
    i2c_a2b_msg[I2C_READ_CMD_NTC].p_value = (void *)Setting_GetAddr(SETID_NTC_INFO);

    // fix me later.
    i2c_a2b_msg[I2C_READ_CMD_DSP_DELAY].p_value = NULL;   
    i2c_a2b_msg[I2C_READ_CMD_DSP_GAIN].p_value = NULL;   
    i2c_a2b_msg[I2C_READ_CMD_DSP_ROOMEQ].p_value = NULL;   
}

static void I2cSlave_TxCallBack()
{
    BRINGUP_printf("\n\rI2C tx len = %d.", i2c_tx_count);
}

static void I2cSlave_RxCallBack()
{
    I2cSlaveRxEvt* pI2cEvt = Q_NEW(I2cSlaveRxEvt, I2C_SLAVE_REQ_SIG);
    pI2cEvt->cmd = i2c_slave_cmd;
    pI2cEvt->p_data = (uint32_t *)p_i2c_rx_buf;
    SendToServer(MAIN_APP_ID,(QEvt*)pI2cEvt);
}

void I2cSlaveDrv_Ctor(void)
{
    tI2CSlaveDevice *p_dev;

    p_dev = (tI2CSlaveDevice *)getDevicebyIdAndType(COMM_DEV_ID, I2C_SLAVE_DEV_TYPE, NULL);
    ASSERT(p_dev);

    I2cSlave_LowLevelInit(p_dev);
}

void I2cSlaveDrv_Xtor(void)
{
}

void I2cSlaveDrv_Init(void)
{
    i2c_slave_ready = TRUE;
    is_i2c_read = FALSE;
    i2c_tx_count = 0;
    p_i2c_tx_buf = NULL;
    i2c_rx_count = 0;
    p_i2c_rx_buf = (uint8_t *)UsbSrv_GetGpbDecodeBuffer();
    I2cSlaveDrv_TxAddrInit();
}

void I2cSlaveDrv_Deinit(void)
{
    i2c_slave_ready = FALSE;
}

void I2cSlaveDrv_Service(void)
{
}

void I2C1_IRQHandler(void)
{
    uint32_t isr_status;

    isr_status = (uint32_t)(I2C1->ISR);

    // check interrupt
    if( isr_status & I2C_ISR_ADDR )
    {   // address matched, check read/write
        if( isr_status & I2C_ISR_DIR )  // tx mode
        {   // master read command
            I2C1->ISR |= I2C_ISR_TXE;
            i2c_tx_count = 0;   
            is_i2c_read = TRUE;
            if( i2c_slave_cmd < I2C_READ_CMD_END )
            {
                p_i2c_tx_buf = (uint8_t *)(i2c_a2b_msg[i2c_slave_cmd].p_value);
            }
            else
            {   // unknown command, something error
                ASSERT(0);  
            }
        }
        else    // rx mode
        {   // master write command
            i2c_rx_count = 0;
            is_i2c_read = FALSE;
            i2c_slave_cmd = NULL;
        }
        I2C1->ICR |= I2C_ICR_ADDRCF;    // clear address match flag
    }
    else if( isr_status & I2C_ISR_RXNE )
    {
        if( i2c_slave_cmd == NULL )
        {
            i2c_slave_cmd = (int32_t)(I2C1->RXDR);
        }
        else
        {
            p_i2c_rx_buf[i2c_rx_count++] = (uint8_t)(I2C1->RXDR);
        }
    }
    else if( isr_status & I2C_ISR_TXIS )
    {
//        ASSERT(is_i2c_read);
        if( i2c_slave_ready )
            I2C1->TXDR = (uint8_t)(p_i2c_tx_buf[i2c_tx_count]);
        else
            I2C1->TXDR = 0;            
        i2c_tx_count ++;
    }
    else if( isr_status & I2C_ISR_STOPF )
    {   // get i2c_stop command from master, transform finished
		I2C1->ICR |= I2C_ICR_STOPCF;
        // call back
        if( is_i2c_read )
        {
            I2cSlave_TxCallBack();
        }
        else
        {
            I2cSlave_RxCallBack();
        }
    }
    else if( isr_status & I2C_ISR_NACKF )
    {
        I2C1->ICR |= I2C_ICR_NACKCF;
    }
    else
        ;
}


