#include <string.h>
#include <stdint.h>
#include "commonTypes.h"


#include "controller.h"
#include "cli.h"
#include "cli_gpio_cmd.h"
#include "assert.h"

#include "LedSrv.h"
#include "DebugSrv_priv.h"

#include "attachedDevices.h"

#ifndef NDEBUG

/* gpio command */

static void handle_read( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;

    pszParam = (const char *)CLIGetParameter( pcCommandString, 2, &length );
    if( strncmp( pszParam, "power_key", length ) == 0 )
    {
        if( POWER_KEY_PIN == 1 )
            strncpy((char *)pcWriteBuffer, "\r\npower_key down\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\npower_key up\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "net_reset", length ) == 0 )
    {
        if( NET_RESET_KEY_PIN == 0 )
            strncpy((char *)pcWriteBuffer, "\r\nnet_reset down\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\nnet_reset up\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "direct_mode", length ) == 0 )
    {
        if( DIRECT_MODE_KEY_PIN == 0 )
            strncpy((char *)pcWriteBuffer, "\r\ndirect_mode down\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\ndirect_mode up\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "ch_status", length ) == 0 )
    {
        if( CH_STATUS_PIN == 1 )
            strncpy((char *)pcWriteBuffer, "\r\nch_status hi\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\nch_status lo\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "sam_reset", length ) == 0 )
    {
        if( SAM_RESET_PIN == 1 )
            strncpy((char *)pcWriteBuffer, "\r\nsam_reset hi\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\nsam_reset lo\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "1oe_ctrl", length ) == 0 )
    {
        if( OE_CTRL_PIN1 == 1 )
            strncpy((char *)pcWriteBuffer, "\r\n1oe_ctrl hi\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\n1oe_ctrl lo\r\n", xWriteBufferLen);
    }
    else if( strncmp( pszParam, "2oe_ctrl", length ) == 0 )
    {
        if( OE_CTRL_PIN2 == 1 )
            strncpy((char *)pcWriteBuffer, "\r\n2oe_ctrl hi\r\n", xWriteBufferLen);
        else
            strncpy((char *)pcWriteBuffer, "\r\n2oe_ctrl lo\r\n", xWriteBufferLen);
    }
    else
    {
        strncpy((char *)pcWriteBuffer, "\r\nerror\r\n", xWriteBufferLen);
    }
}

static void handle_write( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString, unsigned output )
{
    const char *pszParam;
    unsigned length;

    pszParam = (const char *)CLIGetParameter( pcCommandString, 2, &length );
    if( strncmp( pszParam, "rgb_led_red", length ) == 0 )
    {
        if (output)
        {
            DebugSrvOutput("\r\ndebug command to turn ON RED led\r\n",0);
            LedSrv_SetEvtOn(NULL,LED_RED,100); // turn on LED 1 with 100% bright

        }
        else
        {
            DebugSrvOutput("\r\ndebug command to turn OFF RED led\r\n",0);
            LedSrv_SetEvtOff(NULL,LED_RED); // turn on LED 1 with 0% bright
        }
    }
    else if( strncmp( pszParam, "rgb_led_green", length ) == 0 )
    {
        if (output)
        {
            DebugSrvOutput("\r\ndebug command to turn ON GREEN led\r\n",0);
            LedSrv_SetEvtOn(NULL,LED_GREEN,100); // turn on LED 1 with 100% bright
        }
        else
        {
            DebugSrvOutput("\r\ndebug command to turn OFF GREEN led\r\n",0);
            LedSrv_SetEvtOff(NULL,LED_GREEN); // turn on LED 1 with 0% bright
        }
    }
    else if( strncmp( pszParam, "rgb_led_blue", length ) == 0 )
    {
        if (output)
        {
            DebugSrvOutput("\r\ndebug command to turn ON BLUE led\r\n",0);
            LedSrv_SetEvtOn(NULL,LED_BLUE,100); // turn on LED 1 with 100% bright
        }
        else
        {
            DebugSrvOutput("\r\ndebug command to turn OFF BLUE led\r\n",0);
            LedSrv_SetEvtOff(NULL,LED_BLUE); // turn on LED 1 with 0% bright
        }
    }
    else if( strncmp( pszParam, "mute", length ) == 0 )
    {
        output ? (AMP_MUTE_PIN=1) : (AMP_MUTE_PIN=0);
    }
    else if( strncmp( pszParam, "1oe_ctrl", length ) == 0 )
    {
        output ? (OE_CTRL_PIN1=1) : (OE_CTRL_PIN1=0);
    }
    else if( strncmp( pszParam, "2oe_ctrl", length ) == 0 )
    {
        output ? (OE_CTRL_PIN2=1) : (OE_CTRL_PIN2=0);
    }
    else if( strncmp( pszParam, "sam_reset", length ) == 0 )
    {
        output ? (SAM_RESET_PIN_LAT=1) : (SAM_RESET_PIN_LAT=0);
    }
    else
    {
        strncpy((char *)pcWriteBuffer, "\r\nerror\r\n", xWriteBufferLen);
    }
}

static portBASE_TYPE gpio_cmd_callback ( int8_t *pcWriteBuffer, size_t xWriteBufferLen, const int8_t * pcCommandString )
{
    const char *pszParam;
    unsigned length;

    assert( xWriteBufferLen > 32 );

    /* read/set hi/set lo */
    pszParam = (const char *)CLIGetParameter( pcCommandString, 1, &length );
    if( strncmp( pszParam, "read", length ) == 0 )
        handle_read(pcWriteBuffer, xWriteBufferLen, pcCommandString);
    else if( strncmp( pszParam, "hi", length ) == 0 )
        handle_write(pcWriteBuffer, xWriteBufferLen, pcCommandString, 1);
    else if( strncmp( pszParam, "lo", length ) == 0 )
        handle_write(pcWriteBuffer, xWriteBufferLen, pcCommandString, 0);
    else
        strncpy((char *)pcWriteBuffer, "\r\nerror\r\n", xWriteBufferLen);


    return pdFALSE; /* FALSE means done */
}

CLI_Command_Definition_t cli_gpio_cmd =
{
	.pcCommand = (int8_t *)"gpio",
	.pcHelpString = (int8_t *)"gpio {read | hi | lo } { power_key | net_reset | "
	               "direct_mode | ch_status | sam_reset | rgb_led_blue | "
	               "rgb_led_green | rgb_led_red | mute | 1oe_ctrl | 2oe_ctrl}\r\n",
	.pxCommandInterpreter = gpio_cmd_callback,
	.cExpectedNumberOfParameters = 2,
};
#endif
