#pragma once

// init the max7219
void max7219_init();

// set wakeup to 0 (display off) or 1 (display on)
void max7219_set_wakeup(int x);

// set led intensity (0..15)
void max7219_set_intensity(int x);

// set decimal number (0..9999)
void max7219_set_number(uint x);

// set hex number (0..FFFF)
void max7219_set_hex(uint x);

// apply changes to display
void max7219_update();
