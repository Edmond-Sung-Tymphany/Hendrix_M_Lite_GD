/**
 *  @file      SWi2c_Drv.c
 *  @brief     This file contains the software i2c driver implementation.
 *  @author    Viking Wang
 *  @date      03-May-2016
 *  @copyright Tymphany Ltd.
 */


#include "stm32f0xx.h"

#ifdef HAS_SWi2c_DEVICE

#include "commonTypes.h"
#include "SWi2c_Drv.h"
#include "trace.h"
#include "assert.h"
#include "attachedDevices.h"

// how long to wait the ACK from the device?
#define SWI2C_ACK_TIMEOUT_MS   5

extern uint32_t getSysTime(void);

#ifdef EXTERNAL_HIGH_SPEED_CLOCK
// when cpu clock = 32MHz
void SWi2c_NormalDelay()
{
    uint32_t delay = 12;
    while(delay--)
    {
        asm("nop");asm("nop");
        asm("nop");asm("nop");
    }
}
#else
#define SWi2c_NormalDelay()     do{asm("nop");asm("nop");asm("nop");asm("nop");}while(0)
#endif

/* config the software i2c port1 GPIO */
__weak void SWi2c1_LowLevel_init(void)
{
}

/* config the software i2c port2 GPIO */
__weak void SWi2c2_LowLevel_init(void)
{
}

/* config the software i2c port3 GPIO */
__weak void SWi2c3_LowLevel_init(void)
{
}

static void prvSWi2cDelayNOP(uint32_t delay)
{
    while( delay --)
    {
        asm("nop");
        asm("nop");
    }
}

static void SWi2cDrv_LowLevelInit(cSWi2cDrv_t * me)
{
    if( me->pConfig->busID == eSWi2cBus_ID1 )
    {
        SWi2c1_LowLevel_init();
    }
    else if( me->pConfig->busID == eSWi2cBus_ID2 )
    {
        SWi2c2_LowLevel_init();
    }
    else if( me->pConfig->busID == eSWi2cBus_ID3 )
    {
        SWi2c3_LowLevel_init();
    }
    else
    {	// config error
        ASSERT(0);
    }
}

/**
 * Construct the s/w i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void SWi2cDrv_Ctor(cSWi2cDrv_t *me, stSWi2cDevice_t *pConfig)
{
    ASSERT(me && (pConfig->deviceInfo.deviceType==SWI2C_DEV_TYPE));

    /* Duplicate ctor is invalid. If duplicate ctor but no dulipcate xtor,
    */
#ifndef BRINGUP_DEBUG
    ASSERT(!me->isReady);
#endif

    me->isBusy = FALSE;
    me->devReady = FALSE;
    me->pConfig = pConfig;
    SWi2cDrv_LowLevelInit(me);

    me->isReady = TRUE;
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void SWi2cDrv_Xtor(cSWi2cDrv_t * me)
{
    ASSERT(me);

    /* Duplicate xtor is invalid. If duplicate xtor but no dulipcate ctor,
    */
    ASSERT(me->isReady);

    if( me->pConfig->busID == eSWi2cBus_ID1 )
    {
    }
    else if( me->pConfig->busID == eSWi2cBus_ID2 )
    {
    }
    else
    {	// config error
        ASSERT(0);
    }

    me->isReady = FALSE;	
}


static void prvSWi2c_Start(stSWi2cDevice_t *pstDevice)
{
    pstDevice->vSWi2cSDA_Set(1);
    pstDevice->vSWi2cSDA_DirSetup(1);
    SWi2c_NormalDelay();
    pstDevice->vSWi2cSCL_Set(1);
    SWi2c_NormalDelay();
    pstDevice->vSWi2cSDA_Set(0);
    SWi2c_NormalDelay();

    // hold the i2c bus
    pstDevice->vSWi2cSCL_Set(0);
    SWi2c_NormalDelay();
}

static void prvSWi2c_Stop(stSWi2cDevice_t *pstDevice)
{
    pstDevice->vSWi2cSDA_Set(0);
    pstDevice->vSWi2cSDA_DirSetup(1);
    SWi2c_NormalDelay();
    pstDevice->vSWi2cSCL_Set(1);
    SWi2c_NormalDelay();
    pstDevice->vSWi2cSDA_Set(1);
    SWi2c_NormalDelay();
}

static void prvSWi2c_GiveAck(stSWi2cDevice_t *pstDevice)
{
    pstDevice->vSWi2cSDA_Set(0);
    pstDevice->vSWi2cSDA_DirSetup(1);
    SWi2c_NormalDelay();

    pstDevice->vSWi2cSCL_Set(1);
    pstDevice->vSWi2cSCL_Delay();
    pstDevice->vSWi2cSCL_Set(0);
    pstDevice->vSWi2cSCL_Delay();
}

static int prvSWi2c_GetAck(stSWi2cDevice_t *pstDevice)
{
    uint32_t tickEnd;
	int ack_ok;

    pstDevice->vSWi2cSDA_Set(1);
    pstDevice->vSWi2cSDA_DirSetup(0);
    SWi2c_NormalDelay();
    pstDevice->vSWi2cSCL_Set(1);
    pstDevice->vSWi2cSCL_Delay();
    
    // timeout checking
    tickEnd = getSysTime() + SWI2C_ACK_TIMEOUT_MS;
    ack_ok = 0;
    do
    {
        if( ! pstDevice->iSWi2cSDA_Sense() )
        {
            ack_ok = 1;
            break;
        }
        SWi2c_NormalDelay();
    }while( getSysTime() < tickEnd );

    if( ack_ok )
    {
        pstDevice->vSWi2cSCL_Set(0);
        pstDevice->vSWi2cSCL_Delay();
    }
    else
    {
        prvSWi2c_Stop(pstDevice);
    }
    
	return ack_ok;
}

static void prvSWi2c_SendByte(stSWi2cDevice_t *pstDevice, uint8_t tx_byte)
{
	int i;

    pstDevice->vSWi2cSDA_DirSetup(1);

	for(i=0; i<8; i++)
    {
//        pstDevice->vSWi2cSCL_Delay();
		if( tx_byte & 0x80 )
            pstDevice->vSWi2cSDA_Set(1);
		else
            pstDevice->vSWi2cSDA_Set(0);
        pstDevice->vSWi2cSCL_Delay();
        pstDevice->vSWi2cSCL_Set(1);
        pstDevice->vSWi2cSCL_Delay();
        pstDevice->vSWi2cSCL_Set(0);
        tx_byte <<= 1;
    }
}

static uint8_t prvSWi2c_ReadByte(stSWi2cDevice_t *pstDevice)
{
	int i;
    uint8_t rx_byte;

    pstDevice->vSWi2cSDA_DirSetup(0);

	rx_byte = 0;

	for(i=0; i<8; i++)
    {
		rx_byte <<= 1;
        pstDevice->vSWi2cSCL_Set(1);
        pstDevice->vSWi2cSCL_Delay();
        if( pstDevice->iSWi2cSDA_Sense() )
			rx_byte |= 0x01;
        pstDevice->vSWi2cSCL_Set(0);
        pstDevice->vSWi2cSCL_Delay();
        pstDevice->vSWi2cSCL_Delay();
    }

	return rx_byte;
}

/**
 * This function detect whether the i2c device is available or not?
 * @param me - instance of the driver
 * @param retry_cnt - how many times to try the i2c comminucation?
 * @return TRUE on success, otherwise FALSE
 */
bool SWi2cDrv_DeviceAvailable(cSWi2cDrv_t * me, uint32_t retry_cnt)
{
    bool deviceAvailable=FALSE;
    stSWi2cDevice_t *pstDevice;

    pstDevice = me->pConfig;
    while( retry_cnt -- )
    {
        pstDevice->vSWi2cSDA_DirSetup(1);
        pstDevice->vSWi2cSDA_Set(1);
        prvSWi2cDelayNOP(10);

        prvSWi2c_Start(pstDevice);
        prvSWi2c_SendByte(pstDevice, pstDevice->devAddress);
        deviceAvailable = (bool)prvSWi2c_GetAck(pstDevice);
        prvSWi2c_Stop(pstDevice);
        if( deviceAvailable )
            break;
    }

    me->devReady = deviceAvailable;

    return deviceAvailable;
}

/**
 * This function sends one byte over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param data - data for transmit
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_WriteByte(cSWi2cDrv_t * me, uint8_t addr, uint8_t data)
{
    int ack_ok;
    eTpRet tp_ret=TP_SUCCESS;
    stSWi2cDevice_t *pstDevice;

    ASSERT( ((me->isBusy == FALSE) && me->isReady) );

    me->isBusy = TRUE;	// hold the bus

    pstDevice = me->pConfig;

    if( ! me->devReady )
    {   // the i2c device is not available
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }

    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, pstDevice->devAddress);
    ack_ok = prvSWi2c_GetAck(pstDevice);
    if( ! ack_ok )
    {   // no ack error
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }
    prvSWi2c_SendByte(pstDevice, addr);
    prvSWi2c_GetAck(pstDevice);
    prvSWi2c_SendByte(pstDevice, data);
    prvSWi2c_GetAck(pstDevice);
    prvSWi2c_Stop(pstDevice);

    if( pstDevice->delayMSAfterWrite )
    {
        uint32_t tickEnd;
        tickEnd = getSysTime() + pstDevice->delayMSAfterWrite;
        while( getSysTime() < tickEnd );
    }
    
    me->isBusy = FALSE;	// release the bus

    return tp_ret;
}

/**
 * This function sends a variable length uint8 array over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param pArr - data array to be sent
 * @param len - how many bytes should be sent
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_WriteArray(cSWi2cDrv_t * me, uint8_t addr, uint8_t *pArr, uint32_t len)
{
    int ack_ok;
    uint32_t i;
    eTpRet tp_ret=TP_SUCCESS;
    stSWi2cDevice_t *pstDevice;

    ASSERT( ((me->isBusy == FALSE) && me->isReady && len) );

    me->isBusy = TRUE;  // hold the bus

    pstDevice = me->pConfig;

    if( ! me->devReady )
    {   // the i2c device is not available
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }

    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, pstDevice->devAddress);
    ack_ok = prvSWi2c_GetAck(pstDevice);
    if( ! ack_ok )
    {   // no ack error
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }
    for(i=0; i<len; i++)
    {
        prvSWi2c_SendByte(pstDevice, pArr[i]);
        prvSWi2c_GetAck(pstDevice);
    }
    
    prvSWi2c_Stop(pstDevice);

    if( pstDevice->delayMSAfterWrite )
    {
        uint32_t tickEnd;
        tickEnd = getSysTime() + pstDevice->delayMSAfterWrite;
        while( getSysTime() < tickEnd );
    }
    
    me->isBusy = FALSE; // release the bus

    return tp_ret;
}


/**
 * This function read one byte from the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param p_data - store the data
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_ReadByte(cSWi2cDrv_t * me, uint8_t addr, uint8_t *p_data)
{
    int ack_ok;
    eTpRet tp_ret=TP_SUCCESS;
    stSWi2cDevice_t *pstDevice;

    ASSERT( ((me->isBusy == FALSE) && me->isReady) );

    me->isBusy = TRUE;	// hold the bus

    pstDevice = me->pConfig;

    if( ! me->devReady )
    {   // the i2c device is not available
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }

    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, pstDevice->devAddress);
    ack_ok = prvSWi2c_GetAck(pstDevice);
    if( ! ack_ok )
    {   // no ack error
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }
    prvSWi2c_SendByte(pstDevice, addr);
    prvSWi2c_GetAck(pstDevice);
    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, (pstDevice->devAddress|0x01));
    prvSWi2c_GetAck(pstDevice);
    *p_data = prvSWi2c_ReadByte(pstDevice);
    prvSWi2c_Stop(pstDevice);
    
    me->isBusy = FALSE;	// release the bus

    return tp_ret;
}

/**
 * This function read a variable length uint8 array over the I2C bus in master mode
 * @param me - instance of the driver
 * @param addr - register address
 * @param p_data - store the data
 * @return TP_SUCCESS on success, otherwise TP_ACCESS_ERROR or TP_FAIL
 */
eTpRet SWi2cDrv_ReadArray(cSWi2cDrv_t * me, uint8_t addr, uint8_t *pArr, uint32_t len)
{
    int ack_ok;
    uint32_t index = 0;
    eTpRet tp_ret=TP_SUCCESS;
    stSWi2cDevice_t *pstDevice;

    ASSERT( ((me->isBusy == FALSE) && me->isReady && len) );

    me->isBusy = TRUE;	// hold the bus

    pstDevice = me->pConfig;

    if( ! me->devReady )
    {   // the i2c device is not available
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }

    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, pstDevice->devAddress);
    ack_ok = prvSWi2c_GetAck(pstDevice);
    if( ! ack_ok )
    {   // no ack error
        me->isBusy = FALSE; // release the bus
        return TP_ACCESS_ERROR;
    }
    prvSWi2c_SendByte(pstDevice, addr);
    prvSWi2c_GetAck(pstDevice);
    prvSWi2c_Start(pstDevice);
    prvSWi2c_SendByte(pstDevice, (pstDevice->devAddress|0x01));
    prvSWi2c_GetAck(pstDevice);
    index = 0;
    while( --len )
    {
        pArr[index] = prvSWi2c_ReadByte(pstDevice);
        index ++;
        prvSWi2c_GiveAck(pstDevice);
    }
    pArr[index] = prvSWi2c_ReadByte(pstDevice); // last byte, needn't ack
    prvSWi2c_Stop(pstDevice);
    
    me->isBusy = FALSE;	// release the bus

    return tp_ret;
}

#endif	//	HAS_SWi2c_DEVICE

