/*
-------------------------------------------------------------------------------
TYMPHANY LTD





                  Menu Driver
                  -------------------------

                  SW Module Document




@file        MenuDrv.c
@brief       This file defines the interface and structures of genaric menus
@author      Bob.Xu 
@date        2014-08-11
@copyright (c) Tymphany Ltd. All rights reserved.

Change History:
VERSION    : 1    DRAFT      2014-08-11     Bob.Xu
DESCRIPTION: First Draft. Generated by newclass.py
SCO/ERROR  : 
-------------------------------------------------------------------------------
*/

#ifndef MENU_DRV_H
#define MENU_DRV_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cplus.h"
#include "deviceTypes.h"
#include "attachedDevices.h"


 /* structure declaration */
typedef struct tMenuSettingCont tMenuSettingCont;
typedef struct tMenuPage tMenuPage;

typedef enum
{
    MENU_LEVEL_1 = 1,
    MENU_LEVEL_2,
    MENU_LEVEL_3,
    MENU_LEVEL_4,
    MENU_LEVEL_MAX,
}eMenuLevel;

typedef enum
{
    MENU_DATA_BOOL_TYPE,
    MENU_DATA_INT_TYPE,
    MENU_DATA_FLOAT_TYPE
}eMenuDataType;


typedef enum
{
    INVALIDE_PAGE_SETTING_ID,
    PAGE_SETTING_ID_1,
    PAGE_SETTING_ID_2,
    PAGE_SETTING_ID_3,
    PAGE_SETTING_ID_4,
    PAGE_SETTING_ID_5,
    PAGE_SETTING_ID_6,
    PAGE_SETTING_ID_7,
    PAGE_SETTING_ID_8,
    PAGE_SETTING_ID_9,
    PAGE_SETTING_ID_10,
    PAGE_SETTING_ID_11,
    PAGE_SETTING_ID_12,
    PAGE_SETTING_ID_13,
    PAGE_SETTING_ID_14,
    PAGE_SETTING_ID_15,
    PAGE_SETTING_ID_16,
    PAGE_SETTING_ID_17,
    PAGE_SETTING_ID_18,
    PAGE_SETTING_ID_19,
    PAGE_SETTING_ID_20,
    PAGE_SETTING_ID_21,
    PAGE_SETTING_ID_22,
    PAGE_SETTING_ID_23,
    PAGE_SETTING_ID_24,
    PAGE_SETTING_ID_25,
    PAGE_SETTING_ID_26,
    PAGE_SETTING_ID_27,
    PAGE_SETTING_ID_28,
    PAGE_SETTING_ID_29,
    PAGE_SETTING_ID_30,
    PAGE_SETTING_ID_31,    
    PAGE_SETTING_ID_32,
    PAGE_SETTING_ID_33,
    PAGE_SETTING_ID_34,
    PAGE_SETTING_ID_35,
    PAGE_SETTING_ID_36,
    PAGE_SETTING_ID_37,
    PAGE_SETTING_ID_38,
    PAGE_SETTING_ID_39,
    PAGE_SETTING_ID_40,
    PAGE_SETTING_ID_41,
    PAGE_SETTING_ID_42,
    PAGE_SETTING_ID_43,
    PAGE_SETTING_ID_44,
    PAGE_SETTING_ID_45,
    PAGE_SETTING_ID_46,
    PAGE_SETTING_ID_47,
    PAGE_SETTING_ID_48,
    PAGE_SETTING_ID_49,
    PAGE_SETTING_ID_50,
    PAGE_SETTING_ID_MAX,
}ePageSettingId;

typedef struct tMenu
{
    uint8                   numOfPages;
    uint8                   numOfCont;  /* Num of setting content */
    const tMenuPage         *pPage;     /* point to the pages */
    const tMenuSettingCont  *pSettingCont; /* point to the setting content */
}tMenu;

typedef struct tMenuInitSection
{
    ePageSettingId  pageSettingId;  /* pointer point to init sections */
    uint16          delaytime;      /* time duration to next intialization section in ms */
}tMenuInitSection;

CLASS(cMenuDrv)
    /* private data */
    uint8                     initSectionSize;
    uint8                     initPhase;
    BOOL                      isSettingPage;
    BOOL                      preSettingStatus;
    int16                     currentVal;
    int16                     preVal;
    tMenu                     *pMenu;
    const tMenuPage           *pCurrPage;      /* Current page */
    const tMenuPage           *pPrePage;       /* Previous page */
    const tMenuSettingCont    *pCurrSetCont;   /* Current setting content */
    const tMenuSettingCont    *pPreSetCont;    /* Previous setting content */
    const tMenuInitSection    *pInitTable;
METHODS

void MenuDrv_Ctor(cMenuDrv *me);
void MenuDrv_Xtor(cMenuDrv *me);
uint16 MenuDrv_Init(cMenuDrv *me, bool reInit);
void MenuDrv_DisplayPage(cMenuDrv * const me, uint8 pageIndex);
uint8 MenuDrv_GetCurrPage(cMenuDrv *me);
void MenuDrv_BackToParent(cMenuDrv * const me);
void MenuDrv_ScroNext(cMenuDrv * const me);
void MenuDrv_ScroPre(cMenuDrv * const me);
void MenuDrv_BackToParent(cMenuDrv * const me);
void MenuDrv_Enter(cMenuDrv * const me);
void MenuDrv_VolButtonHandler(cMenuDrv * me, eKeyID keyId);

END_CLASS


struct tMenuSettingCont{
    ePageSettingId  settingId;
    eMenuDataType   dataType;
    int16           minVal;
    int16           maxVal;
    int16           defaultVal;
    int16           valPerStep;
    uint8           uintLength;
    uchar           unit[NUM_OF_SCREEN_DIGIT];
    void            (*MenuSettingCb)(cMenuDrv *me);
};

struct tMenuPage
{
    uint8           arryIndex;
    eMenuLevel      menuLevel;
    uchar           dispalyInfo[NUM_OF_SCREEN_DIGIT];
    ePageSettingId  settingID;
    void            (*MenuSettingCb)(cMenuDrv *me);
};
#ifdef __cplusplus
}
#endif

#endif /* MENU_DRV_H */

