#include <string.h>
#include <stdint.h>
#include "commonTypes.h"
#include "assert.h"
#include "crc16.h"
#include "pt.h"

#include "DebugSrv.h"
#include "KeySrv.h"
#include "PowerSrv.h"
#include "KeySrv.h"
#include "AudioSrv.h"
#include "deviceTypes.h"
#include "LedSrv.h"
#include "controller.h"

/* seq value of last command */
static unsigned char current_seq;

/* mapping arrays */
/* key id map array; idx - pt define; value - eKeyID value */
static eKeyID dbg_pt_adc_map[] = {
[PT_COMMAND_MMI_BUTTON_POWER]  =   POWER_KEY,
[PT_COMMAND_MMI_BUTTON_VOL_DOWN] = VOLUME_DOWN_KEY,
[PT_COMMAND_MMI_BUTTON_VOL_UP] = VOLUME_UP_KEY,
[PT_COMMAND_MMI_BUTTON_PREV] = PREV_KEY,
[PT_COMMAND_MMI_BUTTON_NEXT] = NEXT_KEY,
[PT_COMMAND_MMI_BUTTON_PLAY] = PLAY_PAUSE_KEY,
[PT_COMMAND_MMI_BUTTON_SETUP] = NET_RESET_KEY,
[PT_COMMAND_MMI_BUTTON_DIRECT] = DIRECT_MODE_KEY,

[PT_COMMAND_ADC_BATTERY]  = BATTERY_VALUE,
[PT_COMMAND_ADC_AC] = AC_VALUE,
};

/* key event map array; idx - pt define; value eKeyEvent */
static eKeyEvent dbg_pt_keyEvent_map[KEY_EVT_MAX] = {
    [PT_COMMAND_MMI_BUTTON_UP] = KEY_EVT_UP,
    [PT_COMMAND_MMI_BUTTON_DOWN] = KEY_EVT_DOWN,
};

static eAudioChannel dbg_pt_audioSrc_map[AUDIO_CHANNEL_MAX + 1] = {
    [PT_COMMAND_AUX_AUDIOSOURCE_MODE] = AUDIO_CHANNEL_1, 
    [PT_COMMAND_ALLPLAY_AUDIOSOURCE_MODE] = AUDIO_CHANNEL_0,
};

static eAudioVolumeUpDownDirection dbg_dsp_sets_map[END_AUDIO_VOLUME_UPDOWN + 1] = {
    [PT_COMMAND_DSP_VOL_UP] = AUDIO_VOLUME_UP,
    [PT_COMMAND_DSP_VOL_DOWN] = AUDIO_VOLUME_DOWN,
};
/*____________________________________________________________________________*/

static bool check_crc( const unsigned char *byte_stream, unsigned length )
{
    unsigned short calc_crc = crc16( byte_stream, length-2 );
    unsigned short rec_crc = (((unsigned short)byte_stream[length-2])<<8) | byte_stream[length-1] ;

    if( rec_crc==calc_crc )
    {
        return TRUE;
    }

    return FALSE;
}

static unsigned char get_message_seq( const unsigned char *byte_stream, unsigned length )
{
    return byte_stream[0];
}

static unsigned char get_message_type( const unsigned char *byte_stream, unsigned length )
{
    return byte_stream[1];
}

static unsigned char get_message_command( const unsigned char *byte_stream, unsigned length )
{
    return byte_stream[2];
}

static unsigned char get_parameter( const unsigned char *byte_stream, unsigned length, unsigned parameter )
{
    return byte_stream[2+parameter-1];
}

static unsigned char get_msb( unsigned short x )
{
    return (x>>8)&0xFF;
}

static unsigned char get_lsb( unsigned short x )
{
    return x&0xFF;
}

static void calc_crc_msb_lsb( const unsigned char *byte_stream, unsigned length, unsigned char *msb, unsigned char *lsb )
{
    unsigned short crc = crc16( byte_stream, length );
    *msb = get_msb(crc);
    *lsb = get_lsb(crc);
}

static void send_message( const unsigned char *byte_stream, unsigned length )
{
    unsigned i;

     for( i=0; i<length; i++ )
    {
        printf("%02x", byte_stream[i]);
        if(i<length-1)
            printf(" ");
    }

    printf("\n");

}

static void send_general_error( const unsigned char *byte_stream, unsigned length, unsigned char error )
{
    /* | SEQ | TYPE | RESP CMD | CRC16_MSB | CRC16_LSB | */
    unsigned char message[5];
    unsigned char crc_msb, crc_lsb;

    message[0] = get_message_seq(byte_stream,length) + 1;
    current_seq = message[0];
    message[1] = error;
    message[2] = get_message_command(byte_stream,length);
    calc_crc_msb_lsb( message, sizeof(message)-2, &crc_msb, &crc_lsb );
    message[3] = crc_msb;
    message[4] = crc_lsb;

    send_message( message, sizeof(message) );
}

static void send_protocol_version( unsigned char seq )
{
    /* | SEQ | TYPE | RESP CMD | VERSION MAJOR | VERSION MINOR | CRC16_MSB | CRC16_LSB | */
    unsigned char message[7];
    unsigned char crc_msb, crc_lsb;
    
    message[0] = seq+1;
    message[1] = PT_TYPE_RESPONSE_OK;
    message[2] = PT_COMMAND_PROTOCOL_VERSION;
    message[3] = PT_PROTOCOL_VERSION_MAJOR;
    message[4] = PT_PROTOCOL_VERSION_MINOR;
    calc_crc_msb_lsb( message, sizeof(message)-2, &crc_msb, &crc_lsb );
    message[5] = crc_msb;
    message[6] = crc_lsb;

    send_message( message, sizeof(message) );
    current_seq = seq + 1;
}




static void handle_command_adc( unsigned char* p_seq, unsigned char* p_parameter )
{
    if ( *p_parameter < PT_COMMAND_ADC_BUTTON_MAX )
    {
        /* send buttons to the debug server */
        KeyDebugReqEvt* pt_adc_req = Q_NEW(KeyDebugReqEvt, CLI_ADC_MMI_REQ_SIG);
        pt_adc_req->keyId = dbg_pt_adc_map[*p_parameter];
        pt_adc_req->req = DEBUG_RAW_DATA_REQ;
        SendToServer(DEBUG_SRV_ID, (QEvt*) pt_adc_req);
    }
    else if ( *p_parameter < PT_COMMAND_ADC_MAX )
    {
        /* send raw adc to the power server */
        PowerSrvDebugReqEvt* pt_adc_req = Q_NEW(PowerSrvDebugReqEvt, POWER_DEBUG_REQ_SIG);
        pt_adc_req->valueType = dbg_pt_adc_map[*p_parameter];
        SendToServer(POWER_SRV_ID, (QEvt*)pt_adc_req);
    }
    else
    { /* parameter is invalid, send error resp */
        pt_handle_adc_resp(*p_seq, FALSE, 0);
    }
    current_seq = *p_seq;
}

#ifdef AUDIO_MULTI_SOURCE
static void handle_command_setmode( unsigned char* p_seq, unsigned char* p_parameter )
{
    if (*p_parameter < AUDIO_CHANNEL_MAX+1)
    {
        AudioChannelSwitchReqEvt* audio_switch = Q_NEW(AudioChannelSwitchReqEvt, AUDIO_SWITCH_CHANNEL_SIG);
        audio_switch->channel = dbg_pt_audioSrc_map[*p_parameter];
        SendToServer(AUDIO_SRV_ID, (QEvt*)audio_switch);
        pt_handle_cmd_resp(*p_seq, PT_COMMAND_SET_MODE, TRUE);
        return ;
    }

    pt_handle_cmd_resp(*p_seq, PT_COMMAND_SET_MODE, FALSE);
    
}
#endif
static void handle_command_dsp( unsigned char* p_seq, unsigned char* p_parameter )
{
    if (*p_parameter < END_AUDIO_VOLUME_UPDOWN)
    {
        switch(dbg_dsp_sets_map[*p_parameter])
        {
            case AUDIO_VOLUME_DOWN:
            {
               KeySrv_SendKeyEvt(KEY_EVT_SHORT_PRESS,VOLUME_DOWN_KEY);
            }
            break;
            case AUDIO_VOLUME_UP:
            {
                KeySrv_SendKeyEvt(KEY_EVT_SHORT_PRESS,VOLUME_UP_KEY);
            }
            break;
            default:break;
        }
        pt_handle_cmd_resp(*p_seq, PT_COMMAND_DSP, TRUE);
        return;
    }
    pt_handle_cmd_resp(*p_seq, PT_COMMAND_DSP, FALSE);
}

static bool handle_command_mmi_button(unsigned char* p_buttonId, unsigned char* p_keyEvent)
{
    if ((*p_keyEvent >= KEY_EVT_MAX) || (*p_buttonId >= COMB_KEY_ID_MAX))
    {
        return FALSE;
    }
    KeyDebugReqEvt *sneaky_key_event = Q_NEW(KeyDebugReqEvt, CLI_ADC_MMI_REQ_SIG);
    sneaky_key_event->req = DEBUG_KEY_EVT_SIMULATION;
    sneaky_key_event->keyEvent = dbg_pt_keyEvent_map[*p_keyEvent];
    sneaky_key_event->keyId = dbg_pt_adc_map[*p_buttonId];
    SendToServer(DEBUG_SRV_ID, (QEvt*)sneaky_key_event);

    return TRUE;
}
static bool handle_command_mmi_led(unsigned char* p_ledId, unsigned char* p_ledEvent)
{
    switch(*p_ledId)
    {
    case PT_COMMAND_MMI_LED_RED:
    {
#ifdef HAS_LEDS        
        if (PT_COMMAND_MMI_LED_ON == *p_ledEvent)
        {
            //LedSrv_SetEvtOn(NULL,LED_RED,100);
        }
        else
        {
            //LedSrv_SetEvtOff(NULL,LED_RED);
        }
#endif        
        break;
    }
    case PT_COMMAND_MMI_LED_GREEN:
    {
#ifdef HAS_LEDS         
        if (PT_COMMAND_MMI_LED_ON == *p_ledEvent)
        {
            //LedSrv_SetEvtOn(NULL,LED_GREEN,100);
        }
        else
        {
            //LedSrv_SetEvtOff(NULL,LED_GREEN);
        }
#endif        
        break;
    }
    case PT_COMMAND_MMI_LED_BLUE:
    {
#ifdef HAS_LEDS         
        if (PT_COMMAND_MMI_LED_ON == *p_ledEvent)
        {
            //LedSrv_SetEvtOn(NULL,LED_BLUE,100);
        }
        else
        {
            //LedSrv_SetEvtOff(NULL,LED_BLUE);
        }
#endif        
        break;
    }
    default:
        return FALSE;
    }

    return TRUE;
}

static void handle_command_mmi(unsigned char seq, unsigned char parameter0, unsigned char parameter1, unsigned char parameter2)
{
    bool resp;
    switch (parameter0)
    {
    case PT_COMMAND_MMI_BUTTON:
        resp = handle_command_mmi_button(&parameter1, &parameter2);
        break;
    case PT_COMMAND_MMI_LED:
        resp = handle_command_mmi_led(&parameter1, &parameter2);
        break;
    default:
        resp = FALSE;
        break;
    }
    if (!resp)
    {
        pt_handle_cmd_resp(seq, PT_COMMAND_MMI, FALSE);
    }
    current_seq = seq;
}

static void handle_command( const unsigned char *byte_stream, unsigned length )
{
    unsigned char command = get_message_command( byte_stream, length );

    switch( command )
    {
        case PT_COMMAND_PROTOCOL_VERSION:
            send_protocol_version( get_message_seq(byte_stream,length) );
            break;
        case PT_COMMAND_ADC:
        {
            unsigned char parameter0 = get_message_seq(byte_stream,length);
            unsigned char parameter1 = get_parameter( byte_stream,length,2 );
            handle_command_adc( &parameter0, &parameter1 );
            break;
        }
        case PT_COMMAND_MMI:
            handle_command_mmi(get_message_seq(byte_stream,length), get_parameter( byte_stream,length,2 ),
                               get_parameter( byte_stream,length,3 ), get_parameter( byte_stream,length,4 ) );
            break;
#ifdef AUDIO_MULTI_SOURCE
        case PT_COMMAND_SET_MODE:
        {
            unsigned char parameter0 = get_message_seq(byte_stream, length);
            unsigned char parameter1 = get_parameter(byte_stream, length, 2);
            handle_command_setmode(& parameter0, &parameter1);
            break;
        }
#endif
        case PT_COMMAND_DSP:
        {
            unsigned char parameter0 = get_message_seq(byte_stream, length);
            unsigned char parameter1 = get_parameter(byte_stream, length, 2);
            handle_command_dsp( &parameter0, &parameter1 );
            break;
        }
        default:
            send_general_error( byte_stream, length, PT_RESPONSE_ERROR_UNEXPECTED_MESSAGE );
            break;
    }
}

/*
 * Public
 * 
 */
void pt_handle_packet( const unsigned char *byte_stream, unsigned length )
{
    assert( byte_stream );

    if ( byte_stream && (length >= 5) )
    {
        if( check_crc( byte_stream, length ) )
        {
            unsigned char type = get_message_type( byte_stream, length );
            if( type == PT_TYPE_COMMAND )
            {
                handle_command( byte_stream, length );
            }
            else
            {
                send_general_error( byte_stream, length, PT_RESPONSE_ERROR_UNEXPECTED_MESSAGE );
            }
        }
        else
        {
            send_general_error( byte_stream, length, PT_RESPONSE_ERROR_CRC );
        }
    }
    else
    {
        send_general_error( byte_stream, length, PT_RESPONSE_ERROR_UNEXPECTED_MESSAGE );
    }
}

unsigned char get_current_seq()
{
    return current_seq;
}

void pt_handle_indication_1b(unsigned char seq, unsigned char cmd, unsigned char data)
{
    /* SEQ  | TYPE | CMD | DATA | CRC_MSB | CRC_LSB | */
    unsigned char message[6];
    unsigned char crc_msb, crc_lsb;

    message[0] = seq + 1;
    message[1] = PT_TYPE_INDICATION;
    message[2] = cmd;
    message[3] = data;
    calc_crc_msb_lsb(message, sizeof (message) - 2, &crc_msb, &crc_lsb);
    message[4] = crc_msb;
    message[5] = crc_lsb;

    send_message(message, sizeof (message));
    current_seq = seq + 1;

}

void pt_handle_adc_resp(unsigned char seq, bool bIsSuccess, unsigned short adc_data)
{
    unsigned char message[7];
    unsigned char crc_msb, crc_lsb;

    message[0] = seq+1;
    if (bIsSuccess)
    {
        message[1] = PT_TYPE_RESPONSE_OK;
        message[2] = PT_COMMAND_ADC;
        message[3] = get_msb(adc_data);
        message[4] = get_lsb(adc_data);
    }
    else
    {
        message[1] = PT_TYPE_RESPONSE_ERROR;
        message[2] = PT_COMMAND_ADC;
        message[3] = 0x00;
        message[4] = 0x00;

    }
    calc_crc_msb_lsb( message, sizeof(message)-2, &crc_msb, &crc_lsb );
    message[5] = crc_msb;
    message[6] = crc_lsb;

    send_message( message, sizeof(message) );
    current_seq = seq + 1;
}
void pt_handle_cmd_resp(unsigned char seq, unsigned char cmd, bool bIsSuccess)
{
    unsigned char message[5];
    unsigned char crc_msb, crc_lsb;

    message[0] = seq + 1;
    if (bIsSuccess)
    {
        message[1] = PT_TYPE_RESPONSE_OK;
    }
    else
    {
        message[1] = PT_TYPE_RESPONSE_ERROR;
    }
    message[2] = cmd;
    calc_crc_msb_lsb( message, sizeof(message)-2, &crc_msb, &crc_lsb );
    message[3] = crc_msb;
    message[4] = crc_lsb;

    send_message( message, sizeof(message) );
    current_seq = seq + 1;
}
