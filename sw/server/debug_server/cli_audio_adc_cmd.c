#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "trace.h"
#include "cli.h"
#include "cli_audio_adc_cmd.h"
#include "assert.h"
#include "DebugSrv.h"
#include "controller.h"
#ifdef HAS_AUDIO_ADC
#include "I2CDrv.h"
#include "AudioAdcDrv.h"


#ifndef NDEBUG

extern cAudioAdcDrv audioAdcDrv;

#define CHECK_LENGTH(length)    {if(!length){DebugSrvOutput("WARNING: the parameter is null. \r\n",0);return pdFALSE;}}

static portBASE_TYPE audio_adc_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;
    assert( xWriteBufferLen > 32 );

    pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
    if( strncmp( pszParam, "input", length ) == 0 )
    {
        const char *pInput = (const char *)CLIGetParameter( pcCommandString, 2, &length );
        CHECK_LENGTH(length);
        uint8 input = xtoi(pInput);
        AudioAdcDrv_setInput(&audioAdcDrv, input);
        TP_PRINTF("\n set input: 0x%x, successful!\r\n", input);
    }
    else if( strncmp( pszParam, "pga", length ) == 0 )
    {
        const char *pPga= (const char *)CLIGetParameter( pcCommandString, 2, &length );
        CHECK_LENGTH(length);
        uint8 pga = atoi(pPga);
        AudioAdcDrv_setPGA(&audioAdcDrv, pga);
        TP_PRINTF("\n set pga: 0x%x, successful!\r\n", pga);
    }
    else if( strncmp( pszParam, "mute", length ) == 0 )
    {
        const char *pMute= (const char *)CLIGetParameter( pcCommandString, 2, &length );
        CHECK_LENGTH(length);
        uint8 mute = atoi(pMute);
        AudioAdcDrv_enableMute(&audioAdcDrv, mute);
        TP_PRINTF("\n set mute: 0x%x, successful!\r\n", mute);
    }
    else
    {
        strncpy((char *)pcWriteBuffer, "aduio_adc invalid parameter\r\n", xWriteBufferLen);
        return pdFALSE;
    }

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_audio_adc_cmd =
{
	.pcCommand = (int8_t *)"audio_adc",
	.pcHelpString = (int8_t *)"audio_adc { input{value} | pga{value} | mute{0|1}} \r\n",
	.pxCommandInterpreter = audio_adc_cmd_callback,
	.cExpectedNumberOfParameters = 2,
};
#endif //#ifndef NDEBUG
#endif //#ifdef HAS_AUDIO_ADC