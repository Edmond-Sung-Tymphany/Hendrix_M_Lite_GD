#ifndef PT_H
#define PT_H

#include "pt_external.h"
#include "pt_polk_allplay_external.h"

typedef enum
{
    AUDIO_VOLUME_INVALID,
    AUDIO_VOLUME_DOWN,
    AUDIO_VOLUME_UP,
    END_AUDIO_VOLUME_UPDOWN
}eAudioVolumeUpDownDirection;

/* Production test (via a byte stream) public interface
 *
 * Include this file (and this file only) to use within your MCU project
 *
 */

/* Config */

/* Public */

/* Actions the packet, returns when finished, true on success, false on error (ie CRC error) */
void pt_handle_packet( const unsigned char *byte_stream, unsigned length );

/* return seq value of last command */
unsigned char get_current_seq();
/* output indication cmd (1 data byte) */
void pt_handle_indication_1b(unsigned char seq, unsigned char cmd, unsigned char data);
/* output adc resp command (2 data bytes) */
void pt_handle_adc_resp(unsigned char seq, bool bIsSuccess, unsigned short adc_data);
/* output resp for mmi command */
void pt_handle_cmd_resp(unsigned char seq, unsigned char cmd, bool bIsSuccess);
#endif /* PT_H */