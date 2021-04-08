/**
*  @file      LedSrv_group.c
*  @brief     add "group" pin behavior
*  @author    Tandy Liu
*  @date      20-June-2016
*  @copyright Tymphany Ltd.
*  @Note      State Machine:
*/

#include "product.config"
#include "trace.h"
#include "LedSrv.h"
#include "bsp.h"
#include <string.h>

/* internal function */
void    _LedSrvGroup_setLedOffByArray(cLedSrv *me, uint8 ledIdtbl[], uint8 total_leds);
void    _LedSrvGroup_setBrightByArray(cLedSrv *me, uint8 ledIdtbl[], uint8 led_light_array[], uint8 total_leds);
void    _LedSrvGroup_calcGrayValueByArray(const uint8 volLedValue[], uint8 total_leds, uint8 led_light_array[], uint8 fade_level, uint8 max_level);
void    _LedSrvGroup_voumeToLedArray(cLedSrv *me, uint8 volume, uint8 led_light_array[], uint8 total_leds);

/* @brief       disable the group function
 * @param[in]   me      instance of the driver
 */
void    LedSrvGroup_Disable(cLedSrv *me)
{
    me->grp_enabled = 0;
}

/* @brief       enable the group function
 * @param[in]   me      instance of the driver
 * @param[in]   req     parameter from user
 */
void    LedSrvGroup_Enable(cLedSrv *me, LedReqEvt* req)
{
    LEDGROUP_s          *grp_ptr = &me->group_s;
    LedReqGroupEvt      *userReq_ptr = &grp_ptr->userReq;
    uint8               i;
    eLed                led_id;

    memset(grp_ptr, 0, sizeof(*grp_ptr));

    /* keep user parameter */
    *userReq_ptr = *(LedReqGroupEvt*)req;
    ASSERT(userReq_ptr->pattern_tbl);

    me->grp_enabled = 1;

    /* count levels */
    for (i = 0; userReq_ptr->volToOneLed_tbl[i]; i++)
        grp_ptr->volToOneLed_levels ++;

    /* collect pin-to-drv info */
    for (led_id = LED_MIN; led_id < LED_MAX; led_id++)
    {
        ledMask m = GET_LED_MASK(led_id);

        if (!(userReq_ptr->leds&m))
            continue;

        grp_ptr->ledIdtbl[grp_ptr->total_leds] = led_id;
        grp_ptr->total_leds ++;

        LedDrv_PattStop(me->ledDrvList[led_id]);
        me->ledDrvList[led_id]->pLedFunc->LedOff(me->ledDrvList[led_id]);
    }

    if (grp_ptr->volToOneLed_levels)
        _LedSrvGroup_voumeToLedArray(me, userReq_ptr->volume, grp_ptr->volLedValue, grp_ptr->total_leds);

    grp_ptr->pattern_first = 1;
    grp_ptr->pattern_init = 1;
    LedSrvGroup_Show(me);
}

/* @brief       calculate the gray value based on the volume value
 * @param[in]   me              instance of the driver
 * @param[in]   volLedValue     original volume value
 * @param[in]   total_leds      total leds
 * @param[out]  led_light_array output
 * @param[in]   fade_level      fade level
 * @param[in]   max_level       max level
 */
void    _LedSrvGroup_calcGrayValueByArray(const uint8 volLedValue[], uint8 total_leds, uint8 led_light_array[], uint8 fade_level, uint8 max_level)
{
    uint8   i;

    for (i=0;i<total_leds;i++)
        led_light_array[i] = volLedValue[i]*fade_level/max_level;

}

/* @brief       convert the volume to led table
 * @param[in]   me                  instance of the driver
 * @param[in]   volume              volume level
 * @param[out]  led_light_array     output table
 * @param[in]   total_leds          used leds
 */
void    _LedSrvGroup_voumeToLedArray(cLedSrv *me, uint8 volume, uint8 led_light_array[], uint8 total_leds)
{
    LEDGROUP_s          *grp_ptr = &me->group_s;
    LedReqGroupEvt      *userReq_ptr = &grp_ptr->userReq;
    uint8               full_cnt = volume/grp_ptr->volToOneLed_levels;
    uint8               i;

    ASSERT(grp_ptr->volToOneLed_levels);
    //WARNING(total_leds <= total_leds);
    for (i = 0; i < full_cnt && i < total_leds; i++)
        led_light_array[i] = userReq_ptr->volToOneLed_tbl[grp_ptr->volToOneLed_levels-1];

    /* last one */
    ASSERT(i + 1 < total_leds);
    led_light_array[i] = userReq_ptr->volToOneLed_tbl[volume% grp_ptr->volToOneLed_levels];
}

/* @brief       turn off the LED
 * @param[in]   me              instance of the driver
 * @param[in]   ledIdtbl        LED driver id table
 * @param[in]   total_leds      #led
 */
void    _LedSrvGroup_setLedOffByArray(cLedSrv *me, uint8 ledIdtbl[], uint8 total_leds)
{
    uint8   i;

    for (i=0;i<total_leds;i++)
        me->ledDrvList[ledIdtbl[i]]->pLedFunc->LedOff(me->ledDrvList[ledIdtbl[i]]);
}

/* @brief       set the value of the LED with given table
 * @param[in]   me              instance of the driver
 * @param[in]   ledIdtbl        LED driver id table
 * @param[in]   total_leds      #led
 */
void    _LedSrvGroup_setBrightByArray(cLedSrv *me, uint8 ledIdtbl[], uint8 led_light_array[], uint8 total_leds)
{
    uint8   i;

    for (i=0;i<total_leds;i++)
        me->ledDrvList[ledIdtbl[i]]->pLedFunc->LedSetColor(me->ledDrvList[ledIdtbl[i]], led_light_array[i]);
}

/* @brief       show multiple LED at a time
 * @param[in]   me              instance of the driver
 * @return      1:finished
                0:continue
 */
uint8   LedSrvGroup_Show(cLedSrv *me)
{
    LEDGROUP_s          *grp_ptr = &me->group_s;
    tGroupPatternData   *pat_ptr = &grp_ptr->userReq.pattern_tbl[grp_ptr->group_index];
    LedReqGroupEvt      *userReq_ptr = &grp_ptr->userReq;
    uint32              time_elapsed;
    uint8               switch_to_next = 0;
    uint8               led_light_array[LED_MAX] = {0};
    uint8               off_index, on_index;
    eLed                led_id;
    uint8               finished = 0;
    char    *gpcTbl[]={
        "GPC_Init",
        "KnightRider",
        "OnByVolume",
        "OffByVolume",
        "FadeInByVolume",
        "FadeOutByVolume",
        "Delay",
    };

    ASSERT(me->grp_enabled);

    if (grp_ptr->pattStart != 0)
    {
        time_elapsed = getSysTime() - grp_ptr->pattStart;
        if (time_elapsed < pat_ptr->periodTime)
            goto quit;
    }

    if (0 && grp_ptr->pattern_init && !grp_ptr->pattern_loop)
    {
        ASSERT(pat_ptr->cmd < ArraySize(gpcTbl));
        printf("dynamic id:%30s, periodTime:%dms\n", gpcTbl[pat_ptr->cmd], pat_ptr->periodTime);
    }

    switch (pat_ptr->cmd)
    {
    case GPC_KnightRider:
        //WARNING(pat_ptr->periodTime);
        if (grp_ptr->pattern_init && !grp_ptr->pattern_loop)
        {
            grp_ptr->nr_upward = 1;
            grp_ptr->nr_pinIndex = 0;
        }
        on_index = off_index = grp_ptr->nr_pinIndex;

        if (grp_ptr->nr_upward)
        {
            on_index ++;
            if (on_index == grp_ptr->total_leds)
            {
                grp_ptr->nr_upward = 0;
            }
        }
        else    /* downward */
        {
            on_index --;
            if (on_index == 1)
            {
                grp_ptr->nr_upward = 1;
                switch_to_next = 1;
            }
        }
        /* turn on led */
        led_id = grp_ptr->ledIdtbl[on_index - 1];
        me->ledDrvList[led_id]->pLedFunc->LedOn(me->ledDrvList[led_id]);
        grp_ptr->nr_pinIndex = on_index;

        /* turn off previous led */
        if (off_index)
        {
            led_id = grp_ptr->ledIdtbl[off_index - 1];
            me->ledDrvList[led_id]->pLedFunc->LedOff(me->ledDrvList[led_id]);
        }
        //printf("on:%d, off:%d\n", on_index, off_index);
        break;
    case GPC_FadeInByVolume:
        //WARNING(pat_ptr->periodTime);
        if (pat_ptr->periodTime)
        {
            if (grp_ptr->pattern_init)
                grp_ptr->fade_level = 0;
            else
                grp_ptr->fade_level ++;
            _LedSrvGroup_calcGrayValueByArray(grp_ptr->volLedValue, grp_ptr->total_leds, led_light_array, grp_ptr->fade_level, grp_ptr->volToOneLed_levels);
            _LedSrvGroup_setBrightByArray(me, grp_ptr->ledIdtbl, led_light_array, grp_ptr->total_leds);
            if (grp_ptr->fade_level == grp_ptr->volToOneLed_levels)
                switch_to_next = 1;
            //printf("GPC_FadeInByVolume:grp_ptr->fade_level:%d\n", grp_ptr->fade_level);
        }
        else
        {
            _LedSrvGroup_setBrightByArray(me, grp_ptr->ledIdtbl, grp_ptr->volLedValue, grp_ptr->total_leds);
            switch_to_next = 1;
        }
        break;
    case GPC_OnByVolume:
        if (grp_ptr->pattern_init && pat_ptr->periodTime)
            grp_ptr->cur_volLevel = 0;
        else if (pat_ptr->periodTime)
            grp_ptr->cur_volLevel ++;
        else
            grp_ptr->cur_volLevel = userReq_ptr->volume;
        _LedSrvGroup_voumeToLedArray(me, grp_ptr->cur_volLevel, led_light_array, grp_ptr->total_leds);
        _LedSrvGroup_setBrightByArray(me, grp_ptr->ledIdtbl, led_light_array, grp_ptr->total_leds);
        if (grp_ptr->cur_volLevel == userReq_ptr->volume)
            switch_to_next = 1;
        break;

    case GPC_FadeOutByVolume:
        //WARNING(pat_ptr->periodTime);
        if (pat_ptr->periodTime)
        {
            if (grp_ptr->pattern_init)
                grp_ptr->fade_level = grp_ptr->volToOneLed_levels;
            else
                grp_ptr->fade_level --;
            _LedSrvGroup_calcGrayValueByArray(grp_ptr->volLedValue, grp_ptr->total_leds, led_light_array, grp_ptr->fade_level, grp_ptr->volToOneLed_levels);
            _LedSrvGroup_setBrightByArray(me, grp_ptr->ledIdtbl, led_light_array, grp_ptr->total_leds);
            if (grp_ptr->fade_level == 0)
                switch_to_next = 1;
            //printf("GPC_FadeOutByVolume:grp_ptr->fade_level:%d\n", grp_ptr->fade_level);
        }
        else
        {
            _LedSrvGroup_setLedOffByArray(me, grp_ptr->ledIdtbl, grp_ptr->total_leds);
            switch_to_next = 1;
        }
        break;
    case GPC_OffByVolume:
        if (grp_ptr->pattern_init && pat_ptr->periodTime)
            grp_ptr->cur_volLevel = userReq_ptr->volume;
        else if (pat_ptr->periodTime)
            grp_ptr->cur_volLevel --;
        else
            grp_ptr->cur_volLevel = -1;
        if (grp_ptr->cur_volLevel == -1)
        {
            _LedSrvGroup_setLedOffByArray(me, grp_ptr->ledIdtbl, grp_ptr->total_leds);
            switch_to_next = 1;
        }
        else
        {
            _LedSrvGroup_voumeToLedArray(me, grp_ptr->cur_volLevel, led_light_array, grp_ptr->total_leds);
            _LedSrvGroup_setBrightByArray(me, grp_ptr->ledIdtbl, led_light_array, grp_ptr->total_leds);
        }
        break;
    case GPC_Delay:
        if (!grp_ptr->pattern_first)
            switch_to_next = 1;
        break;
    default:
        ASSERT(0);
        break;
    }
    grp_ptr->pattern_first = 0;
    grp_ptr->pattern_init = 0;
    grp_ptr->pattern_loop = 0;

    grp_ptr->pattStart = getSysTime();
    if (switch_to_next)
    {
        grp_ptr->pattern_init = 1;
        if (pat_ptr->next_pattern == 0) /* user not assign, continue to next */
            grp_ptr->group_index ++;
        else if (pat_ptr->next_pattern == GPC_NoNext)
            finished = 1;
        else if (grp_ptr->group_index+1 == pat_ptr->next_pattern)    /* same */
            grp_ptr->pattern_loop = 1;
        else    /* user assign */
            grp_ptr->group_index = pat_ptr->next_pattern-1;

        if (grp_ptr->userReq.pattern_tbl[grp_ptr->group_index].cmd == GPC_Init)  /* next is NULL */
            finished = 1;
    }
quit:
    return finished;
}

void    LedSrvGroup_SetPatt(QActive* sender, ledMask leds, ePattern patternId, tGroupPatternData *pattern_tbl, uint8 volume, uint8 *volToOneLed_tbl)
{
    LedReqGroupEvt* req  = Q_NEW(LedReqGroupEvt, LED_REQ_SIG);

    ASSERT(pattern_tbl);
    
    req->sender     = sender;
    req->ledCommand = LED_PAT_ON_CMD;
    req->leds       = leds;
    req->patternId  = patternId;

    /* extra parameter */
    req->pattern_tbl        = pattern_tbl;
    req->volume             = volume;
    req->volToOneLed_tbl    = volToOneLed_tbl;

    SendToServer(LED_SRV_ID, (QEvt*)req);
}

