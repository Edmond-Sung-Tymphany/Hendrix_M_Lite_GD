#ifndef IR_LEARNING_DRV_H
#define IR_LEARNING_DRV_H

#ifdef __cplusplus
extern "C" {
#endif
#include "cplus.h"
#include "deviceTypes.h"
#include "KeyDrv.h"

#define TOTAL_DIFF	        5
#define NUMBER_OF_BIT		50 	//60





#define NUM_OF_IR_LEARNING_KEY  3       /* This number need to match with no of items in KeyMapTable */




typedef enum IRDECODESTATE
{
    BUTTON_PRESS = 0,
    WAIT_FOR_TIME_OUT,
    IGNORE_REPEATER,
    REPEATED_COMMAND,
    WAIT_FOR_BUTTON_RELEASE,
    WAIT_FOR_REPEATER,
    BUTTON_RELEASE,
    BUTTON_IDLE
} IRDECODESTATE;

typedef enum
{
    IR_FORMAT_NG,		    // ir format 0
    IR_FORMAT_NEC,		    // ir format 1
    IR_FORMAT_SONY,		    // ir format 2
    IR_FORMAT_SHARP,	    // ir format 3
    IR_FORMAT_RC5,		    // ir format 4
    IR_FORMAT_RC6,		    // ir format 5
    IR_FORMAT_RCA,		    // ir format 6
    IR_FORMAT_9184,		    // ir format 7
    IR_FORMAT_XSAT,		    // ir format 8
    IR_FORMAT_OTHER,	    // ir format 9
    IR_FORMAT_HEAD,		    // ir format 10
    IR_FORMAT_NO_HEAD,		// ir format 11
    IR_FORMAT_DELAY,		// ir format 12
    IR_FORMAT_SAME_WIDTH,	// ir format 13			// e.g. Toshiba
    IR_FORMAT_SAMSUNG,      // ir format 14

    IR_FORMAT_MANCHESTER,   // ir format 15

    END_IR_FORMAT,
    IR_FORMAT_INVALID
}IR_FORMAT;



typedef struct
{
	uint16 data[TOTAL_DIFF];
	uint8 counter[TOTAL_DIFF];
	IR_FORMAT format:8;
	uint32 code;
}IR_TIME_MAP;	


typedef struct
{
	uint16 low;
	uint16 high;
}IR_BIT_WIDTH;

// IR_MAP can be exactly the same structure as IR_TIME_MAP if "store ir timing" necessary
typedef struct
{
	uint32 code;
	IR_FORMAT format:8;
}IR_MAP;

typedef struct
{
    bool                IR_can_decode:1;
    bool                bValidIr:1;
    uint8                repHeader;
    uint8               gbBuff_IR_Len;
    uint8               gbBuff_Prev_IR_Len;
    uint8               decodeLen;
    uint32              high_T;         // Pulse Width of High Pulse
    uint32              low_T;          // Pulse Width of Low Pulse
    IRDECODESTATE       IRDecodeState:8;
    IR_TIME_MAP         ir_keytime;
    IR_TIME_MAP         previous_ir_keytime;
    IR_BIT_WIDTH        IR_new_buff[NUMBER_OF_BIT];
    IR_BIT_WIDTH        IR_prev_buff[NUMBER_OF_BIT];
    IR_BIT_WIDTH        IR_decode_buff[NUMBER_OF_BIT];
    IR_MAP              keymap_ram[NUM_OF_IR_LEARNING_KEY][2];
    
    eKeyID              COMMAND;
}cIrLearningDrv;

void IrLearningDrv_Ctor(cIrLearningDrv* me);
void IrLearningDrv_Xtor(cIrLearningDrv* me);
void CopyIRTimeMap(IR_TIME_MAP* dest, IR_TIME_MAP* src);
bool CompareIRTimeMap(IR_TIME_MAP* src1, IR_TIME_MAP* src2);
void CopyIRMap(IR_MAP* dest, IR_TIME_MAP* src);
void IrLearningDrv_getFormatCode(cIrLearningDrv* me);
void CheckIRPress(void);

#ifdef __cplusplus
}
#endif


#endif  //IR_LEARNING_DRV_H
