#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_version_cmd.h"
#include "assert.h"

#include "deviceTypes.h"



#ifndef NDEBUG

/* software version information */

static portBASE_TYPE version_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{

    snprintf((char *)pcWriteBuffer, xWriteBufferLen, "\r\n%s\r\n", getVersionString());
    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_version_cmd =
{
	.pcCommand = (int8_t *)"version",
	.pcHelpString = (int8_t *)"version\r\n",
	.pxCommandInterpreter = version_cmd_callback,
	.cExpectedNumberOfParameters = 0,
};
#endif