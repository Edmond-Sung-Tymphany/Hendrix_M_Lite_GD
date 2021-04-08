/**
 * @file        DmaDrv.h
 * @brief       This file declare the DMA interface and define the DMA interface implementation
 * @author      Bob.Xu 
 * @date        2014-04-01
 * @copyright   Tymphany Ltd.
 */

#ifndef DMADRV_H
#define DMADRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"

typedef enum{
    DMA_CHANNEL_0 = 0,
    DMA_CHANNEL_1,
    DMA_CHANNEL_2,
    DMA_CHANNEL_3,
    MAX_DMA_CHANNEL_ID
}eDmaChannel;

typedef void(*dmaIntHandlerCb)(eDmaChannel dmaChennel);

CLASS(cDmaDrv)
   eDmaChannel channel;
   bool    channelBusy;
   dmaIntHandlerCb pIntHandlerCb;
METHODS
/* public functions */

/**
* Dma Driver object constructor
* @return    me               the dma Driver object
*/
cDmaDrv *DmaDrv_Ctor(void);

/**
* Dma Driver object destructor
* @param[in]    me              the Dma Driver object
*/
void DmaDrv_Xtor(cDmaDrv* me);

/**
* Eable Dma object,Actually disable the channel
* @param[in]    me               the dma Driver object
* @param[in]    channel          DMA channel
*/
void DmaDrv_InitDmaObj(cDmaDrv *me, uint32 triggerIRQ);

/**
* this function set the data transfer details
* @param[in]    me               the dma Driver object
* @param[in]    pSourceAdd       data transfer source address
* @param[in]    pDestinationAdd  data transfer destination address
* @param[in]    srcSize          source data size
* @param[in]    dstSize          destination size
* @param[in]    cellSize         data transfer cell size
 */
void DmaDrv_SetTxBuffer(cDmaDrv *me,const void* pSourceAdd,void* pDestinationAdd, \
                        uint16 srcSize, uint16 dstSize, uint16 cellSize);
/**
* This function set the triger to start dma data transferring,
* This function can not ne used when the DMA channel is triggerred
* by hardware interrupt requests.
* @param[in]    me               the dma Driver object
* @param[in]    event            the event to triger the dma data transfer
*/
void DmaDrv_TriggerTransfer(cDmaDrv *me);

/**
* This function can not ne used when the DMA channel is triggerred
* by hardware interrupt requests.
* @param[in]    me               the dma Driver object
* @param[in]    pattern          the pattern to stop dma data transfer
*/
void DmaDrv_SetMatchPattern(cDmaDrv *me,int pattern);

/**
* Register the function for callback, the DMA interrupt may have different requirements
* upon to the senario, this function provide a flexible way to handle different requirements
* @param[in]    me              the DMA Driver object
* @param[in]    fCb             function pointer to the callback
*/
void DmaDrv_RegisterIntHandlerCb(cDmaDrv* me, dmaIntHandlerCb fCb);

/**
* Disable the interrupt of a specified dma channel
* @param[in]    me              the DMA Driver object
*/
void DmaDrv_DisableInt(cDmaDrv *me);

/**
* Enable the interrupt of a specified dma channel
* @param[in]    me              the DMA Driver object
*/
void DmaDrv_EnableInt(cDmaDrv *me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif /* DMADRV_H */