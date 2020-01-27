#include "main.h"

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

#define max7219_cmd(addr, data) ((((addr)&0xf) << 8) | ((data)&0xff))

#define set(x, y) max7219_cmd(max_##x, (y))

#define set_intensity(x) set(Intensity, (x))
#define set_scan_limit(x) set(ScanLimit, (x))
#define set_wakeup(x) set(WakeUp, (x))
#define set_decode_mode(x) set(DecodeMode, (x))
#define set_digit(x, y) set(Digit##x, (y))

#define B(x) (1 << (x - 1))

uint8_t const seg_digits[16] = { B(2) + B(3) + B(4) + B(5) + B(6) + B(7),
                                 B(5) + B(6),
                                 B(7) + B(6) + B(1) + B(3) + B(4),
                                 B(1) + B(4) + B(5) + B(6) + B(7),
                                 B(2) + B(1) + B(6) + B(5),
                                 B(7) + B(2) + B(1) + B(5) + B(4),
                                 B(1) + B(2) + B(3) + B(4) + B(5) + B(7),
                                 B(7) + B(6) + B(5),
                                 B(1) + B(2) + B(3) + B(4) + B(5) + B(6) + B(7),
                                 B(1) + B(2) + B(4) + B(5) + B(6) + B(7),
                                 B(1) + B(2) + B(3) + B(5) + B(6) + B(7),
                                 B(1) + B(2) + B(3) + B(4) + B(5),
                                 B(7) + B(2) + B(3) + B(4),
                                 B(1) + B(3) + B(4) + B(5) + B(6), 
                                 B(1) + B(2) + B(3) + B(4) + B(7), 
                                 B(1) + B(2) + B(3) + B(7)
};

#undef B

// clang-format off
uint16_t const max_setup[8] = {
    set_scan_limit(3),
    set_intensity(15),
    set_decode_mode(0),
    set_digit(0, 0),
    set_digit(1, 0),
    set_digit(2, 0),
    set_digit(3, 0),
    set_wakeup(1)
};
// clang-format on

uint16_t digits[5] = { set_intensity(15) };

static int dirty = 0;

void max7219_init()
{
    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)max_setup, 8);
    dirty = 0;
}

void max7219_set_number(int x)
{
    for(int i = 0; i < 4; ++i) {
        digits[i + 1] = max7219_cmd(max_Digit0 + i, seg_digits[x % 10]);
        x /= 10;
    }
    dirty = 1;
}

void max7219_set_intensity(int x)
{
    x &= 0xf;
    if(x != (digits[0] & 0xf)) {
        digits[0] = set_intensity(x & 0xf);
        dirty = 1;
    }
}

void max7219_set_hex(int x)
{
    for(int i = 0; i < 4; ++i) {
        digits[i + 1] = max7219_cmd(max_Digit0 + i, seg_digits[x & 0xf]);
        x >>= 4;
    }
    dirty = 1;
}

void max7219_update()
{
    if(dirty) {
        while(hdma_spi1_tx.State == HAL_DMA_STATE_BUSY) {
        }
        HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)digits, 5);
        dirty = 0;
    }
}
