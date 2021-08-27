/**
 * @file      tym_qp_lib.c
 * @brief     Tymphany QP debug library
 * @author    Gavin Lee
 * @date      19-June-2014
 * @copyright Tymphany Ltd.
 */



/*****************************************************************************
 * Include                                                                   *
 *****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h> //va_list
#include <assert.h>
#include "tym_qp_lib.h"
#include "qf_port.h"
#include "persistantObj.h"
#include "product.config"
#include "trace.h"


/*****************************************************************************
 * Global Variable                                                           *
 *****************************************************************************/



/*****************************************************************************
 * Function Implemenataion                                                   *
 *****************************************************************************/

/* Print message and queue context
 * Return Queue count
 * Reference: QActive_get_() in qa_get.c
 */




void TymQP_DumpQueue_WithLog(QActive *me, const char *fun_name, const char *fmt, ...)
{
    assert(fun_name);
    assert(fmt);

    char buf[256];
    va_list argus;
    va_start( argus, fmt );
    vsnprintf (buf, sizeof(buf), fmt, argus );
    va_end( argus );

#ifdef PRINT_LOG
    printf("[%s]%s", fun_name, buf);
    if(me)
        TymQP_DumpQueue(me);
    printf("\r\n");
#endif
}



/* Print message and queue context
 * Return Queue count
 * Reference: QActive_get_() in qa_get.c
 */
void TymQP_DumpQueue(QActive *me)
{
    assert(me);

    /* [Queue structure]
     *   pQueue->end:  queue size (ex. 5 for PowerSrvQueueSto[5])
     *   pQueue->head: next insert (init value: 0)
     *   pQueue->tail: next pop (init value: 0)
     *   Empty condition: pQueue->nFree==pQueue->end
     */
    QEQueue *pQueue= &(me->eQueue);
    int count= ( pQueue->end - pQueue->nFree );
    assert(count>=0);

    if ( count>0 )  /* any events in the buffer? */
    {
        //print header
        TP_PRINTF(" q(%d):", count);

        //print queue context
        int i= pQueue->tail;
        assert(i>=0 && i<pQueue->end);
        while(i != pQueue->head)
        {
            TP_PRINTF("%d,", pQueue->ring[i]->sig);
            if (0 == i)
                i= pQueue->end-1;    /* wrap around */
            else
                i--;
        }
    }
}

//Recall all deferred queue, return recall numbers
uint32 TymQP_QActive_RecallAll(QActive * const me, QEQueue * const eq)
{
    uint32 num_recall= 0;
    while( QActive_recall(me, eq) )
    {
        num_recall++;
    }
    return num_recall;
}

//Recall all deferred queue, return recall numbers
uint32 TymQP_QActive_RecallAllFIIO(QActive * const me, QEQueue * const eq)
{
    uint32 num_recall= 0;
    while( QActive_recallFIFO(me, eq) )
    {
        num_recall++;
    }
    return num_recall;
}
