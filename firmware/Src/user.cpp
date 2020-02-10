//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile uint32 millis = 0;
volatile int    rotation = 0;
int             number = 0;

button_t<32> button;

//////////////////////////////////////////////////////////////////////
// 1KHz timer ISR

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim17)
    {
        int b = (BTN_GPIO_Port->IDR & BTN_Pin) ? 0 : 1;
        button.read(b);
        ticks += 1;
    }
    else if(htim == &htim14)
    {
        millis += 1;
    }
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

int const notes[] = { 36691, 34634, 32690, 30855, 29123, 27489, 25945, 24489, 23115, 21817, 20593, 19437, 18346, 17316, 16344, 15427, 14561,
                      13744, 12972, 12244, 11557, 10908, 10296, 9718,  9172,  8657,  8172,  7713,  7280,  6871,  6486,  6122,  5778,  5454,
                      5147,  4858,  4586,  4328,  4085,  3856,  3639,  3435,  3242,  3060,  2888,  2726,  2573,  2429,  2292 };

int buzzer_pitch = 9718;
int buzzer_volume = 100;

// 0..256
void set_buzzer_volume(int x)
{
    buzzer_volume = x;
    x = (x * x) >> 8;
    x = (x * x) >> 8;
    x = (x * (buzzer_pitch >> 1)) >> 8;
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

int note_volume = 0;    // encoder action: note: 0, volume: 1
int note = 5;
int volume = 10;

int number_limit = countof(notes);
int number_delta = 1;

void set_number(int n)
{
    n = max(n, 0);
    n = min(n, number_limit);
    number = n;
}

extern "C" void user_main()
{
    HAL_TIM_Base_Start_IT(&htim17);
    HAL_TIM_Base_Start_IT(&htim14);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    set_buzzer_note(16);
    set_buzzer_volume(100);
    set_buzzer_state(true);

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
            note_volume = 1 - note_volume;
            
            if(note_volume == 0)
            {
                number_limit = countof(notes) - 1;
                number_delta = 1;
                set_number(note);
            }
            else
            {
                number_limit = 256;
                number_delta = 8;
                set_number(volume);
            }
        }

        // update number
        int r = atomic_exchange(&rotation, 0);
        if(r != 0)
        {
            set_number(number + r * number_delta);
            if(note_volume == 0)
            {
                set_buzzer_note(number);
                note = number;
            }
            else
            {
                set_buzzer_volume(number);
                volume = number;
            }
        }

        // update display
        max7219_set_number(number);
        max7219_set_dp(1 << 2);
        max7219_set_intensity(note_volume * 15);
        max7219_update();
    }
}
