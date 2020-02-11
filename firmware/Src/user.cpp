//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"
#include "flash.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile uint32 millis = 0;
volatile int    rotation = 0;
int             number = 0;
volatile int    minute = 0;
int             ms = 0;

button_t<16> button;

//////////////////////////////////////////////////////////////////////

// state is just a fn pointer
typedef void (*state_fn)();
state_fn current_state = null;

// how long been in current state
uint32 state_time = 0;

//////////////////////////////////////////////////////////////////////

void set_state(state_fn new_state)
{
    current_state = new_state;
    state_time = millis;
}

//////////////////////////////////////////////////////////////////////

int state_time_elapsed()
{
    return millis - state_time;
}

//////////////////////////////////////////////////////////////////////
// 1KHz timer ISR

extern "C" void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim == &htim17)
    {
        int b = gpio_get(BTN_GPIO_Port, BTN_Pin) ? 0 : 1;
        button.read(b);
        ticks += 1;
    }
    else if(htim == &htim14)
    {
        millis += 1;
        ms -= 1;
        if(ms <= 0)
        {
            minute = 1;
            ms = 1000;
        }
    }
}

//////////////////////////////////////////////////////////////////////
// some activity on rotary encoder lines

extern "C" void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    int a = gpio_get(ROTARYA_GPIO_Port, ROTARYA_Pin) ? 2 : 0;
    int b = gpio_get(ROTARYB_GPIO_Port, ROTARYB_Pin) ? 1 : 0;
    rotation += rotary_update(a | b);
}

//////////////////////////////////////////////////////////////////////
// user_main() is called after HW init is complete

int const notes[] = { 36691, 34634, 32690, 30855, 29123, 27489, 25945, 24489, 23115, 21817, 20593, 19437, 18346, 17316, 16344, 15427, 14561,
                      13744, 12972, 12244, 11557, 10908, 10296, 9718,  9172,  8657,  8172,  7713,  7280,  6871,  6486,  6122,  5778,  5454,
                      5147,  4858,  4586,  4328,  4085,  3856,  3639,  3435,  3242,  3060,  2888,  2726,  2573,  2429,  2292 };

int    buzzer_pitch = 9718;
int    buzzer_volume = 100;
uint32 buzzer_off = 0;

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
        TIM1->BDTR |= TIM_BDTR_MOE;
        TIM1->CR1 |= TIM_CR1_CEN;
    }
    else
    {
        TIM1->BDTR &= ~TIM_BDTR_MOE;
        TIM1->CR1 &= ~TIM_CR1_CEN;
    }
}

void set_buzzer_duration(int ms)
{
    set_buzzer_state(true);
    buzzer_off = millis + ms;
}

void buzzer_update()
{
    if(buzzer_off != 0 && millis >= buzzer_off)
    {
        set_buzzer_state(false);
        buzzer_off = 0;
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

int get_display_time(uint16 timer_left)
{
    int mins = timer_left / 60;
    int hours = mins / 60;
    int minutes = mins % 60;
    int seconds = timer_left % 60;
    int high = hours;
    int low = minutes;
    if(hours < 1)
    {
        high = minutes;
        low = seconds;
    }
    return (high * 100) + low;
}

enum
{
    state_off = 0,
    state_on = 1,
    state_menu = 2
};

int    state = state_off;
uint16 timer_left = 0;
int    press_time = 0;
int    beep_threshold = 3;
int    flash_threshold = 58;

void turn_on()
{
    MOSFET_GPIO_Port->BSRR = MOSFET_Pin;
    max7219_set_wakeup(1);
    max7219_set_intensity(15);
    ms = 1000;
    minute = 0;
    timer_left = 120;
    press_time = 0;
    state = state_on;
}

void turn_off()
{
    MOSFET_GPIO_Port->BRR = MOSFET_Pin;
    max7219_set_wakeup(0);
    max7219_set_intensity(15);
    state = state_off;
}

void show_menu()
{
    // state = menu;
}

extern "C" void user_main()
{
    HAL_TIM_Base_Start_IT(&htim17);
    HAL_TIM_Base_Start_IT(&htim14);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    set_buzzer_note(37);
    set_buzzer_volume(250);
    set_buzzer_state(false);

    max7219_init(&hspi1);

    turn_off();

    // flash var ids
    enum
    {
        flashid_timer = 1
    };

    if(flash::load(flashid_timer, sizeof(uint16), (byte *)&timer_left) == flash::ok)
    {
    }

    while(true)
    {
        // sleep until an ISR has fired
        __WFI();

        if(minute && state == state_on)
        {
            minute = 0;
            if(timer_left == 0)
            {
                set_buzzer_note(22);
                set_buzzer_duration(250);
                turn_off();
            }
            else if(beep_threshold != 0 && timer_left <= beep_threshold)
            {
                set_buzzer_note(37);
                set_buzzer_duration(50);
            }
            timer_left = max(0, timer_left - 1);
        }

        button.update();
        buzzer_update();

        if(button.pressed)
        {
            if(state == state_off)
            {
                turn_on();
            }
            else
            {
                press_time = millis + 1000;
            }
        }

        if(button.released)
        {
            if(press_time != 0 && millis < press_time)
            {
                turn_off();
                set_buzzer_state(false);
            }
            else
            {
                show_menu();
            }
            press_time = 0;
        }

        int r = atomic_exchange(&rotation, 0);
        if(r != 0)
        {
            timer_left = max(10, min(60 * 60 * 25, timer_left + r * 60)) / 10 * 10;
            ms = 1000;
        }

        if(timer_left < flash_threshold)
        {
        }

        // update display
        max7219_set_number(get_display_time(timer_left));
        max7219_set_dp(1 << 2);
        max7219_update();
    }
}
