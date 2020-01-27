#include "main.h"

#define valid_state_mask 0x6996

static int state = 0;
static uint8_t store = 0;

int rotary_update()
{
    state = ((state << 2) | (~GPIOA->IDR & 0x3)) & 0xf;
    if((valid_state_mask & (1 << state)) != 0) {
        store = (store << 4) | state;
        switch(store) {
        case 0x17:
            return 1;
        case 0x2b:
            return -1;
        }
    }
    return 0;
}
