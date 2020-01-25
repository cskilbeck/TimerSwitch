#include "main.h"

extern volatile int number;

uint16_t const max_setup[8] = {
    set_scan_limit(3),
    set_intensity(15),
    set_decode_mode(0),
    set_digit(0, 0x0),
    set_digit(1, 0x0),
    set_digit(2, 0x0),
    set_digit(3, 0x0),
    set_wakeup(1)
};

uint16_t digits[4] = { 0 };

#define B(x) (1<<(x-1))

uint8_t const seg_digits[10] = {
    B(2)+B(3)+B(4)+B(5)+B(6)+B(7),
    B(5)+B(6),
    B(7)+B(6)+B(1)+B(3)+B(4),
    B(1)+B(4)+B(5)+B(6)+B(7),
    B(2)+B(1)+B(6)+B(5),
    B(7)+B(2)+B(1)+B(5)+B(4),
    B(1)+B(2)+B(3)+B(4)+B(5)+B(7),
    B(7)+B(6)+B(5),
    B(1)+B(2)+B(3)+B(4)+B(5)+B(6)+B(7),
    B(1)+B(2)+B(4)+B(5)+B(6)+B(7)
};

#undef B

void set_digit_n(int digit, int value)
{
    digits[digit] = max7219_cmd(max_Digit0 + digit, seg_digits[value]);
}

void update_digits()
{
    while(hdma_spi1_tx.State == HAL_DMA_STATE_BUSY) {
    }
    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)digits, 4);
}

void set_number(int x)
{
    int d = 0;
    for(int i=0; i<4; ++i) {
        set_digit_n(i, x % 10);
        x /= 10;
    }
}

int state = 0;

int const rot_enc_table[16] = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};

int pinstate()
{
    return ~GPIOA->IDR & 0x3;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    state = ((state << 2) | pinstate()) & 0xf;
    int rot = rot_enc_table[state];
    if(rot != 0) {
        encoder_callback(rot);
    }
}

void encoder_callback(int x)
{
    number += x;
    if(number < 0) {
        number += 20000;
    } else if(number >= 20000) {
        number -= 20000;
    }
}
