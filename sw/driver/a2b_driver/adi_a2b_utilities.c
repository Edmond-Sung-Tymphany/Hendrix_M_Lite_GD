/*******************************************************************************
Copyright (c) 2014 - Analog Devices Inc. All Rights Reserved.
This software is proprietary & confidential to Analog Devices, Inc.
and its licensors.
******************************************************************************

   Name       : adi_a2b_utilities.c
   
   Description: This file contains utility functions that are used to configure multi-master system
                
                 
   Functions  :     adi_a2b_masterPreset()
                    adi_a2b_slavePreset()
                    adi_a2b_getMasterNodePtr()
                    adi_a2b_getSlaveNodePtr()
                    adi_a2b_getSlaveNodeIndex()
                    adi_a2b_getMasterNodeIndex()
                    adi_a2b_UpdateNodeReferenceTable()
                 

   Prepared &
   Reviewed by: Automotive Software and Systems team, 
                IPDC, Analog Devices,  Bangalore, India
                
   $Date: 2015-07-29 08:44:17 +0530 (Wed, 29 Jul 2015) $
               
******************************************************************************/
/*! \addtogroup Target_Independent Target Independent 
 *  @{
 */
 


/*============= I N C L U D E S =============*/

#include "adi_a2b_datatypes.h"
#include "adi_a2b_graphdata.h"
#include "adi_a2b_framework.h"
#include "adi_a2b_externs.h"
#include "adi_a2b_driverprototypes.h"
#include "string.h"
#include "adiAD2410.h"

#if A2B_PRINT_FOR_DEBUG
#include <stdio.h>
#endif
/*============= D E F I N E S =============*/
/*
** Constants: First set of constants for module Y
** MYDEFINE - definitions My define
*/

/*============= D A T A =============*/



/*============= C O D E =============*/


/*
** Function Prototype section
*/

/*
** Function Definition section
*/

/** @defgroup Utility_Functions
 *
 * This module contains pre-requisite operations to address branch A2B node.
 * It also has utility functions to get the node position in the graph array.
 *
 */

/*! \addtogroup Utility_Functions Utility Functions
 *  @{
 */
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
uint8 adi_a2b_masterPreset(ADI_A2B_NODE *pNode)
{
    uint8 nI2CAddr = 0xFFu,nMasterAddr,nBusAddr,nVal,nSlaveID;
    ADI_A2B_NODE *pSrcNodePtr, *pNodeBranchMaster;
    uint32 nResult;

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
       nI2CAddr =(uint8)pNodeBranchMaster->oProperties.nMasterI2CAddress;
    }
#if A2B_BRANCH_SUPPORT    
    else
    {
        adi_a2b_getSlaveNodePtr(pNodeBranchMaster->nA2BSrcNodeID , &pSrcNodePtr);

        nMasterAddr =(uint8)pSrcNodePtr->oProperties.nMasterI2CAddress;
        nBusAddr   = (uint8)pSrcNodePtr->oProperties.nBusI2CAddress;

        nSlaveID = (uint8)(pSrcNodePtr->nID & 0xFFu);

        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nSlaveID);

        /* Write to CHIP address */
        nVal  = (uint8)pNodeBranchMaster->oProperties.nMasterI2CAddress;
        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nBusAddr,(uint8)REG_A2B0_CHIP ,nVal);

         /* Enable remote I2C write */
        nVal  =  (uint8)(pSrcNodePtr->nID & 0xFFu) | (uint8)BITM_A2B_NODEADR_PERI;
        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nVal);

        /* Return I2C address */
        nI2CAddr = nBusAddr;

    }
#endif

    return(nI2CAddr);

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
void adi_a2b_clearPreset(ADI_A2B_NODE *pNode)
{
    uint8 nI2CAddr,nMasterAddr,nBusAddr,nVal;
    ADI_A2B_NODE *pSrcNodePtr, *pNodeBranchMaster;
    uint32 nResult;

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
       nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,0x00u);
    }

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
uint8 adi_a2b_slavePreset(ADI_A2B_NODE *pNode)
{
    uint8 nI2CAddr = 0xFFu,nMasterAddr,nBusAddr,nVal,nSlaveID;
    ADI_A2B_NODE *pSrcNodePtr,*pNodeBranchMaster;
    uint32 nResult;

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

        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nSlaveID);

        /* Write to CHIP address */
        nVal  = (uint8)pNode->oProperties.nBusI2CAddress;
        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nBusAddr,(uint8)REG_A2B0_CHIP ,nVal);

        /* Enable remote I2C write */
        nVal  =  (uint8)(pSrcNodePtr->nID & 0xFFu) | (uint8)BITM_A2B_NODEADR_PERI;
        nResult = (uint32)adi_a2b_TwiWrite8(A2B_TWI_NO, nMasterAddr,(uint8)REG_A2B0_NODEADR ,nVal);

        /* Return I2C address */
        nI2CAddr = nBusAddr;

    }
#endif
    return(nI2CAddr);

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
   uint8 nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
   uint8 nSlaveID = (uint8)((nNodeID & 0xFFu));
   uint8 nSlaveIndex  =  oFramework.aSlaveNodeReferenceTable[nBranchID][nSlaveID];
   return(nSlaveIndex);
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
   uint8 nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
   uint8 nMasterIndex  =  oFramework.aMasterNodeReferenceTable[nBranchID];
   return(nMasterIndex);
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
   uint8 nBranchID,nSlaveID;
   uint16 nNodeID;
   ADI_A2B_GRAPH *pgraph = pFrameWorkHandle->pgraph;

   for (i = 0u; i < pgraph->nNodeCount; i++)
   {
       nNodeID = pgraph->oNode[i].nID;

       if(pgraph->oNode[i].eType == (uint16)ADI_A2B_MASTER)
       {
          nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
          pFrameWorkHandle->aMasterNodeReferenceTable[nBranchID] = i;
       }
       else if (pgraph->oNode[i].eType == (uint16)ADI_A2B_SLAVE)
       {
          nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
          nSlaveID = (uint8)((nNodeID & 0xFFu));
          pFrameWorkHandle->aSlaveNodeReferenceTable[nBranchID][nSlaveID] = i;
       }
       else
       {
           nBranchID = (uint8)((nNodeID & 0xFF00u) >> 8u);
       }
   }
}

/** 
 @}
*/

/** 
 @}
*/


/*
**
** EOF: $URL$
**
*/

