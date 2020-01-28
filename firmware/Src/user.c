//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;

volatile int number = 0;
int display_number = 0;

//////////////////////////////////////////////////////////////////////

uint32 button_state = 0;
uint32 button_history = 0;
int button_held = 0;
int button_previous_held = 0;
int button_pressed = 0;
int button_released = 0;

//////////////////////////////////////////////////////////////////////

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    button_state = ((BUTTON_GPIO_Port->IDR & BUTTON_Pin) == 0) ? 1 : 0;
    button_history = ((button_history << 1) | button_state) & 0xf;
    if(button_history == 0x1) {
        button_held = 1;
    } else if(button_history == 0xe) {
        button_held = 0;
    }
    int button_diff = button_held ^ button_previous_held;
    button_pressed = button_diff & button_held;
    button_released = button_diff & !button_held;
    button_previous_held = button_held;
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////

void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    number += rotary_update();
    if(number < 0) {
        number += 10000;
    } else if(number >= 10000) {
        number -= 10000;
    }
}

//////////////////////////////////////////////////////////////////////

void begin()
{
    HAL_TIM_Base_Start_IT(&htim17);
    max7219_init();
}

//////////////////////////////////////////////////////////////////////

void loop()
{
    DEBUG_GPIO_Port->BSRR = DEBUG_Pin << (button_state * 16);

    if(button_pressed) {
        MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
        button_pressed = 0;
    }

    max7219_set_intensity(ticks >> 12);

    if(display_number != number) {
        display_number = number;
        max7219_set_number(number);
    }

    max7219_update();
}
