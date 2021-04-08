#ifndef NEC_IR_DRV_PRIV_H
#define NEC_IR_DRV_PRIV_H

typedef enum
{
    DECODE_ERR = 0,
    DECODE_START,
    DECODE_SUCC,
    DECODE_DONE,
    DECODE_LONG_PRESS
}ir_decode_state_t;

#endif
