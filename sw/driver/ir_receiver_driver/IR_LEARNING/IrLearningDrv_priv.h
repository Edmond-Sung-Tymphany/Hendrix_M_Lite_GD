#ifndef IR_LEARNING_DRV_PRIV
#define IR_LEARNING_DRV_PRIV

#include "commonTypes.h"
#include "deviceTypes.h"
#include "IrRxDrv.h"



#define IR_OFFSET		3
#define time_9000_us	142
#define time_4500_us	71
#define time_1125_us	18
#define time_2250_us	36

#define time_0565_us	9
#define time_1690_us	26

#define REP_IR_OFFSET	2			//5 * 0.5ms = 2.5ms
#define time_40200_us	20 			//80 * 0.5ms = 40ms
#define time_11200_us	5			//22 * 0.5ms = 11ms
#define time_97600_us	49			//193 * 0.5ms = 97.5ms

#define MONITOR_AUDIO_CUSTOMER_CODE 0x11ee

//#define TEST_SONY_IR		1
//#define TEST_NEC_IR		1
//#define PHILIPS_RC5_IR			1
//#define PHILIPS_RC6_IR			1
//#define TEST_9184_IR







typedef enum{
	FALLING_EDGE,
	RISING_EDGE,
	BOTH_EDGE
}EDGE_STATE;



typedef enum{
    IR_IDLE,
    LEADER_ON,
    LEADER_OFF,
    CUSTOM_DATA,
    IR_DATA_1,
    IR_DATA_2,
    IR_REP_LEADER,
    IR_REP_1,
    IR_REP_2,
    IR_REP_LEADER_OFF,
    IR_REP_CUSTOM_DATA
}IR_STATE;


typedef struct
{
    uint16 PulseWidth;		// pulse width
    uint8 Ir2ms5Cnt;		// 2.5 ms count
    uint16 RCBits1;		// store customer code
    uint16 RCBits2;		// store ir data
    uint8 RCBitCnt;		// count no. of bits received
    uint8 RemoteKey;		// current ir code
    uint8 receiveCnt;
    unsigned HeadPulse:1;	// head pulse detected
    unsigned RemoteValid:1;	// valid data
    unsigned BitLogic:1;	// Logic data
    unsigned have_key:1;	// have receiver a remote key
    uint8 necirholdtime;		// ir hold time
    unsigned short int custom_code;
    IR_STATE status;
    IR_STATE status_old;
    uint8 NEC_DATA1;
    uint8 NEC_DATA2;
    uint8 release_countdown;
    unsigned just_released:1;
    uint8 repeat_flag;
    uint16 repeat_count;
    uint8 package_number;
    uint8 wait_next_package;
    uint8 non_repeat_code;
    uint8 broken_tail_cnt;
    uint8 tail_exist;
    unsigned long int TimeGapCnt;
    uint8 TimeGapCnt_Enable;
    EDGE_STATE current_trigger_mode:2;
} IRSTRU;


extern void IR_Decode_Proc(void);
extern void IRreset(void);
extern uint8 Get_ir_key(void);



#define DEFAULT_VENDOR_CODE_946B                // use default remote with vendor code: 0x946B


#define HEADER_WIDTH_RC6		1441
#define HEADER_WIDTH_RC5		716
//#define HEADER_WIDTH_NEC		5443
#define HEADER_WIDTH_NEC		5200		//13.5ms
#define HEADER_WIDTH_SONY		1209
#define HEADER_WIDTH_9184		640
#define HEADER_WIDTH_XSAT		4600		// 12ms
#define HEADER_WIDTH_SHARP		0x300		// 2ms

#define THRESHOLD_RC5			850
#define THRESHOLD_RC6			420
#define THRESHOLD_XSAT			0x240
#define THRESHOLD_SHARP			0x240

#define ReadTimer2()    TIM_GetCounter(TIM2)
#define WriteTimer2(x)   TIM_SetCounter(TIM2, x)


static void IR_decode_universal_code(cIrLearningDrv* me);

#endif
