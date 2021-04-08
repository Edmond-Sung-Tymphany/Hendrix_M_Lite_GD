/**
 *  @file      I2CDrv.c
 *  @brief     This file contains the I2C driver implementation.
 *  @author    Dmitry Abdulov
 *  @date      19-Aug-2013
 *  @copyright Tymphany Ltd.
 */
#include "./I2CDrv_priv.h"
#include "attachedDevices.h"
#include "trace.h"


/*============================================================================*/
/* Forward declaration of private functions */
static bool _I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg);
static bool _I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg);
static bool i2cTransmitOneByte(eI2C_Channel id ,  uint8 data );
static void i2cWait(eI2C_Channel id);
static bool i2cSend(cI2CDrv * me, uint8 dat);
static bool i2c_start(cI2CDrv * me, bool restart);
static uint8 i2cRead(eI2C_Channel id);
static void i2cWaitForAck(eI2C_Channel id);
static void i2cWaitForNonAck(eI2C_Channel id);
static I2C_MODULE getI2CChannel(eI2C_Channel id);
/*============================================================================*/
/* PUBLIC FUNCTIONS */
void i2cReset(eI2C_Channel id);

/**
 * Construct the i2c driver instance.
 * @param me - instance of the driver
 * @param pConfig pointer to the config structure
 */
void I2CDrv_Ctor(cI2CDrv * me, tI2CDevice * pConfig)
{
    uint8 tempBuf;
   
    if ((NULL != me) && (0 == me->registeredUser)) // me->registeredUser == 0 means the I2C driver have not been ctor before.
    {
        /* Copy the config over */
        me->pConfig = pConfig;
        
        me->pConfig->pvSlaveCallback = NULL;

        switch (me->pConfig->i2cMode)
        {
            case I2C_MASTER_MODE:
                I2cDrv_Init(me->pConfig->channel);// implemented in attachedDevices.c for project specific.
                I2CEnable(getI2CChannel(me->pConfig->channel), FALSE);

                switch(getI2CChannel(me->pConfig->channel))
                {
                    case I2C1:
                        I2C_CLEAR_INTERRUPT(I2C_ONE);
                        I2C_BAUDRATE_SET(I2C_ONE, me->pConfig->baudRate);
                        I2CEnable(getI2CChannel(me->pConfig->channel), TRUE);
                        tempBuf = I2C_RECEIVE_BUF(I2C_ONE);
                        break;
                    case I2C2:
                        I2C_CLEAR_INTERRUPT(I2C_TWO);
                        I2C_BAUDRATE_SET(I2C_TWO, me->pConfig->baudRate);
                        I2CEnable(getI2CChannel(me->pConfig->channel), TRUE);
                        tempBuf = I2C_RECEIVE_BUF(I2C_TWO);
                        break;
                    default:
                        return;
                }
                I2CDrv_Reset(me);
                break;
            case I2C_SLAVE_MODE: /*TODO: to be implemented*/
                return;
                break;
            default:
                /* i2c mode have to be set as either master or slave*/
                return;
                break;
        }

        me->isReady = TRUE;
    }
    else
    {
        me->registeredUser++;
    }
}

/**
 * Exit & clean up the driver.
 * @param me - instance of the driver
 */
void I2CDrv_Xtor(cI2CDrv * me)
{
    if((NULL != me) && (me->registeredUser > 0))
    {
        me->registeredUser--;
        if (0 == me->registeredUser)
        {/* the I2C drviver should only be xtor if no one use it any more. */
            I2CEnable(getI2CChannel(me->pConfig->channel), FALSE);
            /* set the I2C pin to tri-state to save power*/
            I2cDrv_DeInit(me->pConfig->channel);// implemented in attachedDevices.c for project specific.
        }
    }
}


/**
 * Get i2c baud rate - value of I2C1BRG register
 * @param  me - pointer to driver instance
 * return I2C1BRG value
 */
uint32 I2CDrv_GetBR(cI2CDrv * me)
{
    switch (getI2CChannel(me->pConfig->channel))
    {
        case I2C1:
        {
            me->pConfig->baudRate = I2C_BAUDRATE_GET(I2C_ONE);
            break;
        }
        case I2C2:
        {
            me->pConfig->baudRate = I2C_BAUDRATE_GET(I2C_TWO);
            break;
        }
        default:{}
    }
    return me->pConfig->baudRate;
}

/**
 * Set i2c baud rate - update register
 * @param  me - pointer to driver instance
 * @param update value
 */
void I2CDrv_SetBR(cI2CDrv * me, uint32 baudrate)
{
    me->pConfig->baudRate = baudrate;
    switch (getI2CChannel(me->pConfig->channel))
    {
        case I2C1:
        {
            I2C_BAUDRATE_SET(I2C_ONE, me->pConfig->baudRate);
            break;
        }
        case I2C2:
        {
            I2C_BAUDRATE_SET(I2C_TWO, me->pConfig->baudRate);
            break;
        }
        default:{}
    }

}


/**
 * Check if the driver is in a ready state.
 * @param me - instance of the driver
 * return driver instance status
 * TRUE  : driver is on and ready for read\write etc commands
 * FALSE : driver is off\or busy
 */
bool I2CDrv_isReady(const cI2CDrv * me)
{
    return me->isReady;
}

/**
 * @param me - instance of the driver
 * return driver instance id
 */
uint16 I2CDrv_getID(const cI2CDrv * me)
{
    return me->driverID;
}
/**
 * Reset the i2c channel
 * @param me - instance of the driver
 */
void I2CDrv_Reset(const cI2CDrv * me)
{
    switch (me->pConfig->channel)
    {
        case I2C_CHANNEL_ONE:
        {
            I2C_HW_STOP_COND_ENABLE(I2C_ONE);
            /* wait for hardware clear of stop bit */
            while (I2C_IS_HW_STOP_COND_ENABLED(I2C_ONE));

            I2C_DISABLE_RECEIVE_MODE(I2C_ONE);
            I2C_CLEAR_INTERRUPT(I2C_ONE);
            I2C_CLEAR_WRITECOLLISION_DBIT(I2C_ONE);
            I2C_CLEAR_MASTERBUSCOLLISION_DBIT(I2C_ONE);
            break;
        }
        case I2C_CHANNEL_TWO:
        {
            I2C_HW_STOP_COND_ENABLE(I2C_TWO);
            /* wait for hardware clear of stop bit */
            while (I2C_IS_HW_STOP_COND_ENABLED(I2C_TWO));

            I2C_DISABLE_RECEIVE_MODE(I2C_TWO);
            I2C_CLEAR_INTERRUPT(I2C_TWO);
            I2C_CLEAR_WRITECOLLISION_DBIT(I2C_TWO);
            I2C_CLEAR_MASTERBUSCOLLISION_DBIT(I2C_TWO);
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
}

/**
 * This function sends a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be sent.
 * and pointer to a uint8 array of size length to be sent over the I2C bus
 * @return TRUE on success, otherwise FALSE
 */

bool I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg)
{
    bool ret = FALSE;
    INTDisableInterrupts();
    ret = _I2CDrv_MasterWrite(me, msg);
    INTEnableInterrupts();
    return ret;
}

static bool _I2CDrv_MasterWrite(cI2CDrv * me, tI2CMsg const * const  msg)
{
    uint8 *pMsg = msg->pMsg;
    uint16 unIndex;
    //TP_PRINTF("%s(0x%x , 0x%x)\n", __FUNCTION__, msg->devAddr, *(msg->pMsg));
    me->isReady = FALSE;

    i2c_start(me, FALSE);

    if(!i2cSend(me,msg->devAddr))
    {
        return FALSE;
    }

    for (unIndex = 0; unIndex <  msg->length; unIndex++)
    {
        if(!i2cSend(me, *pMsg))
        {
            return FALSE;
        }
        pMsg++;
    }

    I2CDrv_Reset(me);

    me->isReady = TRUE;

    return TRUE;
}

/**
 * This function reads a variable length uint8 array over the i2C bus in master mode
 * @param me - instance of the driver
 * @param tI2CMsg * const  msg representing structure: length of the array to be read
 * and pointer to a uint8 array of size length to be read from the I2C bus
 * @return TRUE if sucess, otherwise FALSE
 */

bool I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg)
{
    bool ret = FALSE;
    INTDisableInterrupts();
    ret = _I2CDrv_MasterRead(me, msg);
    INTEnableInterrupts();
    return ret;
}

static bool _I2CDrv_MasterRead(cI2CDrv * const  me, tI2CMsg *  msg)
{
    bool result = TRUE;
    uint8 temp;
    uint16 i;
    uint32 regAddr;
    eI2CRegAddLen regAddrLen;
    ASSERT(me && msg && msg->pMsg);
    //TP_PRINTF("%s(0x%x , 0x%x)\n", __FUNCTION__, msg->devAddr, msg->regAddr);

    me->isReady = FALSE;

    regAddr = msg->regAddr;         
    regAddrLen = me->pConfig->regAddrLen;      /* check 8 bit or 16 bit address*/
    switch (getI2CChannel(me->pConfig->channel))
    {
        case I2C1:
        {
            I2C_HW_START_ENABLE(I2C_ONE);
            {
                i=I2C_START_TIMEOUT;
                while((!I2C_INTERRUPT_FLAG(I2C_ONE))&&(i--));

            }
            I2C_CLEAR_INTERRUPT(I2C_ONE);
            break;
        }
        case I2C2:
        {
            I2C_HW_START_ENABLE(I2C_TWO);
            {
                i=I2C_START_TIMEOUT;
                while((!I2C_INTERRUPT_FLAG(I2C_ONE))&&(i--));
            }
            I2C_CLEAR_INTERRUPT(I2C_TWO);
            break;
        }
        default:
            return FALSE;
    }

    result = i2cSend(me, msg->devAddr);

    switch (regAddrLen)
    {
        case REG_LEN_8BITS:
            temp = (uint8) regAddr;
            result = i2cSend(me, temp);
            break;
        case REG_LEN_16BITS:
            temp = (regAddr >> 8);
            result = i2cSend(me, temp); /* sending register high 8 bits */
            temp = (uint8) regAddr;
            result = i2cSend(me, temp); /* sending register low 8 bits */
            break;
        default:
            /* regAddrLen is invalid */
            ASSERT(0);
            break;
    }
    /*doing i2c restart here  */

    /*!< i2c restart */
    switch (me->pConfig->channel)
    {
        case I2C_CHANNEL_ONE:
        {
           I2C_HW_REPEATED_START_COND_ENABLE(I2C_ONE);
           while(I2C_IS_HW_REPEATED_START_COND_ENABLED(I2C_ONE));
           break;
        }
        case I2C_CHANNEL_TWO:
        {
            I2C_HW_REPEATED_START_COND_ENABLE(I2C_TWO);
            while(I2C_IS_HW_REPEATED_START_COND_ENABLED(I2C_TWO));
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }

    i2cSend(me, (msg->devAddr | 0x01)); /* device address bit set to read */

    /* read bytes except the last byte. */
    uint8 *pMsg = msg->pMsg;
    for (i = 0; i < (msg->length-1); i++)
    {
        *pMsg = i2cRead(getI2CChannel(me->pConfig->channel));
        if (NULL != pMsg)
        {
            pMsg++;
        }
        i2cWaitForAck(getI2CChannel(me->pConfig->channel));
    }// end of for

    /* read the last byte */
    *pMsg = i2cRead(getI2CChannel(me->pConfig->channel));
    i2cWaitForNonAck(getI2CChannel(me->pConfig->channel));

    I2CDrv_Reset(me);
    return result;
}
/*============================================================================*/
/* PRIVATE FUNCTIONS */

/**
 * This starts up the i2c channel
 * @param me - instance of the driver
 * @param resart - In restart mode?
 * @return
 *     TRUE    - Started up no problem
 *     FALSE   - Ooops
 */
static bool i2c_start(cI2CDrv * me, bool restart)
{
    I2C_STATUS status;

    I2CStart( getI2CChannel(me->pConfig->channel));
    if(restart)
    {
        I2CRepeatStart(getI2CChannel(me->pConfig->channel));
    }
    else
    {
        // Wait for the bus to be idle, then start the transfer
        //while( !I2CBusIsIdle(getI2CChannel(me->config.channel)) );

        if(I2CStart(getI2CChannel(me->pConfig->channel)) != I2C_SUCCESS)
        {
            //DBPRINTF("Error: Bus collision during transfer Start\n");
            return FALSE;
        }
    }

    // Wait for the signal to complete
    do
    {
        status = I2CGetStatus(getI2CChannel(me->pConfig->channel));

    } while ( !(status & I2C_START) );   // Send the Start (or Restart) signal
    return TRUE;
};


/**
 * This transmits one byte to the I2C, and reports errors for any bus
 * collisions.
 * @param me - instance of the driver
 * @return
 *     TRUE    - data was sent successfully
 *     FALSE   - a bus collision occurred
 */
static bool i2cTransmitOneByte(eI2C_Channel id,  uint8 data )
{
    uint32 i = 0;
    i=I2C_TRANSMIT_TIMEOUT;
    while (!I2CTransmitterIsReady(id) && (i--));
    // Transmit the byte
    if(I2CSendByte(id, data) == I2C_MASTER_BUS_COLLISION)
    {
        return FALSE;
    }
    // Wait for the transmission to finish
    i=I2C_TRANSMIT_TIMEOUT;
    while (!I2CTransmitterIsReady(id) && (i--));
    return TRUE;
}

static void i2cWaitForAck(eI2C_Channel id)
{
    switch (id)
    {
        case I2C1:
        {
            I2C_SEND_RCVD_ACK(I2C_ONE); /* Acknowledge data bit, 0 = ACK */
            I2C_HW_START_ACK_SEQ(I2C_ONE); /* Ack data enabled */
            while (I2C_IS_HW_ACK_SEQ_ACTIVE(I2C_ONE)); /* wait for ack data to send on bus */
            break;
        }
        case I2C2:
        {
            I2C_SEND_RCVD_ACK(I2C_TWO); /* Acknowledge data bit, 0 = ACK */
            I2C_HW_START_ACK_SEQ(I2C_TWO); /* Ack data enabled */
            while (I2C_IS_HW_ACK_SEQ_ACTIVE(I2C_TWO)); /* wait for ack data to send on bus */
            break;
        }
        default:
            break;
    }
}

static void i2cWaitForNonAck(eI2C_Channel id)
{
    switch (id)
    {
        case I2C1:
        {
            I2C_SEND_RCVD_NOT_ACK(I2C_ONE);          /* Acknowledge data bit, 1 = NAK */
            I2C_HW_START_ACK_SEQ(I2C_ONE);           /* Ack data enabled */
            while (I2C_IS_HW_ACK_SEQ_ACTIVE(I2C_ONE)); /* wait for ack data to send on bus */
            break;
        }
        case I2C2:
        {
            I2C_SEND_RCVD_NOT_ACK(I2C_TWO); /* Acknowledge data bit, 1 = ACK */
            I2C_HW_START_ACK_SEQ(I2C_TWO); /* Ack data enabled */
            while (I2C_IS_HW_ACK_SEQ_ACTIVE(I2C_TWO)); /* wait for ack data to send on bus */
            break;
        }
        default:
            break;
    }
}

/* waits if i2c transfer isn't finish yet*/
static void i2cWait(eI2C_Channel id)
{
    switch (id)
    {
        case I2C1:
        {
            while (( ( I2C_SP_CON(I2C_ONE) & I2C_CON_MASK ) || ( I2C_SP_STAT(I2C_ONE) & I2C_STAT_MASK)));
            break;
        }
        case I2C2:
        {
            while (( ( I2C_SP_CON(I2C_TWO) & I2C_CON_MASK ) || ( I2C_SP_STAT(I2C_TWO) & I2C_STAT_MASK)));
            break;
        }
        default:
            break;
    }

}



static bool i2cSend(cI2CDrv * me, uint8 dat)
{
    bool result = TRUE;
    switch (me->pConfig->channel)
    {
        case I2C_CHANNEL_ONE:
        {
            while (I2C_IS_TBUFFER_FULL(I2C_ONE));
            
            
            I2C_CLEAR_INTERRUPT(I2C_ONE);
            if (!i2cTransmitOneByte(getI2CChannel(me->pConfig->channel), dat))
                return FALSE;

            // wait for transmission   
            while (I2C_TRANSMIT_IN_PROGRESS(I2C_ONE));
            
            // Check for NO_ACK from slave, abort if not found
            if (I2C_IS_NACK_RCV(I2C_ONE))
            {
                I2CDrv_Reset(me);
                result = FALSE;
            }
            else
            {
                i2cWait(getI2CChannel(me->pConfig->channel));
                result = TRUE;
            }
            break;
        }
     case I2C_CHANNEL_TWO:
        {
            while (I2C_IS_TBUFFER_FULL(I2C_TWO))
            {
            }
            I2C_CLEAR_INTERRUPT(I2C_TWO);
            if (!i2cTransmitOneByte(getI2CChannel(me->pConfig->channel), dat))
                return FALSE;

            while (I2C_TRANSMIT_IN_PROGRESS(I2C_TWO))
            {
            }

            if (I2C_IS_NACK_RCV(I2C_TWO))
            {
                I2CDrv_Reset(me);
                result = FALSE;
            }
            else
            {
                i2cWait(getI2CChannel(me->pConfig->channel));
                result = TRUE;
            }
            break;
        }
        default:
        {
            ASSERT(0);
        }
    }
    return result;
}

static uint8 i2cRead(eI2C_Channel id)
{
    uint8 temp = 0;
    /* Reception works if transfer is initiated in read mode */
    /* Enable data reception */
    I2C_RESULT ret = I2CReceiverEnable(id, TRUE);
    switch (id)
    {
        case I2C1:
        {
            while(I2C_SUCCESS != ret)
            {
                I2C_CLEAR_RCV_OVER_FLOW_FLAG(I2C_ONE);
                ret = I2CReceiverEnable(id, TRUE);
            }
            while ((!I2C_IS_BUFFER_FULL(I2C_ONE)));
            break;
        }
        case I2C2:
        {
            while(I2C_SUCCESS != ret)
            {
                I2C_CLEAR_RCV_OVER_FLOW_FLAG(I2C_TWO);
                ret = I2CReceiverEnable(id, TRUE);
            }
            while ((!I2C_IS_BUFFER_FULL(I2C_TWO)));
            break;
        }
        default:
            break;
    }
    /* Read data from recv register */
    temp = I2CGetByte(id);
    i2cWait(id);

    return temp;
}

static I2C_MODULE getI2CChannel(eI2C_Channel id)
{
    return (I2C_MODULE) id;
}

bool I2CDrv_MasterWriteWith2ByteRegAddress(cI2CDrv * me, tI2CMsg * const msg)
{
    uint16 unIndex;
    uint8 tmp[2];
    me->isReady = FALSE;
    tmp[0] = (msg->regAddr & 0xFF00)>>8;
    tmp[1] = msg->regAddr & 0x00FF;

    i2c_start(me, FALSE);

    if(!i2cSend(me,msg->devAddr))
    {
        return FALSE;
    }
    if(!i2cSend(me,tmp[0]))
    {
        return FALSE;
    }
    if(!i2cSend(me,tmp[1]))
    {
        return FALSE;
    }

    for (unIndex = 0; unIndex <  msg->length; unIndex++)
    {
        if(!i2cSend(me, *( msg->pMsg)))
        {
            return FALSE;
        }
        (msg->pMsg)++;
    }

    I2CDrv_Reset(me);

    me->isReady = TRUE;

    return TRUE;
}

bool I2CDrv_MasterReadWith2ByteRegAddress( cI2CDrv * const me, tI2CMsg * msg)
{
    bool result = TRUE;
    uint16 i;
    uint8 temp;
    uint8 regAddr;
    //uint8 tmp[2];
    eI2CRegAddLen regAddrLen;
    ASSERT(me && msg && msg->pMsg);
    //TP_PRINTF("%s(0x%x , 0x%x)\n", __FUNCTION__, msg->devAddr, msg->regAddr);

    me->isReady = FALSE;

    regAddrLen = me->pConfig->regAddrLen;      /* check 8 bit or 16 bit address*/
    switch (me->pConfig->channel)
    {
        case I2C_CHANNEL_ONE:
        {
            I2C_HW_START_ENABLE(I2C_ONE);
            {
                i=I2C_START_TIMEOUT;
                while((!I2C_INTERRUPT_FLAG(I2C_ONE))&&(i--));

            }
            I2C_CLEAR_INTERRUPT(I2C_ONE);
            break;
        }
        case I2C_CHANNEL_TWO:
        {
            I2C_HW_START_ENABLE(I2C_TWO);
            {
                i=I2C_START_TIMEOUT;
                while((!I2C_INTERRUPT_FLAG(I2C_ONE))&&(i--));
            }
            I2C_CLEAR_INTERRUPT(I2C_TWO);
            break;
        }
        default:
            return FALSE;
    }

    result = i2cSend(me, msg->devAddr);
    regAddr=msg->regAddr;
    {
            temp = (regAddr >> 8);
            result = i2cSend(me, temp); /* sending register high 8 bits */
            temp = (uint8) regAddr;
            result = i2cSend(me, temp); /* sending register low 8 bits */
    }
    /*doing i2c restart here  */

    /*!< i2c restart */
    switch (me->pConfig->channel)
    {
        case I2C_CHANNEL_ONE:
        {
           I2C_HW_REPEATED_START_COND_ENABLE(I2C_ONE);
           while(I2C_IS_HW_REPEATED_START_COND_ENABLED(I2C_ONE));
           break;
        }
        case I2C_CHANNEL_TWO:
        {
            I2C_HW_REPEATED_START_COND_ENABLE(I2C_TWO);
            while(I2C_IS_HW_REPEATED_START_COND_ENABLED(I2C_TWO));
            break;
        }
        default:
        {}
    }

    i2cSend(me, (msg->devAddr | 0x01)); /* device address bit set to read */

    /* read bytes except the last byte. */
    uint8 *pMsg = msg->pMsg;
    for (i = 0; i < (msg->length-1); i++)
    {
        *pMsg = i2cRead(getI2CChannel(me->pConfig->channel));
        if (NULL != pMsg)
        {
            pMsg++;
        }
        i2cWaitForAck(getI2CChannel(me->pConfig->channel));
    }// end of for

    /* read the last byte */
    *pMsg = i2cRead(getI2CChannel(me->pConfig->channel));
    i2cWaitForNonAck(getI2CChannel(me->pConfig->channel));

    I2CDrv_Reset(me);

    return result;
}


/*============================================================================*/
#ifdef I2C_SLAVE_SUPPORT_ON
/*
 * TODO: to be implemented
 * This function sends a variable length uint8 array over the i2C bus in slave mode
 */
void I2CDrv_SlaveWrite(const cI2CDrv * me, ...)
{

}

/*
 * TODO: to be implemented
 * This function reads a variable length uint8 array over the i2C bus in slave mode
 */
void I2CDrv_SlaveRead(const cI2CDrv * me, ...)
{
    
}

#endif




/*____________________________________________________________________________*/




