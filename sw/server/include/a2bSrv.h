
/**
 * @file        A2bSrv.h
 * @brief       A2B control Server
 * @author      Edmond Sung
 * @date        2016-12-23
 * @copyright   Tymphany Ltd.
 */


#ifndef A2BSRV_V2_H
#define A2BSRV_V2_H

#ifdef __cplusplus
extern "C" {
#endif

#include "product.config"
#include "attachedDevices.h"
  


/********************************New Events******************************/

/**********************************************************************/


SUBCLASS(cA2bSrv, cServer)
    /* private data */
//    QTimeEvt TimeoutEvt;
METHODS
/* public functions */
/* Implement these so the controller can launch the server */
void A2bSrv_StartUp(cPersistantObj *me);
void A2bSrv_ShutDown(cPersistantObj *me);
void A2bSrv_I2cWrite(uint32_t node, uint8_t reg_addr, uint32_t len, uint8_t *p_buf);
void A2bSrv_I2cRead(uint32_t node, uint8_t reg_addr, uint32_t len, uint8_t *p_buf);

END_CLASS
#ifdef __cplusplus
}
#endif

#endif  /* A2BSRV_V2_H */


