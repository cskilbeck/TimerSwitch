//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// button debouncer

template <int history_len> struct button_t
{
    // call this at, say, 1..100 KHz with state of GPIO input in `bit`
    void read(int bit)
    {
        // for masking off old readings
        constexpr uint32 mask = static_cast<uint32>((1llu << history_len) - 1);

        // shift into history, check for a run of ones
        history = ((history << 1) | (bit & 1)) & mask;
        bool on = (history & (history + 1)) == 0;

        // check for a run of zeros
        int  h = ~history & mask;
        bool off = (h & (h + 1)) == 0;

        // set button state
        down = (down || on) && !off;
    }

    // call this in main loop
    void update()
    {
        bool held = down;
        bool change = held != previous;
        previous = held;
        pressed = change && held;
        released = change && !held;
    }

    bool previous;
    bool pressed;
    bool released;

private:
    static_assert(history_len <= 32 && history_len >= 2, "Must use 2..32 for history len");

    // current status of button
    volatile bool down;

    // track last 32 states
    uint32 history{ 0 };
};
