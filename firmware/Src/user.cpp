//////////////////////////////////////////////////////////////////////
// TODO (chs): why buzzer so quiet?
//
// DONE (chs): menu: set timer
// DONE (chs): do flasher time
// DONE (chs): fix flash nvr load/save [unlock before save]
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"
#include "flash.h"
#include "buzzer.h"

//////////////////////////////////////////////////////////////////////
// flash var ids

enum
{
    flash_id_timer = 1,
    flash_id_brightness = 2,
    flash_id_beep = 3,
    flash_id_flash = 4
};

//////////////////////////////////////////////////////////////////////

struct state_t
{
    typedef void (*state_fn)();

    state_fn init;      // called when set_state()
    state_fn update;    // called in main loop
};

//////////////////////////////////////////////////////////////////////
// state machine

enum class state
{
    off = 0,
    countdown = 1,
    menu = 2,
    set_timer = 3,
    set_brightness = 4,
    set_beep = 5,
    set_flash = 6
};

//////////////////////////////////////////////////////////////////////
// state machine functions

void init_off();
void init_countdown();
void init_menu();

void state_off();
void state_countdown();
void state_menu();
void state_set_timer();
void state_set_brightness();
void state_set_beep();
void state_set_flash();

// these must line up with the enum

// clang-format off
state_t all_states[] =
{
    { init_off,                 state_off },
    { init_countdown,           state_countdown },
    { init_menu,                state_menu },
    { null,                     state_set_timer },
    { null,                     state_set_brightness },
    { null,                     state_set_beep },
    { null,                     state_set_flash }
};
// clang-format on

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile uint32 millis = 0;
volatile int    rotary_encoder = 0;
volatile uint32 second_elapsed = 0;
button_t        button;
int             knob_rotation = 0;

state_t *current_state = null;
state_t *next_state = null;
uint32   state_time = 0;
uint32   timer_start = 30;//60 * 30;
uint32   timer_left = 0;
int      press_time = 0;
int      beep_threshold = 3;
int      flash_threshold = 8;
int      display_brightness = 15;
int      menu_index = 0;
uint32   idle_timer = 0;

//////////////////////////////////////////////////////////////////////
// menu

// clang-format off
char const *menu_items[] =
{
    "SET ",
    "BRT ",
    "BEEP",
    "FLSH",
    "DONE"
};

state const menu_states[] =
{
    state::set_timer,
    state::set_brightness,
    state::set_beep,
    state::set_flash,
    state::countdown
};
// clang-format on

//////////////////////////////////////////////////////////////////////

void set_state(state s)
{
    next_state = all_states + static_cast<int>(s);
}

//////////////////////////////////////////////////////////////////////

void update_state()
{
    if(next_state != null)
    {
        current_state = next_state;
        next_state = null;
        if(current_state->init != null)
        {
            current_state->init();
        }
        state_time = millis;
    }
}

//////////////////////////////////////////////////////////////////////

void idle_check()
{
    if(millis > idle_timer)
    {
        set_state(state::countdown);
    }
}

//////////////////////////////////////////////////////////////////////

int state_time_elapsed()
{
    return millis - state_time;
}

//////////////////////////////////////////////////////////////////////

void init_off()
{
    gpio_clear(MOSFET_GPIO_Port, MOSFET_Pin);
    max7219_set_wakeup(0);
}

//////////////////////////////////////////////////////////////////////

void state_off()
{
    if(button.pressed)
    {
        timer_left = timer_start;
        set_state(state::countdown);
    }
}

//////////////////////////////////////////////////////////////////////

void init_countdown()
{
    gpio_set(MOSFET_GPIO_Port, MOSFET_Pin);
    max7219_set_wakeup(1);
    max7219_set_intensity(display_brightness);
    second_elapsed = millis + 1000;
    press_time = 0;
}

//////////////////////////////////////////////////////////////////////

int get_display_time(uint32 seconds)
{
    int minutes = seconds / 60;
    int hours = minutes / 60;

    int secs = seconds % 60;
    int mins = minutes % 60;

    int high = hours;
    int low = mins;
    if(hours < 1)
    {
        high = mins;
        low = secs;
    }
    return (high * 100) + low;
}

//////////////////////////////////////////////////////////////////////

uint32 knob_adjust(int t)
{
    int delta = 60;
    if(t >= 60 * 60)
    {
        delta = 60 * 10;
    }
    return clamp(60, 60 * 60 * 24, t + knob_rotation * delta) / delta * delta;
}

//////////////////////////////////////////////////////////////////////

void state_countdown()
{
    if(millis >= second_elapsed)
    {
        second_elapsed = millis + 1000;
        if(timer_left == 0)
        {
            if(beep_threshold != 0)
            {
                set_buzzer_note(22);
                set_buzzer_duration(250);
            }
            set_state(state::off);
        }
        else if(beep_threshold != 0 && timer_left <= beep_threshold)
        {
            set_buzzer_note(37);
            set_buzzer_duration(50);
        }
        timer_left = max(0, timer_left - 1);
    }

    // rotary encoder changes timer (for this run only)
    if(knob_rotation != 0)
    {
        timer_left = knob_adjust(timer_left);
        second_elapsed = millis + 1000;
    }

    // long press for menu, short press to turn off
    if(button.pressed)
    {
        press_time = millis + 1000;
    }
    if(press_time != 0)
    {
        if(button.down && millis > press_time)
        {
            set_state(state::menu);
        }
        else if(button.released)
        {
            set_state(state::off);
        }
    }

    // flash display when turning off soon
    if(timer_left <= flash_threshold)
    {
        int frac = second_elapsed - millis;
        max7219_set_wakeup((frac < 500) ? 1 : 0);
    }

    // update the 7 segment display
    max7219_set_number(get_display_time(timer_left));
    max7219_set_dp(1 << 2);
}

//////////////////////////////////////////////////////////////////////

void init_menu()
{
    menu_index = 0;
    max7219_set_wakeup(1);
}

//////////////////////////////////////////////////////////////////////

void state_menu()
{
    idle_check();
    menu_index = clamp(0, countof(menu_items) - 1, menu_index + knob_rotation);
    max7219_set_string(menu_items[menu_index]);
    if(button.pressed)
    {
        set_state(menu_states[menu_index]);
    }
}

//////////////////////////////////////////////////////////////////////

template <typename T> void save_var(int id, T const &var)
{
    flash::unlock();
    flash::save(id, var);
    flash::lock();
}

//////////////////////////////////////////////////////////////////////

void state_set_timer()
{
    idle_check();
    timer_start = knob_adjust(timer_start);
    if(button.pressed)
    {
        save_var(flash_id_timer, timer_start);
        set_state(state::menu);
    }
    max7219_set_number(get_display_time(timer_start));
    max7219_set_dp(1 << 2);
}

//////////////////////////////////////////////////////////////////////

void state_set_brightness()
{
    idle_check();
    display_brightness = clamp(0, 15, display_brightness + knob_rotation);
    if(button.pressed)
    {
        save_var(flash_id_brightness, display_brightness);
        set_state(state::menu);
    }
    max7219_set_intensity(display_brightness);
    max7219_set_number(display_brightness + 1, 0);
}

//////////////////////////////////////////////////////////////////////

void state_set_beep()
{
    idle_check();
    beep_threshold = clamp(0, 60, beep_threshold + knob_rotation);
    if(button.pressed)
    {
        save_var(flash_id_beep, beep_threshold);
        set_state(state::menu);
    }
    max7219_set_number(beep_threshold, 0);
}

//////////////////////////////////////////////////////////////////////

void state_set_flash()
{
    idle_check();
    flash_threshold = clamp(0, 60, flash_threshold + knob_rotation);
    if(button.pressed)
    {
        save_var(flash_id_flash, flash_threshold);
        set_state(state::menu);
    }
    max7219_set_number(flash_threshold, 0);
}

//////////////////////////////////////////////////////////////////////
// a timer elapsed
// can't do individual callbacks until STM32CubeMX allows setting
// the special flag. When HW is fixed, we can set it ourselves
// because we won't be regenerating the code again

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
    }
}

//////////////////////////////////////////////////////////////////////
// some activity on rotary encoder lines

extern "C" void HAL_GPIO_EXTI_Callback(uint16 GPIO_Pin)
{
    int a = gpio_get(ROTARYA_GPIO_Port, ROTARYA_Pin) ? 2 : 0;
    int b = gpio_get(ROTARYB_GPIO_Port, ROTARYB_Pin) ? 1 : 0;
    rotary_encoder += rotary_update(a | b);
}

//////////////////////////////////////////////////////////////////////
// user_main() is called after HW init in main.c

extern "C" void user_main()
{
    // start tick / button timer (10KHz)
    HAL_TIM_Base_Start_IT(&htim17);

    // start millisecond timer (1KHz)
    HAL_TIM_Base_Start_IT(&htim14);

    // start buzzer PWM timer
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);

    // but then turn the buzzer off
    set_buzzer_state(false);

    // setup the 7 segment display
    max7219_init(&hspi1);

    // go into 'off' mode
    set_state(state::off);

    flash::load(flash_id_timer, timer_start);
    flash::load(flash_id_beep, beep_threshold);
    flash::load(flash_id_brightness, display_brightness);
    flash::load(flash_id_flash, flash_threshold);

    while(true)
    {
        // sleep until an ISR has fired
        __WFI();

        button.update();
        buzzer_update();
        knob_rotation = atomic_exchange(&rotary_encoder, 0);

        // afk timer
        if(knob_rotation != 0 || button.pressed || button.released)
        {
            idle_timer = millis + 10000;
        }

        update_state();

        current_state->update();

        max7219_update();
    }
}
