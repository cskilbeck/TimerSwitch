//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////
// button debouncer

struct button_t
{
    inline bool is_run(uint32 x)
    {
        return x != 0 && (x & (x + 1)) == 0;
    }

    // call this at, say, 1..100 KHz with state of GPIO input in `bit` (0 or 1)
    void read(int bit)
    {
        // for masking off old readings
        constexpr uint32 mask = static_cast<uint32>((1llu << history_len) - 1);

        // shift into history, check for a run of ones
        history = ((history << 1) | bit) & mask;
        bool on = is_run(history);

        // check for a run of zeros
        int  h = ~history & mask;
        bool off = is_run(h);

        // set button state
        down = (down || on) && !off;
    }

    // call this in main loop
    void update()
    {
        bool held = down;
        bool change = held != previous;
        pressed = change && held;
        released = change && !held;
        previous = held;
    }

    // current status of button
    volatile bool down;

    // status at last update()
    bool previous;

    // was it pressed since last update()
    bool pressed;

    // was it released since last update()
    bool released;

private:
    static constexpr int history_len = 20;

    static_assert(history_len <= 31 && history_len >= 2, "Must use 2..31 for history len");

    // track last 32 states
    uint32 history{ 0 };
};
