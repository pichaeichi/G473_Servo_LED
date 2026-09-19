#include "button.h"

#define BUTTON_DEBOUNCE_TIME_MS    20U
#define BUTTON_DOUBLE_GAP_MS      350U
#define BUTTON_LONG_PRESS_MS     1000U

static uint8_t Button_ReadPressed(const Button *button)
{
    return (HAL_GPIO_ReadPin(button->port, button->pin) == button->pressed_level) ? 1U : 0U;
}

void Button_Init(Button *button,
                 GPIO_TypeDef *port,
                 uint16_t pin,
                 GPIO_PinState pressed_level)
{
    button->port = port;
    button->pin = pin;
    button->pressed_level = pressed_level;
    button->debounce_ms = 0U;
    button->press_ms = 0U;
    button->click_wait_ms = 0U;
    button->stable_pressed = Button_ReadPressed(button);
    button->long_reported = 0U;
    button->click_count = 0U;
    button->pending_events = BUTTON_EVENT_NONE;
}

void Button_Update1ms(Button *button)
{
    const uint8_t raw_pressed = Button_ReadPressed(button);

    if (raw_pressed != button->stable_pressed)
    {
        if (button->debounce_ms < BUTTON_DEBOUNCE_TIME_MS)
        {
            button->debounce_ms++;
        }

        if (button->debounce_ms >= BUTTON_DEBOUNCE_TIME_MS)
        {
            button->stable_pressed = raw_pressed;
            button->debounce_ms = 0U;

            if (button->stable_pressed != 0U)
            {
                button->press_ms = 0U;
                button->long_reported = 0U;
            }
            else if (button->long_reported == 0U)
            {
                if (button->click_count == 0U)
                {
                    button->click_count = 1U;
                    button->click_wait_ms = 0U;
                }
                else
                {
                    button->pending_events |= BUTTON_EVENT_DOUBLE;
                    button->click_count = 0U;
                    button->click_wait_ms = 0U;
                }
            }
        }
    }
    else
    {
        button->debounce_ms = 0U;
    }

    if (button->stable_pressed != 0U)
    {
        if (button->press_ms < BUTTON_LONG_PRESS_MS)
        {
            button->press_ms++;
        }

        if ((button->press_ms >= BUTTON_LONG_PRESS_MS) && (button->long_reported == 0U))
        {
            button->pending_events |= BUTTON_EVENT_LONG;
            button->long_reported = 1U;
            button->click_count = 0U;
            button->click_wait_ms = 0U;
        }
    }
    else if (button->click_count == 1U)
    {
        if (button->click_wait_ms < BUTTON_DOUBLE_GAP_MS)
        {
            button->click_wait_ms++;
        }
        else
        {
            button->pending_events |= BUTTON_EVENT_SHORT;
            button->click_count = 0U;
            button->click_wait_ms = 0U;
        }
    }
}

uint8_t Button_GetEvents(Button *button)
{
    uint8_t events;
    const uint32_t primask = __get_PRIMASK();

    __disable_irq();
    events = button->pending_events;
    button->pending_events = BUTTON_EVENT_NONE;

    if (primask == 0U)
    {
        __enable_irq();
    }

    return events;
}
