/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Seq - common handling on Time Sequence Event
                  -------------------------

                  SW Module Document




@file        seq.c
@brief       Implemented Sequence Event with time delay
@author      Wesley Lee
@date        11-Dec-2015
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2015-12     Wesley Lee
DESCRIPTION: First Draft.
SCO/ERROR  :
-------------------------------------------------------------------------------
*/

#include "seq.h"
#include "trace.h"

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/
static void Seq_iterate(cSeq *me)
{
    ASSERT(me);
    uint32 stage = me->stage;
    
    if (me->seqSect[stage].pfSect)
    {
        me->seqSect[stage].pfSect(me->owner);
    }
    me->delay   = me->seqSect[stage].seqDelay;
    ++me->stage;
}

/*****************************************************************************************************************
 *
 * public functions
 *
 *****************************************************************************************************************/
void Seq_Ctor(cSeq *me, void* owner, tSeqSection* seqSect, uint32 len)
{
    ASSERT(me && seqSect && len);

    me->owner   = owner;
    me->seqSect = seqSect;
    me->length  = len;
    me->stage   = 0;

    Seq_iterate(me);
}

void Seq_Xtor(cSeq *me)
{
    ASSERT(me);
    me->owner   = NULL;
    me->seqSect = NULL;
    me->stage   = 0;
    me->length  = 0;
    me->delay   = 0;
}

uint32 Seq_Refresh(cSeq *me, uint32 time)
{
    ASSERT(me);

    if (me->delay > time)
    {
        me->delay -= time;
    }
    else
    {
        if (me->stage >= me->length)
        {
            Seq_Xtor(me);
        }
        else
        {
            Seq_iterate(me);
        }
    }

    return me->delay;
}

bool Seq_isSeqFinished(cSeq *me)
{
    ASSERT(me);
    return (me->seqSect == NULL);
}

