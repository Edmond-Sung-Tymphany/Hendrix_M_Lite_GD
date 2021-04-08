#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_bluetooth_cmd.h"

#ifndef NDEBUG
/* allplay command */

#include "allplaymcu.h"
#include "DebugSrv_priv.h"

#define CHECK_LENGTH(length)    {if(!length){DebugSrvOutput("WARNING: the parameter is null. \r\n",0);return pdFALSE;}}

extern allplay_ctx_t* apctx;

static portBASE_TYPE bluetooth_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    allplay_status status = ALLPLAY_NOT_CONNECTED;

    const char *pszParam;
    unsigned length;

    if( apctx != NULL )
    {
        pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
        CHECK_LENGTH(length);

        if( strncmp( pszParam, "en", length ) == 0 )
        {
           const char *pBtEnable = (const char *)CLIGetParameter( pcCommandString, 2, &length );
            CHECK_LENGTH(length);
            bool bBtEnable = (*pBtEnable - '0') ? TRUE : FALSE;
            status = allplay_bluetooth_enable(apctx, (void*)0, bBtEnable);
        }
        if( strncmp( pszParam, "pairable", length ) == 0 )
        {
            const char *pBtEnablePairing = (const char *)CLIGetParameter( pcCommandString, 2, &length );
            CHECK_LENGTH(length);
            bool bBtEnablePairing = (*pBtEnablePairing - '0') ? TRUE : FALSE;
            status = allplay_bluetooth_enable_pairing(apctx, (void*)0, bBtEnablePairing);
        }
        if( strncmp( pszParam, "get_st", length ) == 0 )
        {
            status = allplay_bluetooth_get_state(apctx, (void*)0);
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

CLI_Command_Definition_t cli_bluetooth_cmd =
{
	.pcCommand = (int8_t *)"bt",
	.pcHelpString = (int8_t *)"bt { get_st | en{0 | 1} | pairable{0 | 1}};\r\n",
	.pxCommandInterpreter = bluetooth_cmd_callback,
    /*If cExpectedNumberOfParameters is -1, then there could be a variable
     *number of parameters and no check is made. */
	.cExpectedNumberOfParameters = -1,
};
#endif /* #ifndef NDEBUG */
