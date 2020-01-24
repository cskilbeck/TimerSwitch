#pragma once

extern volatile uint32_t milliseconds;

#define max_NoOp 0
#define max_Digit0 1
#define max_Digit1 2
#define max_Digit2 3
#define max_Digit3 4
#define max_Digit4 5
#define max_Digit5 6
#define max_Digit6 7
#define max_Digit7 8
#define max_DecodeMode 9
#define max_Intensity 10
#define max_ScanLimit 11
#define max_WakeUp 12
#define max_DisplayTest 15

#define max7219_cmd(addr, data) ((((addr) & 0xf) << 8) | ((data) & 0xff))

#define set(x, y) max7219_cmd(max_##x, (y))

#define set_intensity(x) set(Intensity, (x))
#define set_scan_limit(x) set(ScanLimit, (x))
#define set_wakeup(x) set(WakeUp, (x))
#define set_decode_mode(x) set(DecodeMode, (x))
#define set_digit(x, y) set(Digit##x, (y))

extern uint16_t const max_setup[];

extern uint8_t const seg_digits[];
extern uint16_t digits[];

extern DMA_HandleTypeDef hdma_spi1_tx;
extern SPI_HandleTypeDef hspi1;


void set_digit_n(int digit, int value);
void update_digits();
void set_number(int n);