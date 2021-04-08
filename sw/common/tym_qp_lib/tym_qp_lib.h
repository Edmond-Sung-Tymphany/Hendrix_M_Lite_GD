/**
 * @file      tym_qp_lib.h
 * @brief     Tymphany QP debug library
 * @author    Gavin Lee
 * @date      19-June-2014
 * @copyright Tymphany Ltd.
 */

#ifndef TYMQPLIB_H
#define	TYMQPLIB_H
/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include "qf_port.h"
#include "persistantObj.h"
#include "product.config"


/*****************************************************************************
 * Function Prototype                                                        *
 *****************************************************************************/
void TymQP_DumpQueue_WithLog(QActive *me, const char *fun_name, const char *fmt, ...);
void TymQP_DumpQueue(QActive *me);
uint32 TymQP_QActive_RecallAll(QActive * const me, QEQueue * const eq);
uint32 TymQP_QActive_RecallAllFIIO(QActive * const me, QEQueue * const eq);


#endif /* #ifndef TYMQPLIB_H */
