//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "buzzer.h"

//////////////////////////////////////////////////////////////////////

int const notes[] = { 36691, 34634, 32690, 30855, 29123, 27489, 25945, 24489, 23115, 21817, 20593, 19437, 18346, 17316, 16344, 15427, 14561,
                      13744, 12972, 12244, 11557, 10908, 10296, 9718,  9172,  8657,  8172,  7713,  7280,  6871,  6486,  6122,  5778,  5454,
                      5147,  4858,  4586,  4328,  4085,  3856,  3639,  3435,  3242,  3060,  2888,  2726,  2573,  2429,  2292 };

int    buzzer_pitch = 9718;
int    buzzer_volume = 100;
uint32 buzzer_off = 0;

//////////////////////////////////////////////////////////////////////
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

//////////////////////////////////////////////////////////////////////

void set_buzzer_pitch(int x)
{
    buzzer_pitch = x;
    TIM1->ARR = x;
    set_buzzer_volume(buzzer_volume);
}

//////////////////////////////////////////////////////////////////////

void set_buzzer_note(int note)
{
    note = max(0, min(note, countof(notes)));
    set_buzzer_pitch(notes[note]);
}

//////////////////////////////////////////////////////////////////////

void set_buzzer_state(bool on_or_off)
{
    if(on_or_off)
    {
        TIM1->BDTR |= TIM_BDTR_MOE;
        TIM1->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        TIM1->BDTR &= ~TIM_BDTR_MOE;
        TIM1->CR1 &= ~TIM_CR1_CEN;
    }
}

//////////////////////////////////////////////////////////////////////

void set_buzzer_duration(int ms)
{
    set_buzzer_state(true);
    buzzer_off = millis + ms;
}

//////////////////////////////////////////////////////////////////////

void buzzer_update()
{
    if(buzzer_off != 0 && millis >= buzzer_off)
    {
        set_buzzer_state(false);
        buzzer_off = 0;
    }
}

