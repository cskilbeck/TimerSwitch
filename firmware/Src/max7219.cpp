//////////////////////////////////////////////////////////////////////
// MAX7219 functions
// https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf

#include "main.h"

//////////////////////////////////////////////////////////////////////
// registers

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

//////////////////////////////////////////////////////////////////////
// encoding for command words

#define max7219_cmd(addr, data) ((((addr)&0xf) << 8) | ((data)&0xff))
#define set(x, y) max7219_cmd(max_##x, (y))
#define set_intensity(x) set(Intensity, (x))
#define set_scan_limit(x) set(ScanLimit, (x))
#define set_wakeup(x) set(WakeUp, (x))
#define set_decode_mode(x) set(DecodeMode, (x))
#define set_digit(x, y) set(Digit##x, (y))

//////////////////////////////////////////////////////////////////////
// hex digit lokup table
// if segments are wired differently, modify this table

#define _SL(x) (1 << (x - 1))

uint8_t const seg_digits[16] = { _SL(2) + _SL(3) + _SL(4) + _SL(5) + _SL(6) + _SL(7),
                                 _SL(5) + _SL(6),
                                 _SL(7) + _SL(6) + _SL(1) + _SL(3) + _SL(4),
                                 _SL(1) + _SL(4) + _SL(5) + _SL(6) + _SL(7),
                                 _SL(2) + _SL(1) + _SL(6) + _SL(5),
                                 _SL(7) + _SL(2) + _SL(1) + _SL(5) + _SL(4),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(4) + _SL(5) + _SL(7),
                                 _SL(7) + _SL(6) + _SL(5),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(4) + _SL(5) + _SL(6) + _SL(7),
                                 _SL(1) + _SL(2) + _SL(4) + _SL(5) + _SL(6) + _SL(7),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(5) + _SL(6) + _SL(7),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(4) + _SL(5),
                                 _SL(7) + _SL(2) + _SL(3) + _SL(4),
                                 _SL(1) + _SL(3) + _SL(4) + _SL(5) + _SL(6),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(4) + _SL(7),
                                 _SL(1) + _SL(2) + _SL(3) + _SL(7) };

#undef _SL

//////////////////////////////////////////////////////////////////////
// setup packet

// offsets into setup_packet[]
enum
{
    setup_base = 0,
    setup_update = 2,

    setup_intensity = 2,
    setup_wakeup = 3,
    setup_digit0 = 4,
    setup_digit3 = 7,

    setup_count = 8
};

// clang-format off
uint16_t setup_packet[8] = {
    set_scan_limit(3),
    set_decode_mode(0),
    set_intensity(15),
    set_wakeup(1),
    set_digit(0, 0x55),
    set_digit(1, 0xaa),
    set_digit(2, 0x00),
    set_digit(3, 0xff),
};
// clang-format on

static bool dirty = false;    // track if setup_packet changed

SPI_HandleTypeDef *max_spi_handle = null;

extern unsigned char segments[128];

//////////////////////////////////////////////////////////////////////

static byte ascii_to_segments(int c)
{
    return segments[c & 127];
}

//////////////////////////////////////////////////////////////////////
// poke new command word in setup_packet[]

static void set_entry(int index, int mask, int addr, int value)
{
    uint16 poke = max7219_cmd(addr, value & mask);
    if(poke != setup_packet[index])
    {
        setup_packet[index] = poke;
        dirty = true;
    }
}

//////////////////////////////////////////////////////////////////////
// send setup_packet[] from some offset

static void transmit_dma(int n)
{
    while(hdma_spi1_tx.State == HAL_DMA_STATE_BUSY)
    {
        // toggle debug line here to see if it's stalling
    }
    HAL_SPI_Transmit_DMA(max_spi_handle, (uint8_t *)(setup_packet + n), setup_count - n);
}

//////////////////////////////////////////////////////////////////////
// public function: init the max7219

void max7219_init(SPI_HandleTypeDef *spi_handle)
{
    max_spi_handle = spi_handle;
    transmit_dma(setup_base);
    dirty = false;
}

//////////////////////////////////////////////////////////////////////
// public function: set wakeup to 0 (display off) or 1 (display on)

void max7219_set_wakeup(int x)
{
    set_entry(setup_wakeup, 0x1, max_WakeUp, x);
}

//////////////////////////////////////////////////////////////////////
// public function: set led intensity (0..15)

void max7219_set_intensity(int x)
{
    set_entry(setup_intensity, 0xf, max_Intensity, x);
}

//////////////////////////////////////////////////////////////////////
// public function: set decimal points (low 4 bits of x)

__volatile__ void max7219_set_dp(int x)
{
    uint16 *p = setup_packet + setup_digit0;
    x <<= 7;
    for(int i=0; i<4; ++i)
    {
        uint16 n = (*p & 0xff7f) | (x & 0x80);
        if(n != *p)
        {
            *p = n;
            dirty = true;
        }
        p += 1;
        x >>= 1;
    }
}

//////////////////////////////////////////////////////////////////////

void max7219_set_string(char const *p)
{
    for(int i=0; i<4; ++i)
    {
        set_entry(setup_digit3 - i, 0xff, max_Digit3 - i, ascii_to_segments(p[i]));
    }
}

//////////////////////////////////////////////////////////////////////
// public function: set decimal number (0..9999)

void max7219_set_number(uint x, int leading)
{
    int i;
    for(i = 0; i < 4; ++i)
    {
        set_entry(setup_digit0 + i, 0xff, max_Digit0 + i, seg_digits[x % 10]);
        x /= 10;
        if(x == 0 && i >= leading)
        {
            break;
        }
    }
    for(i += 1; i < 4; ++i)
    {
        set_entry(setup_digit0 + i, 0xff, max_Digit0 + i, 0);
    }
}

//////////////////////////////////////////////////////////////////////
// public function: set hex number (0..FFFF)

void max7219_set_hex(int x)
{
    for(int i = 0; i < 4; ++i)
    {
        set_entry(setup_digit0 + i, 0xff, max_Digit0 + 1, seg_digits[x & 0xf]);
        x >>= 4;
    }
}

//////////////////////////////////////////////////////////////////////
// public function: apply changes to display

void max7219_update()
{
    if(dirty)
    {
        transmit_dma(setup_update);
        dirty = false;
    }
}
