//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// the actual button, call update() in ~10KHz timer ISR

template <uintptr gpio_port, int bit_num, int history_len> struct button_t
{
    // call this at, say, 1KHz
    void read()
    {
        // for masking off old ones readings
        constexpr uint32 mask = static_cast<uint32>((1llu << history_len) - 1);

        // can't use a ptr as a template arg so cast
        auto port = reinterpret_cast<volatile GPIO_TypeDef *>(gpio_port);

        // get gpio state
        int bit = (~port->IDR >> bit_num) & 1;
        
        // shift into history, check for a run of ones
        history = ((history << 1) | bit) & mask;
        bool on = (history & (history + 1)) == 0;

        // alternative history, check for a run of zeros
        int h = ~history & mask;
        bool off = (h & (h + 1)) == 0;

        // update button state to 0 or 1
        down = int((down || on) && !off);
    }

    // call this in main loop
    void update()
    {
        held = down != 0;
        bool change = held != previous;
        previous = held;
        pressed = change && held;
        released = change && !held;
    }

    bool held;
    bool previous;
    bool pressed;
    bool released;

private:
    
    static_assert(history_len <= 32 && history_len >= 2, "Must use 2..32 for history");

    // track last 32 states
    uint32 history{ 0 };

    // current status of button, 0 or 1
    volatile int down;
};
