//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile int    rotation = 0;

int number = 0;
int display_number = 0;

//////////////////////////////////////////////////////////////////////

uint32 button_state = 0;
uint32 button_history = 0;

volatile int button_held = 0;
volatile int button_pressed = 0;
volatile int button_released = 0;

//////////////////////////////////////////////////////////////////////

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    button_state = ((BUTTON_GPIO_Port->IDR & BUTTON_Pin) == 0) ? 1 : 0;
    button_history = ((button_history << 1) | button_state) & 0xf;

    // 3 offs followed by an on = a button down event
    if(button_history == 0x1)
    {
        button_held = 1;
        button_pressed += 1;
    }

    // 3 ons followed by an off = a button up event
    else if(button_history == 0xe)
    {
        button_held = 0;
        button_released += 1;
    }
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////

void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    rotation += rotary_update();
}

//////////////////////////////////////////////////////////////////////

void begin()
{
    HAL_TIM_Base_Start_IT(&htim17);
    max7219_init(&hspi1);
}

//////////////////////////////////////////////////////////////////////

void loop()
{
    __WFI();

    DEBUG1_GPIO_Port->BSRR = DEBUG1_Pin << (button_state * 16);

    if(button_pressed != 0)
    {
        MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
        button_pressed = 0;
    }

    int i = min(7, max(-7, abs((((int)ticks >> 12) & 63) - 32) - 16)) + 8;

    max7219_set_intensity(i);

    number += rotation;
    rotation = 0;
    if(number < 0)
    {
        number += 10000;
    }
    else if(number >= 10000)
    {
        number -= 10000;
    }

    if(display_number != number)
    {
        display_number = number;
        max7219_set_number(number);
    }

    max7219_update();
}
