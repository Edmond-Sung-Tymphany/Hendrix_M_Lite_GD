/**
 *  @file      seq.h
 *  @brief     Implemented Sequence Event with time delay
 *  @author    Wesley Lee
 *  @date      11-Dec-2015
 *  @copyright Tymphany Ltd.
 */

#ifndef _SEQ_H_
#define _SEQ_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "commonTypes.h"

typedef void (*seqSectionFunc)(void* me);

typedef struct tSeqSection_
{
    seqSectionFunc  pfSect;     // function point to a specific sequence section
    uint32          seqDelay;   // time duration to next section
}tSeqSection;

CLASS(cSeq)
    void    *owner;
    tSeqSection* seqSect;
    uint32  length;
    uint32  stage;       // current stage
    uint32  delay;       // current count down timer
METHODS

/**
* Sequence Event object constructor
* @param[in]    me      Sequence Event object
* @param[in]    owner   Object who own this sequence Event object
* @param[in]    seqSect Array of Sequence Event Section
* @param[in]    len     the length of the array
*/
void Seq_Ctor(cSeq *me, void* owner, tSeqSection* seqSect, uint32 len);

/**
* Sequence Event object destructor
* @param[in]    me              Sequence Event object
*/
void Seq_Xtor(cSeq *me);

/**
* Sequence Event refresh, advance to next stage if time reaches
* @param[in]    me      Sequence Event object
* @param[in]    time    Time passed since last called
* @return       0: seq finished; delay time to next stage otherwise
*/
uint32 Seq_Refresh(cSeq *me, uint32 time);

/**
* Check if the sequence is finished
* @param[in]    me      Sequence Event object
* @return       false: seq finished; true: otherwise
*/
bool Seq_isSeqFinished(cSeq *me);

END_CLASS

#ifdef __cplusplus
}
#endif

#endif

