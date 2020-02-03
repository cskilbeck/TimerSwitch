//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct button_t
{
    button_t(GPIO_TypeDef *_port, int _bit_num);

    // call this at, say, 10KHz
    void update();

    // where to get button state from
    GPIO_TypeDef *port;
    int           bit_num;

    // track last 32 states
    uint32 history{ 0 };

    // tracking admin
    bool held{ false };
    bool prev{ false };
    bool pressed{ false };
    bool released{ false };
};
