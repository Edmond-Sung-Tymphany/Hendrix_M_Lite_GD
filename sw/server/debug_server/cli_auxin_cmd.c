#include <string.h>
#include <stdint.h>
//#include "TLV320AC3254.h"
#include "commonTypes.h"
#include "cli.h"
#include "cli_auxin_cmd.h"
#include "assert.h"

#include "DebugSrv.h"
#include "controller.h"

#ifndef NDEBUG

/* auxin input information */
static portBASE_TYPE auxin_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    QEvt* aux_evt = Q_NEW(QEvt, CLI_AUX_REQ_SIG);
    SendToServer(DEBUG_SRV_ID, aux_evt);
    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_auxin_cmd =
{
	.pcCommand = (int8_t *)"aux_det",
	.pcHelpString = (int8_t *)"aux_det - return the status of auxin input: plugged in or unplugged\r\n",
	.pxCommandInterpreter = auxin_cmd_callback,
	.cExpectedNumberOfParameters = 0,
};
#endif

