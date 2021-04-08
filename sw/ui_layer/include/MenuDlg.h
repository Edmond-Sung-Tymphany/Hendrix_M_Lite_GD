/**
 * @file        MenuDlg.h
 * @brief       This file declare and implement the menu dlg
 * @author      Bob.Xu
 * @date        2014-11-19
 * @copyright   Tymphany Ltd.
 */
 
#ifndef MENUDLG_H
#define MENUDLG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "delegate.h"
#include "deviceTypes.h"
typedef enum
{
    PAGE_SETTING_LP_FRE = 0,
    PAGE_SETTING_LP_SLO,
    PAGE_SETTING_HP_FRE,
    PAGE_SETTING_HP_SLO,
    PAGE_SETTING_POLARITY,
    PAGE_SETTING_PEQ1_FRE,
    PAGE_SETTING_PEQ1_BOOST,
    PAGE_SETTING_PEQ1_Q,
    PAGE_SETTING_PEQ2_FRE,
    PAGE_SETTING_PEQ2_BOOST,
    PAGE_SETTING_PEQ2_Q,
    PAGE_SETTING_PEQ3_FRE,
    PAGE_SETTING_PEQ3_BOOST,
    PAGE_SETTING_PEQ3_Q,
    PAGE_SETTING_PRE1_LOAD,
    PAGE_SETTING_PRE2_LOAD,
    PAGE_SETTING_PRE3_LOAD,
    PAGE_SETTING_PRE4_LOAD,
    PAGE_SETTING_PRE1_SAVE,
    PAGE_SETTING_PRE2_SAVE,
    PAGE_SETTING_PRE3_SAVE,
    PAGE_SETTING_PRE4_SAVE,
    PAGE_SETTING_RGC,
    PAGE_SETTING_VOL,
    PAGE_SETTING_DELAY,
    PAGE_SETTING_DISPLAY,
    PAGE_SETTING_TIMEOUT,
    PAGE_SETTING_STANDBY,
    PAGE_SETTING_TUNNING,
    PAGE_SETTING_LP_ON,
    PAGE_SETTING_HP_ON,
    PAGE_SETTING_PEQ1_ON,
    PAGE_SETTING_PEQ2_ON,
    PAGE_SETTING_PEQ3_ON, 
    PAGE_SETTING_RGC_ON,
    PAGE_SETTING_LP_OFF,
    PAGE_SETTING_HP_OFF,
    PAGE_SETTING_PEQ1_OFF,
    PAGE_SETTING_PEQ2_OFF,
    PAGE_SETTING_PEQ3_OFF,
    PAGE_SETTING_RGC_OFF,
    PAGE_SETTING_LP_STATUS,
    PAGE_SETTING_HP_STATUS,
    PAGE_SETTING_PEQ1_STATUS,
    PAGE_SETTING_PEQ2_STATUS,
    PAGE_SETTING_PEQ3_STATUS,
    PAGE_SETTING_RGC_STATUS,
    PAGE_SETTING_BT_ON,
    PAGE_SETTING_BT_OFF,
    PAGE_SETTING_CLEAR_BT, 
    PAGE_SETTING_RESET_YES,
    PAGE_SETTING_RESET_NO,
    PAGE_SETTING_RGC_FREQ,
    PAGE_SETTING_RGC_SLOPE,
    PAGE_SETTING_PHASE,
    PAGE_SETTING_MUTE,
    PAGE_SETTING_BRIGHTNESS,
    PAGE_SETTING_ID_MAX,
}ePageSettingId;

typedef enum
{
    PAGE_NAVIGATION_TYPE,               //only navigation
    PAGE_ACTION_TYPE,                   //indicate controller to do something
    PAGE_NAVIGATION_ACTION_TYPE,        //navigation also tell controller to do something
    PAGE_NAVIG_ACTION_WITH_DELAY_TYPE,  //navigation also tell controller to do something,however, display next node with delay
    PAGE_EXIT_TYPE,                     //exit menu,
    PAGE_TYPE_MAX
}ePageType;

typedef enum
{
    PAGE_ACTIVE,        //The node to display when the menu enter to next level
    PAGE_DEACTIVE,      
    PAGE_STATUS_INVALID //Menu dlg will not search for the active page if it detect a page who has this status
}ePageStatus;

typedef struct tPageNode tPageNode;
typedef struct tKeyToPageNodeConf tKeyToPageNodeConf;

struct tPageNode
{
    uint8             numOfBrother;  /* How many nodes which share the same parent */
    uint8             *pPageText;    /* What should be displayed */
    ePageStatus       pageStatus;   /* The same menu level may have serveral nodes, this defines if this node is the one to display when the menu enter from top level to this level */
    tPageNode         **ppChildren;
    ePageSettingId    pageSettId;
    ePageType         pageType;      /* This type decides if the DLG should give feedback to controller or simply do navigation */
};

struct tKeyToPageNodeConf
{
    eKeyID                      keyId;
    eKeyEvent                   keyEvt;
    tPageNode                   *pPageNode;   /* The page need to be displayed*/
    uint16                      expireTime;   /* How long will the current page expire and back to the previous page in ms */
    BOOL                        backToPrePage;/* Define if it need to jump back to the previous page */
    const tKeyToPageNodeConf    *pNext;
};

typedef struct tMenu
{
    tPageNode                   **ppMenuEntrance;
    BOOL                        menuLoopBack;  /* Does it have loop back option when it reaches the end */
    tKeyToPageNodeConf          *pKeyToPageListHead;
    uint16                      navigActionwithDelayTime; //ms
}tMenu;

REQ_EVT(MenuDataUpdate)
    tPageNode *pPageNode;
    eKeyID         keyId;
    eKeyEvent      keyEvt;   
END_REQ_EVT(MenuDataUpdate)

SUBCLASS(cMenuDlg, cDelegate)
    /* private data */
    tPageNode **ppCurrNode; // current page node
    tMenu     *pMenu;
    QTimeEvt  timeEvt;
    BOOL      isCreated;
METHODS
    /* public functions */
END_CLASS

/* Basic ctor xtor functions. Must call super class Ctor / Xtor. \sa delegate.c */
cMenuDlg * MenuDlg_Ctor(cMenuDlg *me,  QActive *ownerObj, tMenu * pMenu);
void MenuDlg_Xtor(cMenuDlg *me);
void MenuDlg_Xtor(cMenuDlg *me);
void MenuDlg_Disable(cMenuDlg *me);
void MenuDlg_Enable(cMenuDlg *me);
void MenuDlg_Reset(cMenuDlg *me);

#ifdef __cplusplus
}
#endif

#endif /* MENUDLG_H */
