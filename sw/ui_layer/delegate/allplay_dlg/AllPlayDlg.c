/**
*  @file      allplay_dlg.c
*  @brief     Source file for Allplay Delegate class
*  @author    Christopher Alexander
*  @date      22-Dec-2013
*  @copyright Tymphany Ltd.
*/


#include "./AllPlayDlg_priv.h"

DEF_DLG(me,cAllPlayDlg);

cAllPlayDlg * AllPlayDlgCtor(cAllPlayDlg * me)
{
    CREATE_DLG(me, cAllPlayDlg, 0);
    /* subscribe & intiate*/
    return me;
}

void AllPlayDlgXtor(cAllPlayDlg * me)
{
    /* free / zero any memory */
    DESTROY_DLG(me);
}

