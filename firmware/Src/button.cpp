//////////////////////////////////////////////////////////////////////

#include "button.h"

//////////////////////////////////////////////////////////////////////

button_t::button_t(GPIO_TypeDef *_port, int _bit_num) : port(_port), bit_num(_bit_num)
{
}

//////////////////////////////////////////////////////////////////////

void button_t::update()
{
    // bitmap of 32 recent readings
    int bit = ((_port->IDR) >> bit_num) & 1;
    history = (history << 1) | bit;

    // offs followed by an on = a button down event
    // ons followed by an off = a button up event
    bool press = (history & (history + 1)) != 0;
    bool release = (~history & (~history + 1)) != 0;

    // update button held state
    held = (held || press) && !release;

    // track changes (pressed, released)
    bool diff = held != prev;
    prev = held;
    pressed = pressed || (diff && held);
    released = released || (diff && !held);
}
