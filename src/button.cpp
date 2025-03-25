#include "main.h"

//-----------------------------------------------------------------------------

volatile uint16_t button_debounce_counter = 0;
volatile bool current_button_state = false;
volatile bool button_short_press = false;
volatile bool button_long_press = false;
volatile bool button_long_long_press = false;
volatile bool button_extra_long_press = false;

//-----------------------------------------------------------------------------

void onButtonMsTimerIsr(void)
{
    if (current_button_state && (digitalRead(BUTTON_PIN)?false:true))
    {
        button_debounce_counter++;
    }
    else
    {
        current_button_state = false;
        button_debounce_counter = 0;
    }
    if (button_debounce_counter == 100) button_short_press = true;
    if (button_debounce_counter == 3000) button_long_press = true;
    if (button_debounce_counter == 10000) button_long_long_press = true;
    if (button_debounce_counter == 20000) button_extra_long_press = true;
}

//-----------------------------------------------------------------------------

void IRAM_ATTR buttonIsr() 
{
    timerAlarmEnable(ms_timer);
    current_button_state = true;
    button_debounce_counter = 0;
}

//-----------------------------------------------------------------------------

void buttonInit(void)
{
	pinMode(BUTTON_PIN, INPUT_PULLUP);
	attachInterrupt(BUTTON_PIN, buttonIsr, FALLING);
}

//-----------------------------------------------------------------------------

button_event_t getButtonEvent()
{
    if (button_short_press) 
    {
        button_short_press = false;
        return BUTTON_EVENT_PRESSED;
    }

    if (!current_button_state)
    {
        if (button_extra_long_press)
        {
            button_extra_long_press = false;
            button_long_long_press = false;
            button_long_press = false;
            return BUTTON_EVENT_EXTRA_LONG_PRESS;
        }
        else if (button_long_long_press)
        {
            button_long_long_press = false;
            button_long_press = false;
            return BUTTON_EVENT_LONG_LONG_PRESS;
        }
        else if (button_long_press && !current_button_state)
        {
            button_long_press = false;
            return BUTTON_EVENT_LONG_PRESS;
        }
    }
    return BUTTON_EVENT_NONE;
}

//-----------------------------------------------------------------------------

