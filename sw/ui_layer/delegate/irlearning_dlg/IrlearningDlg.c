/**
*  @file      irlearning_dlg.c
*  @brief     Source file for IR Learning Delegate class
*  @author    Edmond Sung
*  @date      23-Jan-2015
*  @copyright Tymphany Ltd.
*/

#include "IrLearningDlg_priv.h"
#include "IrLearningDlg.config"
#include "settingSrv_light.h"

#ifdef Q_SPY
#define CAST_ME cIrLearningDlg * IrLearningDlg = (cIrLearningDlg *) me;
#else
#define CAST_ME
#endif

#ifndef IR_LEARNING_DEBUG_ENABLE
#undef TP_PRINTF
#define TP_PRINTF(...)
#endif

/* Internal evt queue */
static QEvt const *IrLearningDlgEventQue[3];
static cIrLearningDrv irLearningDrvObj;

enum IrLearningDlgPriSignals /* BT delegate private signals */
{
    IrLearningDlg_TIMEOUT_SIG = MAX_SIG,
};

/* the time (ms) per timeout signal */
#define IR_LEARNING_DLG_TIMEOUT_IN_MS  10
/* the number of ticks for QP to trigger timer out signal*/
static const uint16 IR_LEARNING_DLG_TICK_TIME = GET_TICKS_IN_MS(IR_LEARNING_DLG_TIMEOUT_IN_MS);

/*****************************************************************************************************************
 *
 * Startup/Shutdown functions
 *
 *****************************************************************************************************************/

#ifdef HAS_IR_LEARNING_DELEGATE
/* Start function*/
cIrLearningDlg * IrLearningDlg_Ctor(cIrLearningDlg * me, QActive *ownerObj) /*This Ctor could take parameters*/
{
    me = (cIrLearningDlg *)CREATE_DLG(me, cIrLearningDlg, ownerObj, &IrLearningDlg_Initial);

    QTimeEvt_ctorX(&me->timeEvt, (QActive*)me, IrLearningDlg_TIMEOUT_SIG, 0);
    Delegate_Start((cDelegate*)me, IrLearningDlgEventQue, Q_DIM(IrLearningDlgEventQue));
    /* subscribe & initiate*/
#ifdef HAS_KEYS
    QActive_subscribe((QActive*) me, KEY_STATE_SIG);
#endif
    /* Create the IR learning Driver Object*/
    IrLearningDrv_Ctor(&irLearningDrvObj);
    return me;
}
/* Shut down function*/
void IrLearningDlg_Xtor(cIrLearningDlg * me)
{
    /* Destroy the IR learning Driver Object*/
    IrLearningDrv_Xtor(&irLearningDrvObj);
    QTimeEvt_disarm(&me->timeEvt);
    QActive_unsubscribeAll((QActive *)me);  /* unsubscribe from all signals */
    CLR_DLG_OWNER(me);
    /* free / zero any memory */
    DESTROY_DLG(me);
}
#endif /* HAS_IR_LEARNING_DELEGATE */



/*****************************************************************************************************************
 *
 * State functions
 *
 *****************************************************************************************************************/
/*Intial state*/
static QState IrLearningDlg_Initial(cIrLearningDlg * const me)
{
    CAST_ME;
    QS_OBJ_DICTIONARY(IrLearningDlg);
    QS_OBJ_DICTIONARY(IrLearningDlg_PreActive);
    QS_OBJ_DICTIONARY(IrLearningDlg_Active);
    
    return Q_TRAN(&IrLearningDlg_PreActive);
}

static void IrLearningDlg_RefleshTick(cIrLearningDlg * const me, const uint16 tickTime)
{
    QTimeEvt_disarm(&me->timeEvt);
    QTimeEvt_armX(&me->timeEvt, tickTime, 0);
}


static QState IrLearningDlg_PreActive(cIrLearningDlg * const me, QEvt const * const e)
{
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            return Q_HANDLED();
        }
        case MAINAPP_START_DLG_SIG:
        {
            return Q_TRAN(IrLearningDlg_Active);
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}

static IR_TIME_MAP	temp_keytime[2];
static QState IrLearningDlg_Active(cIrLearningDlg * const me, QEvt const * const e)
{
    cIrLearningDrv* pIrLearningDrv= &irLearningDrvObj;
    static bool bLearnFail=0;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LedSrv_SetPatt((QActive*)me, RGB_LED, SLOW_BLINKING_PATT_AMBER);
            IrLearningDlg_Reset(me);
            QTimeEvt_armX(&me->timeEvt, TIME_1s, 0);
            bLearnFail = 0;
            return Q_HANDLED();
        }
        case IrLearningDlg_TIMEOUT_SIG:         // every 10ms
        {
            IrLearningDlg_RefleshTick(me, IR_LEARNING_DLG_TICK_TIME);
            /* Below declare the main IR learning loop state */
            //if (me->bLearnTimeOut%1000==0)
            //{
                //TP_PRINTF("me->bLearnTimeOut=%d\r\n",me->bLearnTimeOut);
            //}
            if (me->bLearnTimeOut)
            {
                me->bLearnTimeOut-=IR_LEARNING_DLG_TICK_TIME;
                if (me->bLearnTimeOut==4*TIME_1s)
                {
                    LedSrv_SetPatt((QActive*)me, RGB_LED, EXIT_IR_LEARNING_MODE_PATT);
                }
                
                if (me->bLearnTimeOut<=0)
                {
                    IrLearningDlg_Reset(me);
                    return Q_TRAN(IrLearningDlg_PreActive);
                }

            }

            if (me->doublePressTimeout)
            {
                me->doublePressTimeout-=IR_LEARNING_DLG_TICK_TIME;
                if (me->doublePressTimeout<=0)
                {
                    LedSrv_SetPatt((QActive*)me, RGB_LED, EXIT_IR_LEARNING_MODE_PATT);
                    me->bLearnTimeOut = (4*TIME_1s);	// return to normal mode after 5 sec.
               }
            }

            //if (bLearnKeyIndex != LEARN_NOTHING)
            if (me->bLearnKeyIndex < LEARN_FINISH)
            {
                if ((pIrLearningDrv->IR_can_decode==1)&&(pIrLearningDrv->IRDecodeState==BUTTON_PRESS))
                {
                    pIrLearningDrv->IR_can_decode = 0;
                    // get the IR format and code base on the time record
                    IrLearningDrv_getFormatCode(pIrLearningDrv);    
                    //TP_PRINTF("learn code=%d-0x%x repHeader=%d\r\n",pIrLearningDrv->ir_keytime.format, pIrLearningDrv->ir_keytime.code,pIrLearningDrv->repHeader);
                    me->bLearnTimeOut = IR_LEARNING_TIMEOUT;	// return to normal mode after 5min.
                    
                    if (me->bLearnTimes<2)
                    {
                        // blink GREEN LED for Success
                        LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
                        CopyIRTimeMap(&temp_keytime[me->bLearnTimes], &(pIrLearningDrv->ir_keytime));
                        me->bLearnTimes++;

                    }
                    else   // when (me->bLearnTimes == 2 || me->bLearnTimes == 3)
                    {
                        {
                            if (!CompareIRTimeMap(&temp_keytime[(me->bLearnTimes-1)/2], &(pIrLearningDrv->ir_keytime))
                                /*||!CompareIRTimeMap(&temp_keytime[0], &temp_keytime[1])*/)
                            {
                                // Error The first and third key not match or second and forth key not match
                                bLearnFail =1;
                                LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
                                if (me->bLearnTimes == 2)   // if it is the third press
                                {
                                    me->bLearnTimes++;
                                    return Q_HANDLED();
                                }
                           }
                             
                            // blink GREEN LED for Success
                            else if (me->bLearnTimes < 3)
                            {
                                LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
                            }

                            me->bLearnTimes++;
                            if (me->bLearnTimes == 4)
                            {
                                if (bLearnFail)
                                {
                                    bLearnFail = 0;
                                    LedSrv_SetPatt((QActive*)me, RGB_LED, ON_PERIOD_PATT_RED);
                                    me->bLearnTimes=0;
                                    me->bJustFinishIRLearn = 1;
                                }
                                else
                                {
                                    // Success and store to "keymap", exist to previous mode.
                                    CopyIRMap(&(pIrLearningDrv->keymap_ram[me->bLearnKeyIndex][0]), &temp_keytime[0]);
                                    CopyIRMap(&(pIrLearningDrv->keymap_ram[me->bLearnKeyIndex][1]), &temp_keytime[1]);
                                    me->bIsFinish = TRUE;
                                    me->bLearnKeyIndex++;
                                    if (me->bLearnKeyIndex<LEARN_FINISH)
                                    {
                                        me->bLearnTimes = 0;
                                    }
                                    // Save the FULL IR data to flash evertime we have a succesful IR learn
                                    LedSrv_SetPatt((QActive*)me, RGB_LED, ON_PERIOD_PATT_GREEN);
                                }
                            }
                        }
                    }
                }
            }

            if (me->bLearnKeyIndex == LEARN_FINISH)
            {
                if (me->bIsFinish==TRUE)
                {
                    LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_WHITE);
                    Setting_Set(SETID_IR_LEARN_CODE, &(pIrLearningDrv->keymap_ram));
                    Setting_Bookkeeping();
                    me->bLearnTimeOut = TIME_3s;	// return to normal mode after 3 sec.
                    me->bIsFinish = FALSE;
                }
            }

            return Q_HANDLED();
        }
        case MAINAPP_STOP_DLG_SIG:
        {
            return Q_TRAN(IrLearningDlg_PreActive);
        }

        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((evt->keyEvent == KEY_EVT_SHORT_PRESS)&&(evt->keyId == POWER_KEY) )
            {
                if (me->doublePressTimeout ==0)
                {
                    me->doublePressTimeout=500;
                }
                else
                {
                    me->doublePressTimeout = 0;
                }
                    
            }
            return Q_HANDLED();
        }


        case Q_EXIT_SIG:
        {
            IrLearningDlg_StopEvtResp(me);
            QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}
/*
static QState IrLearningDlg_Active(cIrLearningDlg * const me, QEvt const * const e)
{
    static IR_TIME_MAP	temp_keytime[2];
    cIrLearningDrv* pIrLearningDrv= &irLearningDrvObj;
    switch (e->sig)
    {
        case Q_ENTRY_SIG:
        {
            LedSrv_SetPatt((QActive*)me, RGB_LED, SLOW_BLINKING_PATT_AMBER);
            IrLearningDlg_Reset(me);
            QTimeEvt_armX(&me->timeEvt, TIME_1s, 0);
            return Q_HANDLED();
        }
        case IrLearningDlg_TIMEOUT_SIG:         // every 10ms
        {
            IrLearningDlg_RefleshTick(me, IR_LEARNING_DLG_TICK_TIME);
            // Below declare the main IR learning loop state 
            if (me->bLearnTimeOut)
            {
                me->bLearnTimeOut-=IR_LEARNING_DLG_TICK_TIME;
                if (me->bLearnTimeOut<=0)
                {
                    IrLearningDlg_Reset(me);
                    return Q_TRAN(IrLearningDlg_PreActive);
                }

            }

            //if (bLearnKeyIndex != LEARN_NOTHING)
            if (me->bLearnKeyIndex < LEARN_FINISH)
            {
                if ((pIrLearningDrv->IR_can_decode==1)&&(pIrLearningDrv->IRDecodeState==BUTTON_PRESS))
                {
                    pIrLearningDrv->IR_can_decode = 0;
                    // get the IR format and code base on the time record
                    IrLearningDrv_getFormatCode(pIrLearningDrv);    
                    //TP_PRINTF("learn code=%d-0x%x repHeader=%d\r\n",pIrLearningDrv->ir_keytime.format, pIrLearningDrv->ir_keytime.code,pIrLearningDrv->repHeader);
                    me->bLearnTimeOut = TIME_5min;	// return to normal mode after 5min.

                    if ((me->bLearnTimes==0)||(me->bLearnTimes==1))
                    {
                        // blink GREEN LED for Success
                        LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
                        CopyIRTimeMap(&temp_keytime[me->bLearnTimes], &(pIrLearningDrv->ir_keytime));
                        me->bLearnTimes++;

                    }
                    else
                    {
                        if (!CompareIRTimeMap(&temp_keytime[(me->bLearnTimes-1)/2], &(pIrLearningDrv->ir_keytime)))
                        {
                            //4 ÿError The first and third key not match or second and forth key not match
                            // Light RED LED and exist to previous mode.
                            //FlashingLEDStart(LEARNING_ERROR, TIME_500ms, TIME_100ms, TIME_100ms);
                            //TempLEDDisplayStart(SHOW_MODE, TIME_5s);

                            //FlashingLEDStart(LEARNING_ERROR, TIME_500ms, TIME_100ms, TIME_100ms);
                            //TempLEDDisplayStart(SHOW_MODE, TIME_5s);
                            //gLedState = LEARN_ERROR_LED;          // comment out, to fix IN3051
                            LedSrv_SetPatt((QActive*)me, RGB_LED, ON_PERIOD_PATT_RED);
                            //me->bIsFinish = TRUE;
                            //me->bLearnKeyIndex = LEARN_FINISH;
                            me->bLearnTimes=0;

                            //////////////////////////////////////
                            //Vic add, to prevent execute key command, fix IN3020
                            //uKeyPressed = NO_KEY;
                            //uPrevKeyPressed = NO_KEY;
                            //uIRToKeyPressed = NO_KEY;
                            me->bJustFinishIRLearn = 1;
                            /////////////////////////////////////
                        }
                        else
                        {
                            // blink GREEN LED for Success
                            if (me->bLearnTimes < 3)
                            {
                                LedSrv_SetPatt((QActive*)me, RGB_LED, VERY_FAST_BLINKING_PATT);
                                //TempLEDDisplay.fTotalTime = 0;
                                //FlashingLEDStart(LEARNING_SUCCESS, TIME_500ms, TIME_100ms, TIME_100ms);
                                //FlashingLEDStart(LEARNING_KEY_FLAG, TIME_500ms, TIME_500ms, 0);
                                //TempLEDDisplayStart(SHOW_MODE, TIME_5s);
                            }

                            me->bLearnTimes++;
                            if (me->bLearnTimes == 4)
                            {
                                // Success and store to "keymap", exist to previous mode.
                                CopyIRMap(&(pIrLearningDrv->keymap_ram[me->bLearnKeyIndex][0]), &temp_keytime[0]);
                                CopyIRMap(&(pIrLearningDrv->keymap_ram[me->bLearnKeyIndex][1]), &temp_keytime[1]);
                                me->bIsFinish = TRUE;
                                me->bLearnKeyIndex++;
                                if (me->bLearnKeyIndex<LEARN_FINISH)
                                {
                                    me->bLearnTimes = 0;
                                }
                                // Save the FULL IR data to flash evertime we have a succesful IR learn
                                LedSrv_SetPatt((QActive*)me, RGB_LED, ON_PERIOD_PATT_GREEN);
                            }
                        }
                    }
                }
            }

            if (me->bLearnKeyIndex == LEARN_FINISH)
            {
                if (me->bIsFinish==TRUE)
                {
                    LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_WHITE);
                    Setting_Set(SETID_IR_LEARN_CODE, &(pIrLearningDrv->keymap_ram));
                    Setting_Bookkeeping();
                    me->bLearnTimeOut = TIME_3s;	// return to normal mode after 3 sec.
                    me->bIsFinish = FALSE;
                }
            }

            return Q_HANDLED();
        }
        case MAINAPP_STOP_DLG_SIG:
        {
            return Q_TRAN(IrLearningDlg_PreActive);
        }

        case KEY_STATE_SIG:
        {
            KeyStateEvt *evt = (KeyStateEvt*)e;
            if((evt->keyEvent == KEY_EVT_SHORT_PRESS)&&(evt->keyId == POWER_KEY))
            {
                LedSrv_SetPatt((QActive*)me, RGB_LED, SOLID_ON_PATT_WHITE);
                me->bLearnTimeOut = TIME_1s;	// return to normal mode after 3 sec.
            }
            return Q_HANDLED();
        }


        case Q_EXIT_SIG:
        {
            IrLearningDlg_StopEvtResp(me);
            QTimeEvt_disarm(&me->timeEvt);
            return Q_HANDLED();
        }
        default:
            break;
    }
    return Q_SUPER(&QHsm_top);
}
*/

static void IrLearningDlg_StopEvtResp(cIrLearningDlg * me)
{
    //IrLearnStopEvt* StopEvtRespEvt = Q_NEW(IrLearnStopEvt, MAINAPP_STOP_DLG_SIG);
    
    CommonEvtResp((QActive*)me, GET_DLG_OWNER(me) ,RET_SUCCESS, MAINAPP_STOP_DLG_SIG);
}

static void IrLearningDlg_Reset(cIrLearningDlg * me)
{
    cIrLearningDrv* pIrLearningDrv= &irLearningDrvObj;
    me->bLearnKeyIndex = LEARN_VOLUME_UP_KEY_INDEX;         // reset to learn the first key
    me->bLearnTimes =0;
    me->bIsFinish = 0;
    me->bLearnTimeOut = IR_LEARNING_TIMEOUT;	// return to normal mode after 5min.
    pIrLearningDrv->IR_can_decode = 0;
    pIrLearningDrv->bValidIr = 0;
}

void IrLearningDlg_EraseLearntCode()
{
    cIrLearningDrv* pIrLearningDrv= &irLearningDrvObj;
    memset(&(pIrLearningDrv->keymap_ram), 0x00, sizeof(&(pIrLearningDrv->keymap_ram)));
    Setting_Set(SETID_IR_LEARN_CODE, &(pIrLearningDrv->keymap_ram));
    //Setting_Bookkeeping();
}
