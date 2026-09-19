#ifndef APP_H
#define APP_H

#include "main.h"
#include <stdint.h>

typedef struct
{
    uint8_t enabled;
    uint8_t led_on;
    uint8_t frequency_x10;
    uint8_t amplitude_deg;
    int16_t servo_angle_deg;
} AppStatus;

/* 可在 CLion Live Watches 中观察这个变量。 */
extern volatile AppStatus g_app_status;

void App_Init(void);
void App_Process(void);

#endif
