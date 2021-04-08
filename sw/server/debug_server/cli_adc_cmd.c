#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_adc_cmd.h"
#include "assert.h"
#include "PowerSrv.h" //PowerSrv_print_batt_inverse()
#include "KeySrv.h"
#include "DebugSrv.h"
#include "controller.h"

#ifndef NDEBUG

/* adc command */
#define PowerSrv_print_batt_inverse() 

static void send_adc_req(eKeyID keyId_req)
{
    KeyDebugReqEvt* kdre = Q_NEW(KeyDebugReqEvt, CLI_ADC_MMI_REQ_SIG);
    kdre->keyId = keyId_req;
    kdre->req = DEBUG_RAW_DATA_REQ;
    kdre->keyEvent = KEY_EVT_INVALID;
    SendToServer(DEBUG_SRV_ID, (QEvt*) kdre);
}

static portBASE_TYPE adc_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;
    assert( xWriteBufferLen > 32 );

    /* read/set hi/set lo */
    pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
    //KeyDebugReqEvt kdre;
    if( strncmp( pszParam, "play", length ) == 0 )
    {
        send_adc_req(PLAY_PAUSE_KEY);
    }
    else if( strncmp( pszParam, "vol_up", length ) == 0 )
    {
        send_adc_req(VOLUME_UP_KEY);
    }
    else if( strncmp( pszParam, "vol_down", length ) == 0 )
    {
        send_adc_req(VOLUME_DOWN_KEY);
    }
    else if( strncmp( pszParam, "next", length ) == 0 )
    {
        send_adc_req(NEXT_KEY);
    }
    else if( strncmp( pszParam, "prev", length ) == 0 )
    {
        send_adc_req(PREV_KEY);
    }
    else if( strncmp( pszParam, "batt-debug", length ) == 0 )
    {
        PowerSrv_print_batt_inverse();
    }
    else
    {
        strncpy((char *)pcWriteBuffer, "adc_read invalid parameter\r\n", xWriteBufferLen);
        return pdFALSE;
    }
    
    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_adc_cmd =
{
	.pcCommand = (int8_t *)"adc",
	.pcHelpString = (int8_t *)"adc { play | vol_up | vol_down | next | prev} - returns adc reading 0..1023\r\nadc batt-debug - toggle battery adc debug data\r\n",
	.pxCommandInterpreter = adc_cmd_callback,
	.cExpectedNumberOfParameters = 1,
};
#endif