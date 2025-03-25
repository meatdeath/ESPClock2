#ifndef __BUTTON_H__
#define __BUTTON_H__

typedef enum button_event_en 
{
    BUTTON_EVENT_NONE = 0,
    BUTTON_EVENT_PRESSED,
    BUTTON_EVENT_LONG_PRESS,
    BUTTON_EVENT_LONG_LONG_PRESS,
    BUTTON_EVENT_EXTRA_LONG_PRESS,
}
button_event_t;

void onButtonMsTimerIsr(void);
void buttonInit(void);

button_event_t getButtonEvent();

#endif // __BUTTON_H__
