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

volatile uint32_t milliseconds = 0;
volatile int number = 0;
int display_number = 0;

#define B(x) (1 << (x - 1))

#define dig_zero (B(2) + B(3) + B(4) + B(5) + B(6) + B(7))

uint8_t const seg_digits[10] = { B(2) + B(3) + B(4) + B(5) + B(6) + B(7),
                                 B(5) + B(6),
                                 B(7) + B(6) + B(1) + B(3) + B(4),
                                 B(1) + B(4) + B(5) + B(6) + B(7),
                                 B(2) + B(1) + B(6) + B(5),
                                 B(7) + B(2) + B(1) + B(5) + B(4),
                                 B(1) + B(2) + B(3) + B(4) + B(5) + B(7),
                                 B(7) + B(6) + B(5),
                                 B(1) + B(2) + B(3) + B(4) + B(5) + B(6) + B(7),
                                 B(1) + B(2) + B(4) + B(5) + B(6) + B(7) };

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

#undef B

uint16_t digits[4] = { 0 };

int state = 0;
uint32_t last_press = 0;

int const rot_enc_table[16] = { 0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0 };

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch(GPIO_Pin) {
    case ENCODER_A_Pin:
    case ENCODER_B_Pin:
        state = ((state << 2) | (~GPIOA->IDR & 0x3)) & 0xf;
        int rot = rot_enc_table[state];
        if(rot != 0) {
            number += rot;
            if(number < 0) {
                number += 20000;
            } else if(number >= 20000) {
                number -= 20000;
            }
        }
        break;
    case BUTTON_Pin:
        if(milliseconds > last_press)     // work around weird GPIO_EXTI problem
        {
            last_press = milliseconds + 1;
            MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
            DEBUG_LED_GPIO_Port->ODR ^= DEBUG_LED_Pin;
        }
        break;
    }
}

void begin()
{
    HAL_TIM_Base_Start_IT(&htim17);
    HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)max_setup, 8);
}

void loop()
{
    __wfi();
    int x = number / 2;
    if(display_number != x) {
        display_number = x;
        for(int i = 0; i < 4; ++i) {
            digits[i] = max7219_cmd(max_Digit0 + i, seg_digits[x % 10]);
            x /= 10;
        }
        while(hdma_spi1_tx.State == HAL_DMA_STATE_BUSY) {
        }
        HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)digits, 4);
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    milliseconds += 1;
}
