/* Automatically generated nanopb header */
/* Generated by nanopb-0.3.4-dev at Wed Aug 19 17:15:49 2015. */

#ifndef PB_ASE_TK_PB_H_INCLUDED
#define PB_ASE_TK_PB_H_INCLUDED
#include <pb.h>

#if PB_PROTO_HEADER_VERSION != 30
#error Regenerate this file with the current version of nanopb generator.
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Enum definitions */
typedef enum _eType {
    eType_CONFIRMATION = 0,
    eType_NON_CONFIMATION = 1,
    eType_ACK = 2,
    eType_RESET = 3
} eType;

/* Struct definitions */
typedef struct _tHeader {
    int32_t version;
    eType type;
    int32_t message_id;
    pb_callback_t reserve;
} tHeader;

typedef struct _tPayload {
    int32_t command;
    int32_t data;
} tPayload;

typedef struct _tAseTkMessage {
    tHeader header;
    tPayload payload;
} tAseTkMessage;

/* Default values for struct fields */

/* Initializer values for message structs */
#define tHeader_init_default                     {0, (eType)0, 0, {{NULL}, NULL}}
#define tPayload_init_default                    {0, 0}
#define tAseTkMessage_init_default               {tHeader_init_default, tPayload_init_default}
#define tHeader_init_zero                        {0, (eType)0, 0, {{NULL}, NULL}}
#define tPayload_init_zero                       {0, 0}
#define tAseTkMessage_init_zero                  {tHeader_init_zero, tPayload_init_zero}

/* Field tags (for use in manual encoding/decoding) */
#define tHeader_version_tag                      1
#define tHeader_type_tag                         2
#define tHeader_message_id_tag                   3
#define tHeader_reserve_tag                      4
#define tPayload_command_tag                     1
#define tPayload_data_tag                        2
#define tAseTkMessage_header_tag                 1
#define tAseTkMessage_payload_tag                2

/* Struct field encoding specification for nanopb */
extern const pb_field_t tHeader_fields[5];
extern const pb_field_t tPayload_fields[3];
extern const pb_field_t tAseTkMessage_fields[3];

/* Maximum encoded size of messages (where known) */
#define tPayload_size                            22

/* Message IDs (where set with "msgid" option) */
#ifdef PB_MSGID

#define ASE_TK_MESSAGES \


#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif