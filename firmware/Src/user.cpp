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

constexpr uintptr GPIO_PORT_A = 0x48000000UL;

button_t<GPIO_PORT_A, 9, 32> button;

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

struct button_state_t
{
    int held;
    int previous;
    int pressed;
    int released;

    void update(int new_state)
    {
        held = new_state;
        int change = held ^ previous;
        previous = held;
        pressed = change && held;
        released = change && !held;
    }
};

button_state_t button_state;

//////////////////////////////////////////////////////////////////////

extern "C" void loop()
{
    __WFI();

    button_state.update(button.down);

    DEBUG1_GPIO_Port->BSRR = DEBUG1_Pin << (button_state.held * 16);

    if(button_state.pressed != 0)
    {
        MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
    }

    int i = min(7, max(-7, abs((((int)ticks >> 12) & 63) - 32) - 16)) + 8;

    max7219_set_intensity(i);

    if(display_number != number)
    {
        display_number = number;
        max7219_set_number(number);
    }

    max7219_update();
}
