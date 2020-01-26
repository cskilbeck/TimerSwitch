#include "main.h"

int const rot_enc_table[16] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0 };

int state = 0;

int rotary_update()
{
    state = ((state << 2) | (~GPIOA->IDR & 0x3)) & 0xf;
    return rot_enc_table[state];
}
