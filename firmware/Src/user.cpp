
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "button.h"
#include "max7219.h"
#include "rotary.h"
#include "flash.h"
#include "buzzer.h"

//////////////////////////////////////////////////////////////////////

volatile uint32 ticks = 0;
volatile uint32 millis = 0;
volatile int    rotary_encoder = 0;
volatile uint32 second_elapsed = 0;
button_t        button;
int             knob_rotation = 0;

//////////////////////////////////////////////////////////////////////

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

struct state_t
{
    typedef void (*state_fn)();

    state_fn init;
    state_fn update;
};

enum class state
{
    invalid = -1,
    off = 0,
    countdown = 1,
    menu = 2,
    set_timer = 3,
    set_brightness = 4,
    set_beep = 5,
    set_flash = 6
};

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

state_t *current_state = null;
uint32   state_time = 0;
uint16   timer_left = 0;
int      press_time = 0;
int      beep_threshold = 3;
int      flash_threshold = 58;
int      display_brightness = 15;

//////////////////////////////////////////////////////////////////////

void set_state(state s)
{
    current_state = all_states + static_cast<int>(s);
    if(current_state->init != null)
    {
        current_state->init();
    }
    state_time = millis;
}

//////////////////////////////////////////////////////////////////////

int state_time_elapsed()
{
    return millis - state_time;
}

//////////////////////////////////////////////////////////////////////

void init_off()
{
    MOSFET_GPIO_Port->BRR = MOSFET_Pin;
    max7219_set_wakeup(0);
    max7219_set_intensity(15);
}

//////////////////////////////////////////////////////////////////////

void state_off()
{
    if(button.pressed)
    {
        timer_left = 60;    // load from flash
        set_state(state::countdown);
    }
}

//////////////////////////////////////////////////////////////////////

void init_countdown()
{
    MOSFET_GPIO_Port->BSRR = MOSFET_Pin;
    max7219_set_wakeup(1);
    max7219_set_intensity(display_brightness);    // load from flash
    second_elapsed = millis + 1000;
    press_time = 0;
}

//////////////////////////////////////////////////////////////////////

int get_display_time(uint16 seconds)
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

void state_countdown()
{
    // do counting down
    if(millis >= second_elapsed)
    {
        second_elapsed = millis + 1000;
        if(timer_left == 0)
        {
            set_buzzer_note(22);
            set_buzzer_duration(250);
            set_state(state::off);
        }
        else if(beep_threshold != 0 && timer_left <= beep_threshold)
        {
            set_buzzer_note(37);
            set_buzzer_duration(50);
        }
        timer_left = max(0, timer_left - 1);
    }

    // long/short press = menu/turn off
    if(button.pressed)
    {
        press_time = millis + 1000;
    }

    // rotary encoder changes timer (for this run only)
    if(knob_rotation != 0)
    {
        timer_left = max(10, min(60 * 60 * 24, timer_left + knob_rotation * 60)) / 10 * 10;
        second_elapsed = millis + 1000;
    }

    // update the 7 segment display
    max7219_set_number(get_display_time(timer_left));
    max7219_set_dp(1 << 2);

    if(button.down && press_time != 0 && millis > press_time)
    {
        set_state(state::menu);
    }

    else if(button.released && press_time != 0)
    {
        set_state(state::off);
    }
}

//////////////////////////////////////////////////////////////////////

char const *menu_items[] = { "SET ", "BRT ", "BEEP", "FLSH", "DONE" };

//////////////////////////////////////////////////////////////////////

state const menu_states[] = { state::set_timer, state::set_brightness, state::set_beep, state::set_flash, state::countdown };

//////////////////////////////////////////////////////////////////////

int    menu_index = 0;
uint32 idle_timer = 0;

//////////////////////////////////////////////////////////////////////

void setup_menu()
{
    max7219_set_string(menu_items[menu_index]);
}

//////////////////////////////////////////////////////////////////////

void init_menu()
{
    menu_index = 0;
    setup_menu();
}

//////////////////////////////////////////////////////////////////////

void state_menu()
{
    if(millis > idle_timer)
    {
        set_state(state::countdown);
        return;
    }
    if(knob_rotation != 0)
    {
        menu_index = max(0, min(countof(menu_items) - 1, menu_index + knob_rotation));
        setup_menu();
    }
    if(button.pressed)
    {
        set_state(menu_states[menu_index]);
    }
}

//////////////////////////////////////////////////////////////////////

void state_set_timer()
{
}

//////////////////////////////////////////////////////////////////////

void state_set_brightness()
{
    if(knob_rotation != 0)
    {
        display_brightness = max(0, min(15, display_brightness + knob_rotation));
        max7219_set_intensity(display_brightness);
    }
    max7219_set_number(display_brightness);
    if(button.pressed)
    {
        set_state(state::menu);
    }
}

//////////////////////////////////////////////////////////////////////

void state_set_beep()
{
}

//////////////////////////////////////////////////////////////////////

void state_set_flash()
{
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

        // this will be called, at minimum, every tick (100uS)
        button.update();
        buzzer_update();
        knob_rotation = atomic_exchange(&rotary_encoder, 0);

        // afk timer
        if(knob_rotation != 0 || button.pressed || button.released)
        {
            idle_timer = millis + 10000;
        }

        current_state->update();

        max7219_update();
    }
}
