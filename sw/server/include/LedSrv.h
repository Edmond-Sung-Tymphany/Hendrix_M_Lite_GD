/**
 *  @file      LedSrv.h
 *  @brief     This file defines the interface of Led server
 *              User can use public functions to set Led event and control Led
 *  @Functions: 1. turn on/off LED
 *               2. Shonw pattern defined in All_play project
 *               3. Fade on/off LED
 *  @author    Johnny Fan
 *  @date      20-Feb-2014
 *  @copyright Tymphany Ltd.
 */

#ifndef LEDSRV_H
#define	LEDSRV_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "server.h"
#include "deviceTypes.h"
#include "LedDrv.h"

#ifdef LED_SRV_SUPPORT_64_LEDS
#include "bit_shift_64.h"
#define GET_LED_MASK(ledId) shift_left64(1ULL, ledId)
#else
#define GET_LED_MASK(ledId) (1<<ledId)
#endif

#define ALL_LED 0xFF

typedef enum
{
    LED_OFF_CMD = 0,
    LED_PURE_ON_CMD,
    LED_PAT_ON_CMD,
    LED_PAT_STRIP_CMD,
}eLedCmd;

#ifdef LEDSRV_GROUP_ENABLE
typedef enum {
    GPC_Init,   /* place holder */
    GPC_KnightRider,
    GPC_OnByVolume,
    GPC_OffByVolume,
    GPC_FadeInByVolume,
    GPC_FadeOutByVolume,
    GPC_Delay,
} eGPC;
#define GPC_NoNext 0xff

typedef struct tGroupPatternData
{
    eGPC        cmd;
    uint32      periodTime;
    uint8       next_pattern;   /* start from 1 */
}tGroupPatternData;
#endif

/*  can use the public function below to set these event autumatically*/
REQ_EVT(LedReqEvt)
    eLedCmd     ledCommand;
    ledMask     leds;
    ePattern    patternId;
    Color       color;
#ifdef HAS_LED_STRIP
    eStripPattern stripPatternId;
#endif
END_REQ_EVT(LedReqEvt)

#ifdef LEDSRV_GROUP_ENABLE
REQ_EVT(LedReqGroupEvt)
    /* derived from LedReqEvt */
    eLedCmd             ledCommand; /* LED_PAT_ON_CMD */
    ledMask             leds;       /* 0x7f8, bit3~bit10 */
    ePattern            patternId;  /* PATTERN_GROUP */
    Color               color;      /* not used */

    /* extra parameter */
    tGroupPatternData   *pattern_tbl;
    uint8               volume;     /* 30 */
    uint8               *volToOneLed_tbl; /* [60, 120, 180, 255] */
END_REQ_EVT(LedReqGroupEvt)
#endif

IND_EVT(LedRespEvt)
    eEvtReturn result;
END_IND_EVT(LedRespEvt)

IND_EVT(LedStateEvt)
    ledMask   mask;
    ePattern    patternId:8;
END_IND_EVT(LedStateEvt)

#ifdef LEDSRV_GROUP_ENABLE
typedef struct {
    /* user parameter */
    LedReqGroupEvt    userReq;
    
    /* private */
    uint8       ledIdtbl[LED_MAX];
    uint8       total_leds;
    uint8       volToOneLed_levels; /* based on volToOneLed_tbl[] */
    uint8       volLedValue[LED_MAX];
    uint8       group_index;

    /* init for second run */
    uint8       pattern_first:1;    /* this is the frist pattern */
    uint8       pattern_init:1;     /* pattern for init */
    uint8       pattern_loop:1;     /* pattern is loop */
    uint8       nr_upward:1;        /* knight rider mode */
    int8        nr_pinIndex;        /* knight rider mode:1~max */
    uint8       fade_level;
    int8        cur_volLevel;       /* from 0, -1 is off */
    uint32      pattStart;          /**< pattern start time in tick */
} LEDGROUP_s;
#endif
SUBCLASS(cLedSrv, cServer)
    /* private: */
    QActive     *pRequestor;
    QTimeEvt    timeEvt;
    cLedDrv     *ledDrvList[LED_MAX];

#ifdef LEDSRV_GROUP_ENABLE
    /* group mode */
    uint8           grp_enabled:1;
    LEDGROUP_s      group_s;
#endif
METHODS
    /* public functions */
END_CLASS

void LedSrv_StartUp(cPersistantObj *me);
void LedSrv_ShutDown(cPersistantObj *me);

/* public functions to fill in the LedReq event */

void LedSrv_SetEvtOn        (QActive* sender, ledMask leds, Color c);
void LedSrv_SetEvtOff       (QActive* sender, ledMask leds);
void LedSrv_SetPatt (QActive* sender, ledMask leds, ePattern patternId);
#ifdef HAS_LED_STRIP
void LedSrv_SetStripPatt(QActive* sender, eStripPattern patternId);
#endif
#ifdef LEDSRV_GROUP_ENABLE
void    LedSrvGroup_SetPatt(QActive* sender, ledMask leds, ePattern patternId, tGroupPatternData *pattern_tbl, uint8 volume, uint8 *volToOneLed_tbl);
uint8   LedSrvGroup_Show(cLedSrv *me);
void    LedSrvGroup_Disable(cLedSrv *me);
void    LedSrvGroup_Enable(cLedSrv *me, LedReqEvt* req);
#endif
#ifdef	__cplusplus
}
#endif

#endif	/* LEDSRV_H */

