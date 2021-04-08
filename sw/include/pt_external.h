#ifndef PT_EXTERNAL_H
#define PT_EXTERNAL_H

/* Production test (via byte stream) external definitions.
 *
 * Include this file from anywhere (ie PC side communications) to get general message definitions
 * Keep this file up to date with new commands, responses and indications.
 *
 * Additionally include the product specific command data ex: pt_polk_allplay_externl.h
 *
 * Generally the communication is divided up into messages (the whole thing) which carry a
 * payload (a command, response or indication and then some data specific to the command, response or indication)
 * ending with a crc (crc16).
 *
 * The message is framed with a 'start of message' consisting of "pt " and ends with a carriage return "CR"
 * which allows us to pass production test messages in plain text via our debug command line interface
 * bytes are transmitted in hex (upper or lower case) for example 0x0A is transmitted as 0a or 0A
 * 
 * ex: pt 01 ... crc_msb crc_lsbCR
 *
 * All commands (CMD) return a response (RESP OK or RESP ERROR), indications (IND) are sent out asynchronously (at any time).
 *
 * 1. Message structure
 *
 * | SEQ | TYPE | ...DATA.. | CRC16_MSB | CRC16_LSB |
 *
 * SEQ: (1 byte), sequence number, starts at 0 and gets incremented with each message, rolls over
 *
 * TYPE: Command, response or indication
 *
 * DATA: (multibyte) specific command and parameters, response and parameters or indication and parameters
 *
 * CRC16: (2 bytes) uses CRC-CCITT (used in Xmodem, Bluetooth etc.)
 *                  http://en.wikipedia.org/wiki/Cyclic_redundancy_check
 *                  Check your CRC here:
 *                  http://depa.usst.edu.cn/chenjq/www2/SDesign/JavaScript/CRCcalculation.htm
 *                  ex: CRC(0x01,0x02,0x03) -> 0x6131
 *
 * 2. Example commands / responses / indications
 *
 * PC                                              MCU
 *  |                                               |
 *  |   SEQ:0 CMD PROTOCOL VERSION                  |
 *  +----------------------------------------------->
 *  |                                               |
 *  |   SEQ:1 RESP PROTOCOL VERSION OK              |
 *  <-----------------------------------------------+
 *  |                                               |
 *  |   SEQ:2 CMD MMI POWER BUTTON DOWN             |
 *  +----------------------------------------------->
 *  |                                               |
 *  |   SEQ:3 RESP MMI POWER BUTTON DOWN OK         |
 *  <-----------------------------------------------+
 *  |                                               |
 *  |   SEQ:4 CMD MMI POWER BUTTON UP               |
 *  +----------------------------------------------->
 *  |                                               |
 *  |   SEQ:5 RESP MMI POWER BUTTON UP OK           |
 *  <-----------------------------------------------+
 *  |                                               |
 *  |   ~ some time later... ~                      |
 *  |                                               |
 *  |   SEQ:6 IND POWERED ON (result of MMI CMD)    |
 *  <-----------------------------------------------+
 */

/*
 * Protocol version, 1.0 should be the first release
 * Major field - update this when the protocol structure changes
 * Minor field - update this with small additions, structure should be backwards compatible
 */
#define PT_PROTOCOL_VERSION_MAJOR   0
#define PT_PROTOCOL_VERSION_MINOR   1

/* This is the maximum number of bytes to send/receive in the pt packet ie: PT 01 02 03 ... 32*/
#define PT_PACKET_MAX_BYTES 32

/* These define the type of packets: commands, responses and indications */
#define PT_TYPE_COMMAND         0x01
#define PT_TYPE_RESPONSE_OK     0x02
#define PT_TYPE_RESPONSE_ERROR  0x03
#define PT_TYPE_INDICATION      0x04

/* Error codes
 *
 */
#define PT_RESPONSE_ERROR_COMMAND_SPECIFIC   0x01 /* follow with command specific data */
#define PT_RESPONSE_ERROR_CRC                0x02
#define PT_RESPONSE_ERROR_UNEXPECTED_MESSAGE 0x03 /* We received a response or an indication (MCU only handles commands) or an unknown command or an unknown parameter */

/* Command: Get currently implemented protocol version
 * Command data: None
 * Response data: 2 bytes - protocol version major | minor
 * Example:
 *
 *  Protocol version command:
 *  SEQ  | TYPE  | CMD  | CRC16_MSB | CRC16_LSB |
 *  0x00 | 0x01  | 0x01 | 0x23      | 0x10      |
 *
 *  Protocol version response:
 *  SEQ  | TYPE    | RESP |           ..DATA..            | CRC16_MSB | CRC16_LSB |
 *       | RESP OK | CMD  | VERSION MAJOR | VERSION MINOR |           |           |
 *  0x01 | 0x02    | 0x01 | 0x00          | 0x01          | 0x60      | 0x28      |
 *
 */
#define PT_COMMAND_PROTOCOL_VERSION 0x01

/* Command: Read adc (raw value)
 * Command data: 1 byte, product specific button id
 * Response data: 2 bytes: adc msb | adc lsb
 * Example:
 *
 * Read adc channel button_1 command:
 * SEQ  | TYPE | CMD            | DATA                       | CRC_MSB | CRC_LSB |
 *      |      | PT_COMMAND_ADC | PT_COMMAND_ADC_BUTTON_PLAY |         |         |
 * 0x02 | 0x01 | 0x02           | 0x06                       | 0xDC    | 0xFC    |
 *
 * Read adc channel button_1 response:
 * SEQ  | TYPE    | RESP   |      ..DATA..      | CRC_MSB | CRC_LSB |
 *      | RESP OK | CMD    |  ADC_MSB | ADC_LSB |         |         |
 * 0x03 | 0x02    | 0x02   |  0x02    | 0x50    | 0x51    | 0x4D    |
 */
#define PT_COMMAND_ADC 0x02
/* ADC channels are product specific and will found in the pt_product_external.h
 * ex: PT_COMMAND_ADC_BUTTON_PLAY 0x06
 */

/* Command: Action MMI (press a button, illuminated an led etc.)
 * Command data: up to 3 bytes, product specific instuctions
 * Response data: 0 bytes
 * Example: Send button power down
 *
 * Read adc channel button_1 command:
 * SEQ  | TYPE | CMD            |                                 DATA                                             | CRC_MSB | CRC_LSB |
 *      | CMD  | PT_COMMAND_MMI | PT_COMMAND_MMI_BUTTON | PT_COMMAND_MMI_BUTTON_POWER | PT_COMMAND_MMI_BUTTON_DOWN |         |         |
 * 0x04 | 0x01 | 0x03           | 0x01                  | 0x01                        | 0x02                       | 0x13    | 0x6F    |
 *
 * MMI response:
 * SEQ  | TYPE    | RESP | CRC_MSB | CRC_LSB |
 *      | RESP OK | CMD  |         |         |
 * 0x05 | 0x02    | 0x03 | 0xBD    | 0xF1    |
 */
#define PT_COMMAND_MMI 0x03
/* MMI instructions are product specific and will found in the pt_product_external.h
 * ex: PT_COMMAND_MMI_BUTTON 0x01
 *     PT_COMMAND_MMI_LED    0x02
 *     ...
 *     PT_COMMAND_MMI_BUTTON_POWER 0x01
 *     PT_COMMAND_MMI_BUTTON_NEXT  0x02
 *     ...
 *     PT_COMMAND_MMI_BUTTON_UP    0x01
 *     PT_COMMAND_MMU_BUTTON_DOWN  0x02
 */

#define PT_COMMAND_ALLPLAY_PLAYER_STATE 0x04
/* Indication of allplay player state
 */
/* Command: player state indication
 * Command data: 0 bytes
 * Response data: 1 byte
 * Example:
 *
 * SEQ  | TYPE | CMD                             | DATA                                    | CRC_MSB | CRC_LSB |
 *      | IND  | PT_COMMAND_ALLPLAY_PLAYER_STATE | PT_COMMAND_ALLPLAY_PLAYER_STATE_PLAYING |         |         |
 * 0x06 | 0x04 | 0x04                            | 0x05                                    | 0x67    | 0x38    |
 */

#define PT_COMMAND_SET_MODE 0x05
/* change system mode (aux, allplay, direct, wifi setup)
 */
/* Command: change mode
 * Command data: 1 byte
 * Response data: 0 bytes
 * Example:
 *
 * SEQ  | TYPE | CMD                 | DATA                                | CRC_MSB | CRC_LSB |
 *      | CMD  | PT_COMMAND_SET_MODE | PT_COMMAND_ALLPLAY_AUDIOSOURCE_MODE |         |         |
 * 0x07 | 0x01 | 0x05                | 0x02                                | 0xB9    | 0xAA    |
 * response:
 * SEQ  | TYPE    | RESP | CRC_MSB | CRC_LSB |
 *      | RESP OK | CMD  |         |         |
 * 0x08 | 0x02    | 0x05 | 0x9F    | 0x66    |
 */

#define PT_COMMAND_DSP 0x06
/* change dsp settings (dsp volume up, dsp volume down)
 */
/*
 * command: change dsp setting
 * command data: 1 byte
 * response data: 0 bytes
 * Example:
 * SEQ  | TYPE | CMD                 | DATA                                | CRC_MSB | CRC_LSB |
 *      | CMD  | PT_COMMAND_DSP      | PT_COMMAND_DSP_VOL_UP               |         |         |
 * 0x09 | 0x01 | 0x06                | 0x01                                |  0x7E   | 0xC0    |
 * response:
 * SEQ  | TYPE    | RESP | CRC_MSB | CRC_LSB |
 *      | RESP OK | CMD  |         |         |
 * 0x0A | 0x02    | 0x06 | 0xC1    | 0x65    |
 *
 */



/* Fill in the rest of the commands/responses here... */

#endif /* PT_EXTERNAL_H */