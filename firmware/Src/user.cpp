//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile int    rotation = 0;
int             number = 0;

button_t<32> button;

//////////////////////////////////////////////////////////////////////
// 1KHz timer ISR

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    int b = (BTN_GPIO_Port->IDR & BTN_Pin) ? 0 : 1;
    button.read(b);
    ticks += 1;
}

//////////////////////////////////////////////////////////////////////
// some activity on rotary encoder lines

extern "C" void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    int a = (ROTARYA_GPIO_Port->IDR & ROTARYA_Pin) ? 2 : 0;
    int b = (ROTARYB_GPIO_Port->IDR & ROTARYB_Pin) ? 1 : 0;
    rotation += rotary_update(a | b);
}

//////////////////////////////////////////////////////////////////////
// user_main() is called after HW init is complete

int const notes[] = { 365, 345, 325, 307, 290, 273, 258, 243, 230, 217, 204, 193, 182, 172, 162, 153, 144, 136, 128, 121, 114, 108, 101, 96, 90,
                      85,  80,  76,  71,  67,  63,  60,  56,  53,  50,  47,  44,  42,  39,  37,  35,  33,  31,  29,  27,  26,  24,  23,  21 };

int buzzer_pitch = 240;
int buzzer_volume = 4000;

// 0..65535
void set_buzzer_volume(int x)
{
    buzzer_volume = x;
    x = (x * buzzer_pitch) >> 10;
    TIM1->CCER &= ~(TIM_CCER_CC1E | TIM_CCER_CC2E);
    TIM1->CCR2 = x;
    TIM1->CCR3 = x;
    TIM1->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
}

// 23999 is annoying high pitched beep
// etc
void set_buzzer_pitch(int x)
{
    buzzer_pitch = x;
    TIM1->ARR = x;
    set_buzzer_volume(buzzer_volume);
}

void set_buzzer_note(int note)
{
    note = max(0, min(note, countof(notes)));
    set_buzzer_pitch(notes[note]);
}

void set_buzzer_state(bool on_or_off)
{
    if(on_or_off)
    {
        TIM1->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        TIM1->CR1 &= ~TIM_CR1_CEN;
    }
}

bool buzz = false;

extern "C" void user_main()
{
    HAL_TIM_Base_Start_IT(&htim17);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    set_buzzer_note(16);
    set_buzzer_volume(100);
    set_buzzer_state(true);
    buzz = true;

    max7219_init(&hspi1);

    while(true)
    {
        // sleep until an ISR has fired
        __WFI();

        button.update();

        // button toggles relay
        if(button.pressed)
        {
            MOSFET_GPIO_Port->ODR ^= MOSFET_Pin;
            buzz = !buzz;
            set_buzzer_state(buzz);
        }

        // update number
        int r = atomic_exchange(&rotation, 0);
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
            set_buzzer_volume(number);
        }

        // update display
        max7219_set_number(number);
        max7219_set_dp(1 << 2);

        // pulse brightness
        max7219_set_intensity(min(7, max(-7, abs((((int)ticks >> 9) & 63) - 32) - 16)) + 8);

        // send to driver
        max7219_update();
    }
}
