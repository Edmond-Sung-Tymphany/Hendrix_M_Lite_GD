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

    me->isPatternRunning = TRUE;
    if (pattId == PAT_MAX_NUMBER)
    {
        // special handling for FORE to BACK transition
        me->layer = layer = LED_LAYER_BACK;
        me->patt[LED_LAYER_FORE] = &patternConfig[OFF_PATT];
    }
    else
    {
        const tPatternData *patt = &patternConfig[pattId];
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
        const tPatternData *patt = me->patt[me->layer];

        /*
            TODO: Fix a bug. Current design does not support patterns
            with "PREVIOUS_COLOR" and "FADE_XXX_STYLE" together.
        */
        //Get previous color
        me->prevPattColor= me->color;
        Color color= patt->color;
        if( patt->color==PREVIOUS_COLOR ) {
            color= me->prevPattColor;
        }           

        //Set color based on pattern
        if (patt->style == FADE_STYLE)
        {
            // Init the fade brightness
            me->color = getFadeColor(patt, 0);
        }
        else if (patt->style == FADE_IN_STYLE)
        {
            // Init the fade In brightness
            me->color = getFadeInColor(patt, 0);
        }
        else if (patt->style == FADE_OUT_STYLE)
        {
            // Init the fade brightness
            me->color = getFadeOutColor(patt, 0);
        }
#ifndef LED_HAS_RGB
        else if (patt->style == FADE_IN_EX_STYLE)
        {
            me->color = getFadeInExColor(patt, 0, color);
        }
        else if (patt->style == FADE_OUT_EX_STYLE)
        {
            me->color = getFadeOutExColor(patt, 0, color);
        }
#endif        
        else
        {
            me->color = color;
        }
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
        const tPatternData *patt = me->patt[me->layer];
        if (patt->style == FADE_STYLE)
        {
            // Init the fade brightness
            me->color = getFadeColor(patt, 0);
        }
        else
        {
            me->color = patt->color;
        }
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


    //Get previuos color
    Color color= patt->color;
    if( patt->color==PREVIOUS_COLOR ) {
        color= me->prevPattColor;
    }           


    switch (patt->style)
    {
    case BLINK_STYLE:
    {
        // Update Blink style
        if (patt->onTime > timeElapsed % patt->periodTime)
        {
            me->color = patt->color;
        }
        else
        {
#ifdef LED_HAS_RGB
            me->color = LED_OFF_LEVEL;
#else
            me->color = SET_BRIGHTNESS(color , 0);
#endif
        }
        break;
    }
    case FADE_STYLE:
    {
        // Update Fade style
        me->color = getFadeColor(patt, timeElapsed);
        break;
    }
    case FADE_IN_STYLE:
    {
        me->color = getFadeInColor(patt, timeElapsed);
        break;
    }
    case FADE_OUT_STYLE:
    {
        me->color = getFadeOutColor(patt, timeElapsed);
        break;
    }
#ifndef LED_HAS_RGB
    case FADE_IN_EX_STYLE:
    {
        me->color = getFadeInExColor(patt, timeElapsed, color);
        break;
    }
    case FADE_OUT_EX_STYLE:
    {
        me->color = getFadeOutExColor(patt, timeElapsed, color);
        break;
    }
#endif    
    case SOLID_STYLE:
    {
        me->color = color;        
        break;
    }
    default:
        ASSERT(0);
    }

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


/* @brief       Get the brightness of the pattern referring to the current time
 * @param[in]   patt       pointer to pattern information
 * @param[in]   elapsed    elapsed time
 * @return      Color brightness of the corresponding timing
 */
static Color getFadeColor(const tPatternData *patt, uint32 elapsed)
{
    Color ret = 0;

    elapsed %= patt->periodTime;
    if (elapsed < patt->onTime)
    {
        if (elapsed > patt->onTime / 2)
        {
            elapsed = patt->onTime - elapsed;
        }
        
        ASSERT(patt->fading_data);
        uint32 fade_index= elapsed * FADING_DATA_SIZE / patt->onTime;
        ASSERT( fade_index < FADING_DATA_SIZE );
        uint8 fade = patt->fading_data[fade_index];

#ifdef LED_HAS_RGB
        uint8 r = ((uint32)GET_RED(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 g = ((uint32)GET_GREEN(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 b = ((uint32)GET_BLUE(patt->color)) * fade / MAX_BRIGHTNESS;
        ret = RGBA(r,g,b,0);
#else
        uint8 brightness= GET_BRIGHTNESS(patt->color) * fade / MAX_BRIGHTNESS;
        ret = SET_BRIGHTNESS(patt->color, brightness);
#endif
    }

    return ret;
}


/* @brief       Get the brightness of the pattern referring to the current time for Fade in style
 * @param[in]   patt       pointer to pattern information
 * @param[in]   elapsed    elapsed time
 * @return      Color brightness of the corresponding timing
 */
static Color getFadeInColor(const tPatternData *patt, uint32 elapsed)
{
    Color ret = 0;

    {                
        ASSERT(patt->fading_data);
        uint32 fade_index= elapsed * FADING_DATA_SIZE / patt->duration;
        ASSERT( fade_index < FADING_DATA_SIZE );
        uint8 fade = patt->fading_data[fade_index];

#ifdef LED_HAS_RGB
        uint8 r = ((uint32)GET_RED(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 g = ((uint32)GET_GREEN(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 b = ((uint32)GET_BLUE(patt->color)) * fade / MAX_BRIGHTNESS;
        ret = RGBA(r,g,b,0);
#else
        uint8 brightness= GET_BRIGHTNESS(patt->color) * fade / MAX_BRIGHTNESS;
        ret = SET_BRIGHTNESS(patt->color, brightness);
#endif
    }

    return ret;
}

/* @brief       Get the brightness of the pattern referring to the current time for Fade Out style
 * @param[in]   patt       pointer to pattern information
 * @param[in]   elapsed    elapsed time
 * @return      Color brightness of the corresponding timing
 */
static Color getFadeOutColor(const tPatternData *patt, uint32 elapsed)
{
    Color ret = 0;

    ASSERT(patt->duration > elapsed);
    elapsed = patt->duration-elapsed-1;
    {
        ASSERT(patt->fading_data);
        uint32 fade_index= elapsed * FADING_DATA_SIZE / patt->periodTime;
        ASSERT( fade_index < FADING_DATA_SIZE );
        uint8 fade = patt->fading_data[fade_index];
        
#ifdef LED_HAS_RGB
        uint8 r = ((uint32)GET_RED(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 g = ((uint32)GET_GREEN(patt->color)) * fade / MAX_BRIGHTNESS;
        uint8 b = ((uint32)GET_BLUE(patt->color)) * fade / MAX_BRIGHTNESS;
        ret = RGBA(r,g,b,0);
#else
        uint8 brightness= GET_BRIGHTNESS(patt->color) * fade / MAX_BRIGHTNESS;
        ret = SET_BRIGHTNESS(patt->color, brightness);
#endif
    }
    return ret;
}



#ifndef LED_HAS_RGB

/* @brief       Get the brightness of the pattern referring to the current time for Fade in style
 * @param[in]   patt       pointer to pattern information
 * @param[in]   elapsed    elapsed time
 * @return      Color brightness of the corresponding timing
 */
static Color getFadeInExColor(const tPatternData *patt, uint32 elapsed, Color color)
{
    Color ret = 0;
    ASSERT(patt->fading_data==NULL);
    
    uint32 brighBase= GET_BRIGHTNESS(color);
    uint32 brightness= brighBase +  elapsed*(255-brighBase)/patt->duration; 
    ASSERT(brightness<=255);
    ret = SET_BRIGHTNESS(color, (uint8)brightness);

    return ret;
}

/* @brief       Get the brightness of the pattern referring to the current time for Fade Out style
 * @param[in]   patt       pointer to pattern information
 * @param[in]   elapsed    elapsed time
 * @return      Color brightness of the corresponding timing
 */
static Color getFadeOutExColor(const tPatternData *patt, uint32 elapsed, Color color)
{
    Color ret = 0;
    ASSERT(patt->fading_data==NULL);

    uint32 brighBase= GET_BRIGHTNESS(color);
    uint32 brightness= brighBase * (patt->duration-elapsed) / patt->duration; 
    ASSERT(brightness<=255);
    ret = SET_BRIGHTNESS(color, (uint8)brightness);

    return ret;

}
#endif /* #ifndef LED_HAS_RGB */


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

