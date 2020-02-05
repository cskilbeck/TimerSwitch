#pragma once

// init the max7219, assumes SPI is setup like:
//
// Baud Rate = whatever gets you ~10MHz (not faster)
// Data Size = 16 bit
// CLK Polarity = low
// CLK Phase = 1 edge
// SS = hardware enabled
// First Bit = msb
// TI Mode = disabled
// CRC Calculation = disabled
// NSS Pulse Mode = enabled

// also assumes DMA is enabled as 16 bit mem -> peripheral

void max7219_init(SPI_HandleTypeDef *spi_handle);

// set wakeup to 0 (display off) or 1 (display on)
void max7219_set_wakeup(int x);

// set led intensity (0..15)
void max7219_set_intensity(int x);

// set decimal points (low 4 bits of x)
void max7219_set_dp(int x);

// set decimal number (0..9999)
void max7219_set_number(uint x);

// set hex number (0..FFFF)
void max7219_set_hex(uint x);

// apply changes to display
void max7219_update();
