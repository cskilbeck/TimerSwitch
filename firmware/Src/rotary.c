#include "main.h"

// int const rot_enc_table[16] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0 };

// int state = 0;

// int rotary_update()
// {
//     state = ((state << 2) | (~GPIOA->IDR & 0x3)) & 0xf;
//     return rot_enc_table[state];
// }

static int const rot_enc_table[] = { 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 };
static int state = 0;
static uint8_t store = 0;

int rotary_update()
{
    state = ((state << 2) | (~GPIOA->IDR & 0x3)) & 0xf;
    if(rot_enc_table[state] != 0) {
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
