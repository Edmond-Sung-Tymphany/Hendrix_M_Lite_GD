/**
 *  @file      LedDrv.c
 *  @brief     This file defines the driver to control LEDs
 *  @author    Johnny Fan
 *  @date      11-Feb-2014
 *  @copyright Tymphany Ltd.
 */
#include "LedDrv_priv.h"
#include "trace.h"
#include "bsp.h"        // getSysTime()

#include "pattern.Config"

/* Private functions / variables. Declare and drivers here */

/*****************************************************************************************************************
 *
 * Start-up / shut-down functions
 *
 *****************************************************************************************************************/

/* @brief       Led driver constructor
 * @param[in]   me          pointer to Led driver object
 * @param[in]   pConfig     pointer to Led config
 */
void LedDrv_Ctor(cLedDrv* me, const tDevice* pConfig)
{
    eLedLayer i;

    // init Driver member
    me->pConfig         = pConfig;
    me->layer           = LED_LAYER_BACK;
    me->pattStart       = 0;
    me->isPatternRunning = FALSE;
    for (i = LED_LAYER_BACK; i < MAX_LED_LAYER; i++)
    {
        me->patt[i] = &patternConfig[OFF_PATT];
    }
}

/* @brief       Led driver destructor
 * @param[in]   me          pointer to Led driver object
 */
void LedDrv_Xtor(cLedDrv* me)
{
    me->isPatternRunning = FALSE;
    me->pLedFunc->LedXtor(me);
}

/*****************************************************************************************************************
 *
 * Public functions
 *
 *****************************************************************************************************************/

/* @brief       Apply the pattern with the provided ID
 * @param[in]   me          pointer to Led driver object
 * @param[in]   pattId      enum to pattern information
 */
void LedDrv_PattSet(cLedDrv *me, ePattern pattId)
{
    eLedLayer layer;
    ASSERT(me);
    const tPatternData *patt = &patternConfig[pattId];

    me->isPatternRunning = TRUE;
    if (pattId == PAT_MAX_NUMBER)
    {
        // special handling for FORE to BACK transition
        me->layer = layer = LED_LAYER_BACK;
        me->patt[LED_LAYER_FORE] = &patternConfig[OFF_PATT];
    }
    else
    {

        layer = getPattLayer(patt);
        if (me->patt[layer] != patt)
        {
            me->patt[layer] = patt;
        }
        else
        {
            // when the pattern is set twice, ignore
            return;
        }

        if (layer == LED_LAYER_FORE)
        {
            me->layer = layer;
        }
    }

    me->pattStart = getSysTime();


    // Update the brightness if needed
    if (!((layer == LED_LAYER_BACK) && (me->layer == LED_LAYER_FORE)))
    {

        //Get previous color
        me->prevPattColor = me->color;
        Color color = (me->patt[me->layer])->color;
        if( color == PREVIOUS_COLOR )
        {
            color = me->prevPattColor;
        }

        // get color from color with fade in patt
        me->color = getColor(me->patt[me->layer],0,color);

        LedDrv_UpdateColor(me);
    }

}

/* @brief       Apply the pattern with the provided data
 * @param[in]   me          pointer to Led driver object
 * @param[in]   pattId      enum to pattern information
 */
void LedDrv_PattSetCustom(cLedDrv *me, tPatternData *patt)
{
    eLedLayer layer = getPattLayer(patt);

    me->isPatternRunning = TRUE;
    me->patt[layer] = patt;

    if (layer == LED_LAYER_FORE)
    {
        me->layer = layer;
    }

    me->pattStart = getSysTime();

    // Update the brightness if needed
    if (!((layer == LED_LAYER_BACK) && (me->layer == LED_LAYER_FORE)))
    {
        //Get previous color
        me->prevPattColor = me->color;
        Color color = (me->patt[me->layer])->color;
        if( color == PREVIOUS_COLOR )
        {
            color = me->prevPattColor;
        }

        // get color from color with fade in patt
        me->color = getColor(me->patt[me->layer],0,color);
        LedDrv_UpdateColor(me);
    }

}

/* @brief       Stop pattern from running
 * @param[in]   me          pointer to Led driver object
 */
void LedDrv_PattStop(cLedDrv *me)
{
    me->isPatternRunning = FALSE;
}

/* @brief       Update the Led brightness according to the pattern set
 * @param[in]   me          pointer to Led driver object
 * @return      Pattern ID if the pattern is finsihed,
 *              so sever could publish what is finished
 */
ePattern LedDrv_PattShow(cLedDrv *me)
{
    ePattern ret = PAT_MAX_NUMBER;
    if (!me->isPatternRunning)
    {
        return ret;
    }

    const tPatternData *patt = me->patt[me->layer];
    uint32 timeElapsed = getSysTime() - me->pattStart;

    if (patt->duration <= timeElapsed)
    {
        ret = getPattId(me->patt[me->layer]);       // tell everyone an led pattern is finished
        LedDrv_PattSet(me, patt->nextPattern);

        /* LedDrv_PattSet() assign new patten and update pattStart
         * thus we reset timeElapsed.
         */
        timeElapsed = 0;

        patt = me->patt[me->layer];     // make sure the pattern checking is up-to-date
    }

    Color color = (me->patt[me->layer])->color;
    if( color == PREVIOUS_COLOR )
    {
        color = me->prevPattColor;
    }

    // get color from color with fade in patt
    me->color = getColor(me->patt[me->layer],timeElapsed,color);


    // send command
    LedDrv_UpdateColor(me);
    return ret;
}

/*****************************************************************************************************************
 *
 * private functions
 *
 *****************************************************************************************************************/

/* @brief       Check if the pattern is ALWAYS_REPEAT
 * @param[in]   patt       pointer to pattern information
 * @return      true: is ALWAYS_REPEAT; false otherwise
 * @sa          ALWAYS_REPEAT
 */
static bool isPattForever(const tPatternData *patt)
{
    return (patt->duration == ALWAYS_REPEAT);
}

/* @brief       Get the pattern ID from the pointer to pattern
 * @param[in]   patt       pointer to pattern information
 * @return      Pattern ID, PAT_MAX_NUMBER if the given pointer is invalid
 */
static ePattern getPattId(const tPatternData *patt)
{
    const tPatternData *patternConfigUpperBond = &patternConfig[PAT_MAX_NUMBER - 1];
    ePattern ret = PAT_MAX_NUMBER;

    // check if the pattern pointer is within the predefined range
    if ( (patternConfig <= patt)
         && (patternConfigUpperBond >= patt) )
    {
        ret = (ePattern)(patt - patternConfig);
        ASSERT(ret>=0 && ret<PAT_MAX_NUMBER);
    }
    return ret;
}

/* @brief       Check which layer should the pattern to be in
 * @param[in]   patt       pointer to pattern information
 * @return      Led layer of the pattern
 */
static eLedLayer getPattLayer(const tPatternData *patt)
{
    eLedLayer ret = LED_LAYER_BACK;
    /* check if this pattern is linked to forever repeated pattern */
    uint8 count = 1;
    const tPatternData *nextPatt= patt;

    while ( !isPattForever(nextPatt) )
    {
        //Enter loop menas pattern is not forever
        const uint8 PATT_MAX_LOOP = 10;
        count++;

        if( nextPatt->nextPattern==PAT_MAX_NUMBER )
        {
            ret = LED_LAYER_FORE;
            break;
        }
        else if (count > PATT_MAX_LOOP)
        {
            //get maximum count, assume back ground patterns
            break;
        }
        nextPatt = &patternConfig[nextPatt->nextPattern];
    }
    return ret;
}


static Color getColor(const tPatternData * pPatt,uint32 elapsed,Color color)
{
    Color ret = 0;
    uint8 fade = 0;
    uint32 fade_index = 0;

    // get fade
    switch(pPatt->style)
    {
        case BLINK_STYLE:
        {
            // Update Blink style
            if (pPatt->onTime > elapsed % pPatt->periodTime)
            {
                ret = color;
            }
            else
            {
#ifdef LED_HAS_RGB
                ret = LED_OFF_LEVEL;
#else
                ret = SET_BRIGHTNESS(color , 0);
#endif
            }
            return ret;
        }
        case FADE_STYLE:
            elapsed %= pPatt->periodTime;
            if (elapsed < pPatt->onTime)
            {
                if (elapsed > pPatt->onTime / 2)
                {
                    elapsed = pPatt->onTime - elapsed;
                }

                ASSERT(pPatt->fading_data);
                fade_index= elapsed * FADING_DATA_SIZE / pPatt->onTime;
                ASSERT( fade_index < FADING_DATA_SIZE );
                fade = pPatt->fading_data[fade_index];
            }
            break;
        //case FADE_IN_EX_STYLE:
        case FADE_IN_STYLE:
            fade_index= elapsed * FADING_DATA_SIZE / pPatt->duration;
            ASSERT( fade_index < FADING_DATA_SIZE );
            fade = pPatt->fading_data[fade_index];
            break;
        //case FADE_OUT_EX_STYLE:
        case FADE_OUT_STYLE:
            ASSERT(pPatt->fading_data);
            fade_index= (pPatt->duration-elapsed-1) * FADING_DATA_SIZE / pPatt->duration;
            ASSERT( fade_index < FADING_DATA_SIZE );
            fade = pPatt->fading_data[fade_index];
            break;
        case FADE_OUT_EX_STYLE:
            fade = GET_BRIGHTNESS(color)/255;
            fade = fade * (pPatt->duration-elapsed) / pPatt->duration;
            break;
        case FADE_IN_EX_STYLE:
            fade = GET_BRIGHTNESS(color)/255;
            fade = fade +  elapsed*(MAX_BRIGHTNESS-fade)/pPatt->duration;
            break;

        default:
            ret = color;
            return ret;
    }
    if(pPatt->color == PREVIOUS_COLOR)
    {
        ret = getPrevColorFade(fade,color,patternConfig[pPatt->nextPattern].color);
    }
    else
    {
        ret = getColorFade(fade,color);
    }
    return ret;
}

static Color getColorFade(uint8 fade,Color RGBA)
{
    Color ret = 0;
#ifdef LED_HAS_RGB
    uint8 r = ((uint32)GET_RED(RGBA)) * fade / MAX_BRIGHTNESS;
    uint8 g = ((uint32)GET_GREEN(RGBA)) * fade / MAX_BRIGHTNESS;
    uint8 b = ((uint32)GET_BLUE(RGBA)) * fade / MAX_BRIGHTNESS;
    uint8 brightness = ((uint32)GET_BRIGHTNESS(RGBA)) * fade / MAX_BRIGHTNESS;
    ret = RGBA(r,g,b,brightness);
#else
    uint8 brightness= GET_BRIGHTNESS(RGBA) * fade / MAX_BRIGHTNESS;
    ret = SET_BRIGHTNESS(RGBA, brightness);
#endif
    return ret;
}

static Color getPrevColorFade(uint8 fade, Color orig_RGBA,Color tar_RGBA)
{
    Color ret = 0;
    int32 diff;

#ifdef LED_HAS_RGB
    uint8 r;
    diff = GET_RED(tar_RGBA) - GET_RED(orig_RGBA);
    r = diff * fade / MAX_BRIGHTNESS + GET_RED(orig_RGBA);

    uint8 g;
    diff = GET_GREEN(tar_RGBA) - GET_GREEN(orig_RGBA);
    g = diff * fade / MAX_BRIGHTNESS + GET_GREEN(orig_RGBA);

    uint8 b;
    diff = GET_BLUE(tar_RGBA) - GET_BLUE(orig_RGBA);
    b = diff * fade / MAX_BRIGHTNESS + GET_BLUE(orig_RGBA);

    diff = GET_BRIGHTNESS(tar_RGBA) - GET_BRIGHTNESS(orig_RGBA);
    uint8 brightness = diff * fade / MAX_BRIGHTNESS + GET_BRIGHTNESS(orig_RGBA);

    ret = RGBA(r,g,b,brightness);
#else
    diff = GET_BRIGHTNESS(tar_RGBA) - GET_BRIGHTNESS(orig_RGBA);
    uint8 brightness = (diff * fade / MAX_BRIGHTNESS + GET_BRIGHTNESS(orig_RGBA));
    ret = SET_BRIGHTNESS(orig_RGBA, brightness);
#endif
    return ret;
}

/* @brief       Update the color if it is different from the previous
 * @param[in]   me          pointer to Led driver object
 */
static void LedDrv_UpdateColor(cLedDrv *me)
{
    if(me->color != me->prevColor)
    {
        me->pLedFunc->LedSetColor(me, me->color);
        me->prevColor = me->color;
    }
}

