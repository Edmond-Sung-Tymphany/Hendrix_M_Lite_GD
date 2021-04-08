#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_allplay_cmd.h"

#ifndef NDEBUG
/* allplay command */

#include "allplaymcu.h"
#include "DebugSrv_priv.h"
extern allplay_ctx_t* apctx;

static portBASE_TYPE allplay_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    allplay_status status = ALLPLAY_NOT_CONNECTED;

    const char *pszParam;
    unsigned length;

    if( apctx != NULL )
    {
        pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
        if( strncmp( pszParam, "play", length ) == 0 )
        {
            status = allplay_play(apctx, (void*)0);
        }
        if( strncmp( pszParam, "prev", length ) == 0 )
        {
            status = allplay_previous(apctx, (void*)0);
        }
        if( strncmp( pszParam, "pause", length ) == 0 )
        {
            status = allplay_pause(apctx, (void*)0);
        }
        if( strncmp( pszParam, "stop", length ) == 0 )
        {
            status = allplay_stop(apctx, (void*)0);
        }
        if( strncmp( pszParam, "next", length ) == 0 )
        {
            status = allplay_next(apctx, (void*)0);
        }
        if( strncmp( pszParam, "mute", length ) == 0 )
        {
            allplay_mute(apctx, (void*)0, 1);
        }
        if( strncmp( pszParam, "unmute", length ) == 0 )
        {
            allplay_mute(apctx, (void*)0, 0);
        }
        if( strncmp( pszParam, "vol_up", length ) == 0 )
        {
            status = allplay_volume_adjust(apctx, (void*)0, 2);
        }
        if( strncmp( pszParam, "vol_down", length ) == 0 )
        {
            status = allplay_volume_adjust(apctx, (void*)0, -2);
        }
        if( strncmp( pszParam, "get_net", length ) == 0 )
        {
            status = allplay_network_get_info(apctx, (void*)0);
        }
        if( strncmp( pszParam, "get_mode", length ) == 0 )
        {
            status = allplay_get_system_mode(apctx, (void*)0);
        }
        if( strncmp( pszParam, "get_state", length ) == 0 )
        {
            status = allplay_get_player_state(apctx, (void*)0);
        }
        if( strncmp( pszParam, "get_vol", length ) == 0 )
        {
            status = allplay_get_volume_info(apctx, (void*)0);
        }
        if( strncmp( pszParam, "setup", length ) == 0 )
        {
            status = allplay_start_setup(apctx, (void*)0);
        }
        if( strncmp( pszParam, "direct_mode_on", length ) == 0 )
        {
            status = allplay_directmode_enable(apctx, (void*)0, 1);
            DebugSrvOutput("direct mode on\r\n",0);
        }
        if( strncmp( pszParam, "direct_mode_off", length ) == 0 )
        {
            status = allplay_directmode_enable(apctx, (void*)0, 0);
            DebugSrvOutput("direct mode off\r\n",0);
        }
        if( strncmp( pszParam, "factory_reset", length ) == 0 )
        { /* TODO: to be implemented */
//            _WaitTimeMs(500);
//            SAM_GPIO13_PIN = 1;
//            status = ALLPLAY_ERROR_NONE;
            status = ALLPLAY_ERROR_NONE;
            DebugSrvOutput("After configuration you will need to power cycle (until IN5197 is fixed)\r\n",0);
        }
        if( strncmp( pszParam, "fw-check", length ) == 0 )
        {
            status = allplay_firmware_check(apctx, (void*)0);
            DebugSrvOutput("firmware check\r\n",0);
        }
        if( strncmp( pszParam, "fw-update", length ) == 0 )
        {
            status = allplay_firmware_update(apctx, (void*)0);
            DebugSrvOutput("firmware update\r\n",0);
        }
    }

    switch( status )
    {
        case ALLPLAY_ERROR_NONE:
            DebugSrvOutput("allplay success\r\n",0);
            break;
        case ALLPLAY_ERROR_FAILED:
            DebugSrvOutput("allplay error\r\n",0);
            break;
        case ALLPLAY_NOT_CONNECTED:
            DebugSrvOutput("allplay failed, not connected\r\n",0);
            break;
        default: break;
    }

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_allplay_cmd =
{
	.pcCommand = (int8_t *)"allplay",
	.pcHelpString = (int8_t *)"allplay {play | pause | stop | prev | next | vol_up | vol_down | mute | unmute | get_vol | get_net | get_mode | get_state | setup | direct_mode_on | direct_mode_off | factory_reset | fw-check | fw-update}\r\n",
	.pxCommandInterpreter = allplay_cmd_callback,
	.cExpectedNumberOfParameters = 1,
};
#endif
