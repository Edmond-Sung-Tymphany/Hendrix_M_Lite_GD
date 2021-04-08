
#include "configuration.h"

static unsigned char w = HEATSHRINK_STATIC_WINDOW_BITS;
static unsigned char l = HEATSHRINK_STATIC_LOOKAHEAD_BITS;

unsigned char config_get_w()
{
    return w;
}

unsigned char config_get_l()
{
    return l;
}

void config_set_w(unsigned char new_w)
{
    w = new_w;
}

void config_set_l(unsigned char new_l)
{
    l = new_l;
}

