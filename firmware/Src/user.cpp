//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

constexpr uintptr GPIO_PORT_A = 0x48000000UL;
constexpr int     BUTTON_BIT = 9;

volatile uint32 ticks = 0;
volatile int    rotation = 0;
int             number = 0;
button_state_t  button_state;

button_t<GPIO_PORT_A, BUTTON_BIT, 32> button;

//////////////////////////////////////////////////////////////////////
// 10KHz timer ISR

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    button.update();
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////
// some activity on rotary encoder lines

extern "C" void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    rotation += rotary_update();
}

//////////////////////////////////////////////////////////////////////
// user_main() is called after HW init is complete

extern "C" void user_main()
{
    HAL_TIM_Base_Start_IT(&htim17);
    max7219_init(&hspi1);

    while(true)
    {
        // sleep until an ISR has fired
        __WFI();

        button_state.update(button.down);

        // debug button state on DEBUG1
        DEBUG1_GPIO_Port->BSRR = DEBUG1_Pin << (button_state.held * 16);

        // button toggles relay
        if(button_state.pressed != 0)
        {
            MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
        }

        // pulse display brightness
        int i = min(7, max(-7, abs((((int)ticks >> 12) & 63) - 32) - 16)) + 8;
        max7219_set_intensity(i);

        // update number
        int r = atomic_read_clear(&rotation);
        if(r != 0)
        {
            number += r;
            if(number < 0)
            {
                number += 10000;
            }
            else if(number >= 10000)
            {
                number -= 10000;
            }
            max7219_set_number(number);
        }
        max7219_update();
    }
}
