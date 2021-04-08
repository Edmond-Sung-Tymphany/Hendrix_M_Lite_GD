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

static portBASE_TYPE allplayWifi_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    if( apctx != NULL )
    {
#define SSID_MAX_LENGTH 30
#define PW_MAX_LENGTH 30
        unsigned wifiLength;
        unsigned pwLength;
        char ssid[SSID_MAX_LENGTH] = {0};
        char pw[PW_MAX_LENGTH] = {0};

        const char* pSsid = (const char *)CLIGetParameter( pcCommandString, 1, &wifiLength );
        const char* pPw = (const char *)CLIGetParameter( pcCommandString, 2, &pwLength );
        printf("\n"); //give it a newline after user inputs a cmd.
        if(wifiLength < sizeof(ssid) && pwLength < sizeof(pw))
        {
            /* fill ssid */
            strncpy(ssid, pSsid, wifiLength);
            ssid[sizeof(ssid)-1] = 0;    //make sure there is null termination char.
            /* fill pw */
            strncpy(pw, pPw, pwLength);
            pw[sizeof(pw)-1] = 0;    //make sure there is null termination char.

            printf("ssid:%s,pw:%s\n", ssid, pw);
            allplay_connect_wifi(apctx, (void*)0, ssid, pw);
        }
        else
        {
            printf("SSID or pw too long\n");
        }
    }

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_allplayWifi_cmd =
{
	.pcCommand = (int8_t *)"allplayWifiConnect",
	.pcHelpString = (int8_t *)"allplayWifiConnect {ssid} {pw}\r\n",
	.pxCommandInterpreter = allplayWifi_cmd_callback,
	.cExpectedNumberOfParameters = 2,
};
#endif
