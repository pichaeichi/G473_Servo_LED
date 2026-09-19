#ifndef BUTTON_H
#define BUTTON_H

#include "main.h"
#include <stdint.h>

typedef enum
{
    BUTTON_EVENT_NONE   = 0U,
    BUTTON_EVENT_SHORT  = 1U << 0,
    BUTTON_EVENT_DOUBLE = 1U << 1,
    BUTTON_EVENT_LONG   = 1U << 2
} ButtonEvent;

typedef struct
{
    GPIO_TypeDef *port;
    uint16_t pin;
    GPIO_PinState pressed_level;

    uint16_t debounce_ms;
    uint16_t press_ms;
    uint16_t click_wait_ms;
    uint8_t stable_pressed;
    uint8_t long_reported;
    uint8_t click_count;
    volatile uint8_t pending_events;
} Button;

void Button_Init(Button *button,
                 GPIO_TypeDef *port,
                 uint16_t pin,
                 GPIO_PinState pressed_level);

/* 必须每 1 ms 调用一次，用于软件消抖和按键时序判断。 */
void Button_Update1ms(Button *button);

/* 返回并清空已经产生的事件；多个事件使用按位或组合。 */
uint8_t Button_GetEvents(Button *button);

#endif
