//////////////////////////////////////////////////////////////////////

#include "main.h"

//////////////////////////////////////////////////////////////////////

#define valid_state_mask 0x6996

static int state = 0;
uint8_t store = 0;

//////////////////////////////////////////////////////////////////////

int rotary_update()
{
    uint32_t inputs = ~GPIOA->IDR & 0x3;
    state = ((state << 2) | inputs) & 0xf;
    if(((valid_state_mask >> state) & 1) != 0) {
        store = (store << 4) | state;
        switch(store) {
        case 0xe8:
            return 1;
        case 0x2b:
            return -1;
        }
    }
    return 0;
}
