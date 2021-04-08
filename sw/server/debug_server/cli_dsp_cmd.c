#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_dsp_cmd.h"
//#include "hwsetup.h"
//#include "adc.h"
#include "assert.h"

#include "AudioSrv.h"
#include "KeySrv.h"
#include "controller.h"

#ifndef NDEBUG

/* adc command */

static portBASE_TYPE dsp_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;

    assert( xWriteBufferLen > 32 );

    pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );

    if( strncmp( pszParam, "vol_up", length ) == 0 )
    {
        /* dsp vol_up command */
        strncpy((char *)pcWriteBuffer, "dsp vol_up selected\r\n", xWriteBufferLen);
        KeySrv_SendKeyEvt(KEY_EVT_SHORT_PRESS,VOLUME_UP_KEY);
    }
    else if( strncmp( pszParam, "vol_down", length ) == 0 )
    {
        /* dsp vol_down command */
        strncpy((char *)pcWriteBuffer, "dsp vol_down selected\r\n", xWriteBufferLen);
        KeySrv_SendKeyEvt(KEY_EVT_SHORT_PRESS,VOLUME_DOWN_KEY);
    }
    else if( strncmp( pszParam, "plainEQ_On", length ) == 0 )
    {
        AudioSrv_Set(DSP_PLAINEQ_SETT_ID,TRUE);
    }
    else if( strncmp( pszParam, "plainEQ_Off", length ) == 0 )
    {
        AudioSrv_Set(DSP_PLAINEQ_SETT_ID,FALSE);
    }
    else
    {
        strncpy((char *)pcWriteBuffer, "invalid parameter\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_dsp_cmd =
{
	.pcCommand = (int8_t *)"dsp",
	.pcHelpString = (int8_t *)"dsp { vol_up | vol_down | plainEQ_On | plainEQ_Off } set dsp volume up or down, set DSP plain EQ on or off.\r\n",
	.pxCommandInterpreter = dsp_cmd_callback,
	.cExpectedNumberOfParameters = 1,
};
#endif
