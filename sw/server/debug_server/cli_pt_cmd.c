#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "cli.h"
#include "cli_pt_cmd.h"
//#include "hwsetup.h"
#include "assert.h"
#include "pt.h"

/* production test command */

static portBASE_TYPE pt_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned param_length;
    int parameter=1;
    unsigned char byte_stream[PT_PACKET_MAX_BYTES] = {};

    assert( xWriteBufferLen > 32 );

    //TP_PRINTF("pt in: ");

    while( (pszParam = (const char *)CLIGetParameter( pcCommandString, parameter, &param_length )) != NULL )
    {

        unsigned char byte = parameter-1;

        assert( byte < PT_PACKET_MAX_BYTES );

        if( byte < PT_PACKET_MAX_BYTES )
        {
            byte_stream[byte] = strtoul( pszParam, NULL, 16);
            //TP_PRINTF("0x%02x ", byte_stream[byte]);
            parameter++;
        }
    }

    //TP_PRINTF("\r\n");

    pt_handle_packet( byte_stream, parameter-1 );

    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_pt_cmd =
{
	.pcCommand = (int8_t *)"pt",
#ifndef NDEBUG
	.pcHelpString = (int8_t *)"pt | ID | TYPE | ...DATA.. | CRC16_MSB | CRC16_LSB | - action production test\r\n",
#else
    .pcHelpString = (int8_t *)"",
#endif
	.pxCommandInterpreter = pt_cmd_callback,
	.cExpectedNumberOfParameters = -1,
};
