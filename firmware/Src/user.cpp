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
volatile int    rotation = 0;

uint32 second_elapsed = 0;

button_t button;

typedef void (*state_fn)();
state_fn current_state = null;

// how long been in current state
uint32 state_time = 0;

uint16 timer_left = 0;
int    press_time = 0;
int    beep_threshold = 3;
int    flash_threshold = 58;

//////////////////////////////////////////////////////////////////////

void turn_on();
void turn_off();

void state_off();
void state_countdown();
void state_menu();
void state_set_timer();
void state_set_brightness();
void state_set_beep();
void state_set_flash();

//////////////////////////////////////////////////////////////////////

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

void turn_off()
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
        MOSFET_GPIO_Port->BSRR = MOSFET_Pin;
        max7219_set_wakeup(1);
        max7219_set_intensity(15);  // load from flash
        second_elapsed = millis + 1000;
        timer_left = 120;   // load from flash
        press_time = 0;
        set_state(state_countdown);
    }
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
            turn_off();
            set_state(state_off);
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
    else if(button.released)
    {
        if(millis < press_time)
        {
            turn_off();
            set_state(state_off);
        }
        else
        {
            set_state(state_menu);
        }
    }

    // rotary encoder changes timer (for this run only)
    int r = atomic_exchange(&rotation, 0);
    if(r != 0)
    {
        timer_left = max(10, min(60 * 60 * 25, timer_left + r * 60)) / 10 * 10;
        second_elapsed = millis + 1000;
    }

    // update the 7 segment display
    max7219_set_number(get_display_time(timer_left));
    max7219_set_dp(1 << 2);
}

//////////////////////////////////////////////////////////////////////

void state_menu()
{
}

//////////////////////////////////////////////////////////////////////

void state_set_timer()
{
}

//////////////////////////////////////////////////////////////////////

void state_set_brightness()
{
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
    rotation += rotary_update(a | b);
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

        // this will be called, at minimum, every tick (100uS)
        button.update();
        buzzer_update();
        
        (*current_state)();

        max7219_update();
    }
}
