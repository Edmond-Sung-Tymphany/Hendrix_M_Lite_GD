#include "product.config"
#include "trace.h"
#include "bsp.h"
#include "I2CDrv.h"
#include "adi_ad24xx.h"
#include "adi_ad24xx_priv.h"
#include "adi_ad24xx_reg.h"  // from ADI sample code

#include "AD242x.h"

#include "adi_ad24xx_drv_config.inc"

static void adi_a2b_ParseMasterNCD(const ADI_A2B_MASTER_NCD* pMstCfg,ADI_A2B_NODE* pNode,ADI_A2B_COMMON_CONFIG* pCommon);
static void adi_a2b_ParseSlaveNCD(const ADI_A2B_SLAVE_NCD* pSlvCfg,ADI_A2B_NODE* pNode, ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle);
static void adi_a2b_ParseSlavePinMux012( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg);
static void adi_a2b_ParseSlavePinMux34( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg);
static void  adi_a2b_ParseSlavePinMux56( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg);
static void  adi_a2b_ParseSlavePinMux7( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg);
static void  adi_a2b_ParseMasterPinMux12( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseMasterPinMux34( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseMasterPinMux56( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg);
static void  adi_a2b_ParseMasterPinMux7( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg);
static void adi_a2b_ParseMasterNCDForAD242x(const ADI_A2B_MASTER_NCD* pMstCfg,ADI_A2B_NODE* pNode,ADI_A2B_COMMON_CONFIG* pCommon);
static void adi_a2b_ParseSlaveNCDForAD242x(const ADI_A2B_SLAVE_NCD* pSlvCfg,ADI_A2B_NODE* pNode, ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle);



uint8 adi_a2b_getMasterNodeIndex(uint16 nNodeID);
void adi_a2b_clearPreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode);
uint8 adi_a2b_getSlaveNodeIndex(uint16 nNodeID);
uint8 adi_a2b_masterPreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode);
void adi_a2b_getMasterNodePtr(uint16 NodeID ,ADI_A2B_NODE** ppNode);
uint8 adi_a2b_slavePreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode);
void adi_a2b_getSlaveNodePtr(uint16 NodeID ,ADI_A2B_NODE** ppNode);

uint8 adi_a2b_DeviceConfig(ADI_A2B_NODE* pNode, ADI_A2B_CONNECTED_DEVICE* pDevice , ADI_A2B_NODE_PERICONFIG sPeriConfig);

ADI_A2B_RESULT adi_a2b_ConfigNodePowerMode(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , uint8 nSwitchValue );
ADI_A2B_RESULT adi_a2b_ValidateNetwork(cAdiAD2410Drv* me, ADI_A2B_GRAPH *pgraph,ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle );
#ifdef A2B_USE_BUS_DESCRIPTION_FILE
ADI_A2B_RESULT adi_a2b_ParseBusDescription(ADI_A2B_GRAPH* pGraph, const ADI_A2B_BCD*  pBusDescription,ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle);
#endif


static ADI_A2B_GRAPH graph;
static ADI_A2B_FRAMEWORK_HANDLER oFramework;

#ifndef A2B_PRINT_FOR_DEBUG
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif



void  adi_a2b_Delay(uint32 nTime)
{
    while(nTime--)
    {
        uint16 i=100;
        while (i--);
    }
}

static uint32 adi_a2b_PeripheralDeviceInit(ADI_A2B_NODE* pNode , ADI_A2B_NODE_PERICONFIG sPeriConfig)
{
     uint8 nIndex/*,nDeviceAddr*/;
     uint32 nResult = 0u;

    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
       
     for(nIndex = 0u ; nIndex < ADI_A2B_MAX_DEVICES_PER_NODE ;nIndex++)
     {
          //nDeviceAddr = (uint8)pNode->oConnectedTo[nIndex].nI2CAddress;
          
          pNode->oConnectedTo[nIndex].bActive = 1u;
          
          /* Configure only for known device */
          if((uint32)pNode->oConnectedTo[nIndex].eDeviceID != (uint32)UNKNOWN)
          {
              nResult = (uint32)adi_a2b_DeviceConfig(pNode,&(pNode->oConnectedTo[nIndex]),sPeriConfig);
          }       
          if(nResult != 0u)
          {
               break;
          }

     }
          
     return nResult;
}


/*****************************************************************************/
/*!
    @brief      This function updates a table which returns position of the node in graph for a given Node ID.
                Table should to be re-synchronized after graph update(either from ERREPROM or SigmaStudio or BCF) 

    @param [in] pFrameWorkHandle   Pointer framework configuration structure


    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success
*/
/*****************************************************************************/
void adi_a2b_UpdateNodeReferenceTable(ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameWorkHandle)
{
   uint8 i;
   uint8 /*nBranchID,*/nSlaveID;
   uint16 nNodeID;
   ADI_A2B_GRAPH *pgraph = pFrameWorkHandle->pgraph;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);

   for (i = 0u; i < pgraph->nNodeCount; i++)
   {
       nNodeID = pgraph->oNode[i].nID;

       if(pgraph->oNode[i].eType == (uint16)ADI_A2B_MASTER)
       {
          //nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
          pFrameWorkHandle->aMasterNodeReferenceTable = i;
       }
       else if (pgraph->oNode[i].eType == (uint16)ADI_A2B_SLAVE)
       {
          //nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
          nSlaveID = (uint8)((nNodeID & 0xFFu));
          pFrameWorkHandle->aSlaveNodeReferenceTable[nSlaveID] = i;
       }
/*
       else
       {
           nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
       }
*/
   }
}

#if 1
ADI_A2B_RESULT adi_a2b_UpdateGraph(cAdiAD2410Drv* me, ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle)
{

    //uint8  nCount =0u;
    //ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    ADI_A2B_NODE_PERICONFIG sPeriConfig;
    uint32 nResult;
    /* Pointer to master node*/
    ADI_A2B_NODE* pNode = &(pFrameworkHandle->pgraph->oNode[0]);
   
    /* Parse BCF if and only if graphdata is not exported from SigmaStudio */
#ifdef A2B_USE_BUS_DESCRIPTION_FILE
    /*eResult = */adi_a2b_ParseBusDescription(pFrameworkHandle->pgraph, &sBusDescription ,pFrameworkHandle);    
#endif    
         
    
 
#if A2B_SS_ENABLE        
    /* Wait for 0.5 second for USB detection */
    while(pFrameworkHandle->oSSMsgHandler.bSSConnected == 0u)
    { 
        adi_a2b_Delay(50u);
        nCount++;
        if( nCount > 10u)
        {
            break;
        }
    }    
    /* If USB is detected, wait for SigmaStudio, otherwise update from EEPROM */
    if(pFrameworkHandle->oSSMsgHandler.bSSConnected ==1u)
    { 
        adi_a2b_MonitorGraphDownload(pFrameworkHandle);     
    }
    else   
#endif    
    {
#if A2B_UPDATE_FROM_EEPROM 
        adi_a2b_UpdateGraphFromEEPROM(pFrameworkHandle);
#endif         
    }

    /* Update the conversion table */
    adi_a2b_UpdateNodeReferenceTable(pFrameworkHandle);
        
    sPeriConfig = pFrameworkHandle->aPeriDownloadTable[0];
    
    /* Program all the devices connected to target processor */
    nResult = adi_a2b_PeripheralDeviceInit(pNode,sPeriConfig);

    return ((ADI_A2B_RESULT)nResult);
}

#else
void adi_a2b_UpdateGraph(cAdiAD2410Drv* me)
{

    uint8  nCount =0u;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    ADI_A2B_NODE_PERICONFIG sPeriConfig;
    uint32 nResult;
    /* Pointer to master node*/
    ADI_A2B_NODE* pNode;

    pNode = &(me->pFrameworkHandle->pgraph->oNode[0]);
    /* Parse BCF if and only if graphdata is not exported from SigmaStudio */
#ifdef A2B_USE_BUS_DESCRIPTION_FILE
    adi_a2b_ParseBusDescription(me->pGraph, &sBusDescription ,me->pFrameworkHandle);    
#endif    

#if 0//A2B_SS_ENABLE        
    /* Wait for 0.5 second for USB detection */
    while(pFrameworkHandle->oSSMsgHandler.bSSConnected == 0u)
    { 
        adi_a2b_Delay(50u);
        nCount++;
        if( nCount > 10u)
        {
            break;
        }
    }    
    /* If USB is detected, wait for SigmaStudio, otherwise update from EEPROM */
    if(pFrameworkHandle->oSSMsgHandler.bSSConnected ==1u)
    { 
        adi_a2b_MonitorGraphDownload(pFrameworkHandle);     
    }
    else   
#endif    
    {
#if 0//A2B_UPDATE_FROM_EEPROM 
        adi_a2b_UpdateGraphFromEEPROM(pFrameworkHandle);
#endif         
    }

    /* Update the conversion table */
    //adi_a2b_UpdateNodeReferenceTable(me->pFrameworkHandle);
        
    //sPeriConfig = pFrameworkHandle->aPeriDownloadTable[0][0];
    
    /* Program all the devices connected to target processor */
    //nResult = adi_a2b_PeripheralDeviceInit(pNode,sPeriConfig);

}
#endif

void adi_a2b_Ctor(cAdiAD2410Drv* me)
{
    tI2CDevice * devObj = (tI2CDevice *) getDevicebyIdAndType(A2B_MASTER_DEV_ID, I2C_DEV_TYPE, NULL);
    I2CDrv_Ctor(&me->i2cObj, (tI2CDevice*)devObj);

    me->deviceAddr = devObj->devAddress;
    me->isCreated = TRUE;
    me->i2cEnable = TRUE;  
#ifdef TUNING_ON_ST_EVK_BOARD
    me->i2cEnable = FALSE;  
#endif
    memset(&oFramework, 0, sizeof(ADI_A2B_FRAMEWORK_HANDLER));
    memset(&graph, 0, sizeof(ADI_A2B_GRAPH));
    me->pFrameworkHandle =&oFramework;
    me->pFrameworkHandle->pgraph = &graph;
    me->pFrameworkHandle->pgraph->nNodeCount = ADI_A2B_MAX_GRAPH_NODES;
}

void adi_a2b_Xtor(cAdiAD2410Drv* me)
{
    I2CDrv_Xtor(&me->i2cObj);
    me->deviceAddr = NULL;
    me->isCreated = FALSE;
    me->i2cEnable = FALSE;  
}

//uint8 tmp_dat[5]={0x01, 0xaa, 0xbb, 0xcc, 0xdd};
//ADI_A2B_PERI_CONFIG_UNIT peri_write= 	{A2B_WRITE_OP,	1,	0x01,	0x1u,	0x5u,	&tmp_dat[0]};
//ADI_A2B_PERI_CONFIG_UNIT peri_read= 	{A2B_READ_OP,	1,	0x06,	0x1u,	0x4u,	&tmp_dat[0]};
void adi_a2b_drv_init(cAdiAD2410Drv* me)
{
    adi_a2b_UpdateGraph(me, me->pFrameworkHandle);
    adi_a2b_ValidateNetwork(me, me->pFrameworkHandle->pgraph, me->pFrameworkHandle);
}

/*
void adi_a2b_drv_deinit(cAdiAD2410Drv* me,  cI2CDrv *pI2cObj)
{
    //me->i2cObj = NULL;
    me->deviceAddr = NULL;
    me->isCreated = NULL;
    me->i2cEnable = NULL;    
}
*/

static uint32 adi_a2b_I2cWrite(cAdiAD2410Drv* me, uint8 nDeviceAddress,uint8 bytes, uint8 *data)
{
    uint32 retVal=0;
    if (me->i2cEnable == 0)
    {
        return (1);
    }
    tI2CMsg i2cMsg=
    {
        .devAddr = nDeviceAddress,
        .regAddr = NULL,
        .length = bytes,
        .pMsg = (uint8*)data
    };
    retVal=I2CDrv_MasterWrite(&me->i2cObj, &i2cMsg);
    return retVal;
}

static uint32 adi_a2b_I2cRead(cAdiAD2410Drv* me, uint8 nDeviceAddress, uint8 * bufptr,  uint8 reg_add, uint16 bytes)
{
    uint32 retVal;
    if (me->i2cEnable == 0)
    {
        return (1);
    }
    tI2CMsg i2cMsg=
    {
        .devAddr = nDeviceAddress,
        .regAddr = reg_add,
        .length = bytes,
        .pMsg = bufptr
    };
    retVal = I2CDrv_MasterRead(&me->i2cObj, &i2cMsg);
    return retVal;
}

uint32 adi_a2b_TwiWrite8(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nDeviceAddress, uint8 nRegAddress,uint8 nData)
{
    uint32 nReturnValue = (uint32)0;    
    uint8  nTWIData[2];

    nTWIData[0] = nRegAddress;
    nTWIData[1] = nData;
    //TP_PRINTF("W %x  %x %x\n", nDeviceAddress, nRegAddress, nData);

    nReturnValue = adi_a2b_I2cWrite(me, nDeviceAddress, 2, nTWIData);

    return(nReturnValue);    
}

uint32 adi_a2b_TwiRead8(cAdiAD2410Drv* me, uint8 nTWIDeviceNo, uint8 nDeviceAddress, uint8 nRegAddress, uint8* pData)
{
    uint32 nReturnValue = (uint32)0;    
    nReturnValue = adi_a2b_I2cRead(me, nDeviceAddress, pData, nRegAddress, 1);
    //TP_PRINTF("R %x  %x %x\n", nDeviceAddress, nRegAddress, *pData);
    return nReturnValue;
}



/****************************************************************************/
/*!
    @brief          This function sets up slave configuration specific to AD242X

    @param [in]     pNode          Pointer to node structure
    
    @return          Return code
                    - 0: Node is not discovered
                    - 1: Node is discovered.
*/                    
/********************************************************************************/
static uint8 adi_a2b_SlaveAD242xconfig(ADI_A2B_NODE *pNode, ADI_A2B_CONFIG_TABLE aTwiWriteTable[], uint8 nCount)
{
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);
	/* Slot enhancement related */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK0);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_UPMASK0;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK1);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_UPMASK1;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK2);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_UPMASK2;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK3);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_UPMASK3;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_UPOFFSET);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_UPOFFSET;


    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK0);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_DNMASK0;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK1);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_DNMASK1;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK2);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_DNMASK2;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK3);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_DNMASK3;


    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_AD242X0_DNOFFSET);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_DNOFFSET;

    /* GPIO over distance */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD0MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD0MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD1MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD1MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD2MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD2MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD3MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD3MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD4MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD4MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD5MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD5MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD6MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD6MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD7MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD7MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODINV);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIODINV;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODEN);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIODEN;

    /* Clock sustain */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_SUSCFG);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_SUSCFG;

    /* Reduced Rate support  */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRSOFFS);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_I2SRRSOFFS;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRCTL);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_I2SRRCTL;


    /* Clock Config */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_CLK1CFG;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_CLK2CFG;

    return(nCount);


}



/****************************************************************************/
/*!
    @brief          This function sets up master configuration specific to AD242X

    @param [in]     pNode          Pointer to node structure

    @return          Return code
                    - 0: Node is not discovered
                    - 1: Node is discovered.
*/
/********************************************************************************/
static uint8  adi_a2b_MasterAD242xconfig(ADI_A2B_NODE *pNode, ADI_A2B_CONFIG_TABLE aTwiWriteTable[], uint8 nCount)
{
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    /* GPIO over distance */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD0MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD0MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD1MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD1MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD2MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD2MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD3MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD3MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD4MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD4MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD5MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD5MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD6MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD6MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD7MSK);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIOD7MSK;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODINV);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIODINV;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODEN);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_GPIODEN;

    /* Reduced Rate support  */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRCTL);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_I2SRRCTL;

    /* Clock Config */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_CLK1CFG;

    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_CLK2CFG;

    return nCount;


}


/*****************************************************************************/
/*! 
    @brief      This function configures master node with I2S, GPIO settings

    @param [in] pNode   Pointer to the A2B node structure
    
    
    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success  
*/ 
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_masterConfig(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint32 nResult = 0u;
    uint8 nCount = 0u;
    uint8 nIndex/*,nPeriIndex*/;
    uint8 nA2BI2CAddr;
    //TP_PRINTF("%s\r\n", __FUNCTION__);

    /* Make sure enough memory is allocated */
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[A2B_MAX_NUM_REG]; 
    
     /* Preset to write to master */
    nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

        
    if( (pNode->bActive) == 1u)
    {
   
         /* I2S configuration*/
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRXOFFSET);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SRXOFFSET;  
    
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2STXOFFSET);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2STXOFFSET;   
    
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRATE); 
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SRATE; 
    
        if( (pNode->ePartNum != ADI_A2B_AD2403))
        {
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PDMCTL;
        }

    
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SCFG; 

#if A2B_ENABLE_AD241X_SUPPORT
        if((pNode->ePartNum == ADI_A2B_AD2410)||
           (pNode->ePartNum == ADI_A2B_AD2403))
        {
			 /* Clock enabling  */
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_CLKCFG);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CLKCFG;
        }
#endif
        
        /* Interrupt unmask*/
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK2);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK2; 

        /* Interrupt unmask*/
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK1; 

        /* Interrupt unmask*/
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0); /**/
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK0; 

        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2CCFG);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2CCFG;
        
        /* Error management registers */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_ERRMGMT);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_ERRMGMT;  
       
        
              
        /* GPIO programming */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIOOEN; 
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIOIEN; 
                   
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODAT;
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATCLR);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODATCLR; 
              
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATSET);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODATSET; 
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINTEN; 
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINTINV;
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINCFG; 


        /* Test mode register */        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPRBSRegister.nREG_A2B0_TESTMODE);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_TESTMODE; 
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_BECCTL);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_BECCTL; 
#if A2B_ENABLE_AD242X_SUPPORT
        /* Configuration specific to AD242X */
        nCount = adi_a2b_MasterAD242xconfig(pNode, aTwiWriteTable, nCount);
#endif
        for(nIndex  = 0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }
            
            
        }
        
        /* Adding master node configuration failure */ 
        pNode->bActive &= (uint16)( ~((uint16)nResult) );

    }
    else
    {
        nResult = 1u;
    }
    
    /* Check for failure */
    if(nResult!= 0u )
    {
        eResult = ADI_A2B_FAILURE;
    } 
    
#if A2B_PRINT_FOR_DEBUG
    if( nResult == 0u)
    {
        printf(" \n Branch %d : Master Node Configuration : SUCCESS \n ",((pNode->nID & 0xFF00u) >> 8u) );
    }
    else
    {
        printf(" \n Branch %d : Master Node Configuration : FAILURE \n",((pNode->nID & 0xFF00u) >> 8u) );
    }
#endif     
    
       
    return(eResult);
}


/****************************************************************************/
/*!
    @brief             This function configures(Slots & I2S) slave node according 
                       to graphdata 
    
    @param [in]        pNode            Pointer to slave A2B node structure
        
    @return        Return code
                    - 0: Success
                    - 1: Failure     
*/                    
/*******************************************************************************/ 
ADI_A2B_RESULT adi_a2b_SlaveConfig(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint32 nResult = 0u;
    uint8 nVal;
    uint8 nCount = 0u;
    uint8 nIndex;

    uint8 nA2BI2CAddr;
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);

		/* Make sure enough memory is allocated */
		ADI_A2B_CONFIG_TABLE aTwiWriteTable[A2B_MAX_NUM_REG];

         /* Preset to write to master */
        nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

        /*! Enable Slave node addressing */
        /* Normal mode  */
        nVal  = 0x00u | (uint8)(pNode->nID) ;
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
      
        //aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PLLCTL);
        //aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PLLCTL;
        
        /* Unmask the Interrupt*/
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK1; 

        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK0; 

        /* Resp Cycles  */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_RESPCYCS; 

        /* I2C configuration */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2CCFG);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2CCFG;

        if((pNode->ePartNum == ADI_A2B_AD2410) || (pNode->ePartNum == ADI_A2B_AD2403) \
        		|| (pNode->ePartNum == ADI_A2B_AD2425))
        {
            /* I2S configuration */
            aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_SYNCOFFSET);
            aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SYNCOFFSET;
            
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRATE); 
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SRATE; 
            
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SGCFG;

			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SCFG;

        }
                
        if( (pNode->ePartNum != ADI_A2B_AD2403))
        {
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PDMCTL;
        }
#if A2B_ENABLE_AD241X_SUPPORT
        if((pNode->ePartNum == ADI_A2B_AD2410) || (pNode->ePartNum == ADI_A2B_AD2401)\
        || (pNode->ePartNum == ADI_A2B_AD2402))
        {
			/* Clock enabling */
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_CLKCFG);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CLKCFG;
        }
#endif
        /* Error management registers */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_ERRMGMT);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_ERRMGMT; 


		/* Local slot programming */
		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_LDNSLOTS;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_BCDNSLOTS);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_BCDNSLOTS;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_LUPSLOTS;
    
    
        /* GPIO programming */
		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIOOEN;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIOIEN;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODAT;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATCLR);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODATCLR;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATSET);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_GPIODATSET;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINTEN;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINTINV;

		aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG);
		aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PINCFG;

        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nPRBSRegister.nREG_A2B0_TESTMODE);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_TESTMODE; 
        
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_BECCTL);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_BECCTL; 
#if A2B_ENABLE_AD242X_SUPPORT
        /* Update AD242xConfig */
        nCount  = adi_a2b_SlaveAD242xconfig(pNode, aTwiWriteTable, nCount);
#endif
         /* Preset to write to Bus/slave */
        nA2BI2CAddr = adi_a2b_slavePreset(me, pNode);

        /* Write using table */
        for(nIndex  =0u ; nIndex < nCount ;nIndex++)
        {
            if(nResult !=0u)
            {
                break;
            }
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);

        }
 
    /* Check for failure */       
    if (nResult != 0u)
    {
        eResult = ADI_A2B_FAILURE;
    }
    
#if A2B_PRINT_FOR_DEBUG
    if( nResult == 0u)
    {
        printf(" \n \n Branch %d : Slave Node %d Configuration : SUCCESS\n",((pNode->nID & 0xFF00u) >> 8u) , (pNode->nID & 0xFFu) );
    }
    else
    {
        printf(" \n \n Branch %d : Slave Node %d Configuration : FAILURE\n",((pNode->nID & 0xFF00u) >> 8u) , (pNode->nID & 0xFFu));
    }
#endif 

    return(eResult);
}


/*****************************************************************************/
/*! 
    @brief      This function configures master A2B node for slave discovery

    @param [in] pNode           Pointer to the master A2B node structure
    
    
    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success  
*/ 
/*****************************************************************************/
#if 1

static ADI_A2B_RESULT adi_a2b_masterConfigForDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8 nCount = 0u, nVal = 0u;
    uint8 nIndex;
    uint32 nResult = 0u;
    uint8 pA2BI2CAddr;
    //uint8 nStatusReadCount = 0u;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
     /* Make sure enough memory is allocated */
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[10]; 

    /* Preset to write to master */
    pA2BI2CAddr = adi_a2b_masterPreset(me, pNode);
        

    if(pNode->ePartNum == ADI_A2B_AD2425)
    {
        /* Writing to MSTR bit */
        aTwiWriteTable[nCount].nValue  = (uint8)(BITM_AD242X_CONTROL_MSTR | BITM_A2B_CONTROL_SOFTRST);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL;
    }
    else
    {
        /* Soft reset */
        aTwiWriteTable[nCount].nValue  = (uint8)(BITM_A2B_CONTROL_SOFTRST);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL;
    }



    /* PLL configuration */
    //aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PLLCTL);
     //aTwiWriteTable[nCount].nValue  =(sMasterNode0.sConfigCtrlSettings.nPLLTimeBase | sMasterNode0.sConfigCtrlSettings.nBCLKRate);
    //aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PLLCTL;

    /* Rev1.0 requirement - affects PLL */
    /* Part number check is not required  */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG); 
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SGCFG; 

    /* Writing RESPCYCS */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_RESPCYCS;
    
    /* Apply new structure(shadow -> actual)  */
    /* Enable control*/
    aTwiWriteTable[nCount].nValue  = (uint8)(0x01u);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL; 

    nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, pA2BI2CAddr,(uint8)aTwiWriteTable[0].nAddress ,aTwiWriteTable[0].nValue);

    BSP_BlockingDelayMs(100);

    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,pA2BI2CAddr,(uint8)REG_A2B0_INTTYPE,(uint8*)&nVal);
    if( nVal != 0xFFu)
    {
    	nResult = 0u;
    	pNode->bActive = 1u;
    }

    //aTwiWriteTable[nCount].nValue  = 0x01;
    //aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SWCTL; 

    //aTwiWriteTable[nCount].nValue  = 0x7d;
    //aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_DISCVRY; 

    
    //nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, pA2BI2CAddr,(uint8)aTwiWriteTable[0].nAddress ,aTwiWriteTable[0].nValue);
    /* Check whether master is running */
    /* Adding 25ms delay */


    for(nIndex  = 1u; nIndex < nCount; nIndex++)
    {
        if(nResult != 0u)
        {
            break;
        }
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, pA2BI2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
        //ALWAYS_printf("\n\r I2C WRITE: DEV: %x  REG:%x DAT:%x\n\r", pA2BI2CAddr, (uint8)aTwiWriteTable[nIndex].nAddress, aTwiWriteTable[nIndex].nValue);
        //nResult = (uint32) adi_a2b_I2cWrite(me, pA2BI2CAddr, sizeof(aTwiWriteTable[nIndex]), (uint8*) &aTwiWriteTable[nIndex]);
    }
    /* Adding master node configuration failure */
    pNode->bActive = (uint16)( ~(nResult) & 0x01u );

    
    /* Check for failure */
    if(nResult != 0u )
    {
        adi_a2b_clearPreset(me, pNode);
        eResult = ADI_A2B_FAILURE;
    } 
    BSP_BlockingDelayMs(100);
    return(eResult);
}


#else

static ADI_A2B_RESULT adi_a2b_masterConfigForDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8 nCount = 0u, nVal = 0u;
    uint8 nIndex;
    uint32 nResult = 0u;
    uint8 pA2BI2CAddr;
    uint8 nStatusReadCount = 0u;
    
     /* Make sure enough memory is allocated */
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[10]; 

    /* Preset to write to master */
    pA2BI2CAddr = adi_a2b_masterPreset(me, pNode);
        

    if(pNode->ePartNum == ADI_A2B_AD2425)
    {
        /* Writing to MSTR bit */
        aTwiWriteTable[nCount].nValue  = (uint8)(BITM_AD242X_CONTROL_MSTR | BITM_A2B_CONTROL_SOFTRST);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL;
    }
    else
    {
        /* Soft reset */
        aTwiWriteTable[nCount].nValue  = (uint8)(BITM_A2B_CONTROL_SOFTRST);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL;
    }
    /* PLL configuration */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PLLCTL);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_PLLCTL;

    /* Rev1.0 requirement - affects PLL */
    /* Part number check is not required  */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG); 
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_I2SGCFG; 

    /* Writing RESPCYCS */
    aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_RESPCYCS;
    
    /* Apply new structure(shadow -> actual)  */
    /* Enable control*/
    aTwiWriteTable[nCount].nValue  = (uint8)(0x01u);
    aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL; 
    
    //nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, pA2BI2CAddr,(uint8)aTwiWriteTable[0].nAddress ,aTwiWriteTable[0].nValue);
	nResult =  (uint32) adi_a2b_I2cWrite(me, pA2BI2CAddr, sizeof(aTwiWriteTable[0]), (uint8*) &aTwiWriteTable[0]);
    /* Check whether master is running */
#if 0
    do
    {
    	nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,pA2BI2CAddr,(uint8)REG_A2B0_INTSTAT,(uint8*)&nVal);
    	if(nVal == 1)
    	{
    		break;
    	}
    	nStatusReadCount++;
    }  while(nStatusReadCount  < 100);
#else
    /* Adding 25ms delay */
    //adi_a2b_Delay(25u);
	BSP_BlockingDelayMs(50);
#endif
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,pA2BI2CAddr,(uint8)REG_A2B0_INTTYPE,(uint8*)&nVal);

    if( nVal != 0xFFu)
    {
    	nResult = 0u;
    	pNode->bActive = 1u;
    }
	BSP_BlockingDelayMs(100);

    for(nIndex  = 1u; nIndex < nCount; nIndex++)
    {
        if(nResult != 0u)
        {
            break;
        }
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, pA2BI2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
        //nResult = (uint32) adi_a2b_I2cWrite(me, pA2BI2CAddr, sizeof(aTwiWriteTable[nIndex]), (uint8*) &aTwiWriteTable[nIndex]);
    }
    /* Adding master node configuration failure */
    pNode->bActive = (uint16)( ~(nResult) & 0x01u );

    
    /* Check for failure */
    if(nResult != 0u )
    {
        adi_a2b_clearPreset(me, pNode);
        eResult = ADI_A2B_FAILURE;
    } 
    
    return(eResult);
}
#endif



/********************************************************************************/
/*!
@brief      This function parses master node configuration data to A2B node structure

@param [in] pFrameWorkHandle    Framework configuration pointer


@return    void 

*/
/***********************************************************************************/
static void adi_a2b_ParseMasterNCD(const ADI_A2B_MASTER_NCD* pMstCfg,ADI_A2B_NODE* pNode,ADI_A2B_COMMON_CONFIG* pCommon)
{
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    /* Assign ID registers */
    //pNode->oProperties.nIDRegister.nREG_A2B0_CAPABILITY     = pMstCfg->sAuthSettings.nCapability;
    //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT        = pMstCfg->sAuthSettings.nProductID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR         = pMstCfg->sAuthSettings.nVendorID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION        = pMstCfg->sAuthSettings.nVersionID;

    pNode->oProperties.nMasterI2CAddress                    =  pCommon->nMasterI2CAddr;
    pNode->oProperties.nBusI2CAddress                       =  pCommon->nBusI2CAddr;

    /* Master node properties  */
    pNode->eType             = ADI_A2B_MASTER;
    pNode->nID               = pMstCfg->nNodeID;
    pNode->nA2BSrcNodeID     = pMstCfg->nSrcNodeID;
    pNode->ePartNum		     = pMstCfg->ePartNum;
    pNode->bActive           = 0u;

    /* Control registers */
    //pNode->oProperties.nCTLRegister.nREG_A2B0_NODEADR       = 0u;
    pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL         = pMstCfg->sRegSettings.nSWCTL;
    //pNode->oProperties.nCTLRegister.nREG_A2B0_BCDNSLOTS     = 0u;
    //pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS      = 0u;
    //pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS      = 0u;
    pNode->oProperties.nCTLRegister.nREG_A2B0_DNSLOTS       = pMstCfg->sConfigCtrlSettings.nPassDwnSlots;
    pNode->oProperties.nCTLRegister.nREG_A2B0_UPSLOTS       = pMstCfg->sConfigCtrlSettings.nPassUpSlots;
    pNode->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS      = pMstCfg->sConfigCtrlSettings.nRespCycle;

    pNode->oProperties.nCTLRegister.nREG_A2B0_SLOTFMT       = ( pCommon->nDwnSlotSize | pCommon->nUpSlotSize | \
                                                              ( pCommon->bDwnstreamCompression << (uint8)BITP_A2B_SLOTFMT_DNFP) | \
                                                              ( pCommon->bUpstreamCompression << (uint8)BITP_A2B_SLOTFMT_UPFP) );

    pNode->oProperties.nCTLRegister.nREG_A2B0_DATCTL        = ( pCommon->bEnableUpstream << BITP_A2B_DATCTL_UPS) | \
                                                              ( pCommon->bEnableDwnstream << BITP_A2B_DATCTL_DNS);

    //pNode->oProperties.nCTLRegister.nREG_A2B0_CONTROL       = 0u;
    //pNode->oProperties.nCTLRegister.nREG_A2B0_DISCVRY       = 0u;

    /* I2S & PDM registers */
    //pNode->oProperties.nI2CI2SRegister.nREG_A2B0_CHIP       = 0u;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2CCFG     = (uint16)(pMstCfg->sConfigCtrlSettings.bI2CEarlyAck << (uint8)BITP_A2B_I2CCFG_EACK);
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PLLCTL     = (pMstCfg->sConfigCtrlSettings.nPLLTimeBase | pMstCfg->sConfigCtrlSettings.nBCLKRate);

    //pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG    = ( pMstCfg->sI2SSettings.bEarlySync << (uint8)BITP_A2B_I2SGCFG_EARLY ) | \
    //                                                          ( pMstCfg->sI2SSettings.nTDMMode | pMstCfg->sI2SSettings.nTDMChSize ) | \
    //                                                          ( pMstCfg->sI2SSettings.nSyncMode | pMstCfg->sI2SSettings.nSyncPolarity << BITP_A2B_I2SGCFG_INV ) ;

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG    = 0x24;


    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG     = ( pMstCfg->sI2SSettings.bRXInterleave << (uint8)BITP_A2B_I2SCFG_RX2PINTL ) | \
                                                              ( pMstCfg->sI2SSettings.bTXInterleave << (uint8)BITP_A2B_I2SCFG_TX2PINTL ) | \
                                                              ( pMstCfg->sI2SSettings.nBclkRxPolarity << (uint8)BITP_A2B_I2SCFG_RXBCLKINV )| \
                                                              ( pMstCfg->sI2SSettings.nBclkTxPolarity << (uint8)BITP_A2B_I2SCFG_TXBCLKINV);

    //pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRATE    = 0u;

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2STXOFFSET= (uint16)( pMstCfg->sI2SSettings.nTxOffset | pMstCfg->sI2SSettings.bTriStateAfterTx <<(uint8)BITP_A2B_I2STXOFFSET_TSAFTER) | \
                                                              ( pMstCfg->sI2SSettings.bTriStateBeforeTx << BITP_A2B_I2STXOFFSET_TSBEFORE);

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRXOFFSET=  pMstCfg->sI2SSettings.nRxOffset;
    //pNode->oProperties.nI2CI2SRegister.nREG_A2B0_SYNCOFFSET = 0u;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL     =  pMstCfg->sRegSettings.nPDMCTL;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_ERRMGMT    =  pMstCfg->sRegSettings.nERRMGMT;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2STEST    =  pMstCfg->sRegSettings.nI2STEST;

    /* INT registers */
    pNode->oProperties.nINTRegister.nREG_A2B0_BECCTL         = pMstCfg->sRegSettings.nBECCTL;
    //pNode->oProperties.nINTRegister.nREG_A2B0_BECNT          = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTSTAT        = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTSRC         = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTTYPE        = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTPND0        = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTPND1        = 0u;
    //pNode->oProperties.nINTRegister.nREG_A2B0_INTPND2        = 0u;

    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0        =  ( pMstCfg->sInterruptSettings.bReportHDCNTErr         << (uint8)BITP_A2B_INTPND0_HDCNTERR) | \
                                                                ( pMstCfg->sInterruptSettings.bReportDDErr             << (uint8)BITP_A2B_INTPND0_DDERR) | \
                                                                ( pMstCfg->sInterruptSettings.bReportCRCErr         << (uint8)BITP_A2B_INTPND0_CRCERR) | \
                                                                ( pMstCfg->sInterruptSettings.bReportDataParityErr     << (uint8)BITP_A2B_INTPND0_DPERR) | \
                                                                ( pMstCfg->sInterruptSettings.bReportPwrErr         << (uint8)BITP_A2B_INTPND0_PWRERR ) | \
                                                                ( pMstCfg->sInterruptSettings.bReportErrCntOverFlow << (uint8)BITP_A2B_INTPND0_BECOVF )| \
                                                                ( pMstCfg->sInterruptSettings.bReportSRFMissErr << (uint8)BITP_A2B_INTPND0_SRFERR );


    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1        =
                                                                ( pMstCfg->sInterruptSettings.bReportGPIO3 << (uint8)BITP_A2B_INTPND1_IO3PND ) | \
                                                                ( pMstCfg->sInterruptSettings.bReportGPIO4 << (uint8)BITP_A2B_INTPND1_IO4PND ) | \
                                                                ( pMstCfg->sInterruptSettings.bReportGPIO5 << (uint8)BITP_A2B_INTPND1_IO5PND ) | \
                                                                ( pMstCfg->sInterruptSettings.bReportGPIO6 << (uint8)BITP_A2B_INTPND1_IO6PND);


    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK2        = ( pMstCfg->sInterruptSettings.bSlaveIntReq  << (uint8)BITP_A2B_INTPND2_SLVIRQ ) | \
                                                               ( pMstCfg->sInterruptSettings.bReportI2CErr << (uint8)BITP_A2B_INTPND2_I2CERR ) | \
                                                               ( pMstCfg->sInterruptSettings.bDiscComplete << (uint8)BITP_A2B_INTPND2_DSCDONE ) | \
                                                               ( pMstCfg->sInterruptSettings.bIntFrameCRCErr << (uint8)BITP_A2B_INTPND2_ICRCERR );


    /* Parse pin multiplex - 3 and 4 */
    adi_a2b_ParseMasterPinMux34( pNode ,  pMstCfg);
    /* Parse pin multiplex - 5 and 6 */
    adi_a2b_ParseMasterPinMux56( pNode ,  pMstCfg);
                                                               
                                                               
    /* Pin I/O registers */
#if  A2B_ENABLE_AD241X_SUPPORT
    //pNode->oProperties.nPINIORegister.nREG_A2B0_CLKCFG        = 0u;
#endif
    //pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATSET    = 0u;
    //pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATCLR    = 0u;
    pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG        = (pMstCfg->sGPIOSettings.bHighDriveStrength << (uint8)BITP_A2B_PINCFG_DRVSTR );


    /* Test mode */
    pNode->oProperties.nPRBSRegister.nREG_A2B0_TESTMODE       = pMstCfg->sRegSettings.nTESTMODE;
    //pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT0        = 0u;
    //pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT1        = 0u;
    //pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT2        = 0u;
    //pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT3        = 0u;

    /* Status */
    //pNode->oProperties.nSTATRegister.nREG_A2B0_SWSTAT         = 0u;
    //pNode->oProperties.nSTATRegister.nREG_A2B0_NODE           = 0u;
    //pNode->oProperties.nSTATRegister.nREG_A2B0_DISCSTAT       = 0u;

}

/********************************************************************************/
/*!
@brief      This function parses slave node configuration Data to A2B NODE structure pertaining to AD241x

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@param [in] pFrameworkHandle    Framework configuration pointer

@return     ADI_A2B_RESULT
            - ADI_A2B_FAILURE : Failure(If any of the peripheral initialization fails)
            - ADI_A2B_SUCEESS : Success

*/
/***********************************************************************************/
static void adi_a2b_ParseSlaveNCD(const ADI_A2B_SLAVE_NCD* pSlvCfg,ADI_A2B_NODE* pNode, ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle)
{
    //uint8 nIndex2 = 0u;
    //uint8 nPositionID;
    //uint8 nBranchID;
    //uint8 nNumConfig;
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    
    /* Assign ID registers */
    //pNode->oProperties.nIDRegister.nREG_A2B0_CAPABILITY      = pSlvCfg->sAuthSettings.nCapability;
    //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT         = pSlvCfg->sAuthSettings.nProductID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR          = pSlvCfg->sAuthSettings.nVendorID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION         = pSlvCfg->sAuthSettings.nVersionID;

    /* Master node properties  */
    pNode->eType = ADI_A2B_SLAVE;
    pNode->nID = pSlvCfg->nNodeID;
    pNode->nA2BSrcNodeID = pSlvCfg->nSrcNodeID;
    pNode->ePartNum		 = pSlvCfg->ePartNum;
    pNode->bActive = 0u;
    pNode->bEnableAutoConfig = pSlvCfg->bEnableAutoConfig;
    

    /* Control registers */
    pNode->oProperties.nCTLRegister.nREG_A2B0_NODEADR       = 0u;
    pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL         = pSlvCfg->sRegSettings.nSWCTL;
    pNode->oProperties.nCTLRegister.nREG_A2B0_BCDNSLOTS     = pSlvCfg->sConfigCtrlSettings.nBroadCastSlots;
    pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS      = pSlvCfg->sConfigCtrlSettings.nLocalDwnSlotsConsume;
    pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS      = pSlvCfg->sConfigCtrlSettings.nLocalUpSlotsContribute;
    pNode->oProperties.nCTLRegister.nREG_A2B0_DNSLOTS       = pSlvCfg->sConfigCtrlSettings.nPassDwnSlots;
    pNode->oProperties.nCTLRegister.nREG_A2B0_UPSLOTS       = pSlvCfg->sConfigCtrlSettings.nPassUpSlots;
    pNode->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS      = pSlvCfg->sConfigCtrlSettings.nRespCycle;

    pNode->oProperties.nCTLRegister.nREG_A2B0_SLOTFMT       = 0u;

       pNode->oProperties.nCTLRegister.nREG_A2B0_DATCTL     = 0u;
    pNode->oProperties.nCTLRegister.nREG_A2B0_CONTROL       = 0u;
    pNode->oProperties.nCTLRegister.nREG_A2B0_DISCVRY       = 0u;

    /* I2S & PDM registers */
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_CHIP       = 0u;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2CCFG     = (uint16)(pSlvCfg->sConfigCtrlSettings.nI2CFrequency) | \
                                                                    (pSlvCfg->sConfigCtrlSettings.nSuperFrameRate);

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PLLCTL     = pSlvCfg->sRegSettings.nPLLCTL;

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SGCFG    = ( pSlvCfg->sI2SSettings.bEarlySync << (uint8)BITP_A2B_I2SGCFG_EARLY ) | \
                                                              ( pSlvCfg->sI2SSettings.nTDMMode | pSlvCfg->sI2SSettings.nTDMChSize ) | \
                                                              ( pSlvCfg->sI2SSettings.nSyncMode | pSlvCfg->sI2SSettings.nSyncPolarity << BITP_A2B_I2SGCFG_INV ) ;

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG     = ( pSlvCfg->sI2SSettings.bRXInterleave << (uint8)BITP_A2B_I2SCFG_RX2PINTL ) | \
                                                              ( pSlvCfg->sI2SSettings.bTXInterleave << (uint8)BITP_A2B_I2SCFG_TX2PINTL ) | \
                                                              ( pSlvCfg->sI2SSettings.nBclkRxPolarity << (uint8)BITP_A2B_I2SCFG_RXBCLKINV )| \
                                                              ( pSlvCfg->sI2SSettings.nBclkTxPolarity << (uint8)BITP_A2B_I2SCFG_TXBCLKINV);

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRATE    = ( pSlvCfg->sI2SSettings.sI2SRateConfig.bReduce << (uint8)BITP_A2B_I2SRATE_REDUCE )| \
                                                               (pSlvCfg->sI2SSettings.sI2SRateConfig.nSamplingRate);

    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2STXOFFSET=  0u;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRXOFFSET=  0u;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_SYNCOFFSET =  pSlvCfg->sI2SSettings.nSyncOffset;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL     |=  ( pSlvCfg->sPDMSettings.bHPFUse << (uint8)(BITP_A2B_PDMCTL_HPFEN) ) | \
                                                               ( pSlvCfg->sPDMSettings.nHPFCutOff  | pSlvCfg->sPDMSettings.nNumSlotsPDM0 | pSlvCfg->sPDMSettings.nNumSlotsPDM1);
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_ERRMGMT    =  pSlvCfg->sRegSettings.nERRMGMT;
    pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2STEST    =  pSlvCfg->sRegSettings.nI2STEST;

    /* INT registers */
    pNode->oProperties.nINTRegister.nREG_A2B0_BECCTL         = pSlvCfg->sRegSettings.nBECCTL;
    pNode->oProperties.nINTRegister.nREG_A2B0_BECNT          = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTSTAT        = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTSRC         = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTTYPE        = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTPND0        = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTPND1        = 0u;
    pNode->oProperties.nINTRegister.nREG_A2B0_INTPND2        = 0u;

    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0        =  ( pSlvCfg->sInterruptSettings.bReportHDCNTErr         << (uint8)BITP_A2B_INTPND0_HDCNTERR) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportDDErr             << (uint8)BITP_A2B_INTPND0_DDERR) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportCRCErr         << (uint8)BITP_A2B_INTPND0_CRCERR) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportDataParityErr     << (uint8)BITP_A2B_INTPND0_DPERR) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportPwrErr         << (uint8)BITP_A2B_INTPND0_PWRERR ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportErrCntOverFlow << (uint8)BITP_A2B_INTPND0_BECOVF ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportSRFMissErr << (uint8)BITP_A2B_INTPND0_SRFERR );


    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1        =
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO0 << (uint8)BITP_A2B_INTPND1_IO0PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO1 << (uint8)BITP_A2B_INTPND1_IO1PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO2 << (uint8)BITP_A2B_INTPND1_IO2PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO3 << (uint8)BITP_A2B_INTPND1_IO3PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO4 << (uint8)BITP_A2B_INTPND1_IO4PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO5 << (uint8)BITP_A2B_INTPND1_IO5PND ) | \
                                                                ( pSlvCfg->sInterruptSettings.bReportGPIO6 << (uint8)BITP_A2B_INTPND1_IO6PND);


    pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK2        = 0u;

    pNode->oProperties.nINTRegister.nREG_A2B0_RAISE          = pSlvCfg->sRegSettings.nRAISE ;
    pNode->oProperties.nINTRegister.nREG_A2B0_GENERR         = pSlvCfg->sRegSettings.nGENERR ;


    /* Parsing Pin Mux */ 
    adi_a2b_ParseSlavePinMux012(pNode ,  pSlvCfg);
    adi_a2b_ParseSlavePinMux34( pNode ,  pSlvCfg);
    adi_a2b_ParseSlavePinMux56( pNode ,  pSlvCfg);

   
    /* Pin I/O registers */
#if  A2B_ENABLE_AD241X_SUPPORT
    pNode->oProperties.nPINIORegister.nREG_A2B0_CLKCFG        |= (pSlvCfg->sI2SSettings.nCodecClkRate << BITP_A2B_CLKCFG_CCLKRATE);
#endif
    pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATSET    = 0u;
    pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODATCLR    = 0u;
    pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG        = (pSlvCfg->sGPIOSettings.bHighDriveStrength << (uint8)BITP_A2B_PINCFG_DRVSTR );

    /* Test mode */
    pNode->oProperties.nPRBSRegister.nREG_A2B0_TESTMODE       = pSlvCfg->sRegSettings.nTESTMODE;
    pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT0        = 0u;
    pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT1        = 0u;
    pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT2        = 0u;
    pNode->oProperties.nPRBSRegister.nREG_A2B0_ERRCNT3        = 0u;

    /* Status */
    pNode->oProperties.nSTATRegister.nREG_A2B0_SWSTAT        = 0u;
    pNode->oProperties.nSTATRegister.nREG_A2B0_NODE          = 0u;
    pNode->oProperties.nSTATRegister.nREG_A2B0_DISCSTAT      = 0u;

    //nPositionID = (uint8)(pSlvCfg->nNodeID & 0xFFu) + 1u;
    //nBranchID =  (uint8)( (pSlvCfg->nNodeID & 0xFF00u) >> 8u);

#if 0
    /* Peripheral update */
    for(nIndex2 = 0u; nIndex2 < pSlvCfg->nNumPeriDevice; nIndex2++)
    {
        if(pSlvCfg->apPeriConfig[nIndex2]->paPeriConfigUnit != NULL)
        {
            pNode->oConnectedTo[nIndex2].bActive       = 0u;
            pNode->oConnectedTo[nIndex2].nI2CAddress = pSlvCfg->apPeriConfig[nIndex2]->nI2Caddr;
            pNode->oConnectedTo[nIndex2].eDeviceType = (uint16)pSlvCfg->apPeriConfig[nIndex2]->eDeviceType;
            pNode->oConnectedTo[nIndex2].eDeviceID   = (uint16)GENERIC;

            /* Number of config */
            nNumConfig = pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].nNumConfig;
            pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].aDeviceConfig[nNumConfig].nConnectedNodeID         = pSlvCfg->nNodeID;
            pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].aDeviceConfig[nNumConfig].nDeviceAddress           = pSlvCfg->apPeriConfig[nIndex2]->nI2Caddr;
            pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].aDeviceConfig[nNumConfig].nNumPeriConfigUnit       = pSlvCfg->apPeriConfig[nIndex2]->nNumPeriConfigUnit;
            pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].aDeviceConfig[nNumConfig].paPeriConfigUnit         = pSlvCfg->apPeriConfig[nIndex2]->paPeriConfigUnit;
            pFrameworkHandle->aPeriDownloadTable[nBranchID][nPositionID].nNumConfig++;
        }
        else
        {
            pNode->oConnectedTo[nIndex2].bActive        = 0u;
            pNode->oConnectedTo[nIndex2].nI2CAddress    = 0xFFu;
            pNode->oConnectedTo[nIndex2].eDeviceType    = (uint16)(uint16)pSlvCfg->apPeriConfig[nIndex2]->eDeviceType;
            pNode->oConnectedTo[nIndex2].eDeviceID      = (uint16)UNKNOWN;
        }

    }

    for(nIndex2 = pSlvCfg->nNumPeriDevice; nIndex2 < (uint8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++  )
    {
        pNode->oConnectedTo[nIndex2].bActive        = 0u;
        pNode->oConnectedTo[nIndex2].nI2CAddress    = 0xFFu;
        pNode->oConnectedTo[nIndex2].eDeviceType    = (uint16)ADI_A2B_AUDIO_UNKNOWN;
        pNode->oConnectedTo[nIndex2].eDeviceID      = (uint16)UNKNOWN;
    }    
#endif    
    
}

static void adi_a2b_ParseMasterNCDForAD242x(const ADI_A2B_MASTER_NCD* pMstCfg,ADI_A2B_NODE* pNode,ADI_A2B_COMMON_CONFIG* pCommon)
{

    uint8 *psGPIODMask[8];
    A2B_GPIOD_PIN_CONFIG asGPIODConfig[8];
    uint8 nGPIOIndex;
    /* Assign ID registers */
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);

    /* GPIOD  */
	 pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODINV =   (pMstCfg->sGPIODSettings.sGPIOD1Config.bGPIOSignalInv << 1u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD2Config.bGPIOSignalInv << 2u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD3Config.bGPIOSignalInv << 3u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD4Config.bGPIOSignalInv << 4u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD5Config.bGPIOSignalInv << 5u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD6Config.bGPIOSignalInv << 6u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD7Config.bGPIOSignalInv << 7u);
	 pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODEN  =   (pMstCfg->sGPIODSettings.sGPIOD1Config.bGPIODistance << 1u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD2Config.bGPIODistance << 2u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD3Config.bGPIODistance << 3u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD4Config.bGPIODistance << 4u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD5Config.bGPIODistance << 5u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD6Config.bGPIODistance << 6u) |\
																 (pMstCfg->sGPIODSettings.sGPIOD7Config.bGPIODistance << 7u);

	/* GPIOD Mask update */
	psGPIODMask[0] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD0MSK;
	psGPIODMask[1] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD1MSK;
	psGPIODMask[2] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD2MSK;
	psGPIODMask[3] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD3MSK;
	psGPIODMask[4] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD4MSK;
	psGPIODMask[5] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD5MSK;
	psGPIODMask[6] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD6MSK;
	psGPIODMask[7] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD7MSK;

	asGPIODConfig[1] = pMstCfg->sGPIODSettings.sGPIOD1Config;
	asGPIODConfig[2] = pMstCfg->sGPIODSettings.sGPIOD2Config;
	asGPIODConfig[3] = pMstCfg->sGPIODSettings.sGPIOD3Config;
	asGPIODConfig[4] = pMstCfg->sGPIODSettings.sGPIOD4Config;
	asGPIODConfig[5] = pMstCfg->sGPIODSettings.sGPIOD5Config;
	asGPIODConfig[6] = pMstCfg->sGPIODSettings.sGPIOD6Config;
	asGPIODConfig[7] = pMstCfg->sGPIODSettings.sGPIOD7Config;

	for(nGPIOIndex = 1u; nGPIOIndex < 8u; nGPIOIndex++)
	{


		*(psGPIODMask[nGPIOIndex]) =  (uint16) ( (asGPIODConfig[nGPIOIndex].abBusPortMask[0] << 0u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[1] << 1u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[2] << 2u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[3] << 3u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[4] << 4u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[5] << 5u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[6] << 6u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[7] << 7u));
	}





        /* I2S & PDM registers */
		pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRATE = (pCommon->bEnableReduceRate << (uint8)BITP_AD242X_I2SRRATE_RBUS ) |\
															       (pCommon->nSysRateDivFactor << (uint8)BITP_AD242X_I2SRRATE_RRDIV);

		pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRCTL =( pMstCfg->sI2SSettings.sI2SRateConfig.bRRStrobe << (uint8)BITP_AD242X_I2SRRCTL_ENSTRB )|\
															   ( pMstCfg->sI2SSettings.sI2SRateConfig.bRRStrobeDirection << (uint8)BITP_AD242X_I2SRRCTL_STRBDIR )|\
															   ( pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraBit << (uint8)BITP_AD242X_I2SRRCTL_ENXBIT )|\
															   ( pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitLSB << (uint8)BITP_AD242X_I2SRRCTL_ENVLSB )|\
															   ( pMstCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraCh << (uint8)BITP_AD242X_I2SRRCTL_ENCHAN );


		/* INT registers */
		pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1        =  ( pMstCfg->sInterruptSettings.bReportGPIO1 << (uint8)BITP_A2B_INTPND1_IO1PND ) | \
																	( pMstCfg->sInterruptSettings.bReportGPIO2 << (uint8)BITP_A2B_INTPND1_IO2PND ) | \
																	( pMstCfg->sInterruptSettings.bReportGPIO3 << (uint8)BITP_A2B_INTPND1_IO3PND ) | \
																	( pMstCfg->sInterruptSettings.bReportGPIO4 << (uint8)BITP_A2B_INTPND1_IO4PND ) | \
																	( pMstCfg->sInterruptSettings.bReportGPIO5 << (uint8)BITP_A2B_INTPND1_IO5PND ) | \
																	( pMstCfg->sInterruptSettings.bReportGPIO6 << (uint8)BITP_A2B_INTPND1_IO6PND ) |\
																	( pMstCfg->sInterruptSettings.bReportGPIO7 << (uint8)BITP_AD242X_INTPND1_IO7PND );

		pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG   = (pMstCfg->sGPIOSettings.bHighDriveStrength << (uint8)BITP_A2B_PINCFG_DRVSTR ) |\
														       (pMstCfg->sGPIOSettings.bIRQInv << (uint8)BITP_AD242X_PINCFG_IRQINV) |\
														       (pMstCfg->sGPIOSettings.bIRQTriState << (uint8)BITP_AD242X_PINCFG_IRQTS);


		/*ClockCFg1 & CFG2*/
		pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG |= (pMstCfg->sClkOutSettings.bClk1Div << (uint8)BITP_AD242X_CLK1CFG_CLK1DIV) |\
														          (pMstCfg->sClkOutSettings.bClk1PreDiv << (uint8)BITP_AD242X_CLK1CFG_CLK1PDIV) |\
														          (pMstCfg->sClkOutSettings.bClk1Inv << (uint8)BITP_AD242X_CLK1CFG_CLK1INV);

		pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG |= (pMstCfg->sClkOutSettings.bClk2Div << (uint8)BITP_AD242X_CLK2CFG_CLK2DIV) |\
														   (pMstCfg->sClkOutSettings.bClk2PreDiv << (uint8)BITP_AD242X_CLK2CFG_CLK2PDIV) |\
														   (pMstCfg->sClkOutSettings.bClk2Inv << (uint8)BITP_AD242X_CLK2CFG_CLK2INV);

     /* Pin Multiplex  */
     adi_a2b_ParseMasterPinMux12(pNode, pMstCfg);

     /* Pin multiplex for GPIO 7*/
     adi_a2b_ParseMasterPinMux7(pNode, pMstCfg);

    /* Register */
     pNode->oProperties.nCTLRegister.nREG_AD242X0_BMMCFG = pMstCfg->sRegSettings.nBMMCFG;

}

/********************************************************************************/
/*!
@brief      This function parses slave node configuration Data to A2B NODE structure pertaining to AD242x

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@param [in] pFrameworkHandle    Framework configuration pointer

@return     ADI_A2B_RESULT
            - ADI_A2B_FAILURE : Failure(If any of the peripheral initialization fails)
            - ADI_A2B_SUCEESS : Success

*/
/***********************************************************************************/
static void adi_a2b_ParseSlaveNCDForAD242x(const ADI_A2B_SLAVE_NCD* pSlvCfg,ADI_A2B_NODE* pNode, ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle)
{
    //uint8 nIndex2 = 0u;
    //uint8 nPositionID;
    //uint8 nBranchID;
    //uint8 nNumConfig;
    uint8 *psGPIODMask[8];
    A2B_GPIOD_PIN_CONFIG asGPIODConfig[8];
    uint8 nGPIOIndex;
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);

	/* Assign ID registers */

	/* Slave node properties  */

	/* Control registers */
	pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL         = pSlvCfg->sRegSettings.nSWCTL;

	pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS      = pSlvCfg->sConfigCtrlSettings.nLocalUpSlotsContribute;
	pNode->oProperties.nCTLRegister.nREG_A2B0_DNSLOTS       = pSlvCfg->sConfigCtrlSettings.nPassDwnSlots;
	pNode->oProperties.nCTLRegister.nREG_A2B0_UPSLOTS       = pSlvCfg->sConfigCtrlSettings.nPassUpSlots;

	if(pSlvCfg->sConfigCtrlSettings.bUseDwnslotConsumeMasks == 1u)
	{
		pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS = ((uint8)BITM_AD242X_LDNSLOTS_DNMASKEN | pSlvCfg->sConfigCtrlSettings.nSlotsforDwnstrmContribute);
#if 1
#else
		pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK0 = ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[0] << BITP_AD242X_DNMASK0_RXDNSLOT00)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[1] << BITP_AD242X_DNMASK0_RXDNSLOT01 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[2] << BITP_AD242X_DNMASK0_RXDNSLOT02)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[3] << BITP_AD242X_DNMASK0_RXDNSLOT03 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[4] << BITP_AD242X_DNMASK0_RXDNSLOT04)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[5] << BITP_AD242X_DNMASK0_RXDNSLOT05 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[6] << BITP_AD242X_DNMASK0_RXDNSLOT06)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[7] << BITP_AD242X_DNMASK0_RXDNSLOT07 );

		pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK1 = ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[8] << BITP_AD242X_DNMASK0_RXDNSLOT00)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[9] << BITP_AD242X_DNMASK0_RXDNSLOT01 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[10] << BITP_AD242X_DNMASK0_RXDNSLOT02)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[11] << BITP_AD242X_DNMASK0_RXDNSLOT03 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[12] << BITP_AD242X_DNMASK0_RXDNSLOT04)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[13] << BITP_AD242X_DNMASK0_RXDNSLOT05 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[14] << BITP_AD242X_DNMASK0_RXDNSLOT06)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[15] << BITP_AD242X_DNMASK0_RXDNSLOT07 );

		pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK2 = ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[16] << BITP_AD242X_DNMASK0_RXDNSLOT00)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[17] << BITP_AD242X_DNMASK0_RXDNSLOT01 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[18] << BITP_AD242X_DNMASK0_RXDNSLOT02)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[19] << BITP_AD242X_DNMASK0_RXDNSLOT03 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[20] << BITP_AD242X_DNMASK0_RXDNSLOT04)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[21] << BITP_AD242X_DNMASK0_RXDNSLOT05 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[22] << BITP_AD242X_DNMASK0_RXDNSLOT06)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[23] << BITP_AD242X_DNMASK0_RXDNSLOT07 );

		pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK3 = ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[24] << BITP_AD242X_DNMASK0_RXDNSLOT00)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[25] << BITP_AD242X_DNMASK0_RXDNSLOT01 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[26] << BITP_AD242X_DNMASK0_RXDNSLOT02)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[27] << BITP_AD242X_DNMASK0_RXDNSLOT03 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[28] << BITP_AD242X_DNMASK0_RXDNSLOT04)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[29] << BITP_AD242X_DNMASK0_RXDNSLOT05 )|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[30] << BITP_AD242X_DNMASK0_RXDNSLOT06)|\
															   ((uint8)pSlvCfg->sConfigCtrlSettings.anDwnstreamConsumeSlots[31] << BITP_AD242X_DNMASK0_RXDNSLOT07 );

#endif
	}
	else
	{
		pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS      = pSlvCfg->sConfigCtrlSettings.nLocalDwnSlotsConsume;
		//pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK0    = 0u;
		//pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK1    = 0u;
		//pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK2    = 0u;
		//pNode->oProperties.nCTLRegister.nREG_AD242X0_DNMASK3    = 0u;
	}

	pNode->oProperties.nCTLRegister.nREG_AD242X0_DNOFFSET = pSlvCfg->sConfigCtrlSettings.nOffsetDwnstrmContribute;
	pNode->oProperties.nCTLRegister.nREG_AD242X0_UPOFFSET = pSlvCfg->sConfigCtrlSettings.nOffsetUpstrmContribute;

#if 0
	pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK0 = ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[0] << BITP_AD242X_UPMASK0_RXUPSLOT00)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[1] << BITP_AD242X_UPMASK0_RXUPSLOT01 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[2] << BITP_AD242X_UPMASK0_RXUPSLOT02)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[3] << BITP_AD242X_UPMASK0_RXUPSLOT03 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[4] << BITP_AD242X_UPMASK0_RXUPSLOT04)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[5] << BITP_AD242X_UPMASK0_RXUPSLOT05 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[6] << BITP_AD242X_UPMASK0_RXUPSLOT06)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[7] << BITP_AD242X_UPMASK0_RXUPSLOT07 );


	pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK1 = ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[8] << BITP_AD242X_UPMASK0_RXUPSLOT00)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[9] << BITP_AD242X_UPMASK0_RXUPSLOT01 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[10] << BITP_AD242X_UPMASK0_RXUPSLOT02)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[11] << BITP_AD242X_UPMASK0_RXUPSLOT03 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[12] << BITP_AD242X_UPMASK0_RXUPSLOT04)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[13] << BITP_AD242X_UPMASK0_RXUPSLOT05 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[14] << BITP_AD242X_UPMASK0_RXUPSLOT06)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[15] << BITP_AD242X_UPMASK0_RXUPSLOT07 );

	pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK2 = ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[16] << BITP_AD242X_UPMASK0_RXUPSLOT00)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[17] << BITP_AD242X_UPMASK0_RXUPSLOT01 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[18] << BITP_AD242X_UPMASK0_RXUPSLOT02)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[19] << BITP_AD242X_UPMASK0_RXUPSLOT03 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[20] << BITP_AD242X_UPMASK0_RXUPSLOT04)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[21] << BITP_AD242X_UPMASK0_RXUPSLOT05 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[22] << BITP_AD242X_UPMASK0_RXUPSLOT06)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[23] << BITP_AD242X_UPMASK0_RXUPSLOT07 );

	pNode->oProperties.nCTLRegister.nREG_AD242X0_UPMASK3 = ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[24] << BITP_AD242X_UPMASK0_RXUPSLOT00)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[25] << BITP_AD242X_UPMASK0_RXUPSLOT01 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[26] << BITP_AD242X_UPMASK0_RXUPSLOT02)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[27] << BITP_AD242X_UPMASK0_RXUPSLOT03 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[28] << BITP_AD242X_UPMASK0_RXUPSLOT04)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[29] << BITP_AD242X_UPMASK0_RXUPSLOT05 )|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[30] << BITP_AD242X_UPMASK0_RXUPSLOT06)|\
														   ((uint8)pSlvCfg->sConfigCtrlSettings.anUpstreamConsumeSlots[31] << BITP_AD242X_UPMASK0_RXUPSLOT07 );
#endif



	/* I2S & PDM registers */
	pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRCTL = ( pSlvCfg->sI2SSettings.sI2SRateConfig.bRRStrobe << (uint8)BITP_AD242X_I2SRRCTL_ENSTRB )|\
															   ( pSlvCfg->sI2SSettings.sI2SRateConfig.bRRStrobeDirection << (uint8)BITP_AD242X_I2SRRCTL_STRBDIR )|\
															   ( pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraBit << (uint8)BITP_AD242X_I2SRRCTL_ENXBIT )|\
															   ( pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitLSB << (uint8)BITP_AD242X_I2SRRCTL_ENVLSB )|\
															   ( pSlvCfg->sI2SSettings.sI2SRateConfig.bRRValidBitExtraCh << (uint8)BITP_AD242X_I2SRRCTL_ENCHAN );

	pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRSOFFS = ( pSlvCfg->sI2SSettings.sI2SRateConfig.nRROffset << (uint8)BITP_AD242X_I2SRRSOFFS_RRSOFFSET);

	pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SRATE    =  (pSlvCfg->sI2SSettings.sI2SRateConfig.bReduce << (uint8)BITP_A2B_I2SRATE_REDUCE )| \
															   (pSlvCfg->sI2SSettings.sI2SRateConfig.nSamplingRate)| \
															   (pSlvCfg->sI2SSettings.sI2SRateConfig.nRBCLKRate << (uint8)BITP_AD242X_I2SRATE_BCLKRATE ) |\
															   (pSlvCfg->sI2SSettings.sI2SRateConfig.bShareBusSlot << (uint8)BITP_AD242X_I2SRATE_SHARE );


	pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL     |=  ( pSlvCfg->sPDMSettings.bHPFUse << (uint8)(BITP_A2B_PDMCTL_HPFEN) ) | \
															   ( pSlvCfg->sPDMSettings.nPDMRate <<(uint8)(BITP_AD242X_PDMCTL_PDMRATE)  | pSlvCfg->sPDMSettings.nNumSlotsPDM0 | pSlvCfg->sPDMSettings.nNumSlotsPDM1);


	/* INT registers */
	pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0        =  ( pSlvCfg->sInterruptSettings.bReportHDCNTErr       << (uint8)BITP_A2B_INTPND0_HDCNTERR) | \
																( pSlvCfg->sInterruptSettings.bReportDDErr          << (uint8)BITP_A2B_INTPND0_DDERR) | \
																( pSlvCfg->sInterruptSettings.bReportCRCErr         << (uint8)BITP_A2B_INTPND0_CRCERR) | \
																( pSlvCfg->sInterruptSettings.bReportDataParityErr  << (uint8)BITP_A2B_INTPND0_DPERR) | \
																( pSlvCfg->sInterruptSettings.bReportPwrErr         << (uint8)BITP_A2B_INTPND0_PWRERR ) | \
																( pSlvCfg->sInterruptSettings.bReportErrCntOverFlow << (uint8)BITP_A2B_INTPND0_BECOVF ) | \
																( pSlvCfg->sInterruptSettings.bReportSRFMissErr 	<< (uint8)BITP_A2B_INTPND0_SRFERR ) |\
																( pSlvCfg->sInterruptSettings.bReportSRFCrcErr 	    << (uint8)BITL_AD242X_INTPND0_SRFCRCERR );


	pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK1        =
																( pSlvCfg->sInterruptSettings.bReportGPIO0 << (uint8)BITP_A2B_INTPND1_IO0PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO1 << (uint8)BITP_A2B_INTPND1_IO1PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO2 << (uint8)BITP_A2B_INTPND1_IO2PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO3 << (uint8)BITP_A2B_INTPND1_IO3PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO4 << (uint8)BITP_A2B_INTPND1_IO4PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO5 << (uint8)BITP_A2B_INTPND1_IO5PND ) | \
																( pSlvCfg->sInterruptSettings.bReportGPIO6 << (uint8)BITP_A2B_INTPND1_IO6PND ) |\
																( pSlvCfg->sInterruptSettings.bReportGPIO7 << (uint8)BITP_AD242X_INTPND1_IO7PND );

	pNode->oProperties.nPINIORegister.nREG_A2B0_PINCFG   = (pSlvCfg->sGPIOSettings.bHighDriveStrength << (uint8)BITP_A2B_PINCFG_DRVSTR ) |\
																   (pSlvCfg->sGPIOSettings.bIRQInv << (uint8)BITP_AD242X_PINCFG_IRQINV) |\
																   (pSlvCfg->sGPIOSettings.bIRQTriState << (uint8)BITP_AD242X_PINCFG_IRQTS);


	/* Parsing Pin Mux */
	adi_a2b_ParseSlavePinMux7( pNode ,  pSlvCfg);

	/* GPIOD  */
	pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODINV =(pSlvCfg->sGPIODSettings.sGPIOD0Config.bGPIOSignalInv << 0u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD1Config.bGPIOSignalInv << 1u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD2Config.bGPIOSignalInv << 2u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD3Config.bGPIOSignalInv << 3u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD4Config.bGPIOSignalInv << 4u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD5Config.bGPIOSignalInv << 5u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD6Config.bGPIOSignalInv << 6u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD7Config.bGPIOSignalInv << 7u);

	pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIODEN  =(pSlvCfg->sGPIODSettings.sGPIOD0Config.bGPIODistance << 0u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD1Config.bGPIODistance << 1u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD2Config.bGPIODistance << 2u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD3Config.bGPIODistance << 3u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD4Config.bGPIODistance << 4u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD5Config.bGPIODistance << 5u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD6Config.bGPIODistance << 6u) |\
															 (pSlvCfg->sGPIODSettings.sGPIOD7Config.bGPIODistance << 7u);


	/* GPIOD Mask update */
	psGPIODMask[0] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD0MSK;
	psGPIODMask[1] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD1MSK;
	psGPIODMask[2] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD2MSK;
	psGPIODMask[3] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD3MSK;
	psGPIODMask[4] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD4MSK;
	psGPIODMask[5] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD5MSK;
	psGPIODMask[6] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD6MSK;
	psGPIODMask[7] = &pNode->oProperties.nGPIODRegister.nREG_AD242X0_GPIOD7MSK;

	asGPIODConfig[0] = pSlvCfg->sGPIODSettings.sGPIOD0Config;
	asGPIODConfig[1] = pSlvCfg->sGPIODSettings.sGPIOD1Config;
	asGPIODConfig[2] = pSlvCfg->sGPIODSettings.sGPIOD2Config;
	asGPIODConfig[3] = pSlvCfg->sGPIODSettings.sGPIOD3Config;
	asGPIODConfig[4] = pSlvCfg->sGPIODSettings.sGPIOD4Config;
	asGPIODConfig[5] = pSlvCfg->sGPIODSettings.sGPIOD5Config;
	asGPIODConfig[6] = pSlvCfg->sGPIODSettings.sGPIOD6Config;
	asGPIODConfig[7] = pSlvCfg->sGPIODSettings.sGPIOD7Config;

	for(nGPIOIndex = 0u; nGPIOIndex < 8u; nGPIOIndex++)
	{

		*(psGPIODMask[nGPIOIndex]) =  (uint16) ( (asGPIODConfig[nGPIOIndex].abBusPortMask[0] << 0u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[1] << 1u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[2] << 2u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[3] << 3u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[4] << 4u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[5] << 5u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[6] << 6u) |\
									 (asGPIODConfig[nGPIOIndex].abBusPortMask[7] << 7u));
	}




	/*ClockCFg1 & CFG2*/
	pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG |= (pSlvCfg->sClkOutSettings.bClk1Div << (uint8)BITP_AD242X_CLK1CFG_CLK1DIV) |\
															  (pSlvCfg->sClkOutSettings.bClk1PreDiv << (uint8)BITP_AD242X_CLK1CFG_CLK1PDIV) |\
															  (pSlvCfg->sClkOutSettings.bClk1Inv << (uint8)BITP_AD242X_CLK1CFG_CLK1INV);

	//pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG |= (pSlvCfg->sClkOutSettings.bClk2Div << (uint8)BITP_AD242X_CLK2CFG_CLK2DIV) |\
	//												   (pSlvCfg->sClkOutSettings.bClk2PreDiv << (uint8)BITP_AD242X_CLK2CFG_CLK2PDIV) |\
	//												   (pSlvCfg->sClkOutSettings.bClk2Inv << (uint8)BITP_AD242X_CLK2CFG_CLK2INV);
        pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG = 0xc1;


	/* Register */
	pNode->oProperties.nCTLRegister.nREG_AD242X0_BMMCFG = pSlvCfg->sRegSettings.nBMMCFG;
	pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_SUSCFG = pSlvCfg->sRegSettings.nSUSCFG;
	pNode->oProperties.nMailBoxRegister.nREG_AD242X0_MBOX0CTL = pSlvCfg->sRegSettings.nMBOX0CTL;
	pNode->oProperties.nMailBoxRegister.nREG_AD242X0_MBOX1CTL = pSlvCfg->sRegSettings.nMBOX1CTL;



}


ADI_A2B_RESULT adi_a2b_ParseBusDescription(ADI_A2B_GRAPH* pGraph, const ADI_A2B_BCD*  pBusDescription,ADI_A2B_FRAMEWORK_HANDLER_PTR pFrameworkHandle)
{

    ADI_A2B_NODE* pNode;
    ADI_A2B_MASTER_SLAVE_CONFIG* pMasterSlaveChain;
    ADI_A2B_COMMON_CONFIG* pCommon;
    const ADI_A2B_MASTER_NCD* pMstCfg;
    const ADI_A2B_SLAVE_NCD* pSlvCfg;
    uint8 nIndex=0,nIndex1/*,nIndex2*/;
    //uint8 nNumMasternode = pBusDescription->nNumMasterNode;
    uint8 nNodeIndex= 0u;
    //uint8 nNumConfig;
    //void* pRet;
    
    /* Reset graphdata */
    //pRet = memset( (void*)(pGraph) , 0 , ADI_A2B_GRAPH_DATA_LENGTH* sizeof(uint16) );
    
    /* Master-slave chain loop */
    // support single master only.
    //for(nIndex = 0 ; nIndex < nNumMasternode ; nIndex++ )
    {

        pNode = &pGraph->oNode[nNodeIndex];

        /* Get master-slave chain pointer */
        pMasterSlaveChain    = pBusDescription->apNetworkconfig[nIndex];

        /* Pointer to master configuration */
        pMstCfg = pMasterSlaveChain->pMasterConfig;

        /* Common configuration settings  */
        pCommon = (ADI_A2B_COMMON_CONFIG*)&pMasterSlaveChain->sCommonSetting;

        pNode->nNumSlave = pMasterSlaveChain->nNumSlaveNode;

        /* Parse master node */
        adi_a2b_ParseMasterNCD(pMstCfg,pNode,pCommon);
#if  A2B_ENABLE_AD242X_SUPPORT
        if(pMstCfg->sAuthSettings.nProductID == A2B_AD242x_PART)
        {
        	adi_a2b_ParseMasterNCDForAD242x(pMstCfg,pNode,pCommon);
        }
#endif
        nNodeIndex++;

        /* Loop over number of slaves */
        for(nIndex1 = 0u ; nIndex1 < (uint8)pMasterSlaveChain->nNumSlaveNode; nIndex1++)
        {
            pNode = &pGraph->oNode[nNodeIndex];
                       
            pSlvCfg = pMasterSlaveChain->apSlaveConfig[nIndex1];
            
            pNode->oProperties.nMasterI2CAddress                      =  pCommon->nMasterI2CAddr;
            pNode->oProperties.nBusI2CAddress                         =  pCommon->nBusI2CAddr;
            pNode->nNumSlave = 0u;

            /* Parse slave node */
            adi_a2b_ParseSlaveNCD(pSlvCfg,pNode,pFrameworkHandle);
#if A2B_ENABLE_AD242X_SUPPORT
            if(pSlvCfg->sAuthSettings.nProductID == A2B_AD242x_PART)
            {
            	adi_a2b_ParseSlaveNCDForAD242x(pSlvCfg,pNode,pFrameworkHandle);
            }
#endif
            pNode->nID=nNodeIndex-1;
            nNodeIndex++;
       
        }

    }
    /* Update total number of A2B nodes */
    pGraph->nNodeCount = nNodeIndex;

#if 0
    /* Target Properties update */
    pFrameworkHandle->oTrgtProprty.eDiscvryType        = (ADI_A2B_DISCOVERY_TYPE)pBusDescription->sTargetProperties.eDiscoveryMode;
    pFrameworkHandle->oTrgtProprty.bLineDiagnostic     = pBusDescription->sTargetProperties.bLineDiagnostics;
    pFrameworkHandle->oTrgtProprty.bAutoRediscovery    = pBusDescription->sTargetProperties.bAutoRediscOnFault;
    pFrameworkHandle->oTrgtProprty.nAttemptsCriticalFault = pBusDescription->sTargetProperties.nAttemptsCriticalFault;
    pFrameworkHandle->oTrgtProprty.bAutoDiscCriticalFault = pBusDescription->sTargetProperties.bAutoDiscCriticalFault;
    /* Number of peripheral devices connected to target processor */
    for(nIndex2 = 0u; nIndex2 < pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2++)
    {
        /* Include only if the configuration exists */
        if(pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->paPeriConfigUnit != NULL)
        {
            pGraph->oNode[0u].oConnectedTo[nIndex2].bActive         = 0u;
            pGraph->oNode[0u].oConnectedTo[nIndex2].nI2CAddress     = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nI2Caddr;
            pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceType     = (uint16)pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->eDeviceType;
            pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceID       = (uint16)GENERIC;

            /* Number of config */
            nNumConfig = pFrameworkHandle->aPeriDownloadTable[0u][0u].nNumConfig;
            pFrameworkHandle->aPeriDownloadTable[0u][0u].aDeviceConfig[nNumConfig].nConnectedNodeID       = 0xFFu;
            pFrameworkHandle->aPeriDownloadTable[0u][0u].aDeviceConfig[nNumConfig].nDeviceAddress         = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nI2Caddr;
            pFrameworkHandle->aPeriDownloadTable[0u][0u].aDeviceConfig[nNumConfig].nNumPeriConfigUnit     = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->nNumPeriConfigUnit;
            pFrameworkHandle->aPeriDownloadTable[0u][0u].aDeviceConfig[nNumConfig].paPeriConfigUnit       = pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->paPeriConfigUnit;
            pFrameworkHandle->aPeriDownloadTable[0u][0u].nNumConfig++;
        }
        else
        {
            pGraph->oNode[0u].oConnectedTo[nIndex2].bActive        = 0u;
            pGraph->oNode[0u].oConnectedTo[nIndex2].nI2CAddress    = 0xFFu;
            pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceType    = (uint16)pBusDescription->sTargetProperties.apPeriConfig[nIndex2]->eDeviceType;
            pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceID      = (uint16)UNKNOWN;
        }

    }
    /* Initialize empty fields */
    for(nIndex2 = pBusDescription->sTargetProperties.nNumPeriDevice; nIndex2 < (uint8)ADI_A2B_MAX_DEVICES_PER_NODE; nIndex2++  )
    {
        pGraph->oNode[0u].oConnectedTo[nIndex2].bActive        = 0u;
        pGraph->oNode[0u].oConnectedTo[nIndex2].nI2CAddress    = 0xFFu;
        pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceType    = (uint16)ADI_A2B_AUDIO_UNKNOWN;
        pGraph->oNode[0u].oConnectedTo[nIndex2].eDeviceID      = (uint16)UNKNOWN;
    } 
#endif

    return (ADI_A2B_SUCCESS);
            
}


/********************************************************************************/
/*!
@brief      This function parses I0 pin -0,1 & 2 MUX settings into register values

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void  adi_a2b_ParseSlavePinMux012( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg)
{
    
     /* Pin multiplex for GPIO 0*/
    switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO0PinUsage)
    {
        case A2B_GPIO_0_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO0IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO0Interrupt << BITP_A2B_PINTEN_IO0IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO0IntPolarity << BITP_A2B_PINTINV_IO0INV);
            break;
        case A2B_GPIO_0_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO0OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO0Val << BITP_A2B_GPIODAT_IO0DAT);
            break;
        /*case A2B_GPIO_0_DISABLE:
            break; */
        default:
        break;

    }

    /* Pin multiplex for GPIO 1*/
    switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO1PinUsage)
    {
        case A2B_GPIO_1_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO1IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO1Interrupt << BITP_A2B_PINTEN_IO1IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO1IntPolarity << BITP_A2B_PINTINV_IO1INV);
            break;
        case A2B_GPIO_1_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO1OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO1Val << BITP_A2B_GPIODAT_IO1DAT);
            break;
#if  A2B_ENABLE_AD242X_SUPPORT
        case A2B_GPIO_1_AS_CLKOUT:
            pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG     |= (1u << (uint8)BITP_AD242X_CLK1CFG_CLK1EN);
            break;
#endif

        /* case A2B_GPIO_1_DISABLE:
            break; */
        default:
        break;

    }



    /* Pin multiplex for GPIO 2*/
    switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO2PinUsage)
    {
        case A2B_GPIO_2_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO2IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO2Interrupt << BITP_A2B_PINTEN_IO2IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO2IntPolarity << BITP_A2B_PINTINV_IO2INV);
            break;
        case A2B_GPIO_2_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO2OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO2Val << BITP_A2B_GPIODAT_IO2DAT);
            break;
        case A2B_GPIO_2_AS_CLKOUT:
#if  A2B_ENABLE_AD241X_SUPPORT
            pNode->oProperties.nPINIORegister.nREG_A2B0_CLKCFG        |= (1u << (uint8)BITP_A2B_CLKCFG_CCLKEN);
#endif
#if  A2B_ENABLE_AD242X_SUPPORT
            pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG     |= (1u << (uint8)BITP_AD242X_CLK2CFG_CLK2EN);
#endif
            break;
        /* case A2B_GPIO_2_DISABLE:
            break; */
        default:
        break;

    }
}

/********************************************************************************/
/*!
@brief      This function parses I0 pin -0,1 & 2 MUX settings into register values

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void adi_a2b_ParseSlavePinMux34( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg)
{
     /* Pin multiplex for GPIO 3*/
    switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO3PinUsage)
    {
        case A2B_GPIO_3_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO3IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO3Interrupt << BITP_A2B_PINTEN_IO3IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO3IntPolarity << BITP_A2B_PINTINV_IO3INV);
            break;
        case A2B_GPIO_3_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO3OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO3Val << BITP_A2B_GPIODAT_IO3DAT);
            break;
        case A2B_GPIO_3_AS_DTX0:
            pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= ( 1u <<BITP_A2B_I2SCFG_TX0EN);
            break;
    /*    case A2B_GPIO_3_DISABLE:
            break; */
        default:
        break;

    }

   /* Pin multiplex for GPIO 4*/
  switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO4PinUsage)
  {
    case A2B_GPIO_4_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO4IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO4Interrupt << BITP_A2B_PINTEN_IO4IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO4IntPolarity << BITP_A2B_PINTINV_IO4INV);
        break;
    case A2B_GPIO_4_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO4OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO4Val << BITP_A2B_GPIODAT_IO4DAT);
        break;
    case A2B_GPIO_4_AS_DTX1:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= ( 1u <<BITP_A2B_I2SCFG_TX1EN);
        break;
     /* case A2B_GPIO_4_DISABLE:
        break; */
    default:
        break;

  }

    
}
/********************************************************************************/
/*!
@brief      This function parses I0 pin -5 and 6 MUX settings into register values

@param [in] pSlvCfg             Pointer Slave Node Configuration Data(NCD)
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void  adi_a2b_ParseSlavePinMux56( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg)
{
      

  /* Pin multiplex for GPIO 5*/
  switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO5PinUsage)
  {
    case A2B_GPIO_5_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO5IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO5Interrupt << BITP_A2B_PINTEN_IO5IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO5IntPolarity << BITP_A2B_PINTINV_IO5INV);
        break;
    case A2B_GPIO_5_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO5OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO5Val << BITP_A2B_GPIODAT_IO5DAT);
        break;
    case A2B_GPIO_5_AS_DRX0:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= ( 1u <<BITP_A2B_I2SCFG_RX0EN);
        break;
    case A2B_GPIO_5_AS_PDM0:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL       |= ( 1u <<BITP_A2B_PDMCTL_PDM0EN);
        break;
    /* case A2B_GPIO_5_DISABLE:
        break; */
    default:
        break;

  }


  /* Pin multiplex for GPIO 6*/
  switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO6PinUsage)
  {
    case A2B_GPIO_6_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO6IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO6Interrupt << BITP_A2B_PINTEN_IO6IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO6IntPolarity << BITP_A2B_PINTINV_IO6INV);
        break;
    case A2B_GPIO_6_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO6OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO6Val << BITP_A2B_GPIODAT_IO6DAT);
        break;
    case A2B_GPIO_6_AS_DRX1:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= ( 1u <<BITP_A2B_I2SCFG_RX1EN);
        break;
    case A2B_GPIO_6_AS_PDM1:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_PDMCTL       |= ( 1u <<BITP_A2B_PDMCTL_PDM1EN);
        break;
    /* case A2B_GPIO_6_DISABLE:
        break; */
    default:
        break;

  }
}

/********************************************************************************/
/*!
@brief      This function parses I0 pin -7 MUX settings into register values

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void  adi_a2b_ParseSlavePinMux7( ADI_A2B_NODE* pNode , const ADI_A2B_SLAVE_NCD* pSlvCfg)
{


    /* Pin multiplex for GPIO 7*/
   switch(pSlvCfg->sGPIOSettings.sPinMuxSettings.bGPIO7PinUsage)
   {
	   case A2B_GPIO_7_INPUT:
		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_AD242X_GPIOIEN_IO7IEN);
		   pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO7Interrupt << BITP_AD242X_PINTEN_IO7IE);
		   pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pSlvCfg->sGPIOSettings.sPinIntConfig.bGPIO7IntPolarity << BITP_AD242X_PINTINV_IO7INV);
		   break;
	   case A2B_GPIO_7_OUTPUT:
		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_AD242X_GPIOOEN_IO7OEN);
		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pSlvCfg->sGPIOSettings.sOutPinVal.bGPIO7Val << BITP_AD242X_GPIODAT_IO7DAT);
		   break;
	   /*case A2B_GPIO_7_DISABLE:
		   break; */
	   default:
	   break;

   }

}


/********************************************************************************/
/*!
@brief      This function parses I0 pin -3 and 4 MUX settings into register values

@param [in] pMstCfg             Pointer to master Node Configuration Data(NCD)
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void adi_a2b_ParseMasterPinMux34( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg)
{
    
   /* Pin multiplex for GPIO 3*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO3PinUsage)
    {
        case A2B_GPIO_3_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO3IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO3Interrupt << BITP_A2B_PINTEN_IO3IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO3IntPolarity << BITP_A2B_PINTINV_IO3INV);
            break;
        case A2B_GPIO_3_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO3OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO3Val << BITP_A2B_GPIODAT_IO3DAT);
            break;
        case A2B_GPIO_3_AS_DTX0:
            pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG          |= (1u <<BITP_A2B_I2SCFG_TX0EN);
            break;
        /*case A2B_GPIO_3_DISABLE:
            break;*/
        default:
        break;

    }

    /* Pin multiplex for GPIO 4*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO4PinUsage)
    {
    case A2B_GPIO_4_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO4IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO4Interrupt << BITP_A2B_PINTEN_IO4IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO4IntPolarity << BITP_A2B_PINTINV_IO4INV);
        break;
    case A2B_GPIO_4_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO4OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO4Val << BITP_A2B_GPIODAT_IO4DAT);
        break;
    case A2B_GPIO_4_AS_DTX1:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= (1u <<BITP_A2B_I2SCFG_TX1EN);
        break;
    /*case A2B_GPIO_4_DISABLE:
        break;*/
    default:
        break;

    }
  
    
}

/********************************************************************************/
/*!
@brief      This function parses I0 pin- 5 and 6 MUX settings into register values

@param [in] pMstCfg             Pointer master Node Configuration Data(NCD)
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void adi_a2b_ParseMasterPinMux56( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg)
{
    
    /* Pin multiplex for GPIO 5*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO5PinUsage)
    {
    case A2B_GPIO_5_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO5IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO5Interrupt << BITP_A2B_PINTEN_IO5IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO5IntPolarity << BITP_A2B_PINTINV_IO5INV);
        break;
    case A2B_GPIO_5_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO5OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO5Val << BITP_A2B_GPIODAT_IO5DAT);
        break;
    case A2B_GPIO_5_AS_DRX0:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG        |= (1u <<BITP_A2B_I2SCFG_RX0EN);
        break;
    /*case A2B_GPIO_5_DISABLE:
        break;*/
    default:
        break;

    }


    /* Pin multiplex for GPIO 6*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO6PinUsage)
    {
    case A2B_GPIO_6_INPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO6IEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO6Interrupt << BITP_A2B_PINTEN_IO6IE);
        pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO6IntPolarity << BITP_A2B_PINTINV_IO6INV);
        break;
    case A2B_GPIO_6_OUTPUT:
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO6OEN);
        pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO6Val << BITP_A2B_GPIODAT_IO6DAT);
        break;
    case A2B_GPIO_6_AS_DRX1:
        pNode->oProperties.nI2CI2SRegister.nREG_A2B0_I2SCFG       |= (1u <<BITP_A2B_I2SCFG_RX1EN);
        break;
    /*case A2B_GPIO_6_DISABLE:
        break;*/
    default:
        break;

    }

}

/********************************************************************************/
/*!
@brief      This function parses I0 pin -0,1 & 2 MUX settings into register values

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void  adi_a2b_ParseMasterPinMux12( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg)
{


    /* Pin multiplex for GPIO 1*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO1PinUsage)
    {
        case A2B_GPIO_1_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO1IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1Interrupt << BITP_A2B_PINTEN_IO1IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO1IntPolarity << BITP_A2B_PINTINV_IO1INV);
            break;
        case A2B_GPIO_1_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO1OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO1Val << BITP_A2B_GPIODAT_IO1DAT);
            break;
#if  A2B_ENABLE_AD242X_SUPPORT
        case A2B_GPIO_1_AS_CLKOUT:
            pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK1CFG     |= (1u << (uint8)BITP_AD242X_CLK1CFG_CLK1EN);
            break;
#endif

        /* case A2B_GPIO_1_DISABLE:
            break; */
        default:
        break;

    }



    /* Pin multiplex for GPIO 2*/
    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO2PinUsage)
    {
        case A2B_GPIO_2_INPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_A2B_GPIOIEN_IO2IEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO2Interrupt << BITP_A2B_PINTEN_IO2IE);
            pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO2IntPolarity << BITP_A2B_PINTINV_IO2INV);
            break;
        case A2B_GPIO_2_OUTPUT:
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_A2B_GPIOOEN_IO2OEN);
            pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO2Val << BITP_A2B_GPIODAT_IO2DAT);
            break;
        case A2B_GPIO_2_AS_CLKOUT:
#if  A2B_ENABLE_AD242X_SUPPORT
            pNode->oProperties.nPINIORegister.nREG_AD242X0_CLK2CFG     |= (1u << (uint8)BITP_AD242X_CLK2CFG_CLK2EN);
#endif
            break;
        /* case A2B_GPIO_2_DISABLE:
            break; */
        default:
        break;

    }
}




/********************************************************************************/
/*!
@brief      This function parses I0 pin -7 MUX settings into register values

@param [in] pSlvCfg             Pointer Node description data
@param [in] pNode               Pointer to A2B node structure
@return     void

*/
/***********************************************************************************/
static void  adi_a2b_ParseMasterPinMux7( ADI_A2B_NODE* pNode , const ADI_A2B_MASTER_NCD* pMstCfg)
{

    switch(pMstCfg->sGPIOSettings.sPinMuxSettings.bGPIO7PinUsage)
    {
 	   case A2B_GPIO_7_INPUT:
 		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOIEN       |= (1u << (uint8)BITP_AD242X_GPIOIEN_IO7IEN);
 		   pNode->oProperties.nPINIORegister.nREG_A2B0_PINTEN        |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO7Interrupt << BITP_AD242X_PINTEN_IO7IE);
 		   pNode->oProperties.nPINIORegister.nREG_A2B0_PINTINV       |= (pMstCfg->sGPIOSettings.sPinIntConfig.bGPIO7IntPolarity << BITP_AD242X_PINTINV_IO7INV);
 		   break;
 	   case A2B_GPIO_7_OUTPUT:
 		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIOOEN       |= (1u << (uint8)BITP_AD242X_GPIOOEN_IO7OEN);
 		   pNode->oProperties.nPINIORegister.nREG_A2B0_GPIODAT       |= (pMstCfg->sGPIOSettings.sOutPinVal.bGPIO7Val << BITP_AD242X_GPIODAT_IO7DAT);
 		   break;
 	   /*case A2B_GPIO_7_DISABLE:
 		   break; */
 	   default:
 	   break;

    }



}

/****************************************************************************/
/*!
@brief  This function starts specified timer channel.
        Timer period shall be in micro-seconds

@param [in] nTimerNo    Timer number
@param [in] nTime       Time period in micro-seconds

@return         Return code
                - 0: Success
                - 1: Failure 

*/    
/******************************************************************************/
uint32 adi_a2b_TimerStart(uint32 nTimerNo, uint32 nTime)
{ 
  return 0;
}


/****************************************************************************/
/*!
    @brief      This function enables discovery of  the specified slave node by 
                switching on power(SWCTL) and puts master node in discovery mode.
                  

    @param [in]    pNode       Pointer to the A2B node structure
    @param [in]    pTimeHandle Pointer to Timer configuration structure
    @param [in]    nResp       RESPCYS(response cycle) of the node to be discovered    

    
    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success  
    
*/
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_StartDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode ,ADI_A2B_TIMER_HANDLER_PTR  pTimeHandle , uint8 nResp)
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint32 nResult = 0u;
    uint8 nCount = 0u,nVal;
    uint8 nIndex,I2CAddr;
    //uint32 nRes;
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[A2B_MAX_NUM_REG]; 
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    /* Set timer for discovery in diagnostic mode */
    pTimeHandle->bTimeout = 0u;
    pTimeHandle->eContext = ADI_A2B_DISCOVERY;

    /* Start A2B timer  */
    nResult = adi_a2b_TimerStart(pTimeHandle->nTimerNo,pTimeHandle->nTimerExpireVal);

    /* Master Pre-set */
    I2CAddr = adi_a2b_masterPreset(me,pNode);
       
#if A2B_SS_ENABLE
    nRes = adi_a2b_MsgDrvInterruptDisable(1u);
#endif    
    /* For master node */
    if ( pNode->eType == (uint16)ADI_A2B_MASTER)
    {
        TP_PRINTF("Master Node Discovery\r\n");
#if (A2B_DISCOVERY_DIAGNOSTICS_BY_POLLING  == 0u)
       
        /* Unmask Discovery and slave interrupts */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK2 | (uint8)BITM_A2B_INTPND2_DSCDONE | (uint8)BITM_A2B_INTPND2_SLVIRQ );
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK2; 

        /* Unmask Power error interrupt */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0|(uint8)BITM_A2B_INTPND0_PWRERR ); 
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK0; 
         
         /* Clear if any interrupts pending  */
        aTwiWriteTable[nCount].nValue  = (uint8)(0x10u); 
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTPND0;
#endif 
        
        /* Enable switch */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SWCTL; 
         
        for(nIndex  =0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }
            
        }     
    
    }
    /* For the slave Nodes */
    else
    {
        TP_PRINTF("Slave Node Discovery\r\n");
        /* Set the node id for addressing */
        if( nResult == 0u)
        {
            /* Get the slave node id */
            nVal  = (uint8)(pNode->nID);
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
        }
        /* Slave Pre-set */
        I2CAddr = adi_a2b_slavePreset(me, pNode);
        
#if (A2B_DISCOVERY_DIAGNOSTICS_BY_POLLING  == 0u)         
        /* Unmask Power error interrupt */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nINTRegister.nREG_A2B0_INTMSK0|0x10u); 
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTMSK0; 
        
        /* Clear if any interrupts pending  */
        aTwiWriteTable[nCount].nValue  = (uint8)(0x10u); 
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_INTPND0; 
#endif  

         /* Enable switch */
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SWCTL;
        
        for(nIndex  = 0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }
            
       }  
  

    }
    /* Master Pre-set */
    I2CAddr = adi_a2b_masterPreset(me, pNode);
    
    /* Delay between down-stream and Upstream,  entering to discovery mode -Immediate write(silicon anomaly rev 0.1)  */
    nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)REG_A2B0_DISCVRY ,nResp);
    
    /* Check for failure */
    if( nResult != 0u)
    {
        eResult = ADI_A2B_FAILURE;
    }
    
#if A2B_SS_ENABLE    
    nRes = adi_a2b_MsgDrvInterruptDisable(0u);
#endif
 
        
    
    return eResult;
}

/****************************************************************************/
/*!
    @brief          This function checks whether node is discovered or not.
                    ( Checks REG_A2B0_INTPND2 register )

    @param [in]     pNode          Pointer to node structure

    @return          Return code
                    - 0: Node is not discovered
                    - 1: Node is discovered.
*/
/********************************************************************************/ 
static uint8 adi_a2b_CheckNodeDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode ) 
{
        
    uint8 nVal,nRet = 0u;
    uint8 nA2BI2CAddr;
    uint32 nResult;
    nVal = 0u;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    
    /* Preset to write to master */
    nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

    /* Check whether  discovery interrupt is pending */
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_INTPND2,(uint8*)&nVal);
    nVal    = nVal & 0x01u;

    if(nVal == 0x01u)
    {
        /* If discovery is asserted return 1 */
        nRet = 1u;

        /* Write 1 to clear */
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_INTPND2 ,nVal);
    }

    (void)nResult;
    
    return( nRet ); 
}

/****************************************************************************/
 /*! 
    @brief          This function checks authentication status of the discovered node

    @param [in]     pNode     Pointer to node structure
    
    @return          Return code
                    - 0: Node is not authenticated.(Product or version or  vendor IDs are  not matching)
                    - 1: Node is authenticated.(Product,version and vendor IDs are matching)

*/                    
/********************************************************************************/ 
static uint8 adi_a2b_CheckAuthentication(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode ) 
{
        
    uint8 nVal,nVendor,nProduct,nVersion;
    uint8 nA2BI2CAddr;
    uint32 nRet  = 1u ,nResult;
//    uint8 nExpVendorID, nExpProductID, nExpVersionID;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    
    //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR = (uint16)sMasterNode0.sAuthSettings.nVendorID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT = (uint16)sMasterNode0.sAuthSettings.nProductID;
    //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION = (uint16)sMasterNode0.sAuthSettings.nVersionID;
//    nExpVendorID  = (uint8)sMasterNode0.sAuthSettings.nVendorID ;
//    nExpProductID = (uint8)sMasterNode0.sAuthSettings.nProductID ;
//    nExpVersionID = (uint8)sMasterNode0.sAuthSettings.nVersionID ;
    
     /* Preset to write to master */
    nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

    nVal = (uint8)(pNode->nID);
    nResult = adi_a2b_TwiWrite8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_NODEADR,(uint8)nVal);

     /* Preset to write to slave */
    nA2BI2CAddr = adi_a2b_slavePreset(me, pNode);
    
    /* Read Vendor,Capability and ID register */
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_VENDOR,(uint8*)&nVendor);
    
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_PRODUCT,(uint8*)&nProduct);
    
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_VERSION,(uint8*)&nVersion);
#if 0    

    /* Checking with expected value */
    if( nVendor != nExpVendorID)
    {
        /* Storing read Version ,Product and Vendor ID for notification */
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR |= (uint16)nVendor << 8u;
        
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT |= (uint16)nProduct<< 8u;
        
       //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION |= (uint16)nVersion<< 8u;
         
        //nRet = 0u;
    } 
    /* Checking for valid product ID  */
    if( nProduct != nExpProductID)
    {
        /* Storing read Version ,Product and Vendor ID */
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT |= (uint16)nProduct<< 8u;
        
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR |= (uint16)nVendor << 8u;
        
        //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION |= (uint16)nVersion<< 8u;
        
        //nRet = 0u;
         
    }
    /* Checking for valid version  */
    if ( nVersion != nExpVersionID)
    {
        /* Storing read Version ,Product and Vendor ID */
        //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VERSION |= (uint16)nVersion<< 8u;
        
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_PRODUCT |= (uint16)nProduct<< 8u;
        
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR &= 0x00FFu;
        //pNode->oProperties.nIDRegister.nREG_A2B0_VENDOR |= (uint16)nVendor << 8u;
        
        //nRet = 0u;
    }
    nResult = adi_a2b_TwiWrite8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)0xf3,(uint8)0xd4);
#endif

    
#if A2B_PRINT_FOR_DEBUG

    if( nRet == 1u)
    {
        printf(" \n Branch %d : Slave Node %d Authentication : SUCCESS\n",((pNode->nID & 0xFF00u) >> 8u) , (pNode->nID & 0xFFu) );
    }
    else
    {
        printf(" \n Branch %d : Slave Node %d Authentication : FAILURE\n",((pNode->nID & 0xFF00u) >> 8u) , (pNode->nID & 0xFFu) );
    }
#endif  

    (void)nResult;

    return( (uint8)nRet ); 
}

uint32 adi_a2b_TimerStop(uint32 nTimerNo)
{
    return 1;
}


/****************************************************************************/
/*!
   @brief      This function asserts slave node discovery(includes authentication).

   @param [in]    pNodeDiscover             Pointer to the A2B node to be discovered.
   @param [in]    pNodeConfigure            Pointer to preceding node which sources power
   @param [in]    pFrameWorkHandle          Pointer to the Framework configuration structure

   @return        ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure(Node discovery failure)
                    - ADI_A2B_SUCEESS : Success(Successful discovery)

*/
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_DiscoveryConfirm(cAdiAD2410Drv* me, ADI_A2B_NODE *pNodeDiscover ,ADI_A2B_NODE* pNodeConfigure , ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle)
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS , eSwitchResult;
    uint32 bA2BTimeout,/*nResult,*/nCount =0u;
    uint8  bNodeFound =0u,nVal;
     //ADI_A2B_FAULT_STATUS_HANDLER_PTR pFaultStatus = &(pFrameWorkHandle->oLineFault.sFaultStatus);
    //ADI_A2B_EVENT_HANDLER_PTR  pEventInt = &(pFrameWorkHandle->oEventInt);
    uint8  bDiscDone;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    /* Assign values to a local variable */
    bA2BTimeout = pFrameWorkHandle->sTimerHandle.bTimeout;

    /* Clear current fault status */    
    //pFaultStatus->nCurrentFaultCode  = 0u;
    //pFaultStatus->bCurrentFaultNonLoc = 0u;
    //pFaultStatus->bCurrentFaultFound = 0u;
    
    /* Discovery & diagnostics by polling or interrupt */
#if (A2B_DISCOVERY_DIAGNOSTICS_BY_POLLING == 0u)

    pEventInt->bIntProcessDiscDone = 0u;
    pEventInt->bNodeFound          = 0u;
        
    if  (pEventInt->bEvent == TRUE) 
    {  
        adi_a2b_InterruptDuringDiscovery(pFrameWorkHandle);
    }
     /* Start waiting for interrupt */
     pEventInt->eIntContext         = ADI_A2B_INTERRUPT_DURING_DISCOVERY; 
        
#endif   
   
    
#if A2B_DEBUG_STORE_CONFIG
    aDebugData.aStoreDataVal[aDebugData.nNum].eOp = 0x02u;
    aDebugData.aStoreDataVal[aDebugData.nNum].nRegAddr = 0x00u;
    aDebugData.aStoreDataVal[aDebugData.nNum].nTwiAddr = 0x00u;
    aDebugData.aStoreDataVal[aDebugData.nNum].nDatVal = 50u;
    adi_a2b_Delay(50u);
    aDebugData.nNum++;
#endif     

   nCount = 0u;
   
   /* Wait for timer expiry or node discovery */
   do 
   {
        nCount++;
        
#if (A2B_DISCOVERY_DIAGNOSTICS_BY_POLLING == 1u)
        
        bNodeFound  = adi_a2b_CheckNodeDiscovery(me, pNodeDiscover);
        bDiscDone = bNodeFound;
#else
        bDiscDone = pFrameWorkHandle->oEventInt.bIntProcessDiscDone; 
        bNodeFound = pFrameWorkHandle->oEventInt.bNodeFound;  
#endif        
        bA2BTimeout = pFrameWorkHandle->sTimerHandle.bTimeout;
        /*delay(ms);*/
        
    //} while ((!bA2BTimeout) && (bDiscDone == (uint8)0u));
    } while ((nCount<2) && (bDiscDone == (uint8)0u));
    
    /* Stop the timer */
    //nResult = adi_a2b_TimerStop(A2B_TIMER_NO);
    //pFrameWorkHandle->sTimerHandle.bTimeout = 0u;
    //pFrameWorkHandle->oEventInt.eIntContext = ADI_A2B_INTERRUPT_AFTER_DISCOVERY;
    
    BSP_BlockingDelayMs(100); 
     /* Check Discovery status */
    bNodeFound |= adi_a2b_CheckNodeDiscovery(me, pNodeDiscover);
  

    /* If node is discovered, make it active */
    if (bNodeFound == 1u)
    {
    
#if A2B_PRINT_FOR_DEBUG
        printf(" \n \n Branch %d : Slave Node %d : Discovered \n",((pNodeDiscover->nID & 0xFF00u) >> 8u) , (pNodeDiscover->nID & 0xFFu) );
#endif 
        eResult = ADI_A2B_SUCCESS;
        
        /* Assign authentication status */
        pNodeDiscover->bActive = adi_a2b_CheckAuthentication(me, pNodeDiscover);

        if(pNodeDiscover->bActive == 0u )
        {
            /* MSB 0xFF indicates authentication failure */
            pNodeDiscover->bActive |= A2B_AUTHENTICATION_FAILURE;   
            eResult = ADI_A2B_FAILURE; 
        }
        else
        {
            /* Change mode only if line diagnostic is disabled */ 
            if(pFrameWorkHandle->oTrgtProprty.bLineDiagnostic == 0u)
            {
              /* Go to mode 2 */
              nVal = (uint8)(( pNodeConfigure->oProperties.nCTLRegister.nREG_A2B0_SWCTL & ((uint8)BITM_A2B_SWCTL_MODE^0xFFu ) ) | 0x20u);
              eSwitchResult = adi_a2b_ConfigNodePowerMode(me, pNodeConfigure , nVal);
            }

        }
        me->pFrameworkHandle->pgraph->NumberOfSlaveDiscovered++;

    }
    else
    {
        eResult = ADI_A2B_FAILURE;

#if A2B_PRINT_FOR_DEBUG            
        printf("Branch %d : Discovery timed out for Slave Node %d \n", ((pNodeDiscover->nID & 0xFF00u) >> 8u) , (pNodeDiscover->nID & 0xFFu) );
        printf("Check connections: %d\n",pNodeDiscover->nID );
#endif               
    
    }

    (void)eSwitchResult;
    (void)bA2BTimeout;
    
    return(eResult);

}

/****************************************************************************/
/*!
    @brief          This function implements line fault diagnosis during discovery

    @param [in]     pNodeDiscover       Pointer to the A2B node to be discovered
    @param [in]     pNodeConfigure      Pointer to the preceding node which sources the power
    @param [in]     pFrameWorkHandle    Pointer to framework structure

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success(No switch faults)

*/
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_LineDiagDuringDiscovery(ADI_A2B_NODE *pNodeDiscover ,ADI_A2B_NODE *pNodeConfigure,ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    return eResult;
}


/*****************************************************************************/
/*!
 @brief     This function initiates slave discovery and configures the discovered node depending upon discovery mode.

 @param [in]    pNodeDiscover             Pointer to the A2B node to be discovered.
 @param [in]    pNodeConfigure            Pointer to A2B node to be configured (preceding upstream node)
  @param [in]   pFrameWorkHandle          Pointer to the Framework configuration structure
    
 @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success  
    
*/
/*****************************************************************************/
static ADI_A2B_RESULT adi_a2b_EnableNodeDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNodeDiscover ,ADI_A2B_NODE *pNodeConfigure ,ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle  )
{

    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    ADI_A2B_TRGT_PROPERTY_HANDLER* pTrgtPropHandle = &(pFrameWorkHandle->oTrgtProprty);  
    ADI_A2B_TIMER_HANDLER_PTR   pTimeHandle = &(pFrameWorkHandle->sTimerHandle);
    uint8 nResp,nMasterIndex;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    /*Set node status*/
    pNodeDiscover->bActive = 0u;
    
    
    if ( pTrgtPropHandle->eDiscvryType  ==  ADI_A2B_SIMPLE_DISCOVERY )
    {
       
        /* Master settings before first slave discovery */
        if( (pNodeDiscover->nID & 0xFFu) == 0u)
        {
            /* Configuring Master */
        	nMasterIndex = adi_a2b_getMasterNodeIndex(pNodeConfigure->nID);

        }
        
        /* Delay between down-stream and Upstream,  entering to discovery mode*/
        nResp  = (uint8)(pNodeDiscover->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
        
        /* Enable node discovery */
        eResult = adi_a2b_StartDiscovery(me, pNodeConfigure,pTimeHandle,nResp);
        BSP_BlockingDelayMs(100);
        
    }
#if 0
    else if ( pTrgtPropHandle->eDiscvryType  ==  ADI_A2B_MODIFED_DISCOVERY )
    {
    	/* Configuration interleaved with discovery */
        eResult = adi_a2b_ConfigDuringDiscovery(pNodeDiscover,pNodeConfigure,pFrameWorkHandle);
        
        /* Delay between down-stream and Upstream,  entering to discovery mode*/
        nResp  = (uint8)(pNodeDiscover->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
        
        /* Enable node discovery */
        eResult = adi_a2b_StartDiscovery(pNodeConfigure ,pTimeHandle,nResp );

            
    }       

    else if( pTrgtPropHandle->eDiscvryType  ==  ADI_A2B_OPTIMIZED_DISCOVERY)
    {
        
        /* Delay between down-stream and up-stream,  entering to discovery mode*/
        nResp  = (uint8)(pNodeDiscover->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
        
        /* Enable node discovery */
        eResult = adi_a2b_StartDiscovery(pNodeConfigure,pTimeHandle,nResp );

        /* Configuration interleaved with discovery */
        eResult = adi_a2b_ConfigDuringDiscovery(pNodeDiscover,pNodeConfigure,pFrameWorkHandle);


    }  
    else /* if (pTrgtPropHandle->eDiscvryType  ==  ADI_A2B_ADVANCED_DISCOVERY) */
    {
     
        /* Delay between down-stream and Upstream,  entering to discovery mode*/
        nResp  = (uint8)(pNodeDiscover->oProperties.nCTLRegister.nREG_A2B0_RESPCYCS);
        
        /* Enable node discovery */
        eResult = adi_a2b_StartDiscovery(pNodeConfigure,pTimeHandle,nResp );

        /* Configuration interleaved with discovery */
        eResult = adi_a2b_ConfigDuringAdvancedDiscovery(pNodeDiscover,pNodeConfigure,pFrameWorkHandle);
        
    }
#endif    
    /* Wait for discovery to complete */
    eResult = adi_a2b_DiscoveryConfirm(me, pNodeDiscover,pNodeConfigure,pFrameWorkHandle);
    
#if A2B_LINE_DIAGNOSTIC_ENABLE
    if(pTrgtPropHandle->bLineDiagnostic == 1u)
    {
        /* Run line diagnostic */
        eResult = adi_a2b_LineDiagDuringDiscovery(pNodeDiscover,pNodeConfigure,pFrameWorkHandle);
    }
#endif   

    if( eResult != ADI_A2B_SUCCESS)
    {
        TP_PRINTF("Enable Node Discovery Fail\n");
        //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].eError = ADI_A2B_NODE_DISCOVERY_FAILURE;
        //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].nNodeID = (pNodeDiscover->nID);
        //pFrameWorkHandle->nErrorCount++;
        //pFrameWorkHandle->nErrorCount = (pFrameWorkHandle->nErrorCount) % (uint32)A2B_MAX_NUM_ERROR;
    }
    
    (void)nMasterIndex;

    return eResult;
}


/*****************************************************************************/
/*!
    @brief      This function configures the network in order to address slave node.
                In case of branch node, it enables remote program option(PERI) in the branch orginating node
                
    @param [in] pNode          Pointer to the any node in the branch


    @return     uint8
                -I2C address to be used for slave addressing
*/
/*****************************************************************************/
uint8 adi_a2b_slavePreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    uint8 nI2CAddr = 0xFFu/*,nMasterAddr,nBusAddr,nVal,nSlaveID*/;
    ADI_A2B_NODE /**pSrcNodePtr,*/*pNodeBranchMaster;
    //uint32 nResult;
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);

    /* Get the master */
    if(pNode->eType == (uint16)ADI_A2B_SLAVE)
    {
       adi_a2b_getMasterNodePtr(pNode->nID ,&pNodeBranchMaster );
    }
    else
    {
       pNodeBranchMaster = pNode;
    }

    /* Check whether a directly connected master */
    if(pNodeBranchMaster->nA2BSrcNodeID == 0xFFu)
    {
       nI2CAddr = (uint8)pNode->oProperties.nBusI2CAddress;
    }
#if A2B_BRANCH_SUPPORT     
    else
    {
        adi_a2b_getSlaveNodePtr(pNodeBranchMaster->nA2BSrcNodeID , &pSrcNodePtr);

        nMasterAddr = (uint8)pSrcNodePtr->oProperties.nMasterI2CAddress;
        nBusAddr   = (uint8)pSrcNodePtr->oProperties.nBusI2CAddress;
        /* Get position ID*/
        nSlaveID = (uint8)(pSrcNodePtr->nID & 0xFFu);

        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nSlaveID);

        /* Write to CHIP address */
        nVal  = (uint8)pNode->oProperties.nBusI2CAddress;
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nBusAddr,(uint8)REG_A2B0_CHIP ,nVal);

        /* Enable remote I2C write */
        nVal  =  (uint8)(pSrcNodePtr->nID & 0xFFu) | (uint8)BITM_A2B_NODEADR_PERI;
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nVal);

        /* Return I2C address */
        nI2CAddr = nBusAddr;

    }
#endif
    return(nI2CAddr);

}


/****************************************************************************/
/*!
    @brief      This function configures downstream power modes( writes to SWCTL)

    @param [in]    pNode           Pointer to the A2B node structure
    @param [in]    nSwitchValue    Switch value to be configured

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success

*/
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_ConfigNodePowerMode(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , uint8 nSwitchValue )
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint32 nResult = 0u;
    uint8 nVal = 0u;
    uint8 nCount = 0u;
    uint8 nIndex,I2CAddr;
    TP_PRINTF("%s\r\n", __FUNCTION__);

    /* Make sure enough memory is allocated */
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[A2B_MAX_NUM_REG];

    /* Master Pre-set */
    I2CAddr = adi_a2b_masterPreset(me, pNode);

    /* For master node */
    if ( pNode->eType == (uint16)ADI_A2B_MASTER)
    {

        /* Enable switch */
        aTwiWriteTable[nCount].nValue  = (uint8)(nSwitchValue);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SWCTL;

        for(nIndex  =0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }

        }


    }
    /* For the slave Nodes */
    else
    {
        /* Set the node id for addressing */
        /* Get the slave node id */
        nVal  = (uint8)(pNode->nID & 0x00FFu);
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);

        /* Slave/Bus Pre-set */
        I2CAddr = adi_a2b_slavePreset(me, pNode);

        /* Configure switch */
        aTwiWriteTable[nCount].nValue  = nSwitchValue;
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SWCTL;

        for(nIndex  = 0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, I2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }

        }

    }

    /* Check for failure */
    if( nResult != 0u)
    {
        eResult = ADI_A2B_FAILURE;
    }

    return eResult;
}


/****************************************************************************/
/*!
    @brief          This function is responsible for downstream power mode configuration 
                    for all the active nodes after discovery.

    @param [in]     pNode               Pointer to the A2B node structure
    @param [in]     pFrameWorkHandle    Pointer to framework structure

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success

*/
/*****************************************************************************/

static ADI_A2B_RESULT adi_a2b_ConfigNetworkPowerMode(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{
    //uint8 nIndex;
    //uint32 nResult;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8  /*nSlaveIndex , */nMasterIndex,nNumSlave;
    uint16 nSlaveID;
    ADI_A2B_GRAPH *pgraph = (ADI_A2B_GRAPH *)pFrameWorkHandle->pgraph;
    TP_PRINTF("%s\r\n", __FUNCTION__);
    nMasterIndex = adi_a2b_getMasterNodeIndex(((pNode->nID) & 0xFF00u));

    nNumSlave = (uint8)(pgraph->oNode[nMasterIndex].nNumSlave);

    if( nNumSlave  > 1u)
    {
       /* Slave ID of the penultimate node */
        nSlaveID = ((pNode->nID) & 0xFF00u) + nNumSlave - 2u;
#if 0
        /* Re-configure */
        for (nIndex = 0u; nIndex < (nNumSlave - 1u) ; nIndex++)
        {
            nSlaveIndex = adi_a2b_getSlaveNodeIndex(nSlaveID);

            pNode = &pgraph->oNode[nSlaveIndex];
            
            /* Decrement slave ID */
            nSlaveID--;

            if ( pNode->bActive == 1u )
            {   
                /* Ensure node is in normal mode */ 
                pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL &= ((uint8)BITM_A2B_SWCTL_DIAGMODE ^0xFFu);
                eResult = adi_a2b_ConfigNodePowerMode(me, pNode,(uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_SWCTL);

            }

        }
#endif
    }

    nMasterIndex = adi_a2b_getMasterNodeIndex(((pNode->nID) & 0xFF00u));
    pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_SWCTL &= ((uint8)BITM_A2B_SWCTL_DIAGMODE ^0xFFu);
    eResult = adi_a2b_ConfigNodePowerMode(me, &pgraph->oNode[nMasterIndex],(uint8)(pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_SWCTL));

    (void)nSlaveID;
    
    return eResult;
}
/*****************************************************************************/
/*!
    @brief     This function returns the index of the master node in the graph structure

    @param [in] nNodeID    Node ID of the master
    
    <b> Global Variables Used: </b>
            - #variable oFramework.aSlaveNodeReferenceTable : Table to get master node index. 

    @return     uint8
                -Index of the slave node in gaGraphData
*/
/*****************************************************************************/
uint8 adi_a2b_getMasterNodeIndex(uint16 nNodeID)
{
   //uint8 nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
   //uint8 nMasterIndex  =  oFramework.aMasterNodeReferenceTable;
   return(oFramework.aMasterNodeReferenceTable);
}


/*****************************************************************************/
/*!
    @brief      This function gets the master node pointer

    @param [in]     NodeID          master node ID
    @param [in]     ppNode          Double pointer to the A2B node structure
    
    <b> Global Variables Used: </b>
                    - #variable oFramework.pgraph : Pointer to array of nodes(graph)

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success
*/
/*****************************************************************************/
void adi_a2b_getMasterNodePtr(uint16 NodeID ,ADI_A2B_NODE** ppNode)
{
   uint8 nMasterIndex = adi_a2b_getMasterNodeIndex(NodeID);
   *ppNode = &(oFramework.pgraph->oNode[nMasterIndex]);
}

/*****************************************************************************/
/*!
    @brief      This function returns the index/position of the given slave node(ID) in the graph structure.
                It uses the pre-computed table "aSlaveNodeReferenceTable"
                
    @param [in] nNodeID   Slave node ID
    
    <b> Global Variables Used: </b>
                - #variable oFramework.aSlaveNodeReferenceTable : Table to get node index. 

    @return     uint8
                -Index of the slave node in gaGraphData
*/
/*****************************************************************************/
uint8 adi_a2b_getSlaveNodeIndex(uint16 nNodeID)
{
   //uint8 nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
   uint8 nSlaveID = (uint8)((nNodeID & 0xFFu));
   //uint8 nSlaveIndex  =  oFramework.aSlaveNodeReferenceTable[nSlaveID];
   return(oFramework.aSlaveNodeReferenceTable[nSlaveID]);
}


/*****************************************************************************/
/*!
    @brief      This function gets the slave node pointer

    @param [in]     NodeID          Slave node ID
    @param [in]     ppNode          Double pointer to the A2B node structure

    <b> Global Variables Used: </b>
                - #variable oFramework.pgraph : Pointer to array of nodes(graph)

    @return     void
*/
/*****************************************************************************/
void adi_a2b_getSlaveNodePtr(uint16 NodeID ,ADI_A2B_NODE** ppNode)
{
   uint8 nSlaveIndex = adi_a2b_getSlaveNodeIndex(NodeID);
   *ppNode = &(oFramework.pgraph->oNode[nSlaveIndex]);
}

/*****************************************************************************/
/*!
    @brief      This function configures the network in order to address master node and returns I2C device address 
                to be used to program. 
                This function enables remote program option(PERI) in the branch orginating node
                
    @param [in] pNode       Pointer to the any node in the branch

    @return          uint8
                     -I2C address to be used for addressing
*/
/*****************************************************************************/
uint8 adi_a2b_masterPreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
#if 0
    
    return (sCommonSetting.nMasterI2CAddr);
#else
    uint8 nI2CAddr = 0xFFu/*,nMasterAddr,nBusAddr,nVal,nSlaveID*/;
    ADI_A2B_NODE /**pSrcNodePtr, */*pNodeBranchMaster;
    //uint32 nResult;

    /* Get the master */
    if(pNode->eType == (uint16)ADI_A2B_SLAVE)
    {
       adi_a2b_getMasterNodePtr(pNode->nID ,&pNodeBranchMaster );
    }
    else
    {
       pNodeBranchMaster = pNode;
    }

    /* Check whether a directly connected master */
    if(pNodeBranchMaster->nA2BSrcNodeID == 0xFFu)
    {
        pNodeBranchMaster->oProperties.nMasterI2CAddress = (sCommonSetting.nMasterI2CAddr);
       nI2CAddr =(uint8)pNodeBranchMaster->oProperties.nMasterI2CAddress;
    }
#if 0//A2B_BRANCH_SUPPORT    
    else
    {
        adi_a2b_getSlaveNodePtr(pNodeBranchMaster->nA2BSrcNodeID , &pSrcNodePtr);

        nMasterAddr =(uint8)pSrcNodePtr->oProperties.nMasterI2CAddress;
        nBusAddr   = (uint8)pSrcNodePtr->oProperties.nBusI2CAddress;

        nSlaveID = (uint8)(pSrcNodePtr->nID & 0xFFu);

        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nSlaveID);

        /* Write to CHIP address */
        nVal  = (uint8)pNodeBranchMaster->oProperties.nMasterI2CAddress;
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nBusAddr,(uint8)REG_A2B0_CHIP ,nVal);

         /* Enable remote I2C write */
        nVal  =  (uint8)(pSrcNodePtr->nID & 0xFFu) | (uint8)BITM_A2B_NODEADR_PERI;
        nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nVal);

        /* Return I2C address */
        nI2CAddr = nBusAddr;

    }
#endif
    return(nI2CAddr);
#endif

}



/*****************************************************************************/
/*! 
    @brief      This function enables audio data across A2B Bus

    @param [in] pNode           Pointer to the A2B node structure

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success  
*/ 
/*****************************************************************************/
static ADI_A2B_RESULT adi_a2b_EnableDataFlow(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint32 nResult = 0u;
    uint8 nCount = 0u;
    uint8 nIndex/*,nPeriIndex*/;
    uint8 nA2BI2CAddr;
    TP_PRINTF("%s\r\n", __FUNCTION__);

    /* Make sure enough memory is allocated */
    ADI_A2B_CONFIG_TABLE aTwiWriteTable[A2B_MAX_NUM_REG]; 
    
     /* Preset to write to master */
    nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);
        
    if( (pNode->bActive) == 1u)
    {
        /* For auto-broadcast */
        aTwiWriteTable[nCount].nValue =  (uint8)(0x00u) ;
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_NODEADR;

        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_SLOTFMT);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_SLOTFMT;
      
        aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nCTLRegister.nREG_A2B0_DATCTL );
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_DATCTL; 

#if     A2B_ENABLE_AD242X_SUPPORT
        if(pNode->ePartNum == ADI_A2B_AD2425)
        {
			aTwiWriteTable[nCount].nValue  = (uint8)(pNode->oProperties.nI2CI2SRegister.nREG_AD242X0_I2SRRATE);
			aTwiWriteTable[nCount++].nAddress =(uint8)REG_AD242X0_I2SRRATE;
        }
#endif
        /* Apply new structure(shadow -> actual)  */
        /* Enable control*/
        aTwiWriteTable[nCount].nValue  = (uint8)(0x01u);
        aTwiWriteTable[nCount++].nAddress =(uint8)REG_A2B0_CONTROL; 
        
        
        for(nIndex  =0u ; nIndex < nCount ;nIndex++)
        {
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)aTwiWriteTable[nIndex].nAddress ,aTwiWriteTable[nIndex].nValue);
            if(nResult !=0u)
            {
                break;
            }
            
            
        }
    }
    else
    {
        nResult = 1u;
    }
    
    /* Check for failure */
    if(nResult!= 0u )
    {
        eResult = ADI_A2B_FAILURE;
    } 
    
#if A2B_PRINT_FOR_DEBUG
    if( nResult == 0u)
    {
        printf("Branch %d : Data Flow Enable : SUCCESS\n  ",((pNode->nID & 0xFF00u) >> 8u) );
    }
    else
    {
        printf("Branch %d : Data Flow Enable : FAILURE \n",((pNode->nID & 0xFF00u) >> 8u) );
    }
#endif     
     
    return(eResult);
}

/*****************************************************************************/
/*!
    @brief        This function clears the configurations made to address a node.
                  In case of branch node,it disables remote program option in the branch orginating slave node. 

    @param [in] pNode     Pointer to the any node in the branch


    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success
*/
/*****************************************************************************/
void adi_a2b_clearPreset(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode)
{
    uint8 /*nI2CAddr,nBusAddr,nVal*/nMasterAddr;
    ADI_A2B_NODE *pSrcNodePtr, *pNodeBranchMaster;
    uint32 nResult;
    //TP_PRINTF("Enter %s\r\n", __FUNCTION__);

    /* Get the master */
    if(pNode->eType == (uint16)ADI_A2B_SLAVE)
    {
       adi_a2b_getMasterNodePtr(pNode->nID ,&pNodeBranchMaster );
    }
    else
    {
       pNodeBranchMaster = pNode;
    }
    /* Check whether a directly connected master */
    if(pNodeBranchMaster->nA2BSrcNodeID != 0xFFu)
    {
       adi_a2b_getSlaveNodePtr(pNodeBranchMaster->nA2BSrcNodeID , &pSrcNodePtr);
       nMasterAddr = (uint8)pSrcNodePtr->oProperties.nMasterI2CAddress;
       nResult = (uint32)adi_a2b_TwiWrite8(me,A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,0x00u);
    }

    (void)nResult;
}


/****************************************************************************/
/*!
    @brief          This function configures devices connected to slave node
                    through remote I2C

    @param [in]     pNode                   Pointer to A2B node
    @param [in]     psDeviceConfig          Pointer to peripheral device configuration structure

    @return          Return code
                    - 0: Success
                    - 1: Failure
*/
/********************************************************************************/
static uint32 adi_a2b_RemoteDeviceConfig(ADI_A2B_NODE* pNode, ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig , uint8 nTWIdevNo)
{
    return (0);
}

/****************************************************************************/
/*!
    @brief          This function selects the matching peripheral configuration structure
                    by comparing the device address

    @param [in]     pNode                   Pointer to A2B node
    @param [in]     pDevice                 Pointer to peripheral device in the graph 
    @param [in]     sPeriConfig             Array of pointers to all the peripherals connected to the slave node

    @return         nReturn  - 0 Successful
    						   1 Failure  
*/
/********************************************************************************/
uint8 adi_a2b_DeviceConfig(ADI_A2B_NODE* pNode, ADI_A2B_CONNECTED_DEVICE* pDevice , ADI_A2B_NODE_PERICONFIG sPeriConfig)
{
    uint8 nReturn = 0xFFu;
    uint8 nIndex;
    ADI_A2B_PERI_DEVICE_CONFIG* psDeviceConfig;
    uint8 nDeviceAddr = (uint8)pDevice->nI2CAddress;

    /* Check with exported peripheral configuration */
    for(nIndex  = 0u; nIndex < (uint8)sPeriConfig.nNumConfig; nIndex++)
    {
      psDeviceConfig = &sPeriConfig.aDeviceConfig[nIndex];
      
      if(psDeviceConfig->nDeviceAddress == nDeviceAddr)
      {
          nReturn = (uint8)adi_a2b_RemoteDeviceConfig(pNode,psDeviceConfig,A2B_TWI_NO);
          break;
      }
    }

    /* Check the return code */
    if(nReturn == 0xFFu)
    {
        /* No configuration found */
        pDevice->bActive = 0u;
        nReturn = 0u;
    }
    else if( nReturn == 1u)
    {
         /* I2C failure  */
        pDevice->bActive = 0xFFu;
        nReturn = 1u;
    }
    else
    {
        /* Successful configuration */
        pDevice->bActive = 1u;
        nReturn = 0u;
    }              
        
    return( nReturn);

}


/****************************************************************************/
/*!
    @brief          This function configures/programs peripherals connected 
                    to the slave node.(remote I2C)
     
    @param [in]     pNode                 Pointer to node structure
  
    
    @return          Return code
                    - 0: Success
                    - 1: Failure
*/                    
/********************************************************************************/ 
ADI_A2B_RESULT adi_a2b_PeriheralConfig(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , ADI_A2B_NODE_PERICONFIG sPeriConfig )
{
    uint32 nResult = 0u,nReturn;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8 nVal,bErrFlag =0u, nMask2Val;
    uint8 nA2BI2CAddr;
    uint8 nExpVersionID = (uint8)pNode->oProperties.nIDRegister.nREG_A2B0_VERSION;
    
#if A2B_PRINT_FOR_DEBUG
        printf(" Starting peripheral device configuration for slave node %d : \n",pNode->nID );    
#endif 
    
     /* Preset to write to master */
    nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

#if REV_1_0_WORKAROUND

 if(nExpVersionID == 0x10u )
 {
    /* Work around for remote I2C read/write failure with slave interrupts */
    nMask2Val = 0u;
    nResult = adi_a2b_TwiRead8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_INTMSK2,(uint8*)&nMask2Val);
    /* Disable slave interrupts */    
    nVal = nMask2Val & (uint8)(((uint8)BITM_A2B_INTPND2_SLVIRQ) ^ 0xFFu);
    nResult = adi_a2b_TwiWrite8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_INTMSK2,(uint8)nVal); 
 }
    
#endif
    
    /* Addressing slave node */
    nVal  = (uint8)(pNode->nID);
    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
#if 0     
    for(i = 0u;i < (uint8)ADI_A2B_MAX_DEVICES_PER_NODE;i++)
    {
        
        pNode->oConnectedTo[i].bActive  = 1u;

        if( (uint32)pNode->oConnectedTo[i].eDeviceID != (uint32)UNKNOWN)
        {
        
            if( nResult == 0u)
            {
                 /* Preset to write to master */
                nA2BI2CAddr = adi_a2b_slavePreset(me,pNode);

                /* Write to CHIP address */
                nVal  = (uint8)(uint8)pNode->oConnectedTo[i].nI2CAddress;
                nResult = (uint32)adi_a2b_TwiWrite8(me,A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_CHIP ,nVal);
            }

            if(nResult == 0u)
            {
                 /* Preset to write to master */
                nA2BI2CAddr = adi_a2b_masterPreset(me, pNode);

                /* Enable remote I2C write */
                nVal  =  (uint8)(pNode->nID) | (uint8)BITM_A2B_NODEADR_PERI;
                nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
            } 
            /* Program through remote I2C*/
            if(nResult == 0u)
            {
                
                nResult = (uint32)adi_a2b_DeviceConfig(pNode,&(pNode->oConnectedTo[i]),sPeriConfig);
            }
            /* Assign status */ 
            bErrFlag |= (uint8)nResult;

             /* Preset to write to master */
            nA2BI2CAddr = adi_a2b_masterPreset(me,pNode);

            /* Disable  remote I2C write */
            nVal  = (uint8)(pNode->nID);
            nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nA2BI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
      
        }
    }
#endif
	/* Check for failure*/
    eResult = (ADI_A2B_RESULT) bErrFlag;
 
  
#if REV_1_0_WORKAROUND

 if(nExpVersionID == 0x10u )
 {
   /* Enable slave interrupts */   
    nReturn = adi_a2b_TwiWrite8(me, A2B_TWI_NO,nA2BI2CAddr,(uint8)REG_A2B0_INTMSK2,(uint8)nMask2Val); 
 }
#endif         
    
#if A2B_PRINT_FOR_DEBUG
    if( nResult == 0u)
    {
        printf("  Ending peripheral device configuration for  slave node %d \n",pNode->nID );    
    }
#endif 
    
    (void)nReturn;
    (void)nResult;

    return eResult; 
} 



/****************************************************************************/
/*!
    @brief          This function configures a slave node( transceiver + peripheral) either from EEPROM or graph

    @param [in]     pNode               Pointer to A2B slave node
    @param [in]     pFrameWorkHandle    Pointer to framework  configuration structure
    
    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success
*/
/*****************************************************************************/
static ADI_A2B_RESULT adi_a2b_ConfigureNode(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{

    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8 nNodePositionID , nBranchID;
    //ADI_A2B_GRAPH *pgraph = (ADI_A2B_GRAPH *)pFrameWorkHandle->pgraph;
    

    if ( (pNode->eType == (uint16)ADI_A2B_SLAVE ) && ( pNode->bActive == 1u ))
    {
        /* Auto configuration check */
        if(pNode->bEnableAutoConfig == 0u)
        {
            /* Configure AD2410 registers */
            eResult = adi_a2b_SlaveConfig(me, pNode);

            /* Slave peripheral configuration */
            if (eResult == ADI_A2B_SUCCESS)
            {
                nBranchID =  (uint8) ( (pNode->nID & 0xFF00u) >> 8u );
                nNodePositionID = (uint8)(pNode->nID & 0xFFu) + 1u;
                eResult = adi_a2b_PeriheralConfig(me, pNode , pFrameWorkHandle->aPeriDownloadTable[nNodePositionID]);
            }
        }
#if 0
        else
        {
            eResult = adi_a2b_NodeConfigFromEEPROM(pNode, A2B_EEPROM_I2C);
        }
#endif
        /* Error message for debug */
        if(eResult != ADI_A2B_SUCCESS)
        {
            TP_PRINTF("Configuration Node Fail");
            //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].eError = ADI_A2B_SLAVE_CONFIGURATION_ERROR;
            //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].nNodeID = (pNode->nID);
            //pFrameWorkHandle->nErrorCount++;
            //pFrameWorkHandle->nErrorCount = (pFrameWorkHandle->nErrorCount) % (uint32)A2B_MAX_NUM_ERROR;
        }

    }

    (void)nBranchID;
    
    return(eResult);
}

static ADI_A2B_RESULT adi_a2b_ReConfigSlot(cAdiAD2410Drv* me, ADI_A2B_NODE *pNode , uint8 bSlotCalc, ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{
    uint32 nIndex,nResult;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS;
    uint8 nVal , nSlaveIndex , nMasterIndex,nNumSlave , nI2CAddr,nUpslots = 0u,nDnslots = 0u,nMaxBCDSlots = 0u;
    uint16 nSlaveID;
    //ADI_A2B_TRGT_PROPERTY_HANDLER* pTrgtPropHandle = &(pFrameWorkHandle->oTrgtProprty);
    ADI_A2B_GRAPH *pgraph = (ADI_A2B_GRAPH *)pFrameWorkHandle->pgraph;
    TP_PRINTF("%s\r\n", __FUNCTION__);

    nMasterIndex = adi_a2b_getMasterNodeIndex(((pNode->nID) & 0xFF00u));
    nNumSlave = (uint8)((pNode->nID & 0x00FFu) + 1u);
    nSlaveID = (pNode->nID);

    if(pNode->bActive == 1u)
    {
	    nUpslots = (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS;
	    nDnslots = (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS;
    }
     
    /* Decrement slave ID */
    nSlaveID--;

    /* Re-configure */
    for (nIndex = 0u; nIndex < (nNumSlave - 1u); nIndex++)
    {
        nSlaveIndex = adi_a2b_getSlaveNodeIndex(nSlaveID);
        pNode = &pgraph->oNode[nSlaveIndex];
        
        /* Decrement slave ID */
        nSlaveID--;

        if ( pNode->bActive == 1u )
        {
            /* Master Pre-set */
            nI2CAddr = adi_a2b_masterPreset(me, pNode);

            /* Enable remote I2C write */
            nVal  =  (uint8)(pNode->nID);
            nResult = (uint32)adi_a2b_TwiWrite8(me,A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);

            /* Slave Pre-set */
            nI2CAddr = adi_a2b_slavePreset(me,pNode);
           
            /* Slot calculation */
            if (bSlotCalc == 1u)
            {
                /* Change slave slots */
                nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_DNSLOTS ,nDnslots);
                nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_UPSLOTS ,nUpslots);

                pNode->oProperties.nCTLRegister.nREG_A2B0_UPSLOTS = (uint16)nUpslots;
                pNode->oProperties.nCTLRegister.nREG_A2B0_DNSLOTS = (uint16)nDnslots;

                /* Initialize and configure slave nodes */
                nUpslots += (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_LUPSLOTS;
                nDnslots += (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_LDNSLOTS;

                if(nMaxBCDSlots < pNode->oProperties.nCTLRegister.nREG_A2B0_BCDNSLOTS )
                {
                   nMaxBCDSlots = (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_BCDNSLOTS;
                }
            }
            else
            {
                nUpslots = (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_UPSLOTS;
                nDnslots = (uint8)pNode->oProperties.nCTLRegister.nREG_A2B0_DNSLOTS;

                /* Change slave slots */
                nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_DNSLOTS ,nDnslots);
                nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_UPSLOTS ,nUpslots);
                
            }

        }
    }

    if( bSlotCalc == 1u)
    {
		/* Re-configure master slots */
		pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_DNSLOTS = (uint16)nDnslots + (uint16)nMaxBCDSlots;
		pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_UPSLOTS =  (uint16)(nUpslots);
    }

    /* Master Pre-set */
    nI2CAddr = adi_a2b_masterPreset(me, pNode);
    
    /* Write to master */
    nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_DNSLOTS ,(uint8)(pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_DNSLOTS));
    nResult = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, nI2CAddr,(uint8)REG_A2B0_UPSLOTS ,(uint8)(pgraph->oNode[nMasterIndex].oProperties.nCTLRegister.nREG_A2B0_UPSLOTS));
    
    (void)nResult;

    return eResult;
}


/****************************************************************************/
/*!
    @brief          This function is responsible for configuring nodes after discovery.

    @param [in]     pNodeLast               Pointer to the A2B node structure
    @param [in]     pFrameWorkHandle    Pointer to framework configuration structure

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success

*/
/*****************************************************************************/
static ADI_A2B_RESULT adi_a2b_ConfigureNodesPostDiscovery(cAdiAD2410Drv* me, ADI_A2B_NODE *pNodeLast , ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{
    uint32 nResult;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS,eRes;
    uint8  nSlaveIndex , nMasterIndex,nNumSlave , bSlotCalc = 0u;
    uint16 nSlaveID, nIndex;
    ADI_A2B_TRGT_PROPERTY_HANDLER* pTrgtPropHandle = &(pFrameWorkHandle->oTrgtProprty);  
    ADI_A2B_GRAPH *pgraph = (ADI_A2B_GRAPH *)pFrameWorkHandle->pgraph;
    ADI_A2B_NODE *pNode;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    
    /* Get master node index */
    nMasterIndex = adi_a2b_getMasterNodeIndex(((pNodeLast->nID) & 0xFF00u));
    
	/* Get the ID for the first slave */
	nSlaveID = ((pNodeLast->nID) & 0xFF00u);
	nNumSlave = (uint8)(pgraph->oNode[nMasterIndex].nNumSlave);

	/* Configure all active nodes */
	for (nIndex = (nNumSlave); nIndex > 0; nIndex--)
	{
		nSlaveIndex = adi_a2b_getSlaveNodeIndex(nSlaveID + nIndex - 1u);

		pNode = &pgraph->oNode[nSlaveIndex];

		/* Calculate pass up and pass down slots when one of the node is auto configured*/
		if( pNode->bEnableAutoConfig == 1u)
		{
			bSlotCalc = 1u;
		}

		/* In simple discovery flow, all discovered slave nodes will be configured one after the other */
		if ( pTrgtPropHandle->eDiscvryType  == ADI_A2B_SIMPLE_DISCOVERY )
		{
			eResult = adi_a2b_ConfigureNode(me, pNode,pFrameWorkHandle );
		}

	}
	/* Configure last slave- Modified, optimized and advanced */
	if( pTrgtPropHandle->eDiscvryType  != ADI_A2B_SIMPLE_DISCOVERY )
    {
        eResult = adi_a2b_ConfigureNode(me, pNodeLast,pFrameWorkHandle );
    }
	else
	{
        eResult = adi_a2b_masterConfig(me, &(pFrameWorkHandle->pgraph->oNode[nMasterIndex]));
	}
	/* Slot calculation for advanced discovery */
	if( pTrgtPropHandle->eDiscvryType == ADI_A2B_ADVANCED_DISCOVERY)
	{
		bSlotCalc = 1u;
	}

    /* Pass up and pass down slot configuration */
    eRes = adi_a2b_ReConfigSlot(me, pNodeLast , bSlotCalc, pFrameWorkHandle );

    /* Start timer  */
    pFrameWorkHandle->sTimerHandle.eContext = ADI_A2B_POST_DISCOVERY;
    nResult = adi_a2b_TimerStart(A2B_TIMER_NO,A2B_POST_DISC_TIME_INTERVAL);
               
    (void)nResult;
    (void)eRes;
    
    return eResult;
}


/****************************************************************************/
/*!
@brief          This function validates the input Graph/schematic.
                Entry point function for node discovery and network configuration

@param [in]     pgraph              Pointer to the A2B graph structure
@param [in]     pFrameWorkHandle    Pointer to framework configuration structure

@return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success


*/
/*****************************************************************************/
ADI_A2B_RESULT adi_a2b_ValidateNetwork(cAdiAD2410Drv* me, ADI_A2B_GRAPH *pgraph,ADI_A2B_FRAMEWORK_HANDLER* pFrameWorkHandle )
{
    uint8 i;
    ADI_A2B_RESULT eResult = ADI_A2B_SUCCESS,eRes;
    uint8 nDiscIndex,nConfigIndex;
    uint8 /*nI2CAddr,*/nIndex;
    //uint32 nNodeId,nResult = 0u;
    TP_PRINTF("Enter %s\r\n", __FUNCTION__);
    pgraph->NumberOfSlaveDiscovered = 0;

    for (i = 0u; i < (uint8)pgraph->nNodeCount; i++)
    {
        if ((pgraph->oNode[i].eType == (uint16)ADI_A2B_MASTER)) /* the node is a2b master */
        {

            /* Configure master for discovery*/
            eResult = adi_a2b_masterConfigForDiscovery(me, &pgraph->oNode[i]);

            /* Log error message*/
            if( eResult != ADI_A2B_SUCCESS)
            {
                ASSERT(0);
                //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].eError = ADI_A2B_MASTER_CONFIGURATION_ERROR;
                //pFrameWorkHandle->sErrorMessage[pFrameWorkHandle->nErrorCount].nNodeID = pgraph->oNode[i].nID;
                //pFrameWorkHandle->nErrorCount++;
                //pFrameWorkHandle->nErrorCount = (pFrameWorkHandle->nErrorCount) % (uint32)A2B_MAX_NUM_ERROR;
            }
            else
            {
                /* Make node active */
                pgraph->oNode[i].bActive = (uint16)1u;

                nConfigIndex = i;

                for(nIndex = 0u; (nIndex < pgraph->oNode[i].nNumSlave) && (eResult == ADI_A2B_SUCCESS) ;nIndex++ )
                {
                     nDiscIndex = adi_a2b_getSlaveNodeIndex(pgraph->oNode[i].nID + nIndex );
                     /* Discovery starts */
                     eResult =  adi_a2b_EnableNodeDiscovery(me, &pgraph->oNode[nDiscIndex],&pgraph->oNode[nConfigIndex],pFrameWorkHandle);
                     nConfigIndex = nDiscIndex;
                }

                /* Configure last node       - optimized, modified and Advanced discovery flow */
                /* Configure all slave nodes -  simple discovery flow */
                eRes = adi_a2b_ConfigureNodesPostDiscovery(me, &pgraph->oNode[nConfigIndex],  pFrameWorkHandle );

                /* Re-configure switch mode after discovery  */
                eRes = adi_a2b_ConfigNetworkPowerMode(me, &pgraph->oNode[i],pFrameWorkHandle );

                /* Enable data flow */
                eRes = adi_a2b_EnableDataFlow(me, &pgraph->oNode[i]);

                adi_a2b_clearPreset(me, &pgraph->oNode[i]);

                /* Check for failure */
                if(eRes == ADI_A2B_FAILURE)
                {
                    eResult = ADI_A2B_FAILURE;
                }
            }
        }
    }

    return eResult;
}

#if 0
static uint32 adi_a2b_LoadTWIBuffer( uint8 nTWIData[] , ADI_A2B_TWI_ADDR_CONFIG sRegConfig ,ADI_A2B_TWI_DATA_CONFIG sDataConfig)
{
    uint32 nCount = 0u, nIndex;
    uint8 *paConfigDataBuff;

    /* Checking the address length */
    switch(sRegConfig.nRegAddrLen)
    {
        case 1u:
            nTWIData[nCount++] =  (uint8)(sRegConfig.nRegVal & 0xFFu);
            break;
        case 2u:
            nTWIData[nCount++] =  (uint8)( (sRegConfig.nRegVal  & 0xFF00u)>>8u );
            nTWIData[nCount++] =  (uint8)(sRegConfig.nRegVal  & 0xFFu);
            break;
        case 4u:
            nTWIData[nCount++] =  (uint8)((sRegConfig.nRegVal  & 0xFF000000u)>>24u);
            nTWIData[nCount++] =  (uint8)((sRegConfig.nRegVal  & 0xFF0000u)>>16u);
            nTWIData[nCount++] =  (uint8)((sRegConfig.nRegVal  & 0xFF00u)>>8u);
            nTWIData[nCount++] =  (uint8)(sRegConfig.nRegVal  & 0xFFu);
            break;
        default:
            break;
    }

    paConfigDataBuff = sDataConfig.paConfigData;

    /*Checking the data length*/
    switch(sDataConfig.nDataLen)
    {
        case 1u:
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount ; nIndex++)
            {
                nTWIData[nCount++] =  (uint8)(paConfigDataBuff[nIndex] & 0xFFu);
            }
            break;
        case 2u:
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount ; nIndex++)
            {
                nTWIData[nCount++] =  (uint8)((paConfigDataBuff[nIndex] & 0xFF00u)>>8u);
                nTWIData[nCount++] =  (uint8)(paConfigDataBuff[nIndex] & 0xFFu);
            }
            break;
        case 4u:
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount ; nIndex++)
            {
                nTWIData[nCount++] =  (uint8)((paConfigDataBuff[nIndex] & 0xFF000000u)>>24u);
                nTWIData[nCount++] =  (uint8)((paConfigDataBuff[nIndex] & 0xFF0000u)>>16u);
                nTWIData[nCount++] =  (uint8)((paConfigDataBuff[nIndex] & 0xFF00u)>>8u);
                nTWIData[nCount++] =  (uint8)(paConfigDataBuff[nIndex] & 0xFFu);
            }
            break;
        default:
            break;
    }

    return nCount;

}

static void adi_a2b_ByteToWord(uint8 pSrcBuf[], uint32 pDstBuf[] ,uint32 nDatawidth,uint32 nDataCount )
{
    uint32 nIndex;

    /* Store the read values in the place holder */
    switch(nDatawidth)
    {   /* Byte */
        case 1u:
            for(nIndex = 0u; nIndex < nDataCount;nIndex++)
            {
                pDstBuf[nIndex ]  =  pSrcBuf[nIndex];
            }
            break;
         /* 16 bit word*/
        case 2u:
            for(nIndex = 0u; nIndex < nDataCount;nIndex++)
            {
                pDstBuf[nIndex]  =  (uint32)((pSrcBuf[2u*nIndex + 1u]));
                pDstBuf[nIndex] |=  (uint32)((uint32)pSrcBuf[2u*nIndex] << 8u);
            }
            break;
        /* 32 bit word */
        case 4u:
            for(nIndex = 0u; nIndex < nDataCount;nIndex++)
            {
                pDstBuf[nIndex]  =  (uint32)((pSrcBuf[4u*nIndex + 3u]));
                pDstBuf[nIndex] |=  (uint32)((uint32)pSrcBuf[4u*nIndex + 2u] << 8u);
                pDstBuf[nIndex] |=  (uint32)((uint32)pSrcBuf[4u*nIndex + 1u] << 16u);
                pDstBuf[nIndex] |=  (uint32)((uint32)pSrcBuf[4u*nIndex] << 24u);
            }
            break;
        default:
            break;
    }
}

static void adi_a2b_StoreBlockRead( uint8 aTWIReadBuff[] , ADI_A2B_TWI_DATA_CONFIG sDataConfig)
{
    
    uint32 nIndex;
    uint8 *paConfigData = sDataConfig.paConfigData;
    
    /* Store the read values in the place holder */
    switch(sDataConfig.nDataLen)
    {
        case 1u:
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount;nIndex++)
            {
                paConfigData[nIndex ]  =  aTWIReadBuff[nIndex];
            }    
            break;
        case 2u:   
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount;nIndex++)
            {
                paConfigData[nIndex]  =  (uint32)((aTWIReadBuff[2u*nIndex + 1u]));
                paConfigData[nIndex] |=  (uint32)((uint32)aTWIReadBuff[2u*nIndex] << 8u);                        
            }
            break;
        case 4u:
            for(nIndex = 0u; nIndex < sDataConfig.nDataCount;nIndex++)
            {
                paConfigData[nIndex]  =  (uint32)((aTWIReadBuff[4u*nIndex + 3u]));
                paConfigData[nIndex] |=  (uint32)((uint32)aTWIReadBuff[4u*nIndex + 2u] << 8u);
                paConfigData[nIndex] |=  (uint32)((uint32)aTWIReadBuff[4u*nIndex + 1u] << 16u);
                paConfigData[nIndex] |=  (uint32)((uint32)aTWIReadBuff[4u*nIndex] << 24u);   
            }          
            break;

        default:
            break;
    }
}       
#endif

static uint32 adi_a2b_TwiGenericBlockWrite(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nDeviceAddress, uint32 nCount, uint8* dataPtr)
{
    uint32 nReturnValue = 0u;    
    //uint32 nIndex;
    //ADI_TWI_RESULT eTwiResult = ADI_TWI_SUCCESS;

    nReturnValue = ((uint8)TRUE);
   

    nReturnValue= adi_a2b_I2cWrite(me, nDeviceAddress, nCount, dataPtr);

    return(nReturnValue);        
}

uint32 adi_a2b_TwiGenericBlockRead(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nDeviceAddress, uint8 regAddr, uint32 nCount, uint8* dataPtr)
{
    uint32 nReturnValue = 0u;    
  //ADI_TWI_RESULT eTwiResult = ADI_TWI_SUCCESS;

    /* Get register address */
    nReturnValue= adi_a2b_I2cRead(me, nDeviceAddress, dataPtr, regAddr, nCount);
    return(nReturnValue);        
}


/****************************************************************************/
/*!
    @brief          This function is responsible for writing slave A2B chip .

    @param [in]     nTWIDeviceNo           Slave Node number, as first slave node is zero
    @param [in]     nDeviceAddress         I2C device address of the peripheral connect to AD24xx, eg. MCU in slave mode is 0x65
    @param [in]     pOPUnit                config data which is either WRITE/READ in below format
                                ADI_A2B_PERI_CONFIG_UNIT temp_peri= 	{A2B_WRITE_OP,	1,	0x01,	0x1u,	0x2u,	&tmp_dat[0]};
    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success
*/
/*****************************************************************************/
uint32 adi_a2b_slave_peripheral_config(cAdiAD2410Drv* me, uint8 nTWIDeviceNo, uint8 nDeviceAddress,  ADI_A2B_PERI_CONFIG_UNIT* pOPUnit)
{
    uint32 nReturn;
    /* Addressing slave node */
    uint8 nVal;
    //uint32 nCount = 0u;
    ADI_A2B_TWI_DATA_CONFIG sTwiDataConfig;
    ADI_A2B_TWI_ADDR_CONFIG sTwiAddrConfig;
    //static uint8 aDataBuffer[32];
    uint8 nTWIData[MAX_NUMBER_TWI_BYTES];
    uint32 nDelayVal;    
    nVal  = nTWIDeviceNo;

    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nMasterI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);

    nVal  = nDeviceAddress;
    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nBusI2CAddr,(uint8)REG_A2B0_CHIP ,nVal);
    
    nVal  =  (uint8)nTWIDeviceNo | (uint8)BITM_A2B_NODEADR_PERI;
    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nMasterI2CAddr,(uint8)REG_A2B0_NODEADR ,nVal);
    
    /* Operation code*/
    switch(pOPUnit->eOpCode)
    {
       /* write */
        case 0u: //i_a2b_ByteToWord(pOPUnit->paConfigData,(uint32*)aDataBuffer, pOPUnit->nDataWidth, pOPUnit->nDataCount);
            sTwiDataConfig.nDataLen = pOPUnit->nDataWidth;
            sTwiDataConfig.nDataCount = (pOPUnit->nDataCount+1);
            sTwiDataConfig.paConfigData = nTWIData;
            sTwiAddrConfig.nRegAddrLen = pOPUnit->nAddrWidth;
            sTwiAddrConfig.nRegVal = pOPUnit->nAddr;
            nTWIData[0] = sTwiAddrConfig.nRegVal;
            for(nVal=1; nVal<=sTwiDataConfig.nDataCount; nVal++)
            {
                nTWIData[nVal] = pOPUnit->paConfigData[nVal-1];
            }
            nReturn =  adi_a2b_TwiGenericBlockWrite(me, A2B_TWI_NO,sCommonSetting.nBusI2CAddr,sTwiDataConfig.nDataCount,sTwiDataConfig.paConfigData);
            break;
        /* read */
        case 1u: 
            sTwiDataConfig.nDataLen = pOPUnit->nDataWidth;
            sTwiDataConfig.nDataCount = pOPUnit->nDataCount;
            sTwiDataConfig.paConfigData = pOPUnit->paConfigData;

            sTwiAddrConfig.nRegAddrLen = pOPUnit->nAddrWidth;
            sTwiAddrConfig.nRegVal = pOPUnit->nAddr;

            nReturn = adi_a2b_TwiGenericBlockRead(me, A2B_TWI_NO,sCommonSetting.nBusI2CAddr,(uint8)sTwiAddrConfig.nRegVal, sTwiDataConfig.nDataCount,sTwiDataConfig.paConfigData);
            break;
        /* delay */
        case 2u:
            nDelayVal = *((uint32*)(pOPUnit->paConfigData));
            adi_a2b_Delay(nDelayVal);
            break;
        default: 
            break;
    }
    //nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nBusI2CAddr,(uint8)REG_A2B0_CHIP ,nVal);
    return(nReturn);        
}


/****************************************************************************/
/*!
    @brief          This function is responsible for writing slave A2B chip .

    @param [in]     nTWIDeviceNo           Slave Node number, as first slave node is zero
    @param [in]     nRegAddress             Register address of the AD24xx
    @param [in]     nData                       Data Value write to this register.

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success

*/
/*****************************************************************************/
/*
static uint32 adi_a2b_slave_I2C_Write(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nRegAddress,uint8 nData)
{
    uint32 nReturn;
    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nMasterI2CAddr,(uint8)REG_A2B0_NODEADR ,nTWIDeviceNo);

    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nBusI2CAddr,(uint8)nRegAddress ,nData);
    return nReturn;
}
*/
/****************************************************************************/
/*!
    @brief          This function is responsible for reading slave A2B chip .

    @param [in]     nTWIDeviceNo           Slave Node number, as first slave node is zero
    @param [in]     nRegAddress             Register address of the AD24xx
    @param [in]     pData                       pointer to the Value read from the register.

    @return         ADI_A2B_RESULT
                    - ADI_A2B_FAILURE : Failure
                    - ADI_A2B_SUCEESS : Success

*/
/*****************************************************************************/
/*
static uint32 adi_a2b_slave_I2C_Read(cAdiAD2410Drv*me, uint8 nTWIDeviceNo, uint8 nRegAddress,uint8* pData)
{
    uint32 nReturn;

    //TODO: add some warning when node number is larger than number of node
    nReturn = (uint32)adi_a2b_TwiWrite8(me, A2B_TWI_NO, sCommonSetting.nMasterI2CAddr,(uint8)REG_A2B0_NODEADR ,nTWIDeviceNo);

    nReturn = (uint32)adi_a2b_TwiRead8(me, A2B_TWI_NO, sCommonSetting.nBusI2CAddr,(uint8)nRegAddress ,pData);
    return nReturn;
}
*/
