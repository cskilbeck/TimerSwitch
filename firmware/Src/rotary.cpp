//////////////////////////////////////////////////////////////////////

#include "main.h"

//////////////////////////////////////////////////////////////////////

static int state = 0;
uint8_t    store = 0;

//////////////////////////////////////////////////////////////////////

// Valid transitions are:
// 1    00 .. 01
// 2    00 .. 10
// 4    01 .. 00
// 7    01 .. 11
// 8    10 .. 00
// 11   10 .. 11
// 13   11 .. 01
// 14   11 .. 10

// bitmask of which 2-state histories are valid (see table above)
#define valid_state_mask 0x6996

// then, to just get one increment per cycle:

// 11 .. 10 .. 00 is one way
// 00 .. 10 .. 11 is the other way

// So:
// E8 = 11,10 .. 10,00  --> one way
// 2B = 00,10 .. 10,11  <-- other way

int rotary_update()
{
    uint32_t inputs = ~GPIOA->IDR & 0x3;
    state = ((state << 2) | inputs) & 0xf;

    // many states are invalid (noisy switches) so ignore them
    if(((valid_state_mask >> state) & 1) != 0)
    {
        // certain state patterns mean rotation happened
        store = (store << 4) | state;
        switch(store)
        {
        case 0xe8:
            return 1;
        case 0x2b:
            return -1;
        }
    }
    return 0;
}
