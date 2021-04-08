#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "controller.h"
#include "cli.h"
#include "cli_led_cmd.h"
#include "assert.h"

#include "LedSrv.h"
#include "DebugSrv_priv.h"
#include "attachedDevices.h"

#ifndef NDEBUG
/* led command */
static uint8 get_value(const char * sVal)
{
    uint8 ret;

    ret = atoi(sVal);
    if (ret > MAX_BRT)
        ret = MAX_BRT;
    return ret;
}

static portBASE_TYPE led_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;
    char msg[255];
    uint8 r, g, b;

    assert( xWriteBufferLen > 32 );

    /* read/set hi/set lo */
    pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
    r = get_value(pszParam);
    pszParam = (const char *)CLIGetParameter( (int8_t *)pszParam, 1, &length );
    g = get_value(pszParam);
    pszParam = (const char *)CLIGetParameter( (int8_t *)pszParam, 1, &length );
    b = get_value(pszParam);

    // display the RGB value obtained
    sprintf(msg, "\r\nR:%03d; G:%03d; B:%03d;\r\n", r, g, b);
    strncpy((char *)pcWriteBuffer, msg, xWriteBufferLen);

    LedReqEvt* e = Q_NEW(LedReqEvt, LED_REQ_SIG);
//TODO: Wesley_test    LedSrv_SetRGB(NULL, e, RGBA(r,g,b,0));
    SendToServer(LED_SRV_ID, (QEvt*)e);

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_led_cmd =
{
    .pcCommand = (int8_t *)"rgb",
    .pcHelpString = (int8_t *)"rgb {R G B} the value of R, G, and B should be between 0 and 100 inclusively \r\n",
    .pxCommandInterpreter = led_cmd_callback,
    .cExpectedNumberOfParameters = 3,
};
#endif