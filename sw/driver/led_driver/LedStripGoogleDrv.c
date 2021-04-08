/**
*  @file      LedStripGoogleDrv.c
*  @brief     Google Led Driver
*  @version   v0.1
*  @author    Alex.Li
*  @date      2017/12/19
*  @copyright Tymphany Ltd.
*/

#include "LedStripGoogleDrv.h"
#include "LedStripGoogle.Config"
#include "./LedStripGoogleDrv_priv.h"
#include "trace.h"

void LedStripGoogle_Ctor(cGoogleLedDrv * me)
{
    me->stripConfig = NULL;
    me->stripPattNow = SPAT_GOOGLE_OFF;
    me->super_.Strip_SetPatt = LedStripGoogle_SetPatt;
    me->super_.Strip_Update = LedStripGoogle_Update;
    LedStripGoogle_Reset(me);
}

void LedStripGoogle_Xtor(cGoogleLedDrv * me)
{
    me->stripConfig = NULL;
    me->stripPattNow = SPAT_GOOGLE_OFF;
    me->super_.Strip_SetPatt = NULL;
    me->super_.Strip_Update = NULL;
    LedStripGoogle_Reset(me);
}

void LedStripGoogle_SetPatt(void * me, eStripPattern patt)
{
    if(patt <= SPAT_GOOGLE_OFF && patt >= SPAT_GOOGLE_HOTWORD)
    {
        if(((cGoogleLedDrv *)me)->stripPattNow == SPAT_GOOGLE_HOTWORD)
        {
            ((cGoogleLedDrv *)me)->stripPattNext = patt;
        }

        else if(patt != ((cGoogleLedDrv *)me)->stripPattNow)
        {
            LedStripGoogle_Reset((cGoogleLedDrv *)me);
        }

        ((cGoogleLedDrv *)me)->stripPattNow = patt;
        ((cGoogleLedDrv *)me)->stripConfig = (void *)(&ledStripConfig[patt]);
    }
}

void LedStripGoogle_Update(void * me, cLedDrv** ledDrvList)
{
    switch(((cGoogleLedDrv *)me)->stripPattNow)
    {
        case SPAT_GOOGLE_HOTWORD:
        {
            LedStripGoogle_HotWord(((cGoogleLedDrv *)me));
        }
        break;

        case SPAT_GOOGLE_THINKING:
        {
            LedStripGoogle_Thinking(((cGoogleLedDrv *)me));
        }
        break;

        case SPAT_GOOGLE_LISTENING:
        case SPAT_GOOGLE_RESPONDING:
        {
            LedStripGoogle_Listening_Responding(((cGoogleLedDrv *)me));
        }
        break;

        case SPAT_GOOGLE_OFF:
        {
            LedStripGoogle_Reset(((cGoogleLedDrv *)me));
        }
        break;
    }

    if(((cGoogleLedDrv *)me)->stripConfig != NULL)
    {
        tGoogleLedConfig * stripConfig = (tGoogleLedConfig *)(((cGoogleLedDrv *)me)->stripConfig);

        for(int j = 0; j < stripConfig->ledNum; j++)
        {
            for(eLed i = LED_MIN; i < LED_MAX; i++)
            {
                ledMask m = 1 << i;

                if((stripConfig->leds[j]) & m)
                {
                    cLedDrv *p = ledDrvList[i];
                    LedDrv_PattStop(p);
                    p->pLedFunc->LedSetColor(p, ((cGoogleLedDrv *)me)->colors[j]);
                }
            }
        }
        if(((cGoogleLedDrv *)me)->stripPattNow == SPAT_GOOGLE_OFF)
        {
            ((cGoogleLedDrv *)me)->stripConfig = NULL;
        }
    }
}

static void LedStripGoogle_HotWord(cGoogleLedDrv * me)
{
    tGoogleLedConfig* stripConfig = me->stripConfig;
    uint32 elapsed = 0;
    uint16 fade_index = 0;

    if(me->startTime == 0)
    {
        me->startTime = getSysTime();
    }

    elapsed = getSysTime() - me->startTime;

    uint32 temp = HOTWORD_TAIL * stripConfig->fadingTime / FADING_DATA_SIZE;

    if(me->holdingTime == 0)
    {
        for(uint8 i = 0; i < GOOGLE_LED_NUM; i++)
        {
            if(elapsed >= temp * i)
            {
                if(elapsed - temp * i <= stripConfig->fadingTime)
                    fade_index = (elapsed - temp * i) * FADING_DATA_SIZE / stripConfig->fadingTime;

                else
                    fade_index = FADING_DATA_SIZE - 1;
            }

            else
            {
                fade_index = 0;
            }

            fade_index = fade_index > (FADING_DATA_SIZE - 1) ? FADING_DATA_SIZE - 1 : fade_index;
            fade_index = me->isFadingIn ? fade_index : FADING_DATA_SIZE - 1 - fade_index;
            me->colors[i] = SET_BRIGHTNESS(stripConfig->color, GOOGLE_LEDS_FADING_TABLE[fade_index]);
        }
    }

    if((GET_BRIGHTNESS(me->colors[GOOGLE_LED_NUM - 1]) == GOOGLE_LEDS_FADING_TABLE[FADING_DATA_SIZE - 1]) && me->isFadingIn)
    {
        me->startTime = 0;

        if(me->holdingTime == 0)
        {
            me->holdingTime = getSysTime();
        }
    }

    if((getSysTime() - me->holdingTime) >= stripConfig->holdingTime && (me->holdingTime != 0))
    {
        me->isFadingIn = false;
        me->holdingTime = 0;
        return;
    }

    if((fade_index == 0) && !me->isFadingIn && me->holdingTime == 0)
    {
        if(me->stripPattNext == NULL)
        {
            me->stripPattNow = SPAT_GOOGLE_LISTENING;
        }
        else
        {
            me->stripPattNow = me->stripPattNext;
            me->stripPattNext = NULL;
        }
        LedStripGoogle_Reset(me);
    }

}

static void LedStripGoogle_Thinking(cGoogleLedDrv * me)
{
    tGoogleLedConfig* stripConfig = me->stripConfig;
    uint32 elapsed = 0;
    uint16 fade_index = 0;

    if(me->startTime == 0)
    {
        me->startTime = getSysTime();
    }

    elapsed = getSysTime() - me->startTime;

    if(me->holdingTime == 0)
    {
        if(elapsed <= stripConfig->fadingTime)
        {
            fade_index = elapsed * FADING_DATA_SIZE / stripConfig->fadingTime;
            fade_index = fade_index > (FADING_DATA_SIZE - 1) ? FADING_DATA_SIZE - 1 : fade_index;
            fade_index = me->isFadingIn ? fade_index : FADING_DATA_SIZE - 1 - fade_index;
            me->colors[me->ledIndex] = SET_BRIGHTNESS(stripConfig->color, GOOGLE_LEDS_FADING_TABLE[fade_index]);
        }

        else
        {
            fade_index = me->isFadingIn ? FADING_DATA_SIZE - 1 : 0;
            me->colors[me->ledIndex] = SET_BRIGHTNESS(stripConfig->color, GOOGLE_LEDS_FADING_TABLE[fade_index]);
            me->startTime = 0;
            me->isFadingIn = !me->isFadingIn;

            if(me->isFadingIn)
            {
                me->holdingTime = getSysTime();
            }
        }
    }

    else
    {
        if(getSysTime() - me->holdingTime >= stripConfig->intervalTime)
        {
            me->ledIndex ++;
            me->holdingTime = 0;
            me->startTime = 0;
        }
    }

    if(me->ledIndex >= GOOGLE_LED_NUM)
    {
        LedStripGoogle_Reset(me);
    }
}

static void LedStripGoogle_Listening_Responding(cGoogleLedDrv * me)
{
    tGoogleLedConfig* stripConfig = me->stripConfig;
    uint32 elapsed = 0;
    uint16 fade_index = 0;

    if(me->startTime == 0)
    {
        me->startTime = getSysTime();
    }

    elapsed = getSysTime() - me->startTime;

    if(me->holdingTime == 0)
    {
        if(elapsed <= stripConfig->fadingTime)
        {
            fade_index = elapsed * FADING_DATA_SIZE / stripConfig->fadingTime;
            fade_index = fade_index > (FADING_DATA_SIZE - 1) ? FADING_DATA_SIZE - 1 : fade_index;
            fade_index = me->isFadingIn ? fade_index : FADING_DATA_SIZE - 1 - fade_index;
        }

        else
        {
            fade_index = me->isFadingIn ? FADING_DATA_SIZE - 1 : 0;
            me->startTime = 0;
            me->isFadingIn = !me->isFadingIn;

            if(me->isFadingIn)
            {
                me->holdingTime = getSysTime();
            }
        }

        for(uint8 i = 0; i < GOOGLE_LED_NUM; i++)
        {
            me->colors[i] = SET_BRIGHTNESS(stripConfig->color, GOOGLE_LEDS_FADING_TABLE[fade_index]);
        }
    }

    else
    {
        if(getSysTime() - me->holdingTime >= stripConfig->holdingTime)
        {
            me->holdingTime = 0;
            me->startTime = 0;
        }
    }
}

static void LedStripGoogle_Reset(cGoogleLedDrv * me)
{
    me->ledIndex = 0;
    me->startTime = 0;
    me->holdingTime = 0;
    me->isFadingIn = true;
    memset(me->colors, 0, sizeof(me->colors));
}