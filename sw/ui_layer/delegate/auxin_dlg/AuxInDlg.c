/**
*  @file      auxin_dlg.c
*  @brief     Source file for Allplay Delegate class
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*/

/* THIS DELEAGATE IS JUST FOR TEST PURPOSES
 * IN ITS CURRENT INTERATION IT IS MERELY FOR TESTING */

#include "./AuxInDlg_priv.h"


/* State function defintions */
static QState AuxInDlg_Idle(cAuxInDlg * const me, QEvt const * const e);
static QState AuxInDlg_Initial(cAuxInDlg * const me);

/*  */
static QEvt const *eventQue[2];
/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 * In this example the deleagte has no config data but because deleagtes represent specifc and sometimes complex
 * logic then we can add config data for each delegate.
 *
 *****************************************************************************************************************/
/* Start function*/
cAuxInDlg * AuxInDlg_Ctor(cAuxInDlg * me) /*This Ctor could take parameters*/
{
    me = (cAuxInDlg *)CREATE_DLG(me, cAuxInDlg, &AuxInDlg_Initial);
    /* subscribe & initiate*/
    //QTimeEvt_ctorX(&me->timeEvt, (QActive*)audioSrv, TIMEOUT_SIG, 0);

    /* start up the object and let it run. Called by the controller */
    /* active object start */
    Delegate_Start((cDelegate*)me, eventQue);
    //QActive_start((QActive*)me, prior, eventQue, Q_DIM(eventQue), (void *)0, 0U, (QEvt *)0);

    /* Subscribe */
    return me;
}
/* Shut down function*/
void AuxInDlg_Xtor(cAuxInDlg * me)
{
    
    /* free / zero any memory */
    DESTROY_DLG(me);
}
/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/* Initial state */
static QState AuxInDlg_Initial(cAuxInDlg * const me)
{
    QS_OBJ_DICTIONARY(me);

    QS_FUN_DICTIONARY(&AuxInDlg_Initial);
    QS_FUN_DICTIONARY(&AuxInDlg_Idle);

    /* Subsrcribe to all the SIGS */
    /* QActive_subscribe(me, POWER_OFF_SIG); */

    return Q_TRAN(&AuxInDlg_Idle);
}
/* Idle state */
static QState AuxInDlg_Idle(cAuxInDlg * const me, QEvt const * const e)
{
    (void)e; /* suppress the compiler warning about unused parameter */
    /* Based on the subscribed events and requests we can switch delegates at any time */
    return Q_SUPER(&QHsm_top);
}

