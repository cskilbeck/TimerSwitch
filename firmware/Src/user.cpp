//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile int    number = 0;

int display_number = 0;

//////////////////////////////////////////////////////////////////////

volatile button_t button(BUTTON_GPIO_Port, 9);

//////////////////////////////////////////////////////////////////////

// GPIO_PIN_9

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    button.update();
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////

extern "C" void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    number += rotary_update();
    if(number < 0)
    {
        number += 10000;
    }
    else if(number >= 10000)
    {
        number -= 10000;
    }
}

//////////////////////////////////////////////////////////////////////

extern "C" void begin()
{
    HAL_TIM_Base_Start_IT(&htim17);
    max7219_init(&hspi1);
}

//////////////////////////////////////////////////////////////////////

extern "C" void loop()
{
    __WFI();

    DEBUG1_GPIO_Port->BSRR = DEBUG1_Pin << (button_state * 16);

    if(button_pressed != 0)
    {
        MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
        button_pressed = 0;
    }

    int i = min(7, max(-7, abs(((ticks >> 12) & 63) - 32) - 16)) + 8;

    max7219_set_intensity(i);

    if(display_number != number)
    {
        display_number = number;
        max7219_set_number(number);
    }

    max7219_update();
}
